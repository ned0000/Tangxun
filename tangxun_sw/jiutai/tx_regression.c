/**
 *  @file tx_regression.c
 *
 *  @brief Regression analysis.
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

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_mem.h"
#include "jf_clieng.h"
#include "jf_matrix.h"

#include "tx_datastat.h"
#include "tx_regression.h"

/* --- private data/data structure section ------------------------------------------------------ */
#define DEBUG_REGRESSION_ANALYSIS  0
#define MAX_RA_SE_RATIO  2.0

static jf_clieng_caption_t ls_jccRaResultCoefBrief[] =
{
    {"Predictor", 12},
    {"Coef", 13},
    {"SECoef", 13},
    {"T", 8},
    {"P", 8},
};

static jf_clieng_caption_t ls_jccRaResultAnovaBrief[] =
{
    {"Source", 16},
    {"DF", 3},
    {"SS", 10},
    {"MS", 10},
    {"F", 8},
    {"P", 8},
};

static jf_clieng_caption_t ls_jccRaResultCoefSeqSSBrief[] =
{
    {"Source", 12},
    {"DF", 3},
    {"SeqSS", 13},
};

static jf_clieng_caption_t ls_jccRaResultUnusualObservBrief[] =
{
    {"Obs", 4},
    {"Response", 10},
    {"Fit", 10},
    {"SEFit", 10},
    {"Resid", 10},
    {"StResid", 10},
};

/* --- private routine section ------------------------------------------------------------------ */
static u32 agaus(oldouble_t *a, oldouble_t *b, olint_t n)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t *js, l, k, i, j, is, p, q;
    oldouble_t d,t;

    u32Ret = jf_mem_alloc((void **)&js, n*sizeof(olint_t));
    if (u32Ret != JF_ERR_NO_ERROR)
        goto out;

    l = 1;
    for (k = 0; k <= n - 2; k ++)
    {
        d = 0.0;
        for (i = k; i <= n - 1; i++)
            for (j = k; j <= n-1; j++)
            {
                t = fabs(a[i * n + j]);
                if (t > d)
                {
                    d = t; js[k] = j; is = i;
                }
            }
        if (d + 1.0 == 1.0)
            l = 0;
        else
        {
            if (js[k] != k)
                for (i = 0; i <= n - 1; i ++)
                {
                    p = i * n + k; q = i * n + js[k];
                    t = a[p]; a[p] = a[q]; a[q] = t;
                }
            if (is != k)
            {
                for (j = k; j <= n - 1; j ++)
                {
                    p = k * n + j; q = is * n + j;
                    t = a[p]; a[p] = a[q]; a[q] = t;
                }
                t = b[k]; b[k] = b[is]; b[is] = t;
            }
        }
        if (l == 0)
        {
            u32Ret = JF_ERR_INVALID_DATA;
            goto out;
        }
        d = a[k * n + k];
        for ( j = k + 1; j <= n - 1; j ++)
        {
            p = k * n + j;
            a[p] = a[p] / d;
        }
        b[k] = b[k] / d;
        for (i = k + 1; i <= n - 1; i ++)
        {
            for (j = k + 1; j <= n - 1; j++)
            {
                p = i * n + j;
                a[p] = a[p] - a[i * n + k] * a[k * n + j];
            }
            b[i] = b[i] - a[i * n + k] * b[k];
        }
    }
    d = a[(n - 1) * n + n - 1];
    if (fabs(d) + 1.0 == 1.0)
    {
        u32Ret = JF_ERR_INVALID_DATA;
        return u32Ret;
    }
    b[n - 1] = b[n - 1] / d;
    for (i = n - 2; i >= 0; i --)
    {
        t = 0.0;
        for (j = i + 1; j <= n - 1; j ++)
            t = t + a[i * n + j] * b[j];
        b[i] = b[i] - t;
    }
    js[n - 1] = n - 1;
    for (k = n - 1; k >= 0; k --)
        if (js[k] != k)
        {
            t = b[k]; b[k] = b[js[k]]; b[js[k]] = t;
        }

out:
    jf_mem_free((void **)&js);
    return u32Ret;
}

