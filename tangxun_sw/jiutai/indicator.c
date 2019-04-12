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

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <math.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <stdlib.h>
#endif

/* --- internal header files ----------------------------------------------- */
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

/* --- private data/data structure section --------------------------------- */

static jf_clieng_caption_t ls_jccDmiParamVerbose[] =
{
    {"Days", JF_CLIENG_CAP_HALF_LINE}, {"AdxMaDays", JF_CLIENG_CAP_HALF_LINE},
    {"AdxrTrend", JF_CLIENG_CAP_FULL_LINE},
};

static jf_clieng_caption_t ls_jccMacdParamVerbose[] =
{
    {"ShortDays", JF_CLIENG_CAP_HALF_LINE}, {"LongDays", JF_CLIENG_CAP_HALF_LINE},
    {"MDays", JF_CLIENG_CAP_FULL_LINE},
};

static jf_clieng_caption_t ls_jccMtmParamVerbose[] =
{
    {"Days", JF_CLIENG_CAP_HALF_LINE}, {"MaDays", JF_CLIENG_CAP_HALF_LINE},
};

static jf_clieng_caption_t ls_jccRsiParamVerbose[] =
{
    {"1Days", JF_CLIENG_CAP_HALF_LINE}, {"2Days", JF_CLIENG_CAP_HALF_LINE},
    {"3Days", JF_CLIENG_CAP_FULL_LINE},
    {"MaxOpening1", JF_CLIENG_CAP_HALF_LINE}, {"MinCloseout1", JF_CLIENG_CAP_HALF_LINE},
};

static jf_clieng_caption_t ls_jccKdjParamVerbose[] =
{
    {"NDays", JF_CLIENG_CAP_HALF_LINE}, {"M1Days", JF_CLIENG_CAP_HALF_LINE},
    {"M2Days", JF_CLIENG_CAP_FULL_LINE},
    {"MaxOpeningD", JF_CLIENG_CAP_HALF_LINE}, {"MinCloseoutJ", JF_CLIENG_CAP_HALF_LINE},
};

static jf_clieng_caption_t ls_jccAsiParamVerbose[] =
{
    {"MaDays", JF_CLIENG_CAP_FULL_LINE},
};

static jf_clieng_caption_t ls_jccAtrParamVerbose[] =
{
    {"Days", JF_CLIENG_CAP_HALF_LINE}, {"MaDays", JF_CLIENG_CAP_HALF_LINE},
};

static jf_clieng_caption_t ls_jccObvParamVerbose[] =
{
    {"Days", JF_CLIENG_CAP_FULL_LINE},
};

static jf_clieng_caption_t ls_jccDaIndicatorDescVerbose[] =
{
    {"Id", JF_CLIENG_CAP_HALF_LINE}, {"Name", JF_CLIENG_CAP_HALF_LINE},
    {"Desc", JF_CLIENG_CAP_FULL_LINE},
    {"NumOfStock", JF_CLIENG_CAP_FULL_LINE},
};

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

/* --- private routine section---------------------------------------------- */

static u32 _freeDaDaySummaryIndicator(da_day_summary_t *summary, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;

    for (i = 0; i < num; i ++)
    {
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

static boolean_t _isIndiFanShapedUp(
    da_day_summary_t * buffer, olint_t total)
{
    boolean_t bRet = FALSE;
    da_day_summary_t * cur = buffer + total - 1;
    da_day_summary_t * inp[200];
    olint_t nump = 200, i, days;

    return TRUE;

    if (total < 2)
        return bRet;

    if (cur->dds_dbVolumeRatio >= 10.0)
        return bRet;

    getDaySummaryInflexionPoint(buffer, total, inp, &nump);
    if (nump < 4)
        return bRet;

    if (cur->dds_dbClosingPrice < inp[nump - 2]->dds_dbClosingPrice)
        return bRet;

    if (cur->dds_dbClosingPrice >=
        inp[nump - 3]->dds_dbClosingPrice * (1 - STRAIGHT_LINE_MOTION))
        return bRet;

    if (inp[nump - 2]->dds_dbClosingPrice < inp[nump - 4]->dds_dbClosingPrice)
        return bRet;

    days = cur - inp[nump - 2];
    cur = inp[nump - 2] + 1;
    for (i = 0; i < days; i ++)
    {
        if (cur->dds_bGap)
            return bRet;
        cur ++;
    }

    bRet = TRUE;

    return bRet;
}

#if 0
static boolean_t _isNearPressureLine_OLD(
    da_day_summary_t * buffer, olint_t total)
{
    boolean_t bRet = TRUE;
    da_day_summary_t * end, * lastpeak, * prev, * high;
    da_day_summary_t * inp[200];
    olint_t nump = 200;
    oldouble_t dblow, dbhigh;

    if (total < OPTIMIZE_INDI_DAY_SUMMARY)
        return bRet;

    buffer = buffer + total - OPTIMIZE_INDI_DAY_SUMMARY;
    total = OPTIMIZE_INDI_DAY_SUMMARY;

    getDaySummaryInflexionPoint(buffer, total, inp, &nump);
    if (nump < 3)
        return bRet;

    end = buffer + total - 1;
    if (end->dds_dbClosingPrice > inp[nump - 2]->dds_dbClosingPrice)
        lastpeak = inp[nump - 3];
    else
        lastpeak = inp[nump - 2];

    dblow = lastpeak->dds_dbClosingPrice * (1 - STRAIGHT_LINE_MOTION);
    if ((end->dds_dbClosingPrice >= dblow) &&
        (end->dds_dbClosingPrice <= lastpeak->dds_dbClosingPrice))
        return bRet;

//    dbhigh = lastpeak->dds_dbClosingPrice * (1 + STRAIGHT_LINE_MOTION);
    high = getDaySummaryWithHighestHighPrice(buffer, total - 1);
    dbhigh = high->dds_dbHighPrice;
    prev = end - 1;
    if ((end->dds_dbClosingPrice > lastpeak->dds_dbClosingPrice) &&
        (end->dds_dbClosingPrice <= dbhigh) &&
        (prev->dds_dbClosingPrice <= lastpeak->dds_dbClosingPrice))
        return bRet;

    bRet = FALSE;

    return bRet;
}
#endif

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
        (void **)&end->dds_pdaAdxr, sizeof(da_adxr_t),
        JF_FLAG_MASK(JF_JIUKUN_MEM_ALLOC_FLAG_ZERO));

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

    jf_jiukun_allocMemory((void **)&sort, sizeof(da_day_summary_t *) * num, 0);
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

static u32 _isMatchingIndi(
    struct da_indicator_desc * indi, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
    da_adxr_t * daadxr;

    u32Ret = _caluAdxr(buffer, num);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    end = buffer + num - 1;
    daadxr = end->dds_pdaAdxr;
    if (daadxr->da_nKcp == KCP_TREND)
    {
        if ((indi->did_nType != INDI_TYPE_TREND) ||
            (daadxr->da_dbAdxr <= daadxr->da_dbAdxrTrend))
            return JF_ERR_NOT_MATCH;
    }
    else if (daadxr->da_nKcp == KCP_ANTI_TREND)
    {
        if ((indi->did_nType != INDI_TYPE_ANTI_TREND) ||
            (daadxr->da_dbAdxr > daadxr->da_dbAdxrTrend))
            return JF_ERR_NOT_MATCH;
    }

    return u32Ret;
}

static u32 _optIndicatorSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * param,
    da_day_summary_t * buffer, olint_t num, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, start;
    da_conc_t daconc;

    memset(&daconc, 0, sizeof(da_conc_t));
    memset(conc, 0, sizeof(da_conc_sum_t));

    if (num > OPTIMIZE_INDI_DAY_SUMMARY)
        start = num - OPTIMIZE_INDI_DAY_SUMMARY + 1;
    else
        start = 10;

    for (i = start;
         (i <= num) && (u32Ret == JF_ERR_NO_ERROR); i ++)
    {
        u32Ret = indi->did_fnSystem(
            indi, param, buffer, i, &daconc);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (daconc.dc_nAction == CONC_ACTION_BULL_OPENING)
            {

            }
            else if (daconc.dc_nAction == CONC_ACTION_BULL_CLOSEOUT)
            {
                addToDaConcSum(conc, &daconc);
            }
        }
        else if (u32Ret == JF_ERR_NOT_READY)
            u32Ret = JF_ERR_NO_ERROR;
    }

    return u32Ret;
}

static void _newDmi(
    da_day_summary_t * buffer, olint_t num, da_dmi_param_t * param)
{
    da_day_summary_t * end, * start;
    da_dmi_t * dmi;
    olint_t i;
    oldouble_t dbt, dbp, dbn;
    oldouble_t db1, db2;

    end = buffer + num - 1;
    if (end->dds_pddDmi != NULL)
        return;

    jf_jiukun_allocMemory(
        (void **)&end->dds_pddDmi, sizeof(da_dmi_t),
        JF_FLAG_MASK(JF_JIUKUN_MEM_ALLOC_FLAG_ZERO));

    dmi = end->dds_pddDmi;
    dbt = dbp = dbn = 0;
    for (i = 0; i < param->ddp_nDmiDays; i ++)
    {
        dbt += _calcTrueRange(end, end - 1);
        _calcDirectMove(end, end - 1, &db1, &db2);
        dbp += db1;
        dbn += db2;
        end --;
    }
    dmi->dd_dbTr14 = dbt;
    dmi->dd_dbDmp14 = dbp;
    dmi->dd_dbDmm14 = dbn;
    dmi->dd_dbPdi =
        dmi->dd_dbDmp14 * 100 / dmi->dd_dbTr14;
    dmi->dd_dbMdi =
        dmi->dd_dbDmm14 * 100 / dmi->dd_dbTr14;
    dmi->dd_dbDmi =
        ABS((dmi->dd_dbPdi - dmi->dd_dbMdi) * 100 /
            (dmi->dd_dbPdi + dmi->dd_dbMdi));

    end = buffer + num - 1;
    start = end - param->ddp_nDmiAdxMaDays;
    if (start->dds_pddDmi != NULL)
    {
        start ++;
        dbt = 0;
        for (i = 0; i < param->ddp_nDmiAdxMaDays; i ++)
        {
            dbt += start->dds_pddDmi->dd_dbDmi;
            start ++;
        }
        dmi->dd_dbAdx = dbt / param->ddp_nDmiAdxMaDays;
        dmi->dd_dbAdxr =
            (dmi->dd_dbAdx + (end - param->ddp_nDmiAdxMaDays)->dds_pddDmi->dd_dbAdx) / 2;
    }
}

