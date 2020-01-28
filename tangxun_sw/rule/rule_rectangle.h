/**
 *  @file rule_rectangle.h
 *
 *  @brief Header file for rules related rectangle
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_DARULE_RECTANGLE_H
#define TANGXUN_DARULE_RECTANGLE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_rule.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 txRuleRectangle(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam);


#endif /*TANGXUN_DARULE_RECTANGLE_H*/

/*------------------------------------------------------------------------------------------------*/


