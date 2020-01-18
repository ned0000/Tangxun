/**
 *  @file datransd.c
 *
 *  @brief Tangxun transaction daemon.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_process.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "jf_network.h"
#include "jf_sem.h"
#include "jf_webclient.h"
#include "jf_listhead.h"
#include "jf_string.h"
#include "jf_jiukun.h"
#include "jf_time.h"
#include "jf_date.h"
#include "jf_thread.h"

#include "tx_env.h"
#include "datastat.h"
#include "stocktrade.h"
#include "parsedata.h"
#include "stocklist.h"
#include "datransd.h"

/* --- private data structures ------------------------------------------------------------------ */

#define DATRANSD_NAME                  "datransd"

/** For testing purpose.
 */
#define DATRANSD_FAKE_DATA             (0)

#define STOCK_OPEN_POSITION_FILE_NAME  "StockOpenPosition.txt"
#define STOCK_TRANS_FILE_NAME  "StockTrans.txt"

static olchar_t * ls_pstrQuotationServer = "hq.sinajs.cn";

typedef struct
{
    u32 id_u8Reserved[8];

    jf_network_chain_t * id_pjncChain;
    jf_network_utimer_t * id_pjnuUtimer;

    boolean_t id_bDownloaded;

    jf_ipaddr_t id_jiServerAddr;
    jf_webclient_t * id_pjwWebclient;
} internal_datransd_t;

/*for worker thread*/
static boolean_t ls_bToTerminateWorkerThread = FALSE;
static jf_thread_id_t ls_jtiWorkerThreadId;
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
static jf_sem_t ls_ssRawQuoSem;
static jf_mutex_t ls_smRawQuoLock;

typedef struct transd_stock_info
{
    jf_listhead_t tsi_jlList;

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
static jf_listhead_t ls_jlStockSector[MAX_STOCK_SECTOR];
static olint_t ls_nStockSector = 0;

static jf_mutex_t ls_smStockInSectorLock;
static olint_t ls_nCurStockSector = 0;
static olchar_t * ls_pstrStockInSector[MAX_STOCK_SECTOR];
static olint_t ls_nStockInSector[MAX_STOCK_SECTOR];
static olint_t ls_nMaxStockPerReq = 0;

/*for calulating the correlation between 2 stocks*/
static oldouble_t * ls_pdbStockArray1, * ls_pdbStockArray2;

/* --- private routine section ------------------------------------------------------------------ */
static u32 _daGetQuotation(void * pData);

static void _copyRawQuo(u8 * pu8Body, olsize_t sBody)
{
    olint_t i;

    jf_logger_logInfoMsg("copy raw quo");

    jf_mutex_acquire(&ls_smRawQuoLock);
    for (i = 0; i < MAX_RAW_QUO; i ++)
    {
        if (! ls_sqrRawQuo[i].sqr_bBusy && ! ls_sqrRawQuo[i].sqr_bData)
        {
            ls_sqrRawQuo[i].sqr_sData = sBody;
            if (ls_sqrRawQuo[i].sqr_sData > ls_sRawQuoDataMalloc)
            {
                jf_logger_logErrMsg(
                    JF_ERR_PROGRAM_ERROR,
                    "da get quo, wrong packet or buffer is too small");
                ls_sqrRawQuo[i].sqr_sData = ls_sRawQuoDataMalloc;
            }
            else
            {
                jf_logger_logInfoMsg("da get quo, size %d", ls_sqrRawQuo[i].sqr_sData);
                memcpy(
                    ls_sqrRawQuo[i].sqr_pstrData, pu8Body,
                    ls_sqrRawQuo[i].sqr_sData);

                ls_sqrRawQuo[i].sqr_bData = TRUE;
            }

            break;
        }
    }
    jf_mutex_release(&ls_smRawQuoLock);

    if (i == MAX_RAW_QUO)
    {
        jf_logger_logErrMsg(
            JF_ERR_PROGRAM_ERROR,
            "da get quo, cannot find free buffer");
    }

    jf_sem_up(&ls_ssRawQuoSem);
}

static u32 _quoOnResponse(
    jf_network_asocket_t * pAsocket, jf_webclient_event_t InterruptFlag,
    jf_httpparser_packet_header_t * header, void * user)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * buf = NULL;
    olsize_t size;
    internal_datransd_t * pid = (internal_datransd_t *)user;

    jf_logger_logInfoMsg("quo on response, InterruptFlag %d", InterruptFlag);

    if (InterruptFlag != JF_WEBCLIENT_EVENT_INCOMING_DATA)
    {
        jf_logger_logInfoMsg("quo on response, web data obj is destroyed");
        return u32Ret;
    }

    jf_httpparser_getRawPacket(header, &buf, &size);
    jf_logger_logDataMsgWithAscii(
        (u8 *)buf, size, "quo on response, body %d", header->jhph_sBody);
    jf_mem_free((void **)&buf);

    if (header->jhph_sBody > 0)
    {
        _copyRawQuo(header->jhph_pu8Body, header->jhph_sBody);
    }

    jf_network_addUtimerItem(
        pid->id_pjnuUtimer, pid, DA_GET_QUO_INTERVAL,
        _daGetQuotation, NULL);

    return u32Ret;
}

static void _refreshStrStockList(
    jf_listhead_t * head, olint_t idx, olchar_t ** ppstr, olint_t * count)
{
    transd_stock_info_t * ptsi;
    jf_listhead_t * pos;
    olchar_t * pstr;

    pstr = ppstr[idx];
    count[idx] = 0;
    pstr[0] = '\0';
    jf_listhead_forEach(&head[idx], pos)
    {
        ptsi = jf_listhead_getEntry(pos, transd_stock_info_t, tsi_jlList);
        ol_strcat(pstr, ptsi->tsi_psiStock->si_strCode);
        ol_strcat(pstr, ",");
        count[idx] ++;
    }

    pstr[strlen(pstr)] = '\0';
    jf_logger_logInfoMsg("refresh str stock list for %s", pstr);

    return;
}

