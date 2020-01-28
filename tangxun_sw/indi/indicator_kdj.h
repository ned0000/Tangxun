/**
 *  @file indicator_kdj.h
 *
 *  @brief Header file for KDJ indicator.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_INDICATOR_KDJ_H
#define TANGXUN_INDICATOR_KDJ_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_indi.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 indiKdjGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf);

u32 indiKdjGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf);

u32 indiKdjCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total);

u32 indiKdjSetDefaultParam(struct tx_indi * indi, void * param);


#endif /*TANGXUN_INDICATOR_KDJ_H*/

/*------------------------------------------------------------------------------------------------*/


