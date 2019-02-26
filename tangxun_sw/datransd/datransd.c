/**
 *  @file datransd.c
 *
 *  @brief Tangxun transaction daemon
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "logger.h"
#include "process.h"
#include "datransd.h"
#include "files.h"
#include "xmalloc.h"
#include "network.h"
#include "stocklist.h"
#include "syncsem.h"
#include "webclient.h"
#include "bases.h"
#include "envvar.h"
#include "stringparse.h"
#include "parsedata.h"
#include "jiukun.h"
#include "xtime.h"
#include "datastat.h"
#include "stocktrade.h"

/* --- private data structures --------------------------------------------- */

#define DATRANSD_FAKE_DATA  0 /*For testing purpose*/

#define STOCK_OPEN_POSITION_FILE_NAME  "StockOpenPosition.txt"
#define STOCK_TRANS_FILE_NAME  "StockTrans.txt"

static olchar_t * ls_pstrQuotationServer = "hq.sinajs.cn";

typedef struct
{
    u32 id_u8Reserved[8];

    basic_chain_t * id_pbcChain;
    utimer_t * id_putUtimer;

    boolean_t id_bDownloaded;

    ip_addr_t id_iaServerAddr;
    webclient_t * id_pwWebclient;
} internal_datransd_t;

/*for worker thread*/
static boolean_t ls_bToTerminateWorkerThread = FALSE;
static thread_id_t ls_tiWorkerThreadId;
#define MAX_RESOURCE_COUNT  1

/*for worker thread and chain thread*/
typedef struct
{
    boolean_t sqr_bBusy;
    boolean_t sqr_bData;
    u8 sqr_u8Reserved[6];
    olsize_t sqr_sData;
    olchar_t * sqr_pstrData;
} stock_quo_raw_t;

#define MAX_RAW_QUO  5
/*In byte, one stock has the data size*/
#define DATA_SIZE_PER_STOCK  300
static olsize_t ls_sRawQuoDataMalloc;
static stock_quo_raw_t ls_sqrRawQuo[MAX_RAW_QUO];
static sync_sem_t ls_ssRawQuoSem;
static sync_mutex_t ls_smRawQuoLock;

typedef struct transd_stock_info
{
    list_head_t tsi_lhList;

    boolean_t tsi_bDelete;
    boolean_t tsi_bToCloseout;
    u8 tsi_u8Position;
    u8 tsi_u8Operation;
    boolean_t tsi_bData;
    u8 tsi_u8Reserved[3];
    oldouble_t tsi_dbPrice;

    stock_info_t * tsi_psiStock;

    stock_quo_t tsi_sqQuo;
#define STAT_ARBI_STRING_LEN  240
    olchar_t tsi_strStatArbi[STAT_ARBI_STRING_LEN];
} transd_stock_info_t;

#define MAX_STOCK_SECTOR 100
static list_head_t ls_lhStockSector[MAX_STOCK_SECTOR];
static olint_t ls_nStockSector = 0;

static sync_mutex_t ls_smStockInSectorLock;
static olint_t ls_nCurStockSector = 0;
static olchar_t * ls_pstrStockInSector[MAX_STOCK_SECTOR];
static olint_t ls_nStockInSector[MAX_STOCK_SECTOR];
static olint_t ls_nMaxStockPerReq = 0;

/*for calulating the correlation between 2 stocks*/
static oldouble_t * ls_pdbStockArray1, * ls_pdbStockArray2;

/* --- private routine section --------------------------------------------- */
static u32 _daGetQuotation(void * pData);

static void _copyRawQuo(u8 * pu8Body, olsize_t sBody)
{
    olint_t i;

    logInfoMsg("copy raw quo");

    acquireSyncMutex(&ls_smRawQuoLock);
    for (i = 0; i < MAX_RAW_QUO; i ++)
    {
        if (! ls_sqrRawQuo[i].sqr_bBusy && ! ls_sqrRawQuo[i].sqr_bData)
        {
            ls_sqrRawQuo[i].sqr_sData = sBody;
            if (ls_sqrRawQuo[i].sqr_sData > ls_sRawQuoDataMalloc)
            {
                logErrMsg(
                    OLERR_PROGRAM_ERROR,
                    "da get quo, wrong packet or buffer is too small");
                ls_sqrRawQuo[i].sqr_sData = ls_sRawQuoDataMalloc;
            }
            else
            {
                logInfoMsg("da get quo, size %d", ls_sqrRawQuo[i].sqr_sData);
                memcpy(
                    ls_sqrRawQuo[i].sqr_pstrData, pu8Body,
                    ls_sqrRawQuo[i].sqr_sData);

                ls_sqrRawQuo[i].sqr_bData = TRUE;
            }

            break;
        }
    }
    releaseSyncMutex(&ls_smRawQuoLock);

    if (i == MAX_RAW_QUO)
    {
        logErrMsg(
            OLERR_PROGRAM_ERROR,
            "da get quo, cannot find free buffer");
    }

    upSyncSem(&ls_ssRawQuoSem);
}

static u32 _quoOnResponse(
    asocket_t * pAsocket, olint_t InterruptFlag,
    packet_header_t * header, void * user, boolean_t * pbPause)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t * buf = NULL;
    olsize_t size;
    internal_datransd_t * pid = (internal_datransd_t *)user;

    logInfoMsg("quo on response, InterruptFlag %d", InterruptFlag);

    if (InterruptFlag == WIF_WEB_DATAOBJECT_DESTROYED)
    {
        logInfoMsg("quo on response, web data obj is destroyed");
        return u32Ret;
    }

    getRawPacket(header, &buf, &size);
    logDataMsgWithAscii(
        (u8 *)buf, size, "quo on response, body %d", header->ph_sBody);
    xfree((void **)&buf);

    if (header->ph_sBody > 0)
    {
        _copyRawQuo(header->ph_pu8Body, header->ph_sBody);
    }

    addUtimerItem(
        pid->id_putUtimer, pid, DA_GET_QUO_INTERVAL,
        _daGetQuotation, NULL);

    return u32Ret;
}

static void _refreshStrStockList(
    list_head_t * head, olint_t idx, olchar_t ** ppstr, olint_t * count)
{
    transd_stock_info_t * ptsi;
    list_head_t * pos;
    olchar_t * pstr;

    pstr = ppstr[idx];
    count[idx] = 0;
    pstr[0] = '\0';
    listForEach(&head[idx], pos)
    {
        ptsi = listEntry(pos, transd_stock_info_t, tsi_lhList);
        ol_strcat(pstr, ptsi->tsi_psiStock->si_strCode);
        ol_strcat(pstr, ",");
        count[idx] ++;
    }

    pstr[strlen(pstr)] = '\0';
    logInfoMsg("refresh str stock list for %s", pstr);

    return;
}

