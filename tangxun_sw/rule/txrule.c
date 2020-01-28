/**
 *  @file txrule.c
 *
 *  @brief Basic rule library.
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
#include "jf_hashtable.h"

#include "tx_rule.h"
#include "tx_err.h"

#include "rule_rectangle.h"
#include "rule_limit.h"
#include "rule_st.h"
#include "rule_indi_macd.h"
#include "rule_vol.h"
#include "rule_misc.h"
#include "rule_bottom.h"
#include "rule_price.h"
#include "rule_line.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Maximum rules in hash table.
 */
#define MAX_RULE_IN_HASH_TABLE     (100)

static tx_rule_t ls_trRules[] =
{
/* limit */
    {TX_RULE_ID_HIGH_LIMIT_OF_LAST_DAY, 0, 0, "highLimitOfLastDay", txRuleHighLimitOfLastDay},
    {TX_RULE_ID_LOW_LIMIT_OF_LAST_DAY, 0, 0, "lowLimitOfLastDay", txRuleLowLimitOfLastDay},
    {TX_RULE_ID_MIN_HIGH_LIMIT_DAY, 0, 0, "minHighLimitDay", txRuleMinHighLimitDay},
    {TX_RULE_ID_NO_HIGH_HIGH_LIMIT_DAY, 0, 0, "noHighHighLimitDay", txRuleNoHighHighLimitDay},
/* bottom */
    {TX_RULE_ID_IN_BOTTOM_AREA, 0, 0, "inBottomArea", txRuleInBottomArea},
/* st */
    {TX_RULE_ID_NOT_ST_RELATED, 0, 0, "notStRelated", txRuleNotStRelated},
    {TX_RULE_ID_ST, 0, 0, "St", txRuleSt},
    {TX_RULE_ID_ST_DELISTING, 0, 0, "StDelisting", txRuleStDelisting},
/* misc */
    {TX_RULE_ID_MIN_NUM_OF_DAY_SUMMARY, 0, 0, "minNumOfDaySummary", txRuleMinNumOfDaySummary},
/* rectangle */
    {TX_RULE_ID_RECTANGLE, 0, 0, "rectangle", txRuleRectangle},
/* price */
    {TX_RULE_ID_N_DAYS_UP_IN_M_DAYS, 0, 0, "nDaysUpInMDays", txRuleNDaysUpInMDays},
    {TX_RULE_ID_MIN_RAMPING_DAY, 0, 0, "minRampingDay", txRuleMinRampingDay},
    {TX_RULE_ID_NEED_STOP_LOSS, 0, 0, "needStopLoss", txRuleNeedStopLoss},
    {TX_RULE_ID_PRICE_VOLATILITY, 0, 0, "priceVolatility", txRulePriceVolatility},
/* vol */
    {TX_RULE_ID_MIN_ABNORMAL_VOL_RATIO_DAY, 0, 0, "minAbnormalVolRatioDay", txRuleMinAbnormalVolRatioDay},
/* indi macd */
    {TX_RULE_ID_INDI_MACD_DIFF_UP_BREAK_DEA, 0, 0, "indiMacdDiffUpBreakDea", txRuleIndiMacdDiffUpBreakDea},
    {TX_RULE_ID_INDI_MACD_POSITIVE_DIFF_DEA, 0, 0, "indiMacdPositiveDiffDea", txRuleIndiMacdPositiveDiffDea},
/* line */
    {TX_RULE_ID_PRESSURE_LINE, 0, 0, "pressureLine", txRulePressureLine},
};

static u32 ls_u32NumOfRules = sizeof(ls_trRules) / sizeof(tx_rule_t);

static jf_hashtable_t * ls_pjhDaRuleTable = NULL;

/* --- private routine section ------------------------------------------------------------------ */

static olint_t _fnDaRuleCmpKeys(void * pKey1, void * pKey2)
{
    olchar_t * pstrKey1 = (olchar_t *)pKey1;
    olchar_t * pstrKey2 = (olchar_t *)pKey2;

    return ol_strcmp(pstrKey1, pstrKey2);
}

static void * _fnDaRuleGetKeyFromEntry(void * pEntry)
{
    tx_rule_t * rule = (tx_rule_t *)pEntry;

    return rule->tr_pstrName;
}

static u32 _addDaRule(jf_hashtable_t * pTable, tx_rule_t * ls_trRules, u32 u32NumOfRules)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index;

    for (u32Index = 0; (u32Index < u32NumOfRules) && (u32Ret == JF_ERR_NO_ERROR); u32Index ++)
    {
        u32Ret = jf_hashtable_insertEntry(pTable, &ls_trRules[u32Index]);
    }

    
    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 tx_rule_getAllRules(tx_rule_t ** ppRule)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    *ppRule = ls_trRules;

    return u32Ret;
}

u32 tx_rule_getRuleByName(const olchar_t * name, tx_rule_t ** ppRule)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ls_pjhDaRuleTable == NULL)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_hashtable_getEntry(ls_pjhDaRuleTable, (void *)name, (void **)ppRule);

    return u32Ret;
}

u32 tx_rule_getRuleById(const u16 u16Id, tx_rule_t ** ppRule)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (u16Id >= TX_RULE_ID_MAX)
        u32Ret = TX_ERR_INVALID_RULE_ID;

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppRule = &ls_trRules[u16Id];

    return u32Ret;    
}

u32 tx_rule_getNumOfRules(void)
{
    return ls_u32NumOfRules;
}

tx_rule_t * tx_rule_getFirstRule(void)
{
    return &ls_trRules[0];
}

tx_rule_t * tx_rule_getNextRule(tx_rule_t * pRule)
{
    if (pRule - ls_trRules + 1 < ls_u32NumOfRules)
        return pRule + 1;

    return NULL;
}

u32 tx_rule_init(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hashtable_create_param_t jhcp;

    JF_LOGGER_INFO("init rule");

    ol_bzero(&jhcp, sizeof(jhcp));
    jhcp.jhcp_u32MinSize = MAX_RULE_IN_HASH_TABLE;
    jhcp.jhcp_fnCmpKeys = _fnDaRuleCmpKeys;
    jhcp.jhcp_fnHashKey = jf_hashtable_hashPJW;
    jhcp.jhcp_fnGetKeyFromEntry = _fnDaRuleGetKeyFromEntry;
    
    u32Ret = jf_hashtable_create(&ls_pjhDaRuleTable, &jhcp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _addDaRule(ls_pjhDaRuleTable, ls_trRules, ls_u32NumOfRules);
    }

    return u32Ret;
}

u32 tx_rule_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("fini rule");

    if (ls_pjhDaRuleTable != NULL)
        jf_hashtable_destroy(&ls_pjhDaRuleTable);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


