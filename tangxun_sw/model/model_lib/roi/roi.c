/**
 *  @file roi.c
 *
 *  @brief The roi model
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_mem.h"
#include "jf_string.h"

#include "tx_rule.h"
#include "roi.h"
#include "damodel.h"
#include "tradehelper.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define ROI_SETTING_NAME_LEFT_UPPER        "left_upper"
#define ROI_SETTING_NAME_LEFT_LOWER        "left_lower"
#define ROI_SETTING_NAME_RIGHT_UPPER       "right_upper"
#define ROI_SETTING_NAME_RIGHT_LOWER       "right_lower"

/* --- private routine section ------------------------------------------------------------------ */

static u32 _initDaModelRoi(da_model_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_model_roi_data_t * pdmrd = NULL;

    u32Ret = jf_mem_calloc((void **)&pdmrd, sizeof(*pdmrd));
    if (u32Ret == JF_ERR_NO_ERROR)
    {

        pdm->dm_pData = pdmrd;
    }
    
    return u32Ret;
}

static u32 _finiDaModelRoi(da_model_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pdm->dm_pData != NULL)
        jf_mem_free((void **)&pdm->dm_pData);

    return u32Ret;
}

static u32 _canBeTradedInRoi(
    struct da_model * pdm, stock_info_t * stockinfo, trade_pool_stock_t * ptps,
    da_day_summary_t * buffer, int total)
{
    u32 u32Ret = JF_ERR_NOT_READY;
    tx_rule_t * rule;
    tx_rule_param_t trp;
    tx_rule_rectangle_param_t trrp;
    da_day_summary_t * end = buffer + total - 1;

    jf_logger_logInfoMsg("can be traded in roi, %s, total: %d", stockinfo->si_strCode, total);

    u32Ret = tx_rule_getRule("minNumOfDaySummary", &rule);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        bzero(&trp, sizeof(trp));
        trp.trp_trmnodspMinDay.trmnodsp_u32MinDay = RECTANGLE_MAX_DAYS;

        u32Ret = rule->tr_fnExecRule(stockinfo, buffer, total, &trp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_rule_getRule("noHighHighLimitDay", &rule);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&trp, sizeof(trp));

        u32Ret = rule->tr_fnExecRule(
            stockinfo, end - RECTANGLE_MAX_DAYS + 1, RECTANGLE_MAX_DAYS, &trp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_rule_getRule("rectangle", &rule);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&trrp, sizeof(trrp));
        trrp.trrp_u32MinDays = RECTANGLE_MIN_DAYS;
        trrp.trrp_u32MaxDays = RECTANGLE_MAX_DAYS;
        trrp.trrp_bCheckEdge = TRUE;
        trrp.trrp_dbPointThreshold = RECTANGLE_POINT_THRESHOLD;

        u32Ret = rule->tr_fnExecRule(stockinfo, buffer, total, (tx_rule_param_t *)&trrp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_rule_getRule("priceVolatility", &rule);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&trp, sizeof(trp));
        trp.trp_trpvpPriceVolatility.trpvp_u8Condition = PRICE_VOLATILITY_CONDITION_GREATER_EQUAL;
        trp.trp_trpvpPriceVolatility.trpvp_dbVolatility = PRICE_VOLATILITY_RATIO;

        u32Ret = rule->tr_fnExecRule(
            stockinfo, trrp.trrp_pddsRectangle[RECTANGLE_LEFT_UPPER],
            end - trrp.trrp_pddsRectangle[RECTANGLE_LEFT_UPPER] + 1, &trp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_rule_getRule("pressureLine", &rule);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&trp, sizeof(trp));
        trp.trp_trplpPressureLine.trplp_pddsUpperLeft =
            trrp.trrp_pddsRectangle[RECTANGLE_LEFT_UPPER];
        trp.trp_trplpPressureLine.trplp_pddsUpperRight =
            trrp.trrp_pddsRectangle[RECTANGLE_RIGHT_UPPER];
        trp.trp_trplpPressureLine.trplp_u8Condition = PRESSURE_LINE_CONDITION_FAR;
        trp.trp_trplpPressureLine.trplp_dbRatio = PRICE_VOLATILITY_RATIO * 2 / 3;

        u32Ret = rule->tr_fnExecRule(
            stockinfo, trrp.trrp_pddsRectangle[RECTANGLE_LEFT_UPPER],
            end - trrp.trrp_pddsRectangle[RECTANGLE_LEFT_UPPER] + 1, &trp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        initTradePoolStock(ptps, stockinfo->si_strCode, pdm->dm_strName);
        strcpy(ptps->tps_strAddDate, end->dds_strDate);
        snprintf(
            ptps->tps_strModelParam, MAX_TRADE_MODEL_PARAM_LEN,
            "%s=%s,%s=%s,%s=%s,%s=%s",
            ROI_SETTING_NAME_LEFT_UPPER,
            trrp.trrp_pddsRectangle[RECTANGLE_LEFT_UPPER]->dds_strDate,
            ROI_SETTING_NAME_LEFT_LOWER,
            trrp.trrp_pddsRectangle[RECTANGLE_LEFT_LOWER]->dds_strDate,
            ROI_SETTING_NAME_RIGHT_UPPER,
            trrp.trrp_pddsRectangle[RECTANGLE_RIGHT_UPPER]->dds_strDate,
            ROI_SETTING_NAME_RIGHT_LOWER,
            trrp.trrp_pddsRectangle[RECTANGLE_RIGHT_LOWER]->dds_strDate);
        jf_logger_logInfoMsg(
            "can be traded in roi, setting string: %s", ptps->tps_strModelParam);
    }

    return u32Ret;
}

