/**
 *  @file regression.h
 *
 *  @brief Regression analysis
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_REGRESSION_H
#define TANGXUN_JIUTAI_REGRESSION_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    olchar_t rrc_strPredictor[16];

    oldouble_t rrc_dbCoef;
    oldouble_t rrc_dbSECoef;
    oldouble_t rrc_dbT;
    oldouble_t rrc_dbP;

    olint_t rrc_nDF;
    oldouble_t rrc_dbSeqSS;
} ra_result_coef_t;

typedef struct
{
    olint_t rra_nRegressionDF;   /*degree of freedom, = number of predictors */
    oldouble_t rra_dbRegressionSS;  /*sum of square */
    oldouble_t rra_dbRegressionMS;  /*mean square = SS / DF*/
    oldouble_t rra_dbRegressionF;
    oldouble_t rra_dbRegressionP;

    olint_t rra_nResidErrorDF;
    oldouble_t rra_dbResidErrorSS;
    oldouble_t rra_dbResidErrorMS;

    olint_t rra_nTotalDF;
    oldouble_t rra_dbTotalSS;


} ra_result_anova_t;

typedef struct
{
    olint_t rruo_nObs;
#define RA_UNUSUAL_OBSERV_LARGE_REDIDUAL  0x1
#define RA_UNUSUAL_OBSERV_X               0x2
    olint_t rruo_nType;
    oldouble_t rruo_dbResponse;
    oldouble_t rruo_dbFit;
    oldouble_t rruo_dbSEFit;
    oldouble_t rruo_dbResid;
    oldouble_t rruo_dbStResid;

} ra_result_unusual_observ_t;

typedef struct
{
    olchar_t rr_strResponse[16];
    ra_result_coef_t rr_rrcConstant;
    olint_t rr_nCoef;
    ra_result_coef_t * rr_prrcCoef;

    oldouble_t rr_dbS;  /*standard error of regression */
    oldouble_t rr_dbRSq;  /*the percent of the variation in Response
                       that is is explained*/
    oldouble_t rr_dbRSqAdj;

    ra_result_anova_t rr_rraAnova;

    olint_t rr_nMaxObserv;
    olint_t rr_nObserv;
    ra_result_unusual_observ_t * rr_prruoObserv;

} ra_result_t;

/* --- functional routines ------------------------------------------------- */
u32 regressionAnalysis(
    olchar_t * rname, oldouble_t * response, olchar_t ** pname,
    oldouble_t ** predictors, olint_t countp, olint_t num,
    ra_result_t * result);

void printRaResult(ra_result_t * result);

#endif /*TANGXUN_JIUTAI_REGRESSION_H*/

/*---------------------------------------------------------------------------*/


