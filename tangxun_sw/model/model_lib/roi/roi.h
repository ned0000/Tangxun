/**
 *  @file roi.h
 *
 *  @brief Roi header file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_MODEL_ROI_H
#define TANGXUN_MODEL_ROI_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

#include "tx_model.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

typedef struct tx_model_roi_data
{
    u32 tmrd_u32Reserved[8];

} tx_model_roi_data_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 tx_model_fillModel(tx_model_t * ptm);

#endif /*TANGXUN_MODEL_ROI_H*/

/*------------------------------------------------------------------------------------------------*/


