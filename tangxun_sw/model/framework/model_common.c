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

#include "tx_model.h"
#include "model_common.h"

/* --- private data/data structure section ------------------------------------------------------ */



/* --- private routine section ------------------------------------------------------------------ */



/* --- public routine section ------------------------------------------------------------------- */

u32 destroyDaModel(tx_model_t ** pptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_model_t * ptm = *pptm;

    if (ptm->tm_pjdLib != NULL)
        jf_dynlib_unload(&ptm->tm_pjdLib);
    
    jf_mem_free((void **)pptm);

    return u32Ret;
}

u32 createDaModel(tx_model_t ** pptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_model_t * ptm = NULL;

    u32Ret = jf_mem_calloc((void **)&ptm, sizeof(*ptm));
    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *pptm = ptm;
    else if (ptm != NULL)
        destroyDaModel(&ptm);
    
    return u32Ret;
}

u32 checkDaModelField(tx_model_t * ptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ptm->tm_strName[0] == '\0')
        u32Ret = JF_ERR_INVALID_NAME;

    if ((ptm->tm_fnInitModel == NULL) || (ptm->tm_fnFiniModel == NULL) ||
        (ptm->tm_fnCanBeTraded == NULL) || (ptm->tm_fnTrade == NULL))
        u32Ret = JF_ERR_INVALID_CALLBACK_FUNCTION;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

