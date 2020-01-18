/**
 *  @file stock.c
 *
 *  @brief The stock command implementation.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <math.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <stdlib.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_listhead.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "jf_jiukun.h"

#include "tx_stock.h"
#include "statarbitrage.h"
#include "clicmd.h"
#include "damethod.h"
#include "tx_env.h"

/* --- private data/data structure section ------------------------------------------------------ */
static jf_clieng_caption_t ls_jccStockInfoAdditionalVerbose[] =
{
    {"CorrelationWithIndex", JF_CLIENG_CAP_HALF_LINE}, {"CorrelationWithSmeIndex", JF_CLIENG_CAP_HALF_LINE},
};

static jf_clieng_caption_t ls_jccIndustryInfoBrief[] =
{
    {"Id", 4},
    {"Desc", 30},
    {"NumOfStock", 11},
};

static jf_clieng_caption_t ls_jccStockInfoBrief[] =
{
    {"Id", 5},
    {"Code", 10},
    {"GeneralCapital", 16},
    {"TradableShare", 16},
};

static jf_clieng_caption_t ls_jccIndustryInfoVerbose[] =
{
    {"Id", JF_CLIENG_CAP_FULL_LINE},
    {"Desc", JF_CLIENG_CAP_FULL_LINE},
    {"NumOfStock", JF_CLIENG_CAP_FULL_LINE},
    {"Stock", JF_CLIENG_CAP_FULL_LINE},
};

static jf_clieng_caption_t ls_jccStockInfoVerbose[] =
{
    {"Id", JF_CLIENG_CAP_HALF_LINE}, {"Code", JF_CLIENG_CAP_HALF_LINE},
    {"GeneralCapital", JF_CLIENG_CAP_HALF_LINE}, {"TradableShare", JF_CLIENG_CAP_HALF_LINE},
    {"Industry", JF_CLIENG_CAP_FULL_LINE},
};

/* --- private routine section ------------------------------------------------------------------ */

static u32 _stockHelp(tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
stock\n\
stock [-s stock] [-i] [-v]");
    jf_clieng_outputRawLine2("\
  -s: list the specified stock.\n\
  -i: show industry information.");
    jf_clieng_outputRawLine2("\
  -v: verbose.\n\
  All stocks are listed if no parameter is specified");
    jf_clieng_outputLine("");

    return u32Ret;
}

static void _printStockInfoAdditionalVerbose(tx_stock_info_t * info)
{
    jf_clieng_caption_t * pcc = &ls_jccStockInfoAdditionalVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*CorrelationWithIndex*/
    ol_sprintf(strLeft, "%.2f", getCorrelationWithIndex(info));
    ol_sprintf(strRight, "%.2f", getCorrelationWithSmeIndex(info));
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    jf_clieng_outputLine("");
}

static void _printStockInfoVerbose(u32 u32Id, tx_stock_info_t * info)
{
    jf_clieng_caption_t * pcc = &ls_jccStockInfoVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /* Id, Code */
    ol_sprintf(strLeft, "%u", u32Id);
    ol_sprintf(strRight, "%s", info->tsi_strCode);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* GeneralCapital, TradableShare */
    ol_sprintf(strLeft, "%lld", info->tsi_u64GeneralCapital);
    ol_sprintf(strRight, "%lld", info->tsi_u64TradableShare);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* Industry */
    ol_sprintf(strLeft, "%s", getStringIndustry(info->tsi_nIndustry));
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    jf_clieng_outputLine("");
}

static u32 _printStockVerbose(cli_stock_param_t * pcsp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_info_t * info;

    u32Ret = getStockInfo(pcsp->csp_pstrStock, &info);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _printStockInfoVerbose(1, info);
        _printStockInfoAdditionalVerbose(info);
    }

    return u32Ret;
}

static void _printOneIndustryInfoBrief(tx_stock_indu_info_t * info)
{
    jf_clieng_caption_t * pcc = &ls_jccIndustryInfoBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Id */
    ol_sprintf(strField, "%d", info->tsii_nId);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* desc */
    jf_clieng_appendBriefColumn(pcc, strInfo, info->tsii_pstrDesc);
    pcc++;

    /* NumOfStock */
    ol_sprintf(strField, "%d", info->tsii_nStock);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printOneIndustryInfoVerbose(tx_stock_indu_info_t * info)
{
    jf_clieng_caption_t * pcc = &ls_jccIndustryInfoVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN]; //, strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];
    olint_t n, j, r, len;
    olchar_t line[80];

    jf_clieng_printDivider();

    /*Id*/
    ol_sprintf(strLeft, "%d", info->tsii_nId);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Desc*/
    jf_clieng_printOneFullLine(pcc, info->tsii_pstrDesc);
    pcc += 1;

    /*NumOfStock*/
    ol_sprintf(strLeft, "%d", info->tsii_nStock);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Stocks*/
    ol_sprintf(strLeft, "%s", " ");
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    len = ol_strlen(info->tsii_pstrStocks);
    n = (len + 72 - 1) / 72;
    for (j = 0; j < n; j ++)
    {
        r = len - j * 72;
        if (r > 72)
            r = 72;

        memset(line, 0, sizeof(line));
        ol_strncpy(line, info->tsii_pstrStocks + 72 * j, r);
        jf_clieng_outputLine("%s", line);
    }

    jf_clieng_outputLine("");
}

