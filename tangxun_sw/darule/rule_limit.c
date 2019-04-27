/**
 *  @file rule_limit.c
 *
 *  @brief Implementation file for rules related to limit
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_process.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_clieng.h"
#include "jf_mem.h"
#include "jf_jiukun.h"
#include "jf_hashtable.h"

#include "envvar.h"
#include "darule.h"
#include "rule_limit.h"

/* --- private data/data structure section ------------------------------------------------------ */



/* --- private routine section ------------------------------------------------------------------ */



/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleHighLimitOfLastDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_day_summary_t * last = buffer + total - 1;

    if (last->dds_bCloseHighLimit)
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

u32 daRuleLowLimitOfLastDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_day_summary_t * last = buffer + total - 1;

    if (last->dds_bCloseLowLimit)
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

u32 daRuleMinHighLimitDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_rule_min_high_limit_day_param_t * param = (da_rule_min_high_limit_day_param_t *)pdrp;
    u32 highcount = 0, closecount = 0;
    da_day_summary_t * start, * end;

    start = buffer;
    end = buffer + total - 1;

    while (start <= end)
    {
        if (param->drmhldp_bHighHighLimit)
        {
            if (start->dds_bHighHighLimit)
                highcount ++;
        }
        if (param->drmhldp_bCloseHighLimit)
        {
            if (start->dds_bCloseHighLimit)
                closecount ++;
        }

        start ++;
    }

    if (param->drmhldp_bHighHighLimit)
    {
        if (highcount < param->drmhldp_u32MinHighHighLimitDay)
            return u32Ret;
    }

    if (param->drmhldp_bCloseHighLimit)
    {
        if (closecount < param->drmhldp_u32MinCloseHighLimitDay)
            return u32Ret;
    }

    return JF_ERR_NO_ERROR;
}

u32 daRuleNoHighHighLimitDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_day_summary_t * start, * end;

    start = buffer;
    end = buffer + total - 1;

    while (start <= end)
    {
        if (start->dds_bHighHighLimit)
            return u32Ret;

        start ++;
    }

    return JF_ERR_NO_ERROR;
}

/*------------------------------------------------------------------------------------------------*/