static u32 _caluRaLeverage(oldouble_t * response, oldouble_t ** predictors,
    olint_t countp, olint_t num, oldouble_t * hii)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    matrix_t * pmh = NULL, * pmx = NULL;
    olint_t i, j;
    oldouble_t * dbd;

    u32Ret = jf_matrix_alloc(num, num, &pmh);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_matrix_alloc(num, countp + 1, &pmx);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        dbd = pmx->m_pdbData;
        for (i = 0; i < num; i ++)
        {
            *dbd ++ = 1;
            for (j = 0; j < countp; j ++)
            {
                *dbd ++ = predictors[j][i];
            }
        }

        u32Ret = jf_matrix_hat(pmh, pmx);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < num; i ++)
            hii[i] = *(pmh->m_pdbData + num * i + i);
    }

    if (pmh != NULL)
        jf_matrix_free(&pmh);
    if (pmx != NULL)
        jf_matrix_free(&pmx);

    return u32Ret;
}

static u32 _caluRaResultUnusualObserv(
    oldouble_t * response, oldouble_t ** predictors,
    olint_t countp, olint_t num, tx_regression_result_t * result, tx_datastat_desc_t * rdesc,
    oldouble_t * dbfit)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_regression_result_unusual_observ_t * observ;
    oldouble_t * hii = NULL;
    oldouble_t dbt, hir;
    olint_t i, ntype;

    u32Ret = jf_mem_calloc((void **)&hii, sizeof(oldouble_t) * num);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _caluRaLeverage(response, predictors, countp, num, hii);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        hir = 3 * (countp + 1);
        hir /= num;
        if (hir > 0.99)
            hir = 0.99;

        for (i = 0; i < num; i ++)
        {
            ntype = 0;
            dbt = 0;
            if (hii[i] < 1.0)
                dbt = (response[i] - dbfit[i]) /
                    sqrt(result->trr_trraAnova.trra_dbResidErrorMS * (1 - hii[i]));

            if (hii[i] > hir)
                ntype |= TX_REGRESSION_UNUSUAL_OBSERV_X;
            if ((dbt > 2.0 ) || (dbt < -2.0))
                ntype |= TX_REGRESSION_UNUSUAL_OBSERV_LARGE_REDIDUAL;

            if (ntype != 0)
            {
                observ = &result->trr_ptrruoObserv[result->trr_nObserv];

                observ->trruo_nObs = i;
                observ->trruo_dbResponse = response[i];
                observ->trruo_dbFit = dbfit[i];
                observ->trruo_dbResid = response[i] - dbfit[i];
                observ->trruo_dbStResid = dbt;
                observ->trruo_nType = ntype;

                result->trr_nObserv ++;
                if (result->trr_nObserv >= result->trr_nMaxObserv)
                    break;
            }
        }
    }

    if (hii != NULL)
        jf_mem_free((void **)&hii);

    return u32Ret;
}

static u32 _caluRaResultAnova(
    oldouble_t * response, oldouble_t ** predictors,
    olint_t countp, olint_t num, tx_regression_result_t * result, tx_datastat_desc_t * rdesc,
    oldouble_t * dbfit)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;

    for (i = 0; i < num; i ++)
    {
        /* the mean of dbfit[] is 0 */
        result->trr_trraAnova.trra_dbRegressionSS +=
            (dbfit[i] - rdesc->tdd_dbMean) * (dbfit[i] - rdesc->tdd_dbMean);
        result->trr_trraAnova.trra_dbResidErrorSS +=
            (response[i] - dbfit[i]) * (response[i] - dbfit[i]);
    }
    result->trr_trraAnova.trra_nRegressionDF = countp;
    result->trr_trraAnova.trra_dbRegressionMS =
        result->trr_trraAnova.trra_dbRegressionSS /
        result->trr_trraAnova.trra_nRegressionDF;

    result->trr_trraAnova.trra_nResidErrorDF = num - countp - 1;
    result->trr_trraAnova.trra_dbResidErrorMS =
        result->trr_trraAnova.trra_dbResidErrorSS /
        result->trr_trraAnova.trra_nResidErrorDF;

    result->trr_trraAnova.trra_nTotalDF = num - 1;
    result->trr_trraAnova.trra_dbTotalSS =
        result->trr_trraAnova.trra_dbRegressionSS +
        result->trr_trraAnova.trra_dbResidErrorSS;

    result->trr_trraAnova.trra_dbRegressionF =
        result->trr_trraAnova.trra_dbRegressionMS /
        result->trr_trraAnova.trra_dbResidErrorMS;

    return u32Ret;
}

