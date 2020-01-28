/**
 *  @file tangxun_sw/stock/stockindu.h
 *
 *  @brief Header file for stock industry common definition and routines.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_STOCK_STOCKINDU_H
#define TANGXUN_STOCK_STOCKINDU_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_listhead.h"

#include "common.h"

/* --- constant definitions --------------------------------------------------------------------- */



/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

u32 initStockIndu(tx_stock_init_param_t * param);

u32 finiStockIndu(void);

/** Set the industry field in the stock info data structure.
 *
 *  @note
 *  -# The routine will increase the counter in stock indu data structure to record number of
 *   stock in the industry.
 */
u32 setStockIndustry(
    const olchar_t * pstrData, const olsize_t sData, tx_stock_info_t * stockinfo);

#endif /*TANGXUN_STOCK_STOCKINDU_H*/

/*------------------------------------------------------------------------------------------------*/


