/**
 *  @file roi.h
 *
 *  @brief Roi header file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_DAMODEL_ROI_H
#define TANGXUN_DAMODEL_ROI_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */
typedef struct da_model_roi_data
{
    u32 dmrd_u32Reserved[8];

} da_model_roi_data_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 createDaModel(jf_listhead_t * pjl);



#endif /*TANGXUN_DAMODEL_ROI_H*/

/*------------------------------------------------------------------------------------------------*/


