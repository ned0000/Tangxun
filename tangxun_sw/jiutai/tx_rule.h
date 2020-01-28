/**
 *  @file tx_rule.h
 *
 *  @brief Basic rules library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_RULE_H
#define TANGXUN_RULE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "tx_daysummary.h"
#include "tx_stock.h"
#include "tx_indi.h"

/* --- constant definitions --------------------------------------------------------------------- */

typedef enum
{
/* limit */
    TX_RULE_ID_HIGH_LIMIT_OF_LAST_DAY,
    TX_RULE_ID_LOW_LIMIT_OF_LAST_DAY,
    TX_RULE_ID_MIN_HIGH_LIMIT_DAY,
    TX_RULE_ID_NO_HIGH_HIGH_LIMIT_DAY,
/* bottom */
    TX_RULE_ID_IN_BOTTOM_AREA,
/* st */
    TX_RULE_ID_NOT_ST_RELATED,
    TX_RULE_ID_ST,
    TX_RULE_ID_ST_DELISTING,
/* misc */
    TX_RULE_ID_MIN_NUM_OF_DAY_SUMMARY,
/* rectangle */
    TX_RULE_ID_RECTANGLE,
/* price */
    TX_RULE_ID_N_DAYS_UP_IN_M_DAYS,
    TX_RULE_ID_MIN_RAMPING_DAY,
    TX_RULE_ID_NEED_STOP_LOSS,
    TX_RULE_ID_PRICE_VOLATILITY,
/* vol */
    TX_RULE_ID_MIN_ABNORMAL_VOL_RATIO_DAY,
/* indi macd */
    TX_RULE_ID_INDI_MACD_DIFF_UP_BREAK_DEA,
    TX_RULE_ID_INDI_MACD_POSITIVE_DIFF_DEA,
/* line */
    TX_RULE_ID_PRESSURE_LINE,
/* last id */
    TX_RULE_ID_MAX,
} tx_rule_id_t;

/* --- data structures -------------------------------------------------------------------------- */

/** Parameter for rule "highLimitOfLastDay".
 */
typedef struct
{
    u8 trhloldp_u8Reserved[16];
} tx_rule_high_limit_of_last_day_param_t;

/** Parameter for rule "lowLimitOfLastDay".
 */
typedef struct
{
    u8 trlloldp_u8Reserved[16];
} tx_rule_low_limit_of_last_day_param_t;

/** Parameter for rule "minHighLimitDay".
 */
typedef struct
{
    /**high price reach high limit.*/
    boolean_t trmhldp_bHighHighLimit;
    boolean_t trmhldp_bCloseHighLimit;
    u8 trmhldp_u8Reserved[6];
    u32 trmhldp_u32MinHighHighLimitDay;
    u32 trmhldp_u32MinCloseHighLimitDay;
} tx_rule_min_high_limit_day_param_t;

/** Parameter for rule "noHighHighLimitDay".
 */
typedef struct
{
    u8 trnhhldp_u8Reserved[32];
} tx_rule_no_high_high_limit_day_param_t;

/** Parameter for rule "inBottomArea".
 */
typedef struct
{
#define TX_RULE_IN_BOTTOM_AREA_THRESHOLD  (15.0)
    u8 tribap_u8Threshold;
    u8 tribap_u8Reserved[15];
} tx_rule_in_bottom_area_param_t;

/** Parameter for rule "notStRelated".
 */
typedef struct
{
    u8 trnsrp_u8Reserved[32];
} tx_rule_not_st_related_param_t;

/** Parameter for rule "st".
 */
typedef struct
{
    u8 trsp_u8Reserved[32];
} tx_rule_st_param_t;

/** Parameter for rule "stDelisting".
 */
typedef struct
{
    u8 trsdp_u8Reserved[32];
} tx_rule_st_delisting_param_t;

/** Parameter for rule "minNumOfDaySummary".
 */
typedef struct
{
    u32 trmnodsp_u32MinDay;
} tx_rule_min_num_of_day_summary_param_t;

/** Parameter for rule "rectangle".
 */
typedef struct
{
/* IN */
#define TX_RULE_RECTANGLE_MIN_DAYS           (40)
    u32 trrp_u32MinDays;
#define TX_RULE_RECTANGLE_MAX_DAYS           (2 * TX_RULE_RECTANGLE_MIN_DAYS)
    u32 trrp_u32MaxDays;
    /**Check the edge, the trading days in rising edge should be more than days in falling edge.*/
    boolean_t trrp_bCheckEdge;

    u8 trrp_u8Reserved[7];
    u32 trrp_u32Reserved[4];
    /**Rectangle point threshold, the points are treated as in the line between this threshold.*/
#define TX_RULE_RECTANGLE_POINT_THRESHOLD     (0.02)
    oldouble_t trrp_dbPointThreshold;

/* OUT */
#define TX_RULE_RECTANGLE_LEFT_UPPER    (0)
#define TX_RULE_RECTANGLE_LEFT_LOWER    (1)
#define TX_RULE_RECTANGLE_RIGHT_UPPER   (2)
#define TX_RULE_RECTANGLE_RIGHT_LOWER   (3)
    tx_ds_t * trrp_ptdRectangle[4];
} tx_rule_rectangle_param_t;

