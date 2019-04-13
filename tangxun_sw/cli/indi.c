/**
 *  @file indi.c
 *
 *  @brief The indi command
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
#include "clicmd.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "parsedata.h"
#include "indicator.h"
#include "stocklist.h"
#include "envvar.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define MAX_INDI_DAY_SUMMARY (TOTAL_OPTIMIZE_INDI_DAY_SUMMARY + 8 * OPTIMIZE_INDI_DAY_SUMMARY)

/* --- private routine section ------------------------------------------------------------------ */
static u32 _indiHelp(da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Indicators \n\
indi [-l] [-e stock] [-i indicator] [-p param] [-d]\n\
       [-t stock] [-f stock] [-g stock] [-c date] [-v] \n\
  -l: list all available technical indicators.");
    jf_clieng_outputRawLine2("\
  -e: use best technical indicator to evaluate stock.\n\
  -i: specify the indicator.\n\
  -p: specify the parameters.\n\
  -d: prolint_t indicator data");
    jf_clieng_outputRawLine2("\
  -f: find the best technical indicator and parameterss for the stock. If the\n\
      stock is \"all\", all stocks are included.\n\
  -t: test technical indicators.");
    jf_clieng_outputRawLine2("\
  -g: use all available indicators to evaluate stock.\n\
  -a: ADXR.\n\
  -c: the end of date, the format is \"yyyy-mm-dd\".\n\
  -v: verbose.\n");

    return u32Ret;
}

static u32 _daySummaryIndiSystem(
    cli_indi_param_t * pcip, stock_info_t * stockinfo,
    da_indicator_desc_t * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, start;
    da_conc_t daconc;
    da_conc_sum_t concsum;

    jf_clieng_printBanner(JF_CLIENG_MAX_OUTPUT_LINE_LEN);
    jf_clieng_outputLine("%s System", indi->did_pstrName);
    if (pcip->cip_bVerbose)
    {
        indi->did_fnPrintParamVerbose(indi, pdip);
        jf_clieng_printDivider();
    }

    memset(&concsum, 0, sizeof(da_conc_sum_t));
    memset(&daconc, 0, sizeof(da_conc_t));
    if (num > OPTIMIZE_INDI_DAY_SUMMARY)
        start = num - OPTIMIZE_INDI_DAY_SUMMARY + 1;
    else
        start = 10;
    for (i = start;
         (i <= num) && (u32Ret == JF_ERR_NO_ERROR); i ++)
    {
        u32Ret = indi->did_fnSystem(indi, pdip, buffer, i, &daconc);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (daconc.dc_nAction == CONC_ACTION_BULL_OPENING)
            {
                if (pcip->cip_bVerbose)
                    ol_printf(
                        "%s, opening with price %.2f\n",
                        (buffer + i - 1)->dds_strDate,
//                        (buffer + i - 1)->dds_ddDmi->dd_dbAdxr,
                        daconc.dc_dbPrice);
            }
            else if (daconc.dc_nAction == CONC_ACTION_BULL_CLOSEOUT)
            {
                if (pcip->cip_bVerbose)
                    ol_printf(
                        "%s, close out with price %.2f, hold %d days, yield %.2f%%\n",
                        (buffer + i - 1)->dds_strDate,
//                        (buffer + i - 1)->dds_ddDmi->dd_dbAdxr,
                        daconc.dc_dbPrice,
                        daconc.dc_nHoldDays, daconc.dc_dbYield);
                addToDaConcSum(&concsum, &daconc);
            }
            if (pcip->cip_bPrintData)
                indi->did_fnPrintDaySummary(indi, buffer + i - 1, 1);
        }
        else if (u32Ret == JF_ERR_NOT_READY)
            u32Ret = JF_ERR_NO_ERROR;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        printDaConcSumVerbose(&concsum);
        jf_clieng_outputLine("");
    }

    return u32Ret;
}

static u32 _indiEvaluateDaySummary(
    cli_indi_param_t * pcip, stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_indicator_desc_t * indi = NULL;
    da_indicator_param_t dip;

    jf_clieng_outputLine("Stock: %s", stockinfo->si_strCode);
    memset(&dip, 0, sizeof(da_indicator_param_t));
#if 0
    if (pcip->cip_pstrIndicator != NULL)
        indi = getDaIndicatorDescByName(pcip->cip_pstrIndicator);

    if (indi == NULL)
        indi = getDaIndicatorDesc(stockinfo->si_nIndicator);

    if (indi == NULL)
    {
        jf_clieng_outputLine("Cannot get suitable indicator");
        return u32Ret;
    }

    if (pcip->cip_pstrParam != NULL)
        indi->did_fnGetParamFromString(indi, &dip, pcip->cip_pstrParam);
    else
        indi->did_fnGetParamFromString(indi, &dip, stockinfo->si_pstrIndicatorParam);
#endif
    _daySummaryIndiSystem(pcip, stockinfo, indi, &dip, buffer, num);
    freeDaDaySummaryIndicator(buffer, num);

    return u32Ret;
}

