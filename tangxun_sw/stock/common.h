/**
 *  @file tangxun_sw/stock/common.h
 *
 *  @brief Header file for stock common definition and routines.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_STOCK_COMMON_H
#define TANGXUN_STOCK_COMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_listhead.h"

#include "tx_stock.h"

/* --- constant definitions --------------------------------------------------------------------- */



/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

/** Set the industry field in the stock info data structure.
 *
 *  @note
 *  -# The routine will increase the counter in stock indu data structure to record number of
 *   stock in the industry.
 */
u32 setStockIndustry(
    const olchar_t * pstrData, const olsize_t sData, tx_stock_info_t * stockinfo);

u32 classifyStockToIndu(void);

#endif /*TANGXUN_STOCK_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/


