/**
 *  @file fix.c
 *
 *  @brief The fix command implementation.
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
#include "jf_listhead.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"

#include "tx_fixdata.h"

#include "clicmd.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */
static u32 _fixHelp(tx_cli_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputLine("Fix data");
    jf_clieng_outputLine("\
fix [-f file] [-o] [-v]");
    jf_clieng_outputRawLine("\
  -f: fix data from file. The fixed data will be writtten to the new file if");
    jf_clieng_outputLine("\
      overwrite option is not specified. \".fix\" is appended to the file name");
    jf_clieng_outputLine("\
      as the name of the new file.");
    jf_clieng_outputLine("\
  -o: overwrite the original file.");
    jf_clieng_outputLine("\
  -v: verbose.");

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */
u32 processFix(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_fix_param_t * pcfp = (cli_fix_param_t *)pParam;
    tx_cli_master_t * pdm = (tx_cli_master_t *)pMaster;
    fix_param_t fixp;
    fix_result_t fixresult;

    if (pcfp->cfp_u8Action == CLI_ACTION_SHOW_HELP)
    {
        u32Ret = _fixHelp(pdm);
    }
    else if (pcfp->cfp_u8Action == CLI_ACTION_FIX_FILE)
    {
        memset(&fixp, 0, sizeof(fixp));
        fixp.fp_bOverwrite = pcfp->cfp_bOverwrite;

        u32Ret = fixDataFile(pcfp->cfp_pstrData, &fixp, &fixresult);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (fixresult.fr_nDeletedLine != 0)
                jf_clieng_outputLine(
                    "Delete %d lines from data file", fixresult.fr_nDeletedLine);
        }
        else
        {
            jf_clieng_outputLine(
                "Cannot fix data file %s, correct the error by hand", pcfp->cfp_pstrData);
        }
    }
    else
    {
        u32Ret = JF_ERR_MISSING_PARAM;
    }

    return u32Ret;
}

u32 setDefaultParamFix(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_fix_param_t * pcfp = (cli_fix_param_t *)pParam;

    memset(pcfp, 0, sizeof(*pcfp));


    return u32Ret;
}

u32 parseFix(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_fix_param_t * pcfp = (cli_fix_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv, "f:ovh?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
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
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


