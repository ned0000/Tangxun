/**
 *  @file regression.c
 *
 *  @brief Regression analysis
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

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "regression.h"
#include "xmalloc.h"
#include "clieng.h"
#include "datastat.h"
#include "matrix.h"

/* --- private data/data structure section --------------------------------- */
#define DEBUG_REGRESSION_ANALYSIS  0
#define MAX_RA_SE_RATIO  2.0

static clieng_caption_t ls_ccRaResultCoefBrief[] =
{
    {"Predictor", 12},
    {"Coef", 13},
    {"SECoef", 13},
    {"T", 8},
    {"P", 8},
};

static clieng_caption_t ls_ccRaResultAnovaBrief[] =
{
    {"Source", 16},
    {"DF", 3},
    {"SS", 10},
    {"MS", 10},
    {"F", 8},
    {"P", 8},
};

static clieng_caption_t ls_ccRaResultCoefSeqSSBrief[] =
{
    {"Source", 12},
    {"DF", 3},
    {"SeqSS", 13},
};

static clieng_caption_t ls_ccRaResultUnusualObservBrief[] =
{
    {"Obs", 4},
    {"Response", 10},
    {"Fit", 10},
    {"SEFit", 10},
    {"Resid", 10},
    {"StResid", 10},
};

/* --- private routine section---------------------------------------------- */
static u32 agaus(oldouble_t *a, oldouble_t *b, olint_t n)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t *js, l, k, i, j, is, p, q;
    oldouble_t d,t;

    u32Ret = xmalloc((void **)&js, n*sizeof(olint_t));
    if (u32Ret != OLERR_NO_ERROR)
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
            u32Ret = OLERR_INVALID_DATA;
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
        u32Ret = OLERR_INVALID_DATA;
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
    xfree((void **)&js);
    return u32Ret;
}

static u32 _caluRaLeverage(oldouble_t * response, oldouble_t ** predictors,
    olint_t countp, olint_t num, oldouble_t * hii)
{
    u32 u32Ret = OLERR_NO_ERROR;
    matrix_t * pmh = NULL, * pmx = NULL;
    olint_t i, j;
    oldouble_t * dbd;

    u32Ret = allocMatrix(num, num, &pmh);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = allocMatrix(num, countp + 1, &pmx);

    if (u32Ret == OLERR_NO_ERROR)
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

        u32Ret = hatMatrix(pmh, pmx);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        for (i = 0; i < num; i ++)
            hii[i] = *(pmh->m_pdbData + num * i + i);
    }

    if (pmh != NULL)
        freeMatrix(&pmh);
    if (pmx != NULL)
        freeMatrix(&pmx);

    return u32Ret;
}

static u32 _caluRaResultUnusualObserv(
    oldouble_t * response, oldouble_t ** predictors,
    olint_t countp, olint_t num, ra_result_t * result, desc_stat_t * rdesc,
    oldouble_t * dbfit)
{
    u32 u32Ret = OLERR_NO_ERROR;
    ra_result_unusual_observ_t * observ;
    oldouble_t * hii = NULL;
    oldouble_t dbt, hir;
    olint_t i, ntype;

    u32Ret = xcalloc((void **)&hii, sizeof(oldouble_t) * num);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _caluRaLeverage(response, predictors, countp, num, hii);

    if (u32Ret == OLERR_NO_ERROR)
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
                    sqrt(result->rr_rraAnova.rra_dbResidErrorMS * (1 - hii[i]));

            if (hii[i] > hir)
                ntype |= RA_UNUSUAL_OBSERV_X;
            if ((dbt > 2.0 ) || (dbt < -2.0))
                ntype |= RA_UNUSUAL_OBSERV_LARGE_REDIDUAL;

            if (ntype != 0)
            {
                observ = &result->rr_prruoObserv[result->rr_nObserv];

                observ->rruo_nObs = i;
                observ->rruo_dbResponse = response[i];
                observ->rruo_dbFit = dbfit[i];
                observ->rruo_dbResid = response[i] - dbfit[i];
                observ->rruo_dbStResid = dbt;
                observ->rruo_nType = ntype;

                result->rr_nObserv ++;
                if (result->rr_nObserv >= result->rr_nMaxObserv)
                    break;
            }
        }
    }

    if (hii != NULL)
        xfree((void **)&hii);

    return u32Ret;
}

