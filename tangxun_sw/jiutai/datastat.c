/**
 *  @file datastat.c
 *
 *  @brief data statistic
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <math.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <stdlib.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "datastat.h"
#include "errcode.h"
#include "clieng.h"
#include "stringparse.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */
static clieng_caption_t ls_ccDataStatVerbose[] =
{
    {"Name", CLIENG_CAP_HALF_LINE},
    {"NumOfTradingDay", CLIENG_CAP_HALF_LINE}, {"DayForTrend", CLIENG_CAP_HALF_LINE},
    {"StartDate", CLIENG_CAP_HALF_LINE}, {"EndDate", CLIENG_CAP_HALF_LINE},

    {"MinClosingPriceRate", CLIENG_CAP_HALF_LINE}, {"MaxClosingPriceRate", CLIENG_CAP_HALF_LINE},
    {"MinTunePercent", CLIENG_CAP_HALF_LINE}, {"MaxTunePercent", CLIENG_CAP_HALF_LINE},
    {"CloseHighLimit", CLIENG_CAP_HALF_LINE}, {"LastTimeCloseHighLimit", CLIENG_CAP_HALF_LINE},
    {"MaxTimeOpenCloseHighLimit", CLIENG_CAP_FULL_LINE},

    {"MinBuyPercent", CLIENG_CAP_HALF_LINE}, {"MaxBuyPercent", CLIENG_CAP_HALF_LINE},
    {"AveBuyPercent", CLIENG_CAP_HALF_LINE}, {"MinAveBuyPercentForTrend", CLIENG_CAP_HALF_LINE},
    {"MinSoldPercent", CLIENG_CAP_HALF_LINE}, {"MaxSoldPercent", CLIENG_CAP_HALF_LINE},
    {"AveSoldPercent", CLIENG_CAP_FULL_LINE},
    {"MinLambBuyPercent", CLIENG_CAP_HALF_LINE}, {"MaxLambBuyPercent", CLIENG_CAP_HALF_LINE},
    {"AveLambBuyPercent", CLIENG_CAP_FULL_LINE},
    {"MinLambSoldPercent", CLIENG_CAP_HALF_LINE}, {"MaxLambSoldPercent", CLIENG_CAP_HALF_LINE},
    {"AveLambSoldPercent", CLIENG_CAP_FULL_LINE},
    {"BuyInAm", CLIENG_CAP_FULL_LINE},
    {"MaxVolumeRatio", CLIENG_CAP_FULL_LINE},
    {"MaxSoldVolumeRatio", CLIENG_CAP_HALF_LINE}, {"MaxLambSoldVolumeRatio", CLIENG_CAP_HALF_LINE},
    {"MaxCloseHighLimitVolumeRatio", CLIENG_CAP_FULL_LINE},
    {"MaxCloseHighLimitSoldVolumeRatio", CLIENG_CAP_HALF_LINE}, {"MaxCloseHighLimitLambSoldVolumeRatio", CLIENG_CAP_HALF_LINE},

    {"MinBuyA", CLIENG_CAP_HALF_LINE}, {"MaxBuyA", CLIENG_CAP_HALF_LINE},
    {"AveBuyA", CLIENG_CAP_FULL_LINE},
    {"MinSoldA", CLIENG_CAP_HALF_LINE}, {"MaxSoldA", CLIENG_CAP_HALF_LINE},
    {"AveSoldA", CLIENG_CAP_FULL_LINE},
    {"MinLambBuyA", CLIENG_CAP_HALF_LINE}, {"MaxLambBuyA", CLIENG_CAP_HALF_LINE},
    {"AveLambBuyA", CLIENG_CAP_FULL_LINE},
    {"MinLambSoldA", CLIENG_CAP_HALF_LINE}, {"MaxLambSoldA", CLIENG_CAP_HALF_LINE},
    {"AveLambSoldA", CLIENG_CAP_FULL_LINE},

    {"MaxUpperShadowRatio", CLIENG_CAP_HALF_LINE}, {"MaxLowerShadowRatio", CLIENG_CAP_HALF_LINE},

    {"MaxLastLowPriceInc", CLIENG_CAP_HALF_LINE}, {"LastTimeOfLowPrice", CLIENG_CAP_HALF_LINE},
};