static u32 _getSinaQuotation(internal_datransd_t * pid)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t buffer[2048];
    olchar_t strStockList[512];
    olsize_t len;
    olint_t count = 0;
    olint_t nCurStockSector = ls_nCurStockSector;

    strStockList[0] = '\0';
    acquireSyncMutex(&ls_smStockInSectorLock);
    while (ls_nCurStockSector < ls_nStockSector)
    {
        if (count + ls_nStockInSector[ls_nCurStockSector] > ls_nMaxStockPerReq)
            break;
        count += ls_nStockInSector[ls_nCurStockSector];
        if (ls_nStockInSector[ls_nCurStockSector] > 0)
            ol_strcat(strStockList, ls_pstrStockInSector[ls_nCurStockSector]);
        ls_nCurStockSector ++;
        if (ls_nCurStockSector == ls_nStockSector)
            ls_nCurStockSector = 0;
        if (ls_nCurStockSector == nCurStockSector)
            break;
    }
    releaseSyncMutex(&ls_smStockInSectorLock);
    strStockList[strlen(strStockList) - 1] = '\0';

    logInfoMsg("get sina quo, send req for %s", strStockList);
    len = ol_snprintf(
        buffer, 2048,
        "GET /list=%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 6.1; rv:42.0) Gecko/20100101 Firefox/42.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Language: en-us,en;q=0.5\r\n"
        "Accept-Encoding: identity\r\n" //gzip,deflate\r\n"
        "Connection: keep-alive\r\n"
        "\r\n", strStockList, ls_pstrQuotationServer);

    u32Ret = pipelineWebRequestEx(
        pid->id_pwWebclient, &pid->id_iaServerAddr, 80, buffer, len, FALSE, NULL, 0,
        FALSE, _quoOnResponse, pid);

    return u32Ret;
}

static u32 _daGetQuotation(void * pData)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_datransd_t * pid;
#if DATRANSD_FAKE_DATA

    logInfoMsg("get quotation");
    pid = (internal_datransd_t *)pData;

    upSyncSem(&ls_ssRawQuoSem);

    addUtimerItem(
        pid->id_putUtimer, pid, DA_GET_QUO_INTERVAL,
        _daGetQuotation, NULL);

#else
    struct tm * ptm;
    time_t tCur;
    olchar_t buf[100];

    pid = (internal_datransd_t *)pData;

    logInfoMsg("get quotation");

    tCur = time(NULL);
    ptm = localtime(&tCur);

    ol_sprintf(buf, "%02d:%02d", ptm->tm_hour, ptm->tm_min);
    if ((strcmp(buf, "09:30") < 0) ||
        (strcmp(buf, "15:00") > 0) ||
        ((strcmp(buf, "11:30") > 0) &&
         (strcmp(buf, "13:00") < 0)))
    {
        logInfoMsg("get quotation, not in trading time");
        addUtimerItem(
            pid->id_putUtimer, pid, DA_GET_QUO_INTERVAL * 5,
            _daGetQuotation, NULL);
        return u32Ret;
    }

    u32Ret = _getSinaQuotation(pid);
#endif

    return u32Ret;
}

#if 0
/*if we can get 'max' stock per request, how many times we can get all stocks*/
static olint_t _estiReqCount(olint_t * pncount, olint_t num, olint_t max)
{
    olint_t i, times = 0, count;

    count = 0;
    for (i = 0; i < num; i ++)
    {
        if (count + pncount[i] > max)
        {
            times ++;
            count = 0;
        }

        count += pncount[i];
    }
    if (count > 0)
        times ++;

    return times;
}
#endif

static u32 _newStockQuo(
    list_head_t * head, olint_t num, olint_t * pncount, olint_t max)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i, numreq = 0, nMaxEntry;
    transd_stock_info_t * ptsi;
    list_head_t * pos;
    stock_quo_t * psq = NULL;
#if 0 //DATRANSD_FAKE_DATA
    olchar_t filepath[MAX_PATH_LEN];
    olint_t nNumOfEntry;
#endif
    numreq = 1; //_estiReqCount(pncount, num, max);
    nMaxEntry = (4 * 60 * 60) / (DA_GET_QUO_INTERVAL * numreq);
    logInfoMsg("new stock quo, %d req, max %d entry", numreq, nMaxEntry);

    u32Ret = xmalloc((void *)&ls_pdbStockArray1, sizeof(oldouble_t) * nMaxEntry);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = xmalloc((void *)&ls_pdbStockArray2, sizeof(oldouble_t) * nMaxEntry);

    for (i = 0; (i < num) && (u32Ret == OLERR_NO_ERROR); i ++)
    {
        listForEach(&head[i], pos)
        {
            ptsi = listEntry(pos, transd_stock_info_t, tsi_lhList);
            psq = &ptsi->tsi_sqQuo;
            logInfoMsg("new stock quo for %s", psq->sq_strCode);
            psq->sq_nMaxEntry = nMaxEntry;
            u32Ret = xmalloc(
                (void *)&psq->sq_pqeEntry,
                sizeof(quo_entry_t) * psq->sq_nMaxEntry);
        }
    }

#if 0 //DATRANSD_FAKE_DATA
    if (u32Ret == OLERR_NO_ERROR)
    {
        psq->sq_dbOpeningPrice = 10;
        ol_sprintf(filepath, "quotation-%s.xls", psq->sq_strCode);
        nNumOfEntry = psq->sq_nMaxEntry;
        if (readStockQuotationFile(
                filepath, psq->sq_pqeEntry,
                &nNumOfEntry) == OLERR_NO_ERROR)
        {
            logInfoMsg("Succeed to read %s", filepath);
            psq->sq_nMaxEntry = nNumOfEntry;
        }
        else
            logInfoMsg("Failed to read %s", filepath);
    }
#endif

    return u32Ret;
}

static u32 _daSaveTrans(transd_stock_info_t * ptsi)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t filepath[MAX_PATH_LEN];
    file_t fd = INVALID_FILE_VALUE;
    olsize_t size;
    stock_quo_t * psq = &ptsi->tsi_sqQuo;
    quo_entry_t * entry = &psq->sq_pqeEntry[psq->sq_nNumOfEntry - 1];

    ol_sprintf(filepath, "%s", STOCK_TRANS_FILE_NAME);
    u32Ret = openFile2(
        filepath, O_WRONLY | O_APPEND | O_CREAT,
        DEFAULT_CREATE_FILE_MODE, &fd);
    if (u32Ret == OLERR_NO_ERROR)
    {
        size = ol_sprintf(
            filepath,
            "%s\t%s\t%s\t%s\t%s\t%.2f\n",
            psq->sq_strDate, entry->qe_strTime, psq->sq_strCode,
            getStringStockOperation(ptsi->tsi_u8Operation),
            getStringStockPosition(ptsi->tsi_u8Position),
            ptsi->tsi_dbPrice);
        writen(fd, filepath, size);

        closeFile(&fd);

        logInfoMsg("save trans, %s", filepath);
    }

    return u32Ret;
}

static u32 _daSaveOpenTrans(transd_stock_info_t * ptsi)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t filepath[MAX_PATH_LEN];
    file_t fd = INVALID_FILE_VALUE;
    olsize_t size;

    ol_sprintf(filepath, "%s", STOCK_OPEN_POSITION_FILE_NAME);

    u32Ret = openFile2(
        filepath, O_WRONLY | O_APPEND | O_CREAT,
        DEFAULT_CREATE_FILE_MODE, &fd);
    if (u32Ret == OLERR_NO_ERROR)
    {
        size = ol_sprintf(
            filepath,
            "%s\t%u\t%.2f\n",
            ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_u8Position,
            ptsi->tsi_dbPrice);
        writen(fd, filepath, size);

        closeFile(&fd);
    }

    return u32Ret;
}

static u32 _daReadOpenTransLine(transd_stock_info_t * ptsi, olchar_t * line)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strCode[32];
    oldouble_t dbPrice;
    u32 u32Value;

    sscanf(
        line, "%s\t%u\t%lf\n",
        strCode, &u32Value, &dbPrice);

    ptsi->tsi_bToCloseout = TRUE;
    ptsi->tsi_u8Position = (u8)u32Value;

    logInfoMsg(
        "read open trans line, closeout %s, pos %s",
        getStringPositive(ptsi->tsi_bToCloseout),
        getStringStockPosition(ptsi->tsi_u8Position));

    return u32Ret;
}

