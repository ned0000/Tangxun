/**
 *  @file indicator.c
 *
 *  @brief Indicator system
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
#include "jf_clieng.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "indicator.h"
#include "datastat.h"
#include "jf_clieng.h"
#include "stocklist.h"
#include "jf_matrix.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */

static olchar_t * ls_pstrIndicator[] =
{
    "N/A",
    "DMI",
    "MACD",
    "MTM",
    "KDJ",
    "RSI",
    "ASI",
    "ATR",
    "OBV",
    "MAX",
};

/* --- private routine section ------------------------------------------------------------------ */

static oldouble_t _calcTrueRange(
    da_day_summary_t * cur, da_day_summary_t * prev)
{
    oldouble_t ret, db;

    ret = cur->dds_dbHighPrice - cur->dds_dbLowPrice;
    db = cur->dds_dbHighPrice - prev->dds_dbClosingPrice;
    if (ret < db)
        ret = db;
    db = prev->dds_dbClosingPrice - cur->dds_dbLowPrice;
    if (ret < db)
        ret = db;

    return ret;
}

static void _calcDirectMove(
    da_day_summary_t * cur, da_day_summary_t * prev,
    oldouble_t * pdbp, oldouble_t * pdbn)
{
    oldouble_t dbp, dbn;

    dbp = cur->dds_dbHighPrice - prev->dds_dbHighPrice;
    dbn = prev->dds_dbLowPrice - cur->dds_dbLowPrice;
    if (dbp < 0)
        dbp = 0;
    if (dbn < 0)
        dbn = 0;
    if ((dbp > 0) && (dbn > 0))
    {
        if (dbp > dbn)
            dbn = 0;
        else
            dbp = 0;
    }

    *pdbp = dbp;
    *pdbn = dbn;
}

static void _newAdxr(
    da_day_summary_t * buffer, olint_t num,
    olint_t nAdxrDays, olint_t nAdxrMaDays)
{
    da_day_summary_t * end, * start;
    da_adxr_t * adxr;
    olint_t i;
    oldouble_t dbt, dbp, dbn;
    oldouble_t db1, db2;
    oldouble_t dbTr14, dbDmp14, dbDmm14, dbPdi, dbMdi;

    end = buffer + num - 1;
    if (end->dds_pdaAdxr != NULL)
        return;

    jf_jiukun_allocMemory(
        (void **)&end->dds_pdaAdxr, sizeof(da_adxr_t));
    ol_bzero(end->dds_pdaAdxr, sizeof(da_adxr_t));

    adxr = end->dds_pdaAdxr;
    dbt = dbp = dbn = 0;
    for (i = 0; i < nAdxrDays; i ++)
    {
        dbt += _calcTrueRange(end, end - 1);
        _calcDirectMove(end, end - 1, &db1, &db2);
        dbp += db1;
        dbn += db2;
        end --;
    }
    dbTr14 = dbt;
    dbDmp14 = dbp;
    dbDmm14 = dbn;
    dbPdi = dbDmp14 * 100 / dbTr14;
    dbMdi = dbDmm14 * 100 / dbTr14;
    adxr->da_dbDx =
        ABS((dbPdi - dbMdi) * 100 / (dbPdi + dbMdi));

    end = buffer + num - 1;
    start = end - nAdxrMaDays;
    if (start->dds_pdaAdxr != NULL)
    {
        start ++;
        dbt = 0;
        for (i = 0; i < nAdxrMaDays; i ++)
        {
            dbt += start->dds_pdaAdxr->da_dbDx;
            start ++;
        }
        adxr->da_dbAdx = dbt / nAdxrMaDays;
        adxr->da_dbAdxr =
            (adxr->da_dbAdx + (end - nAdxrMaDays)->dds_pdaAdxr->da_dbAdx) / 2;
    }
}

