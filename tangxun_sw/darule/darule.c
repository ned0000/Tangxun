/**
 *  @file rules.c
 *
 *  @brief Base rule library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_process.h"
#include "jf_string.h"
#include "jf_file.h"
#include "darule.h"
#include "jf_clieng.h"
#include "jf_mem.h"
#include "jf_jiukun.h"
#include "envvar.h"

/* --- private data/data structure section --------------------------------- */


static u32 _fsNDaysUpInMDays(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);
static u32 _fsHighLimitOfLastDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);
static u32 _fsLowLimitOfLastDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);
static u32 _fsInBottomArea(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);
static u32 _fsNotSt(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);
static u32 _fsMinNumOfDaySummary(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);
static u32 _fsRectangle(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);
static u32 _fsUpRiseTriangle(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);
static u32 _fsMinRampingDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);
static u32 _fsMinHighLimitDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);
static u32 _fsMinAbnormalVolRatioDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);

static da_rule_t ls_drDaRules[] =
{
    {DA_RULE_N_DAYS_UP_IN_M_DAYS, "n_days_up_in_3_days", _fsNDaysUpInMDays},
    {DA_RULE_HIGH_LIMIT_OF_LAST_DAY, "high_limit_of_last_day", _fsHighLimitOfLastDay},
    {DA_RULE_LOW_LIMIT_OF_LAST_DAY, "low_limit_of_last_day", _fsLowLimitOfLastDay},
    {DA_RULE_IN_BOTTOM_AREA, "in_bottom_area", _fsInBottomArea},
    {DA_RULE_NOT_ST, "not_st", _fsNotSt},
    {DA_RULE_MIN_NUM_OF_DAY_SUMMARY, "min_num_of_day_summary", _fsMinNumOfDaySummary},
    {DA_RULE_RECTANGLE, "rectangle", _fsRectangle},
    {DA_RULE_UP_RISE_TRIANGLE, "up_rise_triangle", _fsUpRiseTriangle},
    {DA_RULE_MIN_RAMPING_DAY, "min_ramping_day", _fsMinRampingDay},
    {DA_RULE_MIN_HIGH_LIMIT_DAY, "min_high_limit_day", _fsMinHighLimitDay},
    {DA_RULE_MIN_ABNORMAL_VOL_RATIO_DAY, "min_abnormal_vol_ratio_day", _fsMinAbnormalVolRatioDay},
};

static u32 ls_u32NumOfRules = sizeof(ls_drDaRules) / sizeof(da_rule_t);

/* --- private routine section---------------------------------------------- */


