/**
 *  @file tx_datastat.h
 *
 *  @brief Data statistic.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_DATASTAT_H
#define TANGXUN_JIUTAI_DATASTAT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

#include "tx_daysummary.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    olchar_t tdd_strName[16];
    olint_t tdd_nNumOfDay;
    olint_t tdd_nDayForTrend;
    olchar_t tdd_strStartDate[16];
    olchar_t tdd_strEndDate[16];

    /*price*/
    oldouble_t tdd_dbMinClosingPriceRate;
    oldouble_t tdd_dbMaxClosingPriceRate;

    oldouble_t tdd_dbMinTunePercent;
    oldouble_t tdd_dbMaxTunePercent;

    boolean_t tdd_bCloseHighLimit;
    u8 tdd_u8Reserved3[7];
    olchar_t tdd_strLastTimeCloseHighLimit[16];
    olint_t tdd_nMaxTimeOpenCloseHighLimit; /* in second */
    olint_t tdd_nReserved5;

    /*volume*/
    u64 tdd_u64AllBuy;
    u64 tdd_u64MinBuy;
    u64 tdd_u64MaxBuy;
    u64 tdd_u64AveBuy;

    u64 tdd_u64AllSold;
    u64 tdd_u64MinSold;
    u64 tdd_u64MaxSold;
    u64 tdd_u64AveSold;

    u64 tdd_u64AllLambBuy;
    u64 tdd_u64MinLambBuy;
    u64 tdd_u64MaxLambBuy;
    u64 tdd_u64AveLambBuy;

    u64 tdd_u64AllLambSold;
    u64 tdd_u64MinLambSold;
    u64 tdd_u64MaxLambSold;
    u64 tdd_u64AveLambSold;

    oldouble_t tdd_dbMinBuyPercent;
    oldouble_t tdd_dbMaxBuyPercent;
    oldouble_t tdd_dbAveBuyPercent;
    oldouble_t tdd_dbMinAveBuyPercentForTrend;

    oldouble_t tdd_dbMinSoldPercent;
    oldouble_t tdd_dbMaxSoldPercent;
    oldouble_t tdd_dbAveSoldPercent;

    oldouble_t tdd_dbMinLambBuyPercent;
    oldouble_t tdd_dbMaxLambBuyPercent;
    oldouble_t tdd_dbAveLambBuyPercent;

    oldouble_t tdd_dbMinLambSoldPercent;
    oldouble_t tdd_dbMaxLambSoldPercent;
    oldouble_t tdd_dbAveLambSoldPercent;

    boolean_t tdd_bBuyInAm;
    u8 tdd_u8Reserved[7];

    oldouble_t tdd_dbMaxVolumeRatio;
    oldouble_t tdd_dbMaxSoldVolumeRatio;
    oldouble_t tdd_dbMaxLambSoldVolumeRatio;
    oldouble_t tdd_dbMaxCloseHighLimitVolumeRatio;
    oldouble_t tdd_dbMaxCloseHighLimitSoldVolumeRatio;
    oldouble_t tdd_dbMaxCloseHighLimitLambSoldVolumeRatio;

    /*amount*/
    u64 tdd_u64AllBuyA;
    u64 tdd_u64MinBuyA;
    u64 tdd_u64MaxBuyA;
    u64 tdd_u64AveBuyA;

    u64 tdd_u64AllSoldA;
    u64 tdd_u64MinSoldA;
    u64 tdd_u64MaxSoldA;
    u64 tdd_u64AveSoldA;

    u64 tdd_u64AllLambBuyA;
    u64 tdd_u64MinLambBuyA;
    u64 tdd_u64MaxLambBuyA;
    u64 tdd_u64AveLambBuyA;

    u64 tdd_u64AllLambSoldA;
    u64 tdd_u64MinLambSoldA;
    u64 tdd_u64MaxLambSoldA;
    u64 tdd_u64AveLambSoldA;

    /*upper shadow, lower shadow*/
    oldouble_t tdd_dbMaxUpperShadowRatio;
    oldouble_t tdd_dbMaxLowerShadowRatio;

    /*last time low price*/
    olchar_t tdd_strLastTimeLowPriceDate[16];
    olchar_t tdd_strLastTimeLowPrice[16];
    oldouble_t tdd_dbMaxLastXInc;

} tx_datastat_daysummary_t;

typedef struct
{
    olchar_t * tddp_pstrName;
    olint_t tddp_nDayForTrend;
    boolean_t tddp_bWithoutCloseHighLimit;
    u8 tddp_u8Reserved[3];
    olint_t tddp_nReserved[6];
} tx_datastat_daysummary_param_t;

typedef struct
{
    olint_t tdd_nCount;

    oldouble_t tdd_dbAll;
    oldouble_t tdd_dbMean;
    oldouble_t tdd_dbSEMean;  // variance / sqrt(count)

    oldouble_t tdd_dbVariance;
    oldouble_t tdd_dbStDev;

    oldouble_t tdd_dbMin;
    oldouble_t tdd_dbQ1;
    oldouble_t tdd_dbMedian;
    oldouble_t tdd_dbQ3;
    oldouble_t tdd_dbMax;

    oldouble_t tdd_dbStDev1Percent;
    oldouble_t tdd_dbStDev2Percent;
} tx_datastat_desc_t;


/* --- functional routines ---------------------------------------------------------------------- */

u32 tx_datastat_statDaySummary(
    tx_datastat_daysummary_param_t * ptddp, tx_datastat_daysummary_t * stat,
    tx_ds_t * buffer, olint_t num);

u32 tx_datastat_descData(tx_datastat_desc_t * stat, oldouble_t * pdbdata, olint_t num);

void tx_datastat_getDoubleFrequency(
    oldouble_t * pdbValue, olint_t num, olint_t numofarea, olint_t * freq, oldouble_t * area);

u32 tx_datastat_getCorrelation2(
    oldouble_t * pdba, tx_datastat_desc_t * ptdda, oldouble_t * pdbb, tx_datastat_desc_t * ptddb,
    olint_t num, oldouble_t * pdbr);

u32 tx_datastat_getCorrelation(
    oldouble_t * pdba, oldouble_t * pdbb, olint_t num, oldouble_t * pdbr);

#endif /*TANGXUN_JIUTAI_DATASTAT_H*/

/*------------------------------------------------------------------------------------------------*/


