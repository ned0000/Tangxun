/**
 *  @file find.c
 *
 *  @brief The find command
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "process.h"
#include "clicmd.h"
#include "stringparse.h"
#include "files.h"
#include "parsedata.h"
#include "indicator.h"
#include "datastat.h"
#include "jiukun.h"
#include "stocklist.h"
#include "xtime.h"
#include "statarbitrage.h"
#include "envvar.h"
#include "damodel.h"
#include "trade_persistency.h"

/* --- private data/data structure section --------------------------------- */
static clieng_caption_t ls_ccStockPoolBrief[] =
{
    {"Id", 4},
    {"Stock", 10}, 
    {"Model", 8},
    {"ModelParam", 44},
    {"AddDate", 12},
};

/* --- private routine section---------------------------------------------- */
static u32 _findHelp(da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputRawLine2("\
Find stock.\n\
find [-a] [-p date~date] [-l] [-m] [-c] [-h] [-v]");
    cliengOutputRawLine2("\
  -a: all stocks including suspended stocks.\n\
  -p: find in time period, the format is \"yyyy-mm-dd~yyyy-mm-dd\".\n\
  -l: list stocks in pool, the stocks are sorted by date.\n\
  -m: list stocks in pool, the stocks are sorted by model type and date.");
    cliengOutputRawLine2("\
  -c: clean stocks in pool.\n\
  -v: verbose.\n\
  -h: print help information.\n\
  By default, those suspended stocks are ignored.");

    return u32Ret;
}

static boolean_t _isSuspendedStock(
    stock_info_t * stockinfo, da_day_summary_t * buffer, olint_t num)
{
    boolean_t bRet = TRUE;
    da_day_summary_t * last = buffer + num - 1;
    olint_t dw, cdays;
    olint_t lyear, lmonth, lday, ldays;

    cdays = convertTodayToDaysFrom1970();
    dw = getDayOfWeekForToday();
    if (dw == 6) /*Saturday*/
        cdays --;
    else if (dw == 0) /*Sunday*/
        cdays -= 2;

    getDate2FromString(last->dds_strDate, &lyear, &lmonth, &lday);
    ldays = convertDateToDaysFrom1970(lyear, lmonth, lday);

    if (ldays == cdays)
    {
        bRet = FALSE;
        logInfoMsg("%s is not suspended", stockinfo->si_strCode);
    }
    else
    {
        logInfoMsg("%s is suspended", stockinfo->si_strCode);
    }

    return bRet;
}

static u32 _throwStockIntoModel(
    stock_info_t * stockinfo, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t index, max = DA_MODEL_MAX_ID;
    da_model_t * model = NULL;
    trade_pool_stock_t tps;
    da_day_summary_t * end;

    end = buffer + num - 1;
    logInfoMsg("throw stock into model, total: %d, last day: %s", num, end->dds_strDate);

    for (index = 0; index < max; index ++)
    {
        u32Ret = getDaModel((da_model_id_t)index, &model);
        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = model->dm_fnCanBeTraded(model, stockinfo, &tps, buffer, num);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            bzero(&tps, sizeof(trade_pool_stock_t));
            ol_strcpy(tps.tps_strStock, stockinfo->si_strCode);
            ol_strcpy(tps.tps_strModel, model->dm_strName);
            u32Ret = getPoolStockInTradePersistency(&tps);
            if (u32Ret == OLERR_NO_ERROR)
            {
                logInfoMsg("%s is already in pool", stockinfo->si_strCode);
                continue;
            }
            else
            {
                u32Ret = OLERR_NO_ERROR;
            }
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            bzero(&tps, sizeof(trade_pool_stock_t));
            ol_strcpy(tps.tps_strStock, stockinfo->si_strCode);
            ol_strcpy(tps.tps_strAddDate, end->dds_strDate);
            ol_strcpy(tps.tps_strModel, model->dm_strName);

            u32Ret = insertPoolStockIntoTradePersistency(&tps);
        }
    }

    return OLERR_NO_ERROR; //u32Ret;
}

static u32 _findStocks(
    cli_find_param_t * pcfp, olchar_t * pstrDataDir, stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t total = num;

    u32Ret = readTradeDaySummaryWithFRoR(pstrDataDir, buffer, &total);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (! pcfp->cfp_bAllStocks &&
            _isSuspendedStock(stockinfo, buffer, total))
            return u32Ret;

        u32Ret = _throwStockIntoModel(stockinfo, buffer, total);
    }
    else
        u32Ret = OLERR_NO_ERROR;

    return u32Ret;
}

static u32 _cleanTradeStockFromPool(void)
{
    u32 u32Ret = OLERR_NO_ERROR;


    return u32Ret;
}

