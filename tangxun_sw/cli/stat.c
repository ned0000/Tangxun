/**
 *  @file stat.c
 *
 *  @brief Stat command
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
#include "clicmd.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "parsedata.h"
#include "datastat.h"
#include "jf_clieng.h"
#include "regression.h"
#include "damodel.h"
#include "stocklist.h"
#include "jf_matrix.h"
#include "jf_jiukun.h"
#include "statarbitrage.h"
#include "envvar.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define STAT_ARBI_STOCK_FILE "StatArbiStocks.txt"

#define MAX_NUM_OF_RESULT  300

#if 0
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

static jf_clieng_caption_t ls_ccStatParamVerbose[] =
{
    {"Threshold", JF_CLIENG_CAP_FULL_LINE},
};

static jf_clieng_caption_t ls_ccDaySummaryVolumnBrief[] =
{
    {"Day", 5}, 
    {"Date", 12},
    {"Buy", 16},
    {"Sold", 16},
    {"LambBuy", 10},
    {"LambSold", 10},
    {"Close", 6},
};
#endif

/* --- private routine section ------------------------------------------------------------------ */
static u32 _statHelp(da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Statistic analysis of stock\n\
stat [-i stock] [-v]\n\
     [Markov stat options] [Descriptive stat options] [Stat arbitrage options]");
    jf_clieng_outputRawLine2("\
  -i: show inflexion point.\n\
  -n: show inflexion poolint_t version 2.\n\
  -l: parse last N days.\n\
  -v: verbose.");
    jf_clieng_outputRawLine2("\
Markov statistic options.\n\
stat [-m dir]\n\
  -m: markov statistic.");
    jf_clieng_outputRawLine2("\
Descriptive statistic options.\n\
stat [-c]\n\
  -c: descriptive statistic.");
    jf_clieng_outputRawLine2("\
Statistical arbitrage options.\n\
stat [-a industry-id] [-p] [-f stock-pair]\n\
  -a: statistical arbitrage for an industry, if \"all\" is specified, for all\n\
      industries.\n\
  -f: find the best stat arbitrage method for the stock.\n\
  -p: stat arbitrage for stock pair, the stock pair is read from file.");
    jf_clieng_outputLine("");

    return u32Ret;
}

#if 0
static void _printFrequencyBrief(oldouble_t * buffer, olint_t num)
{
#define MAX_AREAS 10
    olint_t count[MAX_AREAS + 1];
    oldouble_t area[MAX_AREAS + 1];

    getDoubleFrequency(buffer, num, MAX_AREAS + 1, count, area);

    printDoubleFrequencyBrief(MAX_AREAS + 1, count, area);

    jf_clieng_outputLine("");
}

static u32 _statBuy(da_day_summary_t * buffer, olint_t num, data_stat_t * stat)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t *cur = buffer;
    oldouble_t * pdbbuy;
    olint_t i;
    desc_stat_t descstat;

    u32Ret = jf_mem_alloc((void **)&pdbbuy, sizeof(oldouble_t) * num);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    for (i = 0; i < num; i++)
    {
        pdbbuy[i] = cur->dds_ddrResult->ddr_u64BuyA;
        cur ++;
    }

    u32Ret = descStatFromData(&descstat, pdbbuy, num);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_clieng_printDivider();
        jf_clieng_outputLine("Statistic of Amount of Buy\n");

        printDescStatVerbose(&descstat);

        _printFrequencyBrief(pdbbuy, num);
    }

    jf_mem_free((void **)&pdbbuy);

    return u32Ret;
}

static u32 _raClose(
    cli_stat_param_t * pcsp, 
    da_day_summary_t * buffer, olint_t num, data_stat_t * stat)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t *cur;
    oldouble_t * pdbclose = NULL, *pdbx[4];