static u32 _dmiSystemOpening(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_dmi_param_t * param = &pdip->dip_ddpDmi;
    da_day_summary_t * end;
    da_dmi_t * dmi, * dmip;
    olint_t i;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    for (i = num - (param->ddp_nDmiAdxMaDays + 1) * 2; i <= num; i ++)
        _newDmi(buffer, i, param);

    if (_isMatchingIndi(indi, buffer, num) != JF_ERR_NO_ERROR)
        return u32Ret;

    dmi = end->dds_pddDmi;
    dmip = (end - 1)->dds_pddDmi;

    if (//! _isNearPressureLine(buffer, num) &&
        (dmip->dd_dbAdx < dmi->dd_dbAdx) &&
		(dmi->dd_dbPdi > dmi->dd_dbMdi) &&
        (dmip->dd_dbPdi < dmip->dd_dbMdi))
    {
        conc->dc_nAction = CONC_ACTION_BULL_OPENING;
        conc->dc_nPosition = CONC_POSITION_FULL;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsOpening = end;
    }

    return u32Ret;
}

static u32 _dmiSystemCloseout(
    da_day_summary_t * buffer, olint_t num, da_dmi_param_t * param,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
    da_dmi_t * dmi, * dmip, * dmio;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    _newDmi(buffer, num, param);
    dmi = end->dds_pddDmi;
    dmip = (end - 1)->dds_pddDmi;
    dmio = conc->dc_pddsOpening->dds_pddDmi;
    if (((dmip->dd_dbAdx > dmio->dd_dbAdx) &&
         (dmi->dd_dbAdx < dmip->dd_dbAdx)) ||
        (dmi->dd_dbPdi < dmi->dd_dbMdi))
    {
        conc->dc_nAction = CONC_ACTION_BULL_CLOSEOUT;
        conc->dc_nPosition = CONC_POSITION_SHORT;
        conc->dc_dbYield =
            (end->dds_dbClosingPrice - conc->dc_dbPrice) * 100 / conc->dc_dbPrice;
        /*the yield minus 1%, as other charges*/
        conc->dc_dbYield -= 1.0;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsCloseout = end;
        conc->dc_nHoldDays = conc->dc_pddsCloseout - conc->dc_pddsOpening;
    }

    return u32Ret;
}

static u32 _dmiSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_dmi_param_t * param = &pdip->dip_ddpDmi;
    olint_t mday;
    olint_t nMinDmiDaySummary;

    if (param->ddp_nDmiDays == 0)
    {
        param->ddp_nDmiDays = DEF_DMI_DAYS;
    }
    if (param->ddp_nDmiAdxMaDays == 0)
    {
        param->ddp_nDmiAdxMaDays = DEF_DMI_ADX_MA_DAYS;
    }
	if (param->ddp_dbAdxrTrend == 0)
    {
		param->ddp_dbAdxrTrend = DEF_DMI_ADXR_TREND;
    }
    mday = MAX(param->ddp_nDmiDays, param->ddp_nDmiAdxMaDays);
    nMinDmiDaySummary = (mday + 1) * 3;

    if (num < nMinDmiDaySummary)
        return JF_ERR_NOT_READY;

    if (conc->dc_nPosition == CONC_POSITION_SHORT)
        u32Ret = _dmiSystemOpening(indi, pdip, buffer, num, conc);
    else //if (! _isStopLoss(buffer + num - 1, conc))
        u32Ret = _dmiSystemCloseout(buffer, num, param, conc);

    return u32Ret;
}

static u32 _optimizeDmiSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, j;
//    oldouble_t db;
    da_indicator_param_t myparam;
    da_conc_sum_t myconc;

    memset(conc, 0, sizeof(da_conc_sum_t));
    memset(pdip, 0, sizeof(da_indicator_param_t));
#define DEF_DMI_DAYS_MIN            7
#define DEF_DMI_DAYS_MAX           28
#define DEF_DMI_ADX_MA_DAYS_MIN     3
#define DEF_DMI_ADX_MA_DAYS_MAX    12
#define DEF_DMI_ADXR_TREND_MIN   20.0
#define DEF_DMI_ADXR_TREND_MAX   30.0

    for (i = DEF_DMI_DAYS_MIN; i <= DEF_DMI_DAYS_MAX; i ++)
        for (j = DEF_DMI_ADX_MA_DAYS_MIN; j <= MIN(i - 1, DEF_DMI_ADX_MA_DAYS_MAX); j ++)
//            for (db = DEF_DMI_ADXR_TREND_MIN; db <= DEF_DMI_ADXR_TREND_MAX; db = db + 1)
            {
                myparam.dip_ddpDmi.ddp_nDmiDays = i;
                myparam.dip_ddpDmi.ddp_nDmiAdxMaDays = j;
                myparam.dip_ddpDmi.ddp_dbAdxrTrend = 0;
                u32Ret = _optIndicatorSystem(indi, &myparam, buffer, num, &myconc);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    if (compareDaConcSum(&myconc, conc) > 0)
                    {
                        memcpy(pdip, &myparam, sizeof(da_indicator_param_t));
                        memcpy(conc, &myconc, sizeof(da_conc_sum_t));
                    }
                }
                _freeDaDaySummaryIndicator(buffer, num);

                if (u32Ret != JF_ERR_NO_ERROR)
                    goto out;
            }
out:

    return u32Ret;
}

static void _printDaDmiParamVerbose(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip)
{
    da_dmi_param_t * param = &pdip->dip_ddpDmi;
    jf_clieng_caption_t * pcc = &ls_jccDmiParamVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Days*/
    ol_sprintf(strLeft, "%d", param->ddp_nDmiDays);
    ol_sprintf(strRight, "%d", param->ddp_nDmiAdxMaDays);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*AdxrTrend*/
    ol_sprintf(strLeft, "%.1f", param->ddp_dbAdxrTrend);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;
}

static void _dmiPrintDaySummary(
    struct da_indicator_desc * indi, da_day_summary_t * buffer, olint_t num)
{
    olint_t i;
    olchar_t buf[512];
    da_day_summary_t * cur;
    da_dmi_t * dmi;

    cur = buffer;
    for (i = 0; i < num; i ++)
    {
        dmi = cur->dds_pddDmi;
        if (dmi == NULL)
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f ",
                cur->dds_nIndex, cur->dds_strDate,
                cur->dds_dbHighPrice, cur->dds_dbLowPrice, cur->dds_dbClosingPrice);
        }
        else if (cur->dds_pdaAdxr == NULL)
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f N/A 0.0 0.0 "
                "%.2f %.2f %.2f",
                cur->dds_nIndex, cur->dds_strDate, cur->dds_dbHighPrice,
                cur->dds_dbLowPrice, cur->dds_dbClosingPrice,
                dmi->dd_dbPdi, dmi->dd_dbMdi, dmi->dd_dbAdx);
        }
        else
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f %s %.2f %.2f "
                "%.2f %.2f %.2f",
                cur->dds_nIndex, cur->dds_strDate, cur->dds_dbHighPrice,
                cur->dds_dbLowPrice, cur->dds_dbClosingPrice,
                getAbbrevStringKcp(cur->dds_pdaAdxr->da_nKcp),
                cur->dds_pdaAdxr->da_dbAdxr, cur->dds_pdaAdxr->da_dbAdxrTrend,
                dmi->dd_dbPdi, dmi->dd_dbMdi, dmi->dd_dbAdx);
        }

        jf_clieng_outputLine("%s", buf);
        cur ++;
    }
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

static void _newMacd(
    da_day_summary_t * buffer, olint_t num, da_macd_param_t * param)
{
    da_day_summary_t * end, * prev, * start;
    da_macd_t * macd;
    oldouble_t longk, shortk, mk;

    end = buffer + num - 1;
    if (end->dds_pdmMacd != NULL)
        return;

    jf_jiukun_allocMemory(
        (void **)&end->dds_pdmMacd, sizeof(da_macd_t),
        JF_FLAG_MASK(JF_JIUKUN_MEM_ALLOC_FLAG_ZERO));

    shortk = 2.0 / (param->dmp_nMacdShortDays + 1);
    longk = 2.0 / (param->dmp_nMacdLongDays + 1);
    mk = 2.0 / (param->dmp_nMacdMDays + 1);
    macd = end->dds_pdmMacd;
    prev = end - 1;
    if (prev->dds_pdmMacd == NULL)
    {
        start = buffer;
        macd->dm_dbShortEma = start->dds_dbClosingPrice;
        macd->dm_dbLongEma = start->dds_dbClosingPrice;
        macd->dm_dbDiff = macd->dm_dbShortEma - macd->dm_dbLongEma;
        macd->dm_dbDea = macd->dm_dbDiff;
        start ++;
        while (start <= end)
        {
            macd->dm_dbShortEma =
                start->dds_dbClosingPrice * shortk + macd->dm_dbShortEma * (1 - shortk);
            macd->dm_dbLongEma =
                start->dds_dbClosingPrice * longk + macd->dm_dbLongEma * (1 - longk);
            macd->dm_dbDiff = macd->dm_dbShortEma - macd->dm_dbLongEma;
            macd->dm_dbDea =
                macd->dm_dbDiff * mk + macd->dm_dbDea * (1 - mk);
            start ++;
        }
        macd->dm_dbMacd = 2 * (macd->dm_dbDiff - macd->dm_dbDea);
    }
    else
    {
        macd->dm_dbShortEma =
            end->dds_dbClosingPrice * shortk + prev->dds_pdmMacd->dm_dbShortEma * (1 - shortk);
        macd->dm_dbLongEma =
            end->dds_dbClosingPrice * longk + prev->dds_pdmMacd->dm_dbLongEma * (1 - longk);
        macd->dm_dbDiff = macd->dm_dbShortEma - macd->dm_dbLongEma;
        macd->dm_dbDea = macd->dm_dbDiff * mk + prev->dds_pdmMacd->dm_dbDea * (1 - mk);
        macd->dm_dbMacd = 2 * (macd->dm_dbDiff - macd->dm_dbDea);
    }
}

static u32 _macdSystemOpening(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_macd_param_t * param = &pdip->dip_dmpMacd;
    da_day_summary_t * end;
    da_macd_t * macd, * macdp;
    olint_t i;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    for (i = 2; i <= num; i ++)
        _newMacd(buffer, i, param);

    if (_isMatchingIndi(indi, buffer, num) != JF_ERR_NO_ERROR)
        return u32Ret;

    macd = end->dds_pdmMacd;
    macdp = (end - 1)->dds_pdmMacd;
    if (//! _isNearPressureLine(buffer, num) &&
        (macd->dm_dbDiff >= 0) &&
        (macd->dm_dbDiff > macd->dm_dbDea) &&
        (macd->dm_dbMacd > macdp->dm_dbMacd) &&
        (macdp->dm_dbDiff < macdp->dm_dbDea))
    {
        conc->dc_nAction = CONC_ACTION_BULL_OPENING;
        conc->dc_nPosition = CONC_POSITION_FULL;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsOpening = end;
    }

    return u32Ret;
}