static u32 _caluRaResultCoefSeqSS(
    oldouble_t * response, oldouble_t ** predictors,
    olint_t countp, olint_t num, tx_regression_result_t * result, tx_datastat_desc_t * rdesc,
    oldouble_t * dbfit)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static u32 _caluRaResult(
    oldouble_t * response, oldouble_t ** predictors,
    olint_t countp, olint_t num, tx_regression_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_datastat_desc_t rdescstat;
    oldouble_t * temp = NULL, dbt;
    olint_t i, j;

    u32Ret = jf_mem_duplicate(
        (void **)&temp, (const u8 *)response, sizeof(oldouble_t) * num);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = tx_datastat_descData(&rdescstat, temp, num);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < num; i ++)
        {
            temp[i] = result->trr_trrcConstant.trrc_dbCoef;
            for (j = 0; j < countp; j ++)
            {
                temp[i] += result->trr_ptrrcCoef[j].trrc_dbCoef * predictors[j][i];
            }
        }

        u32Ret = _caluRaResultAnova(response, predictors,
            countp, num, result, &rdescstat, temp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _caluRaResultCoefSeqSS(response, predictors,
            countp, num, result, &rdescstat, temp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        result->trr_dbS = sqrt(result->trr_trraAnova.trra_dbResidErrorMS);
        result->trr_dbRSq = result->trr_trraAnova.trra_dbRegressionSS * 100 /
            result->trr_trraAnova.trra_dbTotalSS;
        dbt = result->trr_dbS / rdescstat.tdd_dbStDev;
        result->trr_dbRSqAdj = (1 - dbt * dbt) * 100;

    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _caluRaResultUnusualObserv(response, predictors,
                   countp, num, result, &rdescstat, temp);
    }

    if (temp != NULL)
        jf_mem_free((void **)&temp);

    return u32Ret;
}

static void _printRaResultCoefBrief(tx_regression_result_coef_t * coef)
{
    jf_clieng_caption_t * pcc = &ls_jccRaResultCoefBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Predictor */
    jf_clieng_appendBriefColumn(pcc, strInfo, coef->trrc_strPredictor);
    pcc++;

    /* Coef */
    ol_sprintf(strField, "%.7f", coef->trrc_dbCoef);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SECoef */
    ol_sprintf(strField, "%.7f", coef->trrc_dbSECoef);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* T */
    ol_sprintf(strField, "%.2f", coef->trrc_dbT);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* P */
    ol_sprintf(strField, "%.2f", coef->trrc_dbP);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputRawLine(strInfo);
}

static void _printRaResultAnovaBrief(tx_regression_result_t * result)
{
    jf_clieng_caption_t * pcc = &ls_jccRaResultAnovaBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Source */
    jf_clieng_appendBriefColumn(pcc, strInfo, "Regression");
    pcc++;

    /* DF */
    ol_sprintf(strField, "%d", result->trr_trraAnova.trra_nRegressionDF);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SS */
    ol_sprintf(strField, "%.3f", result->trr_trraAnova.trra_dbRegressionSS);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* MS */
    ol_sprintf(strField, "%.3f", result->trr_trraAnova.trra_dbRegressionMS);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* F */
    ol_sprintf(strField, "%.3f", result->trr_trraAnova.trra_dbRegressionF);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* P */
    ol_sprintf(strField, "%.3f", result->trr_trraAnova.trra_dbRegressionP);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputRawLine(strInfo);


    pcc = &ls_jccRaResultAnovaBrief[0];
    strInfo[0] = '\0';

    /* Source */
    jf_clieng_appendBriefColumn(pcc, strInfo, "Residual Error");
    pcc++;

    /* DF */
    ol_sprintf(strField, "%d", result->trr_trraAnova.trra_nResidErrorDF);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SS */
    ol_sprintf(strField, "%.3f", result->trr_trraAnova.trra_dbResidErrorSS);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* MS */
    ol_sprintf(strField, "%.3f", result->trr_trraAnova.trra_dbResidErrorMS);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputRawLine(strInfo);


    pcc = &ls_jccRaResultAnovaBrief[0];
    strInfo[0] = '\0';

    /* Source */
    jf_clieng_appendBriefColumn(pcc, strInfo, "Total");
    pcc++;

    /* DF */
    ol_sprintf(strField, "%d", result->trr_trraAnova.trra_nTotalDF);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SS */
    ol_sprintf(strField, "%.3f", result->trr_trraAnova.trra_dbTotalSS);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputRawLine(strInfo);
}

