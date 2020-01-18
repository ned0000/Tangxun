/**
 *  @file model_lib.c
 *
 *  @brief Implementation file for model library file
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

#include "model_lib.h"
#include "damodel.h"
#include "model_common.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The prefix string for model library file
 */
#define MODEL_LIB_FILE_PREFIX  "libdamodel_"

/** the file extension of the model library file
 */
#define MODEL_LIB_FILE_EXT     ".so"

/** The entry name of the library, MUST be implemented
 */
#define MODEL_LIB_ENTRY_NAME  "fillDaModel"

typedef u32 (* fnFillDaModel_t)(da_model_t * pdm);

/* --- private routine section ------------------------------------------------------------------ */

static u32 _handleModelLibFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, jf_listhead_t * pjl)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_model_t * pdm = NULL;
    jf_dynlib_t * pDynlib = NULL;
    fnFillDaModel_t fnFillDaModel;
    
    u32Ret = createDaModel(&pdm);
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
        u32Ret = checkDaModelField(pdm);
    }
    
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_listhead_add(pjl, &pdm->dm_jlList);
    }
    else if (pdm != NULL)
    {
        jf_logger_logErrMsg(u32Ret, "failed to load model");
        destroyDaModel(&pdm);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

boolean_t isDaModelLibFile(const olchar_t * pstrName)
{
    return jf_file_isTypedFile(pstrName, MODEL_LIB_FILE_PREFIX, MODEL_LIB_FILE_EXT);
}

u32 handleDaModelLibFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, jf_listhead_t * pjl)
{
    return _handleModelLibFile(pstrFullpath, pStat, pjl);
}

/*------------------------------------------------------------------------------------------------*/

