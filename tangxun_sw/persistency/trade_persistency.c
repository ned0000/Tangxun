/**
 *  @file trade_persistency.c
 *
 *  @brief Trade persistency library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_mem.h"

#include "tx_persistency.h"

#include "persistencycommon.h"
#include "sqlitepersistency.h"

/* --- private data/data structure section ------------------------------------------------------ */
static tp_manager_t ls_tmTpManager;

/* --- public routine section ------------------------------------------------------------------- */


/* --- private routine section ------------------------------------------------------------------ */

u32 tx_persistency_init(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    jf_logger_logInfoMsg("init trade persistency");

    if (ptm->tm_bInitialized)
        return u32Ret;

    ptm->tm_ttType = TP_TYPE_SQLITE;

    switch (ptm->tm_ttType)
    {
    case TP_TYPE_SQLITE:
        u32Ret = initTpSqlite(ptm);
        break;
    default:
        u32Ret = JF_ERR_INVALID_PARAM;
        break;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ptm->tm_bInitialized = TRUE;
    }
    else
    {
        memset(ptm, 0, sizeof(tp_manager_t));
    }
    
    return u32Ret;
}

u32 tx_persistency_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    u32Ret = ptm->tm_fnFiniTp(ptm);
    if (u32Ret == JF_ERR_NO_ERROR)
        memset(ptm, 0, sizeof(tp_manager_t));

    return u32Ret;
}


u32 tx_persistency_startTransaction(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    u32Ret = ptm->tm_fnStartTransaction(ptm);

    return u32Ret;
}


u32 tx_persistency_commitTransaction(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    u32Ret = ptm->tm_fnCommitTransaction(ptm);

    return u32Ret;
}


u32 tx_persistency_rollbackTransaction(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    u32Ret = ptm->tm_fnRollbackTransaction(ptm);

    return u32Ret;
}

u32 tx_persistency_replacePoolStock(tx_trade_pool_stock_t * pStock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    u32Ret = ptm->tm_fnReplacePoolStock(ptm, pStock);

    return u32Ret;
}

u32 tx_persistency_insertPoolStock(tx_trade_pool_stock_t * pStock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    jf_logger_logInfoMsg("insert pool stock, %s, %s", pStock->ttps_strStock, pStock->ttps_strModel);

    u32Ret = ptm->tm_fnInsertPoolStock(ptm, pStock);

    return u32Ret;
}

u32 tx_persistency_updatePoolStock(tx_trade_pool_stock_t * pStock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    u32Ret = ptm->tm_fnUpdatePoolStock(ptm, pStock);

    return u32Ret;
}

u32 tx_persistency_removePoolStock(tx_trade_pool_stock_t * pStock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    u32Ret = ptm->tm_fnRemovePoolStock(ptm, pStock);

    return u32Ret;
}

u32 tx_persistency_getAllPoolStock(tx_trade_pool_stock_t * pStock, olint_t * pnNum)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    u32Ret = ptm->tm_fnGetAllPoolStock(ptm, pStock, pnNum);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("get all pool stock, total %d stocks", *pnNum);
    }
    else
    {
        jf_logger_logErrMsg(u32Ret, "get all pool stock in tp, not found");
    }

    return u32Ret;
}

u32 tx_persistency_getPoolStock(tx_trade_pool_stock_t * pStock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    u32Ret = ptm->tm_fnGetPoolStock(ptm, pStock);

    return u32Ret;
}

olint_t tx_persistency_getNumOfPoolStock(void)
{
    tp_manager_t * ptm = &ls_tmTpManager;
    olint_t ret = ptm->tm_fnGetNumOfPoolStock(ptm);
    jf_logger_logInfoMsg("get num of pool stock in tp, %d", ret);
    return ret;
}

olint_t tx_persistency_getNumOfTradingRecord(void)
{
    tp_manager_t * ptm = &ls_tmTpManager;
    olint_t ret = ptm->tm_fnGetNumOfTradingRecord(ptm);
    jf_logger_logInfoMsg("get num trading record in tp, %d", ret);
    return ret;
}

u32 tx_persistency_getAllTradingRecord(tx_trade_trading_record_t * pRecord, olint_t * pnNum)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    u32Ret = ptm->tm_fnGetAllTradingRecord(ptm, pRecord, pnNum);

    return u32Ret;
}

u32 tx_persistency_insertTradingRecord(tx_trade_trading_record_t * pRecord)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    u32Ret = ptm->tm_fnInsertTradingRecord(ptm, pRecord);

    return u32Ret;
}

u32 tx_persistency_clearData(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_manager_t * ptm = &ls_tmTpManager;

    jf_logger_logInfoMsg("clear data in tp");
    u32Ret = ptm->tm_fnClearData(ptm);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/