/** Parameter for rule "nDaysUpIn3Days".
 */
typedef struct
{
    u8 trnduimdp_u8NDays;
    u8 trnduimdp_u8MDays;
    u8 trnduimdp_u8Reserved[14];
} tx_rule_n_days_up_in_m_days_param_t;

/** Parameter for rule "minRampingDay".
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

/** Parameter for rule "needStopLoss".
 */
typedef struct
{
/* IN */
    oldouble_t trnslp_dbBuyPrice;
#define TX_RULE_NEED_STOP_LOSS_RATIO    (0.03)
    oldouble_t trnslp_dbRatio;
/* OUT */
    oldouble_t trnslp_dbStopLossPrice;
} tx_rule_need_stop_loss_param_t;

/** Parameter for rule "priceVolatility".
 */
typedef struct
{
    /**The volatility >= expected.*/
#define TX_RULE_PRICE_VOLATILITY_CONDITION_GREATER_EQUAL   (0x0)
    /**The volatility <= expected.*/
#define TX_RULE_PRICE_VOLATILITY_CONDITION_LESSER_EQUAL    (0x1)
    u8 trpvp_u8Condition;
    u8 trpvp_u8Reserved[7];
#define TX_RULE_PRICE_VOLATILITY_RATIO   (0.1)
    oldouble_t trpvp_dbVolatility;
} tx_rule_price_volatility_param_t;

/** Parameter for rule "minAbnormalVolRatioDay".
 */
typedef struct
{
    oldouble_t trmavrdp_dbRatio;
    u32 trmavrdp_u32MinDay;
} tx_rule_min_abnormal_vol_ratio_day_param_t;

/** Parameter for rule "indiMacdDiffUpBreakDea".
 */
typedef struct
{
#define TX_RULE_INDI_MACD_SHORT_DAYS   (TX_INDI_DEF_MACD_SHORT_DAYS)
    olint_t trimdubdp_nMacdShortDays;
#define TX_RULE_INDI_MACD_LONG_DAYS    (TX_INDI_DEF_MACD_LONG_DAYS)    
    olint_t trimdubdp_nMacdLongDays;
#define TX_RULE_INDI_MACD_M_DAYS       (TX_INDI_DEF_MACD_M_DAYS)
    olint_t trimdubdp_nMacdMDays;
} tx_rule_indi_macd_diff_up_break_dea_param_t;

/** Parameter for rule "indiMacdDiffUpBreakDea".
 */
typedef struct
{
    olint_t trimpddp_nMacdShortDays;
    olint_t trimpddp_nMacdLongDays;
    olint_t trimpddp_nMacdMDays;
} tx_rule_indi_macd_positive_diff_dea_param_t;

/** Parameter for rule "pressureLine".
 */
typedef struct
{
/* IN */
    tx_ds_t * trplp_ptdUpperLeft;
    tx_ds_t * trplp_ptdUpperRight;
    /**Check if the last day is near pressure line.*/
#define TX_RULE_PRESSURE_LINE_CONDITION_NEAR    (0x0)
    /**Check if the last day is far from pressure line.*/
#define TX_RULE_PRESSURE_LINE_CONDITION_FAR     (0x1)
    u8 trplp_u8Condition;
    u8 trplp_u8Reserved[7];
#define TX_RULE_PRESSURE_LINE_RATIO             (0.01)
    oldouble_t trplp_dbRatio;
/* OUT */
    oldouble_t trplp_dbPrice;
} tx_rule_pressure_line_param_t;

typedef u32 (* tx_rule_fnExecRule_t)(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, olint_t total, void * param);

typedef struct tx_rule
{
    u16 tr_u16Id;
    u16 tr_u16Reserved;
    u32 tr_u32Reserved;
    char * tr_pstrName;
    tx_rule_fnExecRule_t tr_fnExecRule;
} tx_rule_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 tx_rule_getNumOfRules(void);

u32 tx_rule_getAllRules(tx_rule_t ** ppRule);

tx_rule_t * tx_rule_getFirstRule(void);

tx_rule_t * tx_rule_getNextRule(tx_rule_t * pRule);

u32 tx_rule_getRuleById(const u16 u16Id, tx_rule_t ** ppRule);

u32 tx_rule_getRuleByName(const olchar_t * name, tx_rule_t ** ppRule);

u32 tx_rule_init(void);

u32 tx_rule_fini(void);

#endif /*TANGXUN_RULE_H*/

/*------------------------------------------------------------------------------------------------*/


