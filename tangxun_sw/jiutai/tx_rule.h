/**
 *  @file tx_rule.h
 *
 *  @brief Base rules library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_DARULE_H
#define TANGXUN_DARULE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "parsedata.h"
#include "tx_stock.h"
#include "indicator.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Parameter for rule "highLimitOfLastDay"
 */
typedef struct
{
    u8 trhloldp_u8Reserved[16];
} tx_rule_high_limit_of_last_day_param_t;

/** Parameter for rule "lowLimitOfLastDay"
 */
typedef struct
{
    u8 trlloldp_u8Reserved[16];
} tx_rule_low_limit_of_last_day_param_t;

/** Parameter for rule "minHighLimitDay"
 */
typedef struct
{
    /** high price reach high limit */
    boolean_t trmhldp_bHighHighLimit;
    boolean_t trmhldp_bCloseHighLimit;
    u8 trmhldp_u8Reserved[6];
    u32 trmhldp_u32MinHighHighLimitDay;
    u32 trmhldp_u32MinCloseHighLimitDay;
} tx_rule_min_high_limit_day_param_t;

/** Parameter for rule "noHighHighLimitDay"
 */
typedef struct
{
    u8 trnhhldp_u8Reserved[32];
} tx_rule_no_high_high_limit_day_param_t;

/** Parameter for rule "inBottomArea"
 */
typedef struct
{
#define IN_BOTTOM_AREA_THRESHOLD  (15.0)
    u8 tribap_u8Threshold;
    u8 tribap_u8Reserved[15];
} tx_rule_in_bottom_area_param_t;

/** Parameter for rule "notStRelated"
 */
typedef struct
{
    u8 trnsrp_u8Reserved[32];
} tx_rule_not_st_related_param_t;

/** Parameter for rule "st"
 */
typedef struct
{
    u8 trsp_u8Reserved[32];
} tx_rule_st_param_t;

/** Parameter for rule "stDelisting"
 */
typedef struct
{
    u8 trsdp_u8Reserved[32];
} tx_rule_st_delisting_param_t;

/** Parameter for rule "minNumOfDaySummary"
 */
typedef struct
{
    u32 trmnodsp_u32MinDay;
} tx_rule_min_num_of_day_summary_param_t;

/** Parameter for rule "rectangle"
 */
typedef struct
{
/* IN */
#define RECTANGLE_MIN_DAYS           (40)
    u32 trrp_u32MinDays;
#define RECTANGLE_MAX_DAYS           (2 * RECTANGLE_MIN_DAYS)
    u32 trrp_u32MaxDays;
    /** check the edge, the trading days in rising edge should be more than days in falling edge */
    boolean_t trrp_bCheckEdge;

    u8 trrp_u8Reserved[7];
    u32 trrp_u32Reserved[4];
    /** rectangle point threshold, the points are treated as in the line between this threshold */
#define RECTANGLE_POINT_THRESHOLD     (0.02)
    oldouble_t trrp_dbPointThreshold;

/* OUT */
#define RECTANGLE_LEFT_UPPER    (0)
#define RECTANGLE_LEFT_LOWER    (1)
#define RECTANGLE_RIGHT_UPPER   (2)
#define RECTANGLE_RIGHT_LOWER   (3)
    da_day_summary_t * trrp_pddsRectangle[4];
} tx_rule_rectangle_param_t;

/** Parameter for rule "nDaysUpIn3Days"
 */
typedef struct
{
    u8 trnduimdp_u8NDays;
    u8 trnduimdp_u8MDays;
    u8 trnduimdp_u8Reserved[14];
} tx_rule_n_days_up_in_m_days_param_t;

/** Parameter for rule "minRampingDay"
 */
typedef struct
{
    boolean_t trmrdp_bHighPrice;
    boolean_t trmrdp_bClosePrice;
    u8 trmrdp_u8Reserved[6];
    oldouble_t trmrdp_dbHighPriceRate;
    oldouble_t trmrdp_dbClosePriceRate;
    u32 trmrdp_u32MinHighPriceDay;
    u32 trmrdp_u32MinClosePriceDay;
} tx_rule_min_ramping_day_param_t;

/** Parameter for rule "needStopLoss"
 */
typedef struct
{
/* IN */
    oldouble_t trnslp_dbBuyPrice;
#define NEED_STOP_LOSS_RATIO    (0.03)
    oldouble_t trnslp_dbRatio;
/* OUT */
    oldouble_t trnslp_dbStopLossPrice;
} tx_rule_need_stop_loss_param_t;

/** Parameter for rule "priceVolatility"
 */
