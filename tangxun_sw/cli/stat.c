/**
 *  @file stat.c
 *
 *  @brief The stat command implementation.
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
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "jf_clieng.h"
#include "jf_matrix.h"
#include "jf_jiukun.h"

#include "tx_datastat.h"
#include "tx_regression.h"
#include "tx_statarbi.h"
#include "tx_env.h"
#include "tx_daysummary.h"

#include "clicmd.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define STAT_ARBI_STOCK_FILE   "StatArbiStocks.txt"

#define MAX_NUM_OF_RESULT      (300)

#define DEF_DAY_FOR_TREND      (60)

static jf_clieng_caption_t ls_ccDaySummaryBrief[] =
{
    {"Day", 5}, 
    {"Date", 12},
    {"BuyA", 10},
    {"SoldA", 10},
    {"LambBuyA", 10},
    {"LambSoldA", 10},
    {"Close", 6},
};

static jf_clieng_caption_t ls_jccDataStatVerbose[] =
{
    {"Name", JF_CLIENG_CAP_HALF_LINE},
    {"NumOfTradingDay", JF_CLIENG_CAP_HALF_LINE}, {"DayForTrend", JF_CLIENG_CAP_HALF_LINE},
    {"StartDate", JF_CLIENG_CAP_HALF_LINE}, {"EndDate", JF_CLIENG_CAP_HALF_LINE},

    {"MinClosingPriceRate", JF_CLIENG_CAP_HALF_LINE}, {"MaxClosingPriceRate", JF_CLIENG_CAP_HALF_LINE},
    {"MinTunePercent", JF_CLIENG_CAP_HALF_LINE}, {"MaxTunePercent", JF_CLIENG_CAP_HALF_LINE},
    {"CloseHighLimit", JF_CLIENG_CAP_HALF_LINE}, {"LastTimeCloseHighLimit", JF_CLIENG_CAP_HALF_LINE},
    {"MaxTimeOpenCloseHighLimit", JF_CLIENG_CAP_FULL_LINE},

    {"MinBuyPercent", JF_CLIENG_CAP_HALF_LINE}, {"MaxBuyPercent", JF_CLIENG_CAP_HALF_LINE},
    {"AveBuyPercent", JF_CLIENG_CAP_HALF_LINE}, {"MinAveBuyPercentForTrend", JF_CLIENG_CAP_HALF_LINE},
    {"MinSoldPercent", JF_CLIENG_CAP_HALF_LINE}, {"MaxSoldPercent", JF_CLIENG_CAP_HALF_LINE},
    {"AveSoldPercent", JF_CLIENG_CAP_FULL_LINE},
    {"MinLambBuyPercent", JF_CLIENG_CAP_HALF_LINE}, {"MaxLambBuyPercent", JF_CLIENG_CAP_HALF_LINE},
    {"AveLambBuyPercent", JF_CLIENG_CAP_FULL_LINE},
    {"MinLambSoldPercent", JF_CLIENG_CAP_HALF_LINE}, {"MaxLambSoldPercent", JF_CLIENG_CAP_HALF_LINE},
    {"AveLambSoldPercent", JF_CLIENG_CAP_FULL_LINE},
    {"BuyInAm", JF_CLIENG_CAP_FULL_LINE},
    {"MaxVolumeRatio", JF_CLIENG_CAP_FULL_LINE},
    {"MaxSoldVolumeRatio", JF_CLIENG_CAP_HALF_LINE}, {"MaxLambSoldVolumeRatio", JF_CLIENG_CAP_HALF_LINE},
    {"MaxCloseHighLimitVolumeRatio", JF_CLIENG_CAP_FULL_LINE},
    {"MaxCloseHighLimitSoldVolumeRatio", JF_CLIENG_CAP_HALF_LINE}, {"MaxCloseHighLimitLambSoldVolumeRatio", JF_CLIENG_CAP_HALF_LINE},

    {"MinBuyA", JF_CLIENG_CAP_HALF_LINE}, {"MaxBuyA", JF_CLIENG_CAP_HALF_LINE},
    {"AveBuyA", JF_CLIENG_CAP_FULL_LINE},
    {"MinSoldA", JF_CLIENG_CAP_HALF_LINE}, {"MaxSoldA", JF_CLIENG_CAP_HALF_LINE},
    {"AveSoldA", JF_CLIENG_CAP_FULL_LINE},
    {"MinLambBuyA", JF_CLIENG_CAP_HALF_LINE}, {"MaxLambBuyA", JF_CLIENG_CAP_HALF_LINE},
    {"AveLambBuyA", JF_CLIENG_CAP_FULL_LINE},
    {"MinLambSoldA", JF_CLIENG_CAP_HALF_LINE}, {"MaxLambSoldA", JF_CLIENG_CAP_HALF_LINE},
    {"AveLambSoldA", JF_CLIENG_CAP_FULL_LINE},

    {"MaxUpperShadowRatio", JF_CLIENG_CAP_HALF_LINE}, {"MaxLowerShadowRatio", JF_CLIENG_CAP_HALF_LINE},

    {"MaxLastLowPriceInc", JF_CLIENG_CAP_HALF_LINE}, {"LastTimeOfLowPrice", JF_CLIENG_CAP_HALF_LINE},
};

static jf_clieng_caption_t ls_jccDescStatVerbose[] =
{
    {"Count", JF_CLIENG_CAP_FULL_LINE},
    {"All", JF_CLIENG_CAP_FULL_LINE},
    {"Mean", JF_CLIENG_CAP_HALF_LINE}, {"SEMean", JF_CLIENG_CAP_HALF_LINE},

    {"Variance", JF_CLIENG_CAP_HALF_LINE}, {"StDev", JF_CLIENG_CAP_HALF_LINE},

    {"Min", JF_CLIENG_CAP_HALF_LINE}, {"Q1", JF_CLIENG_CAP_HALF_LINE},
    {"Median", JF_CLIENG_CAP_HALF_LINE}, {"Q3", JF_CLIENG_CAP_HALF_LINE},
    {"Max", JF_CLIENG_CAP_FULL_LINE},

    {"StDev1Percent", JF_CLIENG_CAP_HALF_LINE}, {"StDev2Percent", JF_CLIENG_CAP_HALF_LINE},
};

static jf_clieng_caption_t ls_jccFrequencyBrief[] =
{
    {"Unit", 6},
    {"Region", 24},
    {"Freq", 6},

};

/* --- private routine section ------------------------------------------------------------------ */
static u32 _statHelp(tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Statistic analysis of stock\n\
stat [-v]\n\
  [Inflexion point options] [Descriptive stat options] [Stat arbitrage options]");
    jf_clieng_outputRawLine2("\
  -l: parse last N days.\n\
  -v: verbose.");
    jf_clieng_outputRawLine2("\
Inflexion point options.\n\
stat [-i stock] [-n stock]\n\
  -i: show inflexion point.\n\
  -n: show inflexion point version 2.");
    jf_clieng_outputRawLine2("\
Descriptive statistic options.\n\
stat [-c] [-s stock]\n\
  -c: descriptive statistic.\n\
  -s: statistic of amount.\n");
    jf_clieng_outputRawLine2("\
Statistical arbitrage options.\n\
stat [-a industry-id] [-f stock-pair]\n\
  -a: statistical arbitrage for an industry, if \"all\" is specified, for all\n\
      industries.\n\
  -f: find the best stat arbitrage method for the stock.");
    jf_clieng_outputLine("");

    return u32Ret;
}

