/**
 *  @file tradehelper.h
 *
 *  @brief Provide some helper routine for stock trade
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_TRADEHELPER_H
#define TANGXUN_TRADEHELPER_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "parsedata.h"
#include "xtime.h"
#include "damodel.h"
#include "trade_persistency.h"


/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */

u32 setStockFirstTradeDate(const char * strStockPath);

boolean_t isAfterStockFirstTradeDate(
    const stock_info_t * stockinfo, const olchar_t * pstrDate);

boolean_t isHoliday(olint_t year, olint_t mon, olint_t day);


#endif /*TANGXUN_TRADEHELPER_H*/

/*---------------------------------------------------------------------------*/


