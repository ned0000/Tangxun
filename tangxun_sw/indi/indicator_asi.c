/**
 *  @file indicator_asi.c
 *
 *  @brief Implementation file for ASI indicator.
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

static u32 _cleanAsiData(tx_indi_asi_param_t * param, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bClean = TRUE;
    tx_ds_t * end = buffer + total - 1;
    olint_t index = 0;

    while (index < total)
    {
        /*Check if the ASI is already calculated.*/
        /*Check if the ASI is calculated with the same parameter.*/
        if ((end->td_ptiapAsiParam != NULL) &&
            (ol_memcmp(param, end->td_ptiapAsiParam, sizeof(*param)) == 0))
        {
            JF_LOGGER_DEBUG("no clean");
            bClean = FALSE;
            break;
        }

        end --;
        index ++;
    }

    if (bClean)
        u32Ret = freeIndicatorDataById(buffer, total, TX_INDI_ID_ASI);
    
    return u32Ret;
}

static u32 _calcAsiData(tx_indi_asi_param_t * param, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NOT_IMPLEMENTED;

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 indiAsiGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const tx_indi_asi_param_t * param = pParam;

    ol_sprintf(buf, "%d", param->tiap_nAsiMaDays);

    return u32Ret;
}

u32 indiAsiGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_asi_param_t * param = pParam;

    ol_sscanf(buf, "%d", &param->tiap_nAsiMaDays);

    return u32Ret;
}

u32 indiAsiCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_asi_param_t * param = (tx_indi_asi_param_t *)pParam;

    u32Ret = _cleanAsiData(param, buffer, total);

    /*Do the calculation.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _calcAsiData(param, buffer, total);

    return u32Ret;
}

u32 indiAsiSetDefaultParam(struct tx_indi * indi, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_asi_param_t * param = (tx_indi_asi_param_t *)pParam;

    ol_bzero(param, sizeof(*param));



    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