static oldouble_t _getAdxrTrendRatio(da_day_summary_t * first, olint_t num)
{
    da_day_summary_t * end = first + num - 1;
    da_day_summary_t * high;
    oldouble_t ratio;
    da_day_summary_t * inf[100];
    olint_t numofinf = 100;
    olint_t i;

    getDaySummaryInflexionPoint(first, num, inf, &numofinf);

    high = inf[0];
    for (i = 1; i < numofinf; i ++)
    {
        if (high->dds_dbClosingPrice < inf[i]->dds_dbClosingPrice)
            high = inf[i];
    }
#define TREND_ADXR_RATIO 70  //70
#define ANTI_TREND_ADXR_RATIO 5 //25

    if (high == inf[numofinf - 1])
    {
        end->dds_pdaAdxr->da_nKcp = KCP_TREND;
        ratio = TREND_ADXR_RATIO;
    }
    else if (high == inf[numofinf - 2])
    {
        if (numofinf >= 3)
        {
            if (inf[numofinf - 1]->dds_dbClosingPrice > inf[numofinf - 3]->dds_dbClosingPrice)
            {
                end->dds_pdaAdxr->da_nKcp = KCP_TREND;
                ratio = TREND_ADXR_RATIO;
            }
            else
            {
                end->dds_pdaAdxr->da_nKcp = KCP_ANTI_TREND;
                ratio = ANTI_TREND_ADXR_RATIO;
            }
        }
        else
        {
            end->dds_pdaAdxr->da_nKcp = KCP_ANTI_TREND;
            ratio = ANTI_TREND_ADXR_RATIO;
        }
    }
    else
    {
        if (numofinf >= 4)
        {
            if ((high == inf[numofinf - 3]) &&
                (inf[numofinf - 1]->dds_dbClosingPrice > inf[numofinf - 2]->dds_dbClosingPrice) &&
                (inf[numofinf - 2]->dds_dbClosingPrice > inf[numofinf - 4]->dds_dbClosingPrice))
            {
                end->dds_pdaAdxr->da_nKcp = KCP_TREND;
                ratio = TREND_ADXR_RATIO;
            }
            else
            {
                end->dds_pdaAdxr->da_nKcp = KCP_ANTI_TREND;
                ratio = ANTI_TREND_ADXR_RATIO;
            }
        }
        else
        {
            end->dds_pdaAdxr->da_nKcp = KCP_ANTI_TREND;
            ratio = ANTI_TREND_ADXR_RATIO;
        }
    }

    return ratio;
}

static olint_t _compareAdxr(const void * a, const void * b)
{
    da_day_summary_t * ra, * rb;
    olint_t nRet;

    ra = *((da_day_summary_t **)a);
    rb = *((da_day_summary_t **)b);

    if (ra->dds_pdaAdxr->da_dbAdxr > rb->dds_pdaAdxr->da_dbAdxr)
        nRet = 1;
    else
        nRet = -1;

    return nRet;
}

static void _getAdxrTrend(da_day_summary_t * first, olint_t num)
{
    da_day_summary_t * end;
    olint_t i, m;
    oldouble_t estiratio;
    da_day_summary_t ** sort = NULL;

    end = first + num - 1;

    estiratio = _getAdxrTrendRatio(first, num);

    jf_jiukun_allocMemory((void **)&sort, sizeof(da_day_summary_t *) * num);
    for (i = 0; i <= num; i ++)
    {
        sort[i] = &first[i];
    }

    qsort(sort, num, sizeof(da_day_summary_t *), _compareAdxr);

    m = (100 - estiratio) * num / 100;
    end->dds_pdaAdxr->da_dbAdxrTrend = sort[m - 1]->dds_pdaAdxr->da_dbAdxr;

    jf_jiukun_freeMemory((void **)&sort);
}