static clieng_caption_t ls_ccDescStatVerbose[] =
{
    {"Count", CLIENG_CAP_FULL_LINE},
    {"All", CLIENG_CAP_FULL_LINE},
    {"Mean", CLIENG_CAP_HALF_LINE}, {"SEMean", CLIENG_CAP_HALF_LINE},

    {"Variance", CLIENG_CAP_HALF_LINE}, {"StDev", CLIENG_CAP_HALF_LINE},

    {"Min", CLIENG_CAP_HALF_LINE}, {"Q1", CLIENG_CAP_HALF_LINE},
    {"Median", CLIENG_CAP_HALF_LINE}, {"Q3", CLIENG_CAP_HALF_LINE},
    {"Max", CLIENG_CAP_FULL_LINE},

    {"StDev1Percent", CLIENG_CAP_HALF_LINE}, {"StDev2Percent", CLIENG_CAP_HALF_LINE},
};

static clieng_caption_t ls_ccFrequencyBrief[] =
{
    {"Unit", 6},
    {"Region", 24},
    {"Freq", 6},

};

/* --- private routine section---------------------------------------------- */

static olint_t _compareData(const void * a, const void * b)
{
    oldouble_t ra, rb;

    ra = *(oldouble_t *)a;
    rb = *(oldouble_t *)b;

    return ra > rb ? 1 : -1;
}

static void _dsPriceFromDayResult(
    data_stat_param_t * pdsp, data_stat_t * stat,
    da_day_summary_t * buffer, olint_t num)
{
    da_day_summary_t * summary, * prev, * end = buffer + num - 1;
    da_day_summary_t * high;
    olint_t i;
    oldouble_t dblow = 0.0;

    summary = buffer;
    summary ++;
    stat->ds_dbMinClosingPriceRate = 100;
    stat->ds_dbMinClosingPriceRate = 100;
    for (i = 1; i < num; i ++)
    {
        if (summary->dds_bCloseHighLimit)
        {
            stat->ds_bCloseHighLimit = TRUE;
            if ((stat->ds_strLastTimeCloseHighLimit[0] == '\0') ||
                (strcmp(stat->ds_strLastTimeCloseHighLimit,
                        summary->dds_ddrResult->ddr_strTimeCloseHighLimit) < 0))
                ol_strcpy(stat->ds_strLastTimeCloseHighLimit,
                       summary->dds_ddrResult->ddr_strTimeCloseHighLimit);

            if (stat->ds_nMaxTimeOpenCloseHighLimit <
                summary->dds_ddrResult->ddr_nMaxTimeOpenCloseHighLimit)
                stat->ds_nMaxTimeOpenCloseHighLimit =
                    summary->dds_ddrResult->ddr_nMaxTimeOpenCloseHighLimit;
        }

		if (stat->ds_dbMinClosingPriceRate > summary->dds_dbClosingPriceRate)
			stat->ds_dbMinClosingPriceRate = summary->dds_dbClosingPriceRate;
		if (stat->ds_dbMaxClosingPriceRate < summary->dds_dbClosingPriceRate)
			stat->ds_dbMaxClosingPriceRate = summary->dds_dbClosingPriceRate;

        summary ++;
    }

    prev = summary = buffer;
    summary ++;
    stat->ds_dbMinTunePercent = 100;
    while (summary <= end)
    {
        if (summary->dds_dbClosingPrice < prev->dds_dbClosingPrice)
        {
            high = prev;
            dblow = prev->dds_dbClosingPrice;

            while (summary <= end)
            {
                if (dblow > summary->dds_dbLowPrice)
                    dblow = summary->dds_dbLowPrice;

                if (high->dds_dbClosingPrice < summary->dds_dbClosingPrice)
                {
                    dblow = (high->dds_dbClosingPrice - dblow) * 100 /
                        high->dds_dbClosingPrice;

                    if (stat->ds_dbMinTunePercent > dblow)
                        stat->ds_dbMinTunePercent = dblow;
                    if (stat->ds_dbMaxTunePercent < dblow)
                        stat->ds_dbMaxTunePercent = dblow;

                    break;
                }

                prev ++;
                summary ++;
            }

        }

        prev ++;
        summary ++;
    }

}

