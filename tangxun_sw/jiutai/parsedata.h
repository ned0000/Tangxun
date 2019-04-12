/**
 *  @file parsedata.h
 *
 *  @brief routine for parsing data
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_PARSEDATA_H
#define TANGXUN_JIUTAI_PARSEDATA_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"

/* --- constant definitions ------------------------------------------------ */
#define AMOUNT_UNIT              (10000)

#define STRAIGHT_LINE_MOTION     (0.03)

#define VOL_RATIO_NORMAL         (0.8)
#define VOL_RATIO_MODERATE       (1.5)
#define VOL_RATIO_DISTINCT       (2.5)
#define VOL_RATIO_FIERCE         (5.0)
#define VOL_RATIO_ABNORMAL       (10.0)

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    olchar_t ddr_strDate[16];

    /*price*/
    oldouble_t ddr_dbOpeningPrice;
    oldouble_t ddr_dbClosingPrice;
    oldouble_t ddr_dbHighPrice;
    oldouble_t ddr_dbLowPrice;

    oldouble_t ddr_dbClosingPriceInc;
    oldouble_t ddr_dbClosingPriceDec;
    oldouble_t ddr_dbHighPriceInc;
    oldouble_t ddr_dbLowPriceDec;

    boolean_t ddr_bCloseHighLimit;
    boolean_t ddr_bCloseLowLimit;
    boolean_t ddr_bGap;
    u8 ddr_u8Reserved[5];
    olchar_t ddr_strTimeCloseHighLimit[16];
    olchar_t ddr_strTimeCloseLowLimit[16];
    olint_t ddr_nMaxTimeOpenCloseHighLimit; /* in second */
    olint_t ddr_nReserved5;

    /*volume*/
    u64 ddr_u64All;

    u64 ddr_u64Buy;
    u64 ddr_u64Sold;
    u64 ddr_u64Na; /*don't know it buy or sold*/

    u64 ddr_u64BuyInAm;  /*buy in the morning*/
    u64 ddr_u64SoldInAm;    /*buy in the afternoon*/

    u64 ddr_u64LambBuy;
    u64 ddr_u64LambSold;
    u64 ddr_u64LambNa;

    u64 ddr_u64AboveLastClosingPrice; /*volumes with
                                       higher price than last closing price*/
    u64 ddr_u64LambBuyBelow100;
    u64 ddr_u64LambSoldBelow100;

    u64 ddr_u64CloseHighLimitSold;
    u64 ddr_u64CloseHighLimitLambSold;

    u64 ddr_u64CloseLowLimitBuy;
    u64 ddr_u64CloseLowLimitLambBuy;

    oldouble_t ddr_dbBuyPercent;  /* buy / all */
    oldouble_t ddr_dbSoldPercent;  /* sold / all */

    oldouble_t ddr_dbBuyInAmPercent;
    oldouble_t ddr_dbSoldInAmPercent;

    oldouble_t ddr_dbAboveLastClosingPricePercent; /* ddr_u64AboveLastClosingPrice / all */

    oldouble_t ddr_dbLambBuyPercent;
    oldouble_t ddr_dbLambSoldPercent;

    oldouble_t ddr_dbLambBuyBelow100Percent;
    oldouble_t ddr_dbLambSoldBelow100Percent;

    oldouble_t ddr_dbCloseHighLimitSoldPercent;
    oldouble_t ddr_dbCloseHighLimitLambSoldPercent;

    oldouble_t ddr_dbCloseLowLimitBuyPercent;
    oldouble_t ddr_dbCloseLowLimitLambBuyPercent;

    oldouble_t ddr_dbVolumeRatio;
    oldouble_t ddr_dbSoldVolumeRatio;
    oldouble_t ddr_dbLambSoldVolumeRatio;

    /*amount*/
    u64 ddr_u64AllA;

    u64 ddr_u64BuyA;
    u64 ddr_u64SoldA;
    u64 ddr_u64NaA; /*don't know it buy or sold*/

    u64 ddr_u64LambBuyA;
    u64 ddr_u64LambSoldA;
    u64 ddr_u64LambNaA;

    u64 ddr_u64CloseHighLimitSoldA;
    u64 ddr_u64CloseHighLimitLambSoldA;

    u64 ddr_u64CloseLowLimitBuyA;
    u64 ddr_u64CloseLowLimitLambBuyA;

    /*the low price in the last X minutes*/
    olchar_t ddr_strLastTimeLowPrice[16];
    oldouble_t ddr_dbLastXLowPrice;
    oldouble_t ddr_dbLastXInc;

    /*upper shadow, lower shadow*/
    oldouble_t ddr_dbUpperShadowRatio;
    oldouble_t ddr_dbLowerShadowRatio;

    /*index*/
    olint_t ddr_nIndex;  /* index of a series of result */
} da_day_result_t;

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
    /*index*/
    olint_t dds_nIndex;  /* index of a series of result */
    olchar_t dds_strDate[16];

    /*price*/
    oldouble_t dds_dbOpeningPrice;
    oldouble_t dds_dbClosingPrice;
    oldouble_t dds_dbHighPrice;
    oldouble_t dds_dbLowPrice;

	oldouble_t dds_dbLastClosingPrice; /*exlude right*/

    oldouble_t dds_dbClosingPriceRate;
    oldouble_t dds_dbHighPriceRate;

    boolean_t dds_bCloseHighLimit; /*close price reach high limit*/
    boolean_t dds_bCloseLowLimit; /*close price reach low limit*/
    boolean_t dds_bHighHighLimit; /*high price reach high limit*/
    boolean_t dds_bLowLowLimit; /*low price reach low limit*/
    boolean_t dds_bGap;
    u8 dds_u8Reserved[3];

    /*volume*/
    u64 dds_u64All;
    oldouble_t dds_dbVolumeRatio;
    oldouble_t dds_dbTurnoverRate;

    /*amount*/
    u64 dds_u64AllA;

    /*upper shadow, lower shadow*/
    oldouble_t dds_dbUpperShadowRatio;
    oldouble_t dds_dbLowerShadowRatio;

	/*stock information*/
	u64 dds_u64GeneralCapital;
	u64 dds_u64TradableShare;
	boolean_t dds_bXR;
	boolean_t dds_bXD;
	boolean_t dds_bDR;
	boolean_t dds_bS;
    u8 dds_u8Reserved2[4];

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

