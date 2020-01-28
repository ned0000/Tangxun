/**
 *  @file indicator_mtm.h
 *
 *  @brief Header file for MTM indicator.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_INDICATOR_MTM_H
#define TANGXUN_INDICATOR_MTM_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_indi.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 indiMtmGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf);

u32 indiMtmGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf);

u32 indiMtmCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total);

u32 indiMtmSetDefaultParam(struct tx_indi * indi, void * param);


#endif /*TANGXUN_INDICATOR_MTM_H*/

/*------------------------------------------------------------------------------------------------*/


