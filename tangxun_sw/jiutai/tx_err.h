/**
 *  @file tx_err.h
 *
 *  @brief Header file for error code defintion.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# The object should be initialized after initializing jf_logger library.
 */

#ifndef TANGXUN_JIUTAI_ERR_H
#define TANGXUN_JIUTAI_ERR_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define JF_ERR_TANGXUN_ERROR_START  JF_ERR_VENDOR_SPEC_ERROR_START
#define JF_ERR_NOT_ENOUGH_TRADING_DAY  (JF_ERR_TANGXUN_ERROR_START + 0x0)
/* stock */
#define TX_ERR_STOCK_NOT_FOUND  (JF_ERR_TANGXUN_ERROR_START + 0x10)

/* model */
#define TX_ERR_MODEL_NOT_FOUND  (JF_ERR_TANGXUN_ERROR_START + 0x20)

/* rule */
#define TX_ERR_INVALID_RULE_ID  (JF_ERR_TANGXUN_ERROR_START + 0x30)

/* indi */
#define TX_ERR_INDI_NOT_FOUND  (JF_ERR_TANGXUN_ERROR_START + 0x40)

/* jiutai */

/* parsedata */

/* trade */

/* persistency */

/* statarbitrage */


/* --- data structures -------------------------------------------------------------------------- */



/* --- functional routines ---------------------------------------------------------------------- */

u32 tx_err_init(void);

u32 tx_err_fini(void);

#endif /*TANGXUN_JIUTAI_ERR_H*/

/*------------------------------------------------------------------------------------------------*/


