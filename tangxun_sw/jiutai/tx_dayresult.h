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

#define TX_DR_AMOUNT_UNIT              (10000)

/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    olchar_t td_strDate[16];

    /*price*/
    oldouble_t td_dbOpeningPrice;
    oldouble_t td_dbClosingPrice;
    oldouble_t td_dbHighPrice;
    oldouble_t td_dbLowPrice;

    oldouble_t td_dbClosingPriceInc;
    oldouble_t td_dbClosingPriceDec;
    oldouble_t td_dbHighPriceInc;
    oldouble_t td_dbLowPriceDec;

    boolean_t td_bCloseHighLimit;
    boolean_t td_bCloseLowLimit;
    boolean_t td_bGap;
    u8 td_u8Reserved[5];
    olchar_t td_strTimeCloseHighLimit[16];
    olchar_t td_strTimeCloseLowLimit[16];
    olint_t td_nMaxTimeOpenCloseHighLimit; /* in second */
    olint_t td_nReserved5;

    /*volume*/
    u64 td_u64All;

    u64 td_u64Buy;
    u64 td_u64Sold;
    u64 td_u64Na; /*don't know it buy or sold*/

    u64 td_u64BuyInAm;  /*buy in the morning*/
    u64 td_u64SoldInAm;    /*buy in the afternoon*/

    u64 td_u64LambBuy;
    u64 td_u64LambSold;
    u64 td_u64LambNa;

    u64 td_u64AboveLastClosingPrice; /*volumes with
                                       higher price than last closing price*/
    u64 td_u64LambBuyBelow100;
    u64 td_u64LambSoldBelow100;

    u64 td_u64CloseHighLimitSold;
    u64 td_u64CloseHighLimitLambSold;

    u64 td_u64CloseLowLimitBuy;
    u64 td_u64CloseLowLimitLambBuy;

    oldouble_t td_dbBuyPercent;  /* buy / all */
    oldouble_t td_dbSoldPercent;  /* sold / all */

    oldouble_t td_dbBuyInAmPercent;
    oldouble_t td_dbSoldInAmPercent;

    oldouble_t td_dbAboveLastClosingPricePercent; /* td_u64AboveLastClosingPrice / all */

    oldouble_t td_dbLambBuyPercent;
    oldouble_t td_dbLambSoldPercent;

    oldouble_t td_dbLambBuyBelow100Percent;
    oldouble_t td_dbLambSoldBelow100Percent;

    oldouble_t td_dbCloseHighLimitSoldPercent;
    oldouble_t td_dbCloseHighLimitLambSoldPercent;

    oldouble_t td_dbCloseLowLimitBuyPercent;
    oldouble_t td_dbCloseLowLimitLambBuyPercent;

    oldouble_t td_dbVolumeRatio;
    oldouble_t td_dbSoldVolumeRatio;
    oldouble_t td_dbLambSoldVolumeRatio;

    /*amount*/
    u64 td_u64AllA;

    u64 td_u64BuyA;
    u64 td_u64SoldA;
    u64 td_u64NaA; /*don't know it buy or sold*/

    u64 td_u64LambBuyA;
    u64 td_u64LambSoldA;
    u64 td_u64LambNaA;

    u64 td_u64CloseHighLimitSoldA;
    u64 td_u64CloseHighLimitLambSoldA;

    u64 td_u64CloseLowLimitBuyA;
    u64 td_u64CloseLowLimitLambBuyA;

    /*the low price in the last X minutes*/
    olchar_t td_strLastTimeLowPrice[16];
    oldouble_t td_dbLastXLowPrice;
    oldouble_t td_dbLastXInc;

    /*upper shadow, lower shadow*/
    oldouble_t td_dbUpperShadowRatio;
    oldouble_t td_dbLowerShadowRatio;

    /*index*/
    olint_t td_nIndex;  /* index of a series of result */
} tx_dr_t;

typedef struct
{
    olint_t tdpp_nThres;

    olint_t tdpp_nStart;  /*start from 1*/
    olint_t tdpp_nEnd;

    olint_t tdpp_nLastCount; /*only parse last n files*/

    olchar_t * tdpp_pstrDateFrom;

    olchar_t tdpp_strLastX[16];  /*the last X time we care about the increase*/
} tx_dr_parse_param_t;

/* --- functional routines ---------------------------------------------------------------------- */
u32 tx_dr_parseDataFile(
    olchar_t * file, tx_dr_parse_param_t * ptdpp, olint_t daindex, tx_dr_t * daresult);

u32 tx_dr_readDrDir(
    olchar_t * dirpath, tx_dr_parse_param_t * ptdpp, tx_dr_t * buffer, olint_t * numofresult);

#endif /*TANGXUN_JIUTAI_DAYRESULT_H*/

/*------------------------------------------------------------------------------------------------*/


