/**
 *  @file model_common.c
 *
 *  @brief Implementation file for model common routines
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

#include "model_common.h"
#include "damodel.h"

/* --- private data/data structure section ------------------------------------------------------ */



/* --- private routine section ------------------------------------------------------------------ */



/* --- public routine section ------------------------------------------------------------------- */

u32 destroyDaModel(da_model_t ** ppdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_model_t * pdm = *ppdm;

    if (pdm->dm_pjdLib != NULL)
        jf_dynlib_unload(&pdm->dm_pjdLib);
    
    jf_mem_free((void **)ppdm);

    return u32Ret;
}

u32 createDaModel(da_model_t ** ppdm)
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
        destroyDaModel(&pdm);
    
    return u32Ret;
}

u32 checkDaModelField(da_model_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pdm->dm_strName[0] == '\0')
        u32Ret = JF_ERR_INVALID_NAME;

    if ((pdm->dm_fnInitModel == NULL) || (pdm->dm_fnFiniModel == NULL) ||
        (pdm->dm_fnCanBeTraded == NULL) || (pdm->dm_fnTrade == NULL))
        u32Ret = JF_ERR_INVALID_CALLBACK_FUNCTION;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

