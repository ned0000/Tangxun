/**
 *  @file sqlitepersistency.c
 *
 *  @brief The sqlite persistency
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
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_string.h"
#include "jf_sqlite.h"

#include "tx_persistency.h"

#include "sqlitepersistency.h"
#include "persistencycommon.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define TP_SQLITE_DB_NAME                         "../db/trade.db"

#define TP_SQLITE_TABLE_POOL                      "pool"
#define TP_SQLITE_TABLE_POOL_COLUMN_STOCK         "stock"
#define TP_SQLITE_TABLE_POOL_COLUMN_MODEL         "model"
#define TP_SQLITE_TABLE_POOL_COLUMN_MODEL_PARAM   "modelParam"
#define TP_SQLITE_TABLE_POOL_COLUMN_ADD_DATE      "addDate"
#define TP_SQLITE_TABLE_POOL_COLUMN_START_DATE_OF_DAY_SUMMARY "startDateOfDaySummary"
#define TP_SQLITE_TABLE_POOL_COLUMN_NUM_OF_DAY_SUMMARY "numOfDaySummary"
#define TP_SQLITE_TABLE_POOL_COLUMN_OP            "op"
#define TP_SQLITE_TABLE_POOL_COLUMN_OP_REMARK     "opRemark"
#define TP_SQLITE_TABLE_POOL_COLUMN_TRADE_DATE    "tradeDate"
#define TP_SQLITE_TABLE_POOL_COLUMN_POSITION      "position"
#define TP_SQLITE_TABLE_POOL_COLUMN_VOLUME        "volume"
#define TP_SQLITE_TABLE_POOL_COLUMN_PRICE         "price"

#define TP_SQLITE_TABLE_RECORD                    "record"
#define TP_SQLITE_TABLE_RECORD_COLUMN_STOCK       "stock"
#define TP_SQLITE_TABLE_RECORD_COLUMN_MODEL       "model"
#define TP_SQLITE_TABLE_RECORD_COLUMN_MODEL_PARAM "modelParam"
#define TP_SQLITE_TABLE_RECORD_COLUMN_ADD_DATE    "addDate"
#define TP_SQLITE_TABLE_RECORD_COLUMN_OP          "op"
#define TP_SQLITE_TABLE_RECORD_COLUMN_OP_REMARK   "opRemark"
#define TP_SQLITE_TABLE_RECORD_COLUMN_TRADE_DATE  "tradeDate"
#define TP_SQLITE_TABLE_RECORD_COLUMN_POSITION    "position"
#define TP_SQLITE_TABLE_RECORD_COLUMN_VOLUME      "volume"
#define TP_SQLITE_TABLE_RECORD_COLUMN_PRICE       "price"

/* --- private routine section ------------------------------------------------------------------ */

static u32 _rollbackSqliteTpTransaction(struct tp_manager * ptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    u32Ret = jf_sqlite_rollbackTransaction(&pSqlite->ts_jsSqlite);

    return u32Ret;
}

static u32 _initSqliteTp(struct tp_manager * ptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;
    tp_config_sqlite_t * ptcs = &ptm->tm_tdData.td_tsSqlite.ts_tcsConfigSqlite;
    jf_sqlite_init_param_t param;

    ol_bzero(&param, sizeof(jf_sqlite_init_param_t));
    param.jsip_pstrDbName = ptcs->tcs_strDbName;

    u32Ret = jf_sqlite_init(&pSqlite->ts_jsSqlite, &param);

    return u32Ret;
}


static u32 _finiSqliteTp(struct tp_manager * ptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    u32Ret = jf_sqlite_fini(&pSqlite->ts_jsSqlite);

    return u32Ret;
}


static u32 _replacePoolStockIntoSqliteTp(
    struct tp_manager * ptm, tx_trade_pool_stock_t * pStock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /*replace the value into the DB*/
    ol_snprintf(
        query, sizeof(query), "REPLACE INTO %s VALUES('%s', '%s', '%s', '%s', '%s', '%d', '%s', '%s', '%s', '%s', '%d', '%.2f');",
        TP_SQLITE_TABLE_POOL,
        pStock->ttps_strStock, pStock->ttps_strModel, pStock->ttps_strModelParam, pStock->ttps_strAddDate,
        pStock->ttps_strStartDateOfDaySummary, pStock->ttps_nNumOfDaySummary,
        pStock->ttps_strOp, pStock->ttps_strOpRemark, pStock->ttps_strTradeDate, pStock->ttps_strPosition,
        pStock->ttps_nVolume, pStock->ttps_dbPrice);
    u32Ret = jf_sqlite_execSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));

    return u32Ret;
}

