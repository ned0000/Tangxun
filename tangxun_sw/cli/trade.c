/**
 *  @file trade.c
 *
 *  @brief The trade command implementation.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_process.h"
#include "jf_time.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_jiukun.h"

#include "tx_stock.h"
#include "tx_env.h"
#include "tx_model.h"
#include "tx_persistency.h"

#include "clicmd.h"

/* --- private data/data structure section ------------------------------------------------------ */
static jf_clieng_caption_t ls_ccTradingStockBrief[] =
{
    {"Id", 4},
    {"TradeDate", 12},
    {"Stock", 10},
    {"Model", 8},
    {"Op", 8},
    {"Position", 9},
    {"Volume", 12},
    {"Price", 12},
};

static jf_clieng_caption_t ls_ccTradingRecordBrief[] =
{
    {"Id", 4},
    {"Stock", 9}, 
    {"Model", 8},
    {"TradeDate", 11},
    {"Op", 6},
    {"Remark", 12},
    {"Position", 9},
    {"Volume", 8},
    {"Price", 8},
};

static jf_clieng_caption_t ls_ccTradingRecordVerbose[] =
{
    {"Id", JF_CLIENG_CAP_FULL_LINE},
    {"Stock", JF_CLIENG_CAP_HALF_LINE}, {"Model", JF_CLIENG_CAP_HALF_LINE},
    {"ModelParam", JF_CLIENG_CAP_FULL_LINE},
    {"AddDate", JF_CLIENG_CAP_FULL_LINE},
    {"Op", JF_CLIENG_CAP_HALF_LINE}, {"OpRemark", JF_CLIENG_CAP_HALF_LINE},
    {"TradeDate", JF_CLIENG_CAP_HALF_LINE}, {"Position", JF_CLIENG_CAP_HALF_LINE},
    {"Volume", JF_CLIENG_CAP_HALF_LINE}, {"Price", JF_CLIENG_CAP_HALF_LINE},
};

/* --- private routine section ------------------------------------------------------------------ */
static u32 _tradeHelp(tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Stock trading.\n\
trade [-l] [-r] [-h] [-v]");
    jf_clieng_outputRawLine2("\
  -l: list stock in trading.\n\
  -r: list stock trading record.");
    jf_clieng_outputRawLine2("\
  -v: verbose.\n\
  -h: print help information.\n\
  By default, start stock trading.");

    return u32Ret;
}

static u32 _throwStockIntoModel(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_model_t * model = NULL;
    tx_trade_pool_stock_t ttps;
    tx_ds_t * end;

    end = buffer + num - 1;
    jf_logger_logInfoMsg("throw stock into model, total: %d, last day: %s", num, end->td_strDate);

    model = tx_model_getFirstModel();
    while (model != NULL)
    {
        u32Ret = model->tm_fnCanBeTraded(model, stockinfo, &ttps, buffer, num);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            bzero(&ttps, sizeof(tx_trade_pool_stock_t));
            ol_strcpy(ttps.ttps_strStock, stockinfo->tsi_strCode);
            ol_strcpy(ttps.ttps_strModel, model->tm_strName);
            u32Ret = tx_persistency_getPoolStock(&ttps);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                jf_logger_logInfoMsg("%s is already in pool", stockinfo->tsi_strCode);
                continue;
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            bzero(&ttps, sizeof(tx_trade_pool_stock_t));
            ol_strcpy(ttps.ttps_strStock, stockinfo->tsi_strCode);
            ol_strcpy(ttps.ttps_strAddDate, end->td_strDate);
            ol_strcpy(ttps.ttps_strModel, model->tm_strName);

            u32Ret = tx_persistency_insertPoolStock(&ttps);
        }

        model = tx_model_getNextModel(model);
    }

    return JF_ERR_NO_ERROR;
}

