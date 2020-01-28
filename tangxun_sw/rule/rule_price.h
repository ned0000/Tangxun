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

u32 txRuleNDaysUpInMDays(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

u32 txRuleMinRampingDay(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

u32 txRuleNeedStopLoss(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

u32 txRulePriceVolatility(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

#endif /*TANGXUN_DARULE_PRICE_H*/

/*------------------------------------------------------------------------------------------------*/