static u32 _insertPoolStockIntoSqliteTp(struct tp_manager * ptm, tx_trade_pool_stock_t * pStock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /*insert the value into the DB*/
    ol_snprintf(
        query, sizeof(query), "INSERT INTO %s VALUES('%s', '%s', '%s', '%s', '%s', '%d', '%s', '%s', '%s', '%s', '%d', '%.2f');",
        TP_SQLITE_TABLE_POOL,
        pStock->ttps_strStock, pStock->ttps_strModel, pStock->ttps_strModelParam, pStock->ttps_strAddDate,
        pStock->ttps_strStartDateOfDaySummary, pStock->ttps_nNumOfDaySummary,
        pStock->ttps_strOp, pStock->ttps_strOpRemark, pStock->ttps_strTradeDate, pStock->ttps_strPosition,
        pStock->ttps_nVolume, pStock->ttps_dbPrice);

    u32Ret = jf_sqlite_execSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));

    return u32Ret;
}

static u32 _updatePoolStockInSqliteTp(struct tp_manager * ptm, tx_trade_pool_stock_t * pStock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /*insert the value into the DB*/
    ol_snprintf(
        query, sizeof(query), "UPDATE %s SET %s='%s', %s='%s', %s='%s', %s='%s', %s='%d', %s='%.2f' WHERE %s='%s' AND %s='%s';",
        TP_SQLITE_TABLE_POOL,
        TP_SQLITE_TABLE_POOL_COLUMN_OP, pStock->ttps_strOp,
        TP_SQLITE_TABLE_POOL_COLUMN_OP_REMARK, pStock->ttps_strOpRemark,
        TP_SQLITE_TABLE_POOL_COLUMN_TRADE_DATE, pStock->ttps_strTradeDate,
        TP_SQLITE_TABLE_POOL_COLUMN_POSITION, pStock->ttps_strPosition,
        TP_SQLITE_TABLE_POOL_COLUMN_VOLUME, pStock->ttps_nVolume,
        TP_SQLITE_TABLE_POOL_COLUMN_PRICE, pStock->ttps_dbPrice,
        TP_SQLITE_TABLE_POOL_COLUMN_STOCK, pStock->ttps_strStock,
        TP_SQLITE_TABLE_POOL_COLUMN_MODEL, pStock->ttps_strModel);

    u32Ret = jf_sqlite_execSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));

    return u32Ret;
}

static u32 _removePoolStockFromSqliteTp(
    struct tp_manager * ptm, tx_trade_pool_stock_t * pStock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t query[512];
    olchar_t queryRet[512];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /* Update or insert the value into the DB */
    ol_snprintf(
        query, sizeof(query), "DELETE FROM %s WHERE %s='%s' and %s='%s';",
        TP_SQLITE_TABLE_POOL,
        TP_SQLITE_TABLE_POOL_COLUMN_STOCK, pStock->ttps_strStock,
        TP_SQLITE_TABLE_POOL_COLUMN_MODEL, pStock->ttps_strModel);
    u32Ret = jf_sqlite_execSql(
        &pSqlite->ts_jsSqlite, query, queryRet, sizeof(queryRet));

    return u32Ret;
}

static u32 _startSqliteTpTransaction(struct tp_manager * ptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    u32Ret = jf_sqlite_startTransaction(&pSqlite->ts_jsSqlite);

    return u32Ret;
}


static u32 _commitSqliteTpTransaction(struct tp_manager * ptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    u32Ret = jf_sqlite_commitTransaction(&pSqlite->ts_jsSqlite);

    return u32Ret;
}

