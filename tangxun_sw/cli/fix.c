/**
 *  @file fix.c
 *
 *  @brief The fix command
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
#include "bases.h"
#include "clicmd.h"
#include "stringparse.h"
#include "files.h"
#include "xmalloc.h"
#include "fixdata.h"

/* --- private data/data structure section --------------------------------- */


/* --- private routine section---------------------------------------------- */
static u32 _fixHelp(da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputLine("fix");
    cliengOutputLine(
        "fix [-f file] [-o] [-v]");
    cliengOutputLine("  -f: fix data from file.");
    cliengOutputLine("  -o: overwrite the original file.");
    cliengOutputLine("  -v: verbose.");

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 processFix(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_fix_param_t * pcfp = (cli_fix_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;
    fix_param_t fixp;

    if (pcfp->cfp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _fixHelp(pdm);
    else if (pcfp->cfp_u8Action == CLI_ACTION_FIX_FILE)
    {
        memset(&fixp, 0, sizeof(fixp));
        fixp.fp_bOverwrite = pcfp->cfp_bOverwrite;
        u32Ret = fixDataFile(pcfp->cfp_pstrData, &fixp);
    }
    else
        u32Ret = OLERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamFix(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_fix_param_t * pcfp = (cli_fix_param_t *)pParam;

    memset(pcfp, 0, sizeof(*pcfp));


    return u32Ret;
}

u32 parseFix(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_fix_param_t * pcfp = (cli_fix_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "f:ovh?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'f':
            pcfp->cfp_u8Action = CLI_ACTION_FIX_FILE;
            pcfp->cfp_pstrData = (olchar_t *)optarg;
            break;
        case 'o':
            pcfp->cfp_bOverwrite = TRUE;
            break;
        case 'v':
            pcfp->cfp_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcfp->cfp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


