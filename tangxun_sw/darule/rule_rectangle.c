/**
 *  @file rule_rectangle.c
 *
 *  @brief Implementation file for rules related to rectangle
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
#include "rule_rectangle.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _daRuleRectangle(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_rule_rectangle_param_t * param = (da_rule_rectangle_param_t *)pdrp;
    da_day_summary_t * inp[200];
    da_day_summary_t * lower, * end, * high;
    int nump = 200;
    int index = 0, leftlower = 0, tradedays = 0;
    double dbHigh, dbLow;
    int rectangle_start = 0;

    jf_logger_logInfoMsg("rule rectangle, %s", stockinfo->si_strCode);

    if (total < param->drrp_u32MaxDays)
        return u32Ret;

    getDaySummaryInflexionPoint(buffer, total, inp, &nump);
    if (nump < 4)
        return u32Ret;

    if (inp[nump - 1]->dds_dbClosingPrice < inp[nump - 2]->dds_dbClosingPrice)
        return u32Ret;

    /* 1. find the right lower point*/
    param->drrp_pddsRectangle[RECTANGLE_RIGHT_LOWER] = inp[nump - 2];

    /* 2. find the left lower point*/
    dbHigh = param->drrp_pddsRectangle[RECTANGLE_RIGHT_LOWER]->dds_dbClosingPrice; // * (1 + param->drrp_dbPointThreshold);
    dbLow = param->drrp_pddsRectangle[RECTANGLE_RIGHT_LOWER]->dds_dbClosingPrice * (1 - param->drrp_dbPointThreshold);
    index = nump - 4;
    while (index >= 0)
    {
        if (inp[index]->dds_dbClosingPrice < dbLow)
        {
            rectangle_start = index;
            break;
        }
        if (inp[index]->dds_dbClosingPrice <= dbHigh)
        {
            if (param->drrp_pddsRectangle[RECTANGLE_LEFT_LOWER] == NULL)
            {
                leftlower = index;
                param->drrp_pddsRectangle[RECTANGLE_LEFT_LOWER] = inp[index];
            }
            else if (param->drrp_pddsRectangle[RECTANGLE_LEFT_LOWER]->dds_dbClosingPrice > inp[index]->dds_dbClosingPrice)
            {
                leftlower = index;
                param->drrp_pddsRectangle[RECTANGLE_LEFT_LOWER] = inp[index];
            }
        }

        index -= 2;
    }
    if (param->drrp_pddsRectangle[RECTANGLE_LEFT_LOWER] == NULL)
        return u32Ret;

    /* 3. find the righ upper point*/
    param->drrp_pddsRectangle[RECTANGLE_RIGHT_UPPER] = inp[leftlower];
    for (index = leftlower; index < nump - 2; index ++)
    {
        if (param->drrp_pddsRectangle[RECTANGLE_RIGHT_UPPER]->dds_dbClosingPrice < inp[index]->dds_dbClosingPrice)
        {
            param->drrp_pddsRectangle[RECTANGLE_RIGHT_UPPER] = inp[index];
        }
    }

    /* 4. find the left upper point*/
    dbHigh = param->drrp_pddsRectangle[RECTANGLE_RIGHT_UPPER]->dds_dbClosingPrice * (1 + param->drrp_dbPointThreshold);
    dbLow = param->drrp_pddsRectangle[RECTANGLE_RIGHT_UPPER]->dds_dbClosingPrice * (1 - param->drrp_dbPointThreshold);
    index = leftlower - 1;
    while (index > rectangle_start)
    {
        if (inp[index]->dds_dbClosingPrice > dbHigh)
            break;

        if (inp[index]->dds_dbClosingPrice >= dbLow)
        {
            param->drrp_pddsRectangle[RECTANGLE_LEFT_UPPER] = inp[index];
            break;
        }

        index -= 2;
    }
    if (param->drrp_pddsRectangle[RECTANGLE_LEFT_UPPER] == NULL)
        return u32Ret;

    /* 5. check the number of trading days*/
    tradedays = inp[nump - 1] - param->drrp_pddsRectangle[RECTANGLE_LEFT_UPPER];
    if ((tradedays < param->drrp_u32MinDays) ||
        (tradedays > param->drrp_u32MaxDays))
        return u32Ret;

    /* 6. check the edge */
    if (param->drrp_bCheckEdge)
    {
        if ((param->drrp_pddsRectangle[RECTANGLE_RIGHT_LOWER] - param->drrp_pddsRectangle[RECTANGLE_RIGHT_UPPER]) >
            (param->drrp_pddsRectangle[RECTANGLE_RIGHT_UPPER] - param->drrp_pddsRectangle[RECTANGLE_LEFT_LOWER]))
            return u32Ret;

        if ((param->drrp_pddsRectangle[RECTANGLE_LEFT_LOWER] - param->drrp_pddsRectangle[RECTANGLE_LEFT_UPPER]) >
            (param->drrp_pddsRectangle[RECTANGLE_RIGHT_UPPER] - param->drrp_pddsRectangle[RECTANGLE_LEFT_LOWER]))
            return u32Ret;
    }

    /* 7. below pressure area */
    if (param->drrp_bBelowPressureArea)
    {
        /*use the lower between RECTANGLE_LEFT_UPPER and RECTANGLE_RIGHT_UPPER*/
        lower = param->drrp_pddsRectangle[RECTANGLE_LEFT_UPPER];
        if (lower->dds_dbClosingPrice >
            param->drrp_pddsRectangle[RECTANGLE_RIGHT_UPPER]->dds_dbClosingPrice)
            lower = param->drrp_pddsRectangle[RECTANGLE_RIGHT_UPPER];

        dbLow = lower->dds_dbClosingPrice * (1 - param->drrp_dbPressureArea);
        end = buffer + total - 1;
        /*the last day may not be with the highest closing price*/
        high = getDaySummaryWithHighestClosingPrice(
            param->drrp_pddsRectangle[RECTANGLE_RIGHT_LOWER],
            end - param->drrp_pddsRectangle[RECTANGLE_RIGHT_LOWER] + 1);
        if (high->dds_dbClosingPrice >= dbLow)
            return u32Ret;
    }

    jf_logger_logInfoMsg("rule rectangle is match, %s", stockinfo->si_strCode);
    return JF_ERR_NO_ERROR;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleRectangle(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _daRuleRectangle(stockinfo, buffer, total, pdrp);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