static u32 _getSinaQuotation(internal_datransd_t * pid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t buffer[2048];
    olchar_t strStockList[512];
    olsize_t len;
    olint_t count = 0;
    olint_t nCurStockSector = ls_nCurStockSector;

    strStockList[0] = '\0';
    jf_mutex_acquire(&ls_smStockInSectorLock);
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
    jf_mutex_release(&ls_smStockInSectorLock);
    strStockList[strlen(strStockList) - 1] = '\0';

    jf_logger_logInfoMsg("get sina quo, send req for %s", strStockList);
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

    u32Ret = jf_webclient_sendHttpHeaderAndBody(
        pid->id_pjwWebclient, &pid->id_jiServerAddr, 80, buffer, len, NULL, 0, _quoOnResponse, pid);

    return u32Ret;
}

static u32 _daGetQuotation(void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_datransd_t * pid;
#if DATRANSD_FAKE_DATA

    jf_logger_logInfoMsg("get quotation");
    pid = (internal_datransd_t *)pData;

    jf_sem_up(&ls_ssRawQuoSem);

    jf_network_addUtimerItem(
        pid->id_pjnuUtimer, pid, DA_GET_QUO_INTERVAL,
        _daGetQuotation, NULL);

#else
    struct tm * ptm;
    time_t tCur;
    olchar_t buf[100];

    pid = (internal_datransd_t *)pData;

    jf_logger_logInfoMsg("get quotation");

    tCur = time(NULL);
    ptm = localtime(&tCur);

    ol_sprintf(buf, "%02d:%02d", ptm->tm_hour, ptm->tm_min);
    if ((strcmp(buf, "09:30") < 0) ||
        (strcmp(buf, "15:00") > 0) ||
        ((strcmp(buf, "11:30") > 0) &&
         (strcmp(buf, "13:00") < 0)))
    {
        jf_logger_logInfoMsg("get quotation, not in trading time");
        jf_network_addUtimerItem(
            pid->id_pjnuUtimer, pid, DA_GET_QUO_INTERVAL * 5,
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
    jf_listhead_t * head, olint_t num, olint_t * pncount, olint_t max)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, numreq = 0, nMaxEntry;
    transd_stock_info_t * ptsi;
    jf_listhead_t * pos;
    stock_quo_t * psq = NULL;
#if 0 //DATRANSD_FAKE_DATA
    olchar_t filepath[JF_LIMIT_MAX_PATH_LEN];
    olint_t nNumOfEntry;
#endif
    numreq = 1; //_estiReqCount(pncount, num, max);
    nMaxEntry = (4 * 60 * 60) / (DA_GET_QUO_INTERVAL * numreq);
    jf_logger_logInfoMsg("new stock quo, %d req, max %d entry", numreq, nMaxEntry);

    u32Ret = jf_mem_alloc((void *)&ls_pdbStockArray1, sizeof(oldouble_t) * nMaxEntry);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void *)&ls_pdbStockArray2, sizeof(oldouble_t) * nMaxEntry);

    for (i = 0; (i < num) && (u32Ret == JF_ERR_NO_ERROR); i ++)
    {
        jf_listhead_forEach(&head[i], pos)
        {
            ptsi = jf_listhead_getEntry(pos, transd_stock_info_t, tsi_jlList);
            psq = &ptsi->tsi_sqQuo;
            jf_logger_logInfoMsg("new stock quo for %s", psq->sq_strCode);
            psq->sq_nMaxEntry = nMaxEntry;
            u32Ret = jf_mem_alloc(
                (void *)&psq->sq_pqeEntry,
                sizeof(quo_entry_t) * psq->sq_nMaxEntry);
        }
    }

#if 0 //DATRANSD_FAKE_DATA
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        psq->sq_dbOpeningPrice = 10;
        ol_sprintf(filepath, "quotation-%s.xls", psq->sq_strCode);
        nNumOfEntry = psq->sq_nMaxEntry;
        if (readStockQuotationFile(
                filepath, psq->sq_pqeEntry,
                &nNumOfEntry) == JF_ERR_NO_ERROR)
        {
            jf_logger_logInfoMsg("Succeed to read %s", filepath);
            psq->sq_nMaxEntry = nNumOfEntry;
        }
        else
            jf_logger_logInfoMsg("Failed to read %s", filepath);
    }
#endif

    return u32Ret;
}

static u32 _daSaveTrans(transd_stock_info_t * ptsi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t filepath[JF_LIMIT_MAX_PATH_LEN];
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    olsize_t size;
    stock_quo_t * psq = &ptsi->tsi_sqQuo;
    quo_entry_t * entry = &psq->sq_pqeEntry[psq->sq_nNumOfEntry - 1];

    ol_sprintf(filepath, "%s", STOCK_TRANS_FILE_NAME);
    u32Ret = jf_file_openWithMode(
        filepath, O_WRONLY | O_APPEND | O_CREAT,
        JF_FILE_DEFAULT_CREATE_MODE, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        size = ol_sprintf(
            filepath,
            "%s\t%s\t%s\t%s\t%s\t%.2f\n",
            psq->sq_strDate, entry->qe_strTime, psq->sq_strCode,
            getStringStockOperation(ptsi->tsi_u8Operation),
            getStringStockPosition(ptsi->tsi_u8Position),
            ptsi->tsi_dbPrice);
        jf_file_writen(fd, filepath, size);

        jf_file_close(&fd);

        jf_logger_logInfoMsg("save trans, %s", filepath);
    }

    return u32Ret;
}