//    oldouble_t dbc;
    olint_t i, numra;
    olint_t countx = 4;
    ra_result_t raresult;
    ra_result_coef_t rrccoef[4];
    ra_result_unusual_observ_t observ[5];
    olchar_t * rname = "Close";
    olchar_t * pname[4] = {"BuyA", "SoldA", "LambBuyA", "LambSoldA"};

    ol_memset(pdbx, 0, sizeof(pdbx));

    ol_memset(&raresult, 0, sizeof(raresult));
    ol_memset(rrccoef, 0, sizeof(rrccoef));
    ol_memset(observ, 0, sizeof(observ));
    raresult.rr_prrcCoef = rrccoef;
    raresult.rr_nMaxObserv = 5;
    raresult.rr_prruoObserv = observ;

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
        if ((cur->dds_bCloseHighLimit || cur->dds_bCloseLowLimit) &&
            ! pcsp->csp_bIncHighLowLimit)
        {
            cur ++;
            continue;
        }

        pdbclose[numra] = cur->dds_dbClosingPriceRate;
        pdbx[0][numra] = cur->dds_ddrResult->ddr_u64BuyA;
        pdbx[1][numra] = cur->dds_ddrResult->ddr_u64SoldA;
        pdbx[2][numra] = cur->dds_ddrResult->ddr_u64LambBuyA;
        pdbx[3][numra] = cur->dds_ddrResult->ddr_u64LambSoldA;

        numra ++;
        cur ++;
    }

    u32Ret = regressionAnalysis(
        rname, pdbclose, pname, pdbx, countx, numra, &raresult);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_clieng_printDivider();
        printRaResult(&raresult);
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

static void _dataStat(cli_stat_param_t * pcsp,
    da_day_summary_t * buffer, olint_t num, data_stat_t *stat)
{
    data_stat_param_t dsp;
    olchar_t statname[16];

    getFileName(statname, 16, pcsp->csp_pstrData);
    ol_memset(&dsp, 0, sizeof(dsp));
    dsp.dsp_pstrName = statname;
    dsp.dsp_nDayForTrend = DEF_DAY_FOR_TREND;

    dataStatFromDaySummary(&dsp, stat, buffer, num);

    jf_clieng_printDivider();
    printDataStatVerbose(stat);
}

