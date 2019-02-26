/**
 *  @file stocktrade.h
 *
 *  @brief Provide some helper routine for stock trade
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_STOCKTRADE_H
#define TANGXUN_JIUTAI_STOCKTRADE_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "parsedata.h"
#include "xtime.h"

/* --- constant definitions ------------------------------------------------ */

#define MAX_TRADE_FIELD_LEN               (16)
#define MAX_TRADE_OP_REMARK_LEN           (32)
#define MAX_TRADE_MODEL_PARAM_LEN         (128)

#define TRADE_OP_STR_NONE                 "none"
#define TRADE_OP_STR_BUY                  "buy"
#define TRADE_OP_STR_SELL                 "sell"

#define TRADE_POSITION_STR_NONE           "none"
#define TRADE_POSITION_STR_SHORT          "short"
#define TRADE_POSITION_STR_FULL           "full"

#define STOCK_POSITION_NONE               (0)  /*no position*/
#define STOCK_POSITION_SHORT              (1)  /*short position*/
#define STOCK_POSITION_FULL               (2)  /*full position*/

#define STOCK_OP_NONE                     (0)  /*no transaction*/
#define STOCK_OP_BUY                      (1)  /*open transaction*/
#define STOCK_OP_SELL                     (2)  /*closeout transaction*/


/* --- data structures ----------------------------------------------------- */

typedef struct trade_pool_stock
{
    olchar_t tps_strStock[MAX_TRADE_FIELD_LEN];  /*!< The name of the stock. */
    olchar_t tps_strModel[MAX_TRADE_FIELD_LEN];  /*!< The name of the model. */
    olchar_t tps_strModelParam[MAX_TRADE_MODEL_PARAM_LEN]; /*!< The parameter of the model. */
    olchar_t tps_strAddDate[MAX_TRADE_FIELD_LEN];  /*!< The date to add this stock. */
    olchar_t tps_strOp[MAX_TRADE_FIELD_LEN];  /*!< The operation of the trading. */
    olchar_t tps_strOpRemark[MAX_TRADE_OP_REMARK_LEN];  /*!< The operation remark of the trading. */
    olchar_t tps_strTradeDate[MAX_TRADE_FIELD_LEN];  /*!< The date to trade this stock. */
    olchar_t tps_strPosition[MAX_TRADE_FIELD_LEN]; /*!< The position of the trading. */
    int32_t tps_nVolume; /*!< The volume of the trading. */
    oldouble_t tps_dbPrice;  /*!< The price of the trading. */
} trade_pool_stock_t;

typedef struct trade_trading_record
{
    olchar_t ttr_strStock[MAX_TRADE_FIELD_LEN];  /*!< The name of the stock. */
    olchar_t ttr_strModel[MAX_TRADE_FIELD_LEN];  /*!< The name of the model. */
    olchar_t ttr_strModelParam[MAX_TRADE_MODEL_PARAM_LEN]; /*!< The parameter of the model. */
    olchar_t ttr_strAddDate[MAX_TRADE_FIELD_LEN];  /*!< The date to add this stock. */
    olchar_t ttr_strOp[MAX_TRADE_FIELD_LEN];  /*!< The operation of the trading. */
    olchar_t ttr_strOpRemark[MAX_TRADE_OP_REMARK_LEN];  /*!< The operation remark of the trading. */
    olchar_t ttr_strTradeDate[MAX_TRADE_FIELD_LEN];  /*!< The date to the trading. */
    olchar_t ttr_strPosition[MAX_TRADE_FIELD_LEN]; /*!< The position of the trading. */
    int32_t ttr_nVolume; /*!< The volume of the trading. */
    oldouble_t ttr_dbPrice;  /*!< The price of the trading. */
} trade_trading_record_t;


/* --- functional routines ------------------------------------------------- */
boolean_t isTradePoolStockOpBuy(trade_pool_stock_t * ptps);
void setTradePoolStockOpBuy(trade_pool_stock_t * ptps);
boolean_t isTradePoolStockOpSell(trade_pool_stock_t * ptps);
void setTradePoolStockOpSell(trade_pool_stock_t * ptps);
boolean_t isTradePoolStockOpNone(trade_pool_stock_t * ptps);

void setTradePoolStockPositionShort(trade_pool_stock_t * ptps);
void setTradePoolStockPositionFull(trade_pool_stock_t * ptps);

void initTradePoolStock(
    trade_pool_stock_t * ptps, olchar_t * pstrStock, olchar_t * pstrModel);

void cleanTradePoolStock(trade_pool_stock_t * ptps);

void initTradeTradingRecord(
    trade_trading_record_t * pttr, olchar_t * pstrStock, olchar_t * pstrModel);

void setTradeTradingRecord(
    trade_trading_record_t * pttr, const trade_pool_stock_t * ptps);

boolean_t isTradeTradingRecordOpBuy(trade_trading_record_t * pttr);
boolean_t isTradeTradingRecordOpSell(trade_trading_record_t * pttr);

u32 getNextTradingDate(const olchar_t * pstrCurr, olchar_t * pstrNext);


char * getStringStockPosition(u8 u8Pos);
char * getStringStockOperation(u8 u8Op);

u32 filterPoolStockByOp(
    trade_pool_stock_t * ptps, olint_t count, u8 u8Op,
    trade_pool_stock_t ** ppFilterStock, olint_t * pnFilterCount);


#endif /*TANGXUN_JIUTAI_STOCKTRADE_H*/

/*---------------------------------------------------------------------------*/


