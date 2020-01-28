/**
 *  @file tx_datastat.c
 *
 *  @brief Data statistic.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <math.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <stdlib.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_string.h"
#include "jf_mem.h"

#include "tx_datastat.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

static olint_t _compareData(const void * a, const void * b)
{
    oldouble_t ra, rb;

    ra = *(oldouble_t *)a;
    rb = *(oldouble_t *)b;

    return ra > rb ? 1 : -1;
}

static void _dsPriceFromDayResult(
    tx_datastat_daysummary_param_t * ptddp, tx_datastat_daysummary_t * stat, tx_ds_t * buffer, olint_t num)
{
    tx_ds_t * summary, * prev, * end = buffer + num - 1;
    tx_ds_t * high;
    olint_t i;
    oldouble_t dblow = 0.0;

    summary = buffer;
    summary ++;
    stat->tdd_dbMinClosingPriceRate = 100;
    stat->tdd_dbMinClosingPriceRate = 100;
    for (i = 1; i < num; i ++)
    {
        if (summary->td_bCloseHighLimit)
        {
            stat->tdd_bCloseHighLimit = TRUE;
            if ((stat->tdd_strLastTimeCloseHighLimit[0] == '\0') ||
                (strcmp(stat->tdd_strLastTimeCloseHighLimit,
                        summary->td_ptdResult->td_strTimeCloseHighLimit) < 0))
                ol_strcpy(stat->tdd_strLastTimeCloseHighLimit,
                       summary->td_ptdResult->td_strTimeCloseHighLimit);

            if (stat->tdd_nMaxTimeOpenCloseHighLimit <
                summary->td_ptdResult->td_nMaxTimeOpenCloseHighLimit)
                stat->tdd_nMaxTimeOpenCloseHighLimit =
                    summary->td_ptdResult->td_nMaxTimeOpenCloseHighLimit;
        }

		if (stat->tdd_dbMinClosingPriceRate > summary->td_dbClosingPriceRate)
			stat->tdd_dbMinClosingPriceRate = summary->td_dbClosingPriceRate;
		if (stat->tdd_dbMaxClosingPriceRate < summary->td_dbClosingPriceRate)
			stat->tdd_dbMaxClosingPriceRate = summary->td_dbClosingPriceRate;

        summary ++;
    }

    prev = summary = buffer;
    summary ++;
    stat->tdd_dbMinTunePercent = 100;
    while (summary <= end)
    {
        if (summary->td_dbClosingPrice < prev->td_dbClosingPrice)
        {
            high = prev;
            dblow = prev->td_dbClosingPrice;

            while (summary <= end)
            {
                if (dblow > summary->td_dbLowPrice)
                    dblow = summary->td_dbLowPrice;

                if (high->td_dbClosingPrice < summary->td_dbClosingPrice)
                {
                    dblow = (high->td_dbClosingPrice - dblow) * 100 /
                        high->td_dbClosingPrice;

                    if (stat->tdd_dbMinTunePercent > dblow)
                        stat->tdd_dbMinTunePercent = dblow;
                    if (stat->tdd_dbMaxTunePercent < dblow)
                        stat->tdd_dbMaxTunePercent = dblow;

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
    tx_datastat_daysummary_param_t * ptddp, tx_datastat_daysummary_t * stat,
    tx_ds_t * buffer, olint_t num, boolean_t bWithCloseHighLimit)
{
    tx_ds_t * summary, * end;
	tx_dr_t * result;
    olint_t i, j, count;
    oldouble_t dbtmp = 0.0;

    summary = buffer;
	result = summary->td_ptdResult;
    count = 0;
    stat->tdd_nDayForTrend = ptddp->tddp_nDayForTrend;
    stat->tdd_u64MinBuy = result->td_u64Buy;
    stat->tdd_u64MinSold = result->td_u64Sold;
    stat->tdd_u64MinLambBuy = result->td_u64LambBuy;
    stat->tdd_u64MinLambSold = result->td_u64LambSold;
    stat->tdd_dbMinBuyPercent = result->td_dbBuyPercent;
    stat->tdd_dbMinAveBuyPercentForTrend = 100;
    stat->tdd_dbMinSoldPercent = result->td_dbSoldPercent;
    stat->tdd_dbMinLambBuyPercent = result->td_dbLambBuyPercent;
    stat->tdd_dbMinLambSoldPercent = result->td_dbLambSoldPercent;
    stat->tdd_bBuyInAm = TRUE;
    for (i = 0; i < num; i ++)
    {
		result = summary->td_ptdResult;
        if (summary->td_bCloseHighLimit)
        {
            if (stat->tdd_dbMaxCloseHighLimitVolumeRatio < summary->td_dbVolumeRatio)
                stat->tdd_dbMaxCloseHighLimitVolumeRatio = summary->td_dbVolumeRatio;

            if (stat->tdd_dbMaxCloseHighLimitSoldVolumeRatio < result->td_dbSoldVolumeRatio)
                stat->tdd_dbMaxCloseHighLimitSoldVolumeRatio = result->td_dbSoldVolumeRatio;

            if (stat->tdd_dbMaxCloseHighLimitLambSoldVolumeRatio < result->td_dbLambSoldVolumeRatio)
                stat->tdd_dbMaxCloseHighLimitLambSoldVolumeRatio = result->td_dbLambSoldVolumeRatio;
        }
        else
        {
            if (stat->tdd_dbMaxVolumeRatio < summary->td_dbVolumeRatio)
                stat->tdd_dbMaxVolumeRatio = summary->td_dbVolumeRatio;

            if (stat->tdd_dbMaxSoldVolumeRatio < result->td_dbSoldVolumeRatio)
                stat->tdd_dbMaxSoldVolumeRatio = result->td_dbSoldVolumeRatio;

            if (stat->tdd_dbMaxLambSoldVolumeRatio < result->td_dbLambSoldVolumeRatio)
                stat->tdd_dbMaxLambSoldVolumeRatio = result->td_dbLambSoldVolumeRatio;
        }

        if (stat->tdd_u64MinBuy > result->td_u64Buy)
            stat->tdd_u64MinBuy = result->td_u64Buy;
        if (stat->tdd_u64MaxBuy < result->td_u64Buy)
            stat->tdd_u64MaxBuy = result->td_u64Buy;

        if (stat->tdd_u64MinSold > result->td_u64Sold)
            stat->tdd_u64MinSold = result->td_u64Sold;
        if (stat->tdd_u64MaxSold < result->td_u64Sold)
            stat->tdd_u64MaxSold = result->td_u64Sold;

        if (stat->tdd_u64MinLambBuy > result->td_u64LambBuy)
            stat->tdd_u64MinLambBuy = result->td_u64LambBuy;
        if (stat->tdd_u64MaxLambBuy < result->td_u64LambBuy)
            stat->tdd_u64MaxLambBuy = result->td_u64LambBuy;

        if (stat->tdd_u64MinLambSold > result->td_u64LambSold)
            stat->tdd_u64MinLambSold = result->td_u64LambSold;
        if (stat->tdd_u64MaxLambSold < result->td_u64LambSold)
            stat->tdd_u64MaxLambSold = result->td_u64LambSold;

        if (stat->tdd_dbMinBuyPercent > result->td_dbBuyPercent)
            stat->tdd_dbMinBuyPercent = result->td_dbBuyPercent;
        if (stat->tdd_dbMaxBuyPercent < result->td_dbBuyPercent)
            stat->tdd_dbMaxBuyPercent = result->td_dbBuyPercent;

        if (i >= ptddp->tddp_nDayForTrend)
        {
            end = summary;
            dbtmp = 0;
            for (j = 0; j < ptddp->tddp_nDayForTrend; j ++)
            {
                dbtmp += end->td_ptdResult->td_dbBuyPercent;
                end --;
            }
            dbtmp /= ptddp->tddp_nDayForTrend;
            if (stat->tdd_dbMinAveBuyPercentForTrend > dbtmp)
                stat->tdd_dbMinAveBuyPercentForTrend = dbtmp;
        }

        if (stat->tdd_dbMinSoldPercent > result->td_dbSoldPercent)
            stat->tdd_dbMinSoldPercent = result->td_dbSoldPercent;
        if (stat->tdd_dbMaxSoldPercent < result->td_dbSoldPercent)
            stat->tdd_dbMaxSoldPercent = result->td_dbSoldPercent;

        if (stat->tdd_dbMinLambBuyPercent > result->td_dbLambBuyPercent)
            stat->tdd_dbMinLambBuyPercent = result->td_dbLambBuyPercent;
        if (stat->tdd_dbMaxLambBuyPercent < result->td_dbLambBuyPercent)
            stat->tdd_dbMaxLambBuyPercent = result->td_dbLambBuyPercent;

        if (stat->tdd_dbMinLambSoldPercent > result->td_dbLambSoldPercent)
            stat->tdd_dbMinLambSoldPercent = result->td_dbLambSoldPercent;
        if (stat->tdd_dbMaxLambSoldPercent < result->td_dbLambSoldPercent)
            stat->tdd_dbMaxLambSoldPercent = result->td_dbLambSoldPercent;

        if (result->td_dbBuyInAmPercent == 0)
            stat->tdd_bBuyInAm = FALSE;

//        if (bWithCloseHighLimit || ! result->td_bCloseHighLimit)
        {
            stat->tdd_u64AllBuy += result->td_u64Buy + result->td_u64CloseHighLimitSold;
            stat->tdd_u64AllSold += result->td_u64Sold + result->td_u64CloseLowLimitBuy;
            stat->tdd_u64AllLambBuy += result->td_u64LambBuy + result->td_u64CloseHighLimitLambSold;
            stat->tdd_u64AllLambSold += result->td_u64LambSold + result->td_u64CloseLowLimitLambBuy;

            stat->tdd_dbAveBuyPercent += result->td_dbBuyPercent + result->td_dbCloseHighLimitSoldPercent;
            stat->tdd_dbAveSoldPercent += result->td_dbSoldPercent + result->td_dbCloseLowLimitBuyPercent;
            stat->tdd_dbAveLambBuyPercent += result->td_dbLambBuyPercent + result->td_dbCloseHighLimitLambSoldPercent;
            stat->tdd_dbAveLambSoldPercent += result->td_dbLambSoldPercent + result->td_dbCloseLowLimitLambBuyPercent;

            count ++;
        }

        summary ++;
    }

    if (count != 0)
    {
        stat->tdd_u64AveBuy = stat->tdd_u64AllBuy / count;
        stat->tdd_u64AveSold = stat->tdd_u64AllSold / count;
        stat->tdd_u64AveLambBuy = stat->tdd_u64AllLambBuy / count;
        stat->tdd_u64AveLambSold = stat->tdd_u64AllLambSold / count;

        stat->tdd_dbAveBuyPercent /= count;
        stat->tdd_dbAveSoldPercent /= count;
        stat->tdd_dbAveLambBuyPercent /= count;
        stat->tdd_dbAveLambSoldPercent /= count;
    }
}

static void _dsAmountFromDayResult(
    tx_datastat_daysummary_param_t * ptddp, tx_datastat_daysummary_t * stat,
    tx_ds_t * buffer, olint_t num, boolean_t bWithCloseHighLimit)
{
    tx_ds_t * summary;
	tx_dr_t * result;
    olint_t i, count;

    summary = buffer;
	result = summary->td_ptdResult;
    count = 0;
    stat->tdd_u64MinBuyA = result->td_u64BuyA;
    stat->tdd_u64MinSoldA = result->td_u64SoldA;
    stat->tdd_u64MinLambBuyA = result->td_u64LambBuyA;
    stat->tdd_u64MinLambSoldA = result->td_u64LambSoldA;
    for (i = 0; i < num; i ++)
    {
		result = summary->td_ptdResult;
        if (stat->tdd_u64MinBuyA > result->td_u64BuyA)
            stat->tdd_u64MinBuyA = result->td_u64BuyA;
        if (stat->tdd_u64MaxBuyA < result->td_u64BuyA)
            stat->tdd_u64MaxBuyA = result->td_u64BuyA;

        if (stat->tdd_u64MinSoldA > result->td_u64SoldA)
            stat->tdd_u64MinSoldA = result->td_u64SoldA;
        if (stat->tdd_u64MaxSoldA < result->td_u64SoldA)
            stat->tdd_u64MaxSoldA = result->td_u64SoldA;

        if (stat->tdd_u64MinLambBuyA > result->td_u64LambBuyA)
            stat->tdd_u64MinLambBuyA = result->td_u64LambBuyA;
        if (stat->tdd_u64MaxLambBuyA < result->td_u64LambBuyA)
            stat->tdd_u64MaxLambBuyA = result->td_u64LambBuyA;

        if (stat->tdd_u64MinLambSoldA > result->td_u64LambSoldA)
            stat->tdd_u64MinLambSoldA = result->td_u64LambSoldA;
        if (stat->tdd_u64MaxLambSoldA < result->td_u64LambSoldA)
            stat->tdd_u64MaxLambSoldA = result->td_u64LambSoldA;

//        if (bWithCloseHighLimit || ! result->td_bCloseHighLimit)
        {
            stat->tdd_u64AllBuyA += result->td_u64BuyA + result->td_u64CloseHighLimitSoldA;
            stat->tdd_u64AllSoldA += result->td_u64SoldA + result->td_u64CloseLowLimitBuyA;
            stat->tdd_u64AllLambBuyA += result->td_u64LambBuyA + result->td_u64CloseHighLimitLambSoldA;
            stat->tdd_u64AllLambSoldA += result->td_u64LambSoldA + result->td_u64CloseLowLimitLambBuyA;

            count ++;
        }

        summary ++;
    }

    if (count != 0)
    {
        stat->tdd_u64AveBuyA = stat->tdd_u64AllBuyA / count;
        stat->tdd_u64AveSoldA = stat->tdd_u64AllSoldA / count;
        stat->tdd_u64AveLambBuyA = stat->tdd_u64AllLambBuyA / count;
        stat->tdd_u64AveLambSoldA = stat->tdd_u64AllLambSoldA / count;
    }
}

static void _dsOtherFromDayResult(
    tx_datastat_daysummary_param_t * ptddp, tx_datastat_daysummary_t * stat,
    tx_ds_t * buffer, olint_t num)
{
    tx_ds_t * summary;
	tx_dr_t * result;
    olint_t i;

    summary = buffer;
	result = summary->td_ptdResult;
    ol_strcpy(stat->tdd_strLastTimeLowPrice, result->td_strLastTimeLowPrice);
    stat->tdd_dbMaxLastXInc = result->td_dbLastXInc;
    for (i = 0; i < num; i ++)
    {
		result = summary->td_ptdResult;
        if (stat->tdd_dbMaxUpperShadowRatio < summary->td_dbUpperShadowRatio)
            stat->tdd_dbMaxUpperShadowRatio = summary->td_dbUpperShadowRatio;
        if (stat->tdd_dbMaxLowerShadowRatio < summary->td_dbLowerShadowRatio)
            stat->tdd_dbMaxLowerShadowRatio = summary->td_dbLowerShadowRatio;

        if (stat->tdd_dbMaxLastXInc < result->td_dbLastXInc)
        {
            stat->tdd_dbMaxLastXInc = result->td_dbLastXInc;
            ol_strcpy(stat->tdd_strLastTimeLowPrice, result->td_strLastTimeLowPrice);
            ol_strcpy(stat->tdd_strLastTimeLowPriceDate, result->td_strDate);
        }

        summary ++;
    }

}

/* --- public routine section ------------------------------------------------------------------- */

