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

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "trade_persistency.h"
#include "persistencycommon.h"
#include "sqlitepersistency.h"
#include "stringparse.h"
#include "jtsqlite.h"

/* --- private data/data structure section --------------------------------- */

#define TP_SQLITE_DB_NAME                         "../db/trade.db"

#define TP_SQLITE_TABLE_POOL                      "pool"
#define TP_SQLITE_TABLE_POOL_COLUMN_STOCK         "stock"
#define TP_SQLITE_TABLE_POOL_COLUMN_MODEL         "model"
#define TP_SQLITE_TABLE_POOL_COLUMN_MODEL_PARAM   "modelParam"
#define TP_SQLITE_TABLE_POOL_COLUMN_ADD_DATE      "addDate"
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

/* --- private routine section---------------------------------------------- */

static u32 _rollbackSqliteTpTransaction(struct tp_manager * ptm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    u32Ret = rollbackJtSqliteTransaction(&pSqlite->ts_jsSqlite);

    return u32Ret;
}

static u32 _initSqliteTp(struct tp_manager * ptm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;
    tp_config_sqlite_t * ptcs = &ptm->tm_tdData.td_tsSqlite.ts_tcsConfigSqlite;
    jt_sqlite_param_t param;

    ol_bzero(&param, sizeof(jt_sqlite_param_t));
    param.jsp_pstrDbName = ptcs->tcs_strDbName;

    u32Ret = initJtSqlite(&pSqlite->ts_jsSqlite, &param);

    return u32Ret;
}


static u32 _finiSqliteTp(struct tp_manager * ptm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    u32Ret = finiJtSqlite(&pSqlite->ts_jsSqlite);

    return u32Ret;
}


static u32 _replacePoolStockIntoSqliteTp(
    struct tp_manager * ptm, trade_pool_stock_t * pStock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /*replace the value into the DB*/
    ol_snprintf(
        query, sizeof(query), "REPLACE INTO %s VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%d', '%.2f');",
        TP_SQLITE_TABLE_POOL,
        pStock->tps_strStock, pStock->tps_strModel, pStock->tps_strModelParam, pStock->tps_strAddDate,
        pStock->tps_strOp, pStock->tps_strOpRemark, pStock->tps_strTradeDate, pStock->tps_strPosition,
        pStock->tps_nVolume, pStock->tps_dbPrice);
    u32Ret = execJtSqliteSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));

    return u32Ret;
}

static u32 _insertPoolStockIntoSqliteTp(struct tp_manager * ptm, trade_pool_stock_t * pStock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /*insert the value into the DB*/
    ol_snprintf(
        query, sizeof(query), "INSERT INTO %s VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%d', '%.2f');",
        TP_SQLITE_TABLE_POOL,
        pStock->tps_strStock, pStock->tps_strModel, pStock->tps_strModelParam, pStock->tps_strAddDate,
        pStock->tps_strOp, pStock->tps_strOpRemark, pStock->tps_strTradeDate, pStock->tps_strPosition,
        pStock->tps_nVolume, pStock->tps_dbPrice);

    u32Ret = execJtSqliteSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));

    return u32Ret;
}

static u32 _updatePoolStockInSqliteTp(struct tp_manager * ptm, trade_pool_stock_t * pStock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /*insert the value into the DB*/
    ol_snprintf(
        query, sizeof(query), "UPDATE %s SET %s='%s', %s='%s', %s='%s', %s='%s', %s='%d', %s='%.2f' WHERE %s='%s' AND %s='%s';",
        TP_SQLITE_TABLE_POOL,
        TP_SQLITE_TABLE_POOL_COLUMN_OP, pStock->tps_strOp,
        TP_SQLITE_TABLE_POOL_COLUMN_OP_REMARK, pStock->tps_strOpRemark,
        TP_SQLITE_TABLE_POOL_COLUMN_TRADE_DATE, pStock->tps_strTradeDate,
        TP_SQLITE_TABLE_POOL_COLUMN_POSITION, pStock->tps_strPosition,
        TP_SQLITE_TABLE_POOL_COLUMN_VOLUME, pStock->tps_nVolume,
        TP_SQLITE_TABLE_POOL_COLUMN_PRICE, pStock->tps_dbPrice,
        TP_SQLITE_TABLE_POOL_COLUMN_STOCK, pStock->tps_strStock,
        TP_SQLITE_TABLE_POOL_COLUMN_MODEL, pStock->tps_strModel);

    u32Ret = execJtSqliteSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));

    return u32Ret;
}

