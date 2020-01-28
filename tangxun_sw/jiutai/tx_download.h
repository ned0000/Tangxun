/**
 *  @file tx_download.h
 *
 *  @brief Download data.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_DOWNLOAD_H
#define TANGXUN_DOWNLOAD_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    /**If true, overwrite the existing data file, otherwise ignore.*/
    boolean_t tdtdp_bOverwrite;

    u8 tdtdp_u8Reserved[7];

    olchar_t * tdtdp_pstrStartDate;
    olchar_t * tdtdp_pstrEndDate;
    olchar_t * tdtdp_pstrDataDir;
    olchar_t * tdtdp_pstrStock;
    u32 tdtdp_u32Reserved[2];
} tx_download_trade_detail_param_t;

typedef struct
{
    /**If true, overwrite the existing data file, otherwise ignore.*/
    boolean_t tdtsp_bOverwrite;
    u8 tdtsp_u8Reserved[7];

    olchar_t * tdtsp_pstrDataDir;
    olchar_t * tdtsp_pstrStock;
    u32 tdtsp_u32Reserved[4];
} tx_download_trade_summary_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/* download day result file.*/

u32 tx_download_dlTradeDetail(tx_download_trade_detail_param_t * param);

/* download day summary file.*/

u32 tx_download_dlTradeSummary(tx_download_trade_summary_param_t * param);

/** Download day summary of all recorded index.
 */
u32 tx_download_dlIndexTradeSummary(tx_download_trade_summary_param_t * param);

#endif /*TANGXUN_DOWNLOAD_H*/

/*------------------------------------------------------------------------------------------------*/