static u32 _daSaveOpenTrans(transd_stock_info_t * ptsi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t filepath[JF_LIMIT_MAX_PATH_LEN];
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    olsize_t size;

    ol_sprintf(filepath, "%s", STOCK_OPEN_POSITION_FILE_NAME);

    u32Ret = jf_file_openWithMode(
        filepath, O_WRONLY | O_APPEND | O_CREAT,
        JF_FILE_DEFAULT_CREATE_MODE, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        size = ol_sprintf(
            filepath,
            "%s\t%u\t%.2f\n",
            ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_u8Position,
            ptsi->tsi_dbPrice);
        jf_file_writen(fd, filepath, size);

        jf_file_close(&fd);
    }

    return u32Ret;
}

static u32 _daReadOpenTransLine(transd_stock_info_t * ptsi, olchar_t * line)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strCode[32];
    oldouble_t dbPrice;
    u32 u32Value;

    sscanf(
        line, "%s\t%u\t%lf\n",
        strCode, &u32Value, &dbPrice);

    ptsi->tsi_bToCloseout = TRUE;
    ptsi->tsi_u8Position = (u8)u32Value;

    jf_logger_logInfoMsg(
        "read open trans line, closeout %s, pos %s",
        jf_string_getStringPositive(ptsi->tsi_bToCloseout),
        getStringStockPosition(ptsi->tsi_u8Position));

    return u32Ret;
}

/*Name,OpeningPrice,LastClosingPrice,CurPrice,HighPrice,LowPrice,BuyPrice,SoldPrice,
  Volume,Amount,(BuyVolume,BuyPrice)[5],(SoldVolume,SoldPrice)[5],Date,Time
 */
static u32 _parseSinaQuotationDataOneLine(
    olchar_t * buf, olsize_t sbuf, stock_quo_t * psq, quo_entry_t * cur)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    oldouble_t dbamount, dbtemp;
    jf_string_parse_result_t * result = NULL;
    jf_string_parse_result_field_t * field;
    olint_t index;
    olchar_t * start;

    start = buf + sbuf - 1;
    while (*buf != '"')
        buf ++;
    buf ++;
    while (*start != '"')
        start --;
    sbuf = start - buf + 1;

    u32Ret = jf_string_parse(&result, buf, 0, sbuf, ",", 1);
    if ((u32Ret == JF_ERR_NO_ERROR) &&
        (result->jspr_u32NumOfResult != 33))
        u32Ret = JF_ERR_INVALID_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = result->jspr_pjsprfFirst;
        index = 1;
        while ((field != NULL) && (u32Ret == JF_ERR_NO_ERROR))
        {
            if (index == 1)
                ;
            else if (index == 2)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &psq->sq_dbOpeningPrice);
            }
            else if (index == 3)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &psq->sq_dbLastClosingPrice);
            }
            else if (index == 4)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_dbCurPrice);
            }
            else if (index == 5)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_dbHighPrice);
            }
            else if (index == 6)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_dbLowPrice);
            }
            else if (index == 7)
                ;
            else if (index == 8)
                ;
            else if (index == 9)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_u64Volume);
            }
            else if (index == 10)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_dbAmount);
            }
            else if (index == 11)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpBuy[0].qdp_u64Volume);
            }
            else if (index == 12)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpBuy[0].qdp_dbPrice);
            }
            else if (index == 13)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpBuy[1].qdp_u64Volume);
            }
            else if (index == 14)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpBuy[1].qdp_dbPrice);
            }
            else if (index == 15)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpBuy[2].qdp_u64Volume);
            }
            else if (index == 16)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpBuy[2].qdp_dbPrice);
            }
            else if (index == 17)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpBuy[3].qdp_u64Volume);
            }
            else if (index == 18)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpBuy[3].qdp_dbPrice);
            }
            else if (index == 19)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpBuy[4].qdp_u64Volume);
            }
            else if (index == 20)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpBuy[4].qdp_dbPrice);
            }
            else if (index == 21)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpSold[0].qdp_u64Volume);
            }
            else if (index == 22)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpSold[0].qdp_dbPrice);
            }
            else if (index == 23)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpSold[1].qdp_u64Volume);
            }
            else if (index == 24)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpSold[1].qdp_dbPrice);
            }
            else if (index == 25)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpSold[2].qdp_u64Volume);
            }
            else if (index == 26)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpSold[2].qdp_dbPrice);
            }
            else if (index == 27)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpSold[3].qdp_u64Volume);
            }
            else if (index == 28)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpSold[3].qdp_dbPrice);
            }
            else if (index == 29)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpSold[4].qdp_u64Volume);
            }
            else if (index == 30)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->qe_qdpSold[4].qdp_dbPrice);
            }
            else if (index == 31)
            {
                ol_strncpy(psq->sq_strDate, field->jsprf_pstrData, 10);
            }
            else if (index == 32)
            {
                ol_strncpy(cur->qe_strTime, field->jsprf_pstrData, 8);
            }
            else if (index == 33)
                ;

            field = field->jsprf_pjsprfNext;
            index ++;
        }
    }

    if (result != NULL)
        jf_string_destroyParseResult(&result);

    return u32Ret;
}

static void _saveStockQuo(stock_quo_t * psq)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t j;
    olchar_t filepath[JF_LIMIT_MAX_PATH_LEN];
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    olsize_t size;
    olint_t syear, smonth, sday;
    olchar_t strDate[64];

    jf_logger_logInfoMsg("save stock quo");

    if ((psq->sq_pqeEntry == NULL) || (psq->sq_nNumOfEntry == 0))
        return;

    jf_date_getDateToday(&syear, &smonth, &sday);
    jf_date_getStringDate2(strDate, syear, smonth, sday);
    ol_sprintf(
        filepath, "%s%c%s%cquotation-%s.xls", tx_env_getVar(TX_ENV_VAR_DATA_PATH),
        PATH_SEPARATOR, psq->sq_strCode, PATH_SEPARATOR, strDate);
    jf_logger_logInfoMsg("save stock quo to %s", filepath);

    u32Ret = jf_file_openWithMode(
        filepath, O_WRONLY | O_CREAT | O_TRUNC, JF_FILE_DEFAULT_CREATE_MODE, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
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

            jf_file_writen(fd, filepath, size);
        }

        jf_file_close(&fd);
    }
}