void printDoubleFrequencyBrief(olint_t numofarea, olint_t * freq, oldouble_t * area)
{
    jf_clieng_caption_t * pcc = &ls_jccFrequencyBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];
    olint_t i;

    jf_clieng_printHeader(ls_jccFrequencyBrief,
        sizeof(ls_jccFrequencyBrief) / sizeof(jf_clieng_caption_t));

    for (i = 0; i < numofarea - 1; i ++)
    {
        strInfo[0] = '\0';
        pcc = &ls_jccFrequencyBrief[0];

        /* Unit */
        ol_sprintf(strField, "%d", i);
        jf_clieng_appendBriefColumn(pcc, strInfo, strField);
        pcc++;

        ol_sprintf(strField, "%.1f ~ %.1f", area[i], area[i + 1]);
        jf_clieng_appendBriefColumn(pcc, strInfo, strField);
        pcc++;

        ol_sprintf(strField, "%d", freq[i]);
        jf_clieng_appendBriefColumn(pcc, strInfo, strField);
        pcc++;

        jf_clieng_outputLine(strInfo);
    }

    strInfo[0] = '\0';
    pcc = &ls_jccFrequencyBrief[0];

    ol_sprintf(strField, "%d", i);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    ol_sprintf(strField, "%.1f", area[i]);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    ol_sprintf(strField, "%d", freq[i]);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printDescStatVerbose(tx_datastat_desc_t * stat)
{
    jf_clieng_caption_t * pcc = &ls_jccDescStatVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    /* Count */
    ol_sprintf(strLeft, "%d", stat->tdd_nCount);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* All */
    ol_sprintf(strLeft, "%.3f", stat->tdd_dbAll);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* Mean */
    ol_sprintf(strLeft, "%.3f", stat->tdd_dbMean);
    ol_sprintf(strRight, "%.3f", stat->tdd_dbSEMean);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* Variance */
    ol_sprintf(strLeft, "%.3f", stat->tdd_dbVariance);
    ol_sprintf(strRight, "%.3f", stat->tdd_dbStDev);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* Min */
    ol_sprintf(strLeft, "%.3f", stat->tdd_dbMin);
    ol_sprintf(strRight, "%.3f", stat->tdd_dbQ1);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* Median */
    ol_sprintf(strLeft, "%.3f", stat->tdd_dbMedian);
    ol_sprintf(strRight, "%.3f", stat->tdd_dbQ3);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* Max */
    ol_sprintf(strLeft, "%.3f", stat->tdd_dbMax);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* StDev1Percent */
    ol_sprintf(strLeft, "%.3f", stat->tdd_dbStDev1Percent);
    ol_sprintf(strRight, "%.3f", stat->tdd_dbStDev2Percent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 1;

    jf_clieng_outputLine("");
}

static void _printDataStatVerbose(tx_datastat_daysummary_t * stat)
{
    jf_clieng_caption_t * pcc = &ls_jccDataStatVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    /* Name */
    jf_clieng_printOneFullLine(pcc, stat->tdd_strName);
    pcc += 1;

    /* NumOfTradingDay */
    ol_sprintf(strLeft, "%d", stat->tdd_nNumOfDay);
    ol_sprintf(strRight, "%d", stat->tdd_nDayForTrend);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* StartDate */
    jf_clieng_printTwoHalfLine(pcc, stat->tdd_strStartDate, stat->tdd_strEndDate);
    pcc += 2;

    /* MinClosingPriceRate */
    ol_sprintf(strLeft, "%.2f%%", stat->tdd_dbMinClosingPriceRate);
    ol_sprintf(strRight, "%.2f%%", stat->tdd_dbMaxClosingPriceRate);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* MinTunePercent */
    ol_sprintf(strLeft, "%.2f%%", stat->tdd_dbMinTunePercent);
    ol_sprintf(strRight, "%.2f%%", stat->tdd_dbMaxTunePercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* CloseHighLimit*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(stat->tdd_bCloseHighLimit));
    jf_clieng_printTwoHalfLine(pcc, strLeft, stat->tdd_strLastTimeCloseHighLimit);
    pcc += 2;

    /* MaxTimeOpenCloseHighLimit */
    ol_sprintf(strLeft, "%d", stat->tdd_nMaxTimeOpenCloseHighLimit);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MinBuyPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->tdd_dbMinBuyPercent);
    ol_sprintf(strRight, "%.2f%%", stat->tdd_dbMaxBuyPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveBuyPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->tdd_dbAveBuyPercent);
    ol_sprintf(strRight, "%.2f%%", stat->tdd_dbMinAveBuyPercentForTrend);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* MinSoldPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->tdd_dbMinSoldPercent);
    ol_sprintf(strRight, "%.2f%%", stat->tdd_dbMaxSoldPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveSoldPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->tdd_dbAveSoldPercent);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MinLambBuyPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->tdd_dbMinLambBuyPercent);
    ol_sprintf(strRight, "%.2f%%", stat->tdd_dbMaxLambBuyPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveLambBuyPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->tdd_dbAveLambBuyPercent);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MinLambSoldPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->tdd_dbMinLambSoldPercent);
    ol_sprintf(strRight, "%.2f%%", stat->tdd_dbMaxLambSoldPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveLambSoldPercent */
    ol_sprintf(strLeft, "%.2f%%", stat->tdd_dbAveLambSoldPercent);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* BuyInAm */
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(stat->tdd_bBuyInAm));
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MaxVolumeRatio */
    ol_sprintf(strLeft, "%.2f", stat->tdd_dbMaxVolumeRatio);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MaxSoldVolumeRatio */
    ol_sprintf(strLeft, "%.2f", stat->tdd_dbMaxSoldVolumeRatio);
    ol_sprintf(strRight, "%.2f", stat->tdd_dbMaxLambSoldVolumeRatio);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* MaxCloseHighLimitVolumeRatio */
    ol_sprintf(strLeft, "%.2f", stat->tdd_dbMaxCloseHighLimitVolumeRatio);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MaxCloseHighLimitSoldVolumeRatio */
    ol_sprintf(strLeft, "%.2f", stat->tdd_dbMaxCloseHighLimitSoldVolumeRatio);
    ol_sprintf(strRight, "%.2f", stat->tdd_dbMaxCloseHighLimitLambSoldVolumeRatio);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* MinBuyA */
    ol_sprintf(strLeft, "%llu", stat->tdd_u64MinBuyA);
    ol_sprintf(strRight, "%llu", stat->tdd_u64MaxBuyA);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveBuyA */
    ol_sprintf(strLeft, "%llu", stat->tdd_u64AveBuyA);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MinSoldA */
    ol_sprintf(strLeft, "%llu", stat->tdd_u64MinSoldA);
    ol_sprintf(strRight, "%llu", stat->tdd_u64MaxSoldA);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveSoldA */
    ol_sprintf(strLeft, "%llu", stat->tdd_u64AveSoldA);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MinLambBuyA */
    ol_sprintf(strLeft, "%llu", stat->tdd_u64MinLambBuyA);
    ol_sprintf(strRight, "%llu", stat->tdd_u64MaxLambBuyA);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveLambBuyA */
    ol_sprintf(strLeft, "%llu", stat->tdd_u64AveLambBuyA);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MinLambSoldA */
    ol_sprintf(strLeft, "%llu", stat->tdd_u64MinLambSoldA);
    ol_sprintf(strRight, "%llu", stat->tdd_u64MaxLambSoldA);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* AveLambSoldA */
    ol_sprintf(strLeft, "%llu", stat->tdd_u64AveLambSoldA);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /* MaxUpperShadowRatio */
    ol_sprintf(strLeft, "%.2f", stat->tdd_dbMaxUpperShadowRatio);
    ol_sprintf(strRight, "%.2f", stat->tdd_dbMaxLowerShadowRatio);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* MaxLastXInc */
    ol_sprintf(strLeft, "%.2f%%", stat->tdd_dbMaxLastXInc);
    ol_sprintf(strRight, "%s %s",
         stat->tdd_strLastTimeLowPriceDate, stat->tdd_strLastTimeLowPrice);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    jf_clieng_outputLine("");
}