static void _dsVolFromDayResult(
    data_stat_param_t * pdsp, data_stat_t * stat,
    da_day_summary_t * buffer, olint_t num, boolean_t bWithCloseHighLimit)
{
    da_day_summary_t * summary, * end;
	da_day_result_t * result;
    olint_t i, j, count;
    oldouble_t dbtmp = 0.0;

    summary = buffer;
	result = summary->dds_ddrResult;
    count = 0;
    stat->ds_nDayForTrend = pdsp->dsp_nDayForTrend;
    stat->ds_u64MinBuy = result->ddr_u64Buy;
    stat->ds_u64MinSold = result->ddr_u64Sold;
    stat->ds_u64MinLambBuy = result->ddr_u64LambBuy;
    stat->ds_u64MinLambSold = result->ddr_u64LambSold;
    stat->ds_dbMinBuyPercent = result->ddr_dbBuyPercent;
    stat->ds_dbMinAveBuyPercentForTrend = 100;
    stat->ds_dbMinSoldPercent = result->ddr_dbSoldPercent;
    stat->ds_dbMinLambBuyPercent = result->ddr_dbLambBuyPercent;
    stat->ds_dbMinLambSoldPercent = result->ddr_dbLambSoldPercent;
    stat->ds_bBuyInAm = TRUE;
    for (i = 0; i < num; i ++)
    {
		result = summary->dds_ddrResult;
        if (summary->dds_bCloseHighLimit)
        {
            if (stat->ds_dbMaxCloseHighLimitVolumeRatio < summary->dds_dbVolumeRatio)
                stat->ds_dbMaxCloseHighLimitVolumeRatio = summary->dds_dbVolumeRatio;

            if (stat->ds_dbMaxCloseHighLimitSoldVolumeRatio < result->ddr_dbSoldVolumeRatio)
                stat->ds_dbMaxCloseHighLimitSoldVolumeRatio = result->ddr_dbSoldVolumeRatio;

            if (stat->ds_dbMaxCloseHighLimitLambSoldVolumeRatio < result->ddr_dbLambSoldVolumeRatio)
                stat->ds_dbMaxCloseHighLimitLambSoldVolumeRatio = result->ddr_dbLambSoldVolumeRatio;
        }
        else
        {
            if (stat->ds_dbMaxVolumeRatio < summary->dds_dbVolumeRatio)
                stat->ds_dbMaxVolumeRatio = summary->dds_dbVolumeRatio;

            if (stat->ds_dbMaxSoldVolumeRatio < result->ddr_dbSoldVolumeRatio)
                stat->ds_dbMaxSoldVolumeRatio = result->ddr_dbSoldVolumeRatio;

            if (stat->ds_dbMaxLambSoldVolumeRatio < result->ddr_dbLambSoldVolumeRatio)
                stat->ds_dbMaxLambSoldVolumeRatio = result->ddr_dbLambSoldVolumeRatio;
        }

        if (stat->ds_u64MinBuy > result->ddr_u64Buy)
            stat->ds_u64MinBuy = result->ddr_u64Buy;
        if (stat->ds_u64MaxBuy < result->ddr_u64Buy)
            stat->ds_u64MaxBuy = result->ddr_u64Buy;

        if (stat->ds_u64MinSold > result->ddr_u64Sold)
            stat->ds_u64MinSold = result->ddr_u64Sold;
        if (stat->ds_u64MaxSold < result->ddr_u64Sold)
            stat->ds_u64MaxSold = result->ddr_u64Sold;

        if (stat->ds_u64MinLambBuy > result->ddr_u64LambBuy)
            stat->ds_u64MinLambBuy = result->ddr_u64LambBuy;
        if (stat->ds_u64MaxLambBuy < result->ddr_u64LambBuy)
            stat->ds_u64MaxLambBuy = result->ddr_u64LambBuy;

        if (stat->ds_u64MinLambSold > result->ddr_u64LambSold)
            stat->ds_u64MinLambSold = result->ddr_u64LambSold;
        if (stat->ds_u64MaxLambSold < result->ddr_u64LambSold)
            stat->ds_u64MaxLambSold = result->ddr_u64LambSold;

        if (stat->ds_dbMinBuyPercent > result->ddr_dbBuyPercent)
            stat->ds_dbMinBuyPercent = result->ddr_dbBuyPercent;
        if (stat->ds_dbMaxBuyPercent < result->ddr_dbBuyPercent)
            stat->ds_dbMaxBuyPercent = result->ddr_dbBuyPercent;

        if (i >= pdsp->dsp_nDayForTrend)
        {
            end = summary;
            dbtmp = 0;
            for (j = 0; j < pdsp->dsp_nDayForTrend; j ++)
            {
                dbtmp += end->dds_ddrResult->ddr_dbBuyPercent;
                end --;
            }
            dbtmp /= pdsp->dsp_nDayForTrend;
            if (stat->ds_dbMinAveBuyPercentForTrend > dbtmp)
                stat->ds_dbMinAveBuyPercentForTrend = dbtmp;
        }

        if (stat->ds_dbMinSoldPercent > result->ddr_dbSoldPercent)
            stat->ds_dbMinSoldPercent = result->ddr_dbSoldPercent;
        if (stat->ds_dbMaxSoldPercent < result->ddr_dbSoldPercent)
            stat->ds_dbMaxSoldPercent = result->ddr_dbSoldPercent;

        if (stat->ds_dbMinLambBuyPercent > result->ddr_dbLambBuyPercent)
            stat->ds_dbMinLambBuyPercent = result->ddr_dbLambBuyPercent;
        if (stat->ds_dbMaxLambBuyPercent < result->ddr_dbLambBuyPercent)
            stat->ds_dbMaxLambBuyPercent = result->ddr_dbLambBuyPercent;

        if (stat->ds_dbMinLambSoldPercent > result->ddr_dbLambSoldPercent)
            stat->ds_dbMinLambSoldPercent = result->ddr_dbLambSoldPercent;
        if (stat->ds_dbMaxLambSoldPercent < result->ddr_dbLambSoldPercent)
            stat->ds_dbMaxLambSoldPercent = result->ddr_dbLambSoldPercent;

        if (result->ddr_dbBuyInAmPercent == 0)
            stat->ds_bBuyInAm = FALSE;

//        if (bWithCloseHighLimit || ! result->ddr_bCloseHighLimit)
        {
            stat->ds_u64AllBuy += result->ddr_u64Buy + result->ddr_u64CloseHighLimitSold;
            stat->ds_u64AllSold += result->ddr_u64Sold + result->ddr_u64CloseLowLimitBuy;
            stat->ds_u64AllLambBuy += result->ddr_u64LambBuy + result->ddr_u64CloseHighLimitLambSold;
            stat->ds_u64AllLambSold += result->ddr_u64LambSold + result->ddr_u64CloseLowLimitLambBuy;

            stat->ds_dbAveBuyPercent += result->ddr_dbBuyPercent + result->ddr_dbCloseHighLimitSoldPercent;
            stat->ds_dbAveSoldPercent += result->ddr_dbSoldPercent + result->ddr_dbCloseLowLimitBuyPercent;
            stat->ds_dbAveLambBuyPercent += result->ddr_dbLambBuyPercent + result->ddr_dbCloseHighLimitLambSoldPercent;
            stat->ds_dbAveLambSoldPercent += result->ddr_dbLambSoldPercent + result->ddr_dbCloseLowLimitLambBuyPercent;

            count ++;
        }

        summary ++;
    }

    if (count != 0)
    {
        stat->ds_u64AveBuy = stat->ds_u64AllBuy / count;
        stat->ds_u64AveSold = stat->ds_u64AllSold / count;
        stat->ds_u64AveLambBuy = stat->ds_u64AllLambBuy / count;
        stat->ds_u64AveLambSold = stat->ds_u64AllLambSold / count;

        stat->ds_dbAveBuyPercent /= count;
        stat->ds_dbAveSoldPercent /= count;
        stat->ds_dbAveLambBuyPercent /= count;
        stat->ds_dbAveLambSoldPercent /= count;
    }
}

