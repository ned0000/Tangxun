/**
 *  @file rule_price.c
 *
 *  @brief Implementation file for rules related to price
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
#include "rule_price.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleNDaysUpInMDays(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_rule_n_days_up_in_m_days_param_t * param = (da_rule_n_days_up_in_m_days_param_t *)pdrp;
    da_day_summary_t * summary;
    int i, count = 0;

    if (total < param->drnduimdp_u8MDays)
        return u32Ret;

    if (param->drnduimdp_u8NDays > param->drnduimdp_u8MDays)
        return JF_ERR_INVALID_PARAM;

    summary = buffer + total - 1;

    for (i = 0; i < param->drnduimdp_u8MDays; i++)
    {
        if (summary->dds_dbClosingPriceRate >= 0)
            count ++;

        summary --;
    }

    if (count >= param->drnduimdp_u8NDays)
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

u32 daRuleMinRampingDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_rule_min_ramping_day_param_t * param = (da_rule_min_ramping_day_param_t *)pdrp;
    u32 highcount = 0, closecount = 0;
    da_day_summary_t * start, * end;

    start = buffer;
    end = buffer + total - 1;

    while (start <= end)
    {
        if (param->drmrdp_bHighPrice)
        {
            if (start->dds_dbHighPriceRate >= param->drmrdp_dbHighPriceRate)
                highcount ++;
        }
        if (param->drmrdp_bClosePrice)
        {
            if (start->dds_dbClosingPriceRate >= param->drmrdp_dbClosePriceRate)
                closecount ++;
        }

        start ++;
    }

    if (param->drmrdp_bHighPrice)
    {
        if (highcount < param->drmrdp_u32MinHighPriceDay)
            return u32Ret;
    }

    if (param->drmrdp_bClosePrice)
    {
        if (closecount < param->drmrdp_u32MinClosePriceDay)
            return u32Ret;
    }

    return JF_ERR_NO_ERROR;
}

/*------------------------------------------------------------------------------------------------*/