static void _printFrequencyBrief(oldouble_t * buffer, olint_t num)
{
#define MAX_AREAS 10
    olint_t count[MAX_AREAS + 1];
    oldouble_t area[MAX_AREAS + 1];

    tx_datastat_getDoubleFrequency(buffer, num, MAX_AREAS + 1, count, area);

    printDoubleFrequencyBrief(MAX_AREAS + 1, count, area);

    jf_clieng_outputLine("");
}

static u32 _statBuy(tx_ds_t * buffer, olint_t num, tx_datastat_daysummary_t * stat)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t *cur = buffer;
    oldouble_t * pdbbuy;
    olint_t i;
    tx_datastat_desc_t descstat;

    u32Ret = jf_mem_alloc((void **)&pdbbuy, sizeof(oldouble_t) * num);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    for (i = 0; i < num; i++)
    {
        pdbbuy[i] = cur->td_ptdResult->td_u64BuyA;
        cur ++;
    }

    u32Ret = tx_datastat_descData(&descstat, pdbbuy, num);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_clieng_printDivider();
        jf_clieng_outputLine("Statistic of Amount of Buy\n");

        _printDescStatVerbose(&descstat);

        _printFrequencyBrief(pdbbuy, num);
    }

    jf_mem_free((void **)&pdbbuy);

    return u32Ret;
}