static u32 _indiEvaluateStock(cli_indi_param_t * pcip, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = MAX_INDI_DAY_SUMMARY;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    stock_info_t * stockinfo;
    da_day_summary_t * buffer = NULL;

    u32Ret = jf_mem_calloc((void **)&buffer, sizeof(da_day_summary_t) * total);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = getStockInfo(pcip->cip_pstrStock, &stockinfo);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR,
            pcip->cip_pstrStock);

        u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pcip->cip_pstrEndDate != NULL)
            total = daySummaryEndDataCount(buffer, total, pcip->cip_pstrEndDate);

        u32Ret = _indiEvaluateDaySummary(pcip, stockinfo, buffer, total);
    }

    if (buffer != NULL)
        jf_mem_free((void **)&buffer);

    return u32Ret;
}

static u32 _testIndicatorSystem(
    cli_indi_param_t * pcip, stock_info_t * stockinfo,
    da_indicator_desc_t * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_conc_t daconc;

    jf_clieng_printDivider();
    jf_clieng_outputLine("%s System", indi->did_pstrName);

    memset(&daconc, 0, sizeof(daconc));

    u32Ret = indi->did_fnSystem(indi, pdip, buffer, num, &daconc);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        indi->did_fnPrintDaySummary(indi, buffer, num);
    }

    return u32Ret;
}

static u32 _daySummaryIndiTest(
    cli_indi_param_t * pcip, stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_indicator_desc_t * indi;
    da_indicator_param_t dip;
    olint_t id;

    jf_clieng_outputLine("Stock: %s", stockinfo->si_strCode);

    if (pcip->cip_pstrIndicator != NULL)
    {
        indi = getDaIndicatorDescByName(pcip->cip_pstrIndicator);
        if (indi == NULL)
        {
            jf_clieng_outputLine("Cannot find indicator %s\n", pcip->cip_pstrIndicator);
            u32Ret = JF_ERR_NOT_FOUND;
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            indi->did_fnGetParamFromString(indi, &dip, pcip->cip_pstrParam);
            _testIndicatorSystem(pcip, stockinfo, indi, &dip, buffer, num);
        }
    }
    else
    {
        for (id = STOCK_INDICATOR_UNKNOWN + 1; id < STOCK_INDICATOR_MAX; id ++)
        {
            indi = getDaIndicatorDesc(id);
            memset(&dip, 0, sizeof(da_indicator_param_t));
            _testIndicatorSystem(pcip, stockinfo, indi, &dip, buffer, num);
        }
    }
    jf_clieng_outputLine("");

    return u32Ret;
}

static u32 _indiTest(cli_indi_param_t * pcip, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = 100;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    stock_info_t * stockinfo;
    da_day_summary_t * buffer = NULL;

    u32Ret = jf_mem_calloc((void **)&buffer, sizeof(da_day_summary_t) * total);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = getStockInfo(pcip->cip_pstrStock, &stockinfo);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR,
            pcip->cip_pstrStock);

        u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _daySummaryIndiTest(pcip, stockinfo, buffer, total);

    if (buffer != NULL)
    {
        freeDaDaySummaryIndicator(buffer, total);
        jf_mem_free((void **)&buffer);
    }

    return u32Ret;
}

static u32 _daySummaryIndiFind(
    cli_indi_param_t * pcip, stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_indicator_param_t dip;
    da_conc_sum_t bestconc;
    olint_t nIndicator;
    olchar_t indiparam[512];
    da_indicator_desc_t * indi;
    get_optimized_indicator_param_t goip;
//    da_day_summary_t * end = buffer + num - 1;

    jf_clieng_outputLine("Stock: %s", stockinfo->si_strCode);
    memset(&bestconc, 0, sizeof(da_conc_sum_t));
    memset(&goip, 0, sizeof(get_optimized_indicator_param_t));
    goip.goip_bVerbose = pcip->cip_bVerbose;
    goip.goip_dbMinProfitTimeRatio = MIN_PROFIT_TIME_RATIO;

    u32Ret = getOptimizedIndicator(
        buffer, num, &goip, &nIndicator, &dip, &bestconc);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        indi = getDaIndicatorDesc(nIndicator);
        indi->did_fnGetStringParam(indi, &dip, indiparam);
//        setStockInfoIndicator(
//            stockinfo, nIndicator, indiparam, end->dds_strDate);
        jf_clieng_outputLine(
            "%s, param %s", getStringIndicatorName(nIndicator), indiparam);
    }
    else
    {
        jf_clieng_outputLine(
            "Cannot get optimized indicator.");
        u32Ret = JF_ERR_NO_ERROR;
    }

    return u32Ret;
}

