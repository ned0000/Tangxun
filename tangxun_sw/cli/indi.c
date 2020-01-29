/**
 *  @file indi.c
 *
 *  @brief The indi command implementation.
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
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"

#include "tx_daysummary.h"
#include "tx_indi.h"
#include "tx_stock.h"
#include "tx_env.h"

#include "clicmd.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define CLI_INDI_OPTIMIZE_DAY_SUMMARY  (40)  /*2 months*/

#define CLI_INDI_TOTAL_OPTIMIZE_DAY_SUMMARY  (100 + CLI_INDI_OPTIMIZE_DAY_SUMMARY) 

#define MAX_INDI_DAY_SUMMARY (CLI_INDI_TOTAL_OPTIMIZE_DAY_SUMMARY)

static jf_clieng_caption_t ls_jccTxIndiBrief[] =
{
    {"Id", 3},
    {"Name", 8},
    {"Type", 15},
    {"Description", 65},
};

static jf_clieng_caption_t ls_jccTradeDaySummaryIndiMacdBrief[] =
{
    {"Day", 5}, 
    {"Date", 12},
    {"ClosePrice", 11},
    {"ShortEma", 9},
    {"LongEma", 9},
    {"DIFF", 9},
    {"DEA", 9},
	{"MACD", 9},
};

static jf_clieng_caption_t ls_jccTradeDaySummaryIndiDmiBrief[] =
{
    {"Day", 5}, 
    {"Date", 12},
    {"PDI", 9},
    {"MDI", 9},
    {"ADX", 9},
    {"ADXR", 9},
};

/* --- private routine section ------------------------------------------------------------------ */

static u32 _indiHelp(tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Indicator command.\n\
indi [-l] [-i indicator] [-p param] [-d] [-t] [-s stock] [-c date] [-v] \n\
  -l: list technical indicator.");
    jf_clieng_outputRawLine2("\
  -i: specify the indicator.\n\
  -p: specify the parameters.\n\
  -d: print indicator data.");
    jf_clieng_outputRawLine2("\
  -t: test indicator.\n\
  -s: specify the stock.");
    jf_clieng_outputRawLine2("\
  -c: the end of date, the format is \"yyyy-mm-dd\".\n\
  -v: verbose.\n\
  By default, all technical indicators are listed.\n");

    return u32Ret;
}

