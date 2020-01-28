/**
 *  @file tx_indi.h
 *
 *  @brief Indicator system.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 *  <HR>
 *
 *  @par DMI (trend)
 *  @code
 *    TR := SUM(MAX(MAX(HIGH-LOW,ABS(HIGH-REF(CLOSE,1))),ABS(LOW-REF(CLOSE,1))),N);
 *    HD := HIGH-REF(HIGH,1);
 *    LD := REF(LOW,1)-LOW;
 *    DMP:= SUM(IF(HD>0 AND HD>LD,HD,0),N);
 *    DMM:= SUM(IF(LD>0 AND LD>HD,LD,0),N);
 *    PDI: DMP*100/TR;
 *    MDI: DMM*100/TR;
 *    ADX: MA(ABS(MDI-PDI)/(MDI+PDI)*100,M);
 *    ADXR:(ADX+REF(ADX,M))/2;
 *  @endcode
 *
 *  @par MACD (trend)
 *  @code
 *    DIFF: EMA(CLOSE,SHORT) - EMA(CLOSE,LONG);
 *    DEA: EMA(DIFF,M);
 *    MACD: 2*(DIFF-DEA)
 *  @endcode
 *
 *  @par MTM  (trend)
 *  @code
 *    MTM : CLOSE-REF(CLOSE,N);
 *    MTMMA : MA(MTM,N1);
 *  @endcode
 *
 *  @par RSI  (anti-trend)
 *  @code
 *    LC := REF(CLOSE,1);
 *    RSI1:SMA(MAX(CLOSE-LC,0),N1,1)/SMA(ABS(CLOSE-LC),N1,1)*100;
 *    RSI2:SMA(MAX(CLOSE-LC,0),N2,1)/SMA(ABS(CLOSE-LC),N2,1)*100;
 *    RSI3:SMA(MAX(CLOSE-LC,0),N3,1)/SMA(ABS(CLOSE-LC),N3,1)*100;
 *  @endcode
 *
 *  @par KDJ  (anti-trend)
 *  @code
 *    RSV:=(CLOSE-LLV(LOW,N))/(HHV(HIGH,N)-LLV(LOW,N))*100;
 *    K:SMA(RSV,M1,1);
 *    D:SMA(K,M2,1);
 *    J:3*K-2*D;
 *  @endcode
 *
 *  @par ASI  (trend)
 *  @code
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
 *  @endcode
 *
 *  @ATR  (swing)
 *  @code
 *    TR :=MAX(MAX((HIGH-LOW),ABS(REF(CLOSE,1)-HIGH)),ABS(REF(CLOSE,1)-LOW));
 *    ATR: MA(TR,N);
 *    MATR: MA(ATR,N);
 *  @endcode
 *
 *  @par OBV  (volumn-price)
 *  @code
 *    SUM(IF(CLOSE>REF(CLOSE,1),VOL,IF(CLOSE<REF(CLOSE,1),-VOL,0)),0)
 *  @endcode
 */

#ifndef TANGXUN_INDI_H
#define TANGXUN_INDI_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

#include "tx_daysummary.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

typedef enum tx_indi_id
{
    TX_INDI_ID_UNKNOWN = 0,
    TX_INDI_ID_DMI,
    TX_INDI_ID_MACD,
    TX_INDI_ID_MTM,
    TX_INDI_ID_KDJ,
    TX_INDI_ID_RSI,
    TX_INDI_ID_ASI,
    TX_INDI_ID_ATR,
    TX_INDI_ID_OBV,
    TX_INDI_ID_MAX,
} tx_indi_id_t;

#if 0
typedef struct tx_indi_adxr
{
    oldouble_t tia_dbDx;
	oldouble_t tia_dbAdx;
	oldouble_t tia_dbAdxr;
	oldouble_t tia_dbAdxrTrend;
    /**K chart pattern.*/
#define KCP_UNKNOWN              (0)
#define KCP_TREND                (1)
#define KCP_ANTI_TREND           (2)
    olint_t tia_nKcp;
} tx_indi_adxr_t;
#endif

/*directional movement index
 *DMP: Directional Movement Plus
 *DMM: Directional Movement Minus
 *PDI: Plus Directional Movement Index
 *MDI: Minus Directional Movement Index
 *ADX: Average Directional Movement Index
 *ADXR: Average Directional Movement Index Ratio*/
