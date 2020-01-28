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

#include "tx_model.h"
#include "tx_err.h"

#include "model_manager.h"

/* --- private data/data structure section ------------------------------------------------------ */


static JF_LISTHEAD(ls_jlModel);

/* --- private routine section ------------------------------------------------------------------ */

static u32 _findModelByName(const olchar_t * name, tx_model_t ** model)
{
    u32 u32Ret = TX_ERR_MODEL_NOT_FOUND;
    jf_listhead_t * pos;
    tx_model_t * ptm;

    *model = NULL;

    jf_listhead_forEach(&ls_jlModel, pos)
    {
        ptm = jf_listhead_getEntry(pos, tx_model_t, tm_jlList);
        if (ol_strncasecmp(ptm->tm_strName, name, ol_strlen(ptm->tm_strName)) == 0)
        {
            *model = ptm;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 tx_model_initFramework(const olchar_t * pstrModelDir)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("init framework");

    u32Ret = addDaModel(&ls_jlModel, pstrModelDir);

    return u32Ret;
}

u32 tx_model_finiFramework(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = removeDaModel(&ls_jlModel);

    return u32Ret;
}

u32 tx_model_getModel(const olchar_t * name, tx_model_t ** ppModel)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_model_t * ptm = NULL;

    assert(name != NULL);

    u32Ret = _findModelByName(name, &ptm);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *ppModel = ptm;
    }

    return u32Ret;
}

tx_model_t * tx_model_getFirstModel(void)
{
    tx_model_t * ptm = NULL;

    if (! jf_listhead_isEmpty(&ls_jlModel))
        ptm = jf_listhead_getEntry(ls_jlModel.jl_pjlNext, tx_model_t, tm_jlList);

    return ptm;
}

tx_model_t * tx_model_getNextModel(tx_model_t * ptm)
{
    tx_model_t * pNext = NULL;

    if (! jf_listhead_isLast(&ls_jlModel, &ptm->tm_jlList))
        pNext = jf_listhead_getEntry(ptm->tm_jlList.jl_pjlNext, tx_model_t, tm_jlList);

    return pNext;
}

/*------------------------------------------------------------------------------------------------*/