static u32 _tradeTryBuyInRoi(
    struct da_model * pdm, stock_info_t * stockinfo, trade_pool_stock_t * ptps,
    da_model_trade_data_t * pdmtd, da_day_summary_t * buffer, int total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * end = buffer + total - 1;
    char strNextDate[MAX_TRADE_FIELD_LEN];

    getNextTradingDate(ptps->tps_strAddDate, strNextDate);
    jf_logger_logInfoMsg(
        "try buy in roi, addDate: %s, nextDate: %s",
        ptps->tps_strAddDate, strNextDate);

    if (ol_strcmp(end->dds_strDate, strNextDate) >= 0)
    {
        /*buy stock the next day after adding the stock to pool*/
        setTradePoolStockOpBuy(ptps);
        ol_strcpy(ptps->tps_strTradeDate, end->dds_strDate);
        setTradePoolStockPositionFull(ptps);
        ptps->tps_dbPrice = end->dds_dbOpeningPrice;
        /*TODO: use all funds for the trade, improve this to reduce the risk*/
        ptps->tps_nVolume = pdmtd->dmtd_dbFund / ptps->tps_dbPrice;
        jf_logger_logInfoMsg("try buy in roi, volume: %d", ptps->tps_nVolume);
        /*one board lot is 100 shares*/
        ptps->tps_nVolume /= 100;
        ptps->tps_nVolume *= 100;
        pdmtd->dmtd_dbFund -= ptps->tps_dbPrice * (oldouble_t)ptps->tps_nVolume;
        jf_logger_logInfoMsg(
            "try buy in roi, volume: %d, fund remains %.2f",
            ptps->tps_nVolume, pdmtd->dmtd_dbFund);
    }

    return u32Ret;
}

static u32 _getRoiRectanglePoint(
    trade_pool_stock_t * ptps, da_day_summary_t * buffer, int total,
    da_day_summary_t ** ppLeftUpper, da_day_summary_t ** ppLeftLower,
    da_day_summary_t ** ppRightUpper, da_day_summary_t ** ppRightLower)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    char strParam[MAX_TRADE_MODEL_PARAM_LEN];
    char * pstrArray[8];
    olsize_t sArray = 8;
    char strDate[16];

    memcpy(strParam, ptps->tps_strModelParam, MAX_TRADE_MODEL_PARAM_LEN);

    u32Ret = jf_string_processSettings(strParam, pstrArray, &sArray);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_getSettingsString(
            pstrArray, sArray, ROI_SETTING_NAME_LEFT_UPPER, "\0",
            strDate, sizeof(strDate));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = getDaySummaryWithDate(buffer, total, strDate, ppLeftUpper);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_getSettingsString(
            pstrArray, sArray, ROI_SETTING_NAME_LEFT_LOWER, "\0",
            strDate, sizeof(strDate));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = getDaySummaryWithDate(buffer, total, strDate, ppLeftLower);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_getSettingsString(
            pstrArray, sArray, ROI_SETTING_NAME_RIGHT_UPPER, "\0",
            strDate, sizeof(strDate));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = getDaySummaryWithDate(buffer, total, strDate, ppRightUpper);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_getSettingsString(
            pstrArray, sArray, ROI_SETTING_NAME_RIGHT_LOWER, "\0",
            strDate, sizeof(strDate));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = getDaySummaryWithDate(buffer, total, strDate, ppRightLower);
    }

    return u32Ret;
}

