/**
 *  @file tx_err.c
 *
 *  @brief Implementation file for error code.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"

#include "tx_err.h"

/* --- private data/data structure section ------------------------------------------------------ */



/* --- private routine section ------------------------------------------------------------------ */



/* --- public routine section ------------------------------------------------------------------- */

u32 tx_err_init(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_err_addCode(TX_ERR_STOCK_NOT_FOUND, "Stock not found");

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_err_addCode(TX_ERR_MODEL_NOT_FOUND, "Model not found.");

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_err_addCode(TX_ERR_INVALID_RULE_ID, "Invalid rule ID.");
    
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_err_addCode(TX_ERR_INDI_NOT_FOUND, "Indicator not found.");

    return u32Ret;
}

u32 tx_err_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


