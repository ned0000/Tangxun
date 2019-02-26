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
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */
typedef struct
{
    boolean_t fp_bOverwrite;
    u8 fp_bReserved[7];
    olint_t fp_nReserved[7];
} fix_param_t;

/* --- functional routines ------------------------------------------------- */
u32 fixDataFile(olchar_t * file, fix_param_t * pfp);

#endif /*TANGXUN_JIUTAI_FIXDATA_H*/

/*---------------------------------------------------------------------------*/


