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

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "process.h"
#include "clicmd.h"
#include "stringparse.h"
#include "files.h"
#include "parsedata.h"
#include "indicator.h"
#include "darule.h"
#include "xtime.h"
#include "envvar.h"
#include "jiukun.h"

/* --- private data/data structure section --------------------------------- */
static clieng_caption_t ls_ccDaRuleVerbose[] =
{
    {"Id", CLIENG_CAP_FULL_LINE},
    {"Desc", CLIENG_CAP_FULL_LINE},
};

static clieng_caption_t ls_ccDaRuleBrief[] =
{
    {"Id", 4},
    {"Desc", 50},
};

/* --- private routine section---------------------------------------------- */

static u32 _ruleHelp(da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputRawLine2("\
Manipulate basic fules\n\
rule [-l rule-id] [-v] [-h]");
    cliengOutputRawLine2("\
  -l: list specified basic rule.");
    cliengOutputRawLine2("\
  -v: verbose.\n\
  -h: show this help information.\n\
  If no option is specified, all rules are printed in brief mode.\n");

    return u32Ret;
}

static void _printOneDaRuleBrief(da_rule_t * info)
{
    clieng_caption_t * pcc = &ls_ccDaRuleBrief[0];
    olchar_t strInfo[MAX_OUTPUT_LINE_LEN], strField[MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Id */
    ol_sprintf(strField, "%d", info->dr_driId);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* desc */
    cliengAppendBriefColumn(pcc, strInfo, info->dr_pstrDesc);
    pcc++;

    cliengOutputLine(strInfo);
}

static void _printDaRuleBrief(da_rule_t * pRule, u32 num)
{
    u32 total = 0;
    da_rule_t * pStart = pRule; 

    cliengPrintDivider();

    cliengPrintHeader(
        ls_ccDaRuleBrief,
        sizeof(ls_ccDaRuleBrief) / sizeof(clieng_caption_t));

    for (total = 0; total < num; total ++)
    {
        _printOneDaRuleBrief(pStart);
        pStart ++;
    }

    cliengOutputLine("");
    cliengOutputLine("Total %u rules\n", num);
}

static void _printOneDaRuleVerbose(da_rule_t * rule)
{
    clieng_caption_t * pcc = &ls_ccDaRuleVerbose[0];
    olchar_t strLeft[MAX_OUTPUT_LINE_LEN]; //, strRight[MAX_OUTPUT_LINE_LEN];

    cliengPrintDivider();

    /*Id*/
    ol_sprintf(strLeft, "%d", rule->dr_driId);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Desc*/
    cliengPrintOneFullLine(pcc, rule->dr_pstrDesc);
    pcc += 1;
}

static void _printDaRuleVerbose(da_rule_t * pRule, u32 num)
{
    u32 total = 0;
    da_rule_t * pStart = pRule; 

    for (total = 0; total < num; total ++)
    {
        _printOneDaRuleVerbose(pStart);
        pStart ++;
    }

    cliengOutputLine("");
    cliengOutputLine("Total %u rules\n", num);
}

static u32 _listAllRules(cli_rule_param_t * pcrp, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u32 total = getNumOfDaRules();
    da_rule_t * rule = NULL;

    u32Ret = getAllDaRules(&rule);
    if (u32Ret == OLERR_NO_ERROR)
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
    u32 u32Ret = OLERR_NO_ERROR;
    da_rule_t * pRule;

    u32Ret = getDaRule((da_ruld_id_t)pcrp->crp_u32RuleId, &pRule);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (pcrp->crp_bVerbose)
            _printDaRuleVerbose(pRule, 1);
        else
            _printDaRuleBrief(pRule, 1);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 processRule(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_rule_param_t * pcrp = (cli_rule_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcrp->crp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _ruleHelp(pdm);
    else if (pcrp->crp_u8Action == CLI_ACTION_RULE_LIST_ALL)
        u32Ret = _listAllRules(pcrp, pdm);
    else if (pcrp->crp_u8Action == CLI_ACTION_RULE_LIST)
        u32Ret = _listOneRule(pcrp, pdm);
    else if (*getEnvVar(ENV_VAR_DATA_PATH) == '\0')
    {
        cliengOutputLine("Data path is not set.");
        u32Ret = OLERR_NOT_READY;
    }
    else
        u32Ret = OLERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamRule(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_rule_param_t * pcrp = (cli_rule_param_t *)pParam;

    memset(pcrp, 0, sizeof(cli_rule_param_t));

    pcrp->crp_u8Action = CLI_ACTION_RULE_LIST_ALL;

    return u32Ret;
}

u32 parseRule(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_rule_param_t * pcrp = (cli_rule_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "l:hv?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'l':
            pcrp->crp_u8Action = CLI_ACTION_RULE_LIST;
            u32Ret = getU32FromString(
                optarg, ol_strlen(optarg), &pcrp->crp_u32RuleId);
            if (u32Ret != OLERR_NO_ERROR)
            {
                cliengReportInvalidOpt('l');
                u32Ret = OLERR_INVALID_PARAM;
            }
            break;
        case 'v':
            pcrp->crp_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcrp->crp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