static u32 _caluAdxr(
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t m, start;
    da_day_summary_t * first, * end;
    olint_t nAdxrDays, nAdxrMaDays;

    if (num < TOTAL_OPTIMIZE_INDI_DAY_SUMMARY)
        return JF_ERR_INVALID_INPUT;

    nAdxrDays = 14;
    nAdxrMaDays = 6;
    start = num - OPTIMIZE_INDI_DAY_SUMMARY + 1;

    for (m = start - OPTIMIZE_INDI_DAY_SUMMARY - (nAdxrMaDays + 1) * 2; m <= num; m ++)
    {
        _newAdxr(buffer, m, nAdxrDays, nAdxrMaDays);
    }

    end = buffer + num - 1;
    first = end - OPTIMIZE_INDI_DAY_SUMMARY + 1;
    _getAdxrTrend(first, OPTIMIZE_INDI_DAY_SUMMARY);

    return u32Ret;
}

static void _dmiGetStringIndicatorParam(
    struct da_indicator_desc * indi, const da_indicator_param_t * pdip, olchar_t * buf)
{
    da_dmi_param_t * param = (da_dmi_param_t *)&pdip->dip_ddpDmi;

    ol_sprintf(
        buf, "%d,%d,%.2f",
        param->ddp_nDmiDays, param->ddp_nDmiAdxMaDays,
        param->ddp_dbAdxrTrend);
}

static void _dmiGetIndicatorParamFromString(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip, const olchar_t * buf)
{
    da_dmi_param_t * param = &pdip->dip_ddpDmi;

    sscanf(
        buf, "%d,%d,%lf",
        &param->ddp_nDmiDays, &param->ddp_nDmiAdxMaDays,
        &param->ddp_dbAdxrTrend);
}

static void _macdGetStringIndicatorParam(
    struct da_indicator_desc * indi, const da_indicator_param_t * pdip, olchar_t * buf)
{
    da_macd_param_t * param = (da_macd_param_t *)&pdip->dip_dmpMacd;

    ol_sprintf(
        buf, "%d,%d,%d",
        param->dmp_nMacdShortDays, param->dmp_nMacdLongDays,
        param->dmp_nMacdMDays);
}

static void _macdGetIndicatorParamFromString(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip, const olchar_t * buf)
{
    da_macd_param_t * param = &pdip->dip_dmpMacd;

    sscanf(
        buf, "%d,%d,%d",
        &param->dmp_nMacdShortDays, &param->dmp_nMacdLongDays,
        &param->dmp_nMacdMDays);
}

static void _mtmGetStringIndicatorParam(
    struct da_indicator_desc * indi, const da_indicator_param_t * pdip, olchar_t * buf)
{
    da_mtm_param_t * param = (da_mtm_param_t *)&pdip->dip_dmpMtm;

    ol_sprintf(
        buf, "%d,%d",
        param->dmp_nMtmDays, param->dmp_nMtmMaDays);
}

static void _mtmGetIndicatorParamFromString(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip, const olchar_t * buf)
{
    da_mtm_param_t * param = &pdip->dip_dmpMtm;

    sscanf(
        buf, "%d,%d",
        &param->dmp_nMtmDays, &param->dmp_nMtmMaDays);
}

static void _rsiGetStringIndicatorParam(
    struct da_indicator_desc * indi, const da_indicator_param_t * pdip, olchar_t * buf)
{
    da_rsi_param_t * param = (da_rsi_param_t *)&pdip->dip_drpRsi;

    ol_sprintf(
        buf, "%d,%d,%d,%.1f,%.1f",
        param->drp_nRsi1Days, param->drp_nRsi2Days,
        param->drp_nRsi3Days, param->drp_dbMaxOpening1, param->drp_dbMinCloseout1);
}

static void _rsiGetIndicatorParamFromString(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip, const olchar_t * buf)
{
    da_rsi_param_t * param = &pdip->dip_drpRsi;

    sscanf(
        buf, "%d,%d,%d,%lf,%lf",
        &param->drp_nRsi1Days, &param->drp_nRsi2Days,
        &param->drp_nRsi3Days, &param->drp_dbMaxOpening1, &param->drp_dbMinCloseout1);
}

