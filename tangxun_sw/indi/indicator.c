/**
 *  @file indicator.c
 *
 *  @brief Indicator system.
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

#include "indicator_dmi.h"
#include "indicator_macd.h"
#include "indicator_mtm.h"
#include "indicator_kdj.h"
#include "indicator_rsi.h"
#include "indicator_asi.h"
#include "indicator_atr.h"
#include "indicator_obv.h"

/* --- private data/data structure section ------------------------------------------------------ */

static olchar_t * ls_pstrIndi[] =
{
    "N/A",
    "DMI",
    "MACD",
    "MTM",
    "KDJ",
    "RSI",
    "ASI",
    "ATR",
    "OBV",
    "MAX",
};

/* --- private routine section ------------------------------------------------------------------ */

static tx_indi_t ls_tiIndi[] =
{
    {TX_INDI_ID_DMI, TX_INDI_TYPE_TREND, "DMI", "Directional Movement Index",
     indiDmiGetStringParam, indiDmiGetParamFromString, indiDmiCalc, indiDmiSetDefaultParam},
    {TX_INDI_ID_MACD, TX_INDI_TYPE_TREND, "MACD", "Moving Average Convergence / Divergence",
     indiMacdGetStringParam, indiMacdGetParamFromString, indiMacdCalc, indiMacdSetDefaultParam},
    {TX_INDI_ID_MTM, TX_INDI_TYPE_UNKNOWN, "MTM", "Momentum Index",
     indiMtmGetStringParam, indiMtmGetParamFromString, indiMtmCalc, indiMtmSetDefaultParam},
    {TX_INDI_ID_KDJ, TX_INDI_TYPE_ANTI_TREND, "KDJ", "KDJ",
     indiKdjGetStringParam, indiKdjGetParamFromString, indiKdjCalc, indiKdjSetDefaultParam},
    {TX_INDI_ID_RSI, TX_INDI_TYPE_UNKNOWN, "RSI", "Relative Strength Index",
     indiRsiGetStringParam, indiRsiGetParamFromString, indiRsiCalc, indiRsiSetDefaultParam},
    {TX_INDI_ID_ASI, TX_INDI_TYPE_UNKNOWN, "ASI", "Accumulation Swing Index",
     indiAsiGetStringParam, indiAsiGetParamFromString, indiAsiCalc, indiAsiSetDefaultParam},
    {TX_INDI_ID_ATR, TX_INDI_TYPE_UNKNOWN, "ATR", "Average True Range",
     indiAtrGetStringParam, indiAtrGetParamFromString, indiAtrCalc, indiAtrSetDefaultParam},
    {TX_INDI_ID_OBV, TX_INDI_TYPE_UNKNOWN, "OBV", "On Balance Volume",
     indiObvGetStringParam, indiObvGetParamFromString, indiObvCalc, indiObvSetDefaultParam},
};

/* --- public routine section ------------------------------------------------------------------- */

u32 tx_indi_getIndiById(const olint_t id, tx_indi_t ** ppIndi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if ((id == 0) || (id >= TX_INDI_ID_MAX))
        u32Ret = TX_ERR_INDI_NOT_FOUND;

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppIndi = &ls_tiIndi[id - 1];

    return u32Ret;
}

u32 tx_indi_getIndiByName(const olchar_t * name, tx_indi_t ** ppIndi)
{
    u32 u32Ret = TX_ERR_INDI_NOT_FOUND;
    olint_t id = 0;

    for (id = TX_INDI_ID_UNKNOWN + 1; id < TX_INDI_ID_MAX; id ++)
    {
        tx_indi_getIndiById(id, ppIndi);
        if (ol_strcasecmp(name, (*ppIndi)->ti_pstrName) == 0)
        {
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }
    }

    return u32Ret;
}

u32 tx_indi_freeDaySummaryIndi(tx_ds_t *summary, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;

    for (i = 0; i < num; i ++)
    {
        if (summary->td_ptidDmi != NULL)
            jf_jiukun_freeMemory((void **)&summary->td_ptidDmi);
        if (summary->td_ptimMacd != NULL)
            jf_jiukun_freeMemory((void **)&summary->td_ptimMacd);
        if (summary->td_ptimMtm != NULL)
            jf_jiukun_freeMemory((void **)&summary->td_ptimMtm);
        if (summary->td_ptirRsi != NULL)
            jf_jiukun_freeMemory((void **)&summary->td_ptirRsi);
        if (summary->td_ptikKdj != NULL)
            jf_jiukun_freeMemory((void **)&summary->td_ptikKdj);
        if (summary->td_ptiaAsi != NULL)
            jf_jiukun_freeMemory((void **)&summary->td_ptiaAsi);
        if (summary->td_ptiaAtr != NULL)
            jf_jiukun_freeMemory((void **)&summary->td_ptiaAtr);
        if (summary->td_ptioObv != NULL)
            jf_jiukun_freeMemory((void **)&summary->td_ptioObv);

        summary ++;
    }

    return u32Ret;
}

olchar_t * tx_indi_getStringKcp(olint_t nKcp)
{
    if (nKcp == TX_INDI_KCP_TREND)
        return "KCP trend";
    else if (nKcp == TX_INDI_KCP_ANTI_TREND)
        return "KCP anti trend";
    else
        return "Unknown";
}

olchar_t * tx_indi_getAbbrevStringKcp(olint_t nKcp)
{
    if (nKcp == TX_INDI_KCP_TREND)
        return "T";
    else if (nKcp == TX_INDI_KCP_ANTI_TREND)
        return "AT";
    else
        return "N/A";
}

olchar_t * tx_indi_getStringIndiName(olint_t nIndi)
{
    if ((nIndi == 0) || (nIndi >= TX_INDI_ID_MAX))
        return "Unknown";

    return ls_pstrIndi[nIndi];
}

olchar_t * tx_indi_getStringIndiType(olint_t type)
{
    if (type == TX_INDI_TYPE_TREND)
        return "Trend";
    else if (type == TX_INDI_TYPE_ANTI_TREND)
        return "Anti trend";
    else
        return "unknown";
}

/*------------------------------------------------------------------------------------------------*/


