/**
 *  @file rule_vol.c
 *
 *  @brief Implementation file for rules related to volumn
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
#include "rule_vol.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleMinAbnormalVolRatioDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_rule_min_abnormal_vol_ratio_day_param_t * param =
        (da_rule_min_abnormal_vol_ratio_day_param_t *)pdrp;
    u32 count = 0;
    da_day_summary_t * start, * end;

    start = buffer;
    end = buffer + total - 1;

    while (start <= end)
    {
        if (start->dds_dbVolumeRatio >= param->drmavrdp_dbRatio)
            count ++;
        start ++;
    }

    if (count < param->drmavrdp_u32MinDay)
        return u32Ret;

    return JF_ERR_NO_ERROR;
}

/*------------------------------------------------------------------------------------------------*/


