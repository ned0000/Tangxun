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
#include "olbasic.h"
#include "ollimit.h"
#include "bases.h"
#include "clicmd.h"
#include "stringparse.h"
#include "files.h"
#include "xmalloc.h"
#include "damodel.h"

/* --- private data/data structure section --------------------------------- */
static clieng_caption_t ls_ccDaModelBrief[] =
{
    {"Id", 3},
    {"Name", 9},
    {"LongName", 65},
};


/* --- private routine section---------------------------------------------- */
static u32 _modelHelp(da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputRawLine2("\
Model\n\
model [-m model] [-v] [-h]");
    cliengOutputRawLine2("  -m: specify the model to show modol information.");
    cliengOutputRawLine2("\
  -v: verbose.\n\
  -h: show this help information.\n\
  If no option is specified, all models are printed in brief mode.\n");

    return u32Ret;
}

static void _printOneDaModelBrief(da_model_t * model)
{
    clieng_caption_t * pcc = &ls_ccDaModelBrief[0];
    olchar_t strInfo[MAX_OUTPUT_LINE_LEN], strField[MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Id */
    ol_sprintf(strField, "%d", model->dm_dmiId);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* name */
    cliengAppendBriefColumn(pcc, strInfo, model->dm_strName);
    pcc++;

    /* LongName */
    cliengAppendBriefColumn(pcc, strInfo, model->dm_strLongName);
    pcc++;

    cliengOutputLine(strInfo);
}

static u32 _printAllDaModelBrief(cli_model_param_t * pcmp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    da_model_id_t modelid;
    da_model_t * pdm;

    cliengPrintDivider();

    cliengPrintHeader(
        ls_ccDaModelBrief,
        sizeof(ls_ccDaModelBrief) / sizeof(clieng_caption_t));

    for (modelid = 0; modelid < DA_MODEL_MAX_ID; modelid ++)
    {
        u32Ret = getDaModel(modelid, &pdm);
        if (u32Ret == OLERR_NO_ERROR)
        {
            _printOneDaModelBrief(pdm);
        }
    }
    cliengOutputLine("");

    return u32Ret;
}

static u32 _printAllDaModel(cli_model_param_t * pcmp)
{
    u32 u32Ret = OLERR_NO_ERROR;

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
    u32 u32Ret = OLERR_NO_ERROR;
    da_model_t * pdm;

    u32Ret = getDaModel((da_model_id_t)pcmp->cmp_u32ModelId, &pdm);
    if (u32Ret == OLERR_NO_ERROR)
    {
        cliengPrintDivider();

        cliengPrintHeader(
            ls_ccDaModelBrief,
            sizeof(ls_ccDaModelBrief) / sizeof(clieng_caption_t));

        _printOneDaModelBrief(pdm);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 processModel(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
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
        u32Ret = OLERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamModel(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_model_param_t * pcmp = (cli_model_param_t *)pParam;

    memset(pcmp, 0, sizeof(*pcmp));
    pcmp->cmp_u8Action = CLI_ACTION_MODEL_LIST_ALL;

    return u32Ret;
}

u32 parseModel(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_model_param_t * pcmp = (cli_model_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "m:hv?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'm':
            pcmp->cmp_u8Action = CLI_ACTION_MODEL_LIST;
            u32Ret = getU32FromString(
                optarg, ol_strlen(optarg), &pcmp->cmp_u32ModelId);
            if (u32Ret != OLERR_NO_ERROR)
            {
                cliengReportInvalidOpt('m');
                u32Ret = OLERR_INVALID_PARAM;
            }
            break;
        case ':':
            u32Ret = OLERR_MISSING_PARAM;
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


