/**
 *  @file indicator_kdj.c
 *
 *  @brief Implementation file for KDJ indicator.
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

static u32 _cleanKdjData(tx_indi_kdj_param_t * param, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bClean = TRUE;
    tx_ds_t * end = buffer + total - 1;
    olint_t index = 0;

    while (index < total)
    {
        /*Check if the KDJ is already calculated.*/
        /*Check if the KDJ is calculated with the same parameter.*/
        if ((end->td_ptikpKdjParam != NULL) &&
            (ol_memcmp(param, end->td_ptikpKdjParam, sizeof(*param)) == 0))
        {
            JF_LOGGER_DEBUG("no clean");
            bClean = FALSE;
            break;
        }

        end --;
        index ++;
    }

    if (bClean)
        u32Ret = freeIndicatorDataById(buffer, total, TX_INDI_ID_KDJ);
    
    return u32Ret;
}

static u32 _calcKdjData(tx_indi_kdj_param_t * param, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NOT_IMPLEMENTED;


    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 indiKdjGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const tx_indi_kdj_param_t * param = pParam;

    ol_sprintf(
        buf, "%d,%d,%d,%.1f,%.1f", param->tikp_nKdjNDays, param->tikp_nKdjM1Days,
        param->tikp_nKdjM2Days, param->tikp_dbMaxOpeningD, param->tikp_dbMinCloseoutJ);

    return u32Ret;
}

u32 indiKdjGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_kdj_param_t * param = pParam;

    ol_sscanf(
        buf, "%d,%d,%d,%lf,%lf", &param->tikp_nKdjNDays, &param->tikp_nKdjM1Days,
        &param->tikp_nKdjM2Days, &param->tikp_dbMaxOpeningD, &param->tikp_dbMinCloseoutJ);

    return u32Ret;
}

u32 indiKdjCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_kdj_param_t * param = (tx_indi_kdj_param_t *)pParam;

    u32Ret = _cleanKdjData(param, buffer, total);

    /*Do the calculation.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _calcKdjData(param, buffer, total);

    return u32Ret;
}

u32 indiKdjSetDefaultParam(struct tx_indi * indi, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_kdj_param_t * param = (tx_indi_kdj_param_t *)pParam;

    ol_bzero(param, sizeof(*param));

	param->tikp_nKdjNDays = TX_INDI_DEF_KDJ_N_DAYS;
	param->tikp_nKdjM1Days = TX_INDI_DEF_KDJ_M1_DAYS;
    param->tikp_nKdjM2Days = TX_INDI_DEF_KDJ_M2_DAYS;
    param->tikp_dbMaxOpeningD = TX_INDI_DEF_KDJ_MAX_OPENING_D;
    param->tikp_dbMinCloseoutJ = TX_INDI_DEF_KDJ_MIN_CLOSEOUT_J;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


