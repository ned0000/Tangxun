/**
 *  @file damethod.c
 *
 *  @brief Method used by commands
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <math.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <stdlib.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_listhead.h"
#include "jf_clieng.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "stocklist.h"
#include "indicator.h"
#include "statarbitrage.h"
#include "jf_jiukun.h"
#include "envvar.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

oldouble_t getCorrelationWithIndex(stock_info_t * info)
{
    oldouble_t dbret = -9999.99;
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strStocks[32];
    sa_stock_info_t * sastock = NULL;
    olint_t nDaySummary = 60;

    ol_strcpy(strStocks, info->si_strCode);
    ol_strcat(strStocks, ",");
    if (isShStockExchange(info->si_strCode))
        ol_strcat(strStocks, SH_COMPOSITE_INDEX);
    else
        ol_strcat(strStocks, SZ_COMPOSITIONAL_INDEX);

    u32Ret = newSaStockInfo(
		getEnvVar(ENV_VAR_DATA_PATH), strStocks,
        &sastock, nDaySummary * 2);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        dbret = getSaStockInfoCorrelation(sastock, nDaySummary);
    }

    if (sastock != NULL)
        freeSaStockInfo(&sastock);

    return dbret;
}

oldouble_t getCorrelationWithSmeIndex(stock_info_t * info)
{
    oldouble_t dbret = -9999.99;
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strStocks[32];
    sa_stock_info_t * sastock = NULL;
    olint_t nDaySummary = 60;

    ol_strcpy(strStocks, info->si_strCode);
    ol_strcat(strStocks, ",");
    ol_strcat(strStocks, SME_COMPOSITIONAL_INDEX);

    u32Ret = newSaStockInfo(
		getEnvVar(ENV_VAR_DATA_PATH), strStocks,
        &sastock, nDaySummary * 2);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        dbret = getSaStockInfoCorrelation(sastock, nDaySummary);
    }

    if (sastock != NULL)
        freeSaStockInfo(&sastock);

    return dbret;
}

/*---------------------------------------------------------------------------*/


