/**
 *  @file rule_misc.c
 *
 *  @brief Implementation file for miscellaneous rules.
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

#include "tx_env.h"
#include "tx_rule.h"
#include "rule_misc.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleMinNumOfDaySummary(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_rule_min_num_of_day_summary_param_t * param = (tx_rule_min_num_of_day_summary_param_t *)ptrp;

    if (total < param->trmnodsp_u32MinDay)
        return u32Ret;

    return JF_ERR_NO_ERROR;
}

/*------------------------------------------------------------------------------------------------*/