typedef struct
{
    olint_t pp_nThres;

    olint_t pp_nStart;  /*start from 1*/
    olint_t pp_nEnd;

    olint_t pp_nLastCount; /*only parse last n files*/

    olchar_t * pp_pstrDateFrom;

    olchar_t pp_strLastX[16];  /*the last X time we care about the increase*/
} parse_param_t;

typedef struct
{
#define CONC_POSITION_SHORT  0
#define CONC_POSITION_FULL   1
	olint_t dc_nPosition;
#define CONC_ACTION_NONE      0
#define CONC_ACTION_BULL_OPENING   1
#define CONC_ACTION_BULL_CLOSEOUT  2
#define CONC_ACTION_BEAR_OPENING   3
#define CONC_ACTION_BEAR_CLOSEOUT  4
	olint_t dc_nAction;
	oldouble_t dc_dbPrice;
	oldouble_t dc_dbYield;

    da_day_summary_t * dc_pddsOpening;
    da_day_summary_t * dc_pddsCloseout;
    olint_t dc_nHoldDays; /*days with stock in hand*/
} da_conc_t;

typedef struct
{
    olint_t dcs_nTime;
    olint_t dcs_nProfitTime;
    olint_t dcs_nLossTime;
#define MIN_PROFIT_TIME_RATIO  80
    oldouble_t dcs_dbProfitTimeRatio;

    olint_t dcs_nHoldDays;
    olint_t dcs_nAveHoldDays;

    oldouble_t dcs_dbFund;
    oldouble_t dcs_dbOverallYield;
    oldouble_t dcs_dbAveYield;
} da_conc_sum_t;

typedef struct
{
    u64 qdp_u64Volume;
    oldouble_t qdp_dbPrice;
} quo_data_price_t;

typedef struct
{
    oldouble_t qe_dbCurPrice;
    oldouble_t qe_dbHighPrice;
    oldouble_t qe_dbLowPrice;
    u64 qe_u64Volume;
    oldouble_t qe_dbAmount;
    quo_data_price_t qe_qdpBuy[5];
    quo_data_price_t qe_qdpSold[5];

    olchar_t qe_strTime[16];
} quo_entry_t;

typedef struct stock_quo
{
    olchar_t sq_strCode[16];
    oldouble_t sq_dbOpeningPrice;
    oldouble_t sq_dbLastClosingPrice;
    olchar_t sq_strDate[16];

    olint_t sq_nMaxEntry;
    olint_t sq_nNumOfEntry;
    quo_entry_t * sq_pqeEntry;

//    struct stock_quo * sq_psqPair;
} stock_quo_t;