static u32 _indiOneStockFind(
    cli_indi_param_t * pcip, stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t * num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    olint_t total;

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR,
        stockinfo->si_strCode);

    u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, num);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        total = *num;
        if (pcip->cip_pstrEndDate != NULL)
            total = daySummaryEndDataCount(buffer, *num, pcip->cip_pstrEndDate);

        u32Ret = _daySummaryIndiFind(pcip, stockinfo, buffer, total);
    }

    freeDaDaySummaryIndicator(buffer, *num);

    return u32Ret;
}

static u32 _indiFind(cli_indi_param_t * pcip, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = MAX_INDI_DAY_SUMMARY;
    stock_info_t * stockinfo;
    da_day_summary_t * buffer = NULL;

    u32Ret = jf_mem_calloc((void **)&buffer, sizeof(da_day_summary_t) * total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (strncmp(pcip->cip_pstrStock, "all", 3) == 0)
        {
            stockinfo = getFirstStockInfo();
            while ((stockinfo != NULL) && (u32Ret == JF_ERR_NO_ERROR))
            {
#if 0
                if (stockinfo->si_nIndicator == 0)
                {
                    total = MAX_INDI_DAY_SUMMARY;
                    u32Ret = _indiOneStockFind(
                        pcip, stockinfo, buffer, &total);
                }
#endif
                stockinfo = getNextStockInfo(stockinfo);
            }
        }
        else
        {
            u32Ret = getStockInfo(pcip->cip_pstrStock, &stockinfo);
            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _indiOneStockFind(
                    pcip, stockinfo, buffer, &total);
        }
    }

    if (buffer != NULL)
        jf_mem_free((void **)&buffer);

    return u32Ret;
}

static u32 _indisEvaluateDaySummary(
    cli_indi_param_t * pcip, stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_indicator_desc_t * indi;
    da_indicator_param_t dip;
    olint_t id;

    jf_clieng_outputLine("Stock: %s", stockinfo->si_strCode);

    for (id = STOCK_INDICATOR_UNKNOWN + 1; id < STOCK_INDICATOR_MAX; id ++)
    {
        indi = getDaIndicatorDesc(id);
        memset(&dip, 0, sizeof(da_indicator_param_t));
        _daySummaryIndiSystem(pcip, stockinfo, indi, &dip, buffer, num);
    }

    return u32Ret;
}

static u32 _indisEvaluateStock(
    cli_indi_param_t * pcip, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = MAX_INDI_DAY_SUMMARY;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    stock_info_t * stockinfo;
    da_day_summary_t * buffer = NULL;

    u32Ret = jf_mem_calloc((void **)&buffer, sizeof(da_day_summary_t) * total);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = getStockInfo(pcip->cip_pstrStock, &stockinfo);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR,
            pcip->cip_pstrStock);

        u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _indisEvaluateDaySummary(pcip, stockinfo, buffer, total);
    }

    if (buffer != NULL)
    {
        freeDaDaySummaryIndicator(buffer, total);
        jf_mem_free((void **)&buffer);
    }

    return u32Ret;
}

static u32 _listAllIndicators(cli_indi_param_t * pcip, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_indicator_desc_t * indi;
    olint_t id;

    for (id = STOCK_INDICATOR_DMI; id < STOCK_INDICATOR_MAX; id ++)
    {
        indi = getDaIndicatorDesc(id);
        indi->did_fnPrintDescVerbose(indi);
    }
    jf_clieng_outputLine("");

    return u32Ret;
}

