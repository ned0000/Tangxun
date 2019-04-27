/**
 *  @file rule_line.c
 *
 *  @brief Implementation file for rules related to line
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
#include "jf_process.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_clieng.h"
#include "jf_mem.h"
#include "jf_jiukun.h"
#include "jf_hashtable.h"

#include "envvar.h"
#include "darule.h"
#include "rule_line.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _isNearPressureLine(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total,
    da_rule_pressure_line_param_t * pdrplp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_day_summary_t * highest = pdrplp->drplp_pddsUpperLeft;
    oldouble_t dbPrice;
    da_day_summary_t * end = buffer + total - 1;

    if (highest->dds_dbClosingPrice < pdrplp->drplp_pddsUpperRight->dds_dbClosingPrice)
        highest = pdrplp->drplp_pddsUpperRight;

    dbPrice = highest->dds_dbClosingPrice * (1 - pdrplp->drplp_dbRatio);
    if (pdrplp->drplp_u8Condition == PRESSURE_LINE_CONDITION_NEAR)
    {
        if (end->dds_dbClosingPrice > dbPrice)
        {
            u32Ret = JF_ERR_NO_ERROR;
            pdrplp->drplp_dbPrice = dbPrice;
        }
    }
    else if (pdrplp->drplp_u8Condition == PRESSURE_LINE_CONDITION_FAR)
    {
        if (end->dds_dbClosingPrice < dbPrice)
        {
            u32Ret = JF_ERR_NO_ERROR;
            pdrplp->drplp_dbPrice = dbPrice;
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 daRulePressureLine(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_rule_pressure_line_param_t * pdrplp = (da_rule_pressure_line_param_t *)pdrp;

    u32Ret = _isNearPressureLine(stockinfo, buffer, total, pdrplp);
        
    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


