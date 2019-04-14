/**
 *  @file indicator.h
 *
 *  @brief Indicator system
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/*  DMI (trend)
 *    TR := SUM(MAX(MAX(HIGH-LOW,ABS(HIGH-REF(CLOSE,1))),ABS(LOW-REF(CLOSE,1))),N);
 *    HD := HIGH-REF(HIGH,1);
 *    LD := REF(LOW,1)-LOW;
 *    DMP:= SUM(IF(HD>0 AND HD>LD,HD,0),N);
 *    DMM:= SUM(IF(LD>0 AND LD>HD,LD,0),N);
 *    PDI: DMP*100/TR;
 *    MDI: DMM*100/TR;
 *    ADX: MA(ABS(MDI-PDI)/(MDI+PDI)*100,M);
 *    ADXR:(ADX+REF(ADX,M))/2;
 *  
 *  MACD (trend)
 *    DIFF: EMA(CLOSE,SHORT) - EMA(CLOSE,LONG),colorwhite;
 *    DEA: EMA(DIFF,M),coloryellow;
 *    MACD: 2*(DIFF-DEA), COLORSTICK
 *
 *  MTM  (trend)
 *    MTM : CLOSE-REF(CLOSE,N);
 *    MTMMA : MA(MTM,N1);
 *
 *  RSI  (anti-trend)
 *    LC := REF(CLOSE,1);
 *    RSI1:SMA(MAX(CLOSE-LC,0),N1,1)/SMA(ABS(CLOSE-LC),N1,1)*100;
 *    RSI2:SMA(MAX(CLOSE-LC,0),N2,1)/SMA(ABS(CLOSE-LC),N2,1)*100;
 *    RSI3:SMA(MAX(CLOSE-LC,0),N3,1)/SMA(ABS(CLOSE-LC),N3,1)*100;
 *  
 *  KDJ  (anti-trend)
 *    RSV:=(CLOSE-LLV(LOW,N))/(HHV(HIGH,N)-LLV(LOW,N))*100;
 *    K:SMA(RSV,M1,1);
 *    D:SMA(K,M2,1);
 *    J:3*K-2*D;
 *
 *  ASI  (trend)
 *    LC:=REF(CLOSE,1);
 *    AA:=ABS(HIGH-LC);
 *    BB:=ABS(LOW-LC);
 *    CC:=ABS(HIGH-REF(LOW,1));
 *    DD:=ABS(LC-REF(OPEN,1));
 *    R:=IF(AA>BB AND AA>CC,AA+BB/2+DD/4,IF(BB>CC AND BB>AA,BB+AA/2+DD/4,CC+DD/4));
 *    X:=(CLOSE-LC+(CLOSE-OPEN)/2+LC-REF(OPEN,1));
 *    SI:=16*X/R*MAX(AA,BB);
 *    asi:SUM(SI,0);
 *    MASI:MA(asi,6);
 *
 *  ATR  (swing)
 *    TR :=MAX(MAX((HIGH-LOW),ABS(REF(CLOSE,1)-HIGH)),ABS(REF(CLOSE,1)-LOW));
 *    ATR: MA(TR,N);
 *    MATR: MA(ATR,N);
 *
 *  OBV  (volumn-price)
 *    SUM(IF(CLOSE>REF(CLOSE,1),VOL,IF(CLOSE<REF(CLOSE,1),-VOL,0)),0)
 *  
 *  @remarks
 *    - 10/26/2005 :
 *       -# Initial version
 */

#ifndef TANGXUN_JIUTAI_INDICATOR_H
#define TANGXUN_JIUTAI_INDICATOR_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "parsedata.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

enum stock_indicator
{
    STOCK_INDICATOR_UNKNOWN = 0,
    STOCK_INDICATOR_DMI,
    STOCK_INDICATOR_MACD,
    STOCK_INDICATOR_MTM,
    STOCK_INDICATOR_KDJ,
    STOCK_INDICATOR_RSI,
    STOCK_INDICATOR_ASI,
    STOCK_INDICATOR_ATR,
    STOCK_INDICATOR_OBV,
    STOCK_INDICATOR_MAX,
};

