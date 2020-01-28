/**
 *  @file backtesting.c
 *
 *  @brief Backtesting module.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_process.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_jiukun.h"
#include "jf_file.h"
#include "jf_filestream.h"
#include "jf_dir.h"
#include "jf_time.h"
#include "jf_date.h"

#include "tx_stock.h"
#include "tx_model.h"
#include "tx_persistency.h"
#include "tx_backtesting.h"
#include "tx_trade.h"

#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bBacktestingInitialized = FALSE;

/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 tx_backtesting_init(tx_backtesting_init_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ls_bBacktestingInitialized)
        return u32Ret;

    u32Ret = tx_trade_setStockFirstTradeDate(param->tbip_pstrStockPath);

    if (u32Ret == JF_ERR_NO_ERROR)
        ls_bBacktestingInitialized = TRUE;
    
    return u32Ret;
}

u32 tx_backtesting_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