static u32 _printIndustryInfo(cli_stock_param_t * pcsp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_indu_info_t * info;
    olint_t total = 0;

    if (! pcsp->csp_bVerbose)
    {
        jf_clieng_printDivider();
        jf_clieng_printHeader(
            ls_jccIndustryInfoBrief,
            sizeof(ls_jccIndustryInfoBrief) / sizeof(jf_clieng_caption_t));
    }

    info = getFirstIndustryInfo();
    while (info != NULL)
    {
        if (pcsp->csp_bVerbose)
            _printOneIndustryInfoVerbose(info);
        else
            _printOneIndustryInfoBrief(info);

        total += info->tsii_nStock;

        info = getNextIndustryInfo(info);
    }

    jf_clieng_outputLine("");
    jf_clieng_outputLine("Total %d stocks\n", total);


    return u32Ret;
}

static void _printOneStockInfoBrief(u32 u32Id, tx_stock_info_t * info)
{
    jf_clieng_caption_t * pcc = &ls_jccStockInfoBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Id */
    ol_sprintf(strField, "%u", u32Id);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* code */
    jf_clieng_appendBriefColumn(pcc, strInfo, info->tsi_strCode);
    pcc++;

    /* GeneralCapital */
    ol_sprintf(strField, "%llu", info->tsi_u64GeneralCapital);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* TradableShare */
    ol_sprintf(strField, "%llu", info->tsi_u64TradableShare);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printStocksBrief(cli_stock_param_t * pcsp)
{
    tx_stock_info_t * info;
    u32 u32Id = 0;

    jf_clieng_printDivider();
    jf_clieng_printHeader(
        ls_jccStockInfoBrief, sizeof(ls_jccStockInfoBrief) / sizeof(jf_clieng_caption_t));

    info = getFirstStockInfo();
    while (info != NULL)
    {
        _printOneStockInfoBrief(u32Id + 1, info);
        u32Id ++;

        info = getNextStockInfo(info);
    }

    jf_clieng_outputLine("\nTotal %u stocks\n", u32Id);
}

static void _printStocksVerbose(cli_stock_param_t * pcsp)
{
    tx_stock_info_t * info;
    u32 u32Id = 0;

    info = getFirstStockInfo();
    while (info != NULL)
    {
        _printStockInfoVerbose(u32Id + 1, info);
        u32Id ++;

        info = getNextStockInfo(info);
    }

    jf_clieng_outputLine("\nTotal %u stocks\n", u32Id);
}

static u32 _printAllStocks(cli_stock_param_t * pcsp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pcsp->csp_bVerbose)
    {
        _printStocksVerbose(pcsp);
    }
    else
    {
        _printStocksBrief(pcsp);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processStock(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_stock_param_t * pcsp = (cli_stock_param_t *)pParam;
    tx_cli_master_t * ptcm = (tx_cli_master_t *)pMaster;

    if (pcsp->csp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _stockHelp(ptcm);
    else if (pcsp->csp_u8Action == CLI_ACTION_LIST_INDUSTRY)
        u32Ret = _printIndustryInfo(pcsp);
    else if (tx_env_isNullVarDataPath())
    {
        jf_clieng_outputLine("Data path is not set.");
        u32Ret = JF_ERR_NOT_READY;
    }
    else if (pcsp->csp_u8Action == CLI_ACTION_LIST_STOCK)
        u32Ret = _printStockVerbose(pcsp);
    else if (pcsp->csp_u8Action == CLI_ACTION_LIST_ALL_STOCKS)
        u32Ret = _printAllStocks(pcsp);
    else
        u32Ret = JF_ERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamStock(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_stock_param_t * pcsp = (cli_stock_param_t *)pParam;

    memset(pcsp, 0, sizeof(*pcsp));

    pcsp->csp_u8Action = CLI_ACTION_LIST_ALL_STOCKS;

    return u32Ret;
}

u32 parseStock(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_stock_param_t * pcsp = (cli_stock_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "s:ivh?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 's':
            pcsp->csp_u8Action = CLI_ACTION_LIST_STOCK;
            pcsp->csp_pstrStock = (olchar_t *)optarg;
            break;
        case 'i':
            pcsp->csp_u8Action = CLI_ACTION_LIST_INDUSTRY;
            break;
        case 'v':
            pcsp->csp_bVerbose = TRUE;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case '?':
        case 'h':
            pcsp->csp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