static u32 _macdSystemCloseout(
    da_day_summary_t * buffer, olint_t num, da_macd_param_t * param,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
    da_macd_t * macd;
//    da_macd_t * macdp;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    _newMacd(buffer, num, param);
    macd = end->dds_pdmMacd;
//    macdp = (end - 1)->dds_pdmMacd;
    if (//(macd->dm_dbMacd < macdp->dm_dbMacd) ||
        (macd->dm_dbDiff < macd->dm_dbDea))
    {
        conc->dc_nAction = CONC_ACTION_BULL_CLOSEOUT;
        conc->dc_nPosition = CONC_POSITION_SHORT;
        conc->dc_dbYield =
            (end->dds_dbClosingPrice - conc->dc_dbPrice) * 100 / conc->dc_dbPrice;
        /*the yield minus 1%, as other charges*/
        conc->dc_dbYield -= 1.0;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsCloseout = end;
        conc->dc_nHoldDays = conc->dc_pddsCloseout - conc->dc_pddsOpening;
    }

    return u32Ret;
}

static u32 _macdSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_macd_param_t * param = &pdip->dip_dmpMacd;
    olint_t nMinMacdDaySummary;

    if (param->dmp_nMacdShortDays == 0)
    {
        param->dmp_nMacdShortDays = DEF_MACD_SHORT_DAYS;
    }
    if (param->dmp_nMacdLongDays == 0)
    {
		param->dmp_nMacdLongDays = DEF_MACD_LONG_DAYS;
    }
    if (param->dmp_nMacdMDays == 0)
    {
        param->dmp_nMacdMDays = DEF_MACD_M_DAYS;
    }
    nMinMacdDaySummary = MAX(param->dmp_nMacdShortDays, param->dmp_nMacdLongDays);
    if (nMinMacdDaySummary < param->dmp_nMacdMDays)
        nMinMacdDaySummary = param->dmp_nMacdMDays;

    if (num < nMinMacdDaySummary)
        return JF_ERR_NOT_READY;

    if (conc->dc_nPosition == CONC_POSITION_SHORT)
        u32Ret = _macdSystemOpening(indi, pdip, buffer, num, conc);
    else //if (! _isStopLoss(buffer + num - 1, conc))
        u32Ret = _macdSystemCloseout(buffer, num, param, conc);

    return u32Ret;
}

static u32 _optimizeMacdSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, j, k;
    da_indicator_param_t myparam;
    da_conc_sum_t myconc;

    memset(conc, 0, sizeof(da_conc_sum_t));
    memset(pdip, 0, sizeof(da_indicator_param_t));
#define DEF_MACD_SHORT_DAYS_MIN   6
#define DEF_MACD_SHORT_DAYS_MAX  24
#define DEF_MACD_LONG_DAYS_MIN   13
#define DEF_MACD_LONG_DAYS_MAX   52
#define DEF_MACD_M_DAYS_MIN       4
#define DEF_MACD_M_DAYS_MAX      18

    for (i = DEF_MACD_SHORT_DAYS_MIN; i <= DEF_MACD_SHORT_DAYS_MAX; i ++)
        for (j = MAX(i + 1, DEF_MACD_LONG_DAYS_MIN); j <= MIN((i + 1) * 2, DEF_MACD_LONG_DAYS_MAX); j ++)
            for (k = DEF_MACD_M_DAYS_MIN; k <= MIN(i - 1, DEF_MACD_M_DAYS_MAX); k ++)
            {
                myparam.dip_dmpMacd.dmp_nMacdShortDays = i;
                myparam.dip_dmpMacd.dmp_nMacdLongDays = j;
                myparam.dip_dmpMacd.dmp_nMacdMDays = k;
                u32Ret = _optIndicatorSystem(indi, &myparam, buffer, num, &myconc);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    if (compareDaConcSum(&myconc, conc) > 0)
                    {
                        memcpy(pdip, &myparam, sizeof(da_indicator_param_t));
                        memcpy(conc, &myconc, sizeof(da_conc_sum_t));
                    }
                }
                _freeDaDaySummaryIndicator(buffer, num);

                if (u32Ret != JF_ERR_NO_ERROR)
                    goto out;
            }
out:
    return u32Ret;
}

static void _printDaMacdParamVerbose(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip)
{
    da_macd_param_t * param = &pdip->dip_dmpMacd;
    jf_clieng_caption_t * pcc = &ls_jccMacdParamVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Days*/
    ol_sprintf(strLeft, "%d", param->dmp_nMacdShortDays);
    ol_sprintf(strRight, "%d", param->dmp_nMacdLongDays);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*MDays*/
    ol_sprintf(strLeft, "%d", param->dmp_nMacdMDays);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;
}

static void _macdPrintDaySummary(
    struct da_indicator_desc * indi, da_day_summary_t * buffer, olint_t num)
{
    olint_t i;
    olchar_t buf[512];
    da_day_summary_t * cur;
    da_macd_t * macd;

    cur = buffer;
    for (i = 0; i < num; i ++)
    {
        macd = cur->dds_pdmMacd;
        if (macd == NULL)
        {
            ol_sprintf(
                buf, "%3d %s %.2f ",
                cur->dds_nIndex, cur->dds_strDate, cur->dds_dbClosingPrice);
        }
        else if (cur->dds_pdaAdxr == NULL)
        {
            ol_sprintf(
                buf, "%3d %s %.2f N/A 0.0 0.0 "
                "%.3f %.3f %.3f",
                cur->dds_nIndex, cur->dds_strDate,
                cur->dds_dbClosingPrice,
                macd->dm_dbDiff, macd->dm_dbDea, macd->dm_dbMacd);
        }
        else
        {
            ol_sprintf(
                buf, "%3d %s %.2f %s %.2f %.2f "
                "%.3f %.3f %.3f",
                cur->dds_nIndex, cur->dds_strDate, cur->dds_dbClosingPrice,
                getAbbrevStringKcp(cur->dds_pdaAdxr->da_nKcp),
                cur->dds_pdaAdxr->da_dbAdxr, cur->dds_pdaAdxr->da_dbAdxrTrend,
                macd->dm_dbDiff, macd->dm_dbDea, macd->dm_dbMacd);
        }
        jf_clieng_outputLine("%s", buf);
        cur ++;
    }
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

static void _newMtm(
    da_day_summary_t * buffer, olint_t num, da_mtm_param_t * param)
{
    da_day_summary_t * end, * period, * start;
    da_mtm_t * mtm;
    oldouble_t dbt;
    olint_t i;

    end = buffer + num - 1;
    if (end->dds_pdmMtm != NULL)
        return;

    jf_jiukun_allocMemory(
        (void **)&end->dds_pdmMtm, sizeof(da_mtm_t),
        JF_FLAG_MASK(JF_JIUKUN_MEM_ALLOC_FLAG_ZERO));

    period = end - param->dmp_nMtmDays;
    mtm = end->dds_pdmMtm;
    mtm->dm_dbMtm = end->dds_dbClosingPrice - period->dds_dbClosingPrice;

    start = buffer + num - param->dmp_nMtmMaDays;
    if (start->dds_pdmMtm != NULL)
    {
        dbt = 0;
        for (i = 0; i < param->dmp_nMtmMaDays; i ++)
        {
            dbt += start->dds_pdmMtm->dm_dbMtm;
            start ++;
        }
        mtm->dm_dbMaMtm = dbt / param->dmp_nMtmMaDays;
    }
}

static u32 _mtmSystemOpening(
    da_day_summary_t * buffer, olint_t num, da_mtm_param_t * param,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
    da_mtm_t * mtm, * mtmp;
    olint_t i;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    for (i = num - param->dmp_nMtmMaDays - 1; i <= num; i ++)
        _newMtm(buffer, i, param);
    mtm = end->dds_pdmMtm;
    mtmp = (end - 1)->dds_pdmMtm;
    if (_isIndiFanShapedUp(buffer, num) &&
        (mtm->dm_dbMtm > mtm->dm_dbMaMtm) &&
        (mtmp->dm_dbMtm < mtmp->dm_dbMaMtm))
    {
        conc->dc_nAction = CONC_ACTION_BULL_OPENING;
        conc->dc_nPosition = CONC_POSITION_FULL;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsOpening = end;
    }

    return u32Ret;
}

static u32 _mtmSystemCloseout(
    da_day_summary_t * buffer, olint_t num, da_mtm_param_t * param,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
    da_mtm_t * mtm;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    _newMtm(buffer, num, param);
    mtm = end->dds_pdmMtm;
    if (mtm->dm_dbMtm < mtm->dm_dbMaMtm)
    {
        conc->dc_nAction = CONC_ACTION_BULL_CLOSEOUT;
        conc->dc_nPosition = CONC_POSITION_SHORT;
        conc->dc_dbYield =
            (end->dds_dbClosingPrice - conc->dc_dbPrice) * 100 / conc->dc_dbPrice;
        /*the yield minus 1%, as other charges*/
        conc->dc_dbYield -= 1.0;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsCloseout = end;
        conc->dc_nHoldDays = conc->dc_pddsCloseout - conc->dc_pddsOpening;
    }

    return u32Ret;
}

static u32 _mtmSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_mtm_param_t * param = &pdip->dip_dmpMtm;
    olint_t nMinMtmDaySummary;

    if (param->dmp_nMtmDays == 0)
    {
        param->dmp_nMtmDays = DEF_MTM_DAYS;
    }
	if (param->dmp_nMtmMaDays == 0)
    {
		param->dmp_nMtmMaDays = DEF_MTM_MA_DAYS;
    }
    nMinMtmDaySummary = MAX(param->dmp_nMtmDays, param->dmp_nMtmMaDays);
    nMinMtmDaySummary = (nMinMtmDaySummary + 1) * 2;

    if (num < nMinMtmDaySummary)
        return JF_ERR_NOT_READY;

    if (conc->dc_nPosition == CONC_POSITION_SHORT)
        u32Ret = _mtmSystemOpening(buffer, num, param, conc);
    else //if (! _isStopLoss(buffer + num - 1, conc))
        u32Ret = _mtmSystemCloseout(buffer, num, param, conc);

    return u32Ret;
}

