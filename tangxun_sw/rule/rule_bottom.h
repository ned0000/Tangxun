/**
 *  @file rule_bottom.h
 *
 *  @brief Header file for rules related to bottom
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_DARULE_BOTTOM_H
#define TANGXUN_DARULE_BOTTOM_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "darule.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 daRuleInBottomArea(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);


#endif /*TANGXUN_DARULE_BOTTOM_H*/

/*------------------------------------------------------------------------------------------------*/