typedef struct tx_indi_dmi_param
{
#define TX_INDI_DEF_DMI_DAYS             (14)
	olint_t tidp_nDmiDays;
#define TX_INDI_DEF_DMI_ADX_MA_DAYS      (6)
	olint_t tidp_nDmiAdxMaDays;
    /**Above this, use trend system.*/
#define TX_INDI_DEF_DMI_ADXR_TREND       (25.0)
	oldouble_t tidp_dbAdxrTrend;
} tx_indi_dmi_param_t;

typedef struct tx_indi_dmi
{
    oldouble_t tid_dbDx;
    /**K chart pattern.*/
#define TX_INDI_KCP_UNKNOWN              (0)
#define TX_INDI_KCP_TREND                (1)
#define TX_INDI_KCP_ANTI_TREND           (2)
    olint_t tid_nKcp;
	oldouble_t tid_dbPdi;
	oldouble_t tid_dbMdi;
	oldouble_t tid_dbAdx;
	oldouble_t tid_dbAdxr;
} tx_indi_dmi_t;

/*Moving Average Convergence / Divergence*/
typedef struct tx_indi_macd_param
{
#define TX_INDI_DEF_MACD_SHORT_DAYS    (12)
#define TX_INDI_DEF_MACD_LONG_DAYS     (26)
#define TX_INDI_DEF_MACD_M_DAYS        (9)
	olint_t timp_nMacdShortDays;
	olint_t timp_nMacdLongDays;
    olint_t timp_nMacdMDays;
} tx_indi_macd_param_t;

typedef struct tx_indi_macd
{
	oldouble_t tim_dbShortEma;
	oldouble_t tim_dbLongEma;
	oldouble_t tim_dbDiff;
    oldouble_t tim_dbDea;
    oldouble_t tim_dbMacd;
} tx_indi_macd_t;

typedef struct tx_indi_mtm_param
{
#define TX_INDI_DEF_MTM_DAYS           (12)
	olint_t timp_nMtmDays;
#define TX_INDI_DEF_MTM_MA_DAYS        (6)
	olint_t timp_nMtmMaDays;
} tx_indi_mtm_param_t;

typedef struct tx_indi_mtm
{
    oldouble_t tim_dbMtm;
    oldouble_t tim_dbMaMtm;
} tx_indi_mtm_t;

typedef struct tx_indi_rsi_param
{
#define TX_INDI_DEF_RSI1_DAYS          (6)
	olint_t tirp_nRsi1Days;
#define TX_INDI_DEF_RSI2_DAYS          (12)
	olint_t tirp_nRsi2Days;
#define TX_INDI_DEF_RSI3_DAYS          (24)
	olint_t tirp_nRsi3Days;
#define TX_INDI_DEF_RSI_MAX_OPENING_1  (15.0) //15.0
    oldouble_t tirp_dbMaxOpening1;
#define TX_INDI_DEF_RSI_MIN_CLOSEOUT_1 (85.0)
    oldouble_t tirp_dbMinCloseout1;
} tx_indi_rsi_param_t;

typedef struct tx_indi_rsi
{
    oldouble_t tir_dbSmaRsi1P;
    oldouble_t tir_dbSmaRsi2P;
    oldouble_t tir_dbSmaRsi3P;
    oldouble_t tir_dbSmaRsi1A;
    oldouble_t tir_dbSmaRsi2A;
    oldouble_t tir_dbSmaRsi3A;

    oldouble_t tir_dbRsi1;
    oldouble_t tir_dbRsi2;
    oldouble_t tir_dbRsi3;
} tx_indi_rsi_t;

typedef struct tx_indi_kdj_param
{
#define TX_INDI_DEF_KDJ_N_DAYS          (9)
	olint_t tikp_nKdjNDays;
#define TX_INDI_DEF_KDJ_M1_DAYS         (3)
	olint_t tikp_nKdjM1Days;
#define TX_INDI_DEF_KDJ_M2_DAYS         (3)
	olint_t tikp_nKdjM2Days;
#define TX_INDI_DEF_KDJ_MAX_OPENING_D   (30.0) //20.0
    oldouble_t tikp_dbMaxOpeningD;
#define TX_INDI_DEF_KDJ_MIN_CLOSEOUT_J  (100.0)
    oldouble_t tikp_dbMinCloseoutJ;
} tx_indi_kdj_param_t;

