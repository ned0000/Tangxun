/**
 *  @file indicator_mtm.c
 *
 *  @brief Implementation file for MTM indicator.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <math.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <stdlib.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_clieng.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_clieng.h"
#include "jf_matrix.h"
#include "jf_jiukun.h"

#include "tx_indi.h"
#include "tx_stock.h"
#include "tx_err.h"

#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _cleanMtmData(tx_indi_mtm_param_t * param, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bClean = TRUE;
    tx_ds_t * end = buffer + total - 1;
    olint_t index = 0;

    while (index < total)
    {
        /*Check if the MTM is already calculated.*/
        /*Check if the MTM is calculated with the same parameter.*/
        if ((end->td_ptimpMtmParam != NULL) &&
            (ol_memcmp(param, end->td_ptimpMtmParam, sizeof(*param)) == 0))
        {
            JF_LOGGER_DEBUG("no clean");
            bClean = FALSE;
            break;
        }

        end --;
        index ++;
    }

    if (bClean)
        u32Ret = freeIndicatorDataById(buffer, total, TX_INDI_ID_MTM);
    
    return u32Ret;
}

static u32 _calcMtmData(tx_indi_mtm_param_t * param, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NOT_IMPLEMENTED;

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 indiMtmGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const tx_indi_mtm_param_t * param = pParam;

    ol_sprintf(buf, "%d,%d", param->timp_nMtmDays, param->timp_nMtmMaDays);

    return u32Ret;
}

u32 indiMtmGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_mtm_param_t * param = pParam;

    ol_sscanf(buf, "%d,%d", &param->timp_nMtmDays, &param->timp_nMtmMaDays);

    return u32Ret;
}

u32 indiMtmCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_mtm_param_t * param = (tx_indi_mtm_param_t *)pParam;

    u32Ret = _cleanMtmData(param, buffer, total);

    /*Do the calculation.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _calcMtmData(param, buffer, total);

    return u32Ret;
}

u32 indiMtmSetDefaultParam(struct tx_indi * indi, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_mtm_param_t * param = (tx_indi_mtm_param_t *)pParam;

    ol_bzero(param, sizeof(*param));

	param->timp_nMtmDays = TX_INDI_DEF_MTM_DAYS;
	param->timp_nMtmMaDays = TX_INDI_DEF_MTM_MA_DAYS;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


