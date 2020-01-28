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

#define TX_DS_STRAIGHT_LINE_MOTION     (0.03)

#define TX_DS_VOL_RATIO_NORMAL         (0.8)
#define TX_DS_VOL_RATIO_MODERATE       (1.5)
#define TX_DS_VOL_RATIO_DISTINCT       (2.5)
#define TX_DS_VOL_RATIO_FIERCE         (5.0)
#define TX_DS_VOL_RATIO_ABNORMAL       (10.0)


/* --- data structures -------------------------------------------------------------------------- */

struct tx_indi_dmi;
struct tx_indi_dmi_param;
struct tx_indi_macd;
struct tx_indi_macd_param;
struct tx_indi_mtm;
struct tx_indi_mtm_param;
struct tx_indi_rsi;
struct tx_indi_rsi_param;
struct tx_indi_kdj;
struct tx_indi_kdj_param;
struct tx_indi_asi;
struct tx_indi_asi_param;
struct tx_indi_atr;
struct tx_indi_atr_param;
struct tx_indi_obv;
struct tx_indi_obv_param;

typedef struct
{
    /**Index of a series of day summary.*/
    olint_t td_nIndex;
    /**Trading date.*/
    olchar_t td_strDate[16];

    /**Opening price.*/
    oldouble_t td_dbOpeningPrice;
    /**Closing price.*/
    oldouble_t td_dbClosingPrice;
    /**High price.*/
    oldouble_t td_dbHighPrice;
    /**Low price.*/
    oldouble_t td_dbLowPrice;

    /**Last closing price, exlude right*/
    oldouble_t td_dbLastClosingPrice;

    oldouble_t td_dbClosingPriceRate;
    oldouble_t td_dbHighPriceRate;

    /**Close price reach high limit.*/
    boolean_t td_bCloseHighLimit;
    /**Close price reach low limit.*/
    boolean_t td_bCloseLowLimit;
    /**High price reach high limit.*/
    boolean_t td_bHighHighLimit;
    /**Low price reach low limit.*/
    boolean_t td_bLowLowLimit;
    /**Gap between 2 days.*/
    boolean_t td_bGap;
    u8 td_u8Reserved[3];

    /**Volume.*/
    u64 td_u64All;
    oldouble_t td_dbVolumeRatio;
    oldouble_t td_dbTurnoverRate;

    /**Amount.*/
    u64 td_u64AllA;

    /**Upper shadow, lower shadow.*/
    oldouble_t td_dbUpperShadowRatio;
    oldouble_t td_dbLowerShadowRatio;

    /**General captial of the stock.*/
    u64 td_u64GeneralCapital;
    /**Tradable share of the stock.*/
    u64 td_u64TradableShare;
    /**Exclude right.*/
    boolean_t td_bXR;
    /**Exclude divident.*/
    boolean_t td_bXD;
    /**Exclude divident and right.*/
    boolean_t td_bDR;
    /**Special treatment.*/
    boolean_t td_bSt;
    /**Special treatment, delisting.*/
    boolean_t td_bStDelisting;
    u8 td_u8Reserved2[3];

    tx_dr_t * td_ptdResult;

    /**The DMI indicator.*/
    struct tx_indi_dmi * td_ptidDmi;
    /**The parameter for calculating the DMI indictor, only 1 copy is reserved.*/
    struct tx_indi_dmi_param * td_ptidpDmiParam;
    /**The MACD indicator.*/
    struct tx_indi_macd * td_ptimMacd;
    /**The parameter for calculating the MACD indictor, only 1 copy is reserved.*/
    struct tx_indi_macd_param * td_ptimpMacdParam;
    struct tx_indi_mtm * td_ptimMtm;
    struct tx_indi_mtm_param * td_ptimpMtmParam;
    struct tx_indi_rsi * td_ptirRsi;
    struct tx_indi_rsi_param * td_ptirpRsiParam;
    struct tx_indi_kdj * td_ptikKdj;
    struct tx_indi_kdj_param * td_ptikpKdjParam;
    struct tx_indi_asi * td_ptiaAsi;
    struct tx_indi_asi_param * td_ptiapAsiParam;
    struct tx_indi_atr * td_ptiaAtr;
    struct tx_indi_atr_param * td_ptiapAtrParam;
    struct tx_indi_obv * td_ptioObv;
    struct tx_indi_obv_param * td_ptiopObvParam;
} tx_ds_t;

