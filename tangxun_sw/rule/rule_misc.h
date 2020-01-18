/**
 *  @file rule_misc.h
 *
 *  @brief Header file for miscellaneous rules.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_RULE_MISC_H
#define TANGXUN_RULE_MISC_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_rule.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 daRuleMinNumOfDaySummary(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp);

#endif /*TANGXUN_RULE_MISC_H*/

/*------------------------------------------------------------------------------------------------*/


