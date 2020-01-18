/**
 *  @file tx_backtesting.h
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

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_file.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */
typedef struct
{
/*the method backtests one day then another day*/
#define TX_BACKTESTING_METHOD_DAY_BY_DAY            (0)
/*the method backtests one stock then another stock*/
#define TX_BACKTESTING_METHOD_STOCK_BY_STOCK        (1)
    u8 tbp_u8Method;
    boolean_t tbp_bAllModel;
    u8 tbp_u8Reserved[14];
    olchar_t * tbp_pstrModel;
    u32 tbp_u32Reserved[4];
    olchar_t * tbp_pstrStockPath;
    oldouble_t tbp_dbInitialFund;
} tx_backtesting_param_t;

typedef struct
{
    oldouble_t tbr_dbInitialFund;  /*Initial fund*/
    oldouble_t tbr_dbFund;  /*Fund can be used*/
    oldouble_t tbr_dbMinAsset; /*Fund + Stock*/
    oldouble_t tbr_dbMaxAsset; /*Fund + Stock*/
    /*stat*/
    olchar_t tbr_strStartDate[16];
    olchar_t tbr_strEndDate[16];
    u32 tbr_u32NumOfTrade;
    u32 tbr_u32NumOfTradeProfit;
    u32 tbr_u32NumOfTradeLoss;
    u32 tbr_u32Reserved[5];
    oldouble_t tbr_dbMaxDrawdown;
    oldouble_t tbr_dbRateOfReturn;
} tx_backtesting_result_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 backtestingModel(tx_backtesting_param_t * ptbp, tx_backtesting_result_t * result);

#endif /*TANGXUN_BACKTESTING_H*/

/*------------------------------------------------------------------------------------------------*/


