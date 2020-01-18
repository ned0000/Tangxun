/**
 *  @file model_common.h
 *
 *  @brief Header file for model common routines
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_DAMODEL_COMMON_H
#define TANGXUN_DAMODEL_COMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_listhead.h"

#include "tx_model.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 destroyDaModel(tx_model_t ** pptm);

u32 createDaModel(tx_model_t ** pptm);

u32 checkDaModelField(tx_model_t * ptm);

#endif /*TANGXUN_DAMODEL_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/


