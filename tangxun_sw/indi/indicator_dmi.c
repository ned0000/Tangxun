/**
 *  @file indicator_dmi.c
 *
 *  @brief Implementation file for DMI indicator.
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

static oldouble_t _calcTrueRange(tx_ds_t * cur, tx_ds_t * prev)
{
    oldouble_t ret = 0, db = 0;

    ret = cur->td_dbHighPrice - cur->td_dbLowPrice;
    db = ABS(cur->td_dbHighPrice - prev->td_dbClosingPrice);
    if (ret < db)
        ret = db;
    db = ABS(cur->td_dbLowPrice - prev->td_dbClosingPrice);
    if (ret < db)
        ret = db;

    return ret;
}

static u32 _newAdxr(tx_ds_t * buffer, olint_t num, tx_indi_dmi_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * end = NULL, * start = NULL, * prev = NULL;
    tx_indi_dmi_t * dmi = NULL;
    olint_t index = 0;
    oldouble_t dbTr = 0, dbHd = 0, dbLd = 0, dbDmp = 0, dbDmm = 0;
    oldouble_t dbt = 0;

    end = buffer + num - 1;
    if (end->td_ptidDmi != NULL)
        return u32Ret;

    u32Ret = jf_jiukun_allocMemory((void **)&end->td_ptidDmi, sizeof(tx_indi_dmi_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(end->td_ptidDmi, sizeof(tx_indi_dmi_t));

        prev = end - 1;
        dmi = end->td_ptidDmi;

        for (index = 0; index < param->tidp_nDmiDays; index ++)
        {
            dbTr += _calcTrueRange(end, prev);

            dbHd = end->td_dbHighPrice - prev->td_dbHighPrice;
            dbLd = prev->td_dbLowPrice - end->td_dbLowPrice;

            if ((dbHd > 0) && (dbHd > dbLd))
                dbDmp += dbHd;
            if ((dbLd > 0) && (dbLd > dbHd))
                dbDmm += dbLd;

            end --;
            prev --;
        }

        dmi->tid_dbPdi = dbDmp * 100 / dbTr;
        dmi->tid_dbMdi = dbDmm * 100 / dbTr;
        dmi->tid_dbDx = ABS(dmi->tid_dbPdi - dmi->tid_dbMdi) * 100 / (dmi->tid_dbPdi + dmi->tid_dbMdi);

        end = buffer + num - 1;
        start = end - param->tidp_nDmiAdxMaDays;

        if (start->td_ptidDmi != NULL)
        {
            start ++;
            dbt = 0;
            for (index = 0; index < param->tidp_nDmiAdxMaDays; index ++)
            {
                dbt += start->td_ptidDmi->tid_dbDx;
                start ++;
            }
            dmi->tid_dbAdx = dbt / param->tidp_nDmiAdxMaDays;
            dmi->tid_dbAdxr = (dmi->tid_dbAdx + (end - param->tidp_nDmiAdxMaDays)->td_ptidDmi->tid_dbAdx) / 2;
        }
    }

    return u32Ret;
}

static u32 _cleanDmiData(tx_indi_dmi_param_t * param, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bClean = TRUE;
    tx_ds_t * end = buffer + total - 1;
    olint_t index = 0;

    while (index < total)
    {
        /*Check if the DMI is already calculated.*/
        /*Check if the DMI is calculated with the same parameter.*/
        if ((end->td_ptidpDmiParam != NULL) &&
            (ol_memcmp(param, end->td_ptidpDmiParam, sizeof(*param)) == 0))
        {
            JF_LOGGER_DEBUG("no clean");
            bClean = FALSE;
            break;
        }

        end --;
        index ++;
    }

    if (bClean)
        u32Ret = freeIndicatorDataById(buffer, total, TX_INDI_ID_DMI);
    
    return u32Ret;
}

static u32 _calcDmiData(tx_indi_dmi_param_t * param, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index = 0;

    for (index = param->tidp_nDmiDays + 1;
         (index <= total) && (u32Ret == JF_ERR_NO_ERROR); index ++)
    {
        u32Ret = _newAdxr(buffer, index, param);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 indiDmiGetStringParam(struct tx_indi * indi, const void * pParam, olchar_t * buf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const tx_indi_dmi_param_t * param = pParam;

    ol_sprintf(
        buf, "%d,%d,%.2f", param->tidp_nDmiDays, param->tidp_nDmiAdxMaDays,
        param->tidp_dbAdxrTrend);

    return u32Ret;
}

u32 indiDmiGetParamFromString(struct tx_indi * indi, void * pParam, const olchar_t * buf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_dmi_param_t * param = pParam;

    ol_sscanf(
        buf, "%d,%d,%lf", &param->tidp_nDmiDays, &param->tidp_nDmiAdxMaDays,
        &param->tidp_dbAdxrTrend);

    return u32Ret;
}

u32 indiDmiCalc(struct tx_indi * indi, void * pParam, tx_ds_t * buffer, olint_t total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_dmi_param_t * param = (tx_indi_dmi_param_t *)pParam;

    if (param->tidp_nDmiDays + param->tidp_nDmiAdxMaDays >= total)
        u32Ret = JF_ERR_NOT_ENOUGH_TRADING_DAY;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _cleanDmiData(param, buffer, total);

    /*Do the calculation.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _calcDmiData(param, buffer, total);

    return u32Ret;
}

u32 indiDmiSetDefaultParam(struct tx_indi * indi, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_indi_dmi_param_t * param = (tx_indi_dmi_param_t *)pParam;

    ol_bzero(param, sizeof(*param));

	param->tidp_nDmiDays = TX_INDI_DEF_DMI_DAYS;
	param->tidp_nDmiAdxMaDays = TX_INDI_DEF_DMI_ADX_MA_DAYS;
    param->tidp_dbAdxrTrend = TX_INDI_DEF_DMI_ADXR_TREND;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