static void _freeStockQuo(stock_quo_t * psq)
{
    jf_logger_logInfoMsg("free stock quo");

    _saveStockQuo(psq);

    if (psq->sq_pqeEntry != NULL)
        jf_mem_free((void **)&psq->sq_pqeEntry);
}

static void _freeTransdStockInfo(transd_stock_info_t ** info)
{
    transd_stock_info_t * ptsi = *info;

    jf_logger_logInfoMsg(
        "free transd stock info %s", ptsi->tsi_psiStock->si_strCode);
    jf_listhead_del(&ptsi->tsi_jlList);
    _freeStockQuo(&ptsi->tsi_sqQuo);
    jf_mem_free((void **)info);
}

static u32 _newTransdStockInfo(
    jf_listhead_t * head, stock_info_t * stockinfo, transd_stock_info_t ** ppInfo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    transd_stock_info_t * ptsi = NULL;

    u32Ret = jf_mem_calloc((void **)&ptsi, sizeof(transd_stock_info_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ptsi->tsi_psiStock = stockinfo;
        ol_strcpy(ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_psiStock->si_strCode);

        jf_listhead_addTail(head, &ptsi->tsi_jlList);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppInfo = ptsi;
    else if (ptsi != NULL)
        _freeTransdStockInfo(&ptsi);

    return u32Ret;
}

#if 0
static transd_stock_info_t * _getTransdStockInfo(
    jf_listhead_t * pjl, olint_t num, stock_info_t * stockinfo)
{
    transd_stock_info_t * ptsi;
    jf_listhead_t * pos, * head;

    head = &pjl[stockinfo->si_nIndustry];
    if (jf_listhead_isEmpty(head))
        return NULL;

    jf_listhead_forEach(head, pos)
    {
        ptsi = jf_listhead_getEntry(pos, transd_stock_info_t, tsi_jlList);

        if (ptsi->tsi_psiStock == stockinfo)
        {
            return ptsi;
        }
    }

    return NULL;
}
#endif

static transd_stock_info_t * _getTransdStockInfoByCode(
    jf_listhead_t * head, olchar_t * code)
{
    transd_stock_info_t * ptsi;
    jf_listhead_t * pos;

    jf_listhead_forEach(head, pos)
    {
        ptsi = jf_listhead_getEntry(pos, transd_stock_info_t, tsi_jlList);

        if (strncmp(ptsi->tsi_sqQuo.sq_strCode, code, 8) == 0)
        {
            jf_logger_logInfoMsg(
                "get transd stock info by code, %s",
                ptsi->tsi_psiStock->si_strCode);
            return ptsi;
        }
    }

    jf_logger_logInfoMsg("get transd stock info by code, not found");
    return NULL;
}

static transd_stock_info_t * _getTransdStockInfoByCode2(
    jf_listhead_t * head, olint_t num, olchar_t * code)
{
    jf_listhead_t * pos;
    transd_stock_info_t * ptsi;
    olint_t i;
    static olint_t ls_nLastSectorIdx = 0;

    for (i = ls_nLastSectorIdx; i < num; i ++)
    {
        if (jf_listhead_isEmpty(&head[i]))
            continue;
        jf_listhead_forEach(&head[i], pos)
        {
            ptsi = jf_listhead_getEntry(pos, transd_stock_info_t, tsi_jlList);
            if (strncmp(ptsi->tsi_sqQuo.sq_strCode, code, 8) == 0)
            {
                jf_logger_logInfoMsg(
                    "get transd stock info by code 2, No.1, %s",
                    ptsi->tsi_psiStock->si_strCode);
                ls_nLastSectorIdx = i;
                return ptsi;
            }
        }
    }

    for (i = 0; i < ls_nLastSectorIdx; i ++)
    {
        if (jf_listhead_isEmpty(&head[i]))
            continue;
        jf_listhead_forEach(&head[i], pos)
        {
            ptsi = jf_listhead_getEntry(pos, transd_stock_info_t, tsi_jlList);
            if (strncmp(ptsi->tsi_sqQuo.sq_strCode, code, 8) == 0)
            {
                jf_logger_logInfoMsg(
                    "get transd stock info by code 2, No.2, %s",
                    ptsi->tsi_psiStock->si_strCode);
                ls_nLastSectorIdx = i;
                return ptsi;
            }
        }
    }

    jf_logger_logInfoMsg("get transd stock info by code 2, not found");
    return NULL;
}

static u32 _parseSinaQuotationData(
    stock_quo_raw_t * psqr, jf_listhead_t * head, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stock_quo_t * psq;
    quo_entry_t * cur;
    olchar_t * line, *start, * end;
    olchar_t * data = psqr->sqr_pstrData;
    olsize_t size = psqr->sqr_sData;
    olchar_t * strcode;
    transd_stock_info_t * ptsi;

    jf_logger_logInfoMsg("parse raw quo data");

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
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    psq->sq_nNumOfEntry ++;
                    ptsi->tsi_bData = TRUE;
                    jf_logger_logInfoMsg(
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
static u32 _parseRawQuo(jf_listhead_t * head, olint_t num)
{
    u32 u32Ret = JF_ERR_INVALID_DATA;
#if DATRANSD_FAKE_DATA
    stock_quo_t * psq;
    jf_listhead_t * pos;

    jf_logger_logInfoMsg("parse raw quo");

    jf_listhead_forEach(head, pos)
    {
        psq = jf_listhead_getEntry(pos, stock_quo_t, sq_jlList);

        psq->sq_nNumOfEntry ++;
        jf_logger_logInfoMsg(
            "parse raw quo for %s, numofentry %u",
            psq->sq_strCode, psq->sq_nNumOfEntry);
    }

    u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
#else
    olint_t i;

    jf_logger_logInfoMsg("parse raw quo");

    jf_mutex_acquire(&ls_smRawQuoLock);
    for (i = 0; i < MAX_RAW_QUO; i ++)
    {
        if (! ls_sqrRawQuo[i].sqr_bBusy && ls_sqrRawQuo[i].sqr_bData)
        {
            ls_sqrRawQuo[i].sqr_bBusy = TRUE;
            jf_mutex_release(&ls_smRawQuoLock);

            u32Ret = _parseSinaQuotationData(
                &ls_sqrRawQuo[i], head, num);

            jf_mutex_acquire(&ls_smRawQuoLock);
            ls_sqrRawQuo[i].sqr_bBusy = FALSE;
            ls_sqrRawQuo[i].sqr_bData = FALSE;
            ls_sqrRawQuo[i].sqr_pstrData[0] = '\0';
            ls_sqrRawQuo[i].sqr_sData = 0;
            jf_mutex_release(&ls_smRawQuoLock);

            return u32Ret;
        }
    }
    jf_mutex_release(&ls_smRawQuoLock);
#endif
    return u32Ret;
}

static boolean_t _isDaTradingToday(stock_quo_t * psq)
{
    if (psq->sq_dbOpeningPrice == 0)
    {
        /*opening price cannot be 0. If 0, no trading today*/
        jf_logger_logInfoMsg("No trading today for %s", psq->sq_strCode);

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
    jf_logger_logInfoMsg("mart tsi delete, %s", ptsi->tsi_psiStock->si_strCode);
    ptsi->tsi_bDelete = TRUE;
}

static boolean_t _isReadyOpenPos(
    stock_quo_t * psq, boolean_t bHighLimit, jf_listhead_t * pjlIndex)
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
    jf_logger_logInfoMsg("is ready open pos, no");
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

    jf_time_getTimeFromString(first1->qe_strTime, &hour, &min, &sec);
    seconds1 = jf_time_convertTimeToSeconds(hour, min, sec);
    jf_time_getTimeFromString(last1->qe_strTime, &hour, &min, &sec);
    seconds2 = jf_time_convertTimeToSeconds(hour, min, sec);

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
    jf_logger_logInfoMsg("is stock cor, %s:%s, %.2f", psq1->sq_strCode, psq2->sq_strCode, dbvalue);
    if (dbvalue < MIN_STOCK_QUO_CORRELATION)
        return FALSE;

    jf_logger_logInfoMsg("is stock cor, yes");

    return TRUE;
}

static u32 _tryOpenPosForStock(
    jf_listhead_t * head, transd_stock_info_t * ptsi, jf_listhead_t * pjlIndex)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stock_quo_t * psq = &ptsi->tsi_sqQuo;
    quo_entry_t * entry;
    boolean_t bHighLimit, bLowLimit;
    olint_t i, count;
    transd_stock_info_t * ptsi2;
    boolean_t bHasPair = FALSE;

    jf_logger_logInfoMsg("try open pos stock");

    bHighLimit = _isHighLimit(psq);
    bLowLimit = _isLowLimit(psq);

    if (bHighLimit || bLowLimit)
    {
        jf_logger_logInfoMsg(
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

            if (! _isReadyOpenPos(psq, bHighLimit, pjlIndex))
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

            jf_logger_logInfoMsg(
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
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t filepath[JF_LIMIT_MAX_PATH_LEN];
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    jf_file_t fd2 = JF_FILE_INVALID_FILE_VALUE;
    olchar_t line[256];
    olsize_t sline;
    stock_quo_t * psq = &ptsi->tsi_sqQuo;
    quo_entry_t * entry = &psq->sq_pqeEntry[psq->sq_nNumOfEntry - 1];

    jf_logger_logInfoMsg("closeout pos for stock %s", ptsi->tsi_sqQuo.sq_strCode);

    /*clean the line in the open position file*/
    ol_sprintf(filepath, "%s", STOCK_OPEN_POSITION_FILE_NAME);
    u32Ret = jf_file_open(filepath, O_RDONLY, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_sprintf(filepath, "%s.tmpfile", STOCK_OPEN_POSITION_FILE_NAME);
        u32Ret = jf_file_openWithMode(
            filepath, O_WRONLY | O_CREAT | O_TRUNC,
            JF_FILE_DEFAULT_CREATE_MODE, &fd2);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            do
            {
                sline = sizeof(line);
                u32Ret = jf_file_readLine(fd, line, &sline);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    if (strncmp(line, psq->sq_strCode, 8) != 0)
                        jf_file_writen(fd2, line, sline);
                }
            } while (u32Ret == JF_ERR_NO_ERROR);

            jf_file_close(&fd2);
        }

        if (u32Ret == JF_ERR_END_OF_FILE)
            u32Ret = JF_ERR_NO_ERROR;

        jf_file_close(&fd);

        jf_file_remove(STOCK_OPEN_POSITION_FILE_NAME);
        jf_file_rename(filepath, STOCK_OPEN_POSITION_FILE_NAME);
    }

    ptsi->tsi_u8Operation = STOCK_OP_SELL;
    if (ptsi->tsi_u8Position == STOCK_POSITION_FULL)
        ptsi->tsi_dbPrice = entry->qe_qdpBuy[0].qdp_dbPrice;
    else
        ptsi->tsi_dbPrice = entry->qe_qdpSold[0].qdp_dbPrice;

    _daSaveTrans(ptsi);

    return u32Ret;
}

static u32 _daStartPolicy(jf_listhead_t * head, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pos, * temp;
    transd_stock_info_t * ptsi;
    olint_t i;
    boolean_t bRefresh = FALSE;

    jf_logger_logInfoMsg("start the policy");

    for (i = 0; i < num; i ++)
    {
        if (jf_listhead_isEmpty(&head[i]))
            continue;

        ptsi = jf_listhead_getEntry(head[i].jl_pjlNext, transd_stock_info_t, tsi_jlList);
        if (! ptsi->tsi_bData)
            continue;

        jf_listhead_forEach(&head[i], pos)
        {
            ptsi = jf_listhead_getEntry(pos, transd_stock_info_t, tsi_jlList);
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
        jf_listhead_forEachSafe(&head[i], pos, temp) 
        {
            ptsi = jf_listhead_getEntry(pos, transd_stock_info_t, tsi_jlList);
            if (ptsi->tsi_bDelete)
            {
                jf_logger_logInfoMsg(
                    "start the policy, delete %s", ptsi->tsi_sqQuo.sq_strCode);
                _freeTransdStockInfo(&ptsi);
                bRefresh = TRUE;
            }
        }

        if (bRefresh)
        {
            /*Failed to find stock quo, it may be deleted*/
            jf_mutex_acquire(&ls_smStockInSectorLock);
            _refreshStrStockList(
                head, i, ls_pstrStockInSector, ls_nStockInSector);
            jf_mutex_release(&ls_smStockInSectorLock);
        }
    }

    return u32Ret;
}

JF_THREAD_RETURN_VALUE _daWorkerThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("worker thread %lu", jf_thread_getCurrentId());

    while (! ls_bToTerminateWorkerThread)
    {
        u32Ret = jf_sem_down(&ls_ssRawQuoSem);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (_parseRawQuo(ls_jlStockSector, ls_nStockSector) == JF_ERR_NO_ERROR)
            {
                /*New data is comming, the policy can start*/
                _daStartPolicy(ls_jlStockSector, ls_nStockSector);
            }
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        jf_logger_logInfoMsg("worker thread quits");

    JF_THREAD_RETURN(u32Ret);
}

static u32 _startWorkerThread(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;

    jf_logger_logInfoMsg("start worker thread");

    u32Ret = jf_sem_init(&ls_ssRawQuoSem, 0, MAX_RESOURCE_COUNT);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mutex_init(&ls_smRawQuoLock);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ls_sRawQuoDataMalloc = ls_nMaxStockPerReq * DATA_SIZE_PER_STOCK;
        jf_logger_logInfoMsg("start worker thread, malloc %d", ls_sRawQuoDataMalloc);
        for (i = 0; i < MAX_RAW_QUO; i ++)
        {
            u32Ret = jf_mem_alloc(
                (void **)&ls_sqrRawQuo[i].sqr_pstrData,
                ls_sRawQuoDataMalloc);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_thread_create(
            &ls_jtiWorkerThreadId, NULL, _daWorkerThread, NULL);
    }
    
    return u32Ret;
}

static void _destroyTransdStockInfoList(jf_listhead_t * head, olint_t num)
{
    transd_stock_info_t * ptsi;
    jf_listhead_t * pos, * temp;
    olint_t i;

    jf_logger_logInfoMsg("destroy transd stock info list");

    for (i = 0; i < num; i ++)
    {
        if (jf_listhead_isEmpty(&head[i]))
            continue;

        jf_listhead_forEachSafe(&head[i], pos, temp)
        {
            ptsi = jf_listhead_getEntry(pos, transd_stock_info_t, tsi_jlList);

            _freeTransdStockInfo(&ptsi);
        }
    }
}

#if 0
static u32 _initStockInduListHead(
    jf_listhead_t * head, stock_indu_info_t * stockindu)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stock_info_t * stockinfo;
    transd_stock_info_t * ptsi;

    stockinfo = getFirstStockInfo();
    while ((stockinfo != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        if (stockinfo->si_nIndustry == stockindu->sii_nId)
        {
            u32Ret = jf_mem_calloc((void **)&ptsi, sizeof(transd_stock_info_t));
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                ptsi->tsi_psiStock = stockinfo;
                ol_strcpy(ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_psiStock->si_strCode);

                jf_listhead_addTail(head, &ptsi->tsi_jlList);
            }
        }

        stockinfo = getNextStockInfo(stockinfo);
    }

    return u32Ret;
}
#endif

/*the first stock indu is for index, stock to closeout*/
static u32 _initFirstStockIndu(jf_listhead_t * head)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    olchar_t line[512];
    olsize_t sline;
    stock_info_t * stockinfo;
    transd_stock_info_t * ptsi;
    olint_t nStock = 0;

    u32Ret = jf_mem_calloc((void **)&ptsi, sizeof(transd_stock_info_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ptsi->tsi_psiStock = getStockInfoIndex(SH_COMPOSITE_INDEX);
        ol_strcpy(ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_psiStock->si_strCode);
        jf_listhead_addTail(head, &ptsi->tsi_jlList);
        nStock ++;

        u32Ret = jf_mem_calloc((void **)&ptsi, sizeof(transd_stock_info_t));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ptsi->tsi_psiStock = getStockInfoIndex(SZ_COMPOSITIONAL_INDEX);
        ol_strcpy(ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_psiStock->si_strCode);
        jf_listhead_addTail(head, &ptsi->tsi_jlList);
        nStock ++;

        u32Ret = jf_mem_calloc((void **)&ptsi, sizeof(transd_stock_info_t));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_file_open(STOCK_OPEN_POSITION_FILE_NAME, O_RDONLY, &fd);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            do
            {
                sline = sizeof(line);
                u32Ret = jf_file_readLine(fd, line, &sline);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    getStockInfo(line, &stockinfo);
                    if (stockinfo == NULL)
                        continue;

                    u32Ret = jf_mem_calloc((void **)&ptsi, sizeof(transd_stock_info_t));
                }

                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    ptsi->tsi_psiStock = stockinfo;
                    ol_strcpy(ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_psiStock->si_strCode);
                    jf_listhead_addTail(head, &ptsi->tsi_jlList);
                    nStock ++;

                    u32Ret = _daReadOpenTransLine(ptsi, line);
                }
            } while (u32Ret == JF_ERR_NO_ERROR);

            if (u32Ret == JF_ERR_END_OF_FILE)
                u32Ret = JF_ERR_NO_ERROR;

            jf_file_close(&fd);
        }
        else /*doesn't matter if failed*/
            u32Ret = JF_ERR_NO_ERROR;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_file_open(STOCK_TOUGH_LIST_FILE_NAME, O_RDONLY, &fd);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            do
            {
                sline = sizeof(line);
                u32Ret = jf_file_readLine(fd, line, &sline);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    getStockInfo(line, &stockinfo);
                    if (stockinfo == NULL)
                        continue;

                    u32Ret = jf_mem_calloc((void **)&ptsi, sizeof(transd_stock_info_t));
                }

                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    ptsi->tsi_psiStock = stockinfo;
                    ol_strcpy(ptsi->tsi_sqQuo.sq_strCode, ptsi->tsi_psiStock->si_strCode);
                    jf_listhead_addTail(head, &ptsi->tsi_jlList);
                    nStock ++;
                }
            } while (u32Ret == JF_ERR_NO_ERROR);

            if (u32Ret == JF_ERR_END_OF_FILE)
                u32Ret = JF_ERR_NO_ERROR;

            jf_file_close(&fd);
        }
        else /*doesn't matter if failed*/
            u32Ret = JF_ERR_NO_ERROR;
    }

    jf_logger_logInfoMsg("olint_t first stock indu, %u stocks", nStock);

    return u32Ret;
}

