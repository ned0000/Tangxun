/**
 *  @file rule_limit.c
 *
 *  @brief Implementation file for rules related to limit.
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

#include "tx_env.h"
#include "tx_rule.h"
#include "rule_limit.h"

/* --- private data/data structure section ------------------------------------------------------ */



/* --- private routine section ------------------------------------------------------------------ */



/* --- public routine section ------------------------------------------------------------------- */

u32 txRuleHighLimitOfLastDay(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_ds_t * last = buffer + total - 1;

    if (last->td_bCloseHighLimit)
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

u32 txRuleLowLimitOfLastDay(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_ds_t * last = buffer + total - 1;

    if (last->td_bCloseLowLimit)
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

u32 txRuleMinHighLimitDay(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_rule_min_high_limit_day_param_t * param = pParam;
    u32 highcount = 0, closecount = 0;
    tx_ds_t * start, * end;

    start = buffer;
    end = buffer + total - 1;

    while (start <= end)
    {
        if (param->trmhldp_bHighHighLimit)
        {
            if (start->td_bHighHighLimit)
                highcount ++;
        }
        if (param->trmhldp_bCloseHighLimit)
        {
            if (start->td_bCloseHighLimit)
                closecount ++;
        }

        start ++;
    }

    if (param->trmhldp_bHighHighLimit)
    {
        if (highcount < param->trmhldp_u32MinHighHighLimitDay)
            return u32Ret;
    }

    if (param->trmhldp_bCloseHighLimit)
    {
        if (closecount < param->trmhldp_u32MinCloseHighLimitDay)
            return u32Ret;
    }

    return JF_ERR_NO_ERROR;
}

u32 txRuleNoHighHighLimitDay(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_ds_t * start, * end;

    start = buffer;
    end = buffer + total - 1;

    while (start <= end)
    {
        if (start->td_bHighHighLimit)
            return u32Ret;

        start ++;
    }

    return JF_ERR_NO_ERROR;
}

/*------------------------------------------------------------------------------------------------*/