static void _dsAmountFromDayResult(
    data_stat_param_t * pdsp, data_stat_t * stat,
    da_day_summary_t * buffer, olint_t num, boolean_t bWithCloseHighLimit)
{
    da_day_summary_t * summary;
	da_day_result_t * result;
    olint_t i, count;

    summary = buffer;
	result = summary->dds_ddrResult;
    count = 0;
    stat->ds_u64MinBuyA = result->ddr_u64BuyA;
    stat->ds_u64MinSoldA = result->ddr_u64SoldA;
    stat->ds_u64MinLambBuyA = result->ddr_u64LambBuyA;
    stat->ds_u64MinLambSoldA = result->ddr_u64LambSoldA;
    for (i = 0; i < num; i ++)
    {
		result = summary->dds_ddrResult;
        if (stat->ds_u64MinBuyA > result->ddr_u64BuyA)
            stat->ds_u64MinBuyA = result->ddr_u64BuyA;
        if (stat->ds_u64MaxBuyA < result->ddr_u64BuyA)
            stat->ds_u64MaxBuyA = result->ddr_u64BuyA;

        if (stat->ds_u64MinSoldA > result->ddr_u64SoldA)
            stat->ds_u64MinSoldA = result->ddr_u64SoldA;
        if (stat->ds_u64MaxSoldA < result->ddr_u64SoldA)
            stat->ds_u64MaxSoldA = result->ddr_u64SoldA;

        if (stat->ds_u64MinLambBuyA > result->ddr_u64LambBuyA)
            stat->ds_u64MinLambBuyA = result->ddr_u64LambBuyA;
        if (stat->ds_u64MaxLambBuyA < result->ddr_u64LambBuyA)
            stat->ds_u64MaxLambBuyA = result->ddr_u64LambBuyA;

        if (stat->ds_u64MinLambSoldA > result->ddr_u64LambSoldA)
            stat->ds_u64MinLambSoldA = result->ddr_u64LambSoldA;
        if (stat->ds_u64MaxLambSoldA < result->ddr_u64LambSoldA)
            stat->ds_u64MaxLambSoldA = result->ddr_u64LambSoldA;

//        if (bWithCloseHighLimit || ! result->ddr_bCloseHighLimit)
        {
            stat->ds_u64AllBuyA += result->ddr_u64BuyA + result->ddr_u64CloseHighLimitSoldA;
            stat->ds_u64AllSoldA += result->ddr_u64SoldA + result->ddr_u64CloseLowLimitBuyA;
            stat->ds_u64AllLambBuyA += result->ddr_u64LambBuyA + result->ddr_u64CloseHighLimitLambSoldA;
            stat->ds_u64AllLambSoldA += result->ddr_u64LambSoldA + result->ddr_u64CloseLowLimitLambBuyA;

            count ++;
        }

        summary ++;
    }

    if (count != 0)
    {
        stat->ds_u64AveBuyA = stat->ds_u64AllBuyA / count;
        stat->ds_u64AveSoldA = stat->ds_u64AllSoldA / count;
        stat->ds_u64AveLambBuyA = stat->ds_u64AllLambBuyA / count;
        stat->ds_u64AveLambSoldA = stat->ds_u64AllLambSoldA / count;
    }
}

