/**
 *  @file tx_daysummary.h
 *
 *  @brief Define the day summary data structure.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_DAYSUMMARY_H
#define TANGXUN_JIUTAI_DAYSUMMARY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"

#include "tx_dayresult.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

struct da_adxr;
struct da_dmi;
struct da_macd;
struct da_mtm;
struct da_rsi;
struct da_kdj;
struct da_asi;
struct da_atr;
struct da_obv;

typedef struct
{
    /**Index of a series of day summary.*/
    olint_t dds_nIndex;
    olchar_t dds_strDate[16];

    /**Price*/
    oldouble_t dds_dbOpeningPrice;
    oldouble_t dds_dbClosingPrice;
    oldouble_t dds_dbHighPrice;
    oldouble_t dds_dbLowPrice;

    /**Last closing price, exlude right*/
    oldouble_t dds_dbLastClosingPrice;

    oldouble_t dds_dbClosingPriceRate;
    oldouble_t dds_dbHighPriceRate;

    /**Close price reach high limit.*/
    boolean_t dds_bCloseHighLimit;
    /**Close price reach low limit.*/
    boolean_t dds_bCloseLowLimit;
    /**High price reach high limit.*/
    boolean_t dds_bHighHighLimit;
    /**Low price reach low limit.*/
    boolean_t dds_bLowLowLimit;
    /**Gap between 2 days.*/
    boolean_t dds_bGap;
    u8 dds_u8Reserved[3];

    /**Volume.*/
    u64 dds_u64All;
    oldouble_t dds_dbVolumeRatio;
    oldouble_t dds_dbTurnoverRate;

    /**Amount.*/
    u64 dds_u64AllA;

    /**Upper shadow, lower shadow*/
    oldouble_t dds_dbUpperShadowRatio;
    oldouble_t dds_dbLowerShadowRatio;

    /**general captial of the stock*/
    u64 dds_u64GeneralCapital;
    /*tradable share of the stock*/
    u64 dds_u64TradableShare;
    /**exclude right*/
    boolean_t dds_bXR;
    /**exclude divident*/
    boolean_t dds_bXD;
    /**exclude divident and right*/
    boolean_t dds_bDR;
    /**special treatment*/
    boolean_t dds_bSt;
    /**special treatment, delisting*/
    boolean_t dds_bStDelisting;
    u8 dds_u8Reserved2[3];

    da_day_result_t * dds_ddrResult;

    struct da_adxr * dds_pdaAdxr;
    struct da_dmi * dds_pddDmi;
    struct da_macd * dds_pdmMacd;
    struct da_mtm * dds_pdmMtm;
    struct da_rsi * dds_pdrRsi;
    struct da_kdj * dds_pdkKdj;
    struct da_asi * dds_pdaAsi;
    struct da_atr * dds_pdaAtr;
    struct da_obv * dds_pdoObv;
} da_day_summary_t;

/* --- functional routines ---------------------------------------------------------------------- */

da_day_summary_t * getDaySummaryWithHighestClosingPrice(da_day_summary_t * buffer, olint_t num);

da_day_summary_t * getDaySummaryWithLowestClosingPrice(da_day_summary_t * buffer, olint_t num);

da_day_summary_t * getDaySummaryWithLowestLowPrice(da_day_summary_t * buffer, olint_t num);

da_day_summary_t * getDaySummaryWithHighestHighPrice(da_day_summary_t * buffer, olint_t num);

/*return NULL if the date has no trade*/
u32 getDaySummaryWithDate(
    da_day_summary_t * buffer, olint_t num, olchar_t * strdate, da_day_summary_t ** ret);
/*return the next day if the date has no trade*/
da_day_summary_t * getDaySummaryWithDate2(
    da_day_summary_t * buffer, olint_t num, olchar_t * stardate);
/*return the prior day if the date has no trade*/
da_day_summary_t * getDaySummaryWithDate3(
    da_day_summary_t * buffer, olint_t num, olchar_t * stardate);

void getDaySummaryInflexionPoint(
    da_day_summary_t * buffer, olint_t num, da_day_summary_t ** ppp, olint_t * nump);
void adjustDaySummaryInflexionPoint(
    da_day_summary_t ** ppp, olint_t * nump, olint_t adjust);
void getDaySummaryInflexionPoint2(
    da_day_summary_t * buffer, olint_t num, da_day_summary_t ** ppp, olint_t * nump);
void locateInflexionPointRegion(
    da_day_summary_t ** ppp, olint_t nump, da_day_summary_t * dest, da_day_summary_t ** start,
    da_day_summary_t ** end);

da_day_summary_t * getInflexionPointWithHighestClosingPrice(da_day_summary_t ** ppp, olint_t nump);

/** Read trade day summary until the last trading day
 */
u32 readTradeDaySummary(
    olchar_t * pstrDataDir, da_day_summary_t * buffer, olint_t * numofresult);
u32 readTradeDaySummaryWithFRoR(
    olchar_t * pstrDataDir, da_day_summary_t * buffer, olint_t * numofresult);

/** Read trade day summary from specified date, if pstrDateFrom is NULL, read the first trading day
 */
u32 readTradeDaySummaryFromDate(
    olchar_t * pstrDataDir, olchar_t * pstrDateFrom, da_day_summary_t * buffer,
    olint_t * numofresult);
u32 readTradeDaySummaryFromDateWithFRoR(
    olchar_t * pstrDataDir, olchar_t * pstrDateFrom, da_day_summary_t * buffer,
    olint_t * numofresult);

/** Read trade day summary until end date.
 *  
 *  @note Read maximum u32Count trading day after end date, so buy and sell operation are in
 *   the same page for DR.
 *  @note pstrEndDate cannot be NULL.
 */
u32 readTradeDaySummaryUntilDate(
    olchar_t * pstrDataDir, olchar_t * pstrEndDate, u32 u32Count, da_day_summary_t * buffer,
    olint_t * numofresult);
u32 readTradeDaySummaryUntilDateWithFRoR(
    olchar_t * pstrDataDir, olchar_t * pstrEndDate, u32 u32Count, da_day_summary_t * buffer,
    olint_t * numofresult);

olint_t daySummaryEndDataCount(
    da_day_summary_t * buffer, olint_t num, olchar_t * pstrEndDate);

boolean_t isStraightLineMotion(
    da_day_summary_t * buffer, olint_t total, olint_t ndays);

boolean_t isStraightHighLimitDay(da_day_summary_t * pdds);

void getStringDaySummaryStatus(da_day_summary_t * summary, olchar_t * pstr);

#endif /*TANGXUN_JIUTAI_DAYSUMMARY_H*/

/*------------------------------------------------------------------------------------------------*/


