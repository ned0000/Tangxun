/**
 *  @file model_xml.c
 *
 *  @brief Implementation file for model XML file
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

#include "model_xml.h"
#include "model_common.h"
#include "damodel.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The prefix string for model xml file
 */
#define MODEL_XML_FILE_PREFIX  "model_"

/** The file extension of the model XML file
 */
#define MODEL_XML_FILE_EXT     ".xml"

/* --- private routine section ------------------------------------------------------------------ */

static u32 _parseModelXmlFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, jf_listhead_t * pjl)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

static u32 _handleDaModelXmlFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, jf_listhead_t * pjl)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_model_t * pdm = NULL;
    
    u32Ret = createDaModel(&pdm);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _parseModelXmlFile(pstrFullpath, pStat, pjl);
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

boolean_t isDaModelXmlFile(const olchar_t * pstrName)
{
    return FALSE; //jf_file_isTypedFile(pstrName, MODEL_XML_FILE_PREFIX, MODEL_XML_FILE_EXT);
}

u32 handleDaModelXmlFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, jf_listhead_t * pjl)
{
    return _handleDaModelXmlFile(pstrFullpath, pStat, pjl);
}


/*------------------------------------------------------------------------------------------------*/

