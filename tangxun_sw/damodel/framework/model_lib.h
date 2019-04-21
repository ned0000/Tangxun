/**
 *  @file model_lib.h.h
 *
 *  @brief Header file for model library file
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_DAMODEL_LIB_FILE_H
#define TANGXUN_DAMODEL_LIB_FILE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"
#include "jf_file.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

boolean_t isDaModelLibFile(const olchar_t * pstrName);

u32 handleDaModelLibFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, jf_listhead_t * pjl);

#endif /*TANGXUN_DAMODEL_LIB_FILE_H*/

/*------------------------------------------------------------------------------------------------*/