static u32 _tradeStocks(
    cli_trade_param_t * pctp, olchar_t * pstrDataDir, tx_stock_info_t * stockinfo,
    tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = num;

    u32Ret = tx_ds_readDsWithFRoR(pstrDataDir, buffer, &total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _throwStockIntoModel(stockinfo, buffer, total);
    }
    else
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static u32 _startTradeInStockPool(cli_trade_param_t * pctp, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    olint_t total = 400; //MAX_NUM_OF_DAY_SUMMARY;
    tx_ds_t * buffer = NULL;
    tx_stock_info_t * stockinfo;

    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);

    stockinfo = tx_stock_getFirstStockInfo();
    while ((stockinfo != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            memset(strFullname, 0, JF_LIMIT_MAX_PATH_LEN);
            ol_snprintf(
                strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
                tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, stockinfo->tsi_strCode);

            u32Ret = _tradeStocks(pctp, strFullname, stockinfo, buffer, total);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            stockinfo = tx_stock_getNextStockInfo(stockinfo);
        }
    }

    jf_jiukun_freeMemory((void **)&buffer);

    return u32Ret;
}

static void _printTradingStockBrief(olint_t id, tx_trade_pool_stock_t * info)
{
    jf_clieng_caption_t * pcc = &ls_ccTradingStockBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /*id*/
    ol_sprintf(strField, "%d", id);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*TradeDate*/
    ol_sprintf(strField, "%s", info->ttps_strTradeDate);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Stock*/
    ol_sprintf(strField, "%s", info->ttps_strStock);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Model*/
    ol_sprintf(strField, "%s", info->ttps_strModel);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Op*/
    ol_sprintf(strField, "%s", info->ttps_strOp);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Position*/
    ol_sprintf(strField, "%s", info->ttps_strPosition);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Volume*/
    ol_sprintf(strField, "%d", info->ttps_nVolume);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Price*/
    ol_sprintf(strField, "%.2f", info->ttps_dbPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static u32 _listTradingStock(cli_trade_param_t * pctp, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index = 0, count = 0;
    tx_trade_pool_stock_t * pttps = NULL, * pStock = NULL;

    count = tx_persistency_getNumOfPoolStock();
    jf_clieng_outputLine("Total %u trading stocks.", count);

    if (count == 0)
        return u32Ret;

    jf_jiukun_allocMemory(
        (void **)&pttps, count * sizeof(tx_trade_pool_stock_t));

    u32Ret = tx_persistency_getAllPoolStock(pttps, &count);
    if ((u32Ret == JF_ERR_NO_ERROR) && (count > 0))
    {
        if (pctp->ctp_bVerbose)
        {

        }
        else
        {
            jf_clieng_printDivider();
            jf_clieng_printHeader(
                ls_ccTradingStockBrief,
                sizeof(ls_ccTradingStockBrief) / sizeof(jf_clieng_caption_t));
            pStock = pttps;
            for (index = 0; index < count; index ++)
            {
                _printTradingStockBrief(index + 1, pStock);
                pStock ++;
            }
            jf_clieng_outputLine("");
        }
    }

    jf_jiukun_freeMemory((void **)&pttps);

    return u32Ret;
}

static void _printTradingRecordBrief(olint_t id, tx_trade_trading_record_t * info)
{
    jf_clieng_caption_t * pcc = &ls_ccTradingRecordBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /*id*/
    ol_sprintf(strField, "%d", id);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Stock*/
    ol_sprintf(strField, "%s", info->tttr_strStock);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Model*/
    ol_sprintf(strField, "%s", info->tttr_strModel);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*TradeDate*/
    ol_sprintf(strField, "%s", info->tttr_strTradeDate);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Op*/
    ol_sprintf(strField, "%s", info->tttr_strOp);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Remark*/
    ol_sprintf(strField, "%s", info->tttr_strOpRemark);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Position*/
    ol_sprintf(strField, "%s", info->tttr_strPosition);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Volume*/
    ol_sprintf(strField, "%d", info->tttr_nVolume);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Price*/
    ol_sprintf(strField, "%.2f", info->tttr_dbPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printTradingRecordVerbose(olint_t id, tx_trade_trading_record_t * info)
{
    jf_clieng_caption_t * pcc = &ls_ccTradingRecordVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN - 1] = '\0';
    strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN - 1] = '\0';

    jf_clieng_printDivider();

    /*Id*/
    ol_sprintf(strLeft, "%d", id);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Stock*/
    ol_sprintf(strLeft, "%s", info->tttr_strStock);
    ol_sprintf(strRight, "%s", info->tttr_strModel);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*ModelParam*/
    ol_snprintf(strLeft, JF_CLIENG_MAX_OUTPUT_LINE_LEN - 1, "%s", info->tttr_strModelParam);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*AddDate*/
    ol_sprintf(strLeft, "%s", info->tttr_strAddDate);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Op*/
    ol_sprintf(strLeft, "%s", info->tttr_strOp);
    ol_sprintf(strRight, "%s", info->tttr_strOpRemark);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*TradeDate*/
    ol_sprintf(strLeft, "%s", info->tttr_strTradeDate);
    ol_sprintf(strRight, "%s", info->tttr_strPosition);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*Volume*/
    ol_sprintf(strLeft, "%d", info->tttr_nVolume);
    ol_sprintf(strRight, "%.2f", info->tttr_dbPrice);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    jf_clieng_outputLine("");
}

static u32 _listTradingRecord(cli_trade_param_t * pctp, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index = 0, count = 0;
    tx_trade_trading_record_t * ptrr = NULL, * pRecord = NULL;

    count = tx_persistency_getNumOfTradingRecord();
    jf_clieng_outputLine("Total %u trading records.", count);

    if (count == 0)
        return u32Ret;

    jf_jiukun_allocMemory(
        (void **)&ptrr, count * sizeof(tx_trade_trading_record_t));

    u32Ret = tx_persistency_getAllTradingRecord(ptrr, &count);
    if ((u32Ret == JF_ERR_NO_ERROR) && (count > 0))
    {
        if (pctp->ctp_bVerbose)
        {
            pRecord = ptrr;
            for (index = 0; index < count; index ++)
            {
                _printTradingRecordVerbose(index + 1, pRecord);
                pRecord ++;
            }
            jf_clieng_outputLine("");
        }
        else
        {
            jf_clieng_printDivider();
            jf_clieng_printHeader(
                ls_ccTradingRecordBrief,
                sizeof(ls_ccTradingRecordBrief) / sizeof(jf_clieng_caption_t));
            pRecord = ptrr;
            for (index = 0; index < count; index ++)
            {
                _printTradingRecordBrief(index + 1, pRecord);
                pRecord ++;
            }
            jf_clieng_outputLine("");
        }
    }

    jf_jiukun_freeMemory((void **)&ptrr);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processTrade(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_trade_param_t * pctp = (cli_trade_param_t *)pParam;
    tx_cli_master_t * ptcm = (tx_cli_master_t *)pMaster;

    if (pctp->ctp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _tradeHelp(ptcm);
    else if (pctp->ctp_u8Action == CLI_ACTION_TRADE_LIST)
        u32Ret = _listTradingStock(pctp, ptcm);
    else if (pctp->ctp_u8Action == CLI_ACTION_TRADE_LIST_RECORD)
        u32Ret = _listTradingRecord(pctp, ptcm);
    else if (tx_env_isNullVarDataPath())
    {
        jf_clieng_outputLine("Data path is not set.");
        u32Ret = JF_ERR_NOT_READY;
    }
    else if (pctp->ctp_u8Action == CLI_ACTION_TRADE_STOCK)
        u32Ret = _startTradeInStockPool(pctp, ptcm);
    else
        u32Ret = JF_ERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamTrade(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_trade_param_t * pctp = (cli_trade_param_t *)pParam;

    memset(pctp, 0, sizeof(cli_trade_param_t));

    pctp->ctp_u8Action = CLI_ACTION_TRADE_STOCK;

    return u32Ret;
}

u32 parseTrade(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_trade_param_t * pctp = (cli_trade_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "lrhv?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'l':
            pctp->ctp_u8Action = CLI_ACTION_TRADE_LIST;
            break;
        case 'r':
            pctp->ctp_u8Action = CLI_ACTION_TRADE_LIST_RECORD;
            break;
        case 'v':
            pctp->ctp_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pctp->ctp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