/*DA_RULE_M_DAYS_UP_IN_N_DAYS*/
static u32 _fsNDaysUpInMDays(
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

static u32 _fsHighLimitOfLastDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
//    da_rule_high_limit_of_last_day_param_t * param = (da_rule_high_limit_of_last_day_param_t *)pdrp;
    da_day_summary_t * last = buffer + total - 1;

    if (last->dds_bCloseHighLimit)
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static u32 _fsLowLimitOfLastDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
//    da_rule_low_limit_of_last_day_param_t * param = (da_rule_low_limit_of_last_day_param_t *)pdrp;
    da_day_summary_t * last = buffer + total - 1;

    if (last->dds_bCloseLowLimit)
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static boolean_t _isInBottomArea(
    da_day_summary_t * buffer, int total,
    double maxuprate)
{
    boolean_t bRet = TRUE;
    da_day_summary_t * low;
    double dbInc;
    da_day_summary_t * cur = buffer + total - 1;

    if (total == 0)
        return bRet;

    low = getDaySummaryWithLowestClosingPrice(buffer, total);

    dbInc = (cur->dds_dbClosingPrice - low->dds_dbClosingPrice) * 100 /
        low->dds_dbClosingPrice;
    if (dbInc > maxuprate)
        bRet = FALSE;

    return bRet;
}

static u32 _fsInBottomArea(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_rule_in_bottom_area_param_t * param = (da_rule_in_bottom_area_param_t *)pdrp;

    if (! _isInBottomArea(buffer, total, param->dribap_u8Threshold))
        u32Ret = JF_ERR_NOT_MATCH;

    return u32Ret;
}

static u32 _fsMinRampingDay(
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

static u32 _fsMinHighLimitDay(
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

static u32 _fsMinAbnormalVolRatioDay(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_rule_min_abnormal_vol_ratio_day_param_t * param = (da_rule_min_abnormal_vol_ratio_day_param_t *)pdrp;
    u32 count = 0;
    da_day_summary_t * start, * end;

    start = buffer;
    end = buffer + total - 1;

    while (start <= end)
    {
        if (start->dds_dbVolumeRatio >= param->drmavrdp_dbRatio)
            count ++;
        start ++;
    }

    if (count < param->drmavrdp_u32MinDay)
        return u32Ret;

    return JF_ERR_NO_ERROR;
}

static u32 _fsNotSt(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_day_summary_t * cur = buffer + total - 1;

    if (cur->dds_bS)
        return u32Ret;
    if (buffer->dds_bS)
        return u32Ret;

    return JF_ERR_NO_ERROR;
}

static u32 _fsMinNumOfDaySummary(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_rule_min_num_of_day_summary_param_t * param = (da_rule_min_num_of_day_summary_param_t *)pdrp;

    if (total < param->drmnodsp_u32MinDay)
        return u32Ret;

    return JF_ERR_NO_ERROR;
}

static u32 _fsRectangle(
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
    da_rule_param_t drp;

    jf_logger_logInfoMsg("rule rectangle, %s", stockinfo->si_strCode);

    if (total < param->drrp_u32MaxDays)
        return u32Ret;

    getDaySummaryInflexionPoint(buffer, total, inp, &nump);
    if (nump < 4)
        return u32Ret;

    if (inp[nump - 1]->dds_dbClosingPrice < inp[nump - 2]->dds_dbClosingPrice)
        return u32Ret;

    /* 1. find the right lower point*/
    param->drrp_pddsRectangle[RIGHT_LOWER] = inp[nump - 2];

    /* 2. find the left lower point*/
    dbHigh = param->drrp_pddsRectangle[RIGHT_LOWER]->dds_dbClosingPrice; // * (1 + param->drrp_dbPointThreshold);
    dbLow = param->drrp_pddsRectangle[RIGHT_LOWER]->dds_dbClosingPrice * (1 - param->drrp_dbPointThreshold);
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
            if (param->drrp_pddsRectangle[LEFT_LOWER] == NULL)
            {
                leftlower = index;
                param->drrp_pddsRectangle[LEFT_LOWER] = inp[index];
            }
            else if (param->drrp_pddsRectangle[LEFT_LOWER]->dds_dbClosingPrice > inp[index]->dds_dbClosingPrice)
            {
                leftlower = index;
                param->drrp_pddsRectangle[LEFT_LOWER] = inp[index];
            }
        }

        index -= 2;
    }
    if (param->drrp_pddsRectangle[LEFT_LOWER] == NULL)
        return u32Ret;

    /* 3. find the righ upper point*/
    param->drrp_pddsRectangle[RIGHT_UPPER] = inp[leftlower];
    for (index = leftlower; index < nump - 2; index ++)
    {
        if (param->drrp_pddsRectangle[RIGHT_UPPER]->dds_dbClosingPrice < inp[index]->dds_dbClosingPrice)
        {
            param->drrp_pddsRectangle[RIGHT_UPPER] = inp[index];
        }
    }

    /* 4. find the left upper point*/
    dbHigh = param->drrp_pddsRectangle[RIGHT_UPPER]->dds_dbClosingPrice * (1 + param->drrp_dbPointThreshold);
    dbLow = param->drrp_pddsRectangle[RIGHT_UPPER]->dds_dbClosingPrice * (1 - param->drrp_dbPointThreshold);
    index = leftlower - 1;
    while (index > rectangle_start)
    {
        if (inp[index]->dds_dbClosingPrice > dbHigh)
            break;

        if (inp[index]->dds_dbClosingPrice >= dbLow)
        {
            param->drrp_pddsRectangle[LEFT_UPPER] = inp[index];
            break;
        }

        index -= 2;
    }
    if (param->drrp_pddsRectangle[LEFT_UPPER] == NULL)
        return u32Ret;

    /* 5. check the number of trading days*/
    tradedays = inp[nump - 1] - param->drrp_pddsRectangle[LEFT_UPPER];
    if ((tradedays < param->drrp_u32MinDays) ||
        (tradedays > param->drrp_u32MaxDays))
        return u32Ret;

    /* 6. check the edge */
    if (param->drrp_bCheckEdge)
    {
        if ((param->drrp_pddsRectangle[RIGHT_LOWER] - param->drrp_pddsRectangle[RIGHT_UPPER]) >
            (param->drrp_pddsRectangle[RIGHT_UPPER] - param->drrp_pddsRectangle[LEFT_LOWER]))
            return u32Ret;

        if ((param->drrp_pddsRectangle[LEFT_LOWER] - param->drrp_pddsRectangle[LEFT_UPPER]) >
            (param->drrp_pddsRectangle[RIGHT_UPPER] - param->drrp_pddsRectangle[LEFT_LOWER]))
            return u32Ret;
    }

    /* 7. below pressure area */
    if (param->drrp_bBelowPressureArea)
    {
        /*use the lower between LEFT_UPPER and RIGHT_UPPER*/
        lower = param->drrp_pddsRectangle[LEFT_UPPER];
        if (lower->dds_dbClosingPrice >
            param->drrp_pddsRectangle[RIGHT_UPPER]->dds_dbClosingPrice)
            lower = param->drrp_pddsRectangle[RIGHT_UPPER];

        dbLow = lower->dds_dbClosingPrice * (1 - param->drrp_dbPressureArea);
        end = buffer + total - 1;
        /*the last day may not be with the highest closing price*/
        high = getDaySummaryWithHighestClosingPrice(
            param->drrp_pddsRectangle[RIGHT_LOWER],
            end - param->drrp_pddsRectangle[RIGHT_LOWER] + 1);
        if (high->dds_dbClosingPrice >= dbLow)
            return u32Ret;
    }

    /* 8. check high price high limit*/
    if (param->drrp_bNoHighHighLimit)
    {
        end = buffer + total - 1;

        ol_bzero(&drp, sizeof(drp));
        drp.drp_drmhldpMinHighLimitDay.drmhldp_bHighHighLimit = TRUE;
        drp.drp_drmhldpMinHighLimitDay.drmhldp_u32MinHighHighLimitDay = 1;
        u32Ret = _fsMinHighLimitDay(
            stockinfo, end - param->drrp_u32MaxDays + 1, param->drrp_u32MaxDays, &drp);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = JF_ERR_NOT_MATCH;
            return u32Ret;
        }
    }

    jf_logger_logInfoMsg("rule rectangle is match, %s", stockinfo->si_strCode);
    return JF_ERR_NO_ERROR;
}