static u32 _optimizeMtmSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, j;
    da_indicator_param_t myparam;
    da_conc_sum_t myconc;

    memset(pdip, 0, sizeof(da_indicator_param_t));
    memset(conc, 0, sizeof(da_conc_sum_t));

    return u32Ret;

#define DEF_MTM_DAYS_MIN       6
#define DEF_MTM_DAYS_MAX      24
#define DEF_MTM_MA_DAYS_MIN    3
#define DEF_MTM_MA_DAYS_MAX   12

    for (i = DEF_MTM_DAYS_MIN; i <= DEF_MTM_DAYS_MAX; i ++)
        for (j = DEF_MTM_MA_DAYS_MIN; j <= i; j ++)
        {
            myparam.dip_dmpMtm.dmp_nMtmDays = i;
            myparam.dip_dmpMtm.dmp_nMtmMaDays = j;
            u32Ret = _optIndicatorSystem(indi, &myparam, buffer, num, &myconc);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                if (compareDaConcSum(&myconc, conc) > 0)
                {
                    memcpy(pdip, &myparam, sizeof(da_indicator_param_t));
                    memcpy(conc, &myconc, sizeof(da_conc_sum_t));
                }
            }
            _freeDaDaySummaryIndicator(buffer, num);

            if (u32Ret != JF_ERR_NO_ERROR)
                goto out;
        }
out:
    return u32Ret;
}

static void _printDaMtmParamVerbose(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip)
{
    da_mtm_param_t * param = &pdip->dip_dmpMtm;
    jf_clieng_caption_t * pcc = &ls_jccMtmParamVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Days*/
    ol_sprintf(strLeft, "%d", param->dmp_nMtmDays);
    ol_sprintf(strRight, "%d", param->dmp_nMtmMaDays);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;
}

static void _mtmPrintDaySummary(
    struct da_indicator_desc * indi, da_day_summary_t * buffer, olint_t num)
{
    olint_t i;
    olchar_t buf[512];
    da_day_summary_t * cur;
    da_mtm_t * mtm;

    cur = buffer;
    for (i = 0; i < num; i ++)
    {
        mtm = cur->dds_pdmMtm;
        if (mtm == NULL)
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f ",
                cur->dds_nIndex, cur->dds_strDate,
                cur->dds_dbHighPrice, cur->dds_dbLowPrice, cur->dds_dbClosingPrice);
        }
        else
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f "
                "%.2f %.2f",
                cur->dds_nIndex, cur->dds_strDate,
                cur->dds_dbHighPrice, cur->dds_dbLowPrice, cur->dds_dbClosingPrice,
                mtm->dm_dbMtm, mtm->dm_dbMaMtm);
        }
        jf_clieng_outputLine("%s", buf);
        cur ++;
    }
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

static void _newRsi(
    da_day_summary_t * buffer, olint_t num, da_rsi_param_t * param)
{
    da_day_summary_t * end, * prev, * start;
    da_rsi_t * rsi;
    oldouble_t k2, k1, k3;
    oldouble_t db;

    end = buffer + num - 1;
    if (end->dds_pdrRsi != NULL)
        return;

    jf_jiukun_allocMemory(
        (void **)&end->dds_pdrRsi, sizeof(da_rsi_t),
        JF_FLAG_MASK(JF_JIUKUN_MEM_ALLOC_FLAG_ZERO));

    k1 = 1.0 / param->drp_nRsi1Days;
    k2 = 1.0 / param->drp_nRsi2Days;
    k3 = 1.0 / param->drp_nRsi3Days;
    rsi = end->dds_pdrRsi;
    prev = end - 1;
    if (prev->dds_pdrRsi == NULL)
    {
        start = buffer + 1;
        db = start->dds_dbClosingPrice - (start - 1)->dds_dbClosingPrice;
        rsi->dr_dbSmaRsi1P = rsi->dr_dbSmaRsi2P = rsi->dr_dbSmaRsi3P = MAX(db, 0);
        rsi->dr_dbSmaRsi1A = rsi->dr_dbSmaRsi2A = rsi->dr_dbSmaRsi3A = ABS(db);

        start ++;
        while (start <= end)
        {
            db = start->dds_dbClosingPrice - (start - 1)->dds_dbClosingPrice;
            rsi->dr_dbSmaRsi1P = MAX(db, 0) * k1 + rsi->dr_dbSmaRsi1P * (1 - k1);
            rsi->dr_dbSmaRsi2P = MAX(db, 0) * k2 + rsi->dr_dbSmaRsi2P * (1 - k2);
            rsi->dr_dbSmaRsi3P = MAX(db, 0) * k3 + rsi->dr_dbSmaRsi3P * (1 - k3);
            rsi->dr_dbSmaRsi1A = ABS(db) * k1 + rsi->dr_dbSmaRsi1A * (1 - k1);
            rsi->dr_dbSmaRsi2A = ABS(db) * k2 + rsi->dr_dbSmaRsi2A * (1 - k2);
            rsi->dr_dbSmaRsi3A = ABS(db) * k3 + rsi->dr_dbSmaRsi3A * (1 - k3);

            start ++;
        }
        rsi->dr_dbRsi1 = rsi->dr_dbSmaRsi1P * 100 / rsi->dr_dbSmaRsi1A;
        rsi->dr_dbRsi2 = rsi->dr_dbSmaRsi2P * 100 / rsi->dr_dbSmaRsi2A;
        rsi->dr_dbRsi3 = rsi->dr_dbSmaRsi3P * 100 / rsi->dr_dbSmaRsi3A;
    }
    else
    {
        db = end->dds_dbClosingPrice - prev->dds_dbClosingPrice;
        rsi->dr_dbSmaRsi1P = MAX(db, 0) * k1 + prev->dds_pdrRsi->dr_dbSmaRsi1P * (1 - k1);
        rsi->dr_dbSmaRsi2P = MAX(db, 0) * k2 + prev->dds_pdrRsi->dr_dbSmaRsi2P * (1 - k2);
        rsi->dr_dbSmaRsi3P = MAX(db, 0) * k3 + prev->dds_pdrRsi->dr_dbSmaRsi3P * (1 - k3);
        rsi->dr_dbSmaRsi1A = ABS(db) * k1 + prev->dds_pdrRsi->dr_dbSmaRsi1A * (1 - k1);
        rsi->dr_dbSmaRsi2A = ABS(db) * k2 + prev->dds_pdrRsi->dr_dbSmaRsi2A * (1 - k2);
        rsi->dr_dbSmaRsi3A = ABS(db) * k3 + prev->dds_pdrRsi->dr_dbSmaRsi3A * (1 - k3);

        rsi->dr_dbRsi1 = rsi->dr_dbSmaRsi1P * 100 / rsi->dr_dbSmaRsi1A;
        rsi->dr_dbRsi2 = rsi->dr_dbSmaRsi2P * 100 / rsi->dr_dbSmaRsi2A;
        rsi->dr_dbRsi3 = rsi->dr_dbSmaRsi3P * 100 / rsi->dr_dbSmaRsi3A;
    }
}

static u32 _rsiSystemOpening(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_rsi_param_t * param = &pdip->dip_drpRsi;
    da_day_summary_t * end;
    da_rsi_t * rsi, * rsip;
    olint_t i;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    for (i = 2; i <= num; i ++)
        _newRsi(buffer, i, param);

    if (_isMatchingIndi(indi, buffer, num) != JF_ERR_NO_ERROR)
        return u32Ret;

    rsi = end->dds_pdrRsi;
    rsip = (end - 1)->dds_pdrRsi;
    if (//! _isNearBelowPressureLine(buffer, num) &&
        (rsi->dr_dbRsi1 > rsi->dr_dbRsi2) &&
        (rsip->dr_dbRsi1 < rsip->dr_dbRsi2) &&
        (rsip->dr_dbRsi1 <= param->drp_dbMaxOpening1))
    {
        conc->dc_nAction = CONC_ACTION_BULL_OPENING;
        conc->dc_nPosition = CONC_POSITION_FULL;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsOpening = end;
    }

    return u32Ret;
}

static u32 _rsiSystemCloseout(
    da_day_summary_t * buffer, olint_t num, da_rsi_param_t * param,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
    da_rsi_t * rsi;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    _newRsi(buffer, num, param);
    rsi = end->dds_pdrRsi;
    if ((rsi->dr_dbRsi1 < rsi->dr_dbRsi2) ||
        (rsi->dr_dbRsi1 >= param->drp_dbMinCloseout1))
    {
        conc->dc_nAction = CONC_ACTION_BULL_CLOSEOUT;
        conc->dc_nPosition = CONC_POSITION_SHORT;
        conc->dc_dbYield =
            (end->dds_dbClosingPrice - conc->dc_dbPrice) * 100 / conc->dc_dbPrice;
        /*the yield minus 1%, as other charges*/
        conc->dc_dbYield -= 1.0;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsCloseout = end;
        conc->dc_nHoldDays = conc->dc_pddsCloseout - conc->dc_pddsOpening;
    }

    return u32Ret;
}

static u32 _rsiSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_rsi_param_t * param = &pdip->dip_drpRsi;
    olint_t nMinRsiDaySummary;

    if (param->drp_nRsi1Days == 0)
        param->drp_nRsi1Days = DEF_RSI1_DAYS;
    if (param->drp_nRsi2Days == 0)
        param->drp_nRsi2Days = DEF_RSI2_DAYS;
    if (param->drp_nRsi3Days == 0)
        param->drp_nRsi3Days = DEF_RSI3_DAYS;
    if (param->drp_dbMaxOpening1 == 0)
        param->drp_dbMaxOpening1 = DEF_RSI_MAX_OPENING_1;
    if (param->drp_dbMinCloseout1 == 0)
        param->drp_dbMinCloseout1 = DEF_RSI_MIN_CLOSEOUT_1;
    nMinRsiDaySummary = (param->drp_nRsi3Days + 1) * 2;

    if (num < nMinRsiDaySummary)
        return JF_ERR_NOT_READY;

    if (conc->dc_nPosition == CONC_POSITION_SHORT)
        u32Ret = _rsiSystemOpening(indi, pdip, buffer, num, conc);
    else //if (! _isStopLoss(buffer + num - 1, conc))
        u32Ret = _rsiSystemCloseout(buffer, num, param, conc);

    return u32Ret;
}

static u32 _optimizeRsiSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, j, k;
    da_indicator_param_t myparam;
    da_conc_sum_t myconc;
    oldouble_t db1, db2;

    memset(conc, 0, sizeof(da_conc_sum_t));
    memset(pdip, 0, sizeof(da_indicator_param_t));