static void _kdjGetStringIndicatorParam(
    struct da_indicator_desc * indi, const da_indicator_param_t * pdip, olchar_t * buf)
{
    da_kdj_param_t * param = (da_kdj_param_t *)&pdip->dip_dkpKdj;

    ol_sprintf(
        buf, "%d,%d,%d,%.1f,%.1f",
        param->dkp_nKdjNDays, param->dkp_nKdjM1Days,
        param->dkp_nKdjM2Days, param->dkp_dbMaxOpeningD,
        param->dkp_dbMinCloseoutJ);
}

static void _kdjGetIndicatorParamFromString(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip, const olchar_t * buf)
{
    da_kdj_param_t * param = &pdip->dip_dkpKdj;

    sscanf(
        buf, "%d,%d,%d,%lf,%lf",
        &param->dkp_nKdjNDays, &param->dkp_nKdjM1Days,
        &param->dkp_nKdjM2Days, &param->dkp_dbMaxOpeningD,
        &param->dkp_dbMinCloseoutJ);
}

static void _asiGetStringIndicatorParam(
    struct da_indicator_desc * indi, const da_indicator_param_t * pdip, olchar_t * buf)
{
    da_asi_param_t * param = (da_asi_param_t *)&pdip->dip_dapAsi;

    ol_sprintf(
        buf, "%d",
        param->dap_nAsiMaDays);
}

static void _asiGetIndicatorParamFromString(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip, const olchar_t * buf)
{
    da_asi_param_t * param = &pdip->dip_dapAsi;

    sscanf(
        buf, "%d",
        &param->dap_nAsiMaDays);
}

static void _atrGetStringIndicatorParam(
    struct da_indicator_desc * indi, const da_indicator_param_t * pdip, olchar_t * buf)
{
    da_atr_param_t * param = (da_atr_param_t *)&pdip->dip_dapAtr;

    ol_sprintf(
        buf, "%d,%d",
        param->dap_nAtrDays, param->dap_nAtrMaDays);
}

static void _atrGetIndicatorParamFromString(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip, const olchar_t * buf)
{
    da_atr_param_t * param = &pdip->dip_dapAtr;

    sscanf(
        buf, "%d,%d",
        &param->dap_nAtrDays, &param->dap_nAtrMaDays);
}

static void _obvGetStringIndicatorParam(
    struct da_indicator_desc * indi, const da_indicator_param_t * pdip, olchar_t * buf)
{
    da_obv_param_t * param = (da_obv_param_t *)&pdip->dip_dopObv;

    ol_sprintf(
        buf, "%d",
        param->dop_nObvDays);
}

static void _obvGetIndicatorParamFromString(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip, const olchar_t * buf)
{
    da_obv_param_t * param = &pdip->dip_dopObv;

    sscanf(
        buf, "%d",
        &param->dop_nObvDays);
}

static da_indicator_desc_t ls_didIndicatorDesc[] =
{
    {STOCK_INDICATOR_DMI, INDI_TYPE_TREND, "DMI", "Directional Movement Index",
     _dmiGetStringIndicatorParam, _dmiGetIndicatorParamFromString},
    {STOCK_INDICATOR_MACD, INDI_TYPE_TREND, "MACD", "Moving Average Convergence / Divergence",
     _macdGetStringIndicatorParam, _macdGetIndicatorParamFromString},
    {STOCK_INDICATOR_MTM, INDI_TYPE_UNKNOWN, "MTM", "Momentum Index",
     _mtmGetStringIndicatorParam, _mtmGetIndicatorParamFromString},
    {STOCK_INDICATOR_KDJ, INDI_TYPE_ANTI_TREND, "KDJ", "KDJ",
     _kdjGetStringIndicatorParam, _kdjGetIndicatorParamFromString},
    {STOCK_INDICATOR_RSI, INDI_TYPE_UNKNOWN, "RSI", "Relative Strength Index",
     _rsiGetStringIndicatorParam, _rsiGetIndicatorParamFromString},
    {STOCK_INDICATOR_ASI, INDI_TYPE_UNKNOWN, "ASI", "Accumulation Swing Index",
     _asiGetStringIndicatorParam, _asiGetIndicatorParamFromString},
    {STOCK_INDICATOR_ATR, INDI_TYPE_UNKNOWN, "ATR", "Average True Range",
     _atrGetStringIndicatorParam, _atrGetIndicatorParamFromString},
    {STOCK_INDICATOR_OBV, INDI_TYPE_UNKNOWN, "OBV", "On Balance Volume",
     _obvGetStringIndicatorParam, _obvGetIndicatorParamFromString},
};

