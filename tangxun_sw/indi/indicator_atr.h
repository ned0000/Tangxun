/**
 *  @file indicator_atr.h
 *
 *  @brief Header file for ATR indicator.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_INDICATOR_ATR_H
#define TANGXUN_INDICATOR_ATR_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_indi.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 indiAtrGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf);

u32 indiAtrGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf);

u32 indiAtrCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total);

u32 indiAtrSetDefaultParam(struct tx_indi * indi, void * param);


#endif /*TANGXUN_INDICATOR_ATR_H*/

/*------------------------------------------------------------------------------------------------*/


