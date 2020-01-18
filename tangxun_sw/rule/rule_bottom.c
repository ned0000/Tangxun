/**
 *  @file rule_bottom.c
 *
 *  @brief Implementation file for rules related to bottom
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
#include "rule_bottom.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static boolean_t _isInBottomArea(
    da_day_summary_t * buffer, int total, double maxuprate)
{
    boolean_t bRet = TRUE;
    da_day_summary_t * low;
    double dbInc;
    da_day_summary_t * cur = buffer + total - 1;

    if (total == 0)
        return bRet;

    low = getDaySummaryWithLowestClosingPrice(buffer, total);

    dbInc = (cur->dds_dbClosingPrice - low->dds_dbClosingPrice) * 100 /
        low->dds_dbClosingPrice;
    if (dbInc > maxuprate)
        bRet = FALSE;

    return bRet;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleInBottomArea(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_rule_in_bottom_area_param_t * param = (da_rule_in_bottom_area_param_t *)pdrp;

    if (! _isInBottomArea(buffer, total, param->dribap_u8Threshold))
        u32Ret = JF_ERR_NOT_MATCH;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


