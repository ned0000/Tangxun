/**
 *  @file tx_persistency.h
 *
 *  @brief persistency library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */
 
#ifndef TANGXUN_PERSISTENCY_H
#define TANGXUN_PERSISTENCY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

#include "tx_trade.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 tx_persistency_init(void);
u32 tx_persistency_fini(void);

u32 tx_persistency_clearData(void);

u32 tx_persistency_startTransaction(void);
u32 tx_persistency_commitTransaction(void);
u32 tx_persistency_rollbackTransaction(void);

u32 tx_persistency_replacePoolStock(tx_trade_pool_stock_t * pStock);
u32 tx_persistency_insertPoolStock(tx_trade_pool_stock_t * pStock);
u32 tx_persistency_removePoolStock(tx_trade_pool_stock_t * pStock);
u32 tx_persistency_updatePoolStock(tx_trade_pool_stock_t * pStock);
u32 tx_persistency_getAllPoolStock(tx_trade_pool_stock_t * pStock, olint_t * pnNum);
u32 tx_persistency_getPoolStock(tx_trade_pool_stock_t * pStock);
olint_t tx_persistency_getNumOfPoolStock(void);

olint_t tx_persistency_getNumOfTradingRecord(void);
u32 tx_persistency_getAllTradingRecord(tx_trade_trading_record_t * pRecord, olint_t * pnNum);
u32 tx_persistency_insertTradingRecord(tx_trade_trading_record_t * pRecord);

#endif   /*TANGXUN_PERSISTENCY_H*/

/*------------------------------------------------------------------------------------------------*/

