/**
 *  @file tradehelper.c
 *
 *  @brief Provide some helper routine for stock trade
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "olbasic.h"
#include "ollimit.h"
#include "jiukun.h"
#include "parsedata.h"
#include "xtime.h"
#include "damodel.h"
#include "trade_persistency.h"
#include "tradehelper.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

u32 setStockFirstTradeDate(const char * strStockPath)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t total = 10;
    da_day_summary_t * buffer = NULL;
    olchar_t strFullname[MAX_PATH_LEN];
    stock_info_t * stockinfo;

    logInfoMsg("set stock first trade date, %s", strStockPath);

    allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);

    stockinfo = getFirstStockInfo();
    while (stockinfo != NULL)
    {
        ol_snprintf(
            strFullname, MAX_PATH_LEN - 1, "%s%c%s",
            strStockPath,
            PATH_SEPARATOR, stockinfo->si_strCode);
        strFullname[MAX_PATH_LEN - 1] = '\0';

        total = 10;

        u32Ret = readTradeDaySummaryFromDate(
            strFullname, NULL, buffer, &total);
        if (u32Ret == OLERR_NO_ERROR)
        {
            ol_strcpy(stockinfo->si_strFirstTradeDate, buffer->dds_strDate);
            logDebugMsg(
                "set stock first trade date, stock: %s, firstdate: %s",
                stockinfo->si_strCode, stockinfo->si_strFirstTradeDate);
        }

        stockinfo = getNextStockInfo(stockinfo);
    }

    freeMemory((void **)&buffer);
    
    return u32Ret;
}

boolean_t isAfterStockFirstTradeDate(
    const stock_info_t * stockinfo, const olchar_t * pstrDate)
{
    boolean_t bRet = TRUE;

    if (ol_strcmp(stockinfo->si_strFirstTradeDate, pstrDate) > 0)
        bRet = FALSE;

    return bRet;
}

boolean_t isHoliday(olint_t year, olint_t mon, olint_t day)
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


/*---------------------------------------------------------------------------*/


