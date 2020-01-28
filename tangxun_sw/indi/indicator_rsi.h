/**
 *  @file indicator_rsi.h
 *
 *  @brief Header file for RSI indicator.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_INDICATOR_RSI_H
#define TANGXUN_INDICATOR_RSI_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_indi.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 indiRsiGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf);

u32 indiRsiGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf);

u32 indiRsiCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total);

u32 indiRsiSetDefaultParam(struct tx_indi * indi, void * param);


#endif /*TANGXUN_INDICATOR_RSI_H*/

/*------------------------------------------------------------------------------------------------*/


