/**
 *  @file damethod.h
 *
 *  @brief Method used by commands
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_DAMETHOD_H
#define TANGXUN_JIUTAI_DAMETHOD_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "stocklist.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */
oldouble_t getCorrelationWithIndex(stock_info_t * info);

oldouble_t getCorrelationWithSmeIndex(stock_info_t * info);


#endif /*TANGXUN_JIUTAI_DAMETHOD_H*/

/*---------------------------------------------------------------------------*/


