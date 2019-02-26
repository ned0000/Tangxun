/**
 *  @file trade_persistency.h
 *
 *  @brief trade persistency library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */
 
#ifndef TANGXUN_TRADE_PERSISTENCY_H
#define TANGXUN_TRADE_PERSISTENCY_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "stocktrade.h"

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */

u32 initTradePersistency(void);
u32 finiTradePersistency(void);

u32 clearDataInTradePersistency(void);

u32 startTradePersistencyTransaction(void);
u32 commitTradePersistencyTransaction(void);
u32 rollbackTradePersistencyTransaction(void);

u32 replacePoolStockIntoTradePersistency(trade_pool_stock_t * pStock);
u32 insertPoolStockIntoTradePersistency(trade_pool_stock_t * pStock);
u32 removePoolStockFromTradePersistency(trade_pool_stock_t * pStock);
u32 updatePoolStockInTradePersistency(trade_pool_stock_t * pStock);
u32 getAllPoolStockInTradePersistency(
    trade_pool_stock_t * pStock, olint_t * pnNum);
u32 getPoolStockInTradePersistency(trade_pool_stock_t * pStock);
olint_t getNumOfPoolStockInTradePersistency(void);

olint_t getNumOfTradingRecordInTradePersistency(void);
u32 getAllTradingRecordInTradePersistency(
    trade_trading_record_t * pRecord, olint_t * pnNum);
u32 insertTradingRecordIntoTradePersistency(trade_trading_record_t * pRecord);

#endif   /*TANGXUN_TRADE_PERSISTENCY_H*/

/*---------------------------------------------------------------------------*/

