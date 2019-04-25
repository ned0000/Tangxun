/**
 *  @file model_manager.c
 *
 *  @brief The model manager object
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
#include "model_xml.h"
#include "model_lib.h"
#include "model_common.h"
#include "damodel.h"

/* --- private data/data structure section ------------------------------------------------------ */



/* --- private routine section ------------------------------------------------------------------ */

static u32 _fnHandleModelFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pjl = (jf_listhead_t *)pArg;
    olchar_t strName[128];
    
    jf_file_getFileName(strName, sizeof(strName), pstrFullpath);

    if (isDaModelLibFile(strName))
    {
        handleDaModelLibFile(pstrFullpath, pStat, pjl);
    }
    else if (isDaModelXmlFile(strName))
    {
        handleDaModelXmlFile(pstrFullpath, pStat, pjl);
    }

    return u32Ret;
}

static u32 _parseModelDir(jf_listhead_t * pjl, const char * pstrLibDir)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_dir_parse(pstrLibDir, _fnHandleModelFile, pjl);

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

        destroyDaModel(&pdm);
    }

    return u32Ret;
}


/*------------------------------------------------------------------------------------------------*/