static void _dsOtherFromDayResult(
    data_stat_param_t * pdsp, data_stat_t * stat,
    da_day_summary_t * buffer, olint_t num)
{
    da_day_summary_t * summary;
	da_day_result_t * result;
    olint_t i;

    summary = buffer;
	result = summary->dds_ddrResult;
    ol_strcpy(stat->ds_strLastTimeLowPrice, result->ddr_strLastTimeLowPrice);
    stat->ds_dbMaxLastXInc = result->ddr_dbLastXInc;
    for (i = 0; i < num; i ++)
    {
		result = summary->dds_ddrResult;
        if (stat->ds_dbMaxUpperShadowRatio < summary->dds_dbUpperShadowRatio)
            stat->ds_dbMaxUpperShadowRatio = summary->dds_dbUpperShadowRatio;
        if (stat->ds_dbMaxLowerShadowRatio < summary->dds_dbLowerShadowRatio)
            stat->ds_dbMaxLowerShadowRatio = summary->dds_dbLowerShadowRatio;

        if (stat->ds_dbMaxLastXInc < result->ddr_dbLastXInc)
        {
            stat->ds_dbMaxLastXInc = result->ddr_dbLastXInc;
            ol_strcpy(stat->ds_strLastTimeLowPrice, result->ddr_strLastTimeLowPrice);
            ol_strcpy(stat->ds_strLastTimeLowPriceDate, result->ddr_strDate);
        }

        summary ++;
    }

}

/* --- public routine section ---------------------------------------------- */
u32 dataStatFromDaySummary(
    data_stat_param_t * pdsp, data_stat_t * stat,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = OLERR_NO_ERROR;
    da_day_summary_t * summary = buffer, * end = buffer + num - 1;

    assert(pdsp != NULL);
    assert((pdsp->dsp_pstrName != NULL) && (pdsp->dsp_nDayForTrend != 0));

    memset(stat, 0, sizeof(*stat));

    if (num < 2)
        return OLERR_INVALID_DATA;

    ol_strncpy(stat->ds_strName, pdsp->dsp_pstrName, 15);
    stat->ds_nNumOfDay = num;
    ol_strcpy(stat->ds_strStartDate, summary->dds_strDate);
    ol_strcpy(stat->ds_strEndDate, end->dds_strDate);

    _dsPriceFromDayResult(pdsp, stat, buffer, num);
    _dsVolFromDayResult(pdsp, stat, buffer, num, ! pdsp->dsp_bWithoutCloseHighLimit);
    _dsAmountFromDayResult(pdsp, stat, buffer, num, ! pdsp->dsp_bWithoutCloseHighLimit);
    _dsOtherFromDayResult(pdsp, stat, buffer, num);

    return u32Ret;
}