/*Name,OpeningPrice,LastClosingPrice,CurPrice,HighPrice,LowPrice,BuyPrice,SoldPrice,
  Volume,Amount,(BuyVolume,BuyPrice)[5],(SoldVolume,SoldPrice)[5],Date,Time
 */
static u32 _parseSinaQuotationDataOneLine(
    olchar_t * buf, olsize_t sbuf, stock_quo_t * psq, quo_entry_t * cur)
{
    u32 u32Ret = OLERR_NO_ERROR;
//    oldouble_t dbamount, dbtemp;
    parse_result_t * result = NULL;
    parse_result_field_t * field;
    olint_t index;
    olchar_t * start;

    start = buf + sbuf - 1;
    while (*buf != '"')
        buf ++;
    buf ++;
    while (*start != '"')
        start --;
    sbuf = start - buf + 1;

    u32Ret = parseString(&result, buf, 0, sbuf, ",", 1);
    if ((u32Ret == OLERR_NO_ERROR) &&
        (result->pr_u32NumOfResult != 33))
        u32Ret = OLERR_INVALID_DATA;

    if (u32Ret == OLERR_NO_ERROR)
    {
        field = result->pr_pprfFirst;
        index = 1;
        while ((field != NULL) && (u32Ret == OLERR_NO_ERROR))
        {
            if (index == 1)
                ;
            else if (index == 2)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &psq->sq_dbOpeningPrice);
            }
            else if (index == 3)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &psq->sq_dbLastClosingPrice);
            }
            else if (index == 4)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_dbCurPrice);
            }
            else if (index == 5)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_dbHighPrice);
            }
            else if (index == 6)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_dbLowPrice);
            }
            else if (index == 7)
                ;
            else if (index == 8)
                ;
            else if (index == 9)
            {
                u32Ret = getU64FromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_u64Volume);
            }
            else if (index == 10)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_dbAmount);
            }
            else if (index == 11)
            {
                u32Ret = getU64FromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpBuy[0].qdp_u64Volume);
            }
            else if (index == 12)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpBuy[0].qdp_dbPrice);
            }
            else if (index == 13)
            {
                u32Ret = getU64FromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpBuy[1].qdp_u64Volume);
            }
            else if (index == 14)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpBuy[1].qdp_dbPrice);
            }
            else if (index == 15)
            {
                u32Ret = getU64FromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpBuy[2].qdp_u64Volume);
            }
            else if (index == 16)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpBuy[2].qdp_dbPrice);
            }
            else if (index == 17)
            {
                u32Ret = getU64FromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpBuy[3].qdp_u64Volume);
            }
            else if (index == 18)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpBuy[3].qdp_dbPrice);
            }
            else if (index == 19)
            {
                u32Ret = getU64FromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpBuy[4].qdp_u64Volume);
            }
            else if (index == 20)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpBuy[4].qdp_dbPrice);
            }
            else if (index == 21)
            {
                u32Ret = getU64FromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpSold[0].qdp_u64Volume);
            }
            else if (index == 22)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpSold[0].qdp_dbPrice);
            }
            else if (index == 23)
            {
                u32Ret = getU64FromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpSold[1].qdp_u64Volume);
            }
            else if (index == 24)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpSold[1].qdp_dbPrice);
            }
            else if (index == 25)
            {
                u32Ret = getU64FromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpSold[2].qdp_u64Volume);
            }
            else if (index == 26)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpSold[2].qdp_dbPrice);
            }
            else if (index == 27)
            {
                u32Ret = getU64FromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpSold[3].qdp_u64Volume);
            }
            else if (index == 28)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpSold[3].qdp_dbPrice);
            }
            else if (index == 29)
            {
                u32Ret = getU64FromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpSold[4].qdp_u64Volume);
            }
            else if (index == 30)
            {
                u32Ret = getDoubleFromString(
                    field->prf_pstrData, field->prf_sData, &cur->qe_qdpSold[4].qdp_dbPrice);
            }
            else if (index == 31)
            {
                ol_strncpy(psq->sq_strDate, field->prf_pstrData, 10);
            }
            else if (index == 32)
            {
                ol_strncpy(cur->qe_strTime, field->prf_pstrData, 8);
            }
            else if (index == 33)
                ;

            field = field->prf_pprfNext;
            index ++;
        }
    }

    if (result != NULL)
        destroyParseResult(&result);

    return u32Ret;
}

static void _saveStockQuo(stock_quo_t * psq)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t j;
    olchar_t filepath[MAX_PATH_LEN];
    file_t fd = INVALID_FILE_VALUE;
    olsize_t size;
    olint_t syear, smonth, sday;
    olchar_t strDate[64];

    logInfoMsg("save stock quo");

    if ((psq->sq_pqeEntry == NULL) || (psq->sq_nNumOfEntry == 0))
        return;

    getDateToday(&syear, &smonth, &sday);
    getStringDate2(strDate, syear, smonth, sday);
    ol_sprintf(
        filepath, "%s%c%s%cquotation-%s.xls", getEnvVar(ENV_VAR_DATA_PATH),
        PATH_SEPARATOR, psq->sq_strCode, PATH_SEPARATOR, strDate);
    logInfoMsg("save stock quo to %s", filepath);

    u32Ret = openFile2(
        filepath, O_WRONLY | O_CREAT | O_TRUNC,
        DEFAULT_CREATE_FILE_MODE, &fd);
    if (u32Ret == OLERR_NO_ERROR)
    {
        for (j = 0; j < psq->sq_nNumOfEntry; j ++)
        {
            size = ol_sprintf(
                filepath,
                "%s\t%.2f\t%.2f\t%.2f\t%llu\t%.2f\t"
                "%.2f\t%llu\t%.2f\t%llu\t%.2f\t%llu\t%.2f\t%llu\t%.2f\t%llu\t"
                "%.2f\t%llu\t%.2f\t%llu\t%.2f\t%llu\t%.2f\t%llu\t%.2f\t%llu\n",
                psq->sq_pqeEntry[j].qe_strTime,
                psq->sq_pqeEntry[j].qe_dbCurPrice,
                psq->sq_pqeEntry[j].qe_dbHighPrice, psq->sq_pqeEntry[j].qe_dbLowPrice,
                psq->sq_pqeEntry[j].qe_u64Volume, psq->sq_pqeEntry[j].qe_dbAmount,
                psq->sq_pqeEntry[j].qe_qdpBuy[0].qdp_dbPrice,
                psq->sq_pqeEntry[j].qe_qdpBuy[0].qdp_u64Volume,
                psq->sq_pqeEntry[j].qe_qdpBuy[1].qdp_dbPrice,
                psq->sq_pqeEntry[j].qe_qdpBuy[1].qdp_u64Volume,
                psq->sq_pqeEntry[j].qe_qdpBuy[2].qdp_dbPrice,
                psq->sq_pqeEntry[j].qe_qdpBuy[2].qdp_u64Volume,
                psq->sq_pqeEntry[j].qe_qdpBuy[3].qdp_dbPrice,
                psq->sq_pqeEntry[j].qe_qdpBuy[3].qdp_u64Volume,
                psq->sq_pqeEntry[j].qe_qdpBuy[4].qdp_dbPrice,
                psq->sq_pqeEntry[j].qe_qdpBuy[4].qdp_u64Volume,
                psq->sq_pqeEntry[j].qe_qdpSold[0].qdp_dbPrice,
                psq->sq_pqeEntry[j].qe_qdpSold[0].qdp_u64Volume,
                psq->sq_pqeEntry[j].qe_qdpSold[1].qdp_dbPrice,
                psq->sq_pqeEntry[j].qe_qdpSold[1].qdp_u64Volume,
                psq->sq_pqeEntry[j].qe_qdpSold[2].qdp_dbPrice,
                psq->sq_pqeEntry[j].qe_qdpSold[2].qdp_u64Volume,
                psq->sq_pqeEntry[j].qe_qdpSold[3].qdp_dbPrice,
                psq->sq_pqeEntry[j].qe_qdpSold[3].qdp_u64Volume,
                psq->sq_pqeEntry[j].qe_qdpSold[4].qdp_dbPrice,
                psq->sq_pqeEntry[j].qe_qdpSold[4].qdp_u64Volume);

            writen(fd, filepath, size);
        }

        closeFile(&fd);
    }
}