static void _printDaySummaryBrief(da_day_summary_t * summary)
{
    jf_clieng_caption_t * pcc = &ls_ccDaySummaryBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];
	da_day_result_t * cur = summary->dds_ddrResult;

    strInfo[0] = '\0';

    /* Day */
    ol_sprintf(strField, "%d", summary->dds_nIndex);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Date */
    jf_clieng_appendBriefColumn(pcc, strInfo, summary->dds_strDate);
    pcc++;

    /* BuyA */
    ol_sprintf(strField, "%llu", cur->ddr_u64BuyA);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SoldA */
    ol_sprintf(strField, "%llu", cur->ddr_u64SoldA);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambBuyA */
    ol_sprintf(strField, "%llu", cur->ddr_u64LambBuyA);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambSoldA */
    ol_sprintf(strField, "%llu", cur->ddr_u64LambSoldA);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosePriceInc */
    ol_sprintf(strField, "%.2f", summary->dds_dbClosingPriceRate);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printResultBrief(da_day_summary_t * buffer, olint_t total)
{
    da_day_summary_t * cur;
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

static void _printDaySummaryVolumnBrief(da_day_summary_t * summary)
{
    jf_clieng_caption_t * pcc = &ls_ccDaySummaryVolumnBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];
	da_day_result_t * cur = summary->dds_ddrResult;

    strInfo[0] = '\0';

    /* Day */
    ol_sprintf(strField, "%d", summary->dds_nIndex);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Date */
    jf_clieng_appendBriefColumn(pcc, strInfo, summary->dds_strDate);
    pcc++;

    /* Buy */
    ol_sprintf(strField, "%llu", cur->ddr_u64Buy);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Sold */
    ol_sprintf(strField, "%llu", cur->ddr_u64Sold);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambBuy */
    ol_sprintf(strField, "%llu", cur->ddr_u64LambBuy);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambSold */
    ol_sprintf(strField, "%llu", cur->ddr_u64LambSold);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosePriceInc */
    ol_sprintf(strField, "%.2f",
            cur->ddr_dbClosingPriceInc - cur->ddr_dbClosingPriceDec);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printResultVolumnBrief(
    da_day_summary_t * buffer, olint_t total, data_stat_t * datastat,
    matrix_t * mbuy, matrix_t * msold)
{
    jf_clieng_caption_t * pcc = &ls_ccDaySummaryVolumnBrief[0];
    da_day_summary_t * cur;
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];
    olint_t i;

    jf_clieng_printDivider();

    jf_clieng_printHeader(ls_ccDaySummaryVolumnBrief,
        sizeof(ls_ccDaySummaryVolumnBrief) / sizeof(jf_clieng_caption_t));

    cur = buffer;
    for (i = 0; i < total; i++)
    {
        _printDaySummaryVolumnBrief(cur);
        cur ++;
    }

    cur = buffer + total - 1;
    strInfo[0] = '\0';

    /* Day */
    ol_sprintf(strField, "%d", cur->ddr_nIndex + 1);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Date */
    jf_clieng_appendBriefColumn(pcc, strInfo, "Next");
    pcc++;

    /* Buy */
    ol_sprintf(strField, "(%.3f, %.3f)", mbuy->m_pdbData[0], mbuy->m_pdbData[2]);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Sold */
    ol_sprintf(strField, "(%.3f, %.3f)", msold->m_pdbData[0], msold->m_pdbData[2]);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambBuy */
    ol_sprintf(strField, "%llu", datastat->ds_u64AveLambBuy);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambSold */
    ol_sprintf(strField, "%llu", datastat->ds_u64AveLambSold);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosePriceInc */
    ol_sprintf(strField, "%s", "NA");
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printStatParamVerbose(cli_stat_param_t * pcsp)
{
    jf_clieng_caption_t * pcc = &ls_ccStatParamVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN]; //, strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /* Threshold */
    ol_sprintf(strLeft, "%d", pcsp->csp_nThres);
    jf_clieng_printOneFullLine(pcc, strLeft); 
    pcc += 1;

    jf_clieng_outputLine("");
}

static u32 _parseDir(cli_stat_param_t * pcsp, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = MAX_NUM_OF_RESULT;
    da_day_summary_t * buffer = NULL;
    data_stat_t datastat;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];

    _printStatParamVerbose(pcsp);

    u32Ret = jf_mem_alloc((void **)&buffer, sizeof(da_day_summary_t) * total);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
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
#endif

static u32 _parseDirInflexionPoint(cli_stat_param_t * pcsp, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = 60; //MAX_NUM_OF_RESULT;
    da_day_summary_t * buffer = NULL;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    da_day_summary_t * fp[100];
    olint_t numfp = 100, i, numfp2;

    if (pcsp->csp_nLastCount != 0)
        total = pcsp->csp_nLastCount;

    u32Ret = jf_mem_alloc((void **)&buffer, sizeof(da_day_summary_t) * total);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcsp->csp_pstrData);

    u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        getDaySummaryInflexionPoint(buffer, total, fp, &numfp);
        for (i = 0; i < numfp; i ++)
        {
            jf_clieng_outputLine("%s", fp[i]->dds_strDate);
        }

        numfp2 = numfp;
        adjustDaySummaryInflexionPoint(fp, &numfp, 3);
        jf_clieng_outputLine("\nAfter adjusting (%d -> %d)", numfp2, numfp);
        for (i = 0; i < numfp; i ++)
        {
            jf_clieng_outputLine("%s", fp[i]->dds_strDate);
        }
        jf_clieng_outputLine("");
    }

    jf_mem_free((void **)&buffer);

    return u32Ret;
}

