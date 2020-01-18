/**
 *  @file rule_price.h
 *
 *  @brief Header file for rules related to price.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_RULE_PRICE_H
#define TANGXUN_RULE_PRICE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_rule.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 daRuleNDaysUpInMDays(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * pdrp);

u32 daRuleMinRampingDay(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * pdrp);

u32 daRuleNeedStopLoss(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * pdrp);

u32 daRulePriceVolatility(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * pdrp);

#endif /*TANGXUN_DARULE_PRICE_H*/

/*------------------------------------------------------------------------------------------------*/