static u32 _removePoolStockFromSqliteTp(
    struct tp_manager * ptm, trade_pool_stock_t * pStock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t query[512];
    olchar_t queryRet[512];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /* Update or insert the value into the DB */
    ol_snprintf(
        query, sizeof(query), "DELETE FROM %s WHERE %s='%s' and %s='%s';",
        TP_SQLITE_TABLE_POOL,
        TP_SQLITE_TABLE_POOL_COLUMN_STOCK, pStock->tps_strStock,
        TP_SQLITE_TABLE_POOL_COLUMN_MODEL, pStock->tps_strModel);
    u32Ret = execJtSqliteSql(
        &pSqlite->ts_jsSqlite, query, queryRet, sizeof(queryRet));

    return u32Ret;
}

static u32 _startSqliteTpTransaction(struct tp_manager * ptm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    u32Ret = startJtSqliteTransaction(&pSqlite->ts_jsSqlite);

    return u32Ret;
}


static u32 _commitSqliteTpTransaction(struct tp_manager * ptm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    u32Ret = commitJtSqliteTransaction(&pSqlite->ts_jsSqlite);

    return u32Ret;
}

static u32 _getAllPoolStockInSqliteTp(
    struct tp_manager * ptm, trade_pool_stock_t * pStock, olint_t * pnNum)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t count = 0;
    olchar_t query[512];
    sqlite3_stmt * statement;
    olint_t retCode = SQLITE_OK;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;
    trade_pool_stock_t * ptps = NULL;

    if (*pnNum == 0)
        return u32Ret;

    ol_snprintf(
        query, sizeof(query), "SELECT * FROM %s ORDER BY %s, %s;",
        TP_SQLITE_TABLE_POOL, TP_SQLITE_TABLE_POOL_COLUMN_ADD_DATE, TP_SQLITE_TABLE_POOL_COLUMN_MODEL);

    retCode = sqlite3_prepare_v2(
        pSqlite->ts_jsSqlite.js_psSqlite, query, -1, &statement, NULL);
    if (retCode != SQLITE_OK)
    {
        u32Ret = OLERR_SQL_EVAL_ERROR;
        logErrMsg(u32Ret, "get all stock, sqlite prepare error");
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ptps = pStock;
        ol_bzero(ptps, sizeof(*ptps));
        while ((retCode = sqlite3_step(statement)) == SQLITE_ROW)
        {
            ol_strncpy(ptps->tps_strStock, (olchar_t *)sqlite3_column_text(statement, 0), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(ptps->tps_strModel, (olchar_t *)sqlite3_column_text(statement, 1), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(ptps->tps_strModelParam, (olchar_t *)sqlite3_column_text(statement, 2), MAX_TRADE_MODEL_PARAM_LEN - 1);
            ol_strncpy(ptps->tps_strAddDate, (olchar_t *)sqlite3_column_text(statement, 3), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(ptps->tps_strOp, (olchar_t *)sqlite3_column_text(statement, 4), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(ptps->tps_strOpRemark, (olchar_t *)sqlite3_column_text(statement, 5), MAX_TRADE_OP_REMARK_LEN - 1);
            ol_strncpy(ptps->tps_strTradeDate, (olchar_t *)sqlite3_column_text(statement, 6), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(ptps->tps_strPosition, (olchar_t *)sqlite3_column_text(statement, 7), MAX_TRADE_FIELD_LEN - 1);
            ptps->tps_nVolume = sqlite3_column_int(statement, 8);
            ptps->tps_dbPrice = sqlite3_column_double(statement, 9);


            count ++;
            if (count >= *pnNum)
                break;

            ptps ++;
        }
        sqlite3_finalize(statement);

        if ((retCode != SQLITE_DONE) && (retCode != SQLITE_ROW))
        {
            u32Ret = OLERR_SQL_EVAL_ERROR;
            logErrMsg(u32Ret, "get all stock, sqlite step error");
        }
    }

    *pnNum = count;

    return u32Ret;
}

static u32 _getPoolStockInSqliteTp(
    struct tp_manager * ptm, trade_pool_stock_t * pStock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t query[512];
    sqlite3_stmt * statement;
    olint_t retCode = SQLITE_OK;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    ol_snprintf(
        query, sizeof(query), "SELECT * FROM %s WHERE %s='%s' and %s='%s';",
        TP_SQLITE_TABLE_POOL,
        TP_SQLITE_TABLE_POOL_COLUMN_STOCK, pStock->tps_strStock,
        TP_SQLITE_TABLE_POOL_COLUMN_MODEL, pStock->tps_strModel);

    retCode = sqlite3_prepare_v2(
        pSqlite->ts_jsSqlite.js_psSqlite, query, -1, &statement, NULL);
    if (retCode != SQLITE_OK)
    {
        u32Ret = OLERR_SQL_EVAL_ERROR;
        logErrMsg(u32Ret, "get stock, sqlite prepare error");
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        bzero(pStock, sizeof(*pStock));
        retCode = sqlite3_step(statement);
        if (retCode == SQLITE_ROW)
        {
            ol_strncpy(pStock->tps_strStock, (olchar_t *)sqlite3_column_text(statement, 0), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(pStock->tps_strModel, (olchar_t *)sqlite3_column_text(statement, 1), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(pStock->tps_strModelParam, (olchar_t *)sqlite3_column_text(statement, 2), MAX_TRADE_MODEL_PARAM_LEN - 1);
            ol_strncpy(pStock->tps_strAddDate, (olchar_t *)sqlite3_column_text(statement, 3), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(pStock->tps_strOp, (olchar_t *)sqlite3_column_text(statement, 4), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(pStock->tps_strOpRemark, (olchar_t *)sqlite3_column_text(statement, 5), MAX_TRADE_OP_REMARK_LEN - 1);
            ol_strncpy(pStock->tps_strTradeDate, (olchar_t *)sqlite3_column_text(statement, 6), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(pStock->tps_strPosition, (olchar_t *)sqlite3_column_text(statement, 7), MAX_TRADE_FIELD_LEN - 1);
            pStock->tps_nVolume = sqlite3_column_int(statement, 8);
            pStock->tps_dbPrice = sqlite3_column_double(statement, 9);
        }
        else
        {
            u32Ret = OLERR_NOT_FOUND;
        }
        sqlite3_finalize(statement);
    }

    return u32Ret;
}

static olint_t _getNumOfPoolStockInSqliteTp(struct tp_manager * ptm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t count = 0;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /*insert the value into the DB*/
    ol_snprintf(
        query, sizeof(query), "SELECT COUNT(*) FROM %s;",
        TP_SQLITE_TABLE_POOL);
    u32Ret = execJtSqliteSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = getS32FromString(queryresult, ol_strlen(queryresult), &count);
    }

    return count;
}

static u32 _getAllTradingRecordInSqliteTp(
    struct tp_manager * ptm, trade_trading_record_t * pStock, olint_t * pnNum)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t count = 0;
    olchar_t query[512];
    sqlite3_stmt * statement;
    olint_t retCode = SQLITE_OK;
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;
    trade_trading_record_t * pttr = NULL;

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
        u32Ret = OLERR_SQL_EVAL_ERROR;
        logErrMsg(u32Ret, "get all trading record, sqlite prepare error");
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        pttr = pStock;
        ol_bzero(pttr, sizeof(*pttr));
        while ((retCode = sqlite3_step(statement)) == SQLITE_ROW)
        {
            ol_strncpy(pttr->ttr_strStock, (olchar_t *)sqlite3_column_text(statement, 0), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(pttr->ttr_strModel, (olchar_t *)sqlite3_column_text(statement, 1), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(pttr->ttr_strModelParam, (olchar_t *)sqlite3_column_text(statement, 2), MAX_TRADE_MODEL_PARAM_LEN - 1);
            ol_strncpy(pttr->ttr_strAddDate, (olchar_t *)sqlite3_column_text(statement, 3), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(pttr->ttr_strOp, (olchar_t *)sqlite3_column_text(statement, 4), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(pttr->ttr_strOpRemark, (olchar_t *)sqlite3_column_text(statement, 5), MAX_TRADE_OP_REMARK_LEN - 1);
            ol_strncpy(pttr->ttr_strTradeDate, (olchar_t *)sqlite3_column_text(statement, 6), MAX_TRADE_FIELD_LEN - 1);
            ol_strncpy(pttr->ttr_strPosition, (olchar_t *)sqlite3_column_text(statement, 7), MAX_TRADE_FIELD_LEN - 1);
            pttr->ttr_nVolume = sqlite3_column_int(statement, 8);
            pttr->ttr_dbPrice = sqlite3_column_double(statement, 9);

            count ++;
            if (count >= *pnNum)
                break;

            pttr ++;
        }
        sqlite3_finalize(statement);

        if ((retCode != SQLITE_DONE) && (retCode != SQLITE_ROW))
        {
            u32Ret = OLERR_SQL_EVAL_ERROR;
            logErrMsg(u32Ret, "get all trading record, sqlite step error");
        }
    }

    *pnNum = count;

    return u32Ret;
}

static olint_t _getNumOfTradingRecordInSqliteTp(struct tp_manager * ptm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t count = 0;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /*insert the value into the DB*/
    ol_snprintf(
        query, sizeof(query), "SELECT COUNT(*) FROM %s;",
        TP_SQLITE_TABLE_RECORD);
    u32Ret = execJtSqliteSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = getS32FromString(queryresult, ol_strlen(queryresult), &count);
    }

    return count;
}

static u32 _insertTradingRecordInSqliteTp(
    struct tp_manager * ptm, trade_trading_record_t * pStock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    /*insert the value into the DB*/
    ol_snprintf(
        query, sizeof(query), "INSERT INTO %s VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%d', '%.2f');",
        TP_SQLITE_TABLE_RECORD,
        pStock->ttr_strStock, pStock->ttr_strModel, pStock->ttr_strModelParam, pStock->ttr_strAddDate,
        pStock->ttr_strOp, pStock->ttr_strOpRemark, pStock->ttr_strTradeDate,
        pStock->ttr_strPosition, pStock->ttr_nVolume, pStock->ttr_dbPrice);

    u32Ret = execJtSqliteSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));

    return u32Ret;
}

static u32 _clearDataInSqliteTp(struct tp_manager * ptm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t query[512];
    olchar_t queryresult[32];
    tp_sqlite_t * pSqlite = &ptm->tm_tdData.td_tsSqlite;

    ol_snprintf(
        query, sizeof(query), "DELETE FROM %s;",
        TP_SQLITE_TABLE_POOL);
    u32Ret = execJtSqliteSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));
    if (u32Ret != OLERR_NO_ERROR)
    {
        logErrMsg(
            u32Ret, "clear data in sqlite tp, failed to delete table %s",
            TP_SQLITE_TABLE_POOL);
    }

    ol_snprintf(
        query, sizeof(query), "DELETE FROM %s;", TP_SQLITE_TABLE_RECORD);
    u32Ret = execJtSqliteSql(
        &pSqlite->ts_jsSqlite, query, queryresult, sizeof(queryresult));
    if (u32Ret != OLERR_NO_ERROR)
    {
        logErrMsg(
            u32Ret, "clear data in sqlite tp, failed to delete table %s",
            TP_SQLITE_TABLE_RECORD);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 initTpSqlite(tp_manager_t * pManager)
{
    u32 u32Ret = OLERR_NO_ERROR;

    logInfoMsg("init tp sqlite");

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

/*---------------------------------------------------------------------------*/

