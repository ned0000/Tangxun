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

/* --- private data/data structure section ------------------------------------------------------ */

static JF_LISTHEAD(ls_jlModel);

/* --- private routine section ------------------------------------------------------------------ */
static u32 _findModelByName(olchar_t * name, da_model_t ** model)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pos;
    da_model_t * pmp;

    *model = NULL;

    jf_listhead_forEach(&ls_jlModel, pos)
    {
        pmp = jf_listhead_getEntry(pos, da_model_t, dm_jlList);
        if (strncmp(pmp->dm_strName, name, ol_strlen(pmp->dm_strName)) == 0)
        {
            *model = pmp;
            break;
        }
    }

    if (*model == NULL)
        u32Ret = JF_ERR_NOT_FOUND;

    return u32Ret;
}

static u32 _findModel(da_model_id_t id, da_model_t ** model)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pos;
    da_model_t * pmp;

    *model = NULL;

    jf_listhead_forEach(&ls_jlModel, pos)
    {
        pmp = jf_listhead_getEntry(pos, da_model_t, dm_jlList);
        if (pmp->dm_dmiId == id)
        {
            *model = pmp;
            break;
        }
    }

    if (*model == NULL)
        u32Ret = JF_ERR_NOT_FOUND;

    return u32Ret;
}


/* --- public routine section ------------------------------------------------------------------- */
u32 initDaModel(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pos;
    da_model_t * pmp;

    if (! jf_listhead_isEmpty(&ls_jlModel))
        return u32Ret;

    jf_logger_logInfoMsg("init da model");

    u32Ret = addDaModelRoi(&ls_jlModel);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_listhead_forEach(&ls_jlModel, pos)
        {
            pmp = jf_listhead_getEntry(pos, da_model_t, dm_jlList);

            pmp->dm_fnInitModel(pmp);
        }
    }

    return u32Ret;
}

u32 finiDaModel(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_model_t * pmp;

    while (! jf_listhead_isEmpty(&ls_jlModel))
    {
        pmp = jf_listhead_getEntry(ls_jlModel.jl_pjlNext, da_model_t, dm_jlList);

        pmp->dm_fnFiniModel(pmp);
    }

    return u32Ret;
}

u32 getDaModelByName(olchar_t * name, da_model_t ** model)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_model_t * pdm = NULL;

    assert(name != NULL);

    u32Ret = _findModelByName(name, &pdm);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *model = pdm;
    }

    return u32Ret;
}

u32 getDaModel(da_model_id_t id, da_model_t ** model)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_model_t * pdm = NULL;

    u32Ret = _findModel(id, &pdm);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *model = pdm;
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


