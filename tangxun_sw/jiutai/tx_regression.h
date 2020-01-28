/**
 *  @file regression.h
 *
 *  @brief Regression analysis.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_REGRESSION_H
#define TANGXUN_JIUTAI_REGRESSION_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */
typedef struct
{
    olchar_t trrc_strPredictor[16];

    oldouble_t trrc_dbCoef;
    oldouble_t trrc_dbSECoef;
    oldouble_t trrc_dbT;
    oldouble_t trrc_dbP;

    olint_t trrc_nDF;
    oldouble_t trrc_dbSeqSS;
} tx_regression_result_coef_t;

typedef struct
{
    olint_t trra_nRegressionDF;   /*degree of freedom, = number of predictors */
    oldouble_t trra_dbRegressionSS;  /*sum of square */
    oldouble_t trra_dbRegressionMS;  /*mean square = SS / DF*/
    oldouble_t trra_dbRegressionF;
    oldouble_t trra_dbRegressionP;

    olint_t trra_nResidErrorDF;
    oldouble_t trra_dbResidErrorSS;
    oldouble_t trra_dbResidErrorMS;

    olint_t trra_nTotalDF;
    oldouble_t trra_dbTotalSS;


} tx_regression_result_anova_t;

typedef struct
{
    olint_t trruo_nObs;
#define TX_REGRESSION_UNUSUAL_OBSERV_LARGE_REDIDUAL  (0x1)
#define TX_REGRESSION_UNUSUAL_OBSERV_X               (0x2)
    olint_t trruo_nType;
    oldouble_t trruo_dbResponse;
    oldouble_t trruo_dbFit;
    oldouble_t trruo_dbSEFit;
    oldouble_t trruo_dbResid;
    oldouble_t trruo_dbStResid;

} tx_regression_result_unusual_observ_t;

typedef struct
{
    olchar_t trr_strResponse[16];
    tx_regression_result_coef_t trr_trrcConstant;
    olint_t trr_nCoef;
    tx_regression_result_coef_t * trr_ptrrcCoef;

    oldouble_t trr_dbS;  /*standard error of regression */
    oldouble_t trr_dbRSq;  /*the percent of the variation in Response
                       that is is explained*/
    oldouble_t trr_dbRSqAdj;

    tx_regression_result_anova_t trr_trraAnova;

    olint_t trr_nMaxObserv;
    olint_t trr_nObserv;
    tx_regression_result_unusual_observ_t * trr_ptrruoObserv;

} tx_regression_result_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 tx_regression_analysis(
    olchar_t * rname, oldouble_t * response, olchar_t ** pname, oldouble_t ** predictors,
    olint_t countp, olint_t num, tx_regression_result_t * result);

void tx_regression_printResult(tx_regression_result_t * result);

#endif /*TANGXUN_JIUTAI_REGRESSION_H*/

/*------------------------------------------------------------------------------------------------*/


