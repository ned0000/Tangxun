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

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "tx_stock.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */
oldouble_t getCorrelationWithIndex(tx_stock_info_t * info);

oldouble_t getCorrelationWithSmeIndex(tx_stock_info_t * info);


#endif /*TANGXUN_JIUTAI_DAMETHOD_H*/

/*------------------------------------------------------------------------------------------------*/


