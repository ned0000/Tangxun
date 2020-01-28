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

#include "tx_model.h"

#include "model_xml.h"
#include "model_common.h"

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

static u32 _handleTxModelXmlFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, jf_listhead_t * pjl)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_model_t * ptm = NULL;
    
    JF_LOGGER_INFO("path: %s", pstrFullpath);

    u32Ret = createTxModel(&ptm);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _parseModelXmlFile(pstrFullpath, pStat, pjl);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = checkTxModelField(ptm);
    }
    
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_listhead_add(pjl, &ptm->tm_jlList);
    }
    else if (ptm != NULL)
    {
        JF_LOGGER_ERR(u32Ret, "failed to load model");
        destroyTxModel(&ptm);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

boolean_t isTxModelXmlFile(const olchar_t * pstrName)
{
    return jf_file_isTypedFile(pstrName, MODEL_XML_FILE_PREFIX, MODEL_XML_FILE_EXT);
}

u32 handleTxModelXmlFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, jf_listhead_t * pjl)
{
    return _handleTxModelXmlFile(pstrFullpath, pStat, pjl);
}


/*------------------------------------------------------------------------------------------------*/