static void _printTradeDaySummaryIndiMacdBrief(tx_ds_t * cur)
{
    jf_clieng_caption_t * pcc = &ls_jccTradeDaySummaryIndiMacdBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Day */
    ol_sprintf(strField, "%d", cur->td_nIndex);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Date */
    jf_clieng_appendBriefColumn(pcc, strInfo, cur->td_strDate);
    pcc++;

    /* ClosingPrice */
    ol_sprintf(strField, "%.2f", cur->td_dbClosingPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    if (cur->td_ptimMacd != NULL)
    {
        /* ShortEma */
        ol_sprintf(strField, "%.2f", cur->td_ptimMacd->tim_dbShortEma);
        jf_clieng_appendBriefColumn(pcc, strInfo, strField);
        pcc++;

        /* LongEma */
        ol_sprintf(strField, "%.2f", cur->td_ptimMacd->tim_dbLongEma);
        jf_clieng_appendBriefColumn(pcc, strInfo, strField);
        pcc++;

        /* DIFF */
        ol_sprintf(strField, "%.2f", cur->td_ptimMacd->tim_dbDiff);
        jf_clieng_appendBriefColumn(pcc, strInfo, strField);
        pcc++;

        /* DEA */
        ol_sprintf(strField, "%.2f", cur->td_ptimMacd->tim_dbDea);
        jf_clieng_appendBriefColumn(pcc, strInfo, strField);
        pcc++;

        /* MACD */
        ol_sprintf(strField, "%.2f", cur->td_ptimMacd->tim_dbMacd);
        jf_clieng_appendBriefColumn(pcc, strInfo, strField);
        pcc++;
    }

    jf_clieng_outputLine(strInfo);
}

static void _printTradeDaySummaryIndiMacd(
    cli_indi_param_t * pcip, tx_ds_t * buffer, olint_t total)
{
    tx_ds_t * cur = NULL;
    olint_t index = 0;

    if (! pcip->cip_bVerbose)
    {
        jf_clieng_printDivider();

        jf_clieng_printHeader(
            ls_jccTradeDaySummaryIndiMacdBrief,
            sizeof(ls_jccTradeDaySummaryIndiMacdBrief) / sizeof(jf_clieng_caption_t));
    }

    cur = buffer;
    for (index = 0; index < total; index ++)
    {
        if (pcip->cip_bVerbose)
            ;
        else
            _printTradeDaySummaryIndiMacdBrief(cur);
        cur ++;
    }

    jf_clieng_outputLine("");
}

static void _printTradeDaySummaryIndiDmiBrief(tx_ds_t * cur)
{
    jf_clieng_caption_t * pcc = &ls_jccTradeDaySummaryIndiDmiBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Day */
    ol_sprintf(strField, "%d", cur->td_nIndex);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Date */
    jf_clieng_appendBriefColumn(pcc, strInfo, cur->td_strDate);
    pcc++;

    if (cur->td_ptidDmi != NULL)
    {
        /* PDI */
        ol_sprintf(strField, "%.2f", cur->td_ptidDmi->tid_dbPdi);
        jf_clieng_appendBriefColumn(pcc, strInfo, strField);
        pcc++;

        /* MDI */
        ol_sprintf(strField, "%.2f", cur->td_ptidDmi->tid_dbMdi);
        jf_clieng_appendBriefColumn(pcc, strInfo, strField);
        pcc++;

        /* ADX */
        ol_sprintf(strField, "%.2f", cur->td_ptidDmi->tid_dbAdx);
        jf_clieng_appendBriefColumn(pcc, strInfo, strField);
        pcc++;

        /* ADXR */
        ol_sprintf(strField, "%.2f", cur->td_ptidDmi->tid_dbAdxr);
        jf_clieng_appendBriefColumn(pcc, strInfo, strField);
        pcc++;
    }

    jf_clieng_outputLine(strInfo);
}

static void _printTradeDaySummaryIndiDmi(
    cli_indi_param_t * pcip, tx_ds_t * buffer, olint_t total)
{
    tx_ds_t * cur = NULL;
    olint_t index = 0;

    if (! pcip->cip_bVerbose)
    {
        jf_clieng_printDivider();

        jf_clieng_printHeader(
            ls_jccTradeDaySummaryIndiDmiBrief,
            sizeof(ls_jccTradeDaySummaryIndiDmiBrief) / sizeof(jf_clieng_caption_t));
    }

    cur = buffer;
    for (index = 0; index < total; index ++)
    {
        if (pcip->cip_bVerbose)
            ;
        else
            _printTradeDaySummaryIndiDmiBrief(cur);
        cur ++;
    }

    jf_clieng_outputLine("");
}

static void _listOneIndicatorBrief(tx_indi_t * indi)
{
    jf_clieng_caption_t * pcc = &ls_jccTxIndiBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Id */
    ol_sprintf(strField, "%d", indi->ti_nId);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Name */
    jf_clieng_appendBriefColumn(pcc, strInfo, indi->ti_pstrName);
    pcc++;

    /* Type */
    jf_clieng_appendBriefColumn(pcc, strInfo, tx_indi_getStringIndiType(indi->ti_nType));
    pcc++;

    /* Description */
    jf_clieng_appendBriefColumn(pcc, strInfo, indi->ti_pstrDesc);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static u32 _listAllIndicatorsBrief(cli_indi_param_t * pcip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_t * indi;
    olint_t id;

    for (id = TX_INDI_ID_DMI; id < TX_INDI_ID_MAX; id ++)
    {
        u32Ret = tx_indi_getIndiById(id, &indi);

        _listOneIndicatorBrief(indi);
    }
    jf_clieng_outputLine("");

    return u32Ret;
}

static u32 _listAllIndicators(cli_indi_param_t * pcip, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pcip->cip_bVerbose)
    {

    }
    else
    {
        jf_clieng_printDivider();

        jf_clieng_printHeader(
            ls_jccTxIndiBrief, sizeof(ls_jccTxIndiBrief) / sizeof(jf_clieng_caption_t));

        _listAllIndicatorsBrief(pcip);
    }

    return u32Ret;
}

static u32 _listIndicator(cli_indi_param_t * pcip, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_t * indi = NULL;

    u32Ret = tx_indi_getIndiByName(pcip->cip_pstrIndicator, &indi);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pcip->cip_bVerbose)
        {

        }
        else
        {
            jf_clieng_printDivider();

            jf_clieng_printHeader(
                ls_jccTxIndiBrief, sizeof(ls_jccTxIndiBrief) / sizeof(jf_clieng_caption_t));

            _listOneIndicatorBrief(indi);
        }
    }

    return u32Ret;
}