static u32 _fsUpRiseTriangle(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
//    da_rule_up_rise_triangle_param_t * param = (da_rule_up_rise_triangle_param_t *)pdrp;




    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */


u32 getAllDaRules(da_rule_t ** ppRule)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    *ppRule = ls_drDaRules;

    return u32Ret;
}

u32 getDaRule(da_ruld_id_t id, da_rule_t ** ppRule)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    u32 index = 0;
    da_rule_t * pRule = ls_drDaRules;

    *ppRule = NULL;
    for (index = 0; index < ls_u32NumOfRules; index ++)
    {
        if (pRule->dr_driId == id)
        {
            *ppRule = pRule;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pRule ++;
    }

    return u32Ret;    
}

u32 getDaRuleByDesc(char * desc, da_rule_t ** ppRule)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    u32 index = 0;
    da_rule_t * pRule = ls_drDaRules;

    *ppRule = NULL;
    for (index = 0; index < ls_u32NumOfRules; index ++)
    {
        if (strcmp(pRule->dr_pstrDesc, desc) == 0)
        {
            *ppRule = pRule;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pRule ++;
    }

    return u32Ret;    
}

u32 getNumOfDaRules(void)
{
    return ls_u32NumOfRules;
}

/*---------------------------------------------------------------------------*/


