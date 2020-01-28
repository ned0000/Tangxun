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

u32 txRuleHighLimitOfLastDay(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

u32 txRuleLowLimitOfLastDay(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

u32 txRuleMinHighLimitDay(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

u32 txRuleNoHighHighLimitDay(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

#endif /*TANGXUN_RULE_LIMIT_H*/

/*------------------------------------------------------------------------------------------------*/


