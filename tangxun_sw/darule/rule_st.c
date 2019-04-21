/**
 *  @file rule_st.c
 *
 *  @brief Implementation file for rules related to st
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

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_process.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_clieng.h"
#include "jf_mem.h"
#include "jf_jiukun.h"
#include "jf_hashtable.h"

#include "envvar.h"
#include "darule.h"
#include "rule_st.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleNotSt(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_day_summary_t * cur = buffer + total - 1;

    if (cur->dds_bS)
        return u32Ret;
    if (buffer->dds_bS)
        return u32Ret;

    return JF_ERR_NO_ERROR;
}

u32 daRuleSt(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * cur = buffer + total - 1;

    if (cur->dds_bS)
        return u32Ret;
    if (buffer->dds_bS)
        return u32Ret;

    return JF_ERR_NOT_MATCH;
}

/*------------------------------------------------------------------------------------------------*/


