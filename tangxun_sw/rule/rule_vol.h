/**
 *  @file rule_vol.h
 *
 *  @brief Header file for rules related to volumn.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_RULE_VOL_H
#define TANGXUN_RULE_VOL_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_rule.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 txRuleMinAbnormalVolRatioDay(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);

#endif /*TANGXUN_RULE_VOL_H*/

/*------------------------------------------------------------------------------------------------*/