static u32 _raClose(
    cli_stat_param_t * pcsp, 
    tx_ds_t * buffer, olint_t num, tx_datastat_daysummary_t * stat)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t *cur;
    oldouble_t * pdbclose = NULL, *pdbx[4];
//    oldouble_t dbc;
    olint_t i, numra;
    olint_t countx = 4;
    tx_regression_result_t raresult;
    tx_regression_result_coef_t trrccoef[4];
    tx_regression_result_unusual_observ_t observ[5];
    olchar_t * rname = "Close";
    olchar_t * pname[4] = {"BuyA", "SoldA", "LambBuyA", "LambSoldA"};

    ol_memset(pdbx, 0, sizeof(pdbx));

    ol_memset(&raresult, 0, sizeof(raresult));
    ol_memset(trrccoef, 0, sizeof(trrccoef));
    ol_memset(observ, 0, sizeof(observ));
    raresult.trr_ptrrcCoef = trrccoef;
    raresult.trr_nMaxObserv = 5;
    raresult.trr_ptrruoObserv = observ;

    u32Ret = jf_mem_alloc((void **)&pdbclose, sizeof(oldouble_t) * num);
    if (u32Ret != JF_ERR_NO_ERROR)
        goto out;

    for (i = 0; i < countx; i ++)
    {
        u32Ret = jf_mem_alloc((void **)&pdbx[i], sizeof(oldouble_t) * num);
        if (u32Ret != JF_ERR_NO_ERROR)
            goto out;
    }

    numra = 0;
    cur = buffer + 1;
    for (i = 0; i < num - 1; i ++)
    {
        if (cur->td_bCloseHighLimit || cur->td_bCloseLowLimit)
        {
            cur ++;
            continue;
        }

        pdbclose[numra] = cur->td_dbClosingPriceRate;
        pdbx[0][numra] = cur->td_ptdResult->td_u64BuyA;
        pdbx[1][numra] = cur->td_ptdResult->td_u64SoldA;
        pdbx[2][numra] = cur->td_ptdResult->td_u64LambBuyA;
        pdbx[3][numra] = cur->td_ptdResult->td_u64LambSoldA;

        numra ++;
        cur ++;
    }

    u32Ret = tx_regression_analysis(
        rname, pdbclose, pname, pdbx, countx, numra, &raresult);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_clieng_printDivider();
        tx_regression_printResult(&raresult);
    }

