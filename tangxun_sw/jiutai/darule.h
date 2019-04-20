/**
 *  @file darule.h
 *
 *  @brief Base rules library
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
#include "stocklist.h"
#include "indicator.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    u8 drnduimdp_u8NDays;
    u8 drnduimdp_u8MDays;
    u8 drnduimdp_u8Reserved[14];
} da_rule_n_days_up_in_m_days_param_t;

typedef struct
{
    u8 drhloldp_u8Reserved[16];
} da_rule_high_limit_of_last_day_param_t;

typedef struct
{
    u8 drlloldp_u8Reserved[16];
} da_rule_low_limit_of_last_day_param_t;

typedef struct
{
#define IN_BOTTOM_AREA_THRESHOLD  (15.0)
    u8 dribap_u8Threshold;
    u8 dribap_u8Reserved[15];
} da_rule_in_bottom_area_param_t;

typedef struct
{
    u32 drmnodsp_u32MinDay;
} da_rule_min_num_of_day_summary_param_t;

typedef struct
{
#define FLAG_UP_RECTANGLE_DAYS         (50)
    u32 drfp_u32UpRectangleDays;
#define FLAG_MIN_RECTANGLE_DAYS        (20) /*at least 3 weeks*/
    u32 drfp_u32MinRectangleDays;          /**< rectangle days, from ramping day */
    boolean_t drfp_bRampingHighHighLimit;  /**< high price reach high limit in the ramping day*/
    boolean_t drfp_bRampingCloseHighLimit; /**< close price reach high limit in the ramping day*/
    u8 drfp_u8Reserved[6];
#define FLAG_RAMPING_HIGH_PRICE_RATE   (7.0)  
    double drfp_dbRampingHighPriceRate;    /**< high price rate in the ramping day*/
#define FLAG_RAMPING_CLOSE_PRICE_RATE  (7.0)  
    double drfp_dbRampingClosePriceRate;   /**< close price rate in the ramping day*/
    u32 drfp_u32Reserved[4];
} da_rule_flag_param_t;

typedef struct
{
/* IN */
#define RECTANGLE_MIN_DAYS           (40)
    u32 drrp_u32MinDays;
#define RECTANGLE_MAX_DAYS           (2 * RECTANGLE_MIN_DAYS)
    u32 drrp_u32MaxDays;
    boolean_t drrp_bCheckEdge; /**< check the edge, the trading days in rising edge
                                  should be more than days in falling edge */
    boolean_t drrp_bBelowPressureArea; /**< make sure it's below pressure area*/
    boolean_t drrp_bNoHighHighLimit; /*no high price reach high limit in rectangle*/
    u8 drrp_u8Reserved[5];
    u32 drrp_u32Reserved[4];
#define RECTANGLE_POINT_THRESHOLD     (0.02)
    double drrp_dbPointThreshold; /**< rectangle point threshold, the points are treated as
                                    in the line between this threshold*/
#define RECTANGLE_PRESSURE_AREA       (0.03)
    double drrp_dbPressureArea; /*rectangle pressure area*/
/* OUT */
#define RECTANGLE_LEFT_UPPER    (0)
#define RECTANGLE_LEFT_LOWER    (1)
#define RECTANGLE_RIGHT_UPPER   (2)
#define RECTANGLE_RIGHT_LOWER   (3)
    da_day_summary_t * drrp_pddsRectangle[4];
} da_rule_rectangle_param_t;

typedef struct
{
    u32 drurtp_u32Reserved[4];
} da_rule_up_rise_triangle_param_t;

typedef struct
{
    boolean_t drmrdp_bHighPrice;
    boolean_t drmrdp_bClosePrice;
    u8 drmrdp_u8Reserved[6];
    double drmrdp_dbHighPriceRate;
    double drmrdp_dbClosePriceRate;
    u32 drmrdp_u32MinHighPriceDay;
    u32 drmrdp_u32MinClosePriceDay;
} da_rule_min_ramping_day_param_t;

typedef struct
{
    boolean_t drmhldp_bHighHighLimit;
    boolean_t drmhldp_bCloseHighLimit;
    u8 drmhldp_u8Reserved[6];
    u32 drmhldp_u32MinHighHighLimitDay;
    u32 drmhldp_u32MinCloseHighLimitDay;
} da_rule_min_high_limit_day_param_t;

typedef struct
{
    double drmavrdp_dbRatio;
    u32 drmavrdp_u32MinDay;
} da_rule_min_abnormal_vol_ratio_day_param_t;

typedef struct
{
#define INDICATOR_MACD_SHORT_DAYS   (DEF_MACD_SHORT_DAYS)
    olint_t drimdubdp_nMacdShortDays;
#define INDICATOR_MACD_LONG_DAYS    (DEF_MACD_LONG_DAYS)    
    olint_t drimdubdp_nMacdLongDays;
#define INDICATOR_MACD_M_DAYS       (DEF_MACD_M_DAYS)
    olint_t drimdubdp_nMacdMDays;
} da_rule_indicator_macd_diff_up_break_dea_param_t;

typedef union
{
    da_rule_n_days_up_in_m_days_param_t drp_drnduimdpDyasUp;
    da_rule_high_limit_of_last_day_param_t drp_drhloldpHighLimit;
    da_rule_low_limit_of_last_day_param_t drp_drlloldpLowLimit;
    da_rule_in_bottom_area_param_t drp_dribapBottom;
    da_rule_min_num_of_day_summary_param_t drp_drmnodspMinDay;
    da_rule_flag_param_t drp_drfpFlag;
    da_rule_rectangle_param_t drp_drrpRectangle;
    da_rule_up_rise_triangle_param_t drp_drurtpUpRiseTriangle;
    da_rule_min_ramping_day_param_t drp_drmrdpMinRampingDay;
    da_rule_min_high_limit_day_param_t drp_drmhldpMinHighLimitDay;
    da_rule_min_abnormal_vol_ratio_day_param_t drp_drmavrdpAbnormalVolRatioDay;
    da_rule_indicator_macd_diff_up_break_dea_param_t drp_drimpdubdpIndicatorMacdDiffUpBreakDea;
} da_rule_param_t;

typedef u32 (* fnExecStocksRule_t)(
    stock_info_t * stockinfo, da_day_summary_t * buffer, int total, da_rule_param_t * pdrp);

typedef struct
{
    char * dr_pstrName;
    fnExecStocksRule_t dr_fnExecRule;
} da_rule_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 getNumOfDaRules(void);

u32 getAllDaRules(da_rule_t ** ppRule);

da_rule_t * getFirstDaRule(void);

da_rule_t * getNextDaRule(da_rule_t * pRule);

u32 getDaRule(char * name, da_rule_t ** ppRule);

void printDaRuleBrief(da_rule_t * pRule, u32 num);

u32 initDaRule(void);

u32 finiDaRule(void);

#endif /*TANGXUN_DARULE_H*/

/*------------------------------------------------------------------------------------------------*/


