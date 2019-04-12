/**
 *  @file backtesting.h
 *
 *  @brief Backtesting header file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_BACKTESTING_H
#define TANGXUN_BACKTESTING_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_file.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
typedef struct
{
/*the method backtests one day then another day*/
#define BACKTESTING_METHOD_DAY_BY_DAY            (0)
/*the method backtests one stock then another stock*/
#define BACKTESTING_METHOD_STOCK_BY_STOCK        (1)
    u8 bp_u8Method;
    boolean_t bp_bAllModel;
    u8 bp_u8Reserved[14];
    olchar_t * bp_pstrModel;
    u32 bp_u32Reserved[4];
    olchar_t * bp_pstrStockPath;
    oldouble_t bp_dbInitialFund;
} backtesting_param_t;

typedef struct
{
    oldouble_t br_dbInitialFund;  /*Initial fund*/
    oldouble_t br_dbFund;  /*Fund can be used*/
    oldouble_t br_dbMinAsset; /*Fund + Stock*/
    oldouble_t br_dbMaxAsset; /*Fund + Stock*/
    /*stat*/
    u32 br_u32NumOfTrade;
    u32 br_u32NumOfTradeProfit;
    u32 br_u32NumOfTradeLoss;
    oldouble_t br_dbMaxDrawdown;
    oldouble_t br_dbRateOfReturn;
} backtesting_result_t;

/* --- functional routines ------------------------------------------------- */

u32 backtestingModel(backtesting_param_t * pbp, backtesting_result_t * result);

#endif /*TANGXUN_BACKTESTING_H*/

/*---------------------------------------------------------------------------*/


