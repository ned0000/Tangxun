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

#include "damodel.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 destroyDaModel(da_model_t ** ppdm);

u32 createDaModel(da_model_t ** ppdm);

u32 checkDaModelField(da_model_t * pdm);

#endif /*TANGXUN_DAMODEL_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/


