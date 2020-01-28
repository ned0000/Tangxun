/**
 *  @file indicator_obv.h
 *
 *  @brief Header file for OBV indicator.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_INDICATOR_OBV_H
#define TANGXUN_INDICATOR_OBV_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_indi.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 indiObvGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf);

u32 indiObvGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf);

u32 indiObvCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total);

u32 indiObvSetDefaultParam(struct tx_indi * indi, void * param);


#endif /*TANGXUN_INDICATOR_OBV_H*/

/*------------------------------------------------------------------------------------------------*/