static u32 _daySummaryIndiAdxr(
    cli_indi_param_t * pcip, stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_adxr_t * adxr;
    da_day_summary_t * start, * end;
    olint_t count;
    oldouble_t ratio;

    jf_clieng_outputLine("Stock: %s", stockinfo->si_strCode);

    u32Ret = getIndicatorAdxrTrend(buffer, num);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        end = buffer + num - 1;
        adxr = end->dds_pdaAdxr;
        count = 0;
        start = end - OPTIMIZE_INDI_DAY_SUMMARY + 1;
        while (start <= end)
        {
            adxr = start->dds_pdaAdxr;
            if (adxr->da_dbAdxr > end->dds_pdaAdxr->da_dbAdxrTrend)
                count ++;
            ol_printf("%.2f ", adxr->da_dbAdxr);
            start ++;
        }
        ol_printf("\n\n");
        ratio = count * 100;
        ratio /= OPTIMIZE_INDI_DAY_SUMMARY;
        ol_printf("%s, Adxr %.2f, AdxrTrend %.2f(%.2f%%)\n\n",
               getStringKcp(end->dds_pdaAdxr->da_nKcp),
               end->dds_pdaAdxr->da_dbAdxr, end->dds_pdaAdxr->da_dbAdxrTrend, ratio);
    }

    freeDaDaySummaryIndicator(buffer, num);

    return u32Ret;
}

static u32 _indiAdxrOneStock(
    cli_indi_param_t * pcip, stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t * num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR,
        stockinfo->si_strCode);

    u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, num);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _daySummaryIndiAdxr(pcip, stockinfo, buffer, *num);

    freeDaDaySummaryIndicator(buffer, *num);

    return u32Ret;
}

static u32 _indiAdxr(cli_indi_param_t * pcip, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = MAX_INDI_DAY_SUMMARY;
    stock_info_t * stockinfo;
    da_day_summary_t * buffer = NULL;

    u32Ret = jf_mem_calloc((void **)&buffer, sizeof(da_day_summary_t) * total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (strncmp(pcip->cip_pstrStock, "all", 3) == 0)
        {
            stockinfo = getFirstStockInfo();
            while ((stockinfo != NULL) && (u32Ret == JF_ERR_NO_ERROR))
            {
#if 0
                if (stockinfo->si_nIndicator == 0)
                {
                    total = MAX_INDI_DAY_SUMMARY;
                    u32Ret = _indiAdxrOneStock(
                        pcip, stockinfo, buffer, &total);
                }
#endif
                stockinfo = getNextStockInfo(stockinfo);
            }
        }
        else
        {
            u32Ret = getStockInfo(pcip->cip_pstrStock, &stockinfo);
            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _indiAdxrOneStock(
                    pcip, stockinfo, buffer, &total);
        }
    }

    if (buffer != NULL)
        jf_mem_free((void **)&buffer);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processIndi(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_indi_param_t * pcip = (cli_indi_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcip->cip_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _indiHelp(pdm);
    else if (pcip->cip_u8Action == CLI_ACTION_INDI_LIST)
        u32Ret = _listAllIndicators(pcip, pdm);
    else if (pcip->cip_u8Action == CLI_ACTION_INDI_EVALUATE)
        u32Ret = _indiEvaluateStock(pcip, pdm);
    else if (pcip->cip_u8Action == CLI_ACTION_INDI_TEST)
        u32Ret = _indiTest(pcip, pdm);
    else if (pcip->cip_u8Action == CLI_ACTION_INDI_FIND)
        u32Ret = _indiFind(pcip, pdm);
    else if (pcip->cip_u8Action == CLI_ACTION_INDIS_EVALUATE)
        u32Ret = _indisEvaluateStock(pcip, pdm);
    else if (pcip->cip_u8Action == CLI_ACTION_INDI_ADXR)
        u32Ret = _indiAdxr(pcip, pdm);
    else
        u32Ret = JF_ERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamIndi(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_indi_param_t * pcip = (cli_indi_param_t *)pParam;

    memset(pcip, 0, sizeof(cli_indi_param_t));

    return u32Ret;
}

u32 parseIndi(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_indi_param_t * pcip = (cli_indi_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "a:le:c:i:p:df:g:t:hv?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'a':
            pcip->cip_u8Action = CLI_ACTION_INDI_ADXR;
            pcip->cip_pstrStock = (olchar_t *)optarg;
            break;
        case 'l':
            pcip->cip_u8Action = CLI_ACTION_INDI_LIST;
            break;
        case 'c':
            pcip->cip_pstrEndDate = (olchar_t *)optarg;
            break;
        case 'e':
            pcip->cip_u8Action = CLI_ACTION_INDI_EVALUATE;
            pcip->cip_pstrStock = (olchar_t *)optarg;
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
        case 'f':
            pcip->cip_u8Action = CLI_ACTION_INDI_FIND;
            pcip->cip_pstrStock = (olchar_t *)optarg;
            break;
        case 'g':
            pcip->cip_u8Action = CLI_ACTION_INDIS_EVALUATE;
            pcip->cip_pstrStock = (olchar_t *)optarg;
            break;
        case 't':
            pcip->cip_u8Action = CLI_ACTION_INDI_TEST;
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


