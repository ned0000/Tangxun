/**
 *  @file rule_limit.h
 *
 *  @brief Header file for rules related to limit.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_RULE_LIMIT_H
#define TANGXUN_RULE_LIMIT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_rule.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 daRuleHighLimitOfLastDay(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp);

u32 daRuleLowLimitOfLastDay(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp);

u32 daRuleMinHighLimitDay(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp);

u32 daRuleNoHighHighLimitDay(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp);

#endif /*TANGXUN_RULE_LIMIT_H*/

/*------------------------------------------------------------------------------------------------*/


