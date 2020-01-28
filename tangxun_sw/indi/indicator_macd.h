/**
 *  @file indicator_macd.h
 *
 *  @brief Header file for MACD indicator.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_INDICATOR_MACD_H
#define TANGXUN_INDICATOR_MACD_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_indi.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 indiMacdGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf);

u32 indiMacdGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf);

u32 indiMacdCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total);

u32 indiMacdSetDefaultParam(struct tx_indi * indi, void * param);


#endif /*TANGXUN_INDICATOR_MACD_H*/

/*------------------------------------------------------------------------------------------------*/


