/**
 *  @file backtest.c
 *
 *  @brief Backtest command
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
#include "backtesting.h"

/* --- private data/data structure section --------------------------------- */
static clieng_caption_t ls_ccBacktestingResultVerbose[] =
{
    {"NumOfTrade", CLIENG_CAP_FULL_LINE},
    {"NumOfTradeProfit", CLIENG_CAP_HALF_LINE}, {"NumOfTradeLoss", CLIENG_CAP_HALF_LINE},
    {"MaxDrawdown", CLIENG_CAP_HALF_LINE}, {"RateOfReturn", CLIENG_CAP_HALF_LINE},
    {"InitialFund", CLIENG_CAP_HALF_LINE}, {"Fund", CLIENG_CAP_HALF_LINE},
    {"MinAsset", CLIENG_CAP_HALF_LINE}, {"MaxAsset", CLIENG_CAP_HALF_LINE},
};



/* --- private routine section---------------------------------------------- */
static u32 _backtestHelp(da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputRawLine2("\
Backtest model.\n\
backtest [-m model] [-s] [-h] [-v]");
    cliengOutputRawLine2("\
  -s: use stock-by-stock method.\n\
  -m: backtest specified model.");
    cliengOutputRawLine2("\
  -v: verbose.\n\
  -h: print help information.\n\
  By default, all models are backtested with day by day method.");

    return u32Ret;
}

static void _printBacktestingResultVerbose(backtesting_result_t * pbr)
{
    clieng_caption_t * pcc = &ls_ccBacktestingResultVerbose[0];
    olchar_t strLeft[MAX_OUTPUT_LINE_LEN], strRight[MAX_OUTPUT_LINE_LEN];

    cliengPrintDivider();

    /*NumOfTrade*/
    ol_sprintf(strLeft, "%u", pbr->br_u32NumOfTrade);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*NumOfTrade*/
    ol_sprintf(strLeft, "%u", pbr->br_u32NumOfTradeProfit);
    ol_sprintf(strRight, "%u", pbr->br_u32NumOfTradeLoss);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*MaxDrawdown*/
    ol_sprintf(strLeft, "%.2f%%", pbr->br_dbMaxDrawdown);
    ol_sprintf(strRight, "%.2f%%", pbr->br_dbRateOfReturn);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*InitialFund*/
    ol_sprintf(strLeft, "%.2f", pbr->br_dbInitialFund);
    ol_sprintf(strRight, "%.2f", pbr->br_dbFund);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*MinAsset*/
    ol_sprintf(strLeft, "%.2f", pbr->br_dbMinAsset);
    ol_sprintf(strRight, "%.2f", pbr->br_dbMaxAsset);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    cliengOutputLine("");
}

static u32 _printBacktestingResult(backtesting_result_t * pbr)
{
    u32 u32Ret = OLERR_NO_ERROR;

    _printBacktestingResultVerbose(pbr);

    return u32Ret;
}

static u32 _startBacktestAll(cli_backtest_param_t * pcbp, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strFullname[MAX_PATH_LEN];
    backtesting_param_t bp;
    backtesting_result_t br;

    ol_snprintf(
        strFullname, MAX_PATH_LEN - 1, "%s",
        getEnvVar(ENV_VAR_DATA_PATH));
    strFullname[MAX_PATH_LEN - 1] = '\0';

    bzero(&bp, sizeof(backtesting_param_t));
    bp.bp_bAllModel = TRUE;
    bp.bp_pstrStockPath = strFullname;
    bp.bp_dbInitialFund = 100000;
    bp.bp_u8Method = BACKTESTING_METHOD_DAY_BY_DAY;
    if (pcbp->cbp_bStockByStock)
        bp.bp_u8Method = BACKTESTING_METHOD_STOCK_BY_STOCK;

    bzero(&br, sizeof(backtesting_result_t));

    u32Ret = backtestingModel(&bp, &br);

//    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _printBacktestingResult(&br);
    }

    return u32Ret;
}

static u32 _startBacktestModel(cli_backtest_param_t * pcbp, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strFullname[MAX_PATH_LEN];
    backtesting_param_t bp;
    backtesting_result_t br;

    if (pcbp->cbp_pstrModel == NULL)
        return OLERR_INVALID_PARAM;

    ol_snprintf(
        strFullname, MAX_PATH_LEN - 1, "%s",
        getEnvVar(ENV_VAR_DATA_PATH));
    strFullname[MAX_PATH_LEN - 1] = '\0';

    ol_bzero(&bp, sizeof(backtesting_param_t));
    bp.bp_bAllModel = FALSE;
    bp.bp_pstrModel = pcbp->cbp_pstrModel;
    bp.bp_pstrStockPath = strFullname;
    bp.bp_dbInitialFund = 100000;
    bp.bp_u8Method = BACKTESTING_METHOD_DAY_BY_DAY;
    if (pcbp->cbp_bStockByStock)
        bp.bp_u8Method = BACKTESTING_METHOD_STOCK_BY_STOCK;

    ol_bzero(&br, sizeof(backtesting_result_t));

    u32Ret = backtestingModel(&bp, &br);

//    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _printBacktestingResult(&br);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 processBacktest(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_backtest_param_t * pcbp = (cli_backtest_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcbp->cbp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _backtestHelp(pdm);
    else if (*getEnvVar(ENV_VAR_DATA_PATH) == '\0')
    {
        cliengOutputLine("Data path is not set.");
        u32Ret = OLERR_NOT_READY;
    }
    else if (pcbp->cbp_u8Action == CLI_ACTION_BACKTEST_ALL)
        u32Ret = _startBacktestAll(pcbp, pdm);
    else if (pcbp->cbp_u8Action == CLI_ACTION_BACKTEST_MODEL)
        u32Ret = _startBacktestModel(pcbp, pdm);
    else
        u32Ret = OLERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamBacktest(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_backtest_param_t * pcbp = (cli_backtest_param_t *)pParam;

    memset(pcbp, 0, sizeof(cli_backtest_param_t));

    pcbp->cbp_u8Action = CLI_ACTION_BACKTEST_ALL;

    return u32Ret;
}

u32 parseBacktest(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_backtest_param_t * pcbp = (cli_backtest_param_t *)pParam;
//    tangxun_cli_master_t * pocm = (tangxun_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "sm:hv?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'm':
            pcbp->cbp_u8Action = CLI_ACTION_BACKTEST_MODEL;
            pcbp->cbp_pstrModel = (olchar_t *)optarg;
            break;
        case 's':
            pcbp->cbp_bStockByStock = TRUE;
            break;
        case 'v':
            pcbp->cbp_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcbp->cbp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