void printDataStatVerbose(data_stat_t * stat)
{
    clieng_caption_t * pcc = &ls_ccDataStatVerbose[0];
    olchar_t strLeft[MAX_OUTPUT_LINE_LEN], strRight[MAX_OUTPUT_LINE_LEN];

    /* Name */
    cliengPrintOneFullLine(pcc, stat->ds_strName);
    pcc += 1;

    /* NumOfTradingDay */
    ol_sprintf(strLeft, "%d", stat->ds_nNumOfDay);
    ol_sprintf(strRight, "%d", stat->ds_nDayForTrend);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* StartDate */
    cliengPrintTwoHalfLine(pcc, stat->ds_strStartDate, stat->ds_strEndDate);
    pcc += 2;

    /* MinClosingPriceRate */
    ol_sprintf(strLeft, "%.2f%%", stat->ds_dbMinClosingPriceRate);
    ol_sprintf(strRight, "%.2f%%", stat->ds_dbMaxClosingPriceRate);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* MinTunePercent */
    ol_sprintf(strLeft, "%.2f%%", stat->ds_dbMinTunePercent);
    ol_sprintf(strRight, "%.2f%%", stat->ds_dbMaxTunePercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* CloseHighLimit*/
    ol_sprintf(strLeft, "%s", getStringPositive(stat->ds_bCloseHighLimit));
    cliengPrintTwoHalfLine(pcc, strLeft, stat->ds_strLastTimeCloseHighLimit);
    pcc += 2;

    /* MaxTimeOpenCloseHighLimit */
    ol_sprintf(strLeft, "%d", stat->ds_nMaxTimeOpenCloseHighLimit);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MinBuyPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->ds_dbMinBuyPercent);
    ol_sprintf(strRight, "%.2f%%", stat->ds_dbMaxBuyPercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveBuyPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->ds_dbAveBuyPercent);
    ol_sprintf(strRight, "%.2f%%", stat->ds_dbMinAveBuyPercentForTrend);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* MinSoldPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->ds_dbMinSoldPercent);
    ol_sprintf(strRight, "%.2f%%", stat->ds_dbMaxSoldPercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveSoldPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->ds_dbAveSoldPercent);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MinLambBuyPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->ds_dbMinLambBuyPercent);
    ol_sprintf(strRight, "%.2f%%", stat->ds_dbMaxLambBuyPercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveLambBuyPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->ds_dbAveLambBuyPercent);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MinLambSoldPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->ds_dbMinLambSoldPercent);
    ol_sprintf(strRight, "%.2f%%", stat->ds_dbMaxLambSoldPercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveLambSoldPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->ds_dbAveLambSoldPercent);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* BuyInAm */
    ol_sprintf(strLeft, "%s", getStringPositive(stat->ds_bBuyInAm));
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MaxVolumeRatio */
    ol_sprintf(strLeft, "%.2f", stat->ds_dbMaxVolumeRatio);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MaxSoldVolumeRatio */
    ol_sprintf(strLeft, "%.2f", stat->ds_dbMaxSoldVolumeRatio);
    ol_sprintf(strRight, "%.2f", stat->ds_dbMaxLambSoldVolumeRatio);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* MaxCloseHighLimitVolumeRatio */
    ol_sprintf(strLeft, "%.2f", stat->ds_dbMaxCloseHighLimitVolumeRatio);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MaxCloseHighLimitSoldVolumeRatio */
    ol_sprintf(strLeft, "%.2f", stat->ds_dbMaxCloseHighLimitSoldVolumeRatio);
    ol_sprintf(strRight, "%.2f", stat->ds_dbMaxCloseHighLimitLambSoldVolumeRatio);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* MinBuyA */
    ol_sprintf(strLeft, "%llu", stat->ds_u64MinBuyA);
    ol_sprintf(strRight, "%llu", stat->ds_u64MaxBuyA);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveBuyA */
    ol_sprintf(strLeft, "%llu", stat->ds_u64AveBuyA);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MinSoldA */
    ol_sprintf(strLeft, "%llu", stat->ds_u64MinSoldA);
    ol_sprintf(strRight, "%llu", stat->ds_u64MaxSoldA);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveSoldA */
    ol_sprintf(strLeft, "%llu", stat->ds_u64AveSoldA);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MinLambBuyA */
    ol_sprintf(strLeft, "%llu", stat->ds_u64MinLambBuyA);
    ol_sprintf(strRight, "%llu", stat->ds_u64MaxLambBuyA);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveLambBuyA */
    ol_sprintf(strLeft, "%llu", stat->ds_u64AveLambBuyA);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MinLambSoldA */
    ol_sprintf(strLeft, "%llu", stat->ds_u64MinLambSoldA);
    ol_sprintf(strRight, "%llu", stat->ds_u64MaxLambSoldA);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveLambSoldA */
    ol_sprintf(strLeft, "%llu", stat->ds_u64AveLambSoldA);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MaxUpperShadowRatio */
    ol_sprintf(strLeft, "%.2f", stat->ds_dbMaxUpperShadowRatio);
    ol_sprintf(strRight, "%.2f", stat->ds_dbMaxLowerShadowRatio);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* MaxLastXInc */
    ol_sprintf(strLeft, "%.2f%%", stat->ds_dbMaxLastXInc);
    ol_sprintf(strRight, "%s %s",
         stat->ds_strLastTimeLowPriceDate, stat->ds_strLastTimeLowPrice);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    cliengOutputLine("");
}