static void _printRaResultCoefSeqSSBrief(tx_regression_result_coef_t * coef)
{
    jf_clieng_caption_t * pcc = &ls_jccRaResultCoefSeqSSBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Source */
    jf_clieng_appendBriefColumn(pcc, strInfo, coef->trrc_strPredictor);
    pcc++;

    /* DF */
    ol_sprintf(strField, "%d", coef->trrc_nDF);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SeqSS */
    ol_sprintf(strField, "%.3f", coef->trrc_dbSeqSS);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputRawLine(strInfo);
}

static void _printRaResultUnusualObservBrief(
    tx_regression_result_unusual_observ_t * observ)
{
    jf_clieng_caption_t * pcc = &ls_jccRaResultUnusualObservBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Obs */
    ol_sprintf(strField, "%d", observ->trruo_nObs);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Response */
    ol_sprintf(strField, "%.4f", observ->trruo_dbResponse);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Fit */
    ol_sprintf(strField, "%.4f", observ->trruo_dbFit);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SEFit */
    ol_sprintf(strField, "%.4f", observ->trruo_dbSEFit);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Resid */
    ol_sprintf(strField, "%.4f", observ->trruo_dbResid);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* StResid */
    ol_sprintf(strField, "%.2f", observ->trruo_dbStResid);
    if (observ->trruo_nType & TX_REGRESSION_UNUSUAL_OBSERV_LARGE_REDIDUAL)
        ol_strcat(strField, "R");
    if (observ->trruo_nType & TX_REGRESSION_UNUSUAL_OBSERV_X)
        ol_strcat(strField, "X");
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputRawLine(strInfo);
}

static void _preSetRaResult(olchar_t * rname, olchar_t ** pname,
    olint_t countp, tx_regression_result_t * result)
{
    olint_t i;

    ol_strncpy(
        result->trr_strResponse, rname, sizeof(result->trr_strResponse) - 1);

    ol_strncpy(
        result->trr_trrcConstant.trrc_strPredictor, "Constant",
        sizeof(result->trr_trrcConstant.trrc_strPredictor) - 1);

    result->trr_nCoef = countp;
    for (i = 0; i < countp; i ++)
    {
        ol_strncpy(
            result->trr_ptrrcCoef[i].trrc_strPredictor, pname[i],
            sizeof(result->trr_ptrrcCoef[i].trrc_strPredictor) - 1);
    }

}

static void _makeMatrix(
    oldouble_t * response, oldouble_t ** predictors,
    olint_t countp, olint_t num, oldouble_t * dbx, oldouble_t * dby) 
{
    olint_t i, j, k;

#if DEBUG_REGRESSION_ANALYSIS
    for (i = 0; i < num; i ++)
    {
        ol_printf(" %.2f", response[i]);
        for (j = 0; j < countp; j ++)
        {
            ol_printf(" %.2f", predictors[j][i]);
        }

        ol_printf("\n");
    }
    ol_printf("\n");
#endif

    for (i = 0; i < num; i ++)
        dby[0] += response[i];

    for (i = 0; i < countp; i ++)
    {
        for (j = 0; j < num; j ++)
            dby[i + 1] += response[j] * predictors[i][j];
    }

    dbx[0] = num;
    for (i = 0; i < countp; i ++)
        for (j = 0; j < num; j ++)
            dbx[i + 1] += predictors[i][j];

    for (i = 0; i < countp; i ++)
    {
        for (k = 0; k < num; k ++)
            dbx[(i + 1) * (countp + 1)] += predictors[i][k];

        for (j = 0; j < countp; j ++)
            for (k = 0; k < num; k ++)
                dbx[(i + 1) * (countp + 1) + j + 1] +=
                    predictors[i][k] * predictors[j][k];
    }

#if DEBUG_REGRESSION_ANALYSIS
    for (i = 0; i < countp + 1; i ++)
    {
        ol_printf(" %.2f,", dby[i]);
    }
    ol_printf("\n\n");
    for (i = 0; i < countp + 1; i ++)
    {
        for (j = 0; j < countp + 1; j ++)
        {
            ol_printf(" %.2f,", dbx[i * (countp + 1) + j]);
        }

        ol_printf("\n");
    }
#endif

}

