/**
 *  @file download.c
 *
 *  @brief The download command implementation.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_listhead.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_jiukun.h"
#include "jf_process.h"

#include "tx_download.h"
#include "tx_env.h"

#include "clicmd.h"

/* --- private data/data structure section ------------------------------------------------------ */



/* --- private routine section ------------------------------------------------------------------ */
static u32 _downloadHelp(tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Download data. Download trade summary of stocks in list and trade summary of\n\
SH, SZ index if no option is specified.\n\
download [-s stock] [-e] [-i] [-d dir] [-o]\n\
    [-f first-date] [-l last-date] [-t count] [-p sleep-second] ");
    jf_clieng_outputRawLine2("\
  -s: Specify the stock to download data. By default, download trade summary.\n\
  -e: download trade detail.\n\
  -i: download trade summary of all index.\n\
  -d: directory containing data.");
    jf_clieng_outputRawLine2("\
  -t: iterative count.\n\
  -p: sleep certain minutes.\n\
  -o: overwrite the existing data file.");
    jf_clieng_outputRawLine2("\
  -f: the first date with format \"yyyy-mm-dd\", used with '-e'.\n\
  -l: the last date with format \"yyyy-mm-dd\", used with '-e'.");
    jf_clieng_outputLine("");

    return u32Ret;
}

static u32 _dlStock(cli_download_param_t * pcdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_download_trade_summary_param_t tdtsp;

    if (pcdp->cdp_pstrStock == NULL)
    {
        memset(&tdtsp, 0, sizeof(tdtsp));

        tdtsp.tdtsp_pstrDataDir = pcdp->cdp_pstrDataDir;
        tdtsp.tdtsp_bOverwrite = pcdp->cdp_bOverwrite;

        tx_download_dlIndexTradeSummary(&tdtsp);
    }

    memset(&tdtsp, 0, sizeof(tdtsp));

    tdtsp.tdtsp_pstrStock = pcdp->cdp_pstrStock;
    tdtsp.tdtsp_pstrDataDir = pcdp->cdp_pstrDataDir;
    tdtsp.tdtsp_bOverwrite = pcdp->cdp_bOverwrite;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = tx_download_dlTradeSummary(&tdtsp);

    jf_clieng_outputLine("");

    return u32Ret;
}

static void _dlSleepSecond(cli_download_param_t * pcdp)
{
    if (pcdp->cdp_nSleep != 0)
        sleep(pcdp->cdp_nSleep * 60);
}

static u32 _dlStockIterative(cli_download_param_t * pcdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count = 0;

    _dlSleepSecond(pcdp);

    if (pcdp->cdp_pstrStock != NULL)
        return _dlStock(pcdp);

    while ((u32Ret == JF_ERR_NO_ERROR) && (count < pcdp->cdp_u8IterativeCount))
    {
        jf_clieng_outputLine("\nDownload for the %dth time", count + 1);
        u32Ret = _dlStock(pcdp);

        count ++;
    }
    jf_clieng_outputLine("");

    return u32Ret;
}

static u32 _dlStockDetail(cli_download_param_t * pcdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_download_trade_detail_param_t tdtdp;

    if (pcdp->cdp_pstrStock == NULL)
        return JF_ERR_INVALID_PARAM;

    if ((pcdp->cdp_pstrStartDate == NULL) || (pcdp->cdp_pstrEndDate == NULL))
        return JF_ERR_INVALID_PARAM;

    memset(&tdtdp, 0, sizeof(tdtdp));

    tdtdp.tdtdp_pstrStartDate = pcdp->cdp_pstrStartDate;
    tdtdp.tdtdp_pstrEndDate = pcdp->cdp_pstrEndDate;
    tdtdp.tdtdp_pstrStock = pcdp->cdp_pstrStock;
    tdtdp.tdtdp_pstrDataDir = pcdp->cdp_pstrDataDir;
    tdtdp.tdtdp_bOverwrite = pcdp->cdp_bOverwrite;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = tx_download_dlTradeDetail(&tdtdp);

    return u32Ret;
}

static u32 _dlStockIndex(cli_download_param_t * pcdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_download_trade_summary_param_t tdtsp;

    memset(&tdtsp, 0, sizeof(tdtsp));

    tdtsp.tdtsp_pstrDataDir = pcdp->cdp_pstrDataDir;
    tdtsp.tdtsp_bOverwrite = pcdp->cdp_bOverwrite;

    u32Ret = tx_download_dlIndexTradeSummary(&tdtsp);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */
u32 processDownload(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_download_param_t * pcdp = (cli_download_param_t *)pParam;
    tx_cli_master_t * ptcm = (tx_cli_master_t *)pMaster;

    if (pcdp->cdp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _downloadHelp(ptcm);
    else if (*tx_env_getVar(TX_ENV_VAR_DATA_PATH) == '\0')
    {
        jf_clieng_outputLine("Data path is not set.");
        u32Ret = JF_ERR_NOT_READY;
    }
    else if (pcdp->cdp_u8Action == CLI_ACTION_DOWNLOAD_TRADE_SUMMARY)
        u32Ret = _dlStockIterative(pcdp);
    else if (pcdp->cdp_u8Action == CLI_ACTION_DOWNLOAD_TRADE_DETAIL)
        u32Ret = _dlStockDetail(pcdp);
    else if (pcdp->cdp_u8Action == CLI_ACTION_DOWNLOAD_STOCK_INFO_INDEX)
        u32Ret = _dlStockIndex(pcdp);
    else
        u32Ret = JF_ERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamDownload(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_download_param_t * pcdp = (cli_download_param_t *)pParam;

    memset(pcdp, 0, sizeof(*pcdp));

    pcdp->cdp_u8Action = CLI_ACTION_DOWNLOAD_TRADE_SUMMARY;
    pcdp->cdp_pstrDataDir = tx_env_getVar(TX_ENV_VAR_DATA_PATH);
    pcdp->cdp_u8IterativeCount = 1;

    return u32Ret;
}

u32 parseDownload(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_download_param_t * pcdp = (cli_download_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv, "s:t:p:f:l:d:ieoh?")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 's':
            pcdp->cdp_pstrStock = (olchar_t *)optarg;
            break;
        case 't':
            u32Ret = jf_string_getU8FromString(
                (olchar_t *)optarg, ol_strlen((olchar_t *)optarg), &pcdp->cdp_u8IterativeCount);
            break;
        case 'p':
            u32Ret = jf_string_getS32FromString(optarg, ol_strlen(optarg), &pcdp->cdp_nSleep);
            if ((u32Ret == JF_ERR_NO_ERROR) && (pcdp->cdp_nSleep <= 0))
            {
                jf_clieng_reportInvalidOpt('p');
                u32Ret = JF_ERR_INVALID_PARAM;
            }
            break;
        case 'e':
            pcdp->cdp_u8Action = CLI_ACTION_DOWNLOAD_TRADE_DETAIL;
            break;
        case 'i':
            pcdp->cdp_u8Action = CLI_ACTION_DOWNLOAD_STOCK_INFO_INDEX;
            break;
        case 'd':
            pcdp->cdp_pstrDataDir = (olchar_t *)optarg;
            break;
        case 'f':
            pcdp->cdp_pstrStartDate = (olchar_t *)optarg;
            pcdp->cdp_pstrEndDate = (olchar_t *)optarg;
            break;
        case 'l':
            pcdp->cdp_pstrEndDate = (olchar_t *)optarg;
            break;
        case 'o':
            pcdp->cdp_bOverwrite = TRUE;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case '?':
        case 'h':
            pcdp->cdp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


