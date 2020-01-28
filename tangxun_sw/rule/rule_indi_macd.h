/**
 *  @file rule_indi_macd.h
 *
 *  @brief Header file for rules related to indicator MACD
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_RULE_INDI_MACD_H
#define TANGXUN_RULE_INDI_MACD_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_rule.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 txRuleIndiMacdDiffUpBreakDea(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

u32 txRuleIndiMacdPositiveDiffDea(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

#endif /*TANGXUN_RULE_INDI_MACD_H*/

/*------------------------------------------------------------------------------------------------*/


