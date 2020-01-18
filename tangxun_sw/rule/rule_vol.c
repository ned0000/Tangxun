/**
 *  @file rule_vol.c
 *
 *  @brief Implementation file for rules related to volumn.
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

#include "tx_rule.h"
#include "rule_vol.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleMinAbnormalVolRatioDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_rule_min_abnormal_vol_ratio_day_param_t * param =
        (tx_rule_min_abnormal_vol_ratio_day_param_t *)ptrp;
    u32 count = 0;
    da_day_summary_t * start, * end;

    start = buffer;
    end = buffer + total - 1;

    while (start <= end)
    {
        if (start->dds_dbVolumeRatio >= param->trmavrdp_dbRatio)
            count ++;
        start ++;
    }

    if (count < param->trmavrdp_u32MinDay)
        return u32Ret;

    return JF_ERR_NO_ERROR;
}

/*------------------------------------------------------------------------------------------------*/


