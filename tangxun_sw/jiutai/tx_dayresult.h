/**
 *  @file tx_dayresult.h
 *
 *  @brief Routine for parsing data into day result data structure.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_DAYRESULT_H
#define TANGXUN_JIUTAI_DAYRESULT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"

/* --- constant definitions --------------------------------------------------------------------- */
#define AMOUNT_UNIT              (10000)

#define STRAIGHT_LINE_MOTION     (0.03)

#define VOL_RATIO_NORMAL         (0.8)
#define VOL_RATIO_MODERATE       (1.5)
#define VOL_RATIO_DISTINCT       (2.5)
#define VOL_RATIO_FIERCE         (5.0)
#define VOL_RATIO_ABNORMAL       (10.0)

/* --- data structures -------------------------------------------------------------------------- */
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
    olchar_t psi_strName[64];
    olchar_t * psi_pstrStocks;
} parse_sector_info_t;

/* --- functional routines ---------------------------------------------------------------------- */
u32 parseDataFile(
    olchar_t * file, parse_param_t * ppp, olint_t daindex, da_day_result_t * daresult);

u32 readTradeDayDetail(
    olchar_t * dirpath, parse_param_t * ppp, da_day_result_t * buffer, olint_t * numofresult);

/** Allocate memory for psi_pstrStocks, call freeSectorInfo() to free the memory
 */
u32 parseSectorDir(
    olchar_t * pstrDir, parse_sector_info_t * sector, olint_t * numofsector);

void freeSectorInfo(parse_sector_info_t * sector);


#endif /*TANGXUN_JIUTAI_DAYRESULT_H*/

/*------------------------------------------------------------------------------------------------*/


