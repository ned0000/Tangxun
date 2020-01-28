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
    /**Initial fund.*/
    oldouble_t tbr_dbInitialFund;
    /**Fund can be used.*/
    oldouble_t tbr_dbFund;
    /**Fund + Stock.*/
    oldouble_t tbr_dbMinAsset;
    /**Fund + Stock.*/
    oldouble_t tbr_dbMaxAsset;
    /**Start date.*/
    olchar_t tbr_strStartDate[16];
    olchar_t tbr_strEndDate[16];
    u32 tbr_u32NumOfTrade;
    u32 tbr_u32NumOfTradeProfit;
    u32 tbr_u32NumOfTradeLoss;
    u32 tbr_u32Reserved[5];
    oldouble_t tbr_dbMaxDrawdown;
    oldouble_t tbr_dbRateOfReturn;
} tx_backtesting_result_t;

typedef struct
{
    olchar_t * tbip_pstrStockPath;
    u32 tbip_u32Reserved[4];
} tx_backtesting_init_param_t;

typedef struct
{
    olchar_t * tbep_pstrStockPath;
    oldouble_t tbep_dbInitialFund;
    u32 tbep_u32Reserved[4];
} tx_backtesting_eval_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 tx_backtesting_init(tx_backtesting_init_param_t * param);

u32 tx_backtesting_fini(void);

/** Evaluate one model from trading day.
 *
 *  @note
 *  -# The testing is from one trading day of all stocks and then next trading day.
 */
u32 tx_backtesting_evalModelFromDay(
    const olchar_t * pstrModel, tx_backtesting_eval_param_t * ptbep,
    tx_backtesting_result_t * result);

/** Evaluate one model from stock.
 *
 *  @note
 *  -# The testing is from one stock and then another.
 */
u32 tx_backtesting_evalModelFromStock(
    const olchar_t * pstrModel, tx_backtesting_eval_param_t * ptbep,
    tx_backtesting_result_t * ptbr);

#endif /*TANGXUN_BACKTESTING_H*/

/*------------------------------------------------------------------------------------------------*/


