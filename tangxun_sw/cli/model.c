/**
 *  @file model.c
 *
 *  @brief The model command
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
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_listhead.h"
#include "clicmd.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "damodel.h"

/* --- private data/data structure section --------------------------------- */
static jf_clieng_caption_t ls_ccDaModelBrief[] =
{
    {"Id", 3},
    {"Name", 9},
    {"LongName", 65},
};


/* --- private routine section---------------------------------------------- */
static u32 _modelHelp(da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Model\n\
model [-m model] [-v] [-h]");
    jf_clieng_outputRawLine2("  -m: specify the model to show modol information.");
    jf_clieng_outputRawLine2("\
  -v: verbose.\n\
  -h: show this help information.\n\
  If no option is specified, all models are printed in brief mode.\n");

    return u32Ret;
}

static void _printOneDaModelBrief(da_model_t * model)
{
    jf_clieng_caption_t * pcc = &ls_ccDaModelBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Id */
    ol_sprintf(strField, "%d", model->dm_dmiId);
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
    da_model_id_t modelid;
    da_model_t * pdm;

    jf_clieng_printDivider();

    jf_clieng_printHeader(
        ls_ccDaModelBrief,
        sizeof(ls_ccDaModelBrief) / sizeof(jf_clieng_caption_t));

    for (modelid = 0; modelid < DA_MODEL_MAX_ID; modelid ++)
    {
        u32Ret = getDaModel(modelid, &pdm);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            _printOneDaModelBrief(pdm);
        }
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

    u32Ret = getDaModel((da_model_id_t)pcmp->cmp_u32ModelId, &pdm);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_clieng_printDivider();

        jf_clieng_printHeader(
            ls_ccDaModelBrief,
            sizeof(ls_ccDaModelBrief) / sizeof(jf_clieng_caption_t));

        _printOneDaModelBrief(pdm);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 processModel(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_model_param_t * pcmp = (cli_model_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcmp->cmp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _modelHelp(pdm);
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
            pcmp->cmp_u8Action = CLI_ACTION_MODEL_LIST;
            u32Ret = jf_string_getU32FromString(
                optarg, ol_strlen(optarg), &pcmp->cmp_u32ModelId);
            if (u32Ret != JF_ERR_NO_ERROR)
            {
                jf_clieng_reportInvalidOpt('m');
                u32Ret = JF_ERR_INVALID_PARAM;
            }
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

/*---------------------------------------------------------------------------*/


