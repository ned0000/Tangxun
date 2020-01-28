/**
 *  @file tx_trade.h
 *
 *  @brief Provide some routine for stock trade.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_TRADE_H
#define TANGXUN_TRADE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_time.h"

#include "tx_stock.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define TX_TRADE_MAX_FIELD_LEN               (16)
#define TX_TRADE_MAX_OP_REMARK_LEN           (32)
#define TX_TRADE_MAX_MODEL_PARAM_LEN         (128)

typedef enum
{
    /**No position.*/
    TX_TRADE_STOCK_POSITION_NONE = 0,
    /**Short position.*/
    TX_TRADE_STOCK_POSITION_SHORT,
    /**Full position.*/
    TX_TRADE_STOCK_POSITION_FULL,
} tx_trade_stock_position_t;

typedef enum
{
    /**No transaction.*/
    TX_TRADE_STOCK_OP_NONE = 0,
    /**Open transaction.*/
    TX_TRADE_STOCK_OP_BUY,
    /**Closeout transaction.*/
    TX_TRADE_STOCK_OP_SELL,
} tx_trade_stock_op_t;


/* --- data structures -------------------------------------------------------------------------- */

typedef struct tx_trade_pool_stock
{
    /**The name of the stock.*/
    olchar_t ttps_strStock[TX_TRADE_MAX_FIELD_LEN];
    /**The name of the model.*/
    olchar_t ttps_strModel[TX_TRADE_MAX_FIELD_LEN];
    /**The parameter of the model.*/
    olchar_t ttps_strModelParam[TX_TRADE_MAX_MODEL_PARAM_LEN];
    /**The date to add this stock.*/
    olchar_t ttps_strAddDate[TX_TRADE_MAX_FIELD_LEN];
    /**The start date of the day summary.*/
    olchar_t ttps_strStartDateOfDaySummary[TX_TRADE_MAX_FIELD_LEN];
    /**Number of day summary.*/
    olint_t ttps_nNumOfDaySummary;
    /**The operation of the trading.*/
    olchar_t ttps_strOp[TX_TRADE_MAX_FIELD_LEN];
    /**The operation remark of the trading.*/
    olchar_t ttps_strOpRemark[TX_TRADE_MAX_OP_REMARK_LEN];
    /**The date to trade this stock.*/
    olchar_t ttps_strTradeDate[TX_TRADE_MAX_FIELD_LEN];
    /**The position of the trading.*/
    olchar_t ttps_strPosition[TX_TRADE_MAX_FIELD_LEN];
    /**The volume of the trading.*/
    olint_t ttps_nVolume;
    /**The price of the trading.*/
    oldouble_t ttps_dbPrice;
} tx_trade_pool_stock_t;

typedef struct tx_trade_trading_record
{
    /**The name of the stock.*/
    olchar_t tttr_strStock[TX_TRADE_MAX_FIELD_LEN];
    /**The name of the model.*/
    olchar_t tttr_strModel[TX_TRADE_MAX_FIELD_LEN];
    /**The parameter of the model.*/
    olchar_t tttr_strModelParam[TX_TRADE_MAX_MODEL_PARAM_LEN];
    /**The date to add this stock.*/
    olchar_t tttr_strAddDate[TX_TRADE_MAX_FIELD_LEN];
    /**The operation of the trading.*/
    olchar_t tttr_strOp[TX_TRADE_MAX_FIELD_LEN];
    /**The operation remark of the trading.*/
    olchar_t tttr_strOpRemark[TX_TRADE_MAX_OP_REMARK_LEN];
    /**The date to the trading.*/
    olchar_t tttr_strTradeDate[TX_TRADE_MAX_FIELD_LEN];
    /**The position of the trading.*/
    olchar_t tttr_strPosition[TX_TRADE_MAX_FIELD_LEN];
    /**The volume of the trading.*/
    olint_t tttr_nVolume;
    /**The price of the trading.*/
    oldouble_t tttr_dbPrice;
} tx_trade_trading_record_t;


/* --- functional routines ---------------------------------------------------------------------- */

boolean_t tx_trade_isPoolStockOpNone(tx_trade_pool_stock_t * pttps);

boolean_t tx_trade_isPoolStockOpBuy(tx_trade_pool_stock_t * pttps);

boolean_t tx_trade_isPoolStockOpSell(tx_trade_pool_stock_t * pttps);

void tx_trade_setPoolStockOpSell(tx_trade_pool_stock_t * pttps);

void tx_trade_setPoolStockOpBuy(tx_trade_pool_stock_t * pttps);

void tx_trade_setPoolStockPositionShort(tx_trade_pool_stock_t * pttps);

void tx_trade_setPoolStockPositionFull(tx_trade_pool_stock_t * pttps);

void tx_trade_setPoolStockTradeDate(tx_trade_pool_stock_t * pttps, const olchar_t * pstrDate);

void tx_trade_setPoolStockPrice(tx_trade_pool_stock_t * pttps, const oldouble_t dbPrice);

void tx_trade_setPoolStockVolume(tx_trade_pool_stock_t * pttps, const olint_t nVolume);

void tx_trade_initPoolStock(
    tx_trade_pool_stock_t * pttps, olchar_t * pstrStock, olchar_t * pstrModel);

void tx_trade_cleanPoolStock(tx_trade_pool_stock_t * pttps);

void tx_trade_initTradingRecord(
    tx_trade_trading_record_t * ptttr, olchar_t * pstrStock, olchar_t * pstrModel);

void tx_trade_setTradingRecord(
    tx_trade_trading_record_t * ptttr, const tx_trade_pool_stock_t * pttps);

boolean_t tx_trade_isTradingRecordOpBuy(tx_trade_trading_record_t * ptttr);

boolean_t tx_trade_isTradingRecordOpSell(tx_trade_trading_record_t * ptttr);

olchar_t * tx_trade_getStringStockPosition(u8 u8Pos);

olchar_t * tx_trade_getStringStockOperation(u8 u8Op);

u32 tx_trade_filterPoolStockByOp(
    tx_trade_pool_stock_t * pttps, olint_t count, u8 u8Op, tx_trade_pool_stock_t ** ppFilterStock,
    olint_t * pnFilterCount);

u32 tx_trade_setStockFirstTradeDate(const char * strStockPath);

boolean_t tx_trade_isAfterStockFirstTradeDate(
    const tx_stock_info_t * stockinfo, const olchar_t * pstrDate);

boolean_t tx_trade_isHoliday(olint_t days);

u32 tx_trade_getNextTradingDate(const olchar_t * pstrCurr, olchar_t * pstrNext);

#endif /*TANGXUN_TRADE_H*/

/*------------------------------------------------------------------------------------------------*/


