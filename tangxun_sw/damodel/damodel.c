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

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "bases.h"
#include "damodel.h"
#include "stringparse.h"
#include "files.h"
#include "xmalloc.h"
#include "clieng.h"

/* --- private data/data structure section --------------------------------- */

static LIST_HEAD(ls_lhModel);

/* --- private routine section---------------------------------------------- */
static u32 _findModelByName(olchar_t * name, da_model_t ** model)
{
    u32 u32Ret = OLERR_NO_ERROR;
    list_head_t * pos;
    da_model_t * pmp;

    *model = NULL;

    listForEach(&ls_lhModel, pos)
    {
        pmp = listEntry(pos, da_model_t, dm_lhList);
        if (strncmp(pmp->dm_strName, name, ol_strlen(pmp->dm_strName)) == 0)
        {
            *model = pmp;
            break;
        }
    }

    if (*model == NULL)
        u32Ret = OLERR_NOT_FOUND;

    return u32Ret;
}

static u32 _findModel(da_model_id_t id, da_model_t ** model)
{
    u32 u32Ret = OLERR_NO_ERROR;
    list_head_t * pos;
    da_model_t * pmp;

    *model = NULL;

    listForEach(&ls_lhModel, pos)
    {
        pmp = listEntry(pos, da_model_t, dm_lhList);
        if (pmp->dm_dmiId == id)
        {
            *model = pmp;
            break;
        }
    }

    if (*model == NULL)
        u32Ret = OLERR_NOT_FOUND;

    return u32Ret;
}


/* --- public routine section ---------------------------------------------- */
u32 initDaModel(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    list_head_t * pos;
    da_model_t * pmp;

    if (! listIsEmpty(&ls_lhModel))
        return u32Ret;

    logInfoMsg("init da model");

    u32Ret = addDaModelRoi(&ls_lhModel);

    if (u32Ret == OLERR_NO_ERROR)
    {
        listForEach(&ls_lhModel, pos)
        {
            pmp = listEntry(pos, da_model_t, dm_lhList);

            pmp->dm_fnInitModel(pmp);
        }
    }

    return u32Ret;
}

u32 finiDaModel(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    da_model_t * pmp;

    while (! listIsEmpty(&ls_lhModel))
    {
        pmp = listEntry(ls_lhModel.lh_plhNext, da_model_t, dm_lhList);

        pmp->dm_fnFiniModel(pmp);
    }

    return u32Ret;
}

u32 getDaModelByName(olchar_t * name, da_model_t ** model)
{
    u32 u32Ret = OLERR_NO_ERROR;
    da_model_t * pdm = NULL;

    assert(name != NULL);

    u32Ret = _findModelByName(name, &pdm);
    if (u32Ret == OLERR_NO_ERROR)
    {
        *model = pdm;
    }

    return u32Ret;
}

u32 getDaModel(da_model_id_t id, da_model_t ** model)
{
    u32 u32Ret = OLERR_NO_ERROR;
    da_model_t * pdm = NULL;

    u32Ret = _findModel(id, &pdm);
    if (u32Ret == OLERR_NO_ERROR)
    {
        *model = pdm;
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