static u32 _getAllPoolStockInSqliteTp(
    struct tp_manager * ptm, tx_trade_pool_stock_t * pStock, olint_t * pnNum)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count = 0;
    olchar_t query[512];
    sqlite3_stmt * statement;
    olint_t retCode = SQLITE_OK;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;
    tx_trade_pool_stock_t * pttps = NULL;

    if (*pnNum == 0)
        return u32Ret;

    ol_snprintf(
        query, sizeof(query), "SELECT * FROM %s ORDER BY %s, %s;",
        TP_SQLITE_TABLE_POOL, TP_SQLITE_TABLE_POOL_COLUMN_ADD_DATE, TP_SQLITE_TABLE_POOL_COLUMN_MODEL);

    retCode = sqlite3_prepare_v2(
        pSqlite->ts_jsSqlite.js_psSqlite, query, -1, &statement, NULL);
    if (retCode != SQLITE_OK)
    {
        u32Ret = JF_ERR_SQL_EVAL_ERROR;
        jf_logger_logErrMsg(u32Ret, "get all stock, sqlite prepare error");
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pttps = pStock;
        ol_bzero(pttps, sizeof(*pttps));
        while ((retCode = sqlite3_step(statement)) == SQLITE_ROW)
        {
            ol_strncpy(pttps->ttps_strStock, (olchar_t *)sqlite3_column_text(statement, 0), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(pttps->ttps_strModel, (olchar_t *)sqlite3_column_text(statement, 1), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(pttps->ttps_strModelParam, (olchar_t *)sqlite3_column_text(statement, 2), TX_TRADE_MAX_MODEL_PARAM_LEN - 1);
            ol_strncpy(pttps->ttps_strAddDate, (olchar_t *)sqlite3_column_text(statement, 3), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(pttps->ttps_strStartDateOfDaySummary, (olchar_t *)sqlite3_column_text(statement, 4), TX_TRADE_MAX_FIELD_LEN - 1);
            pttps->ttps_nNumOfDaySummary = sqlite3_column_int(statement, 5);
            ol_strncpy(pttps->ttps_strOp, (olchar_t *)sqlite3_column_text(statement, 6), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(pttps->ttps_strOpRemark, (olchar_t *)sqlite3_column_text(statement, 7), TX_TRADE_MAX_OP_REMARK_LEN - 1);
            ol_strncpy(pttps->ttps_strTradeDate, (olchar_t *)sqlite3_column_text(statement, 8), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(pttps->ttps_strPosition, (olchar_t *)sqlite3_column_text(statement, 9), TX_TRADE_MAX_FIELD_LEN - 1);
            pttps->ttps_nVolume = sqlite3_column_int(statement, 10);
            pttps->ttps_dbPrice = sqlite3_column_double(statement, 11);


            count ++;
            if (count >= *pnNum)
                break;

            pttps ++;
        }
        sqlite3_finalize(statement);

        if ((retCode != SQLITE_DONE) && (retCode != SQLITE_ROW))
        {
            u32Ret = JF_ERR_SQL_EVAL_ERROR;
            jf_logger_logErrMsg(u32Ret, "get all stock, sqlite step error");
        }
    }

    *pnNum = count;

    return u32Ret;
}

static u32 _getPoolStockInSqliteTp(
    struct tp_manager * ptm, tx_trade_pool_stock_t * pStock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t query[512];
    sqlite3_stmt * statement;
    olint_t retCode = SQLITE_OK;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    ol_snprintf(
        query, sizeof(query), "SELECT * FROM %s WHERE %s='%s' and %s='%s';",
        TP_SQLITE_TABLE_POOL,
        TP_SQLITE_TABLE_POOL_COLUMN_STOCK, pStock->ttps_strStock,
        TP_SQLITE_TABLE_POOL_COLUMN_MODEL, pStock->ttps_strModel);

    retCode = sqlite3_prepare_v2(
        pSqlite->ts_jsSqlite.js_psSqlite, query, -1, &statement, NULL);
    if (retCode != SQLITE_OK)
    {
        u32Ret = JF_ERR_SQL_EVAL_ERROR;
        jf_logger_logErrMsg(u32Ret, "get stock, sqlite prepare error");
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        bzero(pStock, sizeof(*pStock));
        retCode = sqlite3_step(statement);
        if (retCode == SQLITE_ROW)
        {
            ol_strncpy(pStock->ttps_strStock, (olchar_t *)sqlite3_column_text(statement, 0), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(pStock->ttps_strModel, (olchar_t *)sqlite3_column_text(statement, 1), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(pStock->ttps_strModelParam, (olchar_t *)sqlite3_column_text(statement, 2), TX_TRADE_MAX_MODEL_PARAM_LEN - 1);
            ol_strncpy(pStock->ttps_strAddDate, (olchar_t *)sqlite3_column_text(statement, 3), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(pStock->ttps_strStartDateOfDaySummary, (olchar_t *)sqlite3_column_text(statement, 4), TX_TRADE_MAX_FIELD_LEN - 1);
            pStock->ttps_nNumOfDaySummary = sqlite3_column_int(statement, 5);
            ol_strncpy(pStock->ttps_strOp, (olchar_t *)sqlite3_column_text(statement, 6), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(pStock->ttps_strOpRemark, (olchar_t *)sqlite3_column_text(statement, 7), TX_TRADE_MAX_OP_REMARK_LEN - 1);
            ol_strncpy(pStock->ttps_strTradeDate, (olchar_t *)sqlite3_column_text(statement, 8), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(pStock->ttps_strPosition, (olchar_t *)sqlite3_column_text(statement, 9), TX_TRADE_MAX_FIELD_LEN - 1);
            pStock->ttps_nVolume = sqlite3_column_int(statement, 10);
            pStock->ttps_dbPrice = sqlite3_column_double(statement, 11);
        }
        else
        {
            u32Ret = JF_ERR_NOT_FOUND;
        }
        sqlite3_finalize(statement);
    }

    return u32Ret;
}

static olint_t _getNumOfPoolStockInSqliteTp(struct tp_manager * ptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count = 0;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /*insert the value into the DB*/
    ol_snprintf(
        query, sizeof(query), "SELECT COUNT(*) FROM %s;",
        TP_SQLITE_TABLE_POOL);
    u32Ret = jf_sqlite_execSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_getS32FromString(queryresult, ol_strlen(queryresult), &count);
    }

    return count;
}

static u32 _getAllTradingRecordInSqliteTp(
    struct tp_manager * ptm, tx_trade_trading_record_t * pStock, olint_t * pnNum)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count = 0;
    olchar_t query[512];
    sqlite3_stmt * statement;
    olint_t retCode = SQLITE_OK;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;
    tx_trade_trading_record_t * ptttr = NULL;

    if (*pnNum == 0)
        return u32Ret;

    ol_snprintf(
        query, sizeof(query), "SELECT * FROM %s ORDER BY %s, %s, %s;",
        TP_SQLITE_TABLE_RECORD, TP_SQLITE_TABLE_RECORD_COLUMN_STOCK,
        TP_SQLITE_TABLE_RECORD_COLUMN_MODEL, TP_SQLITE_TABLE_RECORD_COLUMN_TRADE_DATE);

    retCode = sqlite3_prepare_v2(
        pSqlite->ts_jsSqlite.js_psSqlite, query, -1, &statement, NULL);
    if (retCode != SQLITE_OK)
    {
        u32Ret = JF_ERR_SQL_EVAL_ERROR;
        jf_logger_logErrMsg(u32Ret, "get all trading record, sqlite prepare error");
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ptttr = pStock;
        ol_bzero(ptttr, sizeof(*ptttr));
        while ((retCode = sqlite3_step(statement)) == SQLITE_ROW)
        {
            ol_strncpy(ptttr->tttr_strStock, (olchar_t *)sqlite3_column_text(statement, 0), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(ptttr->tttr_strModel, (olchar_t *)sqlite3_column_text(statement, 1), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(ptttr->tttr_strModelParam, (olchar_t *)sqlite3_column_text(statement, 2), TX_TRADE_MAX_MODEL_PARAM_LEN - 1);
            ol_strncpy(ptttr->tttr_strAddDate, (olchar_t *)sqlite3_column_text(statement, 3), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(ptttr->tttr_strOp, (olchar_t *)sqlite3_column_text(statement, 4), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(ptttr->tttr_strOpRemark, (olchar_t *)sqlite3_column_text(statement, 5), TX_TRADE_MAX_OP_REMARK_LEN - 1);
            ol_strncpy(ptttr->tttr_strTradeDate, (olchar_t *)sqlite3_column_text(statement, 6), TX_TRADE_MAX_FIELD_LEN - 1);
            ol_strncpy(ptttr->tttr_strPosition, (olchar_t *)sqlite3_column_text(statement, 7), TX_TRADE_MAX_FIELD_LEN - 1);
            ptttr->tttr_nVolume = sqlite3_column_int(statement, 8);
            ptttr->tttr_dbPrice = sqlite3_column_double(statement, 9);

            count ++;
            if (count >= *pnNum)
                break;

            ptttr ++;
        }
        sqlite3_finalize(statement);

        if ((retCode != SQLITE_DONE) && (retCode != SQLITE_ROW))
        {
            u32Ret = JF_ERR_SQL_EVAL_ERROR;
            jf_logger_logErrMsg(u32Ret, "get all trading record, sqlite step error");
        }
    }

    *pnNum = count;

    return u32Ret;
}

static olint_t _getNumOfTradingRecordInSqliteTp(struct tp_manager * ptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count = 0;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /*insert the value into the DB*/
    ol_snprintf(
        query, sizeof(query), "SELECT COUNT(*) FROM %s;",
        TP_SQLITE_TABLE_RECORD);
    u32Ret = jf_sqlite_execSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_getS32FromString(queryresult, ol_strlen(queryresult), &count);
    }

    return count;
}

static u32 _insertTradingRecordInSqliteTp(
    struct tp_manager * ptm, tx_trade_trading_record_t * pStock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /*insert the value into the DB*/
    ol_snprintf(
        query, sizeof(query), "INSERT INTO %s VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%d', '%.2f');",
        TP_SQLITE_TABLE_RECORD,
        pStock->tttr_strStock, pStock->tttr_strModel, pStock->tttr_strModelParam, pStock->tttr_strAddDate,
        pStock->tttr_strOp, pStock->tttr_strOpRemark, pStock->tttr_strTradeDate,
        pStock->tttr_strPosition, pStock->tttr_nVolume, pStock->tttr_dbPrice);

    u32Ret = jf_sqlite_execSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));

    return u32Ret;
}

static u32 _clearDataInSqliteTp(struct tp_manager * ptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    ol_snprintf(
        query, sizeof(query), "DELETE FROM %s;",
        TP_SQLITE_TABLE_POOL);
    u32Ret = jf_sqlite_execSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_logger_logErrMsg(
            u32Ret, "clear data in sqlite tp, failed to delete table %s",
            TP_SQLITE_TABLE_POOL);
    }

    ol_snprintf(
        query, sizeof(query), "DELETE FROM %s;", TP_SQLITE_TABLE_RECORD);
    u32Ret = jf_sqlite_execSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_logger_logErrMsg(
            u32Ret, "clear data in sqlite tp, failed to delete table %s",
            TP_SQLITE_TABLE_RECORD);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initTpSqlite(tp_manager_t * pManager)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("init tp sqlite");

    ol_strcpy(
        pManager->tm_tdData.td_tsSqlite.ts_tcsConfigSqlite.tcs_strDbName,
        TP_SQLITE_DB_NAME);

    pManager->tm_fnFiniTp = _finiSqliteTp;

    pManager->tm_fnClearData = _clearDataInSqliteTp;

    pManager->tm_fnStartTransaction = _startSqliteTpTransaction;
    pManager->tm_fnCommitTransaction = _commitSqliteTpTransaction;
    pManager->tm_fnRollbackTransaction = _rollbackSqliteTpTransaction;

    pManager->tm_fnReplacePoolStock = _replacePoolStockIntoSqliteTp;
    pManager->tm_fnInsertPoolStock = _insertPoolStockIntoSqliteTp;
    pManager->tm_fnUpdatePoolStock = _updatePoolStockInSqliteTp;
    pManager->tm_fnRemovePoolStock = _removePoolStockFromSqliteTp;
    pManager->tm_fnGetAllPoolStock = _getAllPoolStockInSqliteTp;
    pManager->tm_fnGetPoolStock = _getPoolStockInSqliteTp;
    pManager->tm_fnGetNumOfPoolStock = _getNumOfPoolStockInSqliteTp;

    pManager->tm_fnGetNumOfTradingRecord = _getNumOfTradingRecordInSqliteTp;
    pManager->tm_fnGetAllTradingRecord = _getAllTradingRecordInSqliteTp;
    pManager->tm_fnInsertTradingRecord = _insertTradingRecordInSqliteTp;

    u32Ret = _initSqliteTp(pManager);
    
    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

