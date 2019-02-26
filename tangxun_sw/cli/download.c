/**
 *  @file download.c
 *
 *  @brief The download command
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "bases.h"
#include "clicmd.h"
#include "stringparse.h"
#include "files.h"
#include "xmalloc.h"
#include "downloaddata.h"
#include "jiukun.h"
#include "stocklist.h"
#include "process.h"
#include "envvar.h"

/* --- private data/data structure section --------------------------------- */



/* --- private routine section---------------------------------------------- */
static u32 _downloadHelp(da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputRawLine2("\
Download data. Download trade summary of stocks in list and trade summary of\n\
SH, SZ index if no option is specified.\n\
download [-s stock] [-e] [-i] [-d dir] [-o]\n\
    [-f first-date] [-l last-date] [-t count] [-p sleep-second] ");
    cliengOutputRawLine2("\
  -s: download the specified stock. By default, download trade summary.\n\
  -e: download trade detail.\n\
  -i: download index.\n\
  -d: directory containing data.");
    cliengOutputRawLine2("\
  -t: iterative count.\n\
  -p: sleep certain minutes.\n\
  -o: overwrite the existing data file.");
    cliengOutputRawLine2("\
  -f: the first date with format \"yyyy-mm-dd\", used with '-e'.\n\
  -l: the last date with format \"yyyy-mm-dd\", used with '-e'.");
    cliengOutputLine("");

    return u32Ret;
}

static u32 _dlStock(cli_download_param_t * pcdp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    download_data_param_t ddp;

    if (pcdp->cdp_pstrStock == NULL)
    {
        memset(&ddp, 0, sizeof(ddp));

        ddp.ddp_pstrDataDir = pcdp->cdp_pstrDataDir;
        ddp.ddp_bOverwrite = pcdp->cdp_bOverwrite;

        downloadStockInfoIndex(&ddp);
    }

    memset(&ddp, 0, sizeof(ddp));

    ddp.ddp_pstrStock = pcdp->cdp_pstrStock;
    ddp.ddp_pstrDataDir = pcdp->cdp_pstrDataDir;
    ddp.ddp_bOverwrite = pcdp->cdp_bOverwrite;
    ddp.ddp_bTradeSummary = TRUE;

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = downloadData(&ddp);

    cliengOutputLine("");

    return u32Ret;
}

static void _dlSleepSecond(cli_download_param_t * pcdp)
{
    if (pcdp->cdp_nSleep != 0)
        sleep(pcdp->cdp_nSleep * 60);
}

static u32 _dlStockIterative(cli_download_param_t * pcdp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t count = 0;

    _dlSleepSecond(pcdp);

    if (pcdp->cdp_pstrStock != NULL)
        return _dlStock(pcdp);

    while ((u32Ret == OLERR_NO_ERROR) && (count < pcdp->cdp_u8IterativeCount))
    {
        cliengOutputLine("\nDownload for the %dth time", count + 1);
        u32Ret = _dlStock(pcdp);

        count ++;
    }
    cliengOutputLine("");

    return u32Ret;
}

static u32 _dlStockDetail(cli_download_param_t * pcdp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    download_data_param_t ddp;

    if (pcdp->cdp_pstrStock == NULL)
        return OLERR_INVALID_PARAM;

    if ((pcdp->cdp_pstrStartDate == NULL) || (pcdp->cdp_pstrEndDate == NULL))
        return OLERR_INVALID_PARAM;

    memset(&ddp, 0, sizeof(ddp));

    ddp.ddp_pstrStartDate = pcdp->cdp_pstrStartDate;
    ddp.ddp_pstrEndDate = pcdp->cdp_pstrEndDate;
    ddp.ddp_pstrStock = pcdp->cdp_pstrStock;
    ddp.ddp_pstrDataDir = pcdp->cdp_pstrDataDir;
    ddp.ddp_bOverwrite = pcdp->cdp_bOverwrite;
    ddp.ddp_bTradeDetail = TRUE;

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = downloadData(&ddp);

    return u32Ret;
}

static u32 _dlStockIndex(cli_download_param_t * pcdp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    download_data_param_t ddp;

    memset(&ddp, 0, sizeof(ddp));

    ddp.ddp_pstrDataDir = pcdp->cdp_pstrDataDir;
    ddp.ddp_bOverwrite = pcdp->cdp_bOverwrite;

    u32Ret = downloadStockInfoIndex(&ddp);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 processDownload(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_download_param_t * pcdp = (cli_download_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcdp->cdp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _downloadHelp(pdm);
    else if (*getEnvVar(ENV_VAR_DATA_PATH) == '\0')
    {
        cliengOutputLine("Data path is not set.");
        u32Ret = OLERR_NOT_READY;
    }
    else if (pcdp->cdp_u8Action == CLI_ACTION_DOWNLOAD_TRADE_SUMMARY)
        u32Ret = _dlStockIterative(pcdp);
    else if (pcdp->cdp_u8Action == CLI_ACTION_DOWNLOAD_TRADE_DETAIL)
        u32Ret = _dlStockDetail(pcdp);
    else if (pcdp->cdp_u8Action == CLI_ACTION_DOWNLOAD_STOCK_INFO_INDEX)
        u32Ret = _dlStockIndex(pcdp);
    else
        u32Ret = OLERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamDownload(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_download_param_t * pcdp = (cli_download_param_t *)pParam;

    memset(pcdp, 0, sizeof(*pcdp));

    pcdp->cdp_u8Action = CLI_ACTION_DOWNLOAD_TRADE_SUMMARY;
    pcdp->cdp_pstrDataDir = getEnvVar(ENV_VAR_DATA_PATH);
    pcdp->cdp_u8IterativeCount = 1;

    return u32Ret;
}

u32 parseDownload(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_download_param_t * pcdp = (cli_download_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "s:t:p:f:l:d:ieoh?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 's':
            pcdp->cdp_pstrStock = (olchar_t *)optarg;
            break;
        case 't':
            getU8FromString(
                (olchar_t *)optarg, ol_strlen((olchar_t *)optarg), &pcdp->cdp_u8IterativeCount);
            break;
        case 'p':
            u32Ret = getS32FromString(
                optarg, ol_strlen(optarg), &pcdp->cdp_nSleep);
            if ((u32Ret == OLERR_NO_ERROR) && (pcdp->cdp_nSleep <= 0))
            {
                cliengReportInvalidOpt('p');
                u32Ret = OLERR_INVALID_PARAM;
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
            u32Ret = OLERR_MISSING_PARAM;
            break;
        case '?':
        case 'h':
            pcdp->cdp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