out:
    if (pdbclose != NULL)
        jf_mem_free((void **)&pdbclose);

    if (pdbx != NULL)
    {
        for (i = 0; i < countx; i ++)
        {
            if (pdbx[i] != NULL)
                jf_mem_free((void **)&pdbx[i]);
        }
    }

    return u32Ret;
}

static void _dataStat(
    cli_stat_param_t * pcsp, tx_ds_t * buffer, olint_t num, tx_datastat_daysummary_t *stat)
{
    tx_datastat_daysummary_param_t tddp;
    olchar_t statname[16];

    jf_file_getFileName(statname, 16, pcsp->csp_pstrData);
    ol_memset(&tddp, 0, sizeof(tddp));
    tddp.tddp_pstrName = statname;
    tddp.tddp_nDayForTrend = DEF_DAY_FOR_TREND;

    tx_datastat_statDaySummary(&tddp, stat, buffer, num);

    jf_clieng_printDivider();
    _printDataStatVerbose(stat);
}

static void _printDaySummaryBrief(tx_ds_t * summary)
{
    jf_clieng_caption_t * pcc = &ls_ccDaySummaryBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];
	tx_dr_t * cur = summary->td_ptdResult;

    strInfo[0] = '\0';

    /* Day */
    ol_sprintf(strField, "%d", summary->td_nIndex);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Date */
    jf_clieng_appendBriefColumn(pcc, strInfo, summary->td_strDate);
    pcc++;

    /* BuyA */
    ol_sprintf(strField, "%llu", cur->td_u64BuyA);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SoldA */
    ol_sprintf(strField, "%llu", cur->td_u64SoldA);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambBuyA */
    ol_sprintf(strField, "%llu", cur->td_u64LambBuyA);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambSoldA */
    ol_sprintf(strField, "%llu", cur->td_u64LambSoldA);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosePriceInc */
    ol_sprintf(strField, "%.2f", summary->td_dbClosingPriceRate);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printResultBrief(tx_ds_t * buffer, olint_t total)
{
    tx_ds_t * cur;
    olint_t i;

    jf_clieng_printDivider();

    jf_clieng_printHeader(ls_ccDaySummaryBrief,
        sizeof(ls_ccDaySummaryBrief) / sizeof(jf_clieng_caption_t));

    cur = buffer;
    for (i = 0; i < total; i++)
    {
        _printDaySummaryBrief(cur);
        cur ++;
    }
}

