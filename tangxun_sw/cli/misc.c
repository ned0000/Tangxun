/**
 *  @file misc.c
 *
 *  @brief The misc command
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
#include "clicmd.h"
#include "stringparse.h"
#include "files.h"
#include "xmalloc.h"
#include "parsedata.h"
#include "regression.h"
#include "datastat.h"
#include "damodel.h"
#include "stocklist.h"
#include "envvar.h"

/* --- private data/data structure section --------------------------------- */

#define DEF_EXDR_FILE_NAME  "ParseData.txt"




/* --- private routine section---------------------------------------------- */
static u32 _miscHelp(da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputLine("misc data");
    cliengOutputLine(
        "misc [-e]");
    cliengOutputLine("  -e: delete ExDR file.");
    cliengOutputLine("  -v: verbose.");

    return u32Ret;
}

static u32 _createExdrFile(cli_misc_param_t * pcmp, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strFullname[MAX_PATH_LEN];
    stock_info_t * stockinfo;

    stockinfo = getFirstStockInfo();
    while ((stockinfo != NULL) && (u32Ret == OLERR_NO_ERROR))
    {
        if (u32Ret == OLERR_NO_ERROR)
        {
            memset(strFullname, 0, MAX_PATH_LEN);
            ol_snprintf(
                strFullname, MAX_PATH_LEN - 1, "%s%c%s%c%s",
                getEnvVar(ENV_VAR_DATA_PATH),
                PATH_SEPARATOR, stockinfo->si_strCode,
                PATH_SEPARATOR, DEF_EXDR_FILE_NAME);

            u32Ret = removeFile(strFullname);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            stockinfo = getNextStockInfo(stockinfo);
        }
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 processMisc(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_misc_param_t * pcmp = (cli_misc_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcmp->cmp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _miscHelp(pdm);
    else if (pcmp->cmp_u8Action == CLI_ACTION_CREATE_EXDR_FILE)
        u32Ret = _createExdrFile(pcmp, pdm);
    else
        u32Ret = OLERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamMisc(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_misc_param_t * pcmp = (cli_misc_param_t *)pParam;

    memset(pcmp, 0, sizeof(cli_misc_param_t));


    return u32Ret;
}

u32 parseMisc(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_misc_param_t * pcmp = (cli_misc_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "ehv?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'e':
            pcmp->cmp_u8Action = CLI_ACTION_CREATE_EXDR_FILE;
            break;
        case 'v':
            pcmp->cmp_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcmp->cmp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}



/*---------------------------------------------------------------------------*/

