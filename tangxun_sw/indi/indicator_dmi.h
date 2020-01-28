/**
 *  @file indicator_dmi.h
 *
 *  @brief Header file for DMI indicator.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_INDICATOR_DMI_H
#define TANGXUN_INDICATOR_DMI_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_indi.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 indiDmiGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf);

u32 indiDmiGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf);

u32 indiDmiCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total);

u32 indiDmiSetDefaultParam(struct tx_indi * indi, void * param);


#endif /*TANGXUN_INDICATOR_DMI_H*/

/*------------------------------------------------------------------------------------------------*/