static u32 _parseDirInflexionPoint2(cli_stat_param_t * pcsp, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = 20; //MAX_NUM_OF_RESULT;
    da_day_summary_t * buffer = NULL;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    da_day_summary_t * fp[100];
    olint_t numfp = 100, i;

    if (pcsp->csp_nLastCount != 0)
        total = pcsp->csp_nLastCount;

    u32Ret = jf_mem_alloc((void **)&buffer, sizeof(da_day_summary_t) * total);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcsp->csp_pstrData);

    u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        getDaySummaryInflexionPoint2(buffer, total, fp, &numfp);
        for (i = 0; i < numfp; i ++)
        {
            jf_clieng_outputLine("%s", fp[i]->dds_strDate);
        }
        jf_clieng_outputLine("");
    }

    jf_mem_free((void **)&buffer);

    return u32Ret;
}

#if 0
static void _printBuyFrequencyBrief(
    oldouble_t * buffer, olint_t num, data_stat_t * stat)
{
#define MAX_AREAS 10
    olint_t count[MAX_AREAS + 1];
    oldouble_t area[MAX_AREAS + 1];

    getDoubleFrequency(buffer, num, MAX_AREAS + 1, count, area);

    printDoubleFrequencyBrief(MAX_AREAS + 1, count, area);

    jf_clieng_outputLine("");
}
#endif

#if 0
static void _transitionMatrix(olint_t * transition, olint_t num, matrix_t * ma)
{
    olint_t i;
    olint_t numoftr = num - 1;
    olint_t count[3 * 3];
    olint_t counta[3];

    ol_memset(count, 0, sizeof(count));
    ol_memset(counta, 0, sizeof(counta));
    for (i = 1; i < numoftr; i ++)
    {
        count[transition[i - 1] * 3 + transition[i]] ++;
        counta[transition[i - 1]] ++;
    }

    for (i = 0; i < numoftr; i ++)
    {
        ol_printf(" %d ", transition[i]);
    }
    ol_printf("\n");
    for (i = 0; i < 9; i ++)
    {
        ol_printf(" %d ", count[i]);
    }
    ol_printf("\n");
    for (i = 0; i < 3; i ++)
    {
        ol_printf(" %d ", counta[i]);
    }
    ol_printf("\n");

    for (i = 0; i < 9; i ++)
    {
        ma->m_pdbData[i] = count[i];
        ma->m_pdbData[i] /= counta[i / 3];
    }

    printMatrix(ma);

}

static u32 _markovVolumn(oldouble_t * pdbvalue, olint_t num, matrix_t * mvolumn)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    matrix_t * ma = NULL;
    matrix_t lasttr;
    oldouble_t prob[3];
    olint_t * transition = NULL;
    oldouble_t high, low;
    olint_t i;

#define STABLE_PERCENT  0.1

    u32Ret = allocMatrix(3, 3, &ma);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_jiukun_allocMemory((void **)&transition, sizeof(olint_t) * (num - 1), 0);

        for (i = 1; i < num; i ++)
        {
            low = pdbvalue[i - 1] * (1.0 - STABLE_PERCENT);
            high = pdbvalue[i - 1] * (1.0 + STABLE_PERCENT);
            if (pdbvalue[i] >= high)
                transition[i - 1] = 2;
            else if (pdbvalue[i] >= low)
                transition[i - 1] = 1;
            else
                transition[i - 1] = 0;
        }

        _transitionMatrix(transition, num - 1, ma);

        initMatrix(&lasttr, 1, 3, prob);

        ol_memset(prob, 0, sizeof(prob));
        prob[transition[num - 2]] = 1.0;

        mulMatrix(mvolumn, &lasttr, ma);
        printMatrix(&lasttr);
        printMatrix(mvolumn);

        jf_jiukun_freeMemory((void **)&transition);
    }

    if (ma != NULL)
        freeMatrix(&ma);

    return u32Ret;
}

