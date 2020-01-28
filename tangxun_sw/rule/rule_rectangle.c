/**
 *  @file rule_rectangle.c
 *
 *  @brief Implementation file for rules related to rectangle.
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

#include "rule_rectangle.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _txRuleRectangle(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, tx_rule_rectangle_param_t * param)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_ds_t * inp[200];
    int nump = 200;
    int index = 0, leftlower = 0, tradedays = 0;
    double dbHigh, dbLow;
    int rectangle_start = 0;

    jf_logger_logInfoMsg("rule rectangle, %s", stockinfo->tsi_strCode);

    if (total < param->trrp_u32MaxDays)
        return u32Ret;

    tx_ds_getDsInflexionPoint(buffer, total, inp, &nump);
    if (nump < 4)
        return u32Ret;

    if (inp[nump - 1]->td_dbClosingPrice < inp[nump - 2]->td_dbClosingPrice)
        return u32Ret;

    /* 1. find the right lower point*/
    param->trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_LOWER] = inp[nump - 2];

    /* 2. find the left lower point*/
    dbHigh = param->trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_LOWER]->td_dbClosingPrice; // * (1 + param->trrp_dbPointThreshold);
    dbLow = param->trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_LOWER]->td_dbClosingPrice * (1 - param->trrp_dbPointThreshold);
    index = nump - 4;
    while (index >= 0)
    {
        if (inp[index]->td_dbClosingPrice < dbLow)
        {
            rectangle_start = index;
            break;
        }
        if (inp[index]->td_dbClosingPrice <= dbHigh)
        {
            if (param->trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_LOWER] == NULL)
            {
                leftlower = index;
                param->trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_LOWER] = inp[index];
            }
            else if (param->trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_LOWER]->td_dbClosingPrice > inp[index]->td_dbClosingPrice)
            {
                leftlower = index;
                param->trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_LOWER] = inp[index];
            }
        }

        index -= 2;
    }
    if (param->trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_LOWER] == NULL)
        return u32Ret;

    /* 3. find the righ upper point*/
    param->trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_UPPER] = inp[leftlower];
    for (index = leftlower; index < nump - 2; index ++)
    {
        if (param->trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_UPPER]->td_dbClosingPrice < inp[index]->td_dbClosingPrice)
        {
            param->trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_UPPER] = inp[index];
        }
    }

    /* 4. find the left upper point*/
    dbHigh = param->trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_UPPER]->td_dbClosingPrice * (1 + param->trrp_dbPointThreshold);
    dbLow = param->trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_UPPER]->td_dbClosingPrice * (1 - param->trrp_dbPointThreshold);
    index = leftlower - 1;
    while (index > rectangle_start)
    {
        if (inp[index]->td_dbClosingPrice > dbHigh)
            break;

        if (inp[index]->td_dbClosingPrice >= dbLow)
        {
            param->trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_UPPER] = inp[index];
            break;
        }

        index -= 2;
    }
    if (param->trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_UPPER] == NULL)
        return u32Ret;

    /* 5. check the number of trading days*/
    tradedays = inp[nump - 1] - param->trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_UPPER];
    if ((tradedays < param->trrp_u32MinDays) ||
        (tradedays > param->trrp_u32MaxDays))
        return u32Ret;

    /* 6. check the edge */
    if (param->trrp_bCheckEdge)
    {
        if ((param->trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_LOWER] - param->trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_UPPER]) >
            (param->trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_UPPER] - param->trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_LOWER]))
            return u32Ret;

        if ((param->trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_LOWER] - param->trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_UPPER]) >
            (param->trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_UPPER] - param->trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_LOWER]))
            return u32Ret;
    }

    jf_logger_logInfoMsg("rule rectangle is match, %s", stockinfo->tsi_strCode);
    return JF_ERR_NO_ERROR;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 txRuleRectangle(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_rule_rectangle_param_t * param = pParam;

    u32Ret = _txRuleRectangle(stockinfo, buffer, total, param);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


