/**
 *  @file rule_st.c
 *
 *  @brief Implementation file for rules related to st.
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
#include "jf_jiukun.h"

#include "tx_rule.h"
#include "rule_st.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleNotStRelated(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_day_summary_t * start = buffer;
    da_day_summary_t * end = buffer + total - 1;

    while (start <= end)
    {
        if ((start->dds_bSt) || (start->dds_bStDelisting))
            return u32Ret;

        start ++;
    }

    return JF_ERR_NO_ERROR;
}

u32 daRuleSt(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * start = buffer;
    da_day_summary_t * end = buffer + total - 1;

    while (start <= end)
    {
        if (start->dds_bSt)
            return u32Ret;

        start ++;
    }

    return JF_ERR_NOT_MATCH;
}

u32 daRuleStDelisting(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * start = buffer;
    da_day_summary_t * end = buffer + total - 1;

    while (start <= end)
    {
        if (start->dds_bStDelisting)
            return u32Ret;

        start ++;
    }

    return JF_ERR_NOT_MATCH;
}

/*------------------------------------------------------------------------------------------------*/