typedef struct da_adxr
{
    oldouble_t da_dbDx;
	oldouble_t da_dbAdx;
	oldouble_t da_dbAdxr;
	oldouble_t da_dbAdxrTrend;
/*K Chart Pattern*/
#define KCP_UNKNOWN     0
#define KCP_TREND       1
#define KCP_ANTI_TREND  2
    olint_t da_nKcp;
} da_adxr_t;

/*directional movement index
 *DMP: Directional Movement Plus
 *DMM: Directional Movement Minus
 *PDI: Plus Directional Movement Index
 *MDI: Minus Directional Movement Index
 *ADX: Average Directional Movement Index
 *ADXR: Average Directional Movement Index Ratio*/
typedef struct da_dmi_param
{
#define DEF_DMI_DAYS  14
#define DEF_DMI_ADX_MA_DAYS    6
#define DEF_DMI_ADXR_TREND   25.0  /*above this, use trend system*/
	olint_t ddp_nDmiDays;
	olint_t ddp_nDmiAdxMaDays;
	oldouble_t ddp_dbAdxrTrend;
} da_dmi_param_t;

typedef struct da_dmi
{
	oldouble_t dd_dbTr14;  /*sum of several days, may be 13, 15, ...*/
	oldouble_t dd_dbDmp14;
	oldouble_t dd_dbDmm14;
	oldouble_t dd_dbPdi;
	oldouble_t dd_dbMdi;
	oldouble_t dd_dbDmi;
	oldouble_t dd_dbAdx;
	oldouble_t dd_dbAdxr;
} da_dmi_t;

/*Moving Average Convergence / Divergence*/
typedef struct da_macd_param
{
#define DEF_MACD_SHORT_DAYS  12
#define DEF_MACD_LONG_DAYS  26
#define DEF_MACD_M_DAYS  9
	olint_t dmp_nMacdShortDays;
	olint_t dmp_nMacdLongDays;
    olint_t dmp_nMacdMDays;
} da_macd_param_t;

typedef struct da_macd
{
	oldouble_t dm_dbShortEma;
	oldouble_t dm_dbLongEma;
	oldouble_t dm_dbDiff;
    oldouble_t dm_dbDea;
    oldouble_t dm_dbMacd;
} da_macd_t;

typedef struct da_mtm_param
{
#define DEF_MTM_DAYS  12
#define DEF_MTM_MA_DAYS  6
	olint_t dmp_nMtmDays;
	olint_t dmp_nMtmMaDays;
} da_mtm_param_t;

typedef struct da_mtm
{
    oldouble_t dm_dbMtm;
    oldouble_t dm_dbMaMtm;
} da_mtm_t;

typedef struct da_rsi_param
{
#define DEF_RSI1_DAYS  6
#define DEF_RSI2_DAYS  12
#define DEF_RSI3_DAYS  24
#define DEF_RSI_MAX_OPENING_1  15.0 //15.0
#define DEF_RSI_MIN_CLOSEOUT_1 85.0
	olint_t drp_nRsi1Days;
	olint_t drp_nRsi2Days;
	olint_t drp_nRsi3Days;
    oldouble_t drp_dbMaxOpening1;
    oldouble_t drp_dbMinCloseout1;
} da_rsi_param_t;

typedef struct da_rsi
{
    oldouble_t dr_dbSmaRsi1P;
    oldouble_t dr_dbSmaRsi2P;
    oldouble_t dr_dbSmaRsi3P;
    oldouble_t dr_dbSmaRsi1A;
    oldouble_t dr_dbSmaRsi2A;
    oldouble_t dr_dbSmaRsi3A;

    oldouble_t dr_dbRsi1;
    oldouble_t dr_dbRsi2;
    oldouble_t dr_dbRsi3;
} da_rsi_t;

typedef struct da_kdj_param
{
#define DEF_KDJ_N_DAYS   9
#define DEF_KDJ_M1_DAYS  3
#define DEF_KDJ_M2_DAYS  3
#define DEF_KDJ_MAX_OPENING_D   30.0 //20.0
#define DEF_KDJ_MIN_CLOSEOUT_J  100.0
	olint_t dkp_nKdjNDays;
	olint_t dkp_nKdjM1Days;
	olint_t dkp_nKdjM2Days;
    oldouble_t dkp_dbMaxOpeningD;
    oldouble_t dkp_dbMinCloseoutJ;
} da_kdj_param_t;