u32 tx_datastat_statDaySummary(
    tx_datastat_daysummary_param_t * ptddp, tx_datastat_daysummary_t * stat,
    tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * summary = buffer, * end = buffer + num - 1;

    assert(ptddp != NULL);
    assert((ptddp->tddp_pstrName != NULL) && (ptddp->tddp_nDayForTrend != 0));

    memset(stat, 0, sizeof(*stat));

    if (num < 2)
        return JF_ERR_INVALID_DATA;

    ol_strncpy(stat->tdd_strName, ptddp->tddp_pstrName, 15);
    stat->tdd_nNumOfDay = num;
    ol_strcpy(stat->tdd_strStartDate, summary->td_strDate);
    ol_strcpy(stat->tdd_strEndDate, end->td_strDate);

    _dsPriceFromDayResult(ptddp, stat, buffer, num);
    _dsVolFromDayResult(ptddp, stat, buffer, num, ! ptddp->tddp_bWithoutCloseHighLimit);
    _dsAmountFromDayResult(ptddp, stat, buffer, num, ! ptddp->tddp_bWithoutCloseHighLimit);
    _dsOtherFromDayResult(ptddp, stat, buffer, num);

    return u32Ret;
}

/*WARNING: data is changed when sorting*/
u32 tx_datastat_descData(tx_datastat_desc_t * stat, oldouble_t * pdbdata, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, m, n;
    oldouble_t diff;
    oldouble_t * data = NULL;

    u32Ret = jf_mem_duplicate((void **)&data, (const u8 *)pdbdata, sizeof(oldouble_t) * num);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    ol_memset(stat, 0, sizeof(*stat));

    qsort(data, num, sizeof(oldouble_t), _compareData);

    stat->tdd_nCount = num;

    for (i = 0; i < num; i++)
    {
        stat->tdd_dbAll += data[i];
//        ol_printf(" %.3f", data[i]);
    }
    stat->tdd_dbMean = stat->tdd_dbAll / num;

    for (i = 0; i < num; i++)
    {
        diff = data[i] - stat->tdd_dbMean;

        stat->tdd_dbVariance += diff * diff;
    }
    stat->tdd_dbVariance /= num - 1;
    stat->tdd_dbStDev = sqrt(stat->tdd_dbVariance);
    stat->tdd_dbSEMean = stat->tdd_dbStDev / sqrt(stat->tdd_nCount);

    stat->tdd_dbMin = data[0];
    stat->tdd_dbMax = data[num - 1];

    if (num % 2)
        stat->tdd_dbMedian = data[num / 2];
    else
        stat->tdd_dbMedian = (data[num / 2 - 1] + data[num / 2]) / 2;

    m = num / 2;
    if (m % 2)
        stat->tdd_dbQ1 = data[m / 2];
    else
        stat->tdd_dbQ1 = (data[m / 2 - 1] + data[m / 2]) / 2;

    if (m % 2)
        stat->tdd_dbQ3 = data[m + m / 2];
    else
        stat->tdd_dbQ3 = (data[m + m / 2 - 1] + data[m + m / 2]) / 2;

    m = n = 0;
    for (i = 0; i < num; i++)
    {
        if ((data[i] >= stat->tdd_dbMean - stat->tdd_dbStDev) &&
            (data[i] <= stat->tdd_dbMean + stat->tdd_dbStDev))
            n ++;
        if ((data[i] >= stat->tdd_dbMean - 2 * stat->tdd_dbStDev) &&
            (data[i] <= stat->tdd_dbMean + 2 * stat->tdd_dbStDev))
            m ++;
    }

    stat->tdd_dbStDev1Percent = n;
    stat->tdd_dbStDev1Percent /= num;
    stat->tdd_dbStDev2Percent = m;
    stat->tdd_dbStDev2Percent /= num;

    jf_mem_free((void **)&data);

    return u32Ret;
}

void tx_datastat_getDoubleFrequency(
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

u32 tx_datastat_getCorrelation2(
    oldouble_t * pdba, tx_datastat_desc_t * ptdda, oldouble_t * pdbb, tx_datastat_desc_t * ptddb,
    olint_t num, oldouble_t * pdbr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t dbr = 0;
    olint_t i;

    for (i = 0; i < num; i ++)
    {
        dbr += (pdba[i] - ptdda->tdd_dbMean) * (pdbb[i] - ptddb->tdd_dbMean);
    }

    dbr /= (ptdda->tdd_dbStDev * ptddb->tdd_dbStDev) * (num - 1);
    *pdbr = dbr;

    return u32Ret;
}

u32 tx_datastat_getCorrelation(
    oldouble_t * pdba, oldouble_t * pdbb, olint_t num, oldouble_t * pdbr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_datastat_desc_t tdda, tddb;

    u32Ret = tx_datastat_descData(&tdda, pdba, num);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = tx_datastat_descData(&tddb, pdbb, num);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = tx_datastat_getCorrelation2(pdba, &tdda, pdbb, &tddb, num, pdbr);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


