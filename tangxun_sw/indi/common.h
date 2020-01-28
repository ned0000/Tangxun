/**
 *  @file tangxun_sw/indi/common.h
 *
 *  @brief Header file for indicator common definition and routines.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_INDI_COMMON_H
#define TANGXUN_INDI_COMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_listhead.h"

#include "tx_indi.h"

/* --- constant definitions --------------------------------------------------------------------- */



/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

u32 freeIndicatorDataById(tx_ds_t * buffer, olint_t total, olint_t id);

#endif /*TANGXUN_INDI_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/


