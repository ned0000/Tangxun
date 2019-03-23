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

/* --- functional routines ------------------------------------------------- */

u32 downloadData(download_data_param_t * param);

u32 downloadStockInfoIndex(download_data_param_t * param);

#endif /*TANGXUN_JIUTAI_DOWNLOADDATA_H*/

/*---------------------------------------------------------------------------*/


