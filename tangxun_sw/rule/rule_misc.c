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

u32 txRuleMinNumOfDaySummary(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_rule_min_num_of_day_summary_param_t * param = pParam;

    if (total < param->trmnodsp_u32MinDay)
        return u32Ret;

    return JF_ERR_NO_ERROR;
}

/*------------------------------------------------------------------------------------------------*/


