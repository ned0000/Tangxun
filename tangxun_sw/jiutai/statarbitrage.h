/**
 *  @file statarbitrage.h
 *
 *  @brief statistic arbitrage
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_STATARBITRAGE_H
#define TANGXUN_JIUTAI_STATARBITRAGE_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "parsedata.h"
#include "stocklist.h"

/* --- constant definitions ------------------------------------------------ */
#define MIN_STAT_ARBI_DAY_SUMMARY  300

/* --- data structures ----------------------------------------------------- */

typedef struct
{
    stock_info_t * ssi_psiStock;
    da_day_summary_t * ssi_pddsSummary;
    olint_t ssi_nDaySummary;
} sa_stock_info_t;

enum stat_arbi_id
{
    STAT_ARBI_UNKNOWN = 0,
    STAT_ARBI_SPREAD_BOUNDARY,
    STAT_ARBI_DESC_STAT,
    STAT_ARBI_MAX,
};

typedef struct
{
    oldouble_t saip_dbMinCorrelation;
    olint_t saip_nDaySummary;
    olint_t saip_nCorrelationArray;
    u32 saip_u32Reserved[8];
} stat_arbi_indu_param_t;

typedef struct
{
    olchar_t saire_strStockPair[24];
    oldouble_t saire_dbCorrelation;
    olint_t saire_nInduId;
} stat_arbi_indu_result_entry_t;

typedef struct
{
    olint_t sair_nMaxPair;
    olint_t sair_nNumOfPair;
    stat_arbi_indu_result_entry_t * sair_psaireEntry;
} stat_arbi_indu_result_t;

typedef struct
{
#define DEF_SA_DB_BOUNDARY  20  
    oldouble_t sasp_dbBoundary;

} stat_arbi_sb_param_t;

typedef struct
{
#define DEF_SA_DS_BOUNDARY  10
    oldouble_t sadp_dbBoundary;

} stat_arbi_ds_param_t;

typedef union
{
    stat_arbi_sb_param_t sap_saspSb;
    stat_arbi_ds_param_t sap_sadpDs;
} stat_arbi_param_t;

struct stat_arbi_desc;

typedef u32 (* fnStatArbiStocks_t)(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * psap,
    sa_stock_info_t * stocks, da_conc_t * conc);
typedef u32 (* fnOptimizeStatArbi_t)(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * psap,
    sa_stock_info_t * stocks, da_conc_sum_t * conc);
typedef void (* fnPrintStatArbiParamVerbose_t)(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * pdip);
typedef void (* fnGetStringStatArbiParam_t)(
    struct stat_arbi_desc * arbi, const stat_arbi_param_t * psap, olchar_t * buf);
typedef void (* fnGetStatArbiParamFromString_t)(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * psap, const olchar_t * buf);
typedef void (* fnPrintStatArbiDescVerbose_t)(
    struct stat_arbi_desc * arbi);

typedef struct stat_arbi_desc
{
    olint_t sad_nId;
    olchar_t * sad_pstrName;
    olchar_t * sad_pstrDesc;

    fnStatArbiStocks_t sad_fnStatArbiStocks;
    fnOptimizeStatArbi_t sad_fnOptimize;
    fnPrintStatArbiParamVerbose_t sad_fnPrintParamVerbose;
    fnGetStringStatArbiParam_t sad_fnGetStringParam;
    fnGetStatArbiParamFromString_t sad_fnGetParamFromString;
    fnPrintStatArbiDescVerbose_t sad_fnPrintDescVerbose;
} stat_arbi_desc_t;

typedef struct
{
    boolean_t gbsap_bVerbose;
    u8 gbsap_u8Reserved[7];
    u32 gbsap_u32Reserved[8];
    oldouble_t gbsap_dbMinProfitTimeRatio;
} get_best_stat_arbi_param_t;

/* --- functional routines ------------------------------------------------- */

stat_arbi_desc_t * getStatArbiDesc(olint_t id);

u32 statArbiStock(
    olchar_t * pstrDataPath, stock_info_t * stockinfo, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result);

u32 statArbiIndustry(
    olchar_t * pstrDataPath, olint_t induid, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result);

u32 statArbiAllIndustry(
    olchar_t * pstrDataPath, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result);

u32 statArbiStockList(
    olchar_t * pstrDataPath, olchar_t * stocklist, olint_t nStock, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result);

u32 getBestStatArbiMethod(
    sa_stock_info_t * stocks, get_best_stat_arbi_param_t * pgbsap,
    olint_t * pnId, stat_arbi_param_t * psap, da_conc_sum_t * conc);

u32 newSaStockInfo(
    olchar_t * pstrDataPath, olchar_t * pstrStocks,
    sa_stock_info_t ** ppsastock, olint_t nDaySummary);

u32 freeSaStockInfo(sa_stock_info_t ** ppsastock);

oldouble_t getSaStockInfoCorrelation(
    sa_stock_info_t * sastock, olint_t nDaySummary);

#endif /*TANGXUN_JIUTAI_STATARBITRAGE_H*/

/*---------------------------------------------------------------------------*/


