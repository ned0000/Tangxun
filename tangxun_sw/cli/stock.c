/**
 *  @file stock.c
 *
 *  @brief The stock command
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <math.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <stdlib.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "bases.h"
#include "clicmd.h"
#include "stringparse.h"
#include "files.h"
#include "xmalloc.h"
#include "stocklist.h"
#include "indicator.h"
#include "statarbitrage.h"
#include "jiukun.h"
#include "damethod.h"
#include "envvar.h"

/* --- private data/data structure section --------------------------------- */
static clieng_caption_t ls_ccStockInfoAdditionalVerbose[] =
{
    {"CorrelationWithIndex", CLIENG_CAP_HALF_LINE}, {"CorrelationWithSmeIndex", CLIENG_CAP_HALF_LINE},
};

/* --- private routine section---------------------------------------------- */
static u32 _stockHelp(da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputRawLine2("\
stock\n\
stock [-s stock] [-i] [-v]");
    cliengOutputRawLine2("\
  -s: list the specified stock.\n\
  -i: show industry information.");
    cliengOutputRawLine2("\
  -v: verbose.");
    cliengOutputLine("");

    return u32Ret;
}

static void _printStockInfoAdditionalVerbose(stock_info_t * info)
{
    clieng_caption_t * pcc = &ls_ccStockInfoAdditionalVerbose[0];
    olchar_t strLeft[MAX_OUTPUT_LINE_LEN], strRight[MAX_OUTPUT_LINE_LEN];

    cliengPrintDivider();

    /*CorrelationWithIndex*/
    ol_sprintf(strLeft, "%.2f", getCorrelationWithIndex(info));
    ol_sprintf(strRight, "%.2f", getCorrelationWithSmeIndex(info));
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    cliengOutputLine("");
}

static u32 _printStockVerbose(cli_stock_param_t * pcsp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    stock_info_t * info;

    u32Ret = getStockInfo(pcsp->csp_pstrStock, &info);
    if (u32Ret == OLERR_NO_ERROR)
    {
        printStockInfoVerbose(info);
        _printStockInfoAdditionalVerbose(info);
    }

    return u32Ret;
}

static u32 _printIndustryInfo(cli_stock_param_t * pcsp)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (pcsp->csp_bVerbose)
        printIndustryInfoVerbose();
    else
        printIndustryInfoBrief();

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 processStock(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_stock_param_t * pcsp = (cli_stock_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcsp->csp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _stockHelp(pdm);
    else if (pcsp->csp_u8Action == CLI_ACTION_LIST_INDUSTRY)
        u32Ret = _printIndustryInfo(pcsp);
    else if (*getEnvVar(ENV_VAR_DATA_PATH) == '\0')
    {
        cliengOutputLine("Data path is not set.");
        u32Ret = OLERR_NOT_READY;
    }
    else if (pcsp->csp_u8Action == CLI_ACTION_LIST_STOCK)
        u32Ret = _printStockVerbose(pcsp);
    else
        u32Ret = OLERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamStock(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_stock_param_t * pcsp = (cli_stock_param_t *)pParam;

    memset(pcsp, 0, sizeof(*pcsp));

    pcsp->csp_u8Action = CLI_ACTION_LIST_STOCK;

    return u32Ret;
}

u32 parseStock(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_stock_param_t * pcsp = (cli_stock_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "s:ivh?")) != -1) && (u32Ret == OLERR_NO_ERROR))
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
            u32Ret = OLERR_MISSING_PARAM;
            break;
        case '?':
        case 'h':
            pcsp->csp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