/* --- functional routines ---------------------------------------------------------------------- */

tx_ds_t * tx_ds_getDsWithHighestClosingPrice(tx_ds_t * buffer, olint_t num);

tx_ds_t * tx_ds_getDsWithLowestClosingPrice(tx_ds_t * buffer, olint_t num);

tx_ds_t * tx_ds_getDsWithLowestLowPrice(tx_ds_t * buffer, olint_t num);

tx_ds_t * tx_ds_getDsWithHighestHighPrice(tx_ds_t * buffer, olint_t num);

/*return NULL if the date has no trade*/
u32 tx_ds_getDsWithDate(
    tx_ds_t * buffer, olint_t num, olchar_t * strdate, tx_ds_t ** ret);

/*return the next day if the date has no trade*/
tx_ds_t * tx_ds_getDsWithDate2(tx_ds_t * buffer, olint_t num, olchar_t * stardate);

/*return the prior day if the date has no trade*/
tx_ds_t * tx_ds_getDsWithDate3(tx_ds_t * buffer, olint_t num, olchar_t * stardate);

void tx_ds_getDsInflexionPoint(tx_ds_t * buffer, olint_t num, tx_ds_t ** ppp, olint_t * nump);

void tx_ds_adjustDsInflexionPoint(tx_ds_t ** ppp, olint_t * nump, olint_t adjust);

void tx_ds_getDsInflexionPoint2(tx_ds_t * buffer, olint_t num, tx_ds_t ** ppp, olint_t * nump);

void tx_ds_locateInflexionPointRegion(
    tx_ds_t ** ppp, olint_t nump, tx_ds_t * dest, tx_ds_t ** start, tx_ds_t ** end);

tx_ds_t * tx_ds_getInflexionPointWithHighestClosingPrice(tx_ds_t ** ppp, olint_t nump);

/** Read trade day summary until the last trading day
 */
u32 tx_ds_readDs(olchar_t * pstrDataDir, tx_ds_t * buffer, olint_t * numofresult);

u32 tx_ds_readDsWithFRoR(olchar_t * pstrDataDir, tx_ds_t * buffer, olint_t * numofresult);

/** Read trade day summary from specified date, if pstrDateFrom is NULL, read the first trading day
 */
u32 tx_ds_readDsFromDate(
    olchar_t * pstrDataDir, olchar_t * pstrDateFrom, tx_ds_t * buffer, olint_t * numofresult);
u32 tx_ds_readDsFromDateWithFRoR(
    olchar_t * pstrDataDir, olchar_t * pstrDateFrom, tx_ds_t * buffer, olint_t * numofresult);

/** Read trade day summary until end date.
 *  
 *  @note Read maximum u32Count trading day after end date, so buy and sell operation are in
 *   the same page for DR.
 *  @note pstrEndDate cannot be NULL.
 */
u32 tx_ds_readDsUntilDate(
    olchar_t * pstrDataDir, olchar_t * pstrEndDate, u32 u32Count, tx_ds_t * buffer,
    olint_t * numofresult);

u32 tx_ds_readDsUntilDateWithFRoR(
    olchar_t * pstrDataDir, olchar_t * pstrEndDate, u32 u32Count, tx_ds_t * buffer,
    olint_t * numofresult);

olint_t tx_ds_countDsWithEndData(tx_ds_t * buffer, olint_t num, olchar_t * pstrEndDate);

boolean_t tx_ds_isStraightLineMotion(tx_ds_t * buffer, olint_t total, olint_t ndays);

boolean_t tx_ds_isStraightHighLimitDay(tx_ds_t * ptd);

void tx_ds_getStringDsStatus(tx_ds_t * summary, olchar_t * pstr);

#endif /*TANGXUN_JIUTAI_DAYSUMMARY_H*/

/*------------------------------------------------------------------------------------------------*/


