/**
 *  @file tangxun_sw/stock/stocklist.h
 *
 *  @brief Header file for stock info common definition and routines.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_STOCK_STOCKLIST_H
#define TANGXUN_STOCK_STOCKLIST_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_listhead.h"

#include "common.h"

/* --- constant definitions --------------------------------------------------------------------- */



/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

u32 initStockList(tx_stock_init_param_t * param);

u32 finiStockList(void);

u32 findStockInfo(const olchar_t * name, tx_stock_info_t ** info);


#endif /*TANGXUN_STOCK_STOCKLIST_H*/

/*------------------------------------------------------------------------------------------------*/


