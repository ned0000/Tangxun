/**
 *  @file rule_bottom.c
 *
 *  @brief Implementation file for rules related to bottom.
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

#include "tx_env.h"
#include "tx_rule.h"
#include "rule_bottom.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static boolean_t _isInBottomArea(tx_ds_t * buffer, int total, double maxuprate)
{
    boolean_t bRet = TRUE;
    tx_ds_t * low;
    double dbInc;
    tx_ds_t * cur = buffer + total - 1;

    if (total == 0)
        return bRet;

    low = tx_ds_getDsWithLowestClosingPrice(buffer, total);

    dbInc = (cur->td_dbClosingPrice - low->td_dbClosingPrice) * 100 / low->td_dbClosingPrice;
    if (dbInc > maxuprate)
        bRet = FALSE;

    return bRet;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 txRuleInBottomArea(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_rule_in_bottom_area_param_t * param = pParam;

    if (! _isInBottomArea(buffer, total, param->tribap_u8Threshold))
        u32Ret = JF_ERR_NOT_MATCH;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