#define DEF_RSI1_DAYS_MIN    4
#define DEF_RSI1_DAYS_MAX    12
#define DEF_RSI2_DAYS_MIN    8
#define DEF_RSI2_DAYS_MAX    24
#define DEF_RSI3_DAYS_MIN    DEF_RSI3_DAYS //12
#define DEF_RSI3_DAYS_MAX    DEF_RSI3_DAYS //48
#define DEF_RSI_MAX_OPENING_1_MIN  (DEF_RSI_MAX_OPENING_1)
#define DEF_RSI_MAX_OPENING_1_MAX  (DEF_RSI_MAX_OPENING_1 + 5)
#define DEF_RSI_MIN_CLOSEOUT_1_MIN (DEF_RSI_MIN_CLOSEOUT_1 - 5)
#define DEF_RSI_MIN_CLOSEOUT_1_MAX (DEF_RSI_MIN_CLOSEOUT_1)

    for (i = DEF_RSI1_DAYS_MIN; i <= DEF_RSI1_DAYS_MAX; i ++)
        for (j = MAX(i + 1, DEF_RSI2_DAYS_MIN); j <= MIN((i + 1) * 2, DEF_RSI2_DAYS_MAX); j ++)
            for (k = DEF_RSI3_DAYS_MIN; k <= DEF_RSI3_DAYS_MAX; k ++)
                for (db1 = DEF_RSI_MAX_OPENING_1_MIN; db1 <= DEF_RSI_MAX_OPENING_1_MAX; db1 = db1 + 1)
                    for (db2 = DEF_RSI_MIN_CLOSEOUT_1_MIN; db2 <= DEF_RSI_MIN_CLOSEOUT_1_MAX; db2 = db2 + 1)
                    {
                        myparam.dip_drpRsi.drp_nRsi1Days = i;
                        myparam.dip_drpRsi.drp_nRsi2Days = j;
                        myparam.dip_drpRsi.drp_nRsi3Days = k;
                        myparam.dip_drpRsi.drp_dbMaxOpening1 = db1;
                        myparam.dip_drpRsi.drp_dbMinCloseout1 = db2;
                        u32Ret = _optIndicatorSystem(indi, &myparam, buffer, num, &myconc);
                        if (u32Ret == JF_ERR_NO_ERROR)
                        {
                            if (compareDaConcSum(&myconc, conc) > 0)
                            {
                                memcpy(pdip, &myparam, sizeof(da_indicator_param_t));
                                memcpy(conc, &myconc, sizeof(da_conc_sum_t));
                            }
                        }
                        _freeDaDaySummaryIndicator(buffer, num);

                        if (u32Ret != JF_ERR_NO_ERROR)
                            goto out;
                    }
out:
    return u32Ret;
}

static void _printDaRsiParamVerbose(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip)
{
    da_rsi_param_t * param = &pdip->dip_drpRsi;
    jf_clieng_caption_t * pcc = &ls_jccRsiParamVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*1Days*/
    ol_sprintf(strLeft, "%d", param->drp_nRsi1Days);
    ol_sprintf(strRight, "%d", param->drp_nRsi2Days);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*1Days*/
    ol_sprintf(strLeft, "%d", param->drp_nRsi3Days);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*MaxOpening1*/
    ol_sprintf(strLeft, "%.1f", param->drp_dbMaxOpening1);
    ol_sprintf(strRight, "%.1f", param->drp_dbMinCloseout1);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

}

static void _rsiPrintDaySummary(
    struct da_indicator_desc * indi, da_day_summary_t * buffer, olint_t num)
{
    olint_t i;
    olchar_t buf[512];
    da_day_summary_t * cur;
    da_rsi_t * rsi;

    cur = buffer;
    for (i = 0; i < num; i ++)
    {
        rsi = cur->dds_pdrRsi;
        if (rsi == NULL)
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f ",
                cur->dds_nIndex, cur->dds_strDate,
                cur->dds_dbHighPrice, cur->dds_dbLowPrice, cur->dds_dbClosingPrice);
        }
        else if (cur->dds_pdaAdxr == NULL)
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f N/A 0.0 0.0 "
                "%.2f %.2f %.2f",
                cur->dds_nIndex, cur->dds_strDate, cur->dds_dbHighPrice,
                cur->dds_dbLowPrice, cur->dds_dbClosingPrice,
                rsi->dr_dbRsi1, rsi->dr_dbRsi2, rsi->dr_dbRsi3);
        }
        else
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f %s %.2f %.2f "
                "%.2f %.2f %.2f",
                cur->dds_nIndex, cur->dds_strDate, cur->dds_dbHighPrice,
                cur->dds_dbLowPrice, cur->dds_dbClosingPrice,
                getAbbrevStringKcp(cur->dds_pdaAdxr->da_nKcp),
                cur->dds_pdaAdxr->da_dbAdxr, cur->dds_pdaAdxr->da_dbAdxrTrend,
                rsi->dr_dbRsi1, rsi->dr_dbRsi2, rsi->dr_dbRsi3);
        }
        jf_clieng_outputLine("%s", buf);
        cur ++;
    }
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

static oldouble_t _calcRsv(
    da_day_summary_t * end, da_kdj_param_t * param)
{
    oldouble_t highp, lowp, closep, rsv;
    olint_t i;

    closep = end->dds_dbClosingPrice;
    highp = 0.0;
    lowp = 99999.99;
    for (i = 0; i < param->dkp_nKdjNDays; i ++)
    {
        if (end->dds_dbHighPrice > highp)
            highp = end->dds_dbHighPrice;
        if (end->dds_dbLowPrice < lowp)
            lowp = end->dds_dbLowPrice;
        end --;
    }

    rsv = (closep - lowp) / (highp - lowp) * 100;

    return rsv;
}

static void _newKdj(
    da_day_summary_t * buffer, olint_t num, da_kdj_param_t * param)
{
    da_day_summary_t * end, * prev, * start;
    da_kdj_t * kdj;
    oldouble_t k1, k2;
    oldouble_t rsv;

    end = buffer + num - 1;
    if (end->dds_pdkKdj != NULL)
        return;

    jf_jiukun_allocMemory(
        (void **)&end->dds_pdkKdj, sizeof(da_kdj_t),
        JF_FLAG_MASK(JF_JIUKUN_MEM_ALLOC_FLAG_ZERO));

    k1 = 1.0 / param->dkp_nKdjM1Days;
    k2 = 1.0 / param->dkp_nKdjM2Days;
    kdj = end->dds_pdkKdj;
    prev = end - 1;
    if (prev->dds_pdkKdj == NULL)
    {
        start = buffer + param->dkp_nKdjNDays - 1;
        kdj->dk_dbSmaK = _calcRsv(start, param);
        kdj->dk_dbSmaD = kdj->dk_dbSmaK;

        start ++;
        while (start <= end)
        {
            rsv = _calcRsv(start, param);
            kdj->dk_dbSmaK = rsv * k1 + kdj->dk_dbSmaK * (1 - k1);
            kdj->dk_dbSmaD = kdj->dk_dbSmaK * k2 + kdj->dk_dbSmaD * (1 - k2);

            start ++;
        }
    }
    else
    {
        rsv = _calcRsv(end, param);
        kdj->dk_dbSmaK = rsv * k1 + prev->dds_pdkKdj->dk_dbSmaK * (1 - k1);
        kdj->dk_dbSmaD = kdj->dk_dbSmaK * k2 + prev->dds_pdkKdj->dk_dbSmaD * (1 - k2);
    }

    kdj->dk_dbK = kdj->dk_dbSmaK;
    if (kdj->dk_dbSmaK < 0)
        kdj->dk_dbK = 0;
    if (kdj->dk_dbSmaK > 100)
        kdj->dk_dbK = 100;
    kdj->dk_dbD = kdj->dk_dbSmaD;
    if (kdj->dk_dbSmaD < 0)
        kdj->dk_dbD = 0;
    if (kdj->dk_dbSmaD > 100)
        kdj->dk_dbD = 100;
    kdj->dk_dbJ = 3 * kdj->dk_dbSmaK - 2 * kdj->dk_dbSmaD;
    if (kdj->dk_dbJ < 0)
        kdj->dk_dbJ = 0;
    if (kdj->dk_dbJ > 100)
        kdj->dk_dbJ = 100;

}

static u32 _kdjSystemOpening(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_kdj_param_t * param = &pdip->dip_dkpKdj;
    da_day_summary_t * end;
    da_kdj_t * kdj, * kdjp;
    olint_t i;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    for (i = num - param->dkp_nKdjNDays * 2; i <= num; i ++)
        _newKdj(buffer, i, param);

    if (_isMatchingIndi(indi, buffer, num) != JF_ERR_NO_ERROR)
        return u32Ret;

    kdj = end->dds_pdkKdj;
    kdjp = (end - 1)->dds_pdkKdj;
    if (_isIndiFanShapedUp(buffer, num) &&
        (kdj->dk_dbK > kdj->dk_dbD) &&
        (kdjp->dk_dbK < kdjp->dk_dbD) &&
        (kdj->dk_dbD <= param->dkp_dbMaxOpeningD))
    {
        conc->dc_nAction = CONC_ACTION_BULL_OPENING;
        conc->dc_nPosition = CONC_POSITION_FULL;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsOpening = end;
    }

    return u32Ret;
}

static u32 _kdjSystemCloseout(
    da_day_summary_t * buffer, olint_t num, da_kdj_param_t * param,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
    da_kdj_t * kdj;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    _newKdj(buffer, num, param);
    kdj = end->dds_pdkKdj;
    if ((kdj->dk_dbJ >= param->dkp_dbMinCloseoutJ) ||
        (//(kdj->dk_dbK >= param->dkp_dbMinCloseoutK) &&
         (kdj->dk_dbK < kdj->dk_dbD)))
    {
        conc->dc_nAction = CONC_ACTION_BULL_CLOSEOUT;
        conc->dc_nPosition = CONC_POSITION_SHORT;
        conc->dc_dbYield =
            (end->dds_dbClosingPrice - conc->dc_dbPrice) * 100 / conc->dc_dbPrice;
        /*the yield minus 1%, as other charges*/
        conc->dc_dbYield -= 1.0;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsCloseout = end;
        conc->dc_nHoldDays = conc->dc_pddsCloseout - conc->dc_pddsOpening;
    }

    return u32Ret;
}