/* --- public routine section ------------------------------------------------------------------- */

da_indicator_desc_t * getDaIndicatorDesc(olint_t id)
{
    if ((id == 0) || (id >= STOCK_INDICATOR_MAX))
        return NULL;

    return &ls_didIndicatorDesc[id - 1];
}

da_indicator_desc_t * getDaIndicatorDescByName(olchar_t * name)
{
    olint_t id;
    da_indicator_desc_t * indi = NULL;

    for (id = STOCK_INDICATOR_UNKNOWN + 1; id < STOCK_INDICATOR_MAX; id ++)
    {
        indi = getDaIndicatorDesc(id);
        if (strcasecmp(name, indi->did_pstrName) == 0)
            return indi;
    }

    return NULL;
}

u32 freeDaDaySummaryIndicator(da_day_summary_t *summary, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;

    for (i = 0; i < num; i ++)
    {
        if (summary->dds_pdaAdxr != NULL)
            jf_jiukun_freeMemory((void **)&summary->dds_pdaAdxr);
        if (summary->dds_pddDmi != NULL)
            jf_jiukun_freeMemory((void **)&summary->dds_pddDmi);
        if (summary->dds_pdmMacd != NULL)
            jf_jiukun_freeMemory((void **)&summary->dds_pdmMacd);
        if (summary->dds_pdmMtm != NULL)
            jf_jiukun_freeMemory((void **)&summary->dds_pdmMtm);
        if (summary->dds_pdrRsi != NULL)
            jf_jiukun_freeMemory((void **)&summary->dds_pdrRsi);
        if (summary->dds_pdkKdj != NULL)
            jf_jiukun_freeMemory((void **)&summary->dds_pdkKdj);
        if (summary->dds_pdaAsi != NULL)
            jf_jiukun_freeMemory((void **)&summary->dds_pdaAsi);
        if (summary->dds_pdaAtr != NULL)
            jf_jiukun_freeMemory((void **)&summary->dds_pdaAtr);
        if (summary->dds_pdoObv != NULL)
            jf_jiukun_freeMemory((void **)&summary->dds_pdoObv);

        summary ++;
    }

    return u32Ret;
}

u32 getIndicatorAdxrTrend(da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _caluAdxr(buffer, num);

    return u32Ret;
}

olchar_t * getStringKcp(olint_t nKcp)
{
    if (nKcp == KCP_TREND)
        return "Trend";
    else if (nKcp == KCP_ANTI_TREND)
        return "Anti Trend";
    else
        return "Unknown";
}

olchar_t * getAbbrevStringKcp(olint_t nKcp)
{
    if (nKcp == KCP_TREND)
        return "T";
    else if (nKcp == KCP_ANTI_TREND)
        return "AT";
    else
        return "N/A";
}

olchar_t * getStringIndicatorName(olint_t nIndicator)
{
    if ((nIndicator == 0) || (nIndicator >= STOCK_INDICATOR_MAX))
        return "Unknown";

    return ls_pstrIndicator[nIndicator];
}

olchar_t * getStringIndiType(olint_t type)
{
    if (type == INDI_TYPE_TREND)
        return "Trend";
    else if (type == INDI_TYPE_ANTI_TREND)
        return "Anti Trend";
    else
        return "unknown";
}

/*------------------------------------------------------------------------------------------------*/


