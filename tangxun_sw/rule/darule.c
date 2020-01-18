/**
 *  @file darule.c
 *
 *  @brief Base rule library.
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

static tx_rule_t ls_trRules[] =
{
/* limite */
    {"highLimitOfLastDay", daRuleHighLimitOfLastDay},
    {"lowLimitOfLastDay", daRuleLowLimitOfLastDay},
    {"minHighLimitDay", daRuleMinHighLimitDay},
    {"noHighHighLimitDay", daRuleNoHighHighLimitDay},
/* bottom */
    {"inBottomArea", daRuleInBottomArea},
/* st */
    {"notStRelated", daRuleNotStRelated},
    {"St", daRuleSt},
    {"StDelisting", daRuleStDelisting},
/* misc */
    {"minNumOfDaySummary", daRuleMinNumOfDaySummary},
/* rectangle */
    {"rectangle", daRuleRectangle},
/* price */
    {"nDaysUpInMDays", daRuleNDaysUpInMDays},
    {"minRampingDay", daRuleMinRampingDay},
    {"needStopLoss", daRuleNeedStopLoss},
    {"priceVolatility", daRulePriceVolatility},
/* vol */
    {"minAbnormalVolRatioDay", daRuleMinAbnormalVolRatioDay},
/* indi macd */
    {"indiMacdDiffUpBreakDea", daRuleIndiMacdDiffUpBreakDea},
/* line */
    {"pressureLine", daRulePressureLine},
};

static u32 ls_u32NumOfRules = sizeof(ls_trRules) / sizeof(tx_rule_t);

#define MAX_DA_RULE_IN_TABLE     (100)

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

u32 tx_rule_getRule(char * name, tx_rule_t ** ppRule)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ls_pjhDaRuleTable == NULL)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_hashtable_getEntry(ls_pjhDaRuleTable, name, (void **)ppRule);

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

    ol_bzero(&jhcp, sizeof(jhcp));
    jhcp.jhcp_u32MinSize = MAX_DA_RULE_IN_TABLE;
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

    if (ls_pjhDaRuleTable != NULL)
        jf_hashtable_destroy(&ls_pjhDaRuleTable);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