typedef struct tx_indi_kdj
{
    oldouble_t tik_dbSmaK;
    oldouble_t tik_dbSmaD;

    oldouble_t tik_dbK;
    oldouble_t tik_dbD;
    oldouble_t tik_dbJ;
} tx_indi_kdj_t;

typedef struct tx_indi_asi_param
{
#define TX_INDI_DEF_ASI_MA_DAYS          (6)
	olint_t tiap_nAsiMaDays;
} tx_indi_asi_param_t;

typedef struct tx_indi_asi
{
    oldouble_t tia_dbSi;
    oldouble_t tia_dbAsi;
    oldouble_t tia_dbMaAsi;
} tx_indi_asi_t;

typedef struct tx_indi_atr_param
{
#define TX_INDI_DEF_ATR_DAYS             (14)
	olint_t tiap_nAtrDays;
#define TX_INDI_DEF_ATR_MA_DAYS          (14)
	olint_t tiap_nAtrMaDays;
} tx_indi_atr_param_t;

typedef struct tx_indi_atr
{
	oldouble_t tia_dbAtr;
	oldouble_t tia_dbMaAtr;
} tx_indi_atr_t;

typedef struct tx_indi_obv_param
{
#define TX_INDI_DEF_OBV_DAYS  2
	olint_t tiop_nObvDays;
} tx_indi_obv_param_t;

typedef struct tx_indi_obv
{
	oldouble_t tio_dbObv;
} tx_indi_obv_t;

#define TX_INDI_OPTIMIZE_DAY_SUMMARY  40  /*2 months*/
#define TX_INDI_TOTAL_OPTIMIZE_DAY_SUMMARY  (100 + TX_INDI_OPTIMIZE_DAY_SUMMARY) 

typedef union
{
    tx_indi_dmi_param_t tip_tidpDmi;
    tx_indi_macd_param_t tip_timpMacd;
    tx_indi_mtm_param_t tip_timpMtm;
    tx_indi_rsi_param_t tip_tirRsi;
    tx_indi_kdj_param_t tip_tikKdj;
    tx_indi_asi_param_t tip_tiapAsi;
    tx_indi_atr_param_t tip_tiapAtr;
    tx_indi_obv_param_t tip_tiopObv;
} tx_indi_param_t;

struct tx_indi;

typedef u32 (* tx_indi_fnGetStringParam_t)(
    struct tx_indi * indi, const void * param, olchar_t * buf);
typedef u32 (* tx_indi_fnGetParamFromString_t)(
    struct tx_indi * indi, void * param, const olchar_t * buf);
typedef u32 (* tx_indi_fnCalc_t)(
    struct tx_indi * indi, void * param, tx_ds_t * buffer, olint_t total);
typedef u32 (* tx_indi_fnSetDefaultParam_t)(struct tx_indi * indi, void * param);

typedef enum
{
    TX_INDI_TYPE_UNKNOWN = 0,
    TX_INDI_TYPE_TREND,
    TX_INDI_TYPE_ANTI_TREND,
} tx_indi_type_t;

typedef struct tx_indi
{
    olint_t ti_nId;
    olint_t ti_nType;
    olchar_t * ti_pstrName;
    olchar_t * ti_pstrDesc;
    tx_indi_fnGetStringParam_t ti_fnGetStringParam;
    tx_indi_fnGetParamFromString_t ti_fnGetParamFromString;
    tx_indi_fnCalc_t ti_fnCalc;
    tx_indi_fnSetDefaultParam_t ti_fnSetDefaultParam;
} tx_indi_t;

typedef struct
{
    boolean_t tigo_bVerbose;
    u8 tigo_u8Reserved[7];
    u32 tigo_u32Reserved[8];
    oldouble_t tigo_dbMinProfitTimeRatio;
} tx_indi_get_optimized_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 tx_indi_getIndiById(const olint_t id, tx_indi_t ** ppIndi);

u32 tx_indi_getIndiByName(const olchar_t * name, tx_indi_t ** ppIndi);

u32 tx_indi_freeDaySummaryIndi(tx_ds_t *summary, olint_t num);

olchar_t * tx_indi_getStringKcp(olint_t nKcp);

olchar_t * tx_indi_getAbbrevStringKcp(olint_t nKcp);

olchar_t * tx_indi_getStringIndiName(olint_t nIndi);

olchar_t * tx_indi_getStringIndiType(olint_t type);

#endif /*TANGXUN_INDI_H*/

/*------------------------------------------------------------------------------------------------*/


