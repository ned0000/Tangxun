/**
 *  @file stock.c
 *
 *  @brief Implementation file for stock routine.
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

#include "common.h"
#include "stocklist.h"
#include "stockindu.h"
#include "stockindex.h"

/* --- private data/data structure section ------------------------------------------------------ */



/* --- private routine section ------------------------------------------------------------------ */



/* --- public routine section ------------------------------------------------------------------- */

u32 tx_stock_init(tx_stock_init_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("init stock");

    u32Ret = initStockList(param);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = initStockIndu(param);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = initStockIndex(param);

    if (u32Ret != JF_ERR_NO_ERROR)
	{
		JF_LOGGER_ERR(u32Ret, "Failed to initiate stock");
        tx_stock_fini();
	}

    return u32Ret;
}

u32 tx_stock_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("fini stock");

    finiStockIndex();

    finiStockIndu();

    finiStockList();

    return u32Ret;
}

u32 tx_stock_getStockInfo(const olchar_t * name, tx_stock_info_t ** info)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    *info = NULL;
    u32Ret = findStockInfo(name, info);
    if (u32Ret != JF_ERR_NO_ERROR)
        u32Ret = findStockIndex(name, info);

    return u32Ret;
}


/*------------------------------------------------------------------------------------------------*/