typedef struct da_kdj
{
    oldouble_t dk_dbSmaK;
    oldouble_t dk_dbSmaD;

    oldouble_t dk_dbK;
    oldouble_t dk_dbD;
    oldouble_t dk_dbJ;
} da_kdj_t;

typedef struct da_asi_param
{
#define DEF_ASI_MA_DAYS  6
	olint_t dap_nAsiMaDays;
} da_asi_param_t;

typedef struct da_asi
{
    oldouble_t da_dbSi;
    oldouble_t da_dbAsi;
    oldouble_t da_dbMaAsi;
} da_asi_t;

typedef struct da_atr_param
{
#define DEF_ATR_DAYS     14
#define DEF_ATR_MA_DAYS  14
	olint_t dap_nAtrDays;
	olint_t dap_nAtrMaDays;
} da_atr_param_t;

typedef struct da_atr
{
	oldouble_t da_dbAtr;
	oldouble_t da_dbMaAtr;
} da_atr_t;

typedef struct da_obv_param
{
#define DEF_OBV_DAYS  2
	olint_t dop_nObvDays;
} da_obv_param_t;

typedef struct da_obv
{
	oldouble_t do_dbObv;
} da_obv_t;

typedef struct
{
    union
    {
        da_dmi_param_t dip_ddpDmi;
        da_macd_param_t dip_dmpMacd;
        da_mtm_param_t dip_dmpMtm;
        da_rsi_param_t dip_drpRsi;
        da_kdj_param_t dip_dkpKdj;
        da_asi_param_t dip_dapAsi;
        da_atr_param_t dip_dapAtr;
        da_obv_param_t dip_dopObv;
    };
    oldouble_t dip_dbReserved;
} da_indicator_param_t;

#define OPTIMIZE_INDI_DAY_SUMMARY  40  /*2 months*/
#define TOTAL_OPTIMIZE_INDI_DAY_SUMMARY  (100 + OPTIMIZE_INDI_DAY_SUMMARY) 

struct da_indicator_desc;

typedef void (* fnGetStringIndicatorParam_t)(
    struct da_indicator_desc * indi, const da_indicator_param_t * pdip, olchar_t * buf);
typedef void (* fnGetIndicatorParamFromString_t)(
    struct da_indicator_desc * indi, da_indicator_param_t * pdip, const olchar_t * buf);

typedef struct da_indicator_desc
{
    olint_t did_nId;
#define INDI_TYPE_UNKNOWN     0
#define INDI_TYPE_TREND       1
#define INDI_TYPE_ANTI_TREND  2
    olint_t did_nType;
    olchar_t * did_pstrName;
    olchar_t * did_pstrDesc;
    fnGetStringIndicatorParam_t did_fnGetStringParam;
    fnGetIndicatorParamFromString_t did_fnGetParamFromString;
} da_indicator_desc_t;

typedef struct
{
    boolean_t goip_bVerbose;
    u8 goip_u8Reserved[7];
    u32 goip_u32Reserved[8];
    oldouble_t goip_dbMinProfitTimeRatio;
} get_optimized_indicator_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

da_indicator_desc_t * getDaIndicatorDesc(olint_t id);

da_indicator_desc_t * getDaIndicatorDescByName(olchar_t * name);

u32 freeDaDaySummaryIndicator(da_day_summary_t *summary, olint_t num);

u32 getIndicatorAdxrTrend(da_day_summary_t * buffer, olint_t num);

olchar_t * getStringKcp(olint_t nKcp);
olchar_t * getAbbrevStringKcp(olint_t nKcp);

olchar_t * getStringIndicatorName(olint_t nIndicator);

olchar_t * getStringIndiType(olint_t type);

#endif /*TANGXUN_JIUTAI_INDICATOR_H*/

/*------------------------------------------------------------------------------------------------*/


