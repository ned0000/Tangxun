/**
 *  @file indicator_rsi.c
 *
 *  @brief Implementation file for RSI indicator.
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

static u32 _cleanRsiData(tx_indi_rsi_param_t * param, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bClean = TRUE;
    tx_ds_t * end = buffer + total - 1;
    olint_t index = 0;

    while (index < total)
    {
        /*Check if the RSI is already calculated.*/
        /*Check if the RSI is calculated with the same parameter.*/
        if ((end->td_ptirpRsiParam != NULL) &&
            (ol_memcmp(param, end->td_ptirpRsiParam, sizeof(*param)) == 0))
        {
            JF_LOGGER_DEBUG("no clean");
            bClean = FALSE;
            break;
        }

        end --;
        index ++;
    }

    if (bClean)
        u32Ret = freeIndicatorDataById(buffer, total, TX_INDI_ID_RSI);
    
    return u32Ret;
}

static u32 _calcRsiData(tx_indi_rsi_param_t * param, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NOT_IMPLEMENTED;

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 indiRsiGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const tx_indi_rsi_param_t * param = pParam;

    ol_sprintf(
        buf, "%d,%d,%d,%.1f,%.1f", param->tirp_nRsi1Days, param->tirp_nRsi2Days,
        param->tirp_nRsi3Days, param->tirp_dbMaxOpening1, param->tirp_dbMinCloseout1);

    return u32Ret;
}

u32 indiRsiGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_rsi_param_t * param = pParam;

    ol_sscanf(
        buf, "%d,%d,%d,%lf,%lf", &param->tirp_nRsi1Days, &param->tirp_nRsi2Days,
        &param->tirp_nRsi3Days, &param->tirp_dbMaxOpening1, &param->tirp_dbMinCloseout1);

    return u32Ret;
}

u32 indiRsiCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_rsi_param_t * param = (tx_indi_rsi_param_t *)pParam;

    u32Ret = _cleanRsiData(param, buffer, total);

    /*Do the calculation.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _calcRsiData(param, buffer, total);

    return u32Ret;
}

u32 indiRsiSetDefaultParam(struct tx_indi * indi, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_rsi_param_t * param = (tx_indi_rsi_param_t *)pParam;

    ol_bzero(param, sizeof(*param));

	param->tirp_nRsi1Days = TX_INDI_DEF_RSI1_DAYS;
    param->tirp_nRsi2Days = TX_INDI_DEF_RSI2_DAYS;
    param->tirp_nRsi3Days = TX_INDI_DEF_RSI3_DAYS;
    param->tirp_dbMaxOpening1 = TX_INDI_DEF_RSI_MAX_OPENING_1;
    param->tirp_dbMinCloseout1 = TX_INDI_DEF_RSI_MIN_CLOSEOUT_1;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