static u32 _kdjSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_kdj_param_t * param = &pdip->dip_dkpKdj;
    olint_t nMinKdjDaySummary;

    if (param->dkp_nKdjNDays == 0)
        param->dkp_nKdjNDays = DEF_KDJ_N_DAYS;
    if (param->dkp_nKdjM1Days == 0)
        param->dkp_nKdjM1Days = DEF_KDJ_M1_DAYS;
    if (param->dkp_nKdjM2Days == 0)
        param->dkp_nKdjM2Days = DEF_KDJ_M2_DAYS;
    if (param->dkp_dbMaxOpeningD == 0)
        param->dkp_dbMaxOpeningD = DEF_KDJ_MAX_OPENING_D;
    if (param->dkp_dbMinCloseoutJ == 0)
        param->dkp_dbMinCloseoutJ = DEF_KDJ_MIN_CLOSEOUT_J;
    nMinKdjDaySummary = (param->dkp_nKdjNDays + 1) * 3;

    if (num < nMinKdjDaySummary)
        return JF_ERR_NOT_READY;

    if (conc->dc_nPosition == CONC_POSITION_SHORT)
        u32Ret = _kdjSystemOpening(indi, pdip, buffer, num, conc);
    else //if (! _isStopLoss(buffer + num - 1, conc))
        u32Ret = _kdjSystemCloseout(buffer, num, param, conc);

    return u32Ret;
}

static u32 _optimizeKdjSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, j, k;
    oldouble_t db1, db2;
    da_indicator_param_t myparam;
    da_conc_sum_t myconc;

    memset(conc, 0, sizeof(da_conc_sum_t));
    memset(pdip, 0, sizeof(da_indicator_param_t));
#define DEF_KDJ_N_DAYS_MIN      5
#define DEF_KDJ_N_DAYS_MAX     18
#define DEF_KDJ_M1_DAYS_MIN     3
#define DEF_KDJ_M1_DAYS_MAX     6
#define DEF_KDJ_M2_DAYS_MIN     3
#define DEF_KDJ_M2_DAYS_MAX     6
#define DEF_KDJ_MAX_OPENING_D_MIN     (DEF_KDJ_MAX_OPENING_D)
#define DEF_KDJ_MAX_OPENING_D_MAX     (DEF_KDJ_MAX_OPENING_D + 5)
#define DEF_KDJ_MIN_CLOSEOUT_J_MIN    (DEF_KDJ_MIN_CLOSEOUT_J - 5)
#define DEF_KDJ_MIN_CLOSEOUT_J_MAX    (DEF_KDJ_MIN_CLOSEOUT_J)

    for (i = DEF_KDJ_N_DAYS_MIN; i <= DEF_KDJ_N_DAYS_MAX; i ++)
        for (j = DEF_KDJ_M1_DAYS_MIN; j < MIN(i, DEF_KDJ_M1_DAYS_MAX); j ++)
            for (k = DEF_KDJ_M2_DAYS_MIN; k < MIN(i, DEF_KDJ_M2_DAYS_MAX); k ++)
                for (db1 = DEF_KDJ_MAX_OPENING_D_MIN; db1 <= DEF_KDJ_MAX_OPENING_D_MAX; db1 = db1 + 1)
                    for (db2 = DEF_KDJ_MIN_CLOSEOUT_J_MIN; db2 <= DEF_KDJ_MIN_CLOSEOUT_J_MAX; db2 = db2 + 1)
                    {
                        myparam.dip_dkpKdj.dkp_nKdjNDays = i;
                        myparam.dip_dkpKdj.dkp_nKdjM1Days = j;
                        myparam.dip_dkpKdj.dkp_nKdjM2Days = k;
                        myparam.dip_dkpKdj.dkp_dbMaxOpeningD = db1;
                        myparam.dip_dkpKdj.dkp_dbMinCloseoutJ = db2;
                        u32Ret = _optIndicatorSystem(indi, &myparam, buffer, num, &myconc);
                        if (u32Ret == JF_ERR_NO_ERROR)
                        {
                            if (compareDaConcSum(&myconc, conc) > 0)
                            {
                                memcpy(pdip, &myparam, sizeof(da_indicator_param_t));
                                memcpy(conc, &myconc, sizeof(da_conc_sum_t));
                            }
                        }
                        _freeDaDaySummaryIndicator(buffer, num);

                        if (u32Ret != JF_ERR_NO_ERROR)
                            goto out;
                    }
out:
    return u32Ret;
}

static void _printDaKdjParamVerbose(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip)
{
    da_kdj_param_t * param = &pdip->dip_dkpKdj;
    jf_clieng_caption_t * pcc = &ls_jccKdjParamVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*NDays*/
    ol_sprintf(strLeft, "%d", param->dkp_nKdjNDays);
    ol_sprintf(strRight, "%d", param->dkp_nKdjM1Days);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*M2Days*/
    ol_sprintf(strLeft, "%d", param->dkp_nKdjM2Days);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*MaxOpeningD*/
    ol_sprintf(strLeft, "%.1f", param->dkp_dbMaxOpeningD);
    ol_sprintf(strRight, "%.1f", param->dkp_dbMinCloseoutJ);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;
}

static void _kdjPrintDaySummary(
    struct da_indicator_desc * indi, da_day_summary_t * buffer, olint_t num)
{
    olint_t i;
    olchar_t buf[512];
    da_day_summary_t * cur;
    da_kdj_t * kdj;

    cur = buffer;
    for (i = 0; i < num; i ++)
    {
        kdj = cur->dds_pdkKdj;
        if (kdj == NULL)
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f ",
                cur->dds_nIndex, cur->dds_strDate,
                cur->dds_dbHighPrice, cur->dds_dbLowPrice, cur->dds_dbClosingPrice);
        }
        else if (cur->dds_pdaAdxr == NULL)
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f N/A 0.0 0.0 "
                "%.2f %.2f %.2f",
                cur->dds_nIndex, cur->dds_strDate, cur->dds_dbHighPrice,
                cur->dds_dbLowPrice, cur->dds_dbClosingPrice,
                kdj->dk_dbK, kdj->dk_dbD, kdj->dk_dbJ);
        }
        else
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f %s %.2f %.2f "
                "%.2f %.2f %.2f",
                cur->dds_nIndex, cur->dds_strDate, cur->dds_dbHighPrice,
                cur->dds_dbLowPrice, cur->dds_dbClosingPrice,
                getAbbrevStringKcp(cur->dds_pdaAdxr->da_nKcp),
                cur->dds_pdaAdxr->da_dbAdxr, cur->dds_pdaAdxr->da_dbAdxrTrend,
                kdj->dk_dbK, kdj->dk_dbD, kdj->dk_dbJ);
        }
        jf_clieng_outputLine("%s", buf);
        cur ++;
    }

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

static oldouble_t _calcAsi(
    da_day_summary_t * end, da_day_summary_t * prev, da_asi_param_t * param)
{
    oldouble_t aa, bb, cc, dd;
    oldouble_t r, x, si;

    aa = ABS(end->dds_dbHighPrice - prev->dds_dbClosingPrice);
    bb = ABS(end->dds_dbLowPrice - prev->dds_dbClosingPrice);
    cc = ABS(end->dds_dbHighPrice - prev->dds_dbLowPrice);
    dd = ABS(prev->dds_dbClosingPrice - prev->dds_dbOpeningPrice);

    if ((aa > bb) && (aa > cc))
        r = aa + bb / 2 + dd / 4;
    else
    {
        if ((bb > cc) && (bb > aa))
            r = bb + aa / 2 + dd / 4;
        else
            r = cc + dd / 4;
    }

    x = end->dds_dbClosingPrice - prev->dds_dbClosingPrice +
        (end->dds_dbClosingPrice - end->dds_dbOpeningPrice) / 2 +
        prev->dds_dbClosingPrice - prev->dds_dbOpeningPrice;
    si = 16 * x / r * MAX(aa, bb);

    return si;
}

static void _newAsi(
    da_day_summary_t * buffer, olint_t num, da_asi_param_t * param)
{
    da_day_summary_t * end, * prev, * start;
    da_asi_t * asi;
    olint_t i;

    end = buffer + num - 1;
    if (end->dds_pdaAsi != NULL)
        return;

    jf_jiukun_allocMemory(
        (void **)&end->dds_pdaAsi, sizeof(da_asi_t),
        JF_FLAG_MASK(JF_JIUKUN_MEM_ALLOC_FLAG_ZERO));

    asi = end->dds_pdaAsi;
    prev = end - 1;
    asi->da_dbSi = _calcAsi(end, end - 1, param);
    if (prev->dds_pdaAsi == NULL)
    {
        asi->da_dbAsi = asi->da_dbSi;
    }
    else
    {
        asi->da_dbAsi = prev->dds_pdaAsi->da_dbAsi + asi->da_dbSi;
    }

    start = buffer + num - param->dap_nAsiMaDays;
    if (start->dds_pdaAsi != NULL)
    {
        for (i = 0; i < param->dap_nAsiMaDays; i ++)
        {
            asi->da_dbMaAsi += start->dds_pdaAsi->da_dbAsi;
            start ++;
        }
        asi->da_dbMaAsi /= param->dap_nAsiMaDays;
    }

}

static u32 _asiSystemOpening(
    da_day_summary_t * buffer, olint_t num, da_asi_param_t * param,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
//    da_asi_t * asi;
//    da_asi_t * asip;
    olint_t i;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    for (i = param->dap_nAsiMaDays + 1; i <= num; i ++)
        _newAsi(buffer, i, param);
//    asi = end->dds_pdaAsi;
//    asip = (end - 1)->dds_pdaAsi;
    if (0)
//        (asi->dk_dbK > asi->dk_dbD) &&
//        (asip->dk_dbK < asip->dk_dbD))
    {
        conc->dc_nAction = CONC_ACTION_BULL_OPENING;
        conc->dc_nPosition = CONC_POSITION_FULL;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsOpening = end;
    }

    return u32Ret;
}

static u32 _asiSystemCloseout(
    da_day_summary_t * buffer, olint_t num, da_asi_param_t * param,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
//    da_asi_t * asi;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    _newAsi(buffer, num, param);
//    asi = end->dds_pdaAsi;
    if (0)
//        (asi->dk_dbK < asi->dk_dbD))
    {
        conc->dc_nAction = CONC_ACTION_BULL_CLOSEOUT;
        conc->dc_nPosition = CONC_POSITION_SHORT;
        conc->dc_dbYield =
            (end->dds_dbClosingPrice - conc->dc_dbPrice) * 100 / conc->dc_dbPrice;
        /*the yield minus 1%, as other charges*/
        conc->dc_dbYield -= 1.0;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsCloseout = end;
        conc->dc_nHoldDays = conc->dc_pddsCloseout - conc->dc_pddsOpening;
    }

    return u32Ret;
}

