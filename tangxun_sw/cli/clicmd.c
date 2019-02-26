/**
 *  @file clicmd.c
 *
 *  @brief Cli command
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "clicmd.h"
#include "stringparse.h"
#include "files.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */


static u32 _setDefaultParamExit(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_exit_param_t * pcep = (cli_exit_param_t *)pParam;

    memset(pcep, 0, sizeof(cli_exit_param_t));

    return u32Ret;
}

static u32 _exitHelp(da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputLine("exit: exit Jiufeng CLI");

    return u32Ret;
}

static u32 _parseExit(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_exit_param_t * pcep = (cli_exit_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "h?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case ':':
            u32Ret = OLERR_MISSING_PARAM;
            break;
        case '?':
        case 'h':
            pcep->cep_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

static u32 _processExit(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    da_master_t * pdm = (da_master_t *)pMaster;
    cli_exit_param_t * pcep = (cli_exit_param_t *)pParam;

    if (pcep->cep_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _exitHelp(pdm);
    else
    {
        cliengOutputLine("Exit CLI\n");
        u32Ret = stopClieng();
    }

    return u32Ret;
}

static u32 _setDefaultParamHelp(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_help_param_t * pchp = (cli_help_param_t *)pParam;

    memset(pchp, 0, sizeof(cli_help_param_t));

    return u32Ret;
}

static u32 _parseHelp(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_help_param_t * pchp = (cli_help_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "h?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case ':':
            u32Ret = OLERR_MISSING_PARAM;
            break;
        case '?':
        case 'h':
            pchp->chp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

static u32 _processHelp(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    cli_help_param_t * pchp = (cli_help_param_t *)pParam;

    if (pchp->chp_u8Action == CLI_ACTION_SHOW_HELP)
    {
        cliengOutputLine("show all available commands");
    }
    else
    {
        cliengOutputLine("analysis: analysis data.");
        cliengOutputLine("backtest: backtest model.");
        cliengOutputLine("download: download data.");
        cliengOutputLine("env: envirionment variable.");
        cliengOutputLine("exit: exit CLI.");
        cliengOutputLine("find: find stocks.");
        cliengOutputLine("fix: fix data.");
        cliengOutputLine("help: list command.");
        cliengOutputLine("indi: technical indicator.");
        cliengOutputLine("model: model information.");
        cliengOutputLine("rule: basic rules.");
        cliengOutputLine("stat: statistic information of data.");
        cliengOutputLine("trade: stock trading.");
        cliengOutputLine("");
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 addDaCmd(da_master_t * pdm, da_cli_param_t * pdcp)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengNewCmd("exit", _setDefaultParamExit, pdcp,
        _parseExit, _processExit, NULL);

    cliengNewCmd("fix", setDefaultParamFix, pdcp,
        parseFix, processFix, NULL);

    cliengNewCmd("help", _setDefaultParamHelp, pdcp,
        _parseHelp, _processHelp, NULL);

    cliengNewCmd("analysis", setDefaultParamAnalysis, pdcp,
        parseAnalysis, processAnalysis, NULL);

    cliengNewCmd("model", setDefaultParamModel, pdcp,
        parseModel, processModel, NULL);

    cliengNewCmd("stat", setDefaultParamStat, pdcp,
        parseStat, processStat, NULL);

    cliengNewCmd("env", setDefaultParamEnv, pdcp,
        parseEnv, processEnv, NULL);

    cliengNewCmd("download", setDefaultParamDownload, pdcp,
        parseDownload, processDownload, NULL);

    cliengNewCmd("find", setDefaultParamFind, pdcp,
        parseFind, processFind, NULL);

    cliengNewCmd("stock", setDefaultParamStock, pdcp,
        parseStock, processStock, NULL);

    cliengNewCmd("parse", setDefaultParamParse, pdcp,
        parseParse, processParse, NULL);

    cliengNewCmd("misc", setDefaultParamMisc, pdcp,
        parseMisc, processMisc, NULL);

    cliengNewCmd("indi", setDefaultParamIndi, pdcp,
        parseIndi, processIndi, NULL);

    cliengNewCmd("rule", setDefaultParamRule, pdcp,
        parseRule, processRule, NULL);

    cliengNewCmd("trade", setDefaultParamTrade, pdcp,
        parseTrade, processTrade, NULL);

    cliengNewCmd("backtest", setDefaultParamBacktest, pdcp,
        parseBacktest, processBacktest, NULL);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


