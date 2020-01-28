/**
 *  @file rule_line.c
 *
 *  @brief Implementation file for rules related to line.
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
#include "rule_line.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _isNearPressureLine(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total,
    tx_rule_pressure_line_param_t * ptrplp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_ds_t * highest = ptrplp->trplp_ptdUpperLeft;
    oldouble_t dbPrice;
    tx_ds_t * end = buffer + total - 1;

    if (highest->td_dbClosingPrice < ptrplp->trplp_ptdUpperRight->td_dbClosingPrice)
        highest = ptrplp->trplp_ptdUpperRight;

    dbPrice = highest->td_dbClosingPrice * (1 - ptrplp->trplp_dbRatio);
    if (ptrplp->trplp_u8Condition == TX_RULE_PRESSURE_LINE_CONDITION_NEAR)
    {
        if (end->td_dbClosingPrice > dbPrice)
        {
            u32Ret = JF_ERR_NO_ERROR;
            ptrplp->trplp_dbPrice = dbPrice;
        }
    }
    else if (ptrplp->trplp_u8Condition == TX_RULE_PRESSURE_LINE_CONDITION_FAR)
    {
        if (end->td_dbClosingPrice < dbPrice)
        {
            u32Ret = JF_ERR_NO_ERROR;
            ptrplp->trplp_dbPrice = dbPrice;
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 txRulePressureLine(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_rule_pressure_line_param_t * ptrplp = pParam;

    u32Ret = _isNearPressureLine(stockinfo, buffer, total, ptrplp);
        
    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


