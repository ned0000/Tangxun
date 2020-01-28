/**
 *  @file tangxun_sw/backtesting/common.h
 *
 *  @brief Header file for backtesting common definition and routines.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_BACKTESTING_COMMON_H
#define TANGXUN_BACKTESTING_COMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_listhead.h"

#include "tx_backtesting.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define BACKTESTING_OUTPUT_DIR           "output"
#define BACKTESTING_OUTPUT_FILE          "asset.txt"

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

u32 wrapupBtPoolStock(tx_backtesting_result_t * ptbr);

#endif /*TANGXUN_BACKTESTING_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/


