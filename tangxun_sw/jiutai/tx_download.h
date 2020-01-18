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
    boolean_t tddp_bTradeSummary;
    boolean_t tddp_bTradeDetail;
    /*if true, overwrite the existing data file, otherwise ignore*/
    boolean_t tddp_bOverwrite;

    u8 tddp_u8Reserved[5];

    olchar_t * tddp_pstrStartDate;
    olchar_t * tddp_pstrEndDate;
    olchar_t * tddp_pstrDataDir;
    olchar_t * tddp_pstrStock;
    u32 tddp_u32Reserved[2];
} tx_download_data_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 tx_download_dlData(tx_download_data_param_t * param);

u32 tx_download_dlStockIndex(tx_download_data_param_t * param);

#endif /*TANGXUN_DOWNLOAD_H*/

/*------------------------------------------------------------------------------------------------*/


