/**
 *  @file rule_indi_macd.c
 *
 *  @brief Implementation file for rules related to indicator MACD.
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
#include "jf_limit.h"
#include "jf_jiukun.h"

#include "tx_rule.h"
#include "rule_indi_macd.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */



/* --- public routine section ------------------------------------------------------------------- */

u32 txRuleIndiMacdDiffUpBreakDea(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_rule_indi_macd_diff_up_break_dea_param_t * param = pParam;
    tx_indi_t * indi = NULL;

    JF_LOGGER_INFO("stock, %s", stockinfo->tsi_strCode);

    if (total < param->trimdubdp_nMacdLongDays)
        return u32Ret;

    /*Calculate diff and dea.*/
    u32Ret = tx_indi_getIndiById(TX_INDI_ID_MACD, &indi);
    if (u32Ret == JF_ERR_NO_ERROR)
        ;

    return u32Ret;
}

u32 txRuleIndiMacdPositiveDiffDea(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, int total, void * pParam)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    tx_rule_indi_macd_positive_diff_dea_param_t * param = pParam;

    JF_LOGGER_INFO("stock, %s", stockinfo->tsi_strCode);

    if (total < param->trimpddp_nMacdLongDays)
        return u32Ret;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