static u32 _asiSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_asi_param_t * param = &pdip->dip_dapAsi;
    olint_t nMinAsiDaySummary;

    if (param->dap_nAsiMaDays == 0)
        param->dap_nAsiMaDays = DEF_ASI_MA_DAYS;
    nMinAsiDaySummary = (param->dap_nAsiMaDays + 1) * 3;

    if (num < nMinAsiDaySummary)
        return JF_ERR_NOT_READY;

    if (conc->dc_nPosition == CONC_POSITION_SHORT)
        u32Ret = _asiSystemOpening(buffer, num, param, conc);
    else //if (! _isStopLoss(buffer + num - 1, conc))
        u32Ret = _asiSystemCloseout(buffer, num, param, conc);

    return u32Ret;
}

static u32 _optimizeAsiSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    memset(pdip, 0, sizeof(da_indicator_param_t));
    memset(conc, 0, sizeof(da_conc_sum_t));

    return u32Ret;
}

static void _printDaAsiParamVerbose(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip)
{
    da_asi_param_t * param = &pdip->dip_dapAsi;
    jf_clieng_caption_t * pcc = &ls_jccAsiParamVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN]; //, strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*MaDays*/
    ol_sprintf(strLeft, "%d", param->dap_nAsiMaDays);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;
}

static void _asiPrintDaySummary(
    struct da_indicator_desc * indi, da_day_summary_t * buffer, olint_t num)    
{
    olint_t i;
    olchar_t buf[512];
    da_day_summary_t * cur;
    da_asi_t * asi;

    cur = buffer;
    for (i = 0; i < num; i ++)
    {
        asi = cur->dds_pdaAsi;
        if (asi == NULL)
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f ",
                cur->dds_nIndex, cur->dds_strDate,
                cur->dds_dbHighPrice, cur->dds_dbLowPrice, cur->dds_dbClosingPrice);
        }
        else
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f "
                "%.2f %.2f %.2f",
                cur->dds_nIndex, cur->dds_strDate,
                cur->dds_dbHighPrice, cur->dds_dbLowPrice, cur->dds_dbClosingPrice,
                asi->da_dbSi, asi->da_dbAsi, asi->da_dbMaAsi);
        }
        jf_clieng_outputLine("%s", buf);
        cur ++;
    }

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

static void _newAtr(
    da_day_summary_t * buffer, olint_t num, da_atr_param_t * param)
{
    da_day_summary_t * end, * start; //, * prev;
    da_atr_t * atr;
    olint_t i;

    end = buffer + num - 1;
    if (end->dds_pdaAtr != NULL)
        return;

    jf_jiukun_allocMemory(
        (void **)&end->dds_pdaAtr, sizeof(da_atr_t),
        JF_FLAG_MASK(JF_JIUKUN_MEM_ALLOC_FLAG_ZERO));

    atr = end->dds_pdaAtr;
//    prev = end - 1;

    for (i = 0; i < param->dap_nAtrDays; i ++)
    {
        atr->da_dbAtr += _calcTrueRange(end, end - 1);
        end --;
    }
    atr->da_dbAtr /= param->dap_nAtrDays;

    start = buffer + num - param->dap_nAtrMaDays;
    if (start->dds_pdaAtr != NULL)
    {
        for (i = 0; i < param->dap_nAtrMaDays; i ++)
        {
            atr->da_dbMaAtr += start->dds_pdaAtr->da_dbAtr;
            start ++;
        }
        atr->da_dbMaAtr /= param->dap_nAtrMaDays;
    }
}

static u32 _atrSystemOpening(
    da_day_summary_t * buffer, olint_t num, da_atr_param_t * param,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
    da_atr_t * atr, * atrp;
    olint_t i;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    for (i = param->dap_nAtrDays + param->dap_nAtrMaDays; i <= num; i ++)
        _newAtr(buffer, i, param);
    atr = end->dds_pdaAtr;
    atrp = (end - 1)->dds_pdaAtr;
    if (_isIndiFanShapedUp(buffer, num) &&
        (atr->da_dbAtr > atr->da_dbMaAtr) &&
        (atrp->da_dbAtr < atrp->da_dbMaAtr))
    {
        conc->dc_nAction = CONC_ACTION_BULL_OPENING;
        conc->dc_nPosition = CONC_POSITION_FULL;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsOpening = end;
    }

    return u32Ret;
}

static u32 _atrSystemCloseout(
    da_day_summary_t * buffer, olint_t num, da_atr_param_t * param,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
    da_atr_t * atr; //, * atrp;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    _newAtr(buffer, num, param);
    atr = end->dds_pdaAtr;
//    atrp = (end - 1)->dds_pdaAtr;
    if (atr->da_dbAtr < atr->da_dbMaAtr)
    {
        conc->dc_nAction = CONC_ACTION_BULL_CLOSEOUT;
        conc->dc_nPosition = CONC_POSITION_SHORT;
        conc->dc_dbYield =
            (end->dds_dbClosingPrice - conc->dc_dbPrice) * 100 / conc->dc_dbPrice;
        /*the yield minus 1%, as other charges*/
        conc->dc_dbYield -= 1.0;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsCloseout = end;
        conc->dc_nHoldDays = conc->dc_pddsCloseout - conc->dc_pddsOpening;
    }

    return u32Ret;
}

static u32 _atrSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_atr_param_t * param = &pdip->dip_dapAtr;
    olint_t nMinAtrDaySummary;

    if (param->dap_nAtrDays == 0)
        param->dap_nAtrDays = DEF_ATR_DAYS;
    if (param->dap_nAtrMaDays == 0)
        param->dap_nAtrMaDays = DEF_ATR_MA_DAYS;
    nMinAtrDaySummary = MAX(param->dap_nAtrDays, param->dap_nAtrMaDays);
    nMinAtrDaySummary = (nMinAtrDaySummary + 1) * 3;

    if (num < nMinAtrDaySummary)
        return JF_ERR_NOT_READY;

    if (conc->dc_nPosition == CONC_POSITION_SHORT)
        u32Ret = _atrSystemOpening(buffer, num, param, conc);
    else //if (! _isStopLoss(buffer + num - 1, conc))
        u32Ret = _atrSystemCloseout(buffer, num, param, conc);

    return u32Ret;
}

static u32 _optimizeAtrSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, j;
    da_indicator_param_t myparam;
    da_conc_sum_t myconc;

    memset(pdip, 0, sizeof(da_indicator_param_t));
    memset(conc, 0, sizeof(da_conc_sum_t));

    return u32Ret;

#define DEF_ATR_DAYS_MIN       7
#define DEF_ATR_DAYS_MAX      28
#define DEF_ATR_MA_DAYS_MIN    7
#define DEF_ATR_MA_DAYS_MAX   28

    for (i = DEF_ATR_DAYS_MIN; i <= DEF_ATR_DAYS_MAX; i ++)
        for (j = MAX(i, DEF_ATR_MA_DAYS_MIN); j <= DEF_ATR_MA_DAYS_MAX; j ++)
        {
            myparam.dip_dapAtr.dap_nAtrDays = i;
            myparam.dip_dapAtr.dap_nAtrMaDays = j;
            u32Ret = _optIndicatorSystem(indi, &myparam, buffer, num, &myconc);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                if (compareDaConcSum(&myconc, conc) > 0)
                {
                    memcpy(pdip, &myparam, sizeof(da_indicator_param_t));
                    memcpy(conc, &myconc, sizeof(da_conc_sum_t));
                }
            }
            _freeDaDaySummaryIndicator(buffer, num);

            if (u32Ret != JF_ERR_NO_ERROR)
                goto out;
        }
out:
    return u32Ret;
}

static void _printDaAtrParamVerbose(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip)
{
    da_atr_param_t * param = &pdip->dip_dapAtr;
    jf_clieng_caption_t * pcc = &ls_jccAtrParamVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Days*/
    ol_sprintf(strLeft, "%d", param->dap_nAtrDays);
    ol_sprintf(strRight, "%d", param->dap_nAtrMaDays);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;
}

static void _atrPrintDaySummary(
    struct da_indicator_desc * indi, da_day_summary_t * buffer, olint_t num)
{
    olint_t i;
    olchar_t buf[512];
    da_day_summary_t * cur;
    da_atr_t * atr;

    cur = buffer;
    for (i = 0; i < num; i ++)
    {
        atr = cur->dds_pdaAtr;
        if (atr == NULL)
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f ",
                cur->dds_nIndex, cur->dds_strDate,
                cur->dds_dbHighPrice, cur->dds_dbLowPrice, cur->dds_dbClosingPrice);
        }
        else
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f "
                "%.3f %.3f",
                cur->dds_nIndex, cur->dds_strDate,
                cur->dds_dbHighPrice, cur->dds_dbLowPrice, cur->dds_dbClosingPrice,
                atr->da_dbAtr, atr->da_dbMaAtr);
        }
        jf_clieng_outputLine("%s", buf);
        cur ++;
    }

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

static void _newObv(
    da_day_summary_t * buffer, olint_t num, da_obv_param_t * param)
{
    da_day_summary_t * end, * prev;
    da_obv_t * obv;

    end = buffer + num - 1;
    if (end->dds_pdoObv != NULL)
        return;

    jf_jiukun_allocMemory(
        (void **)&end->dds_pdoObv, sizeof(da_obv_t),
        JF_FLAG_MASK(JF_JIUKUN_MEM_ALLOC_FLAG_ZERO));

    obv = end->dds_pdoObv;
    prev = end - 1;
    if (prev->dds_pdoObv == NULL)
    {
        if (end->dds_dbClosingPrice > prev->dds_dbClosingPrice)
            obv->do_dbObv = (oldouble_t)prev->dds_u64All + (oldouble_t)end->dds_u64All;
        else if (end->dds_dbClosingPrice < prev->dds_dbClosingPrice)
            obv->do_dbObv = (oldouble_t)prev->dds_u64All - (oldouble_t)end->dds_u64All;
        else
            obv->do_dbObv = (oldouble_t)prev->dds_u64All;
    }
    else
    {
        if (end->dds_dbClosingPrice > prev->dds_dbClosingPrice)
            obv->do_dbObv = prev->dds_pdoObv->do_dbObv + (oldouble_t)end->dds_u64All;
        else if (end->dds_dbClosingPrice < prev->dds_dbClosingPrice)
            obv->do_dbObv = prev->dds_pdoObv->do_dbObv - (oldouble_t)end->dds_u64All;
        else
            obv->do_dbObv = prev->dds_pdoObv->do_dbObv;
    }
}

