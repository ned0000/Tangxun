/**
 *  @file rule_indi_macd.c
 *
 *  @brief Implementation file for rules related to indicator MACD.
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
#include "jf_jiukun.h"

#include "tx_rule.h"
#include "rule_indi_macd.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */



/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleIndiMacdDiffUpBreakDea(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_rule_indi_macd_diff_up_break_dea_param_t * param =
        (tx_rule_indi_macd_diff_up_break_dea_param_t *)ptrp;

    JF_LOGGER_INFO("sotck, %s", stockinfo->si_strCode);

    if (total < param->trimdubdp_nMacdLongDays)
        return u32Ret;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