static void _freeStockQuo(stock_quo_t * psq)
{
    logInfoMsg("free stock quo");

    _saveStockQuo(psq);

    if (psq->sq_pqeEntry != NULL)
        xfree((void **)&psq->sq_pqeEntry);
}

static void _freeTransdStockInfo(transd_stock_info_t ** info)
{
    transd_stock_info_t * ptsi = *info;

    logInfoMsg(
        "free transd stock info %s", ptsi->tsi_psiStock->si_strCode);
    listDel(&ptsi->tsi_lhList);
    _freeStockQuo(&ptsi->tsi_sqQuo);
    xfree((void **)info);
}

static u32 _newTransdStockInfo(
    list_head_t * head, stock_info_t * stockinfo, transd_stock_info_t ** ppInfo)
{
    u32 u32Ret = OLERR_NO_ERROR;
    transd_stock_info_t * ptsi = NULL;

    u32Ret = xcalloc((void **)&ptsi, sizeof(transd_stock_info_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        ptsi->tsi_psiStock = stockinfo;
        ol_strcpy(ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_psiStock->si_strCode);

        listAddTail(head, &ptsi->tsi_lhList);
    }

    if (u32Ret == OLERR_NO_ERROR)
        *ppInfo = ptsi;
    else if (ptsi != NULL)
        _freeTransdStockInfo(&ptsi);

    return u32Ret;
}

#if 0
static transd_stock_info_t * _getTransdStockInfo(
    list_head_t * plh, olint_t num, stock_info_t * stockinfo)
{
    transd_stock_info_t * ptsi;
    list_head_t * pos, * head;

    head = &plh[stockinfo->si_nIndustry];
    if (listIsEmpty(head))
        return NULL;

    listForEach(head, pos)
    {
        ptsi = listEntry(pos, transd_stock_info_t, tsi_lhList);

        if (ptsi->tsi_psiStock == stockinfo)
        {
            return ptsi;
        }
    }

    return NULL;
}
#endif

static transd_stock_info_t * _getTransdStockInfoByCode(
    list_head_t * head, olchar_t * code)
{
    transd_stock_info_t * ptsi;
    list_head_t * pos;

    listForEach(head, pos)
    {
        ptsi = listEntry(pos, transd_stock_info_t, tsi_lhList);

        if (strncmp(ptsi->tsi_sqQuo.sq_strCode, code, 8) == 0)
        {
            logInfoMsg(
                "get transd stock info by code, %s",
                ptsi->tsi_psiStock->si_strCode);
            return ptsi;
        }
    }

    logInfoMsg("get transd stock info by code, not found");
    return NULL;
}

static transd_stock_info_t * _getTransdStockInfoByCode2(
    list_head_t * head, olint_t num, olchar_t * code)
{
    list_head_t * pos;
    transd_stock_info_t * ptsi;
    olint_t i;
    static olint_t ls_nLastSectorIdx = 0;

    for (i = ls_nLastSectorIdx; i < num; i ++)
    {
        if (listIsEmpty(&head[i]))
            continue;
        listForEach(&head[i], pos)
        {
            ptsi = listEntry(pos, transd_stock_info_t, tsi_lhList);
            if (strncmp(ptsi->tsi_sqQuo.sq_strCode, code, 8) == 0)
            {
                logInfoMsg(
                    "get transd stock info by code 2, No.1, %s",
                    ptsi->tsi_psiStock->si_strCode);
                ls_nLastSectorIdx = i;
                return ptsi;
            }
        }
    }

    for (i = 0; i < ls_nLastSectorIdx; i ++)
    {
        if (listIsEmpty(&head[i]))
            continue;
        listForEach(&head[i], pos)
        {
            ptsi = listEntry(pos, transd_stock_info_t, tsi_lhList);
            if (strncmp(ptsi->tsi_sqQuo.sq_strCode, code, 8) == 0)
            {
                logInfoMsg(
                    "get transd stock info by code 2, No.2, %s",
                    ptsi->tsi_psiStock->si_strCode);
                ls_nLastSectorIdx = i;
                return ptsi;
            }
        }
    }

    logInfoMsg("get transd stock info by code 2, not found");
    return NULL;
}

static u32 _parseSinaQuotationData(
    stock_quo_raw_t * psqr, list_head_t * head, olint_t num)
{
    u32 u32Ret = OLERR_NO_ERROR;
    stock_quo_t * psq;
    quo_entry_t * cur;
    olchar_t * line, *start, * end;
    olchar_t * data = psqr->sqr_pstrData;
    olsize_t size = psqr->sqr_sData;
    olchar_t * strcode;
    transd_stock_info_t * ptsi;

    logInfoMsg("parse raw quo data");

    start = line = data;
    end = data + size;
    while (start < end)
    {
        if (*start == '\n')
        {
            *start = '\0';
            strcode = line + 11;
            ptsi = _getTransdStockInfoByCode2(head, num, strcode);
            if (ptsi == NULL)
            {
                /*Failed to find stock quo, it may be deleted*/
                /*ignore, doesn't matter*/

            }
            else if (ptsi->tsi_sqQuo.sq_nNumOfEntry <
                     ptsi->tsi_sqQuo.sq_nMaxEntry)
            {
                psq = &ptsi->tsi_sqQuo;
                cur = &psq->sq_pqeEntry[psq->sq_nNumOfEntry];
                memset(cur, 0, sizeof(*cur));
                u32Ret = _parseSinaQuotationDataOneLine(
                    line, start - line, psq, cur);
                if (u32Ret == OLERR_NO_ERROR)
                {
                    psq->sq_nNumOfEntry ++;
                    ptsi->tsi_bData = TRUE;
                    logInfoMsg(
                        "parse raw quo data, numofentry %u",
                        psq->sq_nNumOfEntry);
                }
            }

            line = start + 1;
        }

        start ++;
    }

    return u32Ret;
}

/*if TRUE, new data is comming and parsed
  if FALSE, no data*/
static u32 _parseRawQuo(list_head_t * head, olint_t num)
{
    u32 u32Ret = OLERR_INVALID_DATA;
#if DATRANSD_FAKE_DATA
    stock_quo_t * psq;
    list_head_t * pos;

    logInfoMsg("parse raw quo");

    listForEach(head, pos)
    {
        psq = listEntry(pos, stock_quo_t, sq_lhList);

        psq->sq_nNumOfEntry ++;
        logInfoMsg(
            "parse raw quo for %s, numofentry %u",
            psq->sq_strCode, psq->sq_nNumOfEntry);
    }

    u32Ret = OLERR_NO_ERROR;

    return u32Ret;
#else
    olint_t i;

    logInfoMsg("parse raw quo");

    acquireSyncMutex(&ls_smRawQuoLock);
    for (i = 0; i < MAX_RAW_QUO; i ++)
    {
        if (! ls_sqrRawQuo[i].sqr_bBusy && ls_sqrRawQuo[i].sqr_bData)
        {
            ls_sqrRawQuo[i].sqr_bBusy = TRUE;
            releaseSyncMutex(&ls_smRawQuoLock);

            u32Ret = _parseSinaQuotationData(
                &ls_sqrRawQuo[i], head, num);

            acquireSyncMutex(&ls_smRawQuoLock);
            ls_sqrRawQuo[i].sqr_bBusy = FALSE;
            ls_sqrRawQuo[i].sqr_bData = FALSE;
            ls_sqrRawQuo[i].sqr_pstrData[0] = '\0';
            ls_sqrRawQuo[i].sqr_sData = 0;
            releaseSyncMutex(&ls_smRawQuoLock);

            return u32Ret;
        }
    }
    releaseSyncMutex(&ls_smRawQuoLock);
#endif
    return u32Ret;
}

static boolean_t _isDaTradingToday(stock_quo_t * psq)
{
    if (psq->sq_dbOpeningPrice == 0)
    {
        /*opening price cannot be 0. If 0, no trading today*/
        logInfoMsg("No trading today for %s", psq->sq_strCode);

        return FALSE;
    }

    return TRUE;
}

static boolean_t _isHighLimit(stock_quo_t * psq)
{
    quo_entry_t * entry = &psq->sq_pqeEntry[psq->sq_nNumOfEntry - 1];
    oldouble_t dbclose;

    dbclose = round(psq->sq_dbLastClosingPrice * 110) / 100;
    if (entry->qe_dbCurPrice >= dbclose)
        return TRUE;

    return FALSE;
}

static boolean_t _isLowLimit(stock_quo_t * psq)
{
    quo_entry_t * entry = &psq->sq_pqeEntry[psq->sq_nNumOfEntry - 1];
    oldouble_t dbclose;

    dbclose = round(psq->sq_dbLastClosingPrice * 90) / 100;
    if (entry->qe_dbCurPrice <= dbclose)
        return TRUE;

    return FALSE;
}

static void _markTsiDelete(transd_stock_info_t * ptsi)
{
    logInfoMsg("mart tsi delete, %s", ptsi->tsi_psiStock->si_strCode);
    ptsi->tsi_bDelete = TRUE;
}

static boolean_t _isReadyOpenPos(
    stock_quo_t * psq, boolean_t bHighLimit, list_head_t * plhIndex)
{
    quo_entry_t * entry = &psq->sq_pqeEntry[psq->sq_nNumOfEntry - 1];
    oldouble_t dbclose;
#define READY_OPEN_POS_PRICE_RATE  7.0
    dbclose = (entry->qe_dbCurPrice - psq->sq_dbLastClosingPrice) * 100 /
        psq->sq_dbLastClosingPrice;

    if (bHighLimit)
    {
        if (dbclose < READY_OPEN_POS_PRICE_RATE)
            return TRUE;
    }
    else
    {
        if (dbclose > -READY_OPEN_POS_PRICE_RATE)
            return TRUE;
    }
    logInfoMsg("is ready open pos, no");
    return FALSE;
}

static boolean_t _isStockCorrelated(
    transd_stock_info_t * ptsi1, transd_stock_info_t * ptsi2)
{
    stock_quo_t * psq1 = &ptsi1->tsi_sqQuo;
    stock_quo_t * psq2 = &ptsi2->tsi_sqQuo;
    quo_entry_t * last1, * first1, * first2; // * last2;
    olint_t hour, min, sec, seconds1, seconds2;
    olint_t i;
    oldouble_t dbvalue;
#define MIN_STOCK_QUO_CORRELATION  0.9
#define MIN_STOCK_QUO_CORRELATION_TIME  (30 * 60) /*in seconds*/
    /*do we have enough data?*/
    first1 = &psq1->sq_pqeEntry[0];
    first2 = &psq2->sq_pqeEntry[0];
    last1 = &psq1->sq_pqeEntry[psq1->sq_nNumOfEntry - 1];
//    last2 = &psq2->sq_pqeEntry[psq2->sq_nNumOfEntry - 1];

    getTimeFromString(first1->qe_strTime, &hour, &min, &sec);
    seconds1 = convertTimeToSeconds(hour, min, sec);
    getTimeFromString(last1->qe_strTime, &hour, &min, &sec);
    seconds2 = convertTimeToSeconds(hour, min, sec);

    /* would 30 minutes be OK*/
    if (seconds2 - seconds1 < MIN_STOCK_QUO_CORRELATION_TIME)
        return FALSE;

    for (i = 0; i < psq1->sq_nNumOfEntry; i ++)
    {
        ls_pdbStockArray1[i] = first1->qe_dbCurPrice;
        ls_pdbStockArray2[i] = first2->qe_dbCurPrice;
    }

    dbvalue = getCorrelation(
        ls_pdbStockArray1, ls_pdbStockArray2, psq1->sq_nNumOfEntry, &dbvalue);
    logInfoMsg("is stock cor, %s:%s, %.2f", psq1->sq_strCode, psq2->sq_strCode, dbvalue);
    if (dbvalue < MIN_STOCK_QUO_CORRELATION)
        return FALSE;

    logInfoMsg("is stock cor, yes");

    return TRUE;
}

static u32 _tryOpenPosForStock(
    list_head_t * head, transd_stock_info_t * ptsi, list_head_t * plhIndex)
{
    u32 u32Ret = OLERR_NO_ERROR;
    stock_quo_t * psq = &ptsi->tsi_sqQuo;
    quo_entry_t * entry;
    boolean_t bHighLimit, bLowLimit;
    olint_t i, count;
    transd_stock_info_t * ptsi2;
    boolean_t bHasPair = FALSE;

    logInfoMsg("try open pos stock");

    bHighLimit = _isHighLimit(psq);
    bLowLimit = _isLowLimit(psq);

    if (bHighLimit || bLowLimit)
    {
        logInfoMsg(
            "try open pos stock, %s reach %s, state arbi %s",
            psq->sq_strCode, bHighLimit ? "high limit" : "low limit",
            ptsi->tsi_strStatArbi);
        count = ol_strlen(ptsi->tsi_strStatArbi) / 9;
        for (i = 0; i < count; i ++)
        {
            ptsi2 = _getTransdStockInfoByCode(
                head, ptsi->tsi_strStatArbi + i * 9);
            if (ptsi2 == NULL)
                continue;

            bHasPair = TRUE;
            psq = &ptsi2->tsi_sqQuo;
            entry = &psq->sq_pqeEntry[psq->sq_nNumOfEntry - 1];

            if (! _isStockCorrelated(ptsi, ptsi2))
                continue;

            if (! _isReadyOpenPos(psq, bHighLimit, plhIndex))
                continue;

            _markTsiDelete(ptsi2);
            ptsi2->tsi_u8Operation = STOCK_OP_BUY;
            if (bHighLimit)
            {
                ptsi2->tsi_u8Position = STOCK_POSITION_FULL;
                ptsi2->tsi_dbPrice = entry->qe_qdpSold[0].qdp_dbPrice;
            }
            else
            {
                ptsi2->tsi_u8Position = STOCK_POSITION_SHORT;
                ptsi2->tsi_dbPrice = entry->qe_qdpBuy[0].qdp_dbPrice;
            }

            logInfoMsg(
                "try open pos stock, open %s for stock %s with price %.2f",
                getStringStockPosition(ptsi2->tsi_u8Position),
                psq->sq_strCode, ptsi2->tsi_dbPrice);

            _daSaveOpenTrans(ptsi2);
            _daSaveTrans(ptsi2);
        }

        if (! bHasPair)
            _markTsiDelete(ptsi);
    }

    return u32Ret;
}

static u32 _daCloseoutPosition(transd_stock_info_t * ptsi)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t filepath[MAX_PATH_LEN];
    file_t fd = INVALID_FILE_VALUE;
    file_t fd2 = INVALID_FILE_VALUE;
    olchar_t line[256];
    olsize_t sline;
    stock_quo_t * psq = &ptsi->tsi_sqQuo;
    quo_entry_t * entry = &psq->sq_pqeEntry[psq->sq_nNumOfEntry - 1];

    logInfoMsg("closeout pos for stock %s", ptsi->tsi_sqQuo.sq_strCode);

    /*clean the line in the open position file*/
    ol_sprintf(filepath, "%s", STOCK_OPEN_POSITION_FILE_NAME);
    u32Ret = openFile(filepath, O_RDONLY, &fd);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_sprintf(filepath, "%s.tmpfile", STOCK_OPEN_POSITION_FILE_NAME);
        u32Ret = openFile2(
            filepath, O_WRONLY | O_CREAT | O_TRUNC,
            DEFAULT_CREATE_FILE_MODE, &fd2);
        if (u32Ret == OLERR_NO_ERROR)
        {
            do
            {
                sline = sizeof(line);
                u32Ret = readLine(fd, line, &sline);
                if (u32Ret == OLERR_NO_ERROR)
                {
                    if (strncmp(line, psq->sq_strCode, 8) != 0)
                        writen(fd2, line, sline);
                }
            } while (u32Ret == OLERR_NO_ERROR);

            closeFile(&fd2);
        }

        if (u32Ret == OLERR_END_OF_FILE)
            u32Ret = OLERR_NO_ERROR;

        closeFile(&fd);

        removeFile(STOCK_OPEN_POSITION_FILE_NAME);
        renameFile(filepath, STOCK_OPEN_POSITION_FILE_NAME);
    }

    ptsi->tsi_u8Operation = STOCK_OP_SELL;
    if (ptsi->tsi_u8Position == STOCK_POSITION_FULL)
        ptsi->tsi_dbPrice = entry->qe_qdpBuy[0].qdp_dbPrice;
    else
        ptsi->tsi_dbPrice = entry->qe_qdpSold[0].qdp_dbPrice;

    _daSaveTrans(ptsi);

    return u32Ret;
}