/* --- public routine section ------------------------------------------------------------------- */
u32 tx_regression_analysis(
    olchar_t * rname, oldouble_t * response, olchar_t ** pname,
    oldouble_t ** predictors, olint_t countp, olint_t num, tx_regression_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t * dbx = NULL, * dby = NULL;
    olint_t i;

    if (countp + 1 >= num)
        return JF_ERR_INVALID_PARAM;

    u32Ret = jf_mem_calloc((void **)&dbx,
         sizeof(oldouble_t) * (countp + 1) * (countp + 1));
    if (u32Ret != JF_ERR_NO_ERROR)
        goto out;

    u32Ret = jf_mem_calloc((void **)&dby, sizeof(oldouble_t) * (countp + 1));
    if (u32Ret != JF_ERR_NO_ERROR)
        goto out;

    _preSetRaResult(rname, pname, countp, result);

    _makeMatrix(response, predictors, countp, num, dbx, dby);

    u32Ret = agaus(dbx, dby, countp + 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        result->trr_trrcConstant.trrc_dbCoef = dby[0];
        for (i = 0; i < countp; i ++)
        {
            result->trr_ptrrcCoef[i].trrc_dbCoef = dby[i + 1];
        }
#if DEBUG_REGRESSION_ANALYSIS
        for (i = 0; i < countp + 1; i ++)
            ol_printf("%.7f\n", dby[i]);
        ol_printf("\n");
#endif

        _caluRaResult(response, predictors, countp, num, result);
    }

out:
    if (dbx != NULL)
        jf_mem_free((void **)&dbx);
    if (dby != NULL)
        jf_mem_free((void **)&dby);

    return u32Ret;
}

void tx_regression_printResult(tx_regression_result_t * result)
{
    olint_t i;
    olchar_t str[512], strt[64];

    memset(str, 0, sizeof(str));
    ol_sprintf(str, "Regression Analysis: %s versus ", result->trr_strResponse);
    for (i = 0; i < result->trr_nCoef; i ++)
    {
        ol_strcat(str, result->trr_ptrrcCoef[i].trrc_strPredictor);
        if (i != result->trr_nCoef - 1)
            ol_strcat(str, ", ");
    }
    jf_clieng_outputLine("%s\n", str);

    ol_sprintf(str, "The regression equation is");
    jf_clieng_outputLine("%s", str);

    ol_sprintf(str, "%s = %.6f", result->trr_strResponse,
            result->trr_trrcConstant.trrc_dbCoef);
    for (i = 0; i < result->trr_nCoef; i ++)
    {
        if (result->trr_ptrrcCoef[i].trrc_dbCoef >= 0.0)
            ol_strcat(str, " + ");
        else
            ol_strcat(str, " - ");
        ol_sprintf(strt, "%.6f", fabs(result->trr_ptrrcCoef[i].trrc_dbCoef));
        ol_strcat(str, strt);
        ol_strcat(str, " ");
        ol_strcat(str, result->trr_ptrrcCoef[i].trrc_strPredictor);
    }
    jf_clieng_outputLine("%s\n\n", str);

    jf_clieng_printHeader(ls_jccRaResultCoefBrief,
             sizeof(ls_jccRaResultCoefBrief) / sizeof(jf_clieng_caption_t));
    _printRaResultCoefBrief(&result->trr_trrcConstant);
    for (i = 0; i < result->trr_nCoef; i ++)
    {
        _printRaResultCoefBrief(&result->trr_ptrrcCoef[i]);        
    }


    ol_sprintf(str, "\n\nS = %.5f   R-Sq = %.1f%%   R-Sq(adj) = %.1f%%",
            result->trr_dbS, result->trr_dbRSq, result->trr_dbRSqAdj);
    jf_clieng_outputRawLine(str);

    jf_clieng_outputLine("\n\nAnalysis of Variance\n");
    jf_clieng_printHeader(ls_jccRaResultAnovaBrief,
             sizeof(ls_jccRaResultAnovaBrief) / sizeof(jf_clieng_caption_t));
    _printRaResultAnovaBrief(result);

    jf_clieng_outputLine("\n\n");
    jf_clieng_printHeader(ls_jccRaResultCoefSeqSSBrief,
        sizeof(ls_jccRaResultCoefSeqSSBrief) / sizeof(jf_clieng_caption_t));
    for (i = 0; i < result->trr_nCoef; i ++)
        _printRaResultCoefSeqSSBrief(&result->trr_ptrrcCoef[i]);

    jf_clieng_outputLine("\n\nUnusual Observations\n");
    jf_clieng_printHeader(ls_jccRaResultUnusualObservBrief,
        sizeof(ls_jccRaResultUnusualObservBrief) / sizeof(jf_clieng_caption_t));
    for (i = 0; i < result->trr_nObserv; i ++)
        _printRaResultUnusualObservBrief(&result->trr_ptrruoObserv[i]);

}

/*------------------------------------------------------------------------------------------------*/


