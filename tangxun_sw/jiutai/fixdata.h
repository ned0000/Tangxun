/**
 *  @file fixdata.h
 *
 *  @brief routine for fix data, the data from files or ...
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_FIXDATA_H
#define TANGXUN_JIUTAI_FIXDATA_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */

typedef struct
{
    boolean_t fp_bOverwrite;
    u8 fp_bReserved[7];
    olint_t fp_nReserved[7];
} fix_param_t;

typedef struct
{
    olint_t fr_nDeletedLine;

} fix_result_t;

/* --- functional routines ------------------------------------------------- */
u32 fixDataFile(olchar_t * file, fix_param_t * pfp, fix_result_t * pResult);

#endif /*TANGXUN_JIUTAI_FIXDATA_H*/

/*---------------------------------------------------------------------------*/


