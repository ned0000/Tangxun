/**
 *  @file rule_limit.h
 *
 *  @brief Header file for rules related to limit
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_DARULE_LIMIT_H
#define TANGXUN_DARULE_LIMIT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "darule.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 daRuleHighLimitOfLastDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);

u32 daRuleLowLimitOfLastDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);

u32 daRuleMinHighLimitDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);

u32 daRuleNoHighHighLimitDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);

#endif /*TANGXUN_DARULE_LIMIT_H*/

/*------------------------------------------------------------------------------------------------*/


