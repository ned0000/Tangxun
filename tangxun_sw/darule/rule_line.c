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

static u32 _isInPressureArea(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total,
    da_rule_near_pressure_line_param_t * pdrnplp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_day_summary_t * highest = pdrnplp->drnplp_pddsUpperLeft;
    oldouble_t dbPrice;
    da_day_summary_t * end = buffer + total - 1;

    if (highest->dds_dbClosingPrice < pdrnplp->drnplp_pddsUpperRight->dds_dbClosingPrice)
        highest = pdrnplp->drnplp_pddsUpperRight;

    dbPrice = highest->dds_dbClosingPrice * (1 - pdrnplp->drnplp_dbRatio);
    if (end->dds_dbHighPrice > dbPrice)
    {
        u32Ret = JF_ERR_NO_ERROR;
        pdrnplp->drnplp_dbPrice = dbPrice;
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleNearPressureLine(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_rule_near_pressure_line_param_t * pdrnplp = (da_rule_near_pressure_line_param_t *)pdrp;

    u32Ret = _isInPressureArea(stockinfo, buffer, total, pdrnplp);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


