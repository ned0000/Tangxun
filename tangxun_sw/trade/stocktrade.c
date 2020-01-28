/**
 *  @file stocktrade.c
 *
 *  @brief Provide some helper routine for stock trade.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_time.h"
#include "jf_date.h"
#include "jf_string.h"

#include "tx_trade.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define TX_TRADE_OP_STR_NONE                 "none"
#define TX_TRADE_OP_STR_BUY                  "buy"
#define TX_TRADE_OP_STR_SELL                 "sell"

#define TX_TRADE_POSITION_STR_NONE           "none"
#define TX_TRADE_POSITION_STR_SHORT          "short"
#define TX_TRADE_POSITION_STR_FULL           "full"

/* --- private routine section ------------------------------------------------------------------ */

/* --- public routine section ------------------------------------------------------------------- */

boolean_t tx_trade_isPoolStockOpBuy(tx_trade_pool_stock_t * pttps)
{
    boolean_t bRet = FALSE;

    if (ol_strcmp(pttps->ttps_strOp, TX_TRADE_OP_STR_BUY) == 0)
        bRet = TRUE;

    return bRet;
}

void tx_trade_setPoolStockOpBuy(tx_trade_pool_stock_t * pttps)
{
    ol_strcpy(pttps->ttps_strOp, TX_TRADE_OP_STR_BUY);
}

void tx_trade_setPoolStockTradeDate(tx_trade_pool_stock_t * pttps, const olchar_t * pstrDate)
{
    ol_strcpy(pttps->ttps_strTradeDate, pstrDate);
}

void tx_trade_setPoolStockPrice(tx_trade_pool_stock_t * pttps, const oldouble_t dbPrice)
{
    pttps->ttps_dbPrice = dbPrice;
}

void tx_trade_setPoolStockVolume(tx_trade_pool_stock_t * pttps, const olint_t nVolume)
{
    pttps->ttps_nVolume = nVolume;
}

boolean_t tx_trade_isPoolStockOpSell(tx_trade_pool_stock_t * pttps)
{
    boolean_t bRet = FALSE;

    if (ol_strcmp(pttps->ttps_strOp, TX_TRADE_OP_STR_SELL) == 0)
        bRet = TRUE;

    return bRet;
}

void tx_trade_setPoolStockOpSell(tx_trade_pool_stock_t * pttps)
{
    ol_strcpy(pttps->ttps_strOp, TX_TRADE_OP_STR_SELL);
}

boolean_t tx_trade_isPoolStockOpNone(tx_trade_pool_stock_t * pttps)
{
    boolean_t bRet = FALSE;

    if (ol_strcmp(pttps->ttps_strOp, TX_TRADE_OP_STR_NONE) == 0)
        bRet = TRUE;

    return bRet;
}

void tx_trade_setPoolStockPositionShort(tx_trade_pool_stock_t * pttps)
{
    ol_strcpy(pttps->ttps_strPosition, TX_TRADE_POSITION_STR_SHORT);
}

void tx_trade_setPoolStockPositionFull(tx_trade_pool_stock_t * pttps)
{
    ol_strcpy(pttps->ttps_strPosition, TX_TRADE_POSITION_STR_FULL);
}

void tx_trade_initPoolStock(
    tx_trade_pool_stock_t * pttps, olchar_t * pstrStock, olchar_t * pstrModel)
{
    bzero(pttps, sizeof(tx_trade_pool_stock_t));
    ol_strncpy(pttps->ttps_strStock, pstrStock, TX_TRADE_MAX_FIELD_LEN - 1);
    ol_strncpy(pttps->ttps_strModel, pstrModel, TX_TRADE_MAX_FIELD_LEN - 1);
    ol_strcpy(pttps->ttps_strOp, TX_TRADE_OP_STR_NONE);
    ol_strcpy(pttps->ttps_strPosition, TX_TRADE_POSITION_STR_NONE);
} 

void tx_trade_cleanPoolStock(tx_trade_pool_stock_t * pttps)
{
    ol_strcpy(pttps->ttps_strOp, TX_TRADE_OP_STR_NONE);
    bzero(pttps->ttps_strTradeDate, TX_TRADE_MAX_FIELD_LEN);
    bzero(pttps->ttps_strOpRemark, TX_TRADE_MAX_OP_REMARK_LEN);
    ol_strcpy(pttps->ttps_strPosition, TX_TRADE_POSITION_STR_NONE);
    pttps->ttps_nVolume = 0;
    pttps->ttps_dbPrice = 0;
} 

