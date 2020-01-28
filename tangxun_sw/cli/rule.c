/**
 *  @file rule.c
 *
 *  @brief The find command implementation.
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
#include "jf_file.h"
#include "jf_time.h"
#include "jf_string.h"
#include "jf_jiukun.h"

#include "clicmd.h"
#include "tx_daysummary.h"
#include "tx_rule.h"
#include "tx_env.h"

/* --- private data/data structure section ------------------------------------------------------ */

static jf_clieng_caption_t ls_ccTxRuleVerbose[] =
{
    {"Id", JF_CLIENG_CAP_FULL_LINE},
    {"Name", JF_CLIENG_CAP_FULL_LINE},
};

static jf_clieng_caption_t ls_ccTxRuleBrief[] =
{
    {"Id", 4},
    {"Name", 50},
};

/* --- private routine section ------------------------------------------------------------------ */

static u32 _ruleHelp(tx_cli_master_t * ptcm)
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

static void _printOneTxRuleBrief(tx_rule_t * info)
{
    jf_clieng_caption_t * pcc = &ls_ccTxRuleBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Id */
    ol_sprintf(strField, "%u", info->tr_u16Id);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Name */
    jf_clieng_appendBriefColumn(pcc, strInfo, info->tr_pstrName);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printTxRuleBrief(tx_rule_t * pRule, u32 num)
{
    u32 index = 0;
    tx_rule_t * pStart = pRule; 

    jf_clieng_printDivider();

    jf_clieng_printHeader(
        ls_ccTxRuleBrief, sizeof(ls_ccTxRuleBrief) / sizeof(jf_clieng_caption_t));

    for (index = 0; index < num; index ++)
    {
        _printOneTxRuleBrief(pStart);
        pStart ++;
    }

    jf_clieng_outputLine("");
    jf_clieng_outputLine("Total %u rules\n", num);
}

static void _printOneTxRuleVerbose(tx_rule_t * rule)
{
    jf_clieng_caption_t * pcc = &ls_ccTxRuleVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN]; //, strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Id*/
    ol_sprintf(strLeft, "%u", rule->tr_u16Id);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Name*/
    jf_clieng_printOneFullLine(pcc, rule->tr_pstrName);
    pcc += 1;
}

static void _printTxRuleVerbose(tx_rule_t * pRule, u32 num)
{
    u32 index = 0;
    tx_rule_t * pStart = pRule; 

    for (index = 0; index < num; index ++)
    {
        _printOneTxRuleVerbose(pStart);
        pStart ++;
    }

    jf_clieng_outputLine("");
    jf_clieng_outputLine("Total %u rules\n", num);
}

static u32 _listAllRules(cli_rule_param_t * pcrp, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 total = tx_rule_getNumOfRules();
    tx_rule_t * rule = NULL;

    u32Ret = tx_rule_getAllRules(&rule);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pcrp->crp_bVerbose)
            _printTxRuleVerbose(rule, total);
        else
            _printTxRuleBrief(rule, total);
    }

    return u32Ret;
}

static u32 _listOneRule(cli_rule_param_t * pcrp, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_rule_t * pRule;

    u32Ret = tx_rule_getRuleByName(pcrp->crp_pstrRuleName, &pRule);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pcrp->crp_bVerbose)
            _printTxRuleVerbose(pRule, 1);
        else
            _printTxRuleBrief(pRule, 1);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processRule(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_rule_param_t * pcrp = (cli_rule_param_t *)pParam;
    tx_cli_master_t * ptcm = (tx_cli_master_t *)pMaster;

    if (pcrp->crp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _ruleHelp(ptcm);
    else if (pcrp->crp_u8Action == CLI_ACTION_RULE_LIST_ALL)
        u32Ret = _listAllRules(pcrp, ptcm);
    else if (pcrp->crp_u8Action == CLI_ACTION_RULE_LIST)
        u32Ret = _listOneRule(pcrp, ptcm);
    else if (tx_env_isNullVarDataPath())
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

    while (((nOpt = getopt(argc, argv, "l:hv?")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
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