typedef struct
{
    /** The volatility >= expected */
#define PRICE_VOLATILITY_CONDITION_GREATER_EQUAL   (0x0)
    /** The volatility <= expected */
#define PRICE_VOLATILITY_CONDITION_LESSER_EQUAL    (0x1)
    u8 trpvp_u8Condition;
    u8 trpvp_u8Reserved[7];
#define PRICE_VOLATILITY_RATIO   (0.1)
    oldouble_t trpvp_dbVolatility;
} tx_rule_price_volatility_param_t;

/** Parameter for rule "minAbnormalVolRatioDay"
 */
typedef struct
{
    oldouble_t trmavrdp_dbRatio;
    u32 trmavrdp_u32MinDay;
} tx_rule_min_abnormal_vol_ratio_day_param_t;

/** Parameter for rule "indiMacdDiffUpBreakDea"
 */
typedef struct
{
#define INDI_MACD_SHORT_DAYS   (DEF_MACD_SHORT_DAYS)
    olint_t trimdubdp_nMacdShortDays;
#define INDI_MACD_LONG_DAYS    (DEF_MACD_LONG_DAYS)    
    olint_t trimdubdp_nMacdLongDays;
#define INDI_MACD_M_DAYS       (DEF_MACD_M_DAYS)
    olint_t trimdubdp_nMacdMDays;
} tx_rule_indi_macd_diff_up_break_dea_param_t;

/** Parameter for rule "pressureLine"
 */
typedef struct
{
/* IN */
    da_day_summary_t * trplp_pddsUpperLeft;
    da_day_summary_t * trplp_pddsUpperRight;
    /** Check if the last day is near pressure line */
#define PRESSURE_LINE_CONDITION_NEAR    (0x0)
    /** Check if the last day is far from pressure line */
#define PRESSURE_LINE_CONDITION_FAR     (0x1)
    u8 trplp_u8Condition;
    u8 trplp_u8Reserved[7];
#define PRESSURE_LINE_RATIO             (0.01)
    oldouble_t trplp_dbRatio;
/* OUT */
    oldouble_t trplp_dbPrice;
} tx_rule_pressure_line_param_t;

typedef union
{
/* limite */
    tx_rule_high_limit_of_last_day_param_t trp_trhloldpHighLimit;
    tx_rule_low_limit_of_last_day_param_t trp_trlloldpLowLimit;
    tx_rule_min_high_limit_day_param_t trp_trmhldpMinHighLimitDay;
    tx_rule_no_high_high_limit_day_param_t trp_trnhhldpNoHighHighLimit;
/* bottom */
    tx_rule_in_bottom_area_param_t trp_tribapBottom;
/* st */
    tx_rule_not_st_related_param_t trp_trnsrpNotStRelated;
    tx_rule_st_param_t trp_trspSt;
    tx_rule_st_delisting_param_t trp_trsdpStDelisting;
/* misc */
    tx_rule_min_num_of_day_summary_param_t trp_trmnodspMinDay;
/* rectangle */
    tx_rule_rectangle_param_t trp_trrpRectangle;
/* price */
    tx_rule_n_days_up_in_m_days_param_t trp_trnduimdpNDaysUp;
    tx_rule_min_ramping_day_param_t trp_trmrdpMinRampingDay;
    tx_rule_need_stop_loss_param_t trp_trnslpNeedStopLoss;
    tx_rule_price_volatility_param_t trp_trpvpPriceVolatility;
/* vol */
    tx_rule_min_abnormal_vol_ratio_day_param_t trp_trmavrdpAbnormalVolRatioDay;
/* indi macd */
    tx_rule_indi_macd_diff_up_break_dea_param_t trp_trimpdubdpIndiMacdDiffUpBreakDea;
/* line */
    tx_rule_pressure_line_param_t trp_trplpPressureLine;
} tx_rule_param_t;

typedef u32 (* fnExecStocksRule_t)(
    tx_stock_info_t * stockinfo, da_day_summary_t * buffer, int total, tx_rule_param_t * ptrp);

typedef struct
{
    char * tr_pstrName;
    fnExecStocksRule_t tr_fnExecRule;
} tx_rule_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 tx_rule_getNumOfRules(void);

u32 tx_rule_getAllRules(tx_rule_t ** ppRule);

tx_rule_t * tx_rule_getFirstRule(void);

tx_rule_t * tx_rule_getNextRule(tx_rule_t * pRule);

u32 tx_rule_getRule(char * name, tx_rule_t ** ppRule);

void printDaRuleBrief(tx_rule_t * pRule, u32 num);

u32 tx_rule_init(void);

u32 tx_rule_fini(void);

#endif /*TANGXUN_DARULE_H*/

/*------------------------------------------------------------------------------------------------*/