static u32 _caluRaResultAnova(
    oldouble_t * response, oldouble_t ** predictors,
    olint_t countp, olint_t num, ra_result_t * result, desc_stat_t * rdesc,
    oldouble_t * dbfit)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i;

    for (i = 0; i < num; i ++)
    {
        /* the mean of dbfit[] is 0 */
        result->rr_rraAnova.rra_dbRegressionSS +=
            (dbfit[i] - rdesc->ds_dbMean) * (dbfit[i] - rdesc->ds_dbMean);
        result->rr_rraAnova.rra_dbResidErrorSS +=
            (response[i] - dbfit[i]) * (response[i] - dbfit[i]);
    }
    result->rr_rraAnova.rra_nRegressionDF = countp;
    result->rr_rraAnova.rra_dbRegressionMS =
        result->rr_rraAnova.rra_dbRegressionSS /
        result->rr_rraAnova.rra_nRegressionDF;

    result->rr_rraAnova.rra_nResidErrorDF = num - countp - 1;
    result->rr_rraAnova.rra_dbResidErrorMS =
        result->rr_rraAnova.rra_dbResidErrorSS /
        result->rr_rraAnova.rra_nResidErrorDF;

    result->rr_rraAnova.rra_nTotalDF = num - 1;
    result->rr_rraAnova.rra_dbTotalSS =
        result->rr_rraAnova.rra_dbRegressionSS +
        result->rr_rraAnova.rra_dbResidErrorSS;

    result->rr_rraAnova.rra_dbRegressionF =
        result->rr_rraAnova.rra_dbRegressionMS /
        result->rr_rraAnova.rra_dbResidErrorMS;

    return u32Ret;
}

static u32 _caluRaResultCoefSeqSS(
    oldouble_t * response, oldouble_t ** predictors,
    olint_t countp, olint_t num, ra_result_t * result, desc_stat_t * rdesc,
    oldouble_t * dbfit)
{
    u32 u32Ret = OLERR_NO_ERROR;

    return u32Ret;
}

static u32 _caluRaResult(
    oldouble_t * response, oldouble_t ** predictors,
    olint_t countp, olint_t num, ra_result_t * result)
{
    u32 u32Ret = OLERR_NO_ERROR;
    desc_stat_t rdescstat;
    oldouble_t * temp = NULL, dbt;
    olint_t i, j;

    u32Ret = dupMemory((void **)&temp, (const u8 *)response,
                       sizeof(oldouble_t) * num);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = descStatFromData(&rdescstat, temp, num);

    if (u32Ret == OLERR_NO_ERROR)
    {
        for (i = 0; i < num; i ++)
        {
            temp[i] = result->rr_rrcConstant.rrc_dbCoef;
            for (j = 0; j < countp; j ++)
            {
                temp[i] += result->rr_prrcCoef[j].rrc_dbCoef * predictors[j][i];
            }
        }

        u32Ret = _caluRaResultAnova(response, predictors,
            countp, num, result, &rdescstat, temp);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _caluRaResultCoefSeqSS(response, predictors,
            countp, num, result, &rdescstat, temp);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        result->rr_dbS = sqrt(result->rr_rraAnova.rra_dbResidErrorMS);
        result->rr_dbRSq = result->rr_rraAnova.rra_dbRegressionSS * 100 /
            result->rr_rraAnova.rra_dbTotalSS;
        dbt = result->rr_dbS / rdescstat.ds_dbStDev;
        result->rr_dbRSqAdj = (1 - dbt * dbt) * 100;

    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _caluRaResultUnusualObserv(response, predictors,
                   countp, num, result, &rdescstat, temp);
    }

    if (temp != NULL)
        xfree((void **)&temp);

    return u32Ret;
}

