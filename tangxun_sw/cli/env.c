/**
 *  @file env.c
 *
 *  @brief The env command
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
#include "envvar.h"

/* --- private data/data structure section --------------------------------- */



/* --- private routine section---------------------------------------------- */
static u32 _envHelp(da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputRawLine2("\
Envirionment variable\n\
env [-l var] [-s var] [-c var]");
    cliengOutputRawLine2("\
  -l: list the specified env variable.\n\
  -s: set the value of the variable with format \"var=value\".\n\
  -c: clear the value of the variable.");
    cliengOutputLine("");

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 processEnv(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_env_param_t * pcep = (cli_env_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcep->cep_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _envHelp(pdm);
    else if (pcep->cep_u8Action == CLI_ACTION_ENV_LIST_ALL)
    {
        printEnvVarVerbose();
    }
    else if (pcep->cep_u8Action == CLI_ACTION_ENV_SET)
    {
        u32Ret = setEnvVar(pcep->cep_pstrData);
    }
    else if (pcep->cep_u8Action == CLI_ACTION_ENV_LIST)
    {
        u32Ret = printEnvVar(pcep->cep_pstrData);
    }
    else if (pcep->cep_u8Action == CLI_ACTION_ENV_CLEAR)
    {
        u32Ret = clearEnvVar(pcep->cep_pstrData);
    }
    else
        u32Ret = OLERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamEnv(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_env_param_t * pcep = (cli_env_param_t *)pParam;

    memset(pcep, 0, sizeof(*pcep));

    pcep->cep_u8Action = CLI_ACTION_MODEL_LIST_ALL;

    return u32Ret;
}

u32 parseEnv(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_env_param_t * pcep = (cli_env_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "s:l:c:h?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'l':
            pcep->cep_u8Action = CLI_ACTION_ENV_LIST;
            pcep->cep_pstrData = (olchar_t *)optarg;
            break;
        case 's':
            pcep->cep_u8Action = CLI_ACTION_ENV_SET;
            pcep->cep_pstrData = (olchar_t *)optarg;
            break;
        case 'c':
            pcep->cep_u8Action = CLI_ACTION_ENV_CLEAR;
            pcep->cep_pstrData = (olchar_t *)optarg;
            break;
        case ':':
            u32Ret = OLERR_MISSING_PARAM;
            break;
        case '?':
        case 'h':
            pcep->cep_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}


/*---------------------------------------------------------------------------*/


