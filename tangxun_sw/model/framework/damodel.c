/**
 *  @file damodel.c
 *
 *  @brief The model implementation
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
#include "jf_listhead.h"
#include "jf_limit.h"
#include "jf_listhead.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "jf_clieng.h"

#include "damodel.h"
#include "model_manager.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The directory includes library which contain model 
 */
#define MODEL_LIB_DIR "../lib/model"

static JF_LISTHEAD(ls_jlModel);

/* --- private routine section ------------------------------------------------------------------ */

static u32 _findModelByName(const olchar_t * name, da_model_t ** model)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pos;
    da_model_t * pdm;

    *model = NULL;

    jf_listhead_forEach(&ls_jlModel, pos)
    {
        pdm = jf_listhead_getEntry(pos, da_model_t, dm_jlList);
        if (ol_strncasecmp(pdm->dm_strName, name, ol_strlen(pdm->dm_strName)) == 0)
        {
            *model = pdm;
            break;
        }
    }

    if (*model == NULL)
        u32Ret = JF_ERR_NOT_FOUND;

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initDaModelFramework(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("init da model framework");

    u32Ret = addDaModel(&ls_jlModel, MODEL_LIB_DIR);

    return u32Ret;
}

u32 finiDaModelFramework(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = removeDaModel(&ls_jlModel);

    return u32Ret;
}

u32 getDaModel(const olchar_t * name, da_model_t ** ppModel)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_model_t * pdm = NULL;

    assert(name != NULL);

    u32Ret = _findModelByName(name, &pdm);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *ppModel = pdm;
    }

    return u32Ret;
}

da_model_t * getFirstDaModel(void)
{
    da_model_t * pdm = NULL;

    if (! jf_listhead_isEmpty(&ls_jlModel))
        pdm = jf_listhead_getEntry(ls_jlModel.jl_pjlNext, da_model_t, dm_jlList);

    return pdm;
}

da_model_t * getNextDaModel(da_model_t * pdm)
{
    da_model_t * pNext = NULL;

    if (! jf_listhead_isLast(&ls_jlModel, &pdm->dm_jlList))
        pNext = jf_listhead_getEntry(pdm->dm_jlList.jl_pjlNext, da_model_t, dm_jlList);

    return pNext;
}

/*------------------------------------------------------------------------------------------------*/


