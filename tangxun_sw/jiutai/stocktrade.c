/**
 *  @file stocktrade.c
 *
 *  @brief Provide some helper routine for stock trade
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
#include "stocktrade.h"
#include "jf_time.h"
#include "jf_date.h"
#include "jf_string.h"


/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

/* --- public routine section ------------------------------------------------------------------- */

boolean_t isTradePoolStockOpBuy(trade_pool_stock_t * ptps)
{
    boolean_t bRet = FALSE;

    if (ol_strcmp(ptps->tps_strOp, TRADE_OP_STR_BUY) == 0)
        bRet = TRUE;

    return bRet;
}

void setTradePoolStockOpBuy(trade_pool_stock_t * ptps)
{
    ol_strcpy(ptps->tps_strOp, TRADE_OP_STR_BUY);
}

boolean_t isTradePoolStockOpSell(trade_pool_stock_t * ptps)
{
    boolean_t bRet = FALSE;

    if (ol_strcmp(ptps->tps_strOp, TRADE_OP_STR_SELL) == 0)
        bRet = TRUE;

    return bRet;
}

void setTradePoolStockOpSell(trade_pool_stock_t * ptps)
{
    ol_strcpy(ptps->tps_strOp, TRADE_OP_STR_SELL);
}

boolean_t isTradePoolStockOpNone(trade_pool_stock_t * ptps)
{
    boolean_t bRet = FALSE;

    if (ol_strcmp(ptps->tps_strOp, TRADE_OP_STR_NONE) == 0)
        bRet = TRUE;

    return bRet;
}

void setTradePoolStockPositionShort(trade_pool_stock_t * ptps)
{
    ol_strcpy(ptps->tps_strPosition, TRADE_POSITION_STR_SHORT);
}

void setTradePoolStockPositionFull(trade_pool_stock_t * ptps)
{
    ol_strcpy(ptps->tps_strPosition, TRADE_POSITION_STR_FULL);
}

void initTradePoolStock(
    trade_pool_stock_t * ptps, olchar_t * pstrStock, olchar_t * pstrModel)
{
    bzero(ptps, sizeof(trade_pool_stock_t));
    ol_strncpy(ptps->tps_strStock, pstrStock, MAX_TRADE_FIELD_LEN - 1);
    ol_strncpy(ptps->tps_strModel, pstrModel, MAX_TRADE_FIELD_LEN - 1);
    ol_strcpy(ptps->tps_strOp, TRADE_OP_STR_NONE);
    ol_strcpy(ptps->tps_strPosition, TRADE_POSITION_STR_NONE);
} 

void cleanTradePoolStock(trade_pool_stock_t * ptps)
{
    ol_strcpy(ptps->tps_strOp, TRADE_OP_STR_NONE);
    bzero(ptps->tps_strTradeDate, MAX_TRADE_FIELD_LEN);
    bzero(ptps->tps_strOpRemark, MAX_TRADE_OP_REMARK_LEN);
    ol_strcpy(ptps->tps_strPosition, TRADE_POSITION_STR_NONE);
    ptps->tps_nVolume = 0;
    ptps->tps_dbPrice = 0;
} 

void initTradeTradingRecord(
    trade_trading_record_t * pttr, olchar_t * pstrStock, olchar_t * pstrModel)
{
    bzero(pttr, sizeof(trade_trading_record_t));
    ol_strncpy(pttr->ttr_strStock, pstrStock, MAX_TRADE_FIELD_LEN - 1);
    ol_strncpy(pttr->ttr_strModel, pstrModel, MAX_TRADE_FIELD_LEN - 1);
    ol_strcpy(pttr->ttr_strOp, TRADE_OP_STR_NONE);
    ol_strcpy(pttr->ttr_strPosition, TRADE_POSITION_STR_NONE);
}

void setTradeTradingRecord(
    trade_trading_record_t * pttr, const trade_pool_stock_t * ptps)
{
    bzero(pttr, sizeof(trade_trading_record_t));
    ol_strcpy(pttr->ttr_strStock, ptps->tps_strStock);
    ol_strcpy(pttr->ttr_strModel, ptps->tps_strModel);
    ol_strcpy(pttr->ttr_strModelParam, ptps->tps_strModelParam);
    ol_strcpy(pttr->ttr_strAddDate, ptps->tps_strAddDate);
    ol_strcpy(pttr->ttr_strOp, ptps->tps_strOp);
    ol_strcpy(pttr->ttr_strOpRemark, ptps->tps_strOpRemark);
    ol_strcpy(pttr->ttr_strTradeDate, ptps->tps_strTradeDate);
    ol_strcpy(pttr->ttr_strPosition, ptps->tps_strPosition);
    pttr->ttr_nVolume = ptps->tps_nVolume;
    pttr->ttr_dbPrice = ptps->tps_dbPrice;
}

boolean_t isTradeTradingRecordOpBuy(trade_trading_record_t * pttr)
{
    boolean_t bRet = FALSE;

    if (ol_strcmp(pttr->ttr_strOp, TRADE_OP_STR_BUY) == 0)
        bRet = TRUE;

    return bRet;
}

boolean_t isTradeTradingRecordOpSell(trade_trading_record_t * pttr)
{
    boolean_t bRet = FALSE;

    if (ol_strcmp(pttr->ttr_strOp, TRADE_OP_STR_SELL) == 0)
        bRet = TRUE;

    return bRet;
}

char * getStringStockPosition(u8 u8Pos)
{
    if (u8Pos == STOCK_POSITION_SHORT)
        return TRADE_POSITION_STR_SHORT;
    else if (u8Pos == STOCK_POSITION_FULL)
        return TRADE_POSITION_STR_FULL;
    else
        return TRADE_POSITION_STR_NONE;
}

char * getStringStockOperation(u8 u8Op)
{
    if (u8Op == STOCK_OP_BUY)
        return TRADE_OP_STR_BUY;
    else if (u8Op == STOCK_OP_SELL)
        return TRADE_OP_STR_SELL;
    else
        return TRADE_OP_STR_NONE;
}

u32 filterPoolStockByOp(
    trade_pool_stock_t * ptps, olint_t count, u8 u8Op,
    trade_pool_stock_t ** ppFilterStock, olint_t * pnFilterCount)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index;
    trade_pool_stock_t * pStock = ptps;
    olint_t filtercount = 0;

    for (index = 0; index < count; index ++)
    {
        if (filtercount > *pnFilterCount)
            break;

        if ((u8Op == STOCK_OP_BUY) &&
            isTradePoolStockOpBuy(pStock))
        {
            ppFilterStock[filtercount] = pStock;
            filtercount ++;
        }
        else if ((u8Op == STOCK_OP_SELL) &&
                 isTradePoolStockOpSell(pStock))
        {
            ppFilterStock[filtercount] = pStock;
            filtercount ++;
        }
        else if ((u8Op == STOCK_OP_NONE) &&
                 isTradePoolStockOpNone(pStock))
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