typedef struct
{
    olchar_t psi_strName[64];
    olchar_t * psi_pstrStocks;
} parse_sector_info_t;

/* --- functional routines ------------------------------------------------- */
u32 parseDataFile(
    olchar_t * file, parse_param_t * ppp, olint_t daindex,
    da_day_result_t * daresult);

u32 readTradeDayDetail(
    olchar_t * dirpath, parse_param_t * ppp,
    da_day_result_t * buffer, olint_t * numofresult);

void printTradeDayDetailVerbose(da_day_result_t * result);

da_day_summary_t * getDaySummaryWithHighestClosingPrice(
    da_day_summary_t * buffer, olint_t num);
da_day_summary_t * getDaySummaryWithLowestClosingPrice(
    da_day_summary_t * buffer, olint_t num);
da_day_summary_t * getDaySummaryWithLowestLowPrice(
    da_day_summary_t * buffer, olint_t num);
da_day_summary_t * getDaySummaryWithHighestHighPrice(
    da_day_summary_t * buffer, olint_t num);
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
    da_day_summary_t * buffer, olint_t num, da_day_summary_t ** ppp,
    olint_t * nump);
void adjustDaySummaryInflexionPoint(
    da_day_summary_t ** ppp, olint_t * nump, olint_t adjust);
void getDaySummaryInflexionPoint2(
    da_day_summary_t * buffer, olint_t num, da_day_summary_t ** ppp,
    olint_t * nump);
void locateInflexionPointRegion(
    da_day_summary_t ** ppp, olint_t nump, da_day_summary_t * dest,
    da_day_summary_t ** start, da_day_summary_t ** end);
da_day_summary_t * getInflexionPointWithHighestClosingPrice(
    da_day_summary_t ** ppp, olint_t nump);

u32 readTradeDaySummary(
    olchar_t * pstrDataDir, da_day_summary_t * buffer, olint_t * numofresult);
u32 readTradeDaySummaryWithFRoR(
    olchar_t * pstrDataDir, da_day_summary_t * buffer, olint_t * numofresult);
u32 readTradeDaySummaryFromDate(
    olchar_t * pstrDataDir, olchar_t * pstrDateFrom, da_day_summary_t * buffer,
    olint_t * numofresult);
u32 readTradeDaySummaryFromDateWithFRoR(
    olchar_t * pstrDataDir, olchar_t * pstrDateFrom, da_day_summary_t * buffer,
    olint_t * numofresult);
u32 readTradeDaySummaryUntilDate(
    olchar_t * pstrDataDir, olchar_t * pstrEndDate, da_day_summary_t * buffer,
    olint_t * numofresult);
u32 readTradeDaySummaryUntilDateWithFRoR(
    olchar_t * pstrDataDir, olchar_t * pstrEndDate, da_day_summary_t * buffer,
    olint_t * numofresult);
olint_t daySummaryEndDataCount(
    da_day_summary_t * buffer, olint_t num, olchar_t * pstrEndDate);
boolean_t isStraightLineMotion(
    da_day_summary_t * buffer, olint_t total, olint_t ndays);
boolean_t isStraightHighLimitDay(da_day_summary_t * pdds);

void printDaDaySummaryVerbose(da_day_summary_t * summary);
void getStringDaySummaryStatus(da_day_summary_t * summary, olchar_t * pstr);

olint_t compareDaConcSum(da_conc_sum_t * a, da_conc_sum_t * b);
void addToDaConcSum(da_conc_sum_t * sum, da_conc_t * conc);
void printDaConcSumVerbose(da_conc_sum_t * sum);

quo_entry_t * getQuoEntryWithHighestPrice(
    quo_entry_t * start, quo_entry_t * end);
quo_entry_t * getQuoEntryWithLowestPrice(
    quo_entry_t * start, quo_entry_t * end);
void getQuoEntryInflexionPoint(
    quo_entry_t * buffer, olint_t num, quo_entry_t ** ppq, olint_t * nump);
u32 readStockQuotationFile(
    olchar_t * pstrDataDir, stock_quo_t * psq);
olint_t getNextTopBottomQuoEntry(
    quo_entry_t ** pqe, olint_t total, olint_t start);

/*the routine allocates memory for psi_pstrStocks, call freeSectorInfo() to
  free the memory*/
u32 parseSectorDir(
    olchar_t * pstrDir, parse_sector_info_t * sector, olint_t * numofsector);
void freeSectorInfo(parse_sector_info_t * sector);


#endif /*TANGXUN_JIUTAI_PARSEDATA_H*/

/*---------------------------------------------------------------------------*/


