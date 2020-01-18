/**
 *  @file model_manager.h
 *
 *  @brief The model manager header file
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_DAMODEL_MANAGER_H
#define TANGXUN_DAMODEL_MANAGER_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 addDaModel(jf_listhead_t * pjl, const char * pstrLibDir);

u32 removeDaModel(jf_listhead_t * pjl);

#endif /*TANGXUN_DAMODEL_MANAGER_H*/

/*------------------------------------------------------------------------------------------------*/


