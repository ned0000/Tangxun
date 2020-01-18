/**
 *  @file model.c
 *
 *  @brief The model command implementation.
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

#include "damodel.h"
#include "clicmd.h"

/* --- private data/data structure section ------------------------------------------------------ */

static jf_clieng_caption_t ls_jccDaModelBrief[] =
{
    {"Id", 3},
    {"Name", 9},
    {"LongName", 65},
};

/* --- private routine section ------------------------------------------------------------------ */

static u32 _modelHelp(tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Model\n\
model [-m model-name] [-v] [-h]");
    jf_clieng_outputRawLine2("\
  -m: specify the model name to show model information.");
    jf_clieng_outputRawLine2("\
  -v: verbose.\n\
  -h: show this help information.\n\
  If no option is specified, all models are printed in brief mode.\n");

    return u32Ret;
}

static void _printOneDaModelBrief(int modelid, da_model_t * model)
{
    jf_clieng_caption_t * pcc = &ls_jccDaModelBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Id */
    ol_sprintf(strField, "%d", modelid);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* name */
    jf_clieng_appendBriefColumn(pcc, strInfo, model->dm_strName);
    pcc++;

    /* LongName */
    jf_clieng_appendBriefColumn(pcc, strInfo, model->dm_strLongName);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static u32 _printAllDaModelBrief(cli_model_param_t * pcmp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    int modelid = 1;
    da_model_t * pdm;

    jf_clieng_printDivider();

    jf_clieng_printHeader(
        ls_jccDaModelBrief,
        sizeof(ls_jccDaModelBrief) / sizeof(jf_clieng_caption_t));

    pdm = getFirstDaModel();
    while (pdm != NULL)
    {
        _printOneDaModelBrief(modelid, pdm);
        modelid ++;

        pdm = getNextDaModel(pdm);
    }
    jf_clieng_outputLine("");

    return u32Ret;
}

static u32 _printAllDaModel(cli_model_param_t * pcmp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pcmp->cmp_bVerbose)
    {
        ;
    }
    else
    {
        _printAllDaModelBrief(pcmp);
    }

    return u32Ret;
}

static u32 _printDaModel(cli_model_param_t * pcmp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_model_t * pdm;

    u32Ret = getDaModel(pcmp->cmp_pstrModelName, &pdm);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_clieng_printDivider();

        jf_clieng_printHeader(
            ls_jccDaModelBrief,
            sizeof(ls_jccDaModelBrief) / sizeof(jf_clieng_caption_t));

        _printOneDaModelBrief(1, pdm);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processModel(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_model_param_t * pcmp = (cli_model_param_t *)pParam;
    tx_cli_master_t * ptcm = (tx_cli_master_t *)pMaster;

    if (pcmp->cmp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _modelHelp(ptcm);
    else if (pcmp->cmp_u8Action == CLI_ACTION_MODEL_LIST_ALL)
    {
        u32Ret = _printAllDaModel(pcmp);
    }
    else if (pcmp->cmp_u8Action == CLI_ACTION_MODEL_LIST)
    {
        u32Ret = _printDaModel(pcmp);
    }
    else
        u32Ret = JF_ERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamModel(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_model_param_t * pcmp = (cli_model_param_t *)pParam;

    memset(pcmp, 0, sizeof(*pcmp));
    pcmp->cmp_u8Action = CLI_ACTION_MODEL_LIST_ALL;

    return u32Ret;
}

u32 parseModel(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_model_param_t * pcmp = (cli_model_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "m:hv?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'm':
            pcmp->cmp_pstrModelName = (olchar_t *)optarg;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case 'v':
            pcmp->cmp_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcmp->cmp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