static void _printRaResultCoefBrief(ra_result_coef_t * coef)
{
    clieng_caption_t * pcc = &ls_ccRaResultCoefBrief[0];
    olchar_t strInfo[MAX_OUTPUT_LINE_LEN], strField[MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Predictor */
    cliengAppendBriefColumn(pcc, strInfo, coef->rrc_strPredictor);
    pcc++;

    /* Coef */
    ol_sprintf(strField, "%.7f", coef->rrc_dbCoef);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SECoef */
    ol_sprintf(strField, "%.7f", coef->rrc_dbSECoef);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* T */
    ol_sprintf(strField, "%.2f", coef->rrc_dbT);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* P */
    ol_sprintf(strField, "%.2f", coef->rrc_dbP);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    cliengOutputRawLine(strInfo);
}

static void _printRaResultAnovaBrief(ra_result_t * result)
{
    clieng_caption_t * pcc = &ls_ccRaResultAnovaBrief[0];
    olchar_t strInfo[MAX_OUTPUT_LINE_LEN], strField[MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Source */
    cliengAppendBriefColumn(pcc, strInfo, "Regression");
    pcc++;

    /* DF */
    ol_sprintf(strField, "%d", result->rr_rraAnova.rra_nRegressionDF);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SS */
    ol_sprintf(strField, "%.3f", result->rr_rraAnova.rra_dbRegressionSS);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* MS */
    ol_sprintf(strField, "%.3f", result->rr_rraAnova.rra_dbRegressionMS);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* F */
    ol_sprintf(strField, "%.3f", result->rr_rraAnova.rra_dbRegressionF);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* P */
    ol_sprintf(strField, "%.3f", result->rr_rraAnova.rra_dbRegressionP);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    cliengOutputRawLine(strInfo);


    pcc = &ls_ccRaResultAnovaBrief[0];
    strInfo[0] = '\0';

    /* Source */
    cliengAppendBriefColumn(pcc, strInfo, "Residual Error");
    pcc++;

    /* DF */
    ol_sprintf(strField, "%d", result->rr_rraAnova.rra_nResidErrorDF);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SS */
    ol_sprintf(strField, "%.3f", result->rr_rraAnova.rra_dbResidErrorSS);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* MS */
    ol_sprintf(strField, "%.3f", result->rr_rraAnova.rra_dbResidErrorMS);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    cliengOutputRawLine(strInfo);


    pcc = &ls_ccRaResultAnovaBrief[0];
    strInfo[0] = '\0';

    /* Source */
    cliengAppendBriefColumn(pcc, strInfo, "Total");
    pcc++;

    /* DF */
    ol_sprintf(strField, "%d", result->rr_rraAnova.rra_nTotalDF);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SS */
    ol_sprintf(strField, "%.3f", result->rr_rraAnova.rra_dbTotalSS);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    cliengOutputRawLine(strInfo);
}

static void _printRaResultCoefSeqSSBrief(ra_result_coef_t * coef)
{
    clieng_caption_t * pcc = &ls_ccRaResultCoefSeqSSBrief[0];
    olchar_t strInfo[MAX_OUTPUT_LINE_LEN], strField[MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Source */
    cliengAppendBriefColumn(pcc, strInfo, coef->rrc_strPredictor);
    pcc++;

    /* DF */
    ol_sprintf(strField, "%d", coef->rrc_nDF);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SeqSS */
    ol_sprintf(strField, "%.3f", coef->rrc_dbSeqSS);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    cliengOutputRawLine(strInfo);
}

static void _printRaResultUnusualObservBrief(
    ra_result_unusual_observ_t * observ)
{
    clieng_caption_t * pcc = &ls_ccRaResultUnusualObservBrief[0];
    olchar_t strInfo[MAX_OUTPUT_LINE_LEN], strField[MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Obs */
    ol_sprintf(strField, "%d", observ->rruo_nObs);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Response */
    ol_sprintf(strField, "%.4f", observ->rruo_dbResponse);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Fit */
    ol_sprintf(strField, "%.4f", observ->rruo_dbFit);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* SEFit */
    ol_sprintf(strField, "%.4f", observ->rruo_dbSEFit);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Resid */
    ol_sprintf(strField, "%.4f", observ->rruo_dbResid);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* StResid */
    ol_sprintf(strField, "%.2f", observ->rruo_dbStResid);
    if (observ->rruo_nType & RA_UNUSUAL_OBSERV_LARGE_REDIDUAL)
        ol_strcat(strField, "R");
    if (observ->rruo_nType & RA_UNUSUAL_OBSERV_X)
        ol_strcat(strField, "X");
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    cliengOutputRawLine(strInfo);
}

static void _preSetRaResult(olchar_t * rname, olchar_t ** pname,
    olint_t countp, ra_result_t * result)
{
    olint_t i;

    ol_strncpy(result->rr_strResponse, rname,
            sizeof(result->rr_strResponse) - 1);

    ol_strncpy(result->rr_rrcConstant.rrc_strPredictor, "Constant",
            sizeof(result->rr_rrcConstant.rrc_strPredictor) - 1);

    result->rr_nCoef = countp;
    for (i = 0; i < countp; i ++)
    {
        ol_strncpy(result->rr_prrcCoef[i].rrc_strPredictor, pname[i],
                sizeof(result->rr_prrcCoef[i].rrc_strPredictor) - 1);
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

/* --- public routine section ---------------------------------------------- */
u32 regressionAnalysis(
    olchar_t * rname, oldouble_t * response, olchar_t ** pname,
    oldouble_t ** predictors, olint_t countp, olint_t num, ra_result_t * result)
{
    u32 u32Ret = OLERR_NO_ERROR;
    oldouble_t * dbx = NULL, * dby = NULL;
    olint_t i;

    if (countp + 1 >= num)
        return OLERR_INVALID_PARAM;

    u32Ret = xcalloc((void **)&dbx,
         sizeof(oldouble_t) * (countp + 1) * (countp + 1));
    if (u32Ret != OLERR_NO_ERROR)
        goto out;

    u32Ret = xcalloc((void **)&dby, sizeof(oldouble_t) * (countp + 1));
    if (u32Ret != OLERR_NO_ERROR)
        goto out;

    _preSetRaResult(rname, pname, countp, result);

    _makeMatrix(response, predictors, countp, num, dbx, dby);

    u32Ret = agaus(dbx, dby, countp + 1);
    if (u32Ret == OLERR_NO_ERROR)
    {
        result->rr_rrcConstant.rrc_dbCoef = dby[0];
        for (i = 0; i < countp; i ++)
        {
            result->rr_prrcCoef[i].rrc_dbCoef = dby[i + 1];
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
        xfree((void **)&dbx);
    if (dby != NULL)
        xfree((void **)&dby);

    return u32Ret;
}

void printRaResult(ra_result_t * result)
{
    olint_t i;
    olchar_t str[512], strt[64];

    memset(str, 0, sizeof(str));
    ol_sprintf(str, "Regression Analysis: %s versus ", result->rr_strResponse);
    for (i = 0; i < result->rr_nCoef; i ++)
    {
        ol_strcat(str, result->rr_prrcCoef[i].rrc_strPredictor);
        if (i != result->rr_nCoef - 1)
            ol_strcat(str, ", ");
    }
    cliengOutputLine("%s\n", str);

    ol_sprintf(str, "The regression equation is");
    cliengOutputLine("%s", str);

    ol_sprintf(str, "%s = %.6f", result->rr_strResponse,
            result->rr_rrcConstant.rrc_dbCoef);
    for (i = 0; i < result->rr_nCoef; i ++)
    {
        if (result->rr_prrcCoef[i].rrc_dbCoef >= 0.0)
            ol_strcat(str, " + ");
        else
            ol_strcat(str, " - ");
        ol_sprintf(strt, "%.6f", fabs(result->rr_prrcCoef[i].rrc_dbCoef));
        ol_strcat(str, strt);
        ol_strcat(str, " ");
        ol_strcat(str, result->rr_prrcCoef[i].rrc_strPredictor);
    }
    cliengOutputLine("%s\n\n", str);

    cliengPrintHeader(ls_ccRaResultCoefBrief,
             sizeof(ls_ccRaResultCoefBrief) / sizeof(clieng_caption_t));
    _printRaResultCoefBrief(&result->rr_rrcConstant);
    for (i = 0; i < result->rr_nCoef; i ++)
    {
        _printRaResultCoefBrief(&result->rr_prrcCoef[i]);        
    }


    ol_sprintf(str, "\n\nS = %.5f   R-Sq = %.1f%%   R-Sq(adj) = %.1f%%",
            result->rr_dbS, result->rr_dbRSq, result->rr_dbRSqAdj);
    cliengOutputRawLine(str);

    cliengOutputLine("\n\nAnalysis of Variance\n");
    cliengPrintHeader(ls_ccRaResultAnovaBrief,
             sizeof(ls_ccRaResultAnovaBrief) / sizeof(clieng_caption_t));
    _printRaResultAnovaBrief(result);

    cliengOutputLine("\n\n");
    cliengPrintHeader(ls_ccRaResultCoefSeqSSBrief,
        sizeof(ls_ccRaResultCoefSeqSSBrief) / sizeof(clieng_caption_t));
    for (i = 0; i < result->rr_nCoef; i ++)
        _printRaResultCoefSeqSSBrief(&result->rr_prrcCoef[i]);

    cliengOutputLine("\n\nUnusual Observations\n");
    cliengPrintHeader(ls_ccRaResultUnusualObservBrief,
        sizeof(ls_ccRaResultUnusualObservBrief) / sizeof(clieng_caption_t));
    for (i = 0; i < result->rr_nObserv; i ++)
        _printRaResultUnusualObservBrief(&result->rr_prruoObserv[i]);

}

/*---------------------------------------------------------------------------*/