static u32 _addStrStockStatArbi(
    jf_listhead_t * head, stock_info_t * psi, olchar_t * code)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    transd_stock_info_t * ptsi;

    u32Ret = _newTransdStockInfo(head, psi, &ptsi);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        code[strlen(code) - 1] = '\0';
        assert(strlen(code) < STAT_ARBI_STRING_LEN);
        ol_strcpy(ptsi->tsi_strStatArbi, code);
    }

    return u32Ret;
}

static u32 _addStockStatArbi(
    jf_listhead_t * head, olint_t max, olint_t * num, olchar_t * filename)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t fd;
    olchar_t * buf;
    olsize_t size = 128 * 1024, sline;
    olchar_t line[512], code[16];
    stock_info_t * psi;
    jf_listhead_t * listhead;

    memset(code, 0, sizeof(code));
    jf_jiukun_allocMemory((void **)&buf, size);

    u32Ret = jf_file_open(filename, O_RDONLY, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        do
        {
            sline = sizeof(line);
            u32Ret = jf_file_readLine(fd, line, &sline);
            if (u32Ret == JF_ERR_NO_ERROR)
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
        } while (u32Ret == JF_ERR_NO_ERROR);

        if (u32Ret == JF_ERR_END_OF_FILE)
            u32Ret = JF_ERR_NO_ERROR;

        jf_file_close(&fd);
    }
    else
        jf_logger_logInfoMsg(
            "add stock stat arbi, file %s is not found", filename);

    jf_jiukun_freeMemory((void **)&buf);

    return u32Ret;
}