static u32 _daStartPolicy(list_head_t * head, olint_t num)
{
    u32 u32Ret = OLERR_NO_ERROR;
    list_head_t * pos, * temp;
    transd_stock_info_t * ptsi;
    olint_t i;
    boolean_t bRefresh = FALSE;

    logInfoMsg("start the policy");

    for (i = 0; i < num; i ++)
    {
        if (listIsEmpty(&head[i]))
            continue;

        ptsi = listEntry(head[i].lh_plhNext, transd_stock_info_t, tsi_lhList);
        if (! ptsi->tsi_bData)
            continue;

        listForEach(&head[i], pos)
        {
            ptsi = listEntry(pos, transd_stock_info_t, tsi_lhList);
            if (isStockInfoIndex(ptsi->tsi_sqQuo.sq_strCode))
                /*Ignore index*/
                continue;

            if (ptsi->tsi_bDelete)
                continue;

            if (! ptsi->tsi_bData)
                /*no data is coming yet*/
                continue;

            ptsi->tsi_bData = FALSE;
            if (! _isDaTradingToday(&ptsi->tsi_sqQuo))
            {
                /*no trading today*/
                _markTsiDelete(ptsi);
                continue;
            }

            if (ptsi->tsi_bToCloseout)
            {
                /*closeout position for stock*/
                _daCloseoutPosition(ptsi);
                _markTsiDelete(ptsi);
                continue;
            }

            _tryOpenPosForStock(&head[i], ptsi, &head[0]);
        }

        bRefresh= FALSE;
        listForEachSafe(&head[i], pos, temp) 
        {
            ptsi = listEntry(pos, transd_stock_info_t, tsi_lhList);
            if (ptsi->tsi_bDelete)
            {
                logInfoMsg(
                    "start the policy, delete %s", ptsi->tsi_sqQuo.sq_strCode);
                _freeTransdStockInfo(&ptsi);
                bRefresh = TRUE;
            }
        }

        if (bRefresh)
        {
            /*Failed to find stock quo, it may be deleted*/
            acquireSyncMutex(&ls_smStockInSectorLock);
            _refreshStrStockList(
                head, i, ls_pstrStockInSector, ls_nStockInSector);
            releaseSyncMutex(&ls_smStockInSectorLock);
        }
    }

    return u32Ret;
}