static u32 _startFindForStockList(cli_find_param_t * pcfp, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strFullname[MAX_PATH_LEN];
    olint_t total = 400; //MAX_NUM_OF_DAY_SUMMARY;
    da_day_summary_t * buffer = NULL;
    stock_info_t * stockinfo;

    _cleanTradeStockFromPool();

    allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);

    stockinfo = getFirstStockInfo();
    while ((stockinfo != NULL) && (u32Ret == OLERR_NO_ERROR))
    {
        if (u32Ret == OLERR_NO_ERROR)
        {
            memset(strFullname, 0, MAX_PATH_LEN);
            ol_snprintf(
                strFullname, MAX_PATH_LEN - 1, "%s%c%s",
                getEnvVar(ENV_VAR_DATA_PATH),
                PATH_SEPARATOR, stockinfo->si_strCode);

            u32Ret = _findStocks(
                pcfp, strFullname, stockinfo, buffer, total);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            stockinfo = getNextStockInfo(stockinfo);
        }
    }

    freeMemory((void **)&buffer);

    return u32Ret;
}

static u32 _findStockInPeriod(
    cli_find_param_t * pcfp, olchar_t * pstrDataDir, stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t * pdate;
    olchar_t strDate[32];
    olint_t syear, smonth, sday, eyear, emonth, eday;
    olint_t sdays, edays;
    olint_t dw, total;

    pdate = pcfp->cfp_pstrPeriod + 11;
    pcfp->cfp_pstrPeriod[10] = '\0';

    getDate2FromString(pcfp->cfp_pstrPeriod, &syear, &smonth, &sday);
    getDate2FromString(pdate, &eyear, &emonth, &eday);

    sdays = convertDateToDaysFrom1970(syear, smonth, sday);
    edays = convertDateToDaysFrom1970(eyear, emonth, eday);

    while (sdays <= edays)
    {
        convertDaysFrom1970ToDate(sdays, &syear, &smonth, &sday);
        dw = getDayOfWeekFromDate(syear, smonth, sday);
        if ((dw == 0) || (dw == 6))
        {
            /*Sunday and Saturday are not trading days */
            sdays ++;
            continue;
        }
        getStringDate2(strDate, syear, smonth, sday);
//        cliengOutputLine("%s", pcfp->cfp_pstrPeriod);
        total = daySummaryEndDataCount(buffer, num, strDate);
        if (total == 0)
        {
            sdays ++;
            continue;
        }

        u32Ret = _throwStockIntoModel(stockinfo, buffer, total);

        sdays ++;
    }

    return u32Ret;
}

static u32 _startFindStockInPeriod(
    cli_find_param_t * pcfp, olchar_t * pstrDataDir, stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t total = num;

    u32Ret = readTradeDaySummaryWithFRoR(pstrDataDir, buffer, &total);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _findStockInPeriod(pcfp, pstrDataDir, stockinfo, buffer, total);
    }
    else
        u32Ret = OLERR_NO_ERROR;

    return u32Ret;
}

static u32 _startFindForStockListInPeriod(cli_find_param_t * pcfp, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strFullname[MAX_PATH_LEN];
    olint_t total = 800; //MAX_NUM_OF_DAY_SUMMARY;
    da_day_summary_t * buffer = NULL;
    stock_info_t * stockinfo;

    if (strlen(pcfp->cfp_pstrPeriod) != 21)
        return OLERR_INVALID_PARAM;

    allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);

    stockinfo = getFirstStockInfo();
    while ((stockinfo != NULL) && (u32Ret == OLERR_NO_ERROR))
    {
#if 0
        if (strcmp(stockinfo->si_strCode, "sh600073") != 0)
        {
            stockinfo = getNextStockInfo(stockinfo);
            continue;
        }
#endif
        if (u32Ret == OLERR_NO_ERROR)
        {
            memset(strFullname, 0, MAX_PATH_LEN);
            ol_snprintf(
                strFullname, MAX_PATH_LEN - 1, "%s%c%s",
                getEnvVar(ENV_VAR_DATA_PATH),
                PATH_SEPARATOR, stockinfo->si_strCode);

            u32Ret = _startFindStockInPeriod(
                pcfp, strFullname, stockinfo, buffer, total);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            stockinfo = getNextStockInfo(stockinfo);
        }
    }

    freeMemory((void **)&buffer);

    return u32Ret;
}

static olint_t _compareTpPoolStockByModel(const void * a, const void * b)
{
    trade_pool_stock_t * ra, * rb;
    olint_t ret;

    ra = (trade_pool_stock_t *)a;
    rb = (trade_pool_stock_t *)b;

    ret = strcmp(ra->tps_strModel, rb->tps_strModel);
    if (ret == 0)
        ret = strcmp(ra->tps_strAddDate, rb->tps_strAddDate);

    return ret;
}