/*WARNING: data is changed when sorting*/
u32 descStatFromData(desc_stat_t * stat, oldouble_t * pdbdata, olint_t num)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i, m, n;
    oldouble_t diff;
    oldouble_t * data = NULL;

    u32Ret = dupMemory((void **)&data, (const u8 *)pdbdata, sizeof(oldouble_t) * num);
    if (u32Ret != OLERR_NO_ERROR)
        return u32Ret;

    memset(stat, 0, sizeof(*stat));

    qsort(data, num, sizeof(oldouble_t), _compareData);

    stat->ds_nCount = num;

    for (i = 0; i < num; i++)
    {
        stat->ds_dbAll += data[i];
//        ol_printf(" %.3f", data[i]);
    }
    stat->ds_dbMean = stat->ds_dbAll / num;

    for (i = 0; i < num; i++)
    {
        diff = data[i] - stat->ds_dbMean;

        stat->ds_dbVariance += diff * diff;
    }
    stat->ds_dbVariance /= num - 1;
    stat->ds_dbStDev = sqrt(stat->ds_dbVariance);
    stat->ds_dbSEMean = stat->ds_dbStDev / sqrt(stat->ds_nCount);

    stat->ds_dbMin = data[0];
    stat->ds_dbMax = data[num - 1];

    if (num % 2)
        stat->ds_dbMedian = data[num / 2];
    else
        stat->ds_dbMedian = (data[num / 2 - 1] + data[num / 2]) / 2;

    m = num / 2;
    if (m % 2)
        stat->ds_dbQ1 = data[m / 2];
    else
        stat->ds_dbQ1 = (data[m / 2 - 1] + data[m / 2]) / 2;

    if (m % 2)
        stat->ds_dbQ3 = data[m + m / 2];
    else
        stat->ds_dbQ3 = (data[m + m / 2 - 1] + data[m + m / 2]) / 2;

    m = n = 0;
    for (i = 0; i < num; i++)
    {
        if ((data[i] >= stat->ds_dbMean - stat->ds_dbStDev) &&
            (data[i] <= stat->ds_dbMean + stat->ds_dbStDev))
            n ++;
        if ((data[i] >= stat->ds_dbMean - 2 * stat->ds_dbStDev) &&
            (data[i] <= stat->ds_dbMean + 2 * stat->ds_dbStDev))
            m ++;
    }

    stat->ds_dbStDev1Percent = n;
    stat->ds_dbStDev1Percent /= num;
    stat->ds_dbStDev2Percent = m;
    stat->ds_dbStDev2Percent /= num;

    xfree((void **)&data);

    return u32Ret;
}