static u32 _initStockIndu(
    jf_listhead_t * head, olint_t max, olint_t * num, olchar_t * filename)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;

    jf_logger_logInfoMsg("init stock indu");

    for (i = 0; i < max; i++)
    {
        jf_listhead_init(&head[i]);
    }

    u32Ret = _initFirstStockIndu(&head[0]);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *num = *num + 1;

        u32Ret = _addStockStatArbi(head, max, num, filename);
    }

    return u32Ret;
}

static void _dumpStockIndu(jf_listhead_t * head, olint_t num)
{
    olchar_t buf[256];
    jf_listhead_t * pos;
    transd_stock_info_t * ptsi;
    olint_t i, count = 0;

    for (i = 0; i < num; i ++)
    {
        buf[0] = '\0';
        if (i == 0)
            ol_strcpy(buf, "Index-Tough-Closeout: ");
        else
            ol_sprintf(buf, "sector %d: ", i);
        jf_logger_logInfoMsg("%s", buf);

        if (jf_listhead_isEmpty(&head[i]))
            continue;

        jf_listhead_forEach(&head[i], pos)
        {
            buf[0] = '\0';
            count ++;
            ptsi = jf_listhead_getEntry(pos, transd_stock_info_t, tsi_jlList);
            ol_strcat(buf, ptsi->tsi_psiStock->si_strCode);
            ol_strcat(buf, "(");
            if (ptsi->tsi_strStatArbi[0] == '\0')
                ol_strcat(buf, "Empty");
            else
                ol_strcat(buf, ptsi->tsi_strStatArbi);
            ol_strcat(buf, ")");
            jf_logger_logInfoMsg("%s", buf);
        }
    }
    jf_logger_logInfoMsg("Total %d stocks", count);
}

