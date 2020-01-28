/**
 *  @file stockindex.c
 *
 *  @brief Implementation file for stock index.
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
#include "jf_listhead.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_time.h"
#include "jf_jiukun.h"

#include "tx_err.h"

#include "common.h"
#include "stockindex.h"

/* --- private data/data structure section ------------------------------------------------------ */

static tx_stock_info_t ls_tsiStockIndex[] =
{
    {TX_STOCK_SH_COMPOSITE_INDEX},
    {TX_STOCK_SZ_COMPOSITIONAL_INDEX},
    {TX_STOCK_SME_COMPOSITIONAL_INDEX}
};

static u32 ls_u32NumOfStockIndex = sizeof(ls_tsiStockIndex) / sizeof(tx_stock_info_t);

/* --- private routine section ------------------------------------------------------------------ */

static u32 _findStockIndex(
    tx_stock_info_t * ptsi, u32 num, const olchar_t * name, tx_stock_info_t ** info)
{
    u32 u32Ret = TX_ERR_STOCK_NOT_FOUND;
    olint_t i = 0, nRet = 0;
    
    for (i = 0; i < num; i ++)
    {
        nRet = ol_strncmp(name, ptsi[i].tsi_strCode, ol_strlen(ptsi[i].tsi_strCode));
        if (nRet == 0)
        {
            *info = &ptsi[i];
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 findStockIndex(const olchar_t * name, tx_stock_info_t ** info)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _findStockIndex(ls_tsiStockIndex, ls_u32NumOfStockIndex, name, info);

    return u32Ret;
}

u32 initStockIndex(tx_stock_init_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("init index");

    if (u32Ret != JF_ERR_NO_ERROR)
	{
		JF_LOGGER_ERR(u32Ret, "failed to initiate stock index");
        finiStockIndex();
	}

    return u32Ret;
}

u32 finiStockIndex(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

tx_stock_info_t * tx_stock_getFirstStockIndex(void)
{
    return &ls_tsiStockIndex[0];
}

tx_stock_info_t * tx_stock_getNextStockIndex(tx_stock_info_t * info)
{
    tx_stock_info_t * ptsi = NULL;

    if (info < &ls_tsiStockIndex[0] + ls_u32NumOfStockIndex - 1)
        ptsi = info + 1;

    return ptsi;
}

tx_stock_info_t * tx_stock_getStockIndex(olchar_t * name)
{
    olint_t i;

    for (i = 0; i < ls_u32NumOfStockIndex; i ++)
    {
        if (ol_strncmp(ls_tsiStockIndex[i].tsi_strCode, name, 8) == 0)
            return &ls_tsiStockIndex[i];
    }

    return NULL;
}

boolean_t tx_stock_isStockIndex(olchar_t * code)
{
    boolean_t bRet = FALSE;
    tx_stock_info_t * ptsi = &ls_tsiStockIndex[0];
    olint_t i;

    for (i = 0; i < ls_u32NumOfStockIndex; i ++)
    {
        if (ol_strncmp(code, ptsi->tsi_strCode, ol_strlen(ptsi->tsi_strCode)) == 0)
            return TRUE;

        ptsi ++;
    }

    return bRet;
}

/*------------------------------------------------------------------------------------------------*/


