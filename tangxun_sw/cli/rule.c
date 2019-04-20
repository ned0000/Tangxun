/**
 *  @file rule.c
 *
 *  @brief The find command
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
#include "jf_process.h"
#include "clicmd.h"
#include "jf_string.h"
#include "jf_file.h"
#include "parsedata.h"
#include "indicator.h"
#include "darule.h"
#include "jf_time.h"
#include "envvar.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */
static jf_clieng_caption_t ls_ccDaRuleVerbose[] =
{
    {"Id", JF_CLIENG_CAP_FULL_LINE},
    {"Name", JF_CLIENG_CAP_FULL_LINE},
};

static jf_clieng_caption_t ls_ccDaRuleBrief[] =
{
    {"Id", 4},
    {"Name", 50},
};

/* --- private routine section ------------------------------------------------------------------ */

static u32 _ruleHelp(da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Manipulate basic fules.\n\
rule [-l name] [-v] [-h]");
    jf_clieng_outputRawLine2("\
  -l: list specified basic rule.");
    jf_clieng_outputRawLine2("\
  -v: verbose.\n\
  -h: show this help information.\n\
  If no option is specified, all rules are printed in brief mode.\n");

    return u32Ret;
}

static void _printOneDaRuleBrief(u32 id, da_rule_t * info)
{
    jf_clieng_caption_t * pcc = &ls_ccDaRuleBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Id */
    ol_sprintf(strField, "%u", id);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Name */
    jf_clieng_appendBriefColumn(pcc, strInfo, info->dr_pstrName);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printDaRuleBrief(da_rule_t * pRule, u32 num)
{
    u32 index = 0;
    da_rule_t * pStart = pRule; 

    jf_clieng_printDivider();

    jf_clieng_printHeader(
        ls_ccDaRuleBrief,
        sizeof(ls_ccDaRuleBrief) / sizeof(jf_clieng_caption_t));

    for (index = 0; index < num; index ++)
    {
        _printOneDaRuleBrief(index + 1, pStart);
        pStart ++;
    }

    jf_clieng_outputLine("");
    jf_clieng_outputLine("Total %u rules\n", num);
}

static void _printOneDaRuleVerbose(u32 id, da_rule_t * rule)
{
    jf_clieng_caption_t * pcc = &ls_ccDaRuleVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN]; //, strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Id*/
    ol_sprintf(strLeft, "%u", id);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Name*/
    jf_clieng_printOneFullLine(pcc, rule->dr_pstrName);
    pcc += 1;
}

static void _printDaRuleVerbose(da_rule_t * pRule, u32 num)
{
    u32 index = 0;
    da_rule_t * pStart = pRule; 

    for (index = 0; index < num; index ++)
    {
        _printOneDaRuleVerbose(index + 1, pStart);
        pStart ++;
    }

    jf_clieng_outputLine("");
    jf_clieng_outputLine("Total %u rules\n", num);
}

static u32 _listAllRules(cli_rule_param_t * pcrp, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 total = getNumOfDaRules();
    da_rule_t * rule = NULL;

    u32Ret = getAllDaRules(&rule);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pcrp->crp_bVerbose)
            _printDaRuleVerbose(rule, total);
        else
            _printDaRuleBrief(rule, total);
    }

    return u32Ret;
}

static u32 _listOneRule(cli_rule_param_t * pcrp, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_rule_t * pRule;

    u32Ret = getDaRule(pcrp->crp_pstrRuleName, &pRule);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pcrp->crp_bVerbose)
            _printDaRuleVerbose(pRule, 1);
        else
            _printDaRuleBrief(pRule, 1);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processRule(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_rule_param_t * pcrp = (cli_rule_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcrp->crp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _ruleHelp(pdm);
    else if (pcrp->crp_u8Action == CLI_ACTION_RULE_LIST_ALL)
        u32Ret = _listAllRules(pcrp, pdm);
    else if (pcrp->crp_u8Action == CLI_ACTION_RULE_LIST)
        u32Ret = _listOneRule(pcrp, pdm);
    else if (isNullEnvVarDataPath())
    {
        jf_clieng_outputLine("Data path is not set.");
        u32Ret = JF_ERR_NOT_READY;
    }
    else
        u32Ret = JF_ERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamRule(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_rule_param_t * pcrp = (cli_rule_param_t *)pParam;

    memset(pcrp, 0, sizeof(cli_rule_param_t));

    pcrp->crp_u8Action = CLI_ACTION_RULE_LIST_ALL;

    return u32Ret;
}

u32 parseRule(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_rule_param_t * pcrp = (cli_rule_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "l:hv?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'l':
            pcrp->crp_u8Action = CLI_ACTION_RULE_LIST;
            pcrp->crp_pstrRuleName = (char *)optarg; 
            break;
        case 'v':
            pcrp->crp_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcrp->crp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