THREAD_RETURN_VALUE _daWorkerThread(void * pArg)
{
    u32 u32Ret = OLERR_NO_ERROR;

    logInfoMsg("worker thread %lu", getCurrentThreadId());

    while (! ls_bToTerminateWorkerThread)
    {
        u32Ret = downSyncSem(&ls_ssRawQuoSem);
        if (u32Ret == OLERR_NO_ERROR)
        {
            if (_parseRawQuo(ls_lhStockSector, ls_nStockSector) == OLERR_NO_ERROR)
            {
                /*New data is comming, the policy can start*/
                _daStartPolicy(ls_lhStockSector, ls_nStockSector);
            }
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
        logInfoMsg("worker thread quits");

    THREAD_RETURN(u32Ret);
}

static u32 _startWorkerThread(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i;

    logInfoMsg("start worker thread");

    u32Ret = initSyncSem(&ls_ssRawQuoSem, 0, MAX_RESOURCE_COUNT);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = initSyncMutex(&ls_smRawQuoLock);

    if (u32Ret == OLERR_NO_ERROR)
    {
        ls_sRawQuoDataMalloc = ls_nMaxStockPerReq * DATA_SIZE_PER_STOCK;
        logInfoMsg("start worker thread, malloc %d", ls_sRawQuoDataMalloc);
        for (i = 0; i < MAX_RAW_QUO; i ++)
        {
            u32Ret = xmalloc(
                (void **)&ls_sqrRawQuo[i].sqr_pstrData,
                ls_sRawQuoDataMalloc);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = createThread(
            &ls_tiWorkerThreadId, NULL, _daWorkerThread, NULL);
    }
    
    return u32Ret;
}

static void _destroyTransdStockInfoList(list_head_t * head, olint_t num)
{
    transd_stock_info_t * ptsi;
    list_head_t * pos, * temp;
    olint_t i;

    logInfoMsg("destroy transd stock info list");

    for (i = 0; i < num; i ++)
    {
        if (listIsEmpty(&head[i]))
            continue;

        listForEachSafe(&head[i], pos, temp)
        {
            ptsi = listEntry(pos, transd_stock_info_t, tsi_lhList);

            _freeTransdStockInfo(&ptsi);
        }
    }
}

#if 0
static u32 _initStockInduListHead(
    list_head_t * head, stock_indu_info_t * stockindu)
{
    u32 u32Ret = OLERR_NO_ERROR;
    stock_info_t * stockinfo;
    transd_stock_info_t * ptsi;

    stockinfo = getFirstStockInfo();
    while ((stockinfo != NULL) && (u32Ret == OLERR_NO_ERROR))
    {
        if (stockinfo->si_nIndustry == stockindu->sii_nId)
        {
            u32Ret = xcalloc((void **)&ptsi, sizeof(transd_stock_info_t));
            if (u32Ret == OLERR_NO_ERROR)
            {
                ptsi->tsi_psiStock = stockinfo;
                ol_strcpy(ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_psiStock->si_strCode);

                listAddTail(head, &ptsi->tsi_lhList);
            }
        }

        stockinfo = getNextStockInfo(stockinfo);
    }

    return u32Ret;
}
#endif

/*the first stock indu is for index, stock to closeout*/
static u32 _initFirstStockIndu(list_head_t * head)
{
    u32 u32Ret = OLERR_NO_ERROR;
    file_t fd = INVALID_FILE_VALUE;
    olchar_t line[512];
    olsize_t sline;
    stock_info_t * stockinfo;
    transd_stock_info_t * ptsi;
    olint_t nStock = 0;

    u32Ret = xcalloc((void **)&ptsi, sizeof(transd_stock_info_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        ptsi->tsi_psiStock = getStockInfoIndex(SH_COMPOSITE_INDEX);
        ol_strcpy(ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_psiStock->si_strCode);
        listAddTail(head, &ptsi->tsi_lhList);
        nStock ++;

        u32Ret = xcalloc((void **)&ptsi, sizeof(transd_stock_info_t));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ptsi->tsi_psiStock = getStockInfoIndex(SZ_COMPOSITIONAL_INDEX);
        ol_strcpy(ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_psiStock->si_strCode);
        listAddTail(head, &ptsi->tsi_lhList);
        nStock ++;

        u32Ret = xcalloc((void **)&ptsi, sizeof(transd_stock_info_t));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = openFile(STOCK_OPEN_POSITION_FILE_NAME, O_RDONLY, &fd);
        if (u32Ret == OLERR_NO_ERROR)
        {
            do
            {
                sline = sizeof(line);
                u32Ret = readLine(fd, line, &sline);
                if (u32Ret == OLERR_NO_ERROR)
                {
                    getStockInfo(line, &stockinfo);
                    if (stockinfo == NULL)
                        continue;

                    u32Ret = xcalloc((void **)&ptsi, sizeof(transd_stock_info_t));
                }

                if (u32Ret == OLERR_NO_ERROR)
                {
                    ptsi->tsi_psiStock = stockinfo;
                    ol_strcpy(ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_psiStock->si_strCode);
                    listAddTail(head, &ptsi->tsi_lhList);
                    nStock ++;

                    u32Ret = _daReadOpenTransLine(ptsi, line);
                }
            } while (u32Ret == OLERR_NO_ERROR);

            if (u32Ret == OLERR_END_OF_FILE)
                u32Ret = OLERR_NO_ERROR;

            closeFile(&fd);
        }
        else /*doesn't matter if failed*/
            u32Ret = OLERR_NO_ERROR;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = openFile(STOCK_TOUGH_LIST_FILE_NAME, O_RDONLY, &fd);
        if (u32Ret == OLERR_NO_ERROR)
        {
            do
            {
                sline = sizeof(line);
                u32Ret = readLine(fd, line, &sline);
                if (u32Ret == OLERR_NO_ERROR)
                {
                    getStockInfo(line, &stockinfo);
                    if (stockinfo == NULL)
                        continue;

                    u32Ret = xcalloc((void **)&ptsi, sizeof(transd_stock_info_t));
                }

                if (u32Ret == OLERR_NO_ERROR)
                {
                    ptsi->tsi_psiStock = stockinfo;
                    ol_strcpy(ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_psiStock->si_strCode);
                    listAddTail(head, &ptsi->tsi_lhList);
                    nStock ++;
                }
            } while (u32Ret == OLERR_NO_ERROR);

            if (u32Ret == OLERR_END_OF_FILE)
                u32Ret = OLERR_NO_ERROR;

            closeFile(&fd);
        }
        else /*doesn't matter if failed*/
            u32Ret = OLERR_NO_ERROR;
    }

    logInfoMsg("olint_t first stock indu, %u stocks", nStock);

    return u32Ret;
}

static u32 _addStrStockStatArbi(
    list_head_t * head, stock_info_t * psi, olchar_t * code)
{
    u32 u32Ret = OLERR_NO_ERROR;
    transd_stock_info_t * ptsi;

    u32Ret = _newTransdStockInfo(head, psi, &ptsi);
    if (u32Ret == OLERR_NO_ERROR)
    {
        code[strlen(code) - 1] = '\0';
        assert(strlen(code) < STAT_ARBI_STRING_LEN);
        ol_strcpy(ptsi->tsi_strStatArbi, code);
    }

    return u32Ret;
}

static u32 _addStockStatArbi(
    list_head_t * head, olint_t max, olint_t * num, olchar_t * filename)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t fd;
    olchar_t * buf;
    olsize_t size = 128 * 1024, sline;
    olchar_t line[512], code[16];
    stock_info_t * psi;
    list_head_t * listhead;

    memset(code, 0, sizeof(code));
    allocMemory((void **)&buf, size, 0);

    u32Ret = openFile(filename, O_RDONLY, &fd);
    if (u32Ret == OLERR_NO_ERROR)
    {
        do
        {
            sline = sizeof(line);
            u32Ret = readLine(fd, line, &sline);
            if (u32Ret == OLERR_NO_ERROR)
            {
                if (line[0] == '[')
                {
                    assert(*num < max);
                    listhead = &head[*num];
                    *num = *num + 1;
                    continue;
                }

                ol_strncpy(code, line, 8);
                getStockInfo(code, &psi);
                if (psi == NULL)
                    continue;

                u32Ret = _addStrStockStatArbi(listhead, psi, line + 9);
            }
        } while (u32Ret == OLERR_NO_ERROR);

        if (u32Ret == OLERR_END_OF_FILE)
            u32Ret = OLERR_NO_ERROR;

        closeFile(&fd);
    }
    else
        logInfoMsg(
            "add stock stat arbi, file %s is not found", filename);

    freeMemory((void **)&buf);

    return u32Ret;
}

static u32 _initStockIndu(
    list_head_t * head, olint_t max, olint_t * num, olchar_t * filename)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i;

    logInfoMsg("init stock indu");

    for (i = 0; i < max; i++)
    {
        listInit(&head[i]);
    }

    u32Ret = _initFirstStockIndu(&head[0]);
    if (u32Ret == OLERR_NO_ERROR)
    {
        *num = *num + 1;

        u32Ret = _addStockStatArbi(head, max, num, filename);
    }

    return u32Ret;
}

static void _dumpStockIndu(list_head_t * head, olint_t num)
{
    olchar_t buf[256];
    list_head_t * pos;
    transd_stock_info_t * ptsi;
    olint_t i, count = 0;

    for (i = 0; i < num; i ++)
    {
        buf[0] = '\0';
        if (i == 0)
            ol_strcpy(buf, "Index-Tough-Closeout: ");
        else
            ol_sprintf(buf, "sector %d: ", i);
        logInfoMsg("%s", buf);

        if (listIsEmpty(&head[i]))
            continue;

        listForEach(&head[i], pos)
        {
            buf[0] = '\0';
            count ++;
            ptsi = listEntry(pos, transd_stock_info_t, tsi_lhList);
            ol_strcat(buf, ptsi->tsi_psiStock->si_strCode);
            ol_strcat(buf, "(");
            if (ptsi->tsi_strStatArbi[0] == '\0')
                ol_strcat(buf, "Empty");
            else
                ol_strcat(buf, ptsi->tsi_strStatArbi);
            ol_strcat(buf, ")");
            logInfoMsg("%s", buf);
        }
    }
    logInfoMsg("Total %d stocks", count);
}

#if 0
static void _removeTransdStock(list_head_t * head, olint_t num)
{
    olint_t i;
    list_head_t * pos, * temp;
    transd_stock_info_t * ptsi;

    for (i = 1; i < num; i ++)
    {
        if (listIsEmpty(&head[i]))
            continue;

        listForEachSafe(&head[i], pos, temp)
        {
            ptsi = listEntry(pos, transd_stock_info_t, tsi_lhList);
            if (strlen(ptsi->tsi_strStatArbi) == 0)
            {
                _freeTransdStockInfo(&ptsi);
            }
        }
    }
}
#endif

static u32 _initStrStockList(
    list_head_t * head, olint_t num, olchar_t ** ppstr, olint_t * pnCount, olint_t * pnMax)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i, max = 0;
    list_head_t * pos;
    transd_stock_info_t * ptsi;
    olchar_t * pstr;

    for (i = 0; (i < num) && (u32Ret == OLERR_NO_ERROR); i ++)
    {
        listForEach(&head[i], pos)
            pnCount[i] ++;
        logInfoMsg("init str stock list, total %d stocks", pnCount[i]);

        if (pnCount[i] > max)
            max = pnCount[i];

        u32Ret = xcalloc((void **)&pstr, pnCount[i] * 9 + 1);
        if (u32Ret == OLERR_NO_ERROR)
        {
            ppstr[i] = pstr;
            listForEach(&head[i], pos)
            {
                ptsi = listEntry(pos, transd_stock_info_t, tsi_lhList);

                ol_strcat(pstr, ptsi->tsi_psiStock->si_strCode);
                ol_strcat(pstr, ",");
            }
            pstr[strlen(pstr)] = '\0';

            logInfoMsg("init str stock list, %s", pstr);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
#define MAX_STOCKS_PER_REQ  20
        *pnMax = MAX_STOCKS_PER_REQ;
//        *pnMax = max;
        logInfoMsg("init str stock list, max %d stocks", *pnMax);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 createDatransd(datransd_t ** ppDatransd, datransd_param_t * pdp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_datransd_t * pid;
    olchar_t strExecutablePath[MAX_PATH_LEN];
    struct hostent * servp;
    webclient_param_t wp;

    logInfoMsg("create datransd");

    u32Ret = xcalloc((void **)&pid, sizeof(internal_datransd_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        /*change the working directory*/
        getDirectoryName(strExecutablePath, MAX_PATH_LEN, pdp->dp_pstrCmdLine);
        if (strlen(strExecutablePath) > 0)
            u32Ret = setCurrentWorkingDirectory(strExecutablePath);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = getHostByName(ls_pstrQuotationServer, &servp);

    if (u32Ret == OLERR_NO_ERROR)
    {
        setIpV4Addr(&pid->id_iaServerAddr, *(long *)(servp->h_addr));

        u32Ret = _initStockIndu(
            ls_lhStockSector, MAX_STOCK_SECTOR, &ls_nStockSector,
            STOCK_STAT_ARBI_LIST_FILE_NAME);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = initSyncMutex(&ls_smStockInSectorLock);

    if (u32Ret == OLERR_NO_ERROR)
    {
        _dumpStockIndu(ls_lhStockSector, ls_nStockSector);

        u32Ret = _initStrStockList(
            ls_lhStockSector, ls_nStockSector, ls_pstrStockInSector,
            ls_nStockInSector, &ls_nMaxStockPerReq);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _newStockQuo(
            ls_lhStockSector, ls_nStockSector, ls_nStockInSector, ls_nMaxStockPerReq);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _startWorkerThread();

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = createBasicChain(&pid->id_pbcChain);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = createUtimer(pid->id_pbcChain, &pid->id_putUtimer);

    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(&wp, 0, sizeof(wp));
        wp.wp_nPoolSize = 5;
        wp.wp_sBuffer = ALIGN(ls_sRawQuoDataMalloc, 1024); //4096;
        logInfoMsg(
            "create datransd, buffer size for webclient %d", wp.wp_sBuffer);

        u32Ret = createWebclient(pid->id_pbcChain, &pid->id_pwWebclient, &wp);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = addUtimerItem(
            pid->id_putUtimer, pid, DA_GET_QUO_INTERVAL,
            _daGetQuotation, NULL);

    if (u32Ret == OLERR_NO_ERROR)
        *ppDatransd = pid;
    else if (pid != NULL)
        destroyDatransd((datransd_t **)&pid);

    return u32Ret;
}

u32 destroyDatransd(datransd_t ** ppDatransd)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_datransd_t * pid;
    olint_t i;

    assert((ppDatransd != NULL) && (*ppDatransd != NULL));

    pid = (internal_datransd_t *)*ppDatransd;

    if (pid->id_pwWebclient != NULL)
        destroyWebclient((void **)&pid->id_pwWebclient);

    if (pid->id_pbcChain != NULL)
        destroyBasicChain(&pid->id_pbcChain);

    xfree(ppDatransd);

    for (i = 0; i < MAX_RAW_QUO; i ++)
    {
        if (ls_sqrRawQuo[i].sqr_pstrData != NULL)
            xfree((void **)&ls_sqrRawQuo[i].sqr_pstrData);
    }

    for (i = 0; i < ls_nStockSector; i ++)
    {
        if (ls_pstrStockInSector[i] != NULL)
            xfree((void **)&ls_pstrStockInSector[i]);
    }

    if (ls_pdbStockArray1 != NULL)
        xfree((void **)&ls_pdbStockArray1);
    if (ls_pdbStockArray2 != NULL)
        xfree((void **)&ls_pdbStockArray2);

    _destroyTransdStockInfoList(ls_lhStockSector, ls_nStockSector);

    finiSyncSem(&ls_ssRawQuoSem);
    finiSyncMutex(&ls_smRawQuoLock);
    finiSyncMutex(&ls_smStockInSectorLock);

    return u32Ret;
}

u32 startDatransd(datransd_t * pDatransd)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_datransd_t * pid;

    assert(pDatransd != NULL);

    pid = (internal_datransd_t *)pDatransd;

    u32Ret = startBasicChain(pid->id_pbcChain);

    return u32Ret;
}

u32 stopDatransd(datransd_t * pDatransd)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_datransd_t * pid;

    assert(pDatransd != NULL);

    pid = (internal_datransd_t *)pDatransd;

    stopBasicChain(pid->id_pbcChain);

    ls_bToTerminateWorkerThread = TRUE;

    return u32Ret;
}

u32 setDefaultDatransdParam(datransd_param_t * pdp)
{
    u32 u32Ret = OLERR_NO_ERROR;

    memset(pdp, 0, sizeof(datransd_param_t));


    return u32Ret;
}

/*---------------------------------------------------------------------------*/