static void _printStockPoolBrief(olint_t id, trade_pool_stock_t * info)
{
    clieng_caption_t * pcc = &ls_ccStockPoolBrief[0];
    olchar_t strInfo[MAX_OUTPUT_LINE_LEN], strField[MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /*id*/
    ol_sprintf(strField, "%d", id);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Stock*/
    ol_sprintf(strField, "%s", info->tps_strStock);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Model*/
    ol_sprintf(strField, "%s", info->tps_strModel);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*ModelParam*/
    ol_sprintf(strField, "%s", info->tps_strModelParam);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*AddDate*/
    ol_sprintf(strField, "%s", info->tps_strAddDate);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    cliengOutputLine(strInfo);
}

static u32 _listStocksInPool(cli_find_param_t * pcfp, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t index = 0, count = 0;
    trade_pool_stock_t * stockinpool = NULL, * pStock = NULL;

    count = getNumOfPoolStockInTradePersistency();
    cliengOutputLine("Total %u stocks in pool.", count);

    if (count == 0)
        return u32Ret;

    allocMemory(
        (void **)&stockinpool, count * sizeof(trade_pool_stock_t), 0);

    u32Ret = getAllPoolStockInTradePersistency(stockinpool, &count);
    if ((u32Ret == OLERR_NO_ERROR) && (count > 0))
    {
        if (pcfp->cfp_u8Action == CLI_ACTION_FIND_LIST_POOL_BY_MODEL)
            qsort(stockinpool, count, sizeof(trade_pool_stock_t), _compareTpPoolStockByModel);

        if (pcfp->cfp_bVerbose)
        {

        }
        else
        {
            cliengPrintDivider();
            cliengPrintHeader(
                ls_ccStockPoolBrief,
                sizeof(ls_ccStockPoolBrief) / sizeof(clieng_caption_t));
            pStock = stockinpool;
            for (index = 0; index < count; index ++)
            {
                _printStockPoolBrief(index + 1, pStock);
                pStock ++;
            }
            cliengOutputLine("");
        }
    }

    freeMemory((void **)&stockinpool);

    return u32Ret;
}

static u32 _cleanStocksInPool(cli_find_param_t * pcfp, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;


    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 processFind(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_find_param_t * pcfp = (cli_find_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcfp->cfp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _findHelp(pdm);
    else if (pcfp->cfp_u8Action == CLI_ACTION_FIND_LIST_POOL)
        u32Ret = _listStocksInPool(pcfp, pdm);
    else if (pcfp->cfp_u8Action == CLI_ACTION_FIND_LIST_POOL_BY_MODEL)
        u32Ret = _listStocksInPool(pcfp, pdm);
    else if (pcfp->cfp_u8Action == CLI_ACTION_CLEAN_STOCK_POOL)
        u32Ret = _cleanStocksInPool(pcfp, pdm);
    else if (*getEnvVar(ENV_VAR_DATA_PATH) == '\0')
    {
        cliengOutputLine("Data path is not set.");
        u32Ret = OLERR_NOT_READY;
    }
    else if (pcfp->cfp_u8Action == CLI_ACTION_FIND_STOCK)
        u32Ret = _startFindForStockList(pcfp, pdm);
    else if (pcfp->cfp_u8Action == CLI_ACTION_FIND_STOCK_IN_PERIOD)
        u32Ret = _startFindForStockListInPeriod(pcfp, pdm);
    else
        u32Ret = OLERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamFind(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_find_param_t * pcfp = (cli_find_param_t *)pParam;

    memset(pcfp, 0, sizeof(cli_find_param_t));

    pcfp->cfp_u8Action = CLI_ACTION_FIND_STOCK;

    return u32Ret;
}

u32 parseFind(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_find_param_t * pcfp = (cli_find_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "almcp:hv?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'a':
            pcfp->cfp_bAllStocks = TRUE;
            break;
        case 'p':
            pcfp->cfp_u8Action = CLI_ACTION_FIND_STOCK_IN_PERIOD;
            pcfp->cfp_pstrPeriod = (olchar_t *)optarg;
            break;
        case 'l':
            pcfp->cfp_u8Action = CLI_ACTION_FIND_LIST_POOL;
            break;
        case 'c':
            pcfp->cfp_u8Action = CLI_ACTION_CLEAN_STOCK_POOL;
            break;
        case 'm':
            pcfp->cfp_u8Action = CLI_ACTION_FIND_LIST_POOL_BY_MODEL;
            break;
        case 'v':
            pcfp->cfp_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcfp->cfp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