void printDescStatVerbose(desc_stat_t * stat)
{
    clieng_caption_t * pcc = &ls_ccDescStatVerbose[0];
    olchar_t strLeft[MAX_OUTPUT_LINE_LEN], strRight[MAX_OUTPUT_LINE_LEN];

    /* Count */
    ol_sprintf(strLeft, "%d", stat->ds_nCount);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* All */
    ol_sprintf(strLeft, "%.3f", stat->ds_dbAll);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* Mean */
    ol_sprintf(strLeft, "%.3f", stat->ds_dbMean);
    ol_sprintf(strRight, "%.3f", stat->ds_dbSEMean);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* Variance */
    ol_sprintf(strLeft, "%.3f", stat->ds_dbVariance);
    ol_sprintf(strRight, "%.3f", stat->ds_dbStDev);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* Min */
    ol_sprintf(strLeft, "%.3f", stat->ds_dbMin);
    ol_sprintf(strRight, "%.3f", stat->ds_dbQ1);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* Median */
    ol_sprintf(strLeft, "%.3f", stat->ds_dbMedian);
    ol_sprintf(strRight, "%.3f", stat->ds_dbQ3);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* Max */
    ol_sprintf(strLeft, "%.3f", stat->ds_dbMax);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /* StDev1Percent */
    ol_sprintf(strLeft, "%.3f", stat->ds_dbStDev1Percent);
    ol_sprintf(strRight, "%.3f", stat->ds_dbStDev2Percent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 1;

    cliengOutputLine("");
}

void getDoubleFrequency(
    oldouble_t * pdbValue, olint_t num, olint_t numofarea, olint_t * freq, oldouble_t * area)
{
    oldouble_t dbHigh, dbLow;
    olint_t i, j;
    oldouble_t space;

    dbHigh = 0;
    dbLow = (oldouble_t)U64_MAX;
    for (i = 0; i < num; i ++)
    {
        if (dbHigh < pdbValue[i])
            dbHigh = pdbValue[i];
        if (dbLow > pdbValue[i])
            dbLow = pdbValue[i];
    }

    space = (dbHigh - dbLow) / (numofarea - 1);

    memset(area, 0, sizeof(oldouble_t) * numofarea);
    memset(freq, 0, sizeof(olint_t) * numofarea);

    if (dbLow > space / 2)
        area[0] = dbLow - space / 2;
    for (i = 1; i < numofarea; i ++)
        area[i] = area[i - 1] + space;

    for (i = 0; i < num; i ++)
    {
        j = 0;
        while ((pdbValue[i] >= area[j]) && (j < numofarea))
            j ++;

        freq[--j] ++;
    }
}

void printDoubleFrequencyBrief(olint_t numofarea, olint_t * freq, oldouble_t * area)
{
    clieng_caption_t * pcc = &ls_ccFrequencyBrief[0];
    olchar_t strInfo[MAX_OUTPUT_LINE_LEN], strField[MAX_OUTPUT_LINE_LEN];
    olint_t i;

    cliengPrintHeader(ls_ccFrequencyBrief,
        sizeof(ls_ccFrequencyBrief) / sizeof(clieng_caption_t));

    for (i = 0; i < numofarea - 1; i ++)
    {
        strInfo[0] = '\0';
        pcc = &ls_ccFrequencyBrief[0];

        /* Unit */
        ol_sprintf(strField, "%d", i);
        cliengAppendBriefColumn(pcc, strInfo, strField);
        pcc++;

        ol_sprintf(strField, "%.1f ~ %.1f", area[i], area[i + 1]);
        cliengAppendBriefColumn(pcc, strInfo, strField);
        pcc++;

        ol_sprintf(strField, "%d", freq[i]);
        cliengAppendBriefColumn(pcc, strInfo, strField);
        pcc++;

        cliengOutputLine(strInfo);
    }

    strInfo[0] = '\0';
    pcc = &ls_ccFrequencyBrief[0];

    ol_sprintf(strField, "%d", i);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    ol_sprintf(strField, "%.1f", area[i]);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    ol_sprintf(strField, "%d", freq[i]);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    cliengOutputLine(strInfo);
}

u32 getCorrelation2(
    oldouble_t * pdba, desc_stat_t * pdsa, oldouble_t * pdbb, desc_stat_t * pdsb,
    olint_t num, oldouble_t * pdbr)
{
    u32 u32Ret = OLERR_NO_ERROR;
    oldouble_t dbr = 0;
    olint_t i;

    for (i = 0; i < num; i ++)
    {
        dbr += (pdba[i] - pdsa->ds_dbMean) * (pdbb[i] - pdsb->ds_dbMean);
    }

    dbr /= (pdsa->ds_dbStDev * pdsb->ds_dbStDev) * (num - 1);
    *pdbr = dbr;

    return u32Ret;
}

u32 getCorrelation(
    oldouble_t * pdba, oldouble_t * pdbb, olint_t num, oldouble_t * pdbr)
{
    u32 u32Ret = OLERR_NO_ERROR;
    desc_stat_t dsa, dsb;

    u32Ret = descStatFromData(&dsa, pdba, num);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = descStatFromData(&dsb, pdbb, num);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = getCorrelation2(pdba, &dsa, pdbb, &dsb, num, pdbr);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


