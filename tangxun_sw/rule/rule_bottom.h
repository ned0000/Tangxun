/**
 *  @file rule_bottom.h
 *
 *  @brief Header file for rules related to bottom
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_RULE_BOTTOM_H
#define TANGXUN_RULE_BOTTOM_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_rule.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 txRuleInBottomArea(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);


#endif /*TANGXUN_RULE_BOTTOM_H*/

/*------------------------------------------------------------------------------------------------*/


