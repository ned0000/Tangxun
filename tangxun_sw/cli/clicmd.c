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

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_clieng.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"

#include "clicmd.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */


static u32 _setDefaultParamExit(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_exit_param_t * pcep = (cli_exit_param_t *)pParam;

    ol_memset(pcep, 0, sizeof(cli_exit_param_t));

    return u32Ret;
}

static u32 _exitHelp(da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputLine("exit: exit Jiufeng CLI");

    return u32Ret;
}

static u32 _parseExit(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_exit_param_t * pcep = (cli_exit_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "h?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case '?':
        case 'h':
            pcep->cep_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

static u32 _processExit(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_master_t * pdm = (da_master_t *)pMaster;
    cli_exit_param_t * pcep = (cli_exit_param_t *)pParam;

    if (pcep->cep_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _exitHelp(pdm);
    else
    {
        jf_clieng_outputLine("Exit CLI\n");
        u32Ret = jf_clieng_stop();
    }

    return u32Ret;
}

static u32 _setDefaultParamHelp(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_help_param_t * pchp = (cli_help_param_t *)pParam;

    ol_memset(pchp, 0, sizeof(cli_help_param_t));

    return u32Ret;
}

static u32 _parseHelp(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_help_param_t * pchp = (cli_help_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "h?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case '?':
        case 'h':
            pchp->chp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

static u32 _processHelp(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    cli_help_param_t * pchp = (cli_help_param_t *)pParam;

    if (pchp->chp_u8Action == CLI_ACTION_SHOW_HELP)
    {
        jf_clieng_outputLine("show all available commands");
    }
    else
    {
        jf_clieng_outputLine("analysis: analysis data.");
        jf_clieng_outputLine("backtest: backtest model.");
        jf_clieng_outputLine("download: download data.");
        jf_clieng_outputLine("env: envirionment variable.");
        jf_clieng_outputLine("exit: exit CLI.");
        jf_clieng_outputLine("find: find stocks.");
        jf_clieng_outputLine("fix: fix data.");
        jf_clieng_outputLine("help: list command.");
        jf_clieng_outputLine("indi: technical indicator.");
        jf_clieng_outputLine("model: model information.");
        jf_clieng_outputLine("rule: basic rules.");
        jf_clieng_outputLine("stat: statistic information of data.");
        jf_clieng_outputLine("trade: stock trading.");
        jf_clieng_outputLine("");
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 addDaCmd(da_master_t * pdm, da_cli_param_t * pdcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_newCmd(
        "exit", _setDefaultParamExit,
        _parseExit, _processExit, pdcp, NULL);

    jf_clieng_newCmd("fix", setDefaultParamFix,
        parseFix, processFix, pdcp, NULL);

    jf_clieng_newCmd("help", _setDefaultParamHelp,
        _parseHelp, _processHelp, pdcp, NULL);

    jf_clieng_newCmd("analysis", setDefaultParamAnalysis,
        parseAnalysis, processAnalysis, pdcp, NULL);

    jf_clieng_newCmd("model", setDefaultParamModel,
        parseModel, processModel, pdcp, NULL);

    jf_clieng_newCmd("stat", setDefaultParamStat,
        parseStat, processStat, pdcp, NULL);

    jf_clieng_newCmd("env", setDefaultParamEnv,
        parseEnv, processEnv, pdcp, NULL);

    jf_clieng_newCmd("download", setDefaultParamDownload,
        parseDownload, processDownload, pdcp, NULL);

    jf_clieng_newCmd("find", setDefaultParamFind,
        parseFind, processFind, pdcp, NULL);

    jf_clieng_newCmd("stock", setDefaultParamStock,
        parseStock, processStock, pdcp, NULL);

    jf_clieng_newCmd("parse", setDefaultParamParse,
        parseParse, processParse, pdcp, NULL);

    jf_clieng_newCmd("misc", setDefaultParamMisc,
        parseMisc, processMisc, pdcp, NULL);

    jf_clieng_newCmd("indi", setDefaultParamIndi,
        parseIndi, processIndi, pdcp, NULL);

    jf_clieng_newCmd("rule", setDefaultParamRule,
        parseRule, processRule, pdcp, NULL);

    jf_clieng_newCmd("trade", setDefaultParamTrade,
        parseTrade, processTrade, pdcp, NULL);

    jf_clieng_newCmd("backtest", setDefaultParamBacktest,
        parseBacktest, processBacktest, pdcp, NULL);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


