/**
 *  @file rule_price.c
 *
 *  @brief Implementation file for rules related to price.
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
#include "rule_price.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleNDaysUpInMDays(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_rule_n_days_up_in_m_days_param_t * param = (tx_rule_n_days_up_in_m_days_param_t *)ptrp;
    da_day_summary_t * summary;
    int i, count = 0;

    if (total < param->trnduimdp_u8MDays)
        return u32Ret;

    if (param->trnduimdp_u8NDays > param->trnduimdp_u8MDays)
        return JF_ERR_INVALID_PARAM;

    summary = buffer + total - 1;

    for (i = 0; i < param->trnduimdp_u8MDays; i++)
    {
        if (summary->dds_dbClosingPriceRate >= 0)
            count ++;

        summary --;
    }

    if (count >= param->trnduimdp_u8NDays)
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

u32 daRuleMinRampingDay(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_rule_min_ramping_day_param_t * param = (tx_rule_min_ramping_day_param_t *)ptrp;
    u32 highcount = 0, closecount = 0;
    da_day_summary_t * start, * end;

    start = buffer;
    end = buffer + total - 1;

    while (start <= end)
    {
        if (param->trmrdp_bHighPrice)
        {
            if (start->dds_dbHighPriceRate >= param->trmrdp_dbHighPriceRate)
                highcount ++;
        }
        if (param->trmrdp_bClosePrice)
        {
            if (start->dds_dbClosingPriceRate >= param->trmrdp_dbClosePriceRate)
                closecount ++;
        }

        start ++;
    }

    if (param->trmrdp_bHighPrice)
    {
        if (highcount < param->trmrdp_u32MinHighPriceDay)
            return u32Ret;
    }

    if (param->trmrdp_bClosePrice)
    {
        if (closecount < param->trmrdp_u32MinClosePriceDay)
            return u32Ret;
    }

    return JF_ERR_NO_ERROR;
}

u32 daRuleNeedStopLoss(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_rule_need_stop_loss_param_t * ptrnslp = (tx_rule_need_stop_loss_param_t *)ptrp;
    da_day_summary_t * end = buffer + total - 1;
    oldouble_t dbPrice;

    dbPrice = ptrnslp->trnslp_dbBuyPrice * (1 - ptrnslp->trnslp_dbRatio);
    if (end->dds_dbLowPrice < dbPrice)
    {
        u32Ret = JF_ERR_NO_ERROR;
        ptrnslp->trnslp_dbStopLossPrice = dbPrice;
    }

    return u32Ret;
}

u32 daRulePriceVolatility(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_rule_price_volatility_param_t * param = (tx_rule_price_volatility_param_t *)ptrp; 
    oldouble_t dbRatio;
    da_day_summary_t * high, * low;

    high = getDaySummaryWithHighestClosingPrice(buffer, total);
    low = getDaySummaryWithLowestClosingPrice(buffer, total);

    dbRatio = (high->dds_dbClosingPrice - low->dds_dbClosingPrice) / low->dds_dbClosingPrice;

    if (param->trpvp_u8Condition == PRICE_VOLATILITY_CONDITION_GREATER_EQUAL)
    {
        if (dbRatio >= param->trpvp_dbVolatility)
            u32Ret = JF_ERR_NO_ERROR;
    }
    else if (param->trpvp_u8Condition == PRICE_VOLATILITY_CONDITION_LESSER_EQUAL)
    {
        if (dbRatio <= param->trpvp_dbVolatility)
            u32Ret = JF_ERR_NO_ERROR;
    }
        
    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