static u32 _markovDir(cli_stat_param_t * pcsp, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = MAX_NUM_OF_RESULT;
    da_day_result_t * buffer = NULL;
    parse_param_t pp;
    data_stat_t datastat;
    da_model_t * model = getCurrentDaModel();
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    stock_info_t * stockinfo;
    data_stat_param_t dsp;
    olchar_t statname[16];
    da_day_result_t *cur;
    oldouble_t * pdbvalue = NULL;
    olint_t i;
    matrix_t mbuy, msold;
    oldouble_t dbbuyp[3], dbsoldp[3];

    _printStatParamVerbose(pcsp);

    u32Ret = jf_mem_alloc((void **)&buffer, sizeof(da_day_result_t) * total);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = getStockInfo(pcsp->csp_pstrData, &stockinfo);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(&pp, 0, sizeof(pp));
        if (pcsp->csp_nThres == 0)
            pp.pp_nThres = getStockShareThres(stockinfo);
        else
            pp.pp_nThres = pcsp->csp_nThres;
        pp.pp_nStart = pcsp->csp_nStart;
        pp.pp_nEnd = pcsp->csp_nEnd;
        ol_strcpy(pp.pp_strLastX, model->dm_strLastTimeLowPrice);
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcsp->csp_pstrData);

        u32Ret = parseDataDir(dirpath, &pp, buffer, &total);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (total < DEF_DAY_FOR_TREND)
            u32Ret = JF_ERR_NOT_READY;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        getFileName(statname, 16, pcsp->csp_pstrData);
        ol_memset(&dsp, 0, sizeof(dsp));
        dsp.dsp_pstrName = statname;
        dsp.dsp_nDayForTrend = DEF_DAY_FOR_TREND;

        dataStatFromDayResult(
            &dsp, &datastat, buffer + total - DEF_DAY_FOR_TREND,
            DEF_DAY_FOR_TREND);

    }

    /*stat of buy*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        initMatrix(&mbuy, 1, 3, dbbuyp);
        initMatrix(&msold, 1, 3, dbsoldp);
        jf_jiukun_allocMemory((void **)&pdbvalue, sizeof(oldouble_t) * total, 0);
        cur = buffer;
        for (i = 0; i < total; i++)
        {
            pdbvalue[i] = cur->ddr_u64Buy;
            cur ++;
        }

        u32Ret = _markovVolumn(pdbvalue, total, &mbuy);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        cur = buffer;
        for (i = 0; i < total; i++)
        {
            pdbvalue[i] = cur->ddr_u64Sold;
            cur ++;
        }

        u32Ret = _markovVolumn(pdbvalue, total, &msold);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _printResultVolumnBrief(
            buffer + total - DEF_DAY_FOR_TREND, DEF_DAY_FOR_TREND,
            &datastat, &mbuy, &msold);

    }

    jf_clieng_outputLine("");

    if (pdbvalue != NULL)
        jf_jiukun_freeMemory((void **)&pdbvalue);
    jf_mem_free((void **)&buffer);

    return u32Ret;
}
#endif

static u32 _descStat(cli_stat_param_t * pcsp, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t dbva[10] = {38, 56, 59, 64, 74};
    oldouble_t dbvb[10] = {41, 63, 70, 72, 84};
    desc_stat_t dsa, dsb;
    oldouble_t dbr;

    u32Ret = descStatFromData(&dsa, dbva, 5);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = descStatFromData(&dsb, dbvb, 5);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = getCorrelation2(dbva, &dsa, dbvb, &dsb, 5, &dbr);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        printDescStatVerbose(&dsa);
        printDescStatVerbose(&dsb);
        jf_clieng_outputLine("correlation: %.3f", dbr);
    }

    return u32Ret;
}

static u32 _saveStatArbiStocks(
    cli_stat_param_t * pcsp, stat_arbi_indu_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t filepath[JF_LIMIT_MAX_PATH_LEN];
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    olchar_t buf[32];
    olint_t i;

    if (result->sair_nNumOfPair == 0)
        return u32Ret;

    ol_sprintf(filepath, "%s", STAT_ARBI_STOCK_FILE);

    u32Ret = jf_file_openWithMode(
        filepath, O_WRONLY | O_CREAT | O_TRUNC,
        JF_FILE_DEFAULT_CREATE_MODE, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0;
             (i < result->sair_nNumOfPair) && (u32Ret == JF_ERR_NO_ERROR); i ++)
        {
            ol_memset(buf, 0, sizeof(buf));
            ol_strcpy(buf, result->sair_psaireEntry[i].saire_strStockPair);
            ol_strcat(buf, "\n");
            u32Ret = jf_file_writen(fd, buf, ol_strlen(buf));
        }

        jf_file_close(&fd);
    }

    return u32Ret;
}

static u32 _statArbiIndustry(cli_stat_param_t * pcsp, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stat_arbi_indu_param_t saip;
    stat_arbi_indu_result_t result;
    olchar_t buf[2048];
    olint_t nIndustry;
    olint_t i;

    ol_memset(&saip, 0, sizeof(stat_arbi_indu_param_t));
    saip.saip_dbMinCorrelation = 0.80;
    saip.saip_nDaySummary = 50;
    saip.saip_nCorrelationArray = 40;

    ol_memset(&result, 0, sizeof(stat_arbi_indu_result_t));
    result.sair_nMaxPair = sizeof(buf) / sizeof(stat_arbi_indu_result_entry_t);
    result.sair_psaireEntry = (stat_arbi_indu_result_entry_t *)buf;
    buf[0] = '\0';

    if (strcmp(pcsp->csp_pstrIndustry, "all") == 0)
    {
        u32Ret = statArbiAllIndustry(
            getEnvVar(ENV_VAR_DATA_PATH), &saip, &result);
    }
    else
    {
        u32Ret = jf_string_getS32FromString(
            pcsp->csp_pstrIndustry, ol_strlen(pcsp->csp_pstrIndustry), &nIndustry);
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = statArbiIndustry(
                getEnvVar(ENV_VAR_DATA_PATH), nIndustry, &saip, &result);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0;
             i < result.sair_nNumOfPair; i ++)
        {
            jf_clieng_outputLine(
                "%s", result.sair_psaireEntry[i].saire_strStockPair);
        }

        u32Ret = _saveStatArbiStocks(pcsp, &result);
    }

    return u32Ret;
}

static u32 _onSamePage(
    sa_stock_info_t * sastock, olint_t * nDaySummary)
{
    u32 u32Ret = JF_ERR_NOT_READY;
    da_day_summary_t * end[2];
    olint_t ret;

    while ((sastock[0].ssi_nDaySummary < nDaySummary[0]) &&
           (sastock[1].ssi_nDaySummary < nDaySummary[1]))
    {
        end[0] = sastock[0].ssi_pddsSummary + sastock[0].ssi_nDaySummary - 1;
        end[1] = sastock[1].ssi_pddsSummary + sastock[1].ssi_nDaySummary - 1;

        ret = strcmp(end[0]->dds_strDate, end[1]->dds_strDate);
        if (ret == 0)
        {
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }
        else if (ret < 0)
            sastock[0].ssi_nDaySummary ++;
        else
            sastock[1].ssi_nDaySummary ++;
    }

    return u32Ret;
}

static u32 _saStockPair(
    cli_stat_param_t * pcsp, stat_arbi_desc_t * arbi,
    sa_stock_info_t * sastock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stat_arbi_param_t sap;
    da_conc_t daconc[2];
    da_conc_sum_t concsum;
    da_day_summary_t * end[2];
    olint_t nDaySummary[2];

    jf_clieng_printBanner(JF_CLIENG_MAX_OUTPUT_LINE_LEN);
    jf_clieng_outputLine("%s", arbi->sad_pstrName);
    if (pcsp->csp_bVerbose)
    {
        arbi->sad_fnPrintParamVerbose(arbi, &sap);
        jf_clieng_printDivider();
    }

    ol_memset(&sap, 0, sizeof(stat_arbi_param_t));
    ol_memset(&concsum, 0, sizeof(da_conc_sum_t));
    ol_memset(&daconc, 0, sizeof(da_conc_t) * 2);
    nDaySummary[0] = sastock[0].ssi_nDaySummary;
    nDaySummary[1] = sastock[1].ssi_nDaySummary;
    sastock[0].ssi_nDaySummary -= 250;
    sastock[1].ssi_nDaySummary -= 250;
    while ((sastock[0].ssi_nDaySummary < nDaySummary[0]) &&
           (sastock[1].ssi_nDaySummary < nDaySummary[1]) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        u32Ret = _onSamePage(sastock, nDaySummary);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = arbi->sad_fnStatArbiStocks(arbi, &sap, sastock, daconc);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            end[0] = sastock[0].ssi_pddsSummary + sastock[0].ssi_nDaySummary - 1;
            end[1] = sastock[1].ssi_pddsSummary + sastock[1].ssi_nDaySummary - 1;
            if (daconc[0].dc_nAction == CONC_ACTION_BULL_OPENING)
            {
                if (pcsp->csp_bVerbose)
                    ol_printf(
                        "Stock %s, %s, opening with price %.2f\n",
                        sastock[0].ssi_psiStock->si_strCode, end[0]->dds_strDate,
                        daconc[0].dc_dbPrice);
            }
            if (daconc[1].dc_nAction == CONC_ACTION_BULL_OPENING)
            {
                if (pcsp->csp_bVerbose)
                    ol_printf(
                        "Stock %s, %s, opening with price %.2f\n",
                        sastock[1].ssi_psiStock->si_strCode, end[1]->dds_strDate,
                        daconc[1].dc_dbPrice);
            }

            if (daconc[0].dc_nAction == CONC_ACTION_BULL_CLOSEOUT)
            {
                if (pcsp->csp_bVerbose)
                    ol_printf(
                        "Stock %s, %s, close out with price %.2f, hold %d days, yield %.2f%%\n",
                        sastock[0].ssi_psiStock->si_strCode, end[0]->dds_strDate,
                        daconc[0].dc_dbPrice,
                        daconc[0].dc_nHoldDays, daconc[0].dc_dbYield);
                addToDaConcSum(&concsum, &daconc[0]);
            }
            if (daconc[1].dc_nAction == CONC_ACTION_BULL_CLOSEOUT)
            {
                if (pcsp->csp_bVerbose)
                    ol_printf(
                        "Stock %s, %s, close out with price %.2f, hold %d days, yield %.2f%%\n",
                        sastock[1].ssi_psiStock->si_strCode, end[1]->dds_strDate,
                        daconc[1].dc_dbPrice,
                        daconc[1].dc_nHoldDays, daconc[1].dc_dbYield);
                addToDaConcSum(&concsum, &daconc[1]);
            }
        }
        else if (u32Ret == JF_ERR_NOT_READY)
            u32Ret = JF_ERR_NO_ERROR;

        sastock[0].ssi_nDaySummary ++;
        sastock[1].ssi_nDaySummary ++;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        printDaConcSumVerbose(&concsum);
        jf_clieng_outputLine("");
    }

    return u32Ret;
}

static u32 _statArbiStockPair(
    cli_stat_param_t * pcsp, olchar_t * pstrStocks)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sa_stock_info_t * sastock = NULL;
    stat_arbi_desc_t * arbi;
    olint_t nDaySummary = MIN_STAT_ARBI_DAY_SUMMARY;

    jf_clieng_printDivider();
    jf_clieng_outputLine(
        "Stat arbitrage for stock pair: %s", pstrStocks);

    u32Ret = newSaStockInfo(
		getEnvVar(ENV_VAR_DATA_PATH), pstrStocks,
        &sastock, nDaySummary);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((sastock[0].ssi_nDaySummary != nDaySummary) ||
            (sastock[1].ssi_nDaySummary != nDaySummary))
        {
            jf_clieng_outputLine("Not enough day summary");
            u32Ret = JF_ERR_NOT_READY;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        arbi = getStatArbiDesc(STAT_ARBI_DESC_STAT);

        _saStockPair(pcsp, arbi, sastock);
    }

    if (sastock != NULL)
        freeSaStockInfo(&sastock);

    return u32Ret;
}

static u32 _statArbiStocks(cli_stat_param_t * pcsp, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t filepath[JF_LIMIT_MAX_PATH_LEN];
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    olchar_t line[512];
    olsize_t sline;

    ol_sprintf(filepath, "%s", STAT_ARBI_STOCK_FILE);

    u32Ret = jf_file_open(filepath, O_RDONLY, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        do
        {
            sline = sizeof(line);
            u32Ret = jf_file_readLine(fd, line, &sline);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                line[17] = '\0';
                _statArbiStockPair(pcsp, line);
            }

        } while (u32Ret == JF_ERR_NO_ERROR);

        jf_file_close(&fd);

        if (u32Ret == JF_ERR_END_OF_FILE)
            u32Ret = JF_ERR_NO_ERROR;
    }
    else
    {
        jf_clieng_outputLine("Cannot find file %s", STAT_ARBI_STOCK_FILE);
    }

    return u32Ret;
}

static u32 _statArbiFind(cli_stat_param_t * pcsp, da_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

//	u32Ret = statArbiStockPair(
//		getEnvVar(ENV_VAR_DATA_PATH), pcsp->csp_pstrData, NULL);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processStat(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_stat_param_t * pcsp = (cli_stat_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcsp->csp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _statHelp(pdm);
    else if (pcsp->csp_u8Action == CLI_ACTION_INFLEXION_POINT)
        u32Ret = _parseDirInflexionPoint(pcsp, pdm);
    else if (pcsp->csp_u8Action == CLI_ACTION_INFLEXION_POINT_2)
        u32Ret = _parseDirInflexionPoint2(pcsp, pdm);
//    else if (pcsp->csp_u8Action == CLI_ACTION_MARKOV_DIR)
//        u32Ret = _markovDir(pcsp, pdm);
    else if (pcsp->csp_u8Action == CLI_ACTION_DESCRIPTIVE_STAT)
        u32Ret = _descStat(pcsp, pdm);
    else if (pcsp->csp_u8Action == CLI_ACTION_STAT_ARBI_INDUSTRY)
        u32Ret = _statArbiIndustry(pcsp, pdm);
	else if (pcsp->csp_u8Action == CLI_ACTION_STAT_ARBI_STOCKS)
		u32Ret = _statArbiStocks(pcsp, pdm);
	else if (pcsp->csp_u8Action == CLI_ACTION_STAT_ARBI_FIND)
		u32Ret = _statArbiFind(pcsp, pdm);
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
        "i:n:l:m:a:pf:chv?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
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
        case 'p':
            pcsp->csp_u8Action = CLI_ACTION_STAT_ARBI_STOCKS;
            break;		
        case 'f':
            pcsp->csp_u8Action = CLI_ACTION_STAT_ARBI_FIND;
			pcsp->csp_pstrData = (olchar_t *)optarg;
            break;		
        case 'm':
            pcsp->csp_u8Action = CLI_ACTION_MARKOV_DIR;
            pcsp->csp_pstrData = (olchar_t *)optarg;
            break;		
        case 'i':
            pcsp->csp_u8Action = CLI_ACTION_INFLEXION_POINT;
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


