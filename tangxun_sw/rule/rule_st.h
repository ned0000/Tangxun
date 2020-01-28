/**
 *  @file rule_st.h
 *
 *  @brief Header file for rules related to st.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_RULE_ST_H
#define TANGXUN_RULE_ST_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_rule.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 txRuleNotStRelated(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

u32 txRuleSt(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

u32 txRuleStDelisting(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

#endif /*TANGXUN_RULE_ST_H*/

/*------------------------------------------------------------------------------------------------*/