static u32 _tradeTrySellInRoi(
    struct da_model * pdm, stock_info_t * stockinfo, trade_pool_stock_t * ptps,
    da_model_trade_data_t * pdmtd, da_day_summary_t * buffer, int total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * pLeftUpper, * pLeftLower, * pRightUpper, * pRightLower;
    da_day_summary_t * end = buffer + total - 1;
    tx_rule_param_t trp;
    oldouble_t dbPrice;
    tx_rule_t * rule;
    boolean_t bSell = FALSE;

    jf_logger_logDebugMsg("try sell in roi, %s", stockinfo->si_strCode);
    
    if (ol_strcmp(end->dds_strDate, ptps->tps_strTradeDate) <= 0)
    {
        jf_logger_logDebugMsg("try sell in roi, T+1, cannot sell stock");
        return u32Ret;
    }
    
    u32Ret = _getRoiRectanglePoint(
        ptps, buffer, total, &pLeftUpper, &pLeftLower, &pRightUpper, &pRightLower);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_rule_getRule("pressureLine", &rule);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&trp, sizeof(trp));
        trp.trp_trplpPressureLine.trplp_pddsUpperLeft = pLeftUpper;
        trp.trp_trplpPressureLine.trplp_pddsUpperRight = pRightUpper;
        trp.trp_trplpPressureLine.trplp_u8Condition = PRESSURE_LINE_CONDITION_NEAR;
        trp.trp_trplpPressureLine.trplp_dbRatio = PRESSURE_LINE_RATIO;

        u32Ret = rule->tr_fnExecRule(stockinfo, buffer, total, &trp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        dbPrice = trp.trp_trplpPressureLine.trplp_dbPrice;
        bSell = TRUE;
        strcpy(ptps->tps_strOpRemark, "sell out");
    }
    else
    {
        u32Ret = tx_rule_getRule("needStopLoss", &rule);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_bzero(&trp, sizeof(trp));
            trp.trp_trnslpNeedStopLoss.trnslp_dbBuyPrice = ptps->tps_dbPrice;
            trp.trp_trnslpNeedStopLoss.trnslp_dbRatio = NEED_STOP_LOSS_RATIO;

            u32Ret = rule->tr_fnExecRule(stockinfo, buffer, total, &trp);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            dbPrice = trp.trp_trnslpNeedStopLoss.trnslp_dbStopLossPrice;
            bSell = TRUE;
            strcpy(ptps->tps_strOpRemark, "stop loss");
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (bSell)
        {
            /*sell stock*/
            setTradePoolStockOpSell(ptps);
            strcpy(ptps->tps_strTradeDate, end->dds_strDate);
            ptps->tps_dbPrice = dbPrice;
            pdmtd->dmtd_dbFund += ptps->tps_dbPrice * (oldouble_t)ptps->tps_nVolume;
            jf_logger_logInfoMsg(
                "try sell in roi, fund remains %.2f",
                pdmtd->dmtd_dbFund);
        }
    }

    return u32Ret;
}

static u32 _tradeInRoi(
    struct da_model * pdm, stock_info_t * stockinfo, trade_pool_stock_t * ptps,
    da_model_trade_data_t * pdmtd, da_day_summary_t * buffer, int total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (isTradePoolStockOpNone(ptps))
    {
        /*try to buy*/
        u32Ret = _tradeTryBuyInRoi(pdm, stockinfo, ptps, pdmtd, buffer, total);
    }
    else if (isTradePoolStockOpBuy(ptps))
    {
        /*try to sell*/
        u32Ret = _tradeTrySellInRoi(pdm, stockinfo, ptps, pdmtd, buffer, total);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 fillDaModel(da_model_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_strcpy(pdm->dm_strName, "ROI");
    ol_strcpy(pdm->dm_strLongName, "rectangle_over_index");
    pdm->dm_fnInitModel = _initDaModelRoi;
    pdm->dm_fnFiniModel = _finiDaModelRoi;
    pdm->dm_fnCanBeTraded = _canBeTradedInRoi;
    pdm->dm_fnTrade = _tradeInRoi;

    return u32Ret;
}


/*------------------------------------------------------------------------------------------------*/


