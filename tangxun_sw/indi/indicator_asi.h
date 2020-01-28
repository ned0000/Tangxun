/**
 *  @file indicator_asi.h
 *
 *  @brief Header file for ASI indicator.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_INDICATOR_ASI_H
#define TANGXUN_INDICATOR_ASI_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_indi.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 indiAsiGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf);

u32 indiAsiGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf);

u32 indiAsiCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total);

u32 indiAsiSetDefaultParam(struct tx_indi * indi, void * param);


#endif /*TANGXUN_INDICATOR_ASI_H*/

/*------------------------------------------------------------------------------------------------*/