static u32 _obvSystemOpening(
    da_day_summary_t * buffer, olint_t num, da_obv_param_t * param,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
//    da_obv_t * obv, * obvp;
    olint_t i;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    for (i = param->dop_nObvDays; i <= num; i ++)
        _newObv(buffer, i, param);
//    obv = end->dds_pdoObv;
//    obvp = (end - 1)->dds_pdoObv;
    if (0)
    {
        conc->dc_nAction = CONC_ACTION_BULL_OPENING;
        conc->dc_nPosition = CONC_POSITION_FULL;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsOpening = end;
    }

    return u32Ret;
}

static u32 _obvSystemCloseout(
    da_day_summary_t * buffer, olint_t num, da_obv_param_t * param,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end;
//    da_obv_t * obv;

    conc->dc_nAction = CONC_ACTION_NONE;

    end = buffer + num - 1;
    _newObv(buffer, num, param);
//    obv = end->dds_pdoObv;
    if (0)
    {
        conc->dc_nAction = CONC_ACTION_BULL_CLOSEOUT;
        conc->dc_nPosition = CONC_POSITION_SHORT;
        conc->dc_dbYield =
            (end->dds_dbClosingPrice - conc->dc_dbPrice) * 100 / conc->dc_dbPrice;
        /*the yield minus 1%, as other charges*/
        conc->dc_dbYield -= 1.0;
        conc->dc_dbPrice = end->dds_dbClosingPrice;
        conc->dc_pddsCloseout = end;
        conc->dc_nHoldDays = conc->dc_pddsCloseout - conc->dc_pddsOpening;
    }

    return u32Ret;
}

static u32 _obvSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_obv_param_t * param = &pdip->dip_dopObv;
    olint_t nMinObvDaySummary;

    if (param->dop_nObvDays == 0)
        param->dop_nObvDays = DEF_OBV_DAYS;
    nMinObvDaySummary = param->dop_nObvDays + 1;

    if (num < nMinObvDaySummary)
        return JF_ERR_NOT_READY;

    if (conc->dc_nPosition == CONC_POSITION_SHORT)
        u32Ret = _obvSystemOpening(buffer, num, param, conc);
    else // if (! _isStopLoss(today, conc))
        u32Ret = _obvSystemCloseout(buffer, num, param, conc);

    return u32Ret;
}

static u32 _optimizeObvSystem(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip,
    da_day_summary_t * buffer, olint_t num, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    memset(pdip, 0, sizeof(da_indicator_param_t));
    memset(conc, 0, sizeof(da_conc_sum_t));

    return u32Ret;
}

static void _printDaObvParamVerbose(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip)
{
    da_obv_param_t * param = &pdip->dip_dopObv;
    jf_clieng_caption_t * pcc = &ls_jccObvParamVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN]; //, strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Days*/
    ol_sprintf(strLeft, "%d", param->dop_nObvDays);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;
}

static void _obvPrintDaySummary(
    struct da_indicator_desc * indi, da_day_summary_t * buffer, olint_t num)
{
    olint_t i;
    olchar_t buf[512];
    da_day_summary_t * cur;
    da_obv_t * obv;

    cur = buffer;
    for (i = 0; i < num; i ++)
    {
        obv = cur->dds_pdoObv;
        if (obv == NULL)
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f ",
                cur->dds_nIndex, cur->dds_strDate,
                cur->dds_dbHighPrice, cur->dds_dbLowPrice, cur->dds_dbClosingPrice);
        }
        else
        {
            ol_sprintf(
                buf, "%3d %s %.2f %.2f %.2f "
                "%lld %.2f",
                cur->dds_nIndex, cur->dds_strDate,
                cur->dds_dbHighPrice, cur->dds_dbLowPrice, cur->dds_dbClosingPrice,
                cur->dds_u64All, obv->do_dbObv);
        }
        jf_clieng_outputLine("%s", buf);
        cur ++;
    }

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

static olchar_t * _getStringIndiType(olint_t type)
{
    if (type == INDI_TYPE_TREND)
        return "Trend";
    else if (type == INDI_TYPE_ANTI_TREND)
        return "Anti Trend";
    else
        return "unknown";
}

static void _printIndicatorDescVerbose(struct da_indicator_desc * indi)
{
    jf_clieng_caption_t * pcc = &ls_jccDaIndicatorDescVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN]; //, strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Id*/
    ol_sprintf(strLeft, "%d", indi->did_nId);
    jf_clieng_printTwoHalfLine(pcc, strLeft, indi->did_pstrName);
    pcc += 2;

    /*Desc*/
    jf_clieng_printOneFullLine(pcc, indi->did_pstrDesc);
    pcc += 1;

    /*type*/
    ol_sprintf(strLeft, "%s", _getStringIndiType(indi->did_nType));
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;
}

static da_indicator_desc_t ls_didIndicatorDesc[] =
{
    {STOCK_INDICATOR_DMI, INDI_TYPE_TREND, "DMI", "Directional Movement Index",
     _dmiSystem, _optimizeDmiSystem, _printDaDmiParamVerbose, _dmiPrintDaySummary,
     _dmiGetStringIndicatorParam, _dmiGetIndicatorParamFromString,
     _printIndicatorDescVerbose},
    {STOCK_INDICATOR_MACD, INDI_TYPE_TREND, "MACD", "Moving Average Convergence / Divergence",
     _macdSystem, _optimizeMacdSystem, _printDaMacdParamVerbose, _macdPrintDaySummary,
     _macdGetStringIndicatorParam, _macdGetIndicatorParamFromString,
     _printIndicatorDescVerbose},
    {STOCK_INDICATOR_MTM, INDI_TYPE_UNKNOWN, "MTM", "Momentum Index",
     _mtmSystem, _optimizeMtmSystem, _printDaMtmParamVerbose, _mtmPrintDaySummary,
     _mtmGetStringIndicatorParam, _mtmGetIndicatorParamFromString,
     _printIndicatorDescVerbose},
    {STOCK_INDICATOR_KDJ, INDI_TYPE_ANTI_TREND, "KDJ", "KDJ",
     _kdjSystem, _optimizeKdjSystem, _printDaKdjParamVerbose, _kdjPrintDaySummary,
     _kdjGetStringIndicatorParam, _kdjGetIndicatorParamFromString,
     _printIndicatorDescVerbose},
    {STOCK_INDICATOR_RSI, INDI_TYPE_UNKNOWN, "RSI", "Relative Strength Index",
     _rsiSystem, _optimizeRsiSystem, _printDaRsiParamVerbose, _rsiPrintDaySummary,
     _rsiGetStringIndicatorParam, _rsiGetIndicatorParamFromString,
     _printIndicatorDescVerbose},
    {STOCK_INDICATOR_ASI, INDI_TYPE_UNKNOWN, "ASI", "Accumulation Swing Index",
     _asiSystem, _optimizeAsiSystem, _printDaAsiParamVerbose, _asiPrintDaySummary,
     _asiGetStringIndicatorParam, _asiGetIndicatorParamFromString,
     _printIndicatorDescVerbose},
    {STOCK_INDICATOR_ATR, INDI_TYPE_UNKNOWN, "ATR", "Average True Range",
     _atrSystem, _optimizeAtrSystem, _printDaAtrParamVerbose, _atrPrintDaySummary,
     _atrGetStringIndicatorParam, _atrGetIndicatorParamFromString,
     _printIndicatorDescVerbose},
    {STOCK_INDICATOR_OBV, INDI_TYPE_UNKNOWN, "OBV", "On Balance Volume",
     _obvSystem, _optimizeObvSystem, _printDaObvParamVerbose, _obvPrintDaySummary,
     _obvGetStringIndicatorParam, _obvGetIndicatorParamFromString,
     _printIndicatorDescVerbose},
};

/* --- public routine section ---------------------------------------------- */

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

u32 getOptimizedIndicator(
    da_day_summary_t * buffer, olint_t num, get_optimized_indicator_param_t * pgoip,
    olint_t * pnId, da_indicator_param_t * pdip, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_indicator_param_t myparam;
    da_conc_sum_t myconc;
    olint_t id, i;
    da_indicator_desc_t * indi;
    olchar_t buf[512];

    if (num < TOTAL_OPTIMIZE_INDI_DAY_SUMMARY + OPTIMIZE_INDI_DAY_SUMMARY)
        return JF_ERR_NOT_READY;

    for (i = num - OPTIMIZE_INDI_DAY_SUMMARY + 1; i <= num; i ++)
        _caluAdxr(buffer, i);

    memset(pdip, 0, sizeof(da_indicator_param_t));
    memset(conc, 0, sizeof(da_conc_sum_t));

    for (id = STOCK_INDICATOR_UNKNOWN + 1; id < STOCK_INDICATOR_MAX; id ++)
    {
        indi = getDaIndicatorDesc(id);
        u32Ret = _isMatchingIndi(indi, buffer, num);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = indi->did_fnOptimize(indi, &myparam, buffer, num, &myconc);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (pgoip->goip_bVerbose)
            {
                jf_clieng_printBanner(JF_CLIENG_MAX_OUTPUT_LINE_LEN);
                indi->did_fnGetStringParam(indi, &myparam, buf);
                jf_clieng_outputLine("%s System, %s", indi->did_pstrName, buf);
                indi->did_fnPrintParamVerbose(indi, &myparam);
                printDaConcSumVerbose(&myconc);
                jf_clieng_outputLine("");
            }

            if (compareDaConcSum(&myconc, conc) > 0)
            {
                memcpy(pdip, &myparam, sizeof(da_indicator_param_t));
                memcpy(conc, &myconc, sizeof(da_conc_sum_t));
                *pnId = id;
            }
        }
    }

    freeDaDaySummaryIndicator(buffer, num);
    u32Ret = JF_ERR_NO_ERROR;

    if ((conc->dcs_dbOverallYield <= 0) ||
        (conc->dcs_dbProfitTimeRatio < pgoip->goip_dbMinProfitTimeRatio))
        u32Ret = JF_ERR_NOT_FOUND;

    return u32Ret;
}

u32 getIndicatorAdxrTrend(da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _caluAdxr(buffer, num);

    return u32Ret;
}

char * getStringKcp(olint_t nKcp)
{
    if (nKcp == KCP_TREND)
        return "Trend";
    else if (nKcp == KCP_ANTI_TREND)
        return "Anti Trend";
    else
        return "Unknown";
}

char * getAbbrevStringKcp(olint_t nKcp)
{
    if (nKcp == KCP_TREND)
        return "T";
    else if (nKcp == KCP_ANTI_TREND)
        return "AT";
    else
        return "N/A";
}

char * getStringIndicatorName(olint_t nIndicator)
{
    if ((nIndicator == 0) || (nIndicator >= STOCK_INDICATOR_MAX))
        return "Unknown";

    return ls_pstrIndicator[nIndicator];
}

/*---------------------------------------------------------------------------*/


