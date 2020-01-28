/**
 *  @file tradehelper.c
 *
 *  @brief Provide some helper routine for stock trade.
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
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_jiukun.h"
#include "jf_time.h"
#include "jf_date.h"

#include "tx_daysummary.h"
#include "tx_persistency.h"
#include "tx_trade.h"
#include "tx_daysummary.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

static boolean_t _isHoliday(olint_t year, olint_t mon, olint_t day)
{
    boolean_t bRet = FALSE;

    if ((mon == 1) && (day == 1))
        return TRUE;

    if ((mon == 5) && (day == 1))
        return TRUE;
    
    if ((mon == 10) &&
        ((day >= 1) && (day <= 7)))
        return TRUE;
    
    return bRet;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 tx_trade_setStockFirstTradeDate(const char * strStockPath)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = 10;
    tx_ds_t * buffer = NULL;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    tx_stock_info_t * stockinfo;

    jf_logger_logInfoMsg("set stock first trade date, %s", strStockPath);

    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);

    stockinfo = tx_stock_getFirstStockInfo();
    while (stockinfo != NULL)
    {
        ol_snprintf(
            strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
            strStockPath, PATH_SEPARATOR, stockinfo->tsi_strCode);
        strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';

        total = 10;

        u32Ret = tx_ds_readDsFromDate(strFullname, NULL, buffer, &total);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_strcpy(stockinfo->tsi_strFirstTradeDate, buffer->td_strDate);
            jf_logger_logDebugMsg(
                "set stock first trade date, stock: %s, firstdate: %s",
                stockinfo->tsi_strCode, stockinfo->tsi_strFirstTradeDate);
        }

        stockinfo = tx_stock_getNextStockInfo(stockinfo);
    }

    jf_jiukun_freeMemory((void **)&buffer);
    
    return u32Ret;
}

boolean_t tx_trade_isAfterStockFirstTradeDate(
    const tx_stock_info_t * stockinfo, const olchar_t * pstrDate)
{
    boolean_t bRet = TRUE;

    if (ol_strcmp(stockinfo->tsi_strFirstTradeDate, pstrDate) > 0)
        bRet = FALSE;

    return bRet;
}

boolean_t tx_trade_isHoliday(olint_t days)
{
    boolean_t bRet = FALSE;
    olint_t year, month, day;

    jf_date_convertDaysFrom1970ToDate(days, &year, &month, &day);
    if (jf_date_isWeekendForDate(year, month, day) || _isHoliday(year, month, day))
        bRet = TRUE;
    
    return bRet;
}

u32 tx_trade_getNextTradingDate(const olchar_t * pstrCurr, olchar_t * pstrNext)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t year, month, day;
    olint_t days, dw;

    jf_date_getDate2FromString(pstrCurr, &year, &month, &day);
    days = jf_date_convertDateToDaysFrom1970(year, month, day);
    dw = jf_date_getDayOfWeekFromDate(year, month, day);
    if (dw == 5)
    {
        jf_date_convertDaysFrom1970ToDate(days + 3, &year, &month, &day);
        jf_date_getStringDate2(pstrNext, year, month, day);
    }
    else if (dw == 6)
    {
        jf_date_convertDaysFrom1970ToDate(days + 2, &year, &month, &day);
        jf_date_getStringDate2(pstrNext, year, month, day);
    }
    else
    {
        jf_date_convertDaysFrom1970ToDate(days + 1, &year, &month, &day);
        jf_date_getStringDate2(pstrNext, year, month, day);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


