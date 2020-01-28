/**
 *  @file tx_statarbi.h
 *
 *  @brief Statistic arbitrage.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_STATARBITRAGE_H
#define TANGXUN_STATARBITRAGE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

#include "tx_daysummary.h"
#include "tx_stock.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define TX_STATARBI_MAX_DAY_SUMMARY  (300)

/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    tx_stock_info_t * tss_ptsiStock;
    tx_ds_t * tss_ptdSummary;
    olint_t tss_nDaySummary;
} tx_statarbi_stock_t;

enum tx_statarbi_id
{
    TX_STATARBI_UNKNOWN = 0,
    TX_STATARBI_SPREAD_BOUNDARY,
    TX_STATARBI_DESC_STAT,
    TX_STATARBI_MAX,
};

typedef struct
{
    oldouble_t tsep_dbMinCorrelation;
    olint_t tsep_nDaySummary;
    olint_t tsep_nCorrelationArray;
    u32 tsep_u32Reserved[8];
} tx_statarbi_eval_param_t;

typedef struct
{
    olchar_t tsere_strStockPair[24];
    oldouble_t tsere_dbCorrelation;
    olint_t tsere_nInduId;
} tx_statarbi_eval_result_entry_t;

typedef struct
{
    olint_t tser_nMaxPair;
    olint_t tser_nNumOfPair;
    tx_statarbi_eval_result_entry_t * tser_ptsereEntry;
} tx_statarbi_eval_result_t;

typedef struct
{
#define DEF_SA_DB_BOUNDARY  20  
    oldouble_t tsdsp_dbBoundary;

} tx_statarbi_desc_sb_param_t;

typedef struct
{
#define DEF_SA_DS_BOUNDARY  10
    oldouble_t tsddp_dbBoundary;

} tx_statarbi_desc_ds_param_t;

typedef union
{
    tx_statarbi_desc_sb_param_t tsdp_tsdspSb;
    tx_statarbi_desc_ds_param_t tsdp_tsddpDs;
} tx_statarbi_desc_param_t;

struct tx_statarbi_desc;

typedef void (* tx_statarbi_fnGetStringParam_t)(
    struct tx_statarbi_desc * arbi, const tx_statarbi_desc_param_t * ptsdp, olchar_t * buf);
typedef void (* tx_statarbi_fnGetParamFromString_t)(
    struct tx_statarbi_desc * arbi, tx_statarbi_desc_param_t * ptsdp, const olchar_t * buf);

typedef struct tx_statarbi_desc
{
    olint_t tsd_nId;
    olchar_t * tsd_pstrName;
    olchar_t * tsd_pstrDesc;

    tx_statarbi_fnGetStringParam_t tsd_fnGetStringParam;
    tx_statarbi_fnGetParamFromString_t tsd_fnGetParamFromString;
} tx_statarbi_desc_t;

typedef struct
{
    boolean_t tsgbp_bVerbose;
    u8 tsgbp_u8Reserved[7];
    u32 tsgbp_u32Reserved[8];
    oldouble_t tsgbp_dbMinProfitTimeRatio;
} tx_statarbi_get_best_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

tx_statarbi_desc_t * tx_statarbi_getDesc(olint_t id);

u32 tx_statarbi_evalStock(
    olchar_t * pstrDataPath, tx_stock_info_t * stockinfo, tx_statarbi_eval_param_t * param,
    tx_statarbi_eval_result_t * result);

u32 tx_statarbi_evalIndu(
    olchar_t * pstrDataPath, olint_t induid, tx_statarbi_eval_param_t * param,
    tx_statarbi_eval_result_t * result);

u32 tx_statarbi_evalAllIndu(
    olchar_t * pstrDataPath, tx_statarbi_eval_param_t * param, tx_statarbi_eval_result_t * result);

u32 tx_statarbi_evalStockArray(
    olchar_t * pstrDataPath, tx_stock_info_t ** ppStocks, u32 u32Stock,
    tx_statarbi_eval_param_t * param, tx_statarbi_eval_result_t * result);

u32 tx_statarbi_evalStockList(
    olchar_t * pstrDataPath, olchar_t * stocklist, u32 u32Stock, tx_statarbi_eval_param_t * param,
    tx_statarbi_eval_result_t * result);

u32 tx_statarbi_newStockInfo(
    olchar_t * pstrDataPath, olchar_t * pstrStocks, tx_statarbi_stock_t ** ppsastock,
    olint_t nDaySummary);

u32 tx_statarbi_freeStockInfo(tx_statarbi_stock_t ** ppsastock);

oldouble_t tx_statarbi_getCorrelation(tx_statarbi_stock_t * sastock, olint_t nDaySummary);

oldouble_t tx_statarbi_getCorrelationWithIndex(tx_stock_info_t * info);

oldouble_t tx_statarbi_getCorrelationWithSmeIndex(tx_stock_info_t * info);

#endif /*TANGXUN_STATARBITRAGE_H*/

/*------------------------------------------------------------------------------------------------*/


