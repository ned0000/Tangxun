/**
 *  @file tangxun_sw/model/model_lib/example/example.h
 *
 *  @brief Example model header file.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef TANGXUN_MODEL_EXAMPLE_H
#define TANGXUN_MODEL_EXAMPLE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

#include "tx_model.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

typedef struct tx_model_example_data
{
    u32 tmed_u32Reserved[8];

} tx_model_example_data_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 tx_model_fillModel(tx_model_t * ptm);

#endif /*TANGXUN_MODEL_EXAMPLE_H*/

/*------------------------------------------------------------------------------------------------*/


