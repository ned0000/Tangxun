/**
 *  @file model_common.h
 *
 *  @brief Header file for model common routines
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_TXMODEL_COMMON_H
#define TANGXUN_TXMODEL_COMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_listhead.h"

#include "tx_model.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 destroyTxModel(tx_model_t ** pptm);

u32 createTxModel(tx_model_t ** pptm);

u32 checkTxModelField(tx_model_t * ptm);

#endif /*TANGXUN_TXMODEL_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/