static u32 _printIndiData(
    cli_indi_param_t * pcip, struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t str[256];

    u32Ret = indi->ti_fnGetStringParam(indi, pParam, str);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_clieng_outputLine("Indicator Parameter: %s", str);

        switch (indi->ti_nId)
        {
        case TX_INDI_ID_MACD:
            _printTradeDaySummaryIndiMacd(pcip, buffer, total);
            break;
        case TX_INDI_ID_DMI:
            _printTradeDaySummaryIndiDmi(pcip, buffer, total);
            break;
        default:
            break;
        }
    }

    return u32Ret;
}

static u32 _indiTest(cli_indi_param_t * pcip, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = MAX_INDI_DAY_SUMMARY;
    tx_stock_info_t * stockinfo = NULL;
    tx_ds_t * buffer = NULL;
    tx_indi_t * indi = NULL;
    tx_indi_param_t tip;

    if ((pcip->cip_pstrStock == NULL) || (pcip->cip_pstrIndicator == NULL))
        u32Ret = JF_ERR_INVALID_PARAM;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = tx_indi_getIndiByName(pcip->cip_pstrIndicator, &indi);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = tx_stock_getStockInfo(pcip->cip_pstrStock, &stockinfo);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];

        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, stockinfo->tsi_strCode);

        u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = indi->ti_fnSetDefaultParam(indi, &tip);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = indi->ti_fnCalc(indi, &tip, buffer, total);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _printIndiData(pcip, indi, &tip, buffer, total);

    if (buffer != NULL)
    {
        tx_indi_freeDaySummaryIndi(buffer, total);
        jf_jiukun_freeMemory((void **)&buffer);
    }    

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processIndi(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_indi_param_t * pcip = (cli_indi_param_t *)pParam;
    tx_cli_master_t * ptcm = (tx_cli_master_t *)pMaster;

    if (pcip->cip_u8Action == CLI_ACTION_SHOW_HELP)
    {
        u32Ret = _indiHelp(ptcm);
    }
    else if (pcip->cip_u8Action == CLI_ACTION_INDI_LIST)
    {
        if (pcip->cip_pstrIndicator == NULL)
            u32Ret = _listAllIndicators(pcip, ptcm);
        else
            u32Ret = _listIndicator(pcip, ptcm);
    }
    else if (pcip->cip_u8Action == CLI_ACTION_INDI_TEST)
    {
        u32Ret = _indiTest(pcip, ptcm);
    }
    else
    {
        u32Ret = JF_ERR_MISSING_PARAM;
    }

    return u32Ret;
}

u32 setDefaultParamIndi(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_indi_param_t * pcip = (cli_indi_param_t *)pParam;

    ol_bzero(pcip, sizeof(cli_indi_param_t));
    pcip->cip_u8Action = CLI_ACTION_INDI_LIST;

    return u32Ret;
}

u32 parseIndi(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_indi_param_t * pcip = (cli_indi_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv, "lc:i:p:df:ts:hv?")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'l':
            pcip->cip_u8Action = CLI_ACTION_INDI_LIST;
            break;
        case 'c':
            pcip->cip_pstrEndDate = (olchar_t *)optarg;
            break;
        case 'i':
            pcip->cip_pstrIndicator = (olchar_t *)optarg;
            break;
        case 'p':
            pcip->cip_pstrParam = (olchar_t *)optarg;
            break;
        case 'd':
            pcip->cip_bPrintData = TRUE;
            break;
        case 't':
            pcip->cip_u8Action = CLI_ACTION_INDI_TEST;
            break;
        case 's':
            pcip->cip_pstrStock = (olchar_t *)optarg;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case 'v':
            pcip->cip_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcip->cip_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

