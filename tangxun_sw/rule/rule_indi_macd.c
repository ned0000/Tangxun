/**
 *  @file rule_indi_macd.c
 *
 *  @brief Implementation file for rules related to indicator MACD
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
#include "rule_indi_macd.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */



/* --- public routine section ------------------------------------------------------------------- */

u32 daRuleIndiMacdDiffUpBreakDea(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp)
{
    u32 u32Ret = JF_ERR_NOT_MATCH;
    da_rule_indi_macd_diff_up_break_dea_param_t * param =
        (da_rule_indi_macd_diff_up_break_dea_param_t *)pdrp;

    jf_logger_logInfoMsg("rule indi macd, %s", stockinfo->si_strCode);

    if (total < param->drimdubdp_nMacdLongDays)
        return u32Ret;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


