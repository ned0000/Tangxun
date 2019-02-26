/**
 *  @file downloaddata.h
 *
 *  @brief Download data
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_DOWNLOADDATA_H
#define TANGXUN_JIUTAI_DOWNLOADDATA_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    boolean_t ddp_bTradeSummary;
    boolean_t ddp_bTradeDetail;
    /*if true, overwrite the existing data file, otherwise ignore*/
    boolean_t ddp_bOverwrite;

    u8 ddp_u8Reserved[5];

    olchar_t * ddp_pstrStartDate;
    olchar_t * ddp_pstrEndDate;
    olchar_t * ddp_pstrDataDir;
    olchar_t * ddp_pstrStock;
    u32 ddp_u32Reserved[2];
} download_data_param_t;

typedef struct
{
    u64 qdp_u64Volume;
    oldouble_t qdp_dbPrice;
} quo_data_price_t;

typedef struct
{
    olchar_t qe_strCode[16];
    oldouble_t qe_dbOpeningPrice;
    oldouble_t qe_dbLastClosingPrice;
    oldouble_t qe_dbCurPrice;
    oldouble_t qe_dbHighPrice;
    oldouble_t qe_dbLowPrice;
    u64 qe_u64Volume;
    u64 qe_u64Amount;
    quo_data_price_t qe_qdpBuy[5];
    quo_data_price_t qe_qdpSold[5];
    olchar_t qe_strDate[16];
    olchar_t qe_strTime[16];
} quo_entry_t;

typedef struct
{
    olchar_t * gcqp_pstrStocks;
    u32 gcqp_u32Reserved[6];
    u32 gcqp_u32MaxEntry;
    u32 gcqp_u32NumOfEntry;
    quo_entry_t * gcqp_pqeEntry;
} get_cur_quo_param_t;

/* --- functional routines ------------------------------------------------- */
u32 downloadData(download_data_param_t * param);

u32 downloadStockInfoIndex(download_data_param_t * param);

void printQuoEntryVerbose(quo_entry_t * entry);

#endif /*TANGXUN_JIUTAI_DOWNLOADDATA_H*/

/*---------------------------------------------------------------------------*/