#if 0
static void _removeTransdStock(jf_listhead_t * head, olint_t num)
{
    olint_t i;
    jf_listhead_t * pos, * temp;
    transd_stock_info_t * ptsi;

    for (i = 1; i < num; i ++)
    {
        if (jf_listhead_isEmpty(&head[i]))
            continue;

        jf_listhead_forEachSafe(&head[i], pos, temp)
        {
            ptsi = jf_listhead_getEntry(pos, transd_stock_info_t, tsi_jlList);
            if (strlen(ptsi->tsi_strStatArbi) == 0)
            {
                _freeTransdStockInfo(&ptsi);
            }
        }
    }
}
#endif

static u32 _initStrStockList(
    jf_listhead_t * head, olint_t num, olchar_t ** ppstr, olint_t * pnCount, olint_t * pnMax)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, max = 0;
    jf_listhead_t * pos;
    transd_stock_info_t * ptsi;
    olchar_t * pstr;

    for (i = 0; (i < num) && (u32Ret == JF_ERR_NO_ERROR); i ++)
    {
        jf_listhead_forEach(&head[i], pos)
            pnCount[i] ++;
        jf_logger_logInfoMsg("init str stock list, total %d stocks", pnCount[i]);

        if (pnCount[i] > max)
            max = pnCount[i];

        u32Ret = jf_mem_calloc((void **)&pstr, pnCount[i] * 9 + 1);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ppstr[i] = pstr;
            jf_listhead_forEach(&head[i], pos)
            {
                ptsi = jf_listhead_getEntry(pos, transd_stock_info_t, tsi_jlList);

                ol_strcat(pstr, ptsi->tsi_psiStock->si_strCode);
                ol_strcat(pstr, ",");
            }
            pstr[strlen(pstr)] = '\0';

            jf_logger_logInfoMsg("init str stock list, %s", pstr);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
#define MAX_STOCKS_PER_REQ  20
        *pnMax = MAX_STOCKS_PER_REQ;
//        *pnMax = max;
        jf_logger_logInfoMsg("init str stock list, max %d stocks", *pnMax);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */
u32 createDatransd(datransd_t ** ppDatransd, datransd_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_datransd_t * pid;
    olchar_t strExecutablePath[JF_LIMIT_MAX_PATH_LEN];
    struct hostent * servp;
    jf_webclient_create_param_t jwcp;

    jf_logger_logInfoMsg("create datransd");

    u32Ret = jf_mem_calloc((void **)&pid, sizeof(internal_datransd_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*change the working directory*/
        jf_file_getDirectoryName(strExecutablePath, JF_LIMIT_MAX_PATH_LEN, pdp->dp_pstrCmdLine);
        if (strlen(strExecutablePath) > 0)
            u32Ret = jf_process_setCurrentWorkingDirectory(strExecutablePath);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_getHostByName(ls_pstrQuotationServer, &servp);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ipaddr_setIpV4Addr(&pid->id_jiServerAddr, *(long *)(servp->h_addr));

        u32Ret = _initStockIndu(
            ls_jlStockSector, MAX_STOCK_SECTOR, &ls_nStockSector,
            STOCK_STAT_ARBI_LIST_FILE_NAME);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mutex_init(&ls_smStockInSectorLock);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _dumpStockIndu(ls_jlStockSector, ls_nStockSector);

        u32Ret = _initStrStockList(
            ls_jlStockSector, ls_nStockSector, ls_pstrStockInSector,
            ls_nStockInSector, &ls_nMaxStockPerReq);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _newStockQuo(
            ls_jlStockSector, ls_nStockSector, ls_nStockInSector, ls_nMaxStockPerReq);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _startWorkerThread();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_createChain(&pid->id_pjncChain);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_createUtimer(pid->id_pjncChain, &pid->id_pjnuUtimer, DATRANSD_NAME);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(&jwcp, 0, sizeof(jwcp));
        jwcp.jwcp_u32PoolSize = 5;
        jwcp.jwcp_sBuffer = ALIGN(ls_sRawQuoDataMalloc, 1024); //4096;
        jf_logger_logInfoMsg(
            "create datransd, buffer size for webclient %d", jwcp.jwcp_sBuffer);

        u32Ret = jf_webclient_create(pid->id_pjncChain, &pid->id_pjwWebclient, &jwcp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_addUtimerItem(
            pid->id_pjnuUtimer, pid, DA_GET_QUO_INTERVAL,
            _daGetQuotation, NULL);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppDatransd = pid;
    else if (pid != NULL)
        destroyDatransd((datransd_t **)&pid);

    return u32Ret;
}

u32 destroyDatransd(datransd_t ** ppDatransd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_datransd_t * pid;
    olint_t i;

    assert((ppDatransd != NULL) && (*ppDatransd != NULL));

    pid = (internal_datransd_t *)*ppDatransd;

    if (pid->id_pjwWebclient != NULL)
        jf_webclient_destroy((void **)&pid->id_pjwWebclient);

    if (pid->id_pjncChain != NULL)
        jf_network_destroyChain(&pid->id_pjncChain);

    jf_mem_free(ppDatransd);

    for (i = 0; i < MAX_RAW_QUO; i ++)
    {
        if (ls_sqrRawQuo[i].sqr_pstrData != NULL)
            jf_mem_free((void **)&ls_sqrRawQuo[i].sqr_pstrData);
    }

    for (i = 0; i < ls_nStockSector; i ++)
    {
        if (ls_pstrStockInSector[i] != NULL)
            jf_mem_free((void **)&ls_pstrStockInSector[i]);
    }

    if (ls_pdbStockArray1 != NULL)
        jf_mem_free((void **)&ls_pdbStockArray1);
    if (ls_pdbStockArray2 != NULL)
        jf_mem_free((void **)&ls_pdbStockArray2);

    _destroyTransdStockInfoList(ls_jlStockSector, ls_nStockSector);

    jf_sem_fini(&ls_ssRawQuoSem);
    jf_mutex_fini(&ls_smRawQuoLock);
    jf_mutex_fini(&ls_smStockInSectorLock);

    return u32Ret;
}

u32 startDatransd(datransd_t * pDatransd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_datransd_t * pid;

    assert(pDatransd != NULL);

    pid = (internal_datransd_t *)pDatransd;

    u32Ret = jf_network_startChain(pid->id_pjncChain);

    return u32Ret;
}

u32 stopDatransd(datransd_t * pDatransd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_datransd_t * pid;

    assert(pDatransd != NULL);

    pid = (internal_datransd_t *)pDatransd;

    jf_network_stopChain(pid->id_pjncChain);

    ls_bToTerminateWorkerThread = TRUE;

    return u32Ret;
}

u32 setDefaultDatransdParam(datransd_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    memset(pdp, 0, sizeof(datransd_param_t));


    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


