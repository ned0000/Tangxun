/**
 *  @file persistencycommon.h
 *
 *  @brief Common header file for trade persistency library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TRADE_PERSISTENCY_COMMON_H
#define TRADE_PERSISTENCY_COMMON_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "trade_persistency.h"
#include "jtsqlite.h"

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */
typedef struct tp_config_sqlite
{
#define MAX_TP_CONFIG_NAME_LEN    (64)
    olchar_t tcs_strDbName[MAX_TP_CONFIG_NAME_LEN];  /*!< The SQLite DB name to be used with SQLitePersistence. */
} tp_config_sqlite_t;

typedef enum tp_type
{
    TP_TYPE_SQLITE = 0 /*!< SQLite3 backend DB service. */
} tp_type_t;

struct tp_manager;

typedef u32 (* fnFiniTradePersistency_t)(struct tp_manager * pMananger);

typedef u32 (* fnClearDataInTp_t)(struct tp_manager * pMananger);

typedef u32 (* fnStartTpTransaction_t)(struct tp_manager * pMananger);
typedef u32 (* fnCommitTpTransaction_t)(struct tp_manager * pMananger);
typedef u32 (* fnRollbackTpTransaction_t)(struct tp_manager * pMananger);

/*insert if pStock is not existing, otherwise replace the old one*/
typedef u32 (* fnReplacePoolStockIntoTp_t)(
    struct tp_manager * pMananger, trade_pool_stock_t * pStock);
/*insert if pStock is not existing, otherwise failed*/
typedef u32 (* fnInsertPoolStockIntoTp_t)(
    struct tp_manager * pMananger, trade_pool_stock_t * pStock);
typedef u32 (* fnUpdatePoolStockInTp_t)(
    struct tp_manager * pMananger, trade_pool_stock_t * pStock);
typedef u32 (* fnRemovePoolStockFromTp_t)(
    struct tp_manager * pMananger, trade_pool_stock_t * pStock);
typedef u32 (* fnGetAllPoolStockInTp_t)(
    struct tp_manager * pMananger, trade_pool_stock_t * pStock, olint_t * pnNum);
typedef u32 (* fnGetPoolStockInTp_t)(
    struct tp_manager * pMananger, trade_pool_stock_t * pStock);
typedef olint_t (* fnGetNumOfPoolStockInTp_t)(struct tp_manager * pMananger);

typedef olint_t (* fnGetNumOfTradingRecordInTp_t)(struct tp_manager * pMananger);
typedef u32 (* fnGetAllTradingRecordInTp_t)(
    struct tp_manager * pMananger, trade_trading_record_t * pRecord, olint_t * pnNum);
typedef u32 (* fnInsertTradingRecordIntoTp_t)(
    struct tp_manager * pMananger, trade_trading_record_t * pRecord);


typedef struct tp_sqlite
{
    tp_config_sqlite_t ts_tcsConfigSqlite;
    jt_sqlite_t ts_jsSqlite;
} tp_sqlite_t;

typedef union tp_data
{
    tp_sqlite_t td_tsSqlite;
} tp_data_t;

typedef struct tp_manager 
{
    tp_type_t tm_ttType; 

    fnFiniTradePersistency_t tm_fnFiniTp;

    fnClearDataInTp_t tm_fnClearData;

    fnStartTpTransaction_t tm_fnStartTransaction;
    fnCommitTpTransaction_t tm_fnCommitTransaction;
    fnRollbackTpTransaction_t tm_fnRollbackTransaction;

    fnReplacePoolStockIntoTp_t tm_fnReplacePoolStock;
    fnInsertPoolStockIntoTp_t tm_fnInsertPoolStock;
    fnUpdatePoolStockInTp_t tm_fnUpdatePoolStock;
    fnRemovePoolStockFromTp_t tm_fnRemovePoolStock;
    fnGetAllPoolStockInTp_t tm_fnGetAllPoolStock;
    fnGetPoolStockInTp_t tm_fnGetPoolStock;
    fnGetNumOfPoolStockInTp_t tm_fnGetNumOfPoolStock;

    fnGetNumOfTradingRecordInTp_t tm_fnGetNumOfTradingRecord;
    fnGetAllTradingRecordInTp_t tm_fnGetAllTradingRecord;
    fnInsertTradingRecordIntoTp_t tm_fnInsertTradingRecord;

    boolean_t tm_bTransactionStarted;
    boolean_t tm_bInitialized;
    u8 tm_u8Reserved[14];

    tp_data_t tm_tdData;

} tp_manager_t;

/* --- functional routines ------------------------------------------------- */


#endif /*TRADE_PERSISTENCY_COMMON_H*/

/*---------------------------------------------------------------------------*/


