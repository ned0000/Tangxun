/**
 *  @file datastat.h
 *
 *  @brief data statistic
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_DATASTAT_H
#define TANGXUN_JIUTAI_DATASTAT_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "parsedata.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    olchar_t ds_strName[16];
    olint_t ds_nNumOfDay;
    olint_t ds_nDayForTrend;
    olchar_t ds_strStartDate[16];
    olchar_t ds_strEndDate[16];

    /*price*/
    oldouble_t ds_dbMinClosingPriceRate;
    oldouble_t ds_dbMaxClosingPriceRate;

    oldouble_t ds_dbMinTunePercent;
    oldouble_t ds_dbMaxTunePercent;

    boolean_t ds_bCloseHighLimit;
    u8 ds_u8Reserved3[7];
    olchar_t ds_strLastTimeCloseHighLimit[16];
    olint_t ds_nMaxTimeOpenCloseHighLimit; /* in second */
    olint_t ds_nReserved5;

    /*volume*/
    u64 ds_u64AllBuy;
    u64 ds_u64MinBuy;
    u64 ds_u64MaxBuy;
    u64 ds_u64AveBuy;

    u64 ds_u64AllSold;
    u64 ds_u64MinSold;
    u64 ds_u64MaxSold;
    u64 ds_u64AveSold;

    u64 ds_u64AllLambBuy;
    u64 ds_u64MinLambBuy;
    u64 ds_u64MaxLambBuy;
    u64 ds_u64AveLambBuy;

    u64 ds_u64AllLambSold;
    u64 ds_u64MinLambSold;
    u64 ds_u64MaxLambSold;
    u64 ds_u64AveLambSold;

    oldouble_t ds_dbMinBuyPercent;
    oldouble_t ds_dbMaxBuyPercent;
    oldouble_t ds_dbAveBuyPercent;
    oldouble_t ds_dbMinAveBuyPercentForTrend;

    oldouble_t ds_dbMinSoldPercent;
    oldouble_t ds_dbMaxSoldPercent;
    oldouble_t ds_dbAveSoldPercent;

    oldouble_t ds_dbMinLambBuyPercent;
    oldouble_t ds_dbMaxLambBuyPercent;
    oldouble_t ds_dbAveLambBuyPercent;

    oldouble_t ds_dbMinLambSoldPercent;
    oldouble_t ds_dbMaxLambSoldPercent;
    oldouble_t ds_dbAveLambSoldPercent;

    boolean_t ds_bBuyInAm;
    u8 ds_u8Reserved[7];

    oldouble_t ds_dbMaxVolumeRatio;
    oldouble_t ds_dbMaxSoldVolumeRatio;
    oldouble_t ds_dbMaxLambSoldVolumeRatio;
    oldouble_t ds_dbMaxCloseHighLimitVolumeRatio;
    oldouble_t ds_dbMaxCloseHighLimitSoldVolumeRatio;
    oldouble_t ds_dbMaxCloseHighLimitLambSoldVolumeRatio;

    /*amount*/
    u64 ds_u64AllBuyA;
    u64 ds_u64MinBuyA;
    u64 ds_u64MaxBuyA;
    u64 ds_u64AveBuyA;

    u64 ds_u64AllSoldA;
    u64 ds_u64MinSoldA;
    u64 ds_u64MaxSoldA;
    u64 ds_u64AveSoldA;

    u64 ds_u64AllLambBuyA;
    u64 ds_u64MinLambBuyA;
    u64 ds_u64MaxLambBuyA;
    u64 ds_u64AveLambBuyA;

    u64 ds_u64AllLambSoldA;
    u64 ds_u64MinLambSoldA;
    u64 ds_u64MaxLambSoldA;
    u64 ds_u64AveLambSoldA;

    /*upper shadow, lower shadow*/
    oldouble_t ds_dbMaxUpperShadowRatio;
    oldouble_t ds_dbMaxLowerShadowRatio;

    /*last time low price*/
    olchar_t ds_strLastTimeLowPriceDate[16];
    olchar_t ds_strLastTimeLowPrice[16];
    oldouble_t ds_dbMaxLastXInc;

} data_stat_t;

typedef struct
{
    olchar_t * dsp_pstrName;
    olint_t dsp_nDayForTrend;
    boolean_t dsp_bWithoutCloseHighLimit;
    u8 dsp_u8Reserved[3];
    olint_t dsp_nReserved[6];
} data_stat_param_t;

typedef struct
{
    olint_t ds_nCount;

    oldouble_t ds_dbAll;
    oldouble_t ds_dbMean;
    oldouble_t ds_dbSEMean;  // variance / sqrt(count)

    oldouble_t ds_dbVariance;
    oldouble_t ds_dbStDev;

    oldouble_t ds_dbMin;
    oldouble_t ds_dbQ1;
    oldouble_t ds_dbMedian;
    oldouble_t ds_dbQ3;
    oldouble_t ds_dbMax;

    oldouble_t ds_dbStDev1Percent;
    oldouble_t ds_dbStDev2Percent;
} desc_stat_t;


/* --- functional routines ------------------------------------------------- */

u32 dataStatFromDaySummary(
    data_stat_param_t * pdsp, data_stat_t * stat,
    da_day_summary_t * buffer, olint_t num);

void printDataStatVerbose(data_stat_t * stat);

u32 descStatFromData(desc_stat_t * stat, oldouble_t * pdbdata, olint_t num);

void printDescStatVerbose(desc_stat_t * stat);

void getDoubleFrequency(
    oldouble_t * pdbValue, olint_t num, olint_t numofarea, olint_t * freq,
    oldouble_t * area);

void printDoubleFrequencyBrief(
    olint_t numofarea, olint_t * freq, oldouble_t * area);

u32 getCorrelation2(
    oldouble_t * pdba, desc_stat_t * pdsa, oldouble_t * pdbb,
    desc_stat_t * pdsb, olint_t num, oldouble_t * pdbr);

u32 getCorrelation(
    oldouble_t * pdba, oldouble_t * pdbb, olint_t num, oldouble_t * pdbr);

#endif /*TANGXUN_JIUTAI_DATASTAT_H*/

/*---------------------------------------------------------------------------*/


