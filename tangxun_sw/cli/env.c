/**
 *  @file env.c
 *
 *  @brief The env command implementation.
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

#include "tx_env.h"
#include "clicmd.h"

/* --- private data/data structure section ------------------------------------------------------ */

static jf_clieng_caption_t ls_ccEnvVarVerbose[] =
{
    {TX_ENV_VAR_DATA_PATH, JF_CLIENG_CAP_FULL_LINE},
    {TX_ENV_VAR_DAYS_STOCK_POOL, JF_CLIENG_CAP_HALF_LINE}, {TX_ENV_VAR_MAX_STOCK_IN_POOL, JF_CLIENG_CAP_HALF_LINE},
};


/* --- private routine section ------------------------------------------------------------------ */

static u32 _envHelp(tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Env command.\n\
env [-l var] [-s var] [-c var]");
    jf_clieng_outputRawLine2("\
  -l: list the specified env variable.\n\
  -s: set the value of the variable with format \"var=value\".\n\
  -c: clear the value of the variable.");
    jf_clieng_outputLine("");

    return u32Ret;
}

static void _printEnvVarVerbose(void)
{
    jf_clieng_caption_t * pcc = &ls_ccEnvVarVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    /* DataPath */
    jf_clieng_printOneFullLine(pcc, tx_env_getVar(TX_ENV_VAR_DATA_PATH)); 
    pcc += 1;

    /* DaysForStockInPool */
    ol_sprintf(strLeft, "%d", tx_env_getVarDaysStockPool());
    ol_sprintf(strRight, "%d", tx_env_getVarMaxStockInPool());
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight); 
    pcc += 2;

    jf_clieng_outputLine("");
}

static u32 _printEnvVar(olchar_t * name)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (strcasecmp(name, TX_ENV_VAR_DATA_PATH) == 0)
    {
        jf_clieng_outputLine("%s: %s\n", TX_ENV_VAR_DATA_PATH, tx_env_getVar(TX_ENV_VAR_DATA_PATH));
    }
    else if (strcasecmp(name, TX_ENV_VAR_DAYS_STOCK_POOL) == 0)
    {
        jf_clieng_outputLine("%s: %d\n", TX_ENV_VAR_DAYS_STOCK_POOL, tx_env_getVarDaysStockPool());
    }
    else if (strcasecmp(name, TX_ENV_VAR_MAX_STOCK_IN_POOL) == 0)
    {
        jf_clieng_outputLine(
            "%s: %d\n", TX_ENV_VAR_MAX_STOCK_IN_POOL, tx_env_getVarMaxStockInPool());
    }
    else
    {
        u32Ret = JF_ERR_NOT_FOUND;
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processEnv(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_env_param_t * pcep = (cli_env_param_t *)pParam;
    tx_cli_master_t * ptcm = (tx_cli_master_t *)pMaster;

    if (pcep->cep_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _envHelp(ptcm);
    else if (pcep->cep_u8Action == CLI_ACTION_ENV_LIST_ALL)
        _printEnvVarVerbose();
    else if (pcep->cep_u8Action == CLI_ACTION_ENV_SET)
        u32Ret = tx_env_setVar(pcep->cep_pstrData);
    else if (pcep->cep_u8Action == CLI_ACTION_ENV_LIST)
        u32Ret = _printEnvVar(pcep->cep_pstrData);
    else if (pcep->cep_u8Action == CLI_ACTION_ENV_CLEAR)
        u32Ret = tx_env_clearVar(pcep->cep_pstrData);
    else
        u32Ret = JF_ERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamEnv(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_env_param_t * pcep = (cli_env_param_t *)pParam;

    memset(pcep, 0, sizeof(*pcep));

    pcep->cep_u8Action = CLI_ACTION_MODEL_LIST_ALL;

    return u32Ret;
}

u32 parseEnv(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_env_param_t * pcep = (cli_env_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv, "s:l:c:h?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
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
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case '?':
        case 'h':
            pcep->cep_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}


/*------------------------------------------------------------------------------------------------*/


