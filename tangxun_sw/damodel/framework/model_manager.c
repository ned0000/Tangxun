/**
 *  @file
 *
 *  @brief The template C file
 *
 *  @author Min Zhang
 *
 *  @note
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_mem.h"
#include "jf_dir.h"

#include "model_manager.h"
#include "damodel.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The prefix string for library contains model
 */
#define MODEL_LIB_PREFIX  "libdamodel_"

/** The entry name of the library, MUST be implemented
 */
#define MODEL_LIB_ENTRY_NAME  "fillDaModel"

typedef u32 (* fnFillDaModel_t)(da_model_t * pdm);

/* --- private routine section ------------------------------------------------------------------ */

static u32 _destroyDaModel(da_model_t ** ppdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_model_t * pdm = *ppdm;

    if (pdm->dm_pjdLib != NULL)
        jf_dynlib_unload(&pdm->dm_pjdLib);
    
    jf_mem_free((void **)ppdm);

    return u32Ret;
}

static u32 _createDaModel(da_model_t ** ppdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_model_t * pdm = NULL;

    u32Ret = jf_mem_calloc((void **)&pdm, sizeof(*pdm));
    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppdm = pdm;
    else if (pdm != NULL)
        _destroyDaModel(&pdm);
    
    return u32Ret;
}

static u32 _checkDaModelField(da_model_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pdm->dm_strName[0] == '\0')
        u32Ret = JF_ERR_INVALID_NAME;

    if ((pdm->dm_fnInitModel == NULL) || (pdm->dm_fnFiniModel == NULL) ||
        (pdm->dm_fnCanBeTraded == NULL) || (pdm->dm_fnTrade == NULL))
        u32Ret = JF_ERR_INVALID_CALLBACK_FUNCTION;

    return u32Ret;
}

static u32 _fnHandleModelLibFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pjl = (jf_listhead_t *)pArg;
    olchar_t strLibName[128];
    da_model_t * pdm = NULL;
    jf_dynlib_t * pDynlib = NULL;
    fnFillDaModel_t fnFillDaModel;
    
    jf_file_getFileName(strLibName, sizeof(strLibName), pstrFullpath);

    if (ol_strncmp(strLibName, MODEL_LIB_PREFIX, strlen(MODEL_LIB_PREFIX)) != 0)
        return u32Ret;
    
    u32Ret = _createDaModel(&pdm);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_dynlib_load(pstrFullpath, &pDynlib);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_dynlib_getSymbolAddress(
            pDynlib, MODEL_LIB_ENTRY_NAME, (void **)&fnFillDaModel);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = fnFillDaModel(pdm);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _checkDaModelField(pdm);
    }
    
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_listhead_add(pjl, &pdm->dm_jlList);
    }
    else if (pdm != NULL)
    {
        jf_logger_logErrMsg(u32Ret, "failed to load model");
        _destroyDaModel(&pdm);
    }

    return u32Ret;
}

static u32 _parseModelDir(jf_listhead_t * pjl, const char * pstrLibDir)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_dir_parse(pstrLibDir, _fnHandleModelLibFile, pjl);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 addDaModel(jf_listhead_t * pjl, const char * pstrLibDir)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _parseModelDir(pjl, pstrLibDir);

    return u32Ret;
}

u32 removeDaModel(jf_listhead_t * pjl)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    da_model_t * pdm;

    while (! jf_listhead_isEmpty(pjl))
    {
        pdm = jf_listhead_getEntry(pjl->jl_pjlNext, da_model_t, dm_jlList);

        jf_listhead_del(&pdm->dm_jlList);

        pdm->dm_fnFiniModel(pdm);

        _destroyDaModel(&pdm);
    }

    return u32Ret;
}


/*------------------------------------------------------------------------------------------------*/