void tx_trade_initTradingRecord(
    tx_trade_trading_record_t * ptttr, olchar_t * pstrStock, olchar_t * pstrModel)
{
    bzero(ptttr, sizeof(tx_trade_trading_record_t));
    ol_strncpy(ptttr->tttr_strStock, pstrStock, TX_TRADE_MAX_FIELD_LEN - 1);
    ol_strncpy(ptttr->tttr_strModel, pstrModel, TX_TRADE_MAX_FIELD_LEN - 1);
    ol_strcpy(ptttr->tttr_strOp, TX_TRADE_OP_STR_NONE);
    ol_strcpy(ptttr->tttr_strPosition, TX_TRADE_POSITION_STR_NONE);
}

void tx_trade_setTradingRecord(
    tx_trade_trading_record_t * ptttr, const tx_trade_pool_stock_t * pttps)
{
    bzero(ptttr, sizeof(tx_trade_trading_record_t));
    ol_strcpy(ptttr->tttr_strStock, pttps->ttps_strStock);
    ol_strcpy(ptttr->tttr_strModel, pttps->ttps_strModel);
    ol_strcpy(ptttr->tttr_strModelParam, pttps->ttps_strModelParam);
    ol_strcpy(ptttr->tttr_strAddDate, pttps->ttps_strAddDate);
    ol_strcpy(ptttr->tttr_strOp, pttps->ttps_strOp);
    ol_strcpy(ptttr->tttr_strOpRemark, pttps->ttps_strOpRemark);
    ol_strcpy(ptttr->tttr_strTradeDate, pttps->ttps_strTradeDate);
    ol_strcpy(ptttr->tttr_strPosition, pttps->ttps_strPosition);
    ptttr->tttr_nVolume = pttps->ttps_nVolume;
    ptttr->tttr_dbPrice = pttps->ttps_dbPrice;
}

boolean_t tx_trade_isTradingRecordOpBuy(tx_trade_trading_record_t * ptttr)
{
    boolean_t bRet = FALSE;

    if (ol_strcmp(ptttr->tttr_strOp, TX_TRADE_OP_STR_BUY) == 0)
        bRet = TRUE;

    return bRet;
}

boolean_t tx_trade_isTradingRecordOpSell(tx_trade_trading_record_t * ptttr)
{
    boolean_t bRet = FALSE;

    if (ol_strcmp(ptttr->tttr_strOp, TX_TRADE_OP_STR_SELL) == 0)
        bRet = TRUE;

    return bRet;
}

olchar_t * tx_trade_getStringStockPosition(u8 u8Pos)
{
    if (u8Pos == TX_TRADE_STOCK_POSITION_SHORT)
        return TX_TRADE_POSITION_STR_SHORT;
    else if (u8Pos == TX_TRADE_STOCK_POSITION_FULL)
        return TX_TRADE_POSITION_STR_FULL;
    else
        return TX_TRADE_POSITION_STR_NONE;
}

olchar_t * tx_trade_getStringStockOperation(u8 u8Op)
{
    if (u8Op == TX_TRADE_STOCK_OP_BUY)
        return TX_TRADE_OP_STR_BUY;
    else if (u8Op == TX_TRADE_STOCK_OP_SELL)
        return TX_TRADE_OP_STR_SELL;
    else
        return TX_TRADE_OP_STR_NONE;
}

u32 tx_trade_filterPoolStockByOp(
    tx_trade_pool_stock_t * pttps, olint_t count, u8 u8Op, tx_trade_pool_stock_t ** ppFilterStock,
    olint_t * pnFilterCount)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index;
    tx_trade_pool_stock_t * pStock = pttps;
    olint_t filtercount = 0;

    for (index = 0; index < count; index ++)
    {
        if (filtercount > *pnFilterCount)
            break;

        if ((u8Op == TX_TRADE_STOCK_OP_BUY) && tx_trade_isPoolStockOpBuy(pStock))
        {
            ppFilterStock[filtercount] = pStock;
            filtercount ++;
        }
        else if ((u8Op == TX_TRADE_STOCK_OP_SELL) && tx_trade_isPoolStockOpSell(pStock))
        {
            ppFilterStock[filtercount] = pStock;
            filtercount ++;
        }
        else if ((u8Op == TX_TRADE_STOCK_OP_NONE) && tx_trade_isPoolStockOpNone(pStock))
        {
            ppFilterStock[filtercount] = pStock;            
            filtercount ++;
        }
        else
        {

        }

        pStock ++;
    }

    *pnFilterCount = filtercount;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


