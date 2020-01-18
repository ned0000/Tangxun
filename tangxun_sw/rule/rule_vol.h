/**
 *  @file rule_vol.h
 *
 *  @brief Header file for rules related to volumn
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_DARULE_VOL_H
#define TANGXUN_DARULE_VOL_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "darule.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 daRuleMinAbnormalVolRatioDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);

#endif /*TANGXUN_DARULE_VOL_H*/

/*------------------------------------------------------------------------------------------------*/


