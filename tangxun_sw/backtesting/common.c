/**
 *  @file common.c
 *
 *  @brief Implementation file for backtesting common definition and routines.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_jiukun.h"

#include "tx_backtesting.h"
#include "tx_persistency.h"

#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 wrapupBtPoolStock(tx_backtesting_result_t * ptbr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index = 0, count = 0;
    tx_trade_pool_stock_t * stockinpool = NULL, * pStock = NULL;
    tx_trade_trading_record_t tttr;

    count = tx_persistency_getNumOfPoolStock();
    if (count == 0)
        return u32Ret;

    JF_LOGGER_INFO("%d stocks", count);

    jf_jiukun_allocMemory((void **)&stockinpool, count * sizeof(tx_trade_pool_stock_t));

    u32Ret = tx_persistency_getAllPoolStock(stockinpool, &count);
    if ((u32Ret == JF_ERR_NO_ERROR) && (count > 0))
    {
        pStock = stockinpool;
        for (index = 0; index < count; index ++)
        {
            if (tx_trade_isPoolStockOpBuy(pStock))
            {
                /*Found a bought stock without sold, sell it forcely.*/
                JF_LOGGER_INFO("sell stock: %s", pStock->ttps_strStock);
                /*Set the stock as "sold".*/
                tx_trade_setPoolStockOpSell(pStock);
                ol_strcpy(pStock->ttps_strOpRemark, "wrap up");
                /*Save the trading record.*/
                tx_trade_setTradingRecord(&tttr, pStock);
                tx_persistency_insertTradingRecord(&tttr);
                /*Add the fund.*/
                ptbr->tbr_dbFund += pStock->ttps_dbPrice * (oldouble_t)pStock->ttps_nVolume;
            }

            tx_persistency_removePoolStock(pStock);
            pStock ++;
        }
    }

    jf_jiukun_freeMemory((void **)&stockinpool);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


