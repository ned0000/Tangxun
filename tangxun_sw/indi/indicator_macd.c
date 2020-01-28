/**
 *  @file indicator_macd.c
 *
 *  @brief Implementation file for MACD indicator.
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

static u32 _cleanMacdData(tx_indi_macd_param_t * param, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bClean = TRUE;
    tx_ds_t * end = buffer + total - 1;
    olint_t index = 0;

    while (index < total)
    {
        /*Check if the MACD is already calculated.*/
        /*Check if the MACD is calculated with the same parameter.*/
        if ((end->td_ptimpMacdParam != NULL) &&
            (ol_memcmp(param, end->td_ptimpMacdParam, sizeof(*param)) == 0))
        {
            JF_LOGGER_DEBUG("no clean");
            bClean = FALSE;
            break;
        }

        end --;
        index ++;
    }

    if (bClean)
        u32Ret = freeIndicatorDataById(buffer, total, TX_INDI_ID_MACD);
    
    return u32Ret;
}

static u32 _calcMacdData(tx_indi_macd_param_t * param, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * start = buffer, * prev = NULL;
    tx_ds_t * end = buffer + total - 1;
    olint_t index = 0;
    tx_indi_macd_t * ptim = NULL;

    while (index < total)
    {
        /*If the MACD data is NULL, start from here.*/
        if (start->td_ptimMacd == NULL)
            break;
    }

    while (start <= end)
    {
        u32Ret = jf_jiukun_allocMemory((void **)&start->td_ptimMacd, sizeof(tx_indi_macd_t));
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_bzero(start->td_ptimMacd, sizeof(tx_indi_macd_t));

            ptim = start->td_ptimMacd;
            if (start == buffer)
            {
                /*Set the MACD data in The first day summary.*/
                ptim->tim_dbShortEma = start->td_dbClosingPrice;
                ptim->tim_dbLongEma = start->td_dbClosingPrice;
            }
            else
            {
                prev = start - 1;

                ptim->tim_dbShortEma = 2 * start->td_dbClosingPrice + \
                    (param->timp_nMacdShortDays - 1) * prev->td_ptimMacd->tim_dbShortEma;
                ptim->tim_dbShortEma /= param->timp_nMacdShortDays + 1;

                ptim->tim_dbLongEma = 2 * start->td_dbClosingPrice + \
                    (param->timp_nMacdLongDays - 1) * prev->td_ptimMacd->tim_dbLongEma;
                ptim->tim_dbLongEma /= param->timp_nMacdLongDays + 1;
            }
            ptim->tim_dbDiff = ptim->tim_dbShortEma - ptim->tim_dbLongEma;
            if (start == buffer)
            {
                ptim->tim_dbDea = ptim->tim_dbDiff;
            }
            else
            {
                ptim->tim_dbDea = 2 * ptim->tim_dbDiff + \
                    (param->timp_nMacdMDays - 1) * prev->td_ptimMacd->tim_dbDea;
                ptim->tim_dbDea /= param->timp_nMacdMDays + 1;
            }
            ptim->tim_dbMacd = 2 * (ptim->tim_dbDiff - ptim->tim_dbDea);
        }
        start ++;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_jiukun_cloneMemory(
            (void **)&end->td_ptimpMacdParam, (u8 *)param, sizeof(*param));

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 indiMacdGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const tx_indi_macd_param_t * param = pParam;

    ol_sprintf(
        buf, "%d,%d,%d", param->timp_nMacdShortDays, param->timp_nMacdLongDays,
        param->timp_nMacdMDays);

    return u32Ret;
}

u32 indiMacdGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_macd_param_t * param = pParam;

    ol_sscanf(
        buf, "%d,%d,%d", &param->timp_nMacdShortDays, &param->timp_nMacdLongDays,
        &param->timp_nMacdMDays);

    return u32Ret;
}

u32 indiMacdCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_macd_param_t * param = (tx_indi_macd_param_t *)pParam;

    u32Ret = _cleanMacdData(param, buffer, total);

    /*Do the calculation.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _calcMacdData(param, buffer, total);

    return u32Ret;
}

u32 indiMacdSetDefaultParam(struct tx_indi * indi, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_macd_param_t * param = (tx_indi_macd_param_t *)pParam;

    ol_bzero(param, sizeof(*param));

	param->timp_nMacdShortDays = TX_INDI_DEF_MACD_SHORT_DAYS;
	param->timp_nMacdLongDays = TX_INDI_DEF_MACD_LONG_DAYS;
    param->timp_nMacdMDays = TX_INDI_DEF_MACD_M_DAYS;


    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


