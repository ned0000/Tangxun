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

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    boolean_t tfp_bOverwrite;
    u8 tfp_bReserved[7];
    olint_t tfp_nReserved[7];
} tx_fixdata_param_t;

typedef struct
{
    olint_t tfr_nDeletedLine;

} tx_fixdata_result_t;

/* --- functional routines ---------------------------------------------------------------------- */
u32 tx_fixdata_fixDataFile(
    olchar_t * file, tx_fixdata_param_t * ptfp, tx_fixdata_result_t * pResult);

#endif /*TANGXUN_JIUTAI_FIXDATA_H*/

/*------------------------------------------------------------------------------------------------*/


