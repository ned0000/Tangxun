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

#include "parsedata.h"
#include "indicator.h"
#include "tx_stock.h"
#include "tx_env.h"
#include "clicmd.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define MAX_INDI_DAY_SUMMARY (TOTAL_OPTIMIZE_INDI_DAY_SUMMARY + 8 * OPTIMIZE_INDI_DAY_SUMMARY)

/* --- private routine section ------------------------------------------------------------------ */
static u32 _indiHelp(tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Indicators \n\
indi [-l] [-i indicator] [-p param] [-d]\n\
       [-t stock] [-c date] [-v] \n\
  -l: list all available technical indicators.");
    jf_clieng_outputRawLine2("\
  -i: specify the indicator.\n\
  -p: specify the parameters.\n\
  -d: prolint_t indicator data");
    jf_clieng_outputRawLine2("\
  -t: test technical indicators.");
    jf_clieng_outputRawLine2("\
  -a: ADXR.\n\
  -c: the end of date, the format is \"yyyy-mm-dd\".\n\
  -v: verbose.\n");

    return u32Ret;
}

static u32 _listAllIndicators(cli_indi_param_t * pcip, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    da_indicator_desc_t * indi;
    olint_t id;

    for (id = STOCK_INDICATOR_DMI; id < STOCK_INDICATOR_MAX; id ++)
    {
//        indi = getDaIndicatorDesc(id);

        
    }
    jf_clieng_outputLine("");

    return u32Ret;
}

static u32 _daySummaryIndiAdxr(
    cli_indi_param_t * pcip, tx_stock_info_t * stockinfo, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_adxr_t * adxr;
    da_day_summary_t * start, * end;
    olint_t count;
    oldouble_t ratio;

    jf_clieng_outputLine("Stock: %s", stockinfo->tsi_strCode);

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
    cli_indi_param_t * pcip, tx_stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t * num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, stockinfo->tsi_strCode);

    u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, num);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _daySummaryIndiAdxr(pcip, stockinfo, buffer, *num);

    freeDaDaySummaryIndicator(buffer, *num);

    return u32Ret;
}

static u32 _indiAdxr(cli_indi_param_t * pcip, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = MAX_INDI_DAY_SUMMARY;
    tx_stock_info_t * stockinfo;
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

static u32 _indiTest(cli_indi_param_t * pcip, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processIndi(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_indi_param_t * pcip = (cli_indi_param_t *)pParam;
    tx_cli_master_t * ptcm = (tx_cli_master_t *)pMaster;

    if (pcip->cip_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _indiHelp(ptcm);
    else if (pcip->cip_u8Action == CLI_ACTION_INDI_LIST)
        u32Ret = _listAllIndicators(pcip, ptcm);
    else if (pcip->cip_u8Action == CLI_ACTION_INDI_TEST)
        u32Ret = _indiTest(pcip, ptcm);
    else if (pcip->cip_u8Action == CLI_ACTION_INDI_ADXR)
        u32Ret = _indiAdxr(pcip, ptcm);
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
        "a:lc:i:p:df:t:hv?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
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