static u32 _statAmount(cli_stat_param_t * pcsp, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = MAX_NUM_OF_RESULT;
    tx_ds_t * buffer = NULL;
    tx_datastat_daysummary_t datastat;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];

    u32Ret = jf_mem_alloc((void **)&buffer, sizeof(tx_ds_t) * total);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _dataStat(pcsp, buffer, total, &datastat);

        _printResultBrief(buffer, total);
    }

    /*stat of buy amount*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _statBuy(buffer, total, &datastat);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _raClose(pcsp, buffer, total, &datastat);

    jf_clieng_outputLine("");

    jf_mem_free((void **)&buffer);

    return u32Ret;
}

static u32 _parseDirInflexionPoint(cli_stat_param_t * pcsp, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = 60; //MAX_NUM_OF_RESULT;
    tx_ds_t * buffer = NULL;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_ds_t * fp[100];
    olint_t numfp = 100, i, numfp2;

    if (pcsp->csp_nLastCount != 0)
        total = pcsp->csp_nLastCount;

    u32Ret = jf_mem_alloc((void **)&buffer, sizeof(tx_ds_t) * total);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcsp->csp_pstrData);

    u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        tx_ds_getDsInflexionPoint(buffer, total, fp, &numfp);
        for (i = 0; i < numfp; i ++)
        {
            jf_clieng_outputLine("%s", fp[i]->td_strDate);
        }

        numfp2 = numfp;
        tx_ds_adjustDsInflexionPoint(fp, &numfp, 3);
        jf_clieng_outputLine("\nAfter adjusting (%d -> %d)", numfp2, numfp);
        for (i = 0; i < numfp; i ++)
        {
            jf_clieng_outputLine("%s", fp[i]->td_strDate);
        }
        jf_clieng_outputLine("");
    }

    jf_mem_free((void **)&buffer);

    return u32Ret;
}

static u32 _parseDirInflexionPoint2(cli_stat_param_t * pcsp, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = 20; //MAX_NUM_OF_RESULT;
    tx_ds_t * buffer = NULL;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_ds_t * fp[100];
    olint_t numfp = 100, i;

    if (pcsp->csp_nLastCount != 0)
        total = pcsp->csp_nLastCount;

    u32Ret = jf_mem_alloc((void **)&buffer, sizeof(tx_ds_t) * total);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcsp->csp_pstrData);

    u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        tx_ds_getDsInflexionPoint2(buffer, total, fp, &numfp);
        for (i = 0; i < numfp; i ++)
        {
            jf_clieng_outputLine("%s", fp[i]->td_strDate);
        }
        jf_clieng_outputLine("");
    }

    jf_mem_free((void **)&buffer);

    return u32Ret;
}

static u32 _descStat(cli_stat_param_t * pcsp, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t dbva[10] = {38, 56, 59, 64, 74};
    oldouble_t dbvb[10] = {41, 63, 70, 72, 84};
    tx_datastat_desc_t dsa, dsb;
    oldouble_t dbr;

    u32Ret = tx_datastat_descData(&dsa, dbva, 5);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = tx_datastat_descData(&dsb, dbvb, 5);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = tx_datastat_getCorrelation2(dbva, &dsa, dbvb, &dsb, 5, &dbr);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _printDescStatVerbose(&dsa);
        _printDescStatVerbose(&dsb);
        jf_clieng_outputLine("correlation: %.3f", dbr);
    }

    return u32Ret;
}

static u32 _saveStatArbiStocks(
    cli_stat_param_t * pcsp, tx_statarbi_eval_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t filepath[JF_LIMIT_MAX_PATH_LEN];
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    olchar_t buf[32];
    olint_t i;

    if (result->tser_nNumOfPair == 0)
        return u32Ret;

    ol_sprintf(filepath, "%s", STAT_ARBI_STOCK_FILE);

    u32Ret = jf_file_openWithMode(
        filepath, O_WRONLY | O_CREAT | O_TRUNC,
        JF_FILE_DEFAULT_CREATE_MODE, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0;
             (i < result->tser_nNumOfPair) && (u32Ret == JF_ERR_NO_ERROR); i ++)
        {
            ol_memset(buf, 0, sizeof(buf));
            ol_strcpy(buf, result->tser_ptsereEntry[i].tsere_strStockPair);
            ol_strcat(buf, "\n");
            u32Ret = jf_file_writen(fd, buf, ol_strlen(buf));
        }

        jf_file_close(&fd);
    }

    return u32Ret;
}

static u32 _statArbiIndustry(cli_stat_param_t * pcsp, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_statarbi_eval_param_t tsep;
    tx_statarbi_eval_result_t result;
    olchar_t buf[2048];
    olint_t nIndustry;
    olint_t i;

    ol_memset(&tsep, 0, sizeof(tx_statarbi_eval_param_t));
    tsep.tsep_dbMinCorrelation = 0.80;
    tsep.tsep_nDaySummary = 50;
    tsep.tsep_nCorrelationArray = 40;

    ol_memset(&result, 0, sizeof(tx_statarbi_eval_result_t));
    result.tser_nMaxPair = sizeof(buf) / sizeof(tx_statarbi_eval_result_entry_t);
    result.tser_ptsereEntry = (tx_statarbi_eval_result_entry_t *)buf;
    buf[0] = '\0';

    if (strcmp(pcsp->csp_pstrIndustry, "all") == 0)
    {
        u32Ret = tx_statarbi_evalAllIndu(
            tx_env_getVar(TX_ENV_VAR_DATA_PATH), &tsep, &result);
    }
    else
    {
        u32Ret = jf_string_getS32FromString(
            pcsp->csp_pstrIndustry, ol_strlen(pcsp->csp_pstrIndustry), &nIndustry);
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = tx_statarbi_evalIndu(
                tx_env_getVar(TX_ENV_VAR_DATA_PATH), nIndustry, &tsep, &result);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0;
             i < result.tser_nNumOfPair; i ++)
        {
            jf_clieng_outputLine(
                "%s", result.tser_ptsereEntry[i].tsere_strStockPair);
        }

        u32Ret = _saveStatArbiStocks(pcsp, &result);
    }

    return u32Ret;
}

static u32 _statArbiFind(cli_stat_param_t * pcsp, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

//	u32Ret = statArbiStockPair(
//		tx_env_getVar(TX_ENV_VAR_DATA_PATH), pcsp->csp_pstrData, NULL);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processStat(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_stat_param_t * pcsp = (cli_stat_param_t *)pParam;
    tx_cli_master_t * ptcm = (tx_cli_master_t *)pMaster;

    if (pcsp->csp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _statHelp(ptcm);
    else if (pcsp->csp_u8Action == CLI_ACTION_INFLEXION_POINT)
        u32Ret = _parseDirInflexionPoint(pcsp, ptcm);
    else if (pcsp->csp_u8Action == CLI_ACTION_INFLEXION_POINT_2)
        u32Ret = _parseDirInflexionPoint2(pcsp, ptcm);
    else if (pcsp->csp_u8Action == CLI_ACTION_DESCRIPTIVE_STAT)
        u32Ret = _descStat(pcsp, ptcm);
    else if (pcsp->csp_u8Action == CLI_ACTION_STAT_AMOUNT)
        u32Ret = _statAmount(pcsp, ptcm);
    else if (pcsp->csp_u8Action == CLI_ACTION_STAT_ARBI_INDUSTRY)
        u32Ret = _statArbiIndustry(pcsp, ptcm);
	else if (pcsp->csp_u8Action == CLI_ACTION_STAT_ARBI_FIND)
		u32Ret = _statArbiFind(pcsp, ptcm);
    else
        u32Ret = JF_ERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamStat(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_stat_param_t * pcsp = (cli_stat_param_t *)pParam;

    ol_memset(pcsp, 0, sizeof(cli_stat_param_t));

    return u32Ret;
}

u32 parseStat(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_stat_param_t * pcsp = (cli_stat_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "i:n:l:a:s:f:chv?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'c':
            pcsp->csp_u8Action = CLI_ACTION_DESCRIPTIVE_STAT;
            break;		
        case 'a':
            pcsp->csp_u8Action = CLI_ACTION_STAT_ARBI_INDUSTRY;
            pcsp->csp_pstrIndustry = (olchar_t *)optarg;
            break;		
        case 'f':
            pcsp->csp_u8Action = CLI_ACTION_STAT_ARBI_FIND;
			pcsp->csp_pstrData = (olchar_t *)optarg;
            break;		
        case 'i':
            pcsp->csp_u8Action = CLI_ACTION_INFLEXION_POINT;
            pcsp->csp_pstrData = (olchar_t *)optarg;
            break;		
        case 's':
            pcsp->csp_u8Action = CLI_ACTION_STAT_AMOUNT;
            pcsp->csp_pstrData = (olchar_t *)optarg;
            break;		
        case 'n':
            pcsp->csp_u8Action = CLI_ACTION_INFLEXION_POINT_2;
            pcsp->csp_pstrData = (olchar_t *)optarg;
            break;		
        case 'l':
            u32Ret = jf_string_getS32FromString(
                optarg, ol_strlen(optarg), &pcsp->csp_nLastCount);
            if ((u32Ret == JF_ERR_NO_ERROR) && (pcsp->csp_nLastCount <= 0))
            {
                jf_clieng_reportInvalidOpt('l');
                u32Ret = JF_ERR_INVALID_PARAM;
            }
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case 'v':
            pcsp->csp_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcsp->csp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


