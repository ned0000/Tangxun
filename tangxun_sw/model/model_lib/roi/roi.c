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
#include "jf_jiukun.h"
#include "jf_string.h"

#include "tx_rule.h"
#include "tx_trade.h"

#include "roi.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define ROI_SETTING_NAME_LEFT_UPPER        "left_upper"
#define ROI_SETTING_NAME_LEFT_LOWER        "left_lower"
#define ROI_SETTING_NAME_RIGHT_UPPER       "right_upper"
#define ROI_SETTING_NAME_RIGHT_LOWER       "right_lower"

/* --- private routine section ------------------------------------------------------------------ */

static u32 _initTxModelRoi(tx_model_t * ptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_model_roi_data_t * ptmrd = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&ptmrd, sizeof(*ptmrd));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(ptmrd, sizeof(*ptmrd));

        ptm->tm_pData = ptmrd;
    }
    
    return u32Ret;
}

static u32 _finiTxModelRoi(tx_model_t * ptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ptm->tm_pData != NULL)
        jf_jiukun_freeMemory((void **)&ptm->tm_pData);

    return u32Ret;
}

static u32 _canBeTradedInRoi(
    struct tx_model * ptm, tx_stock_info_t * stockinfo, tx_trade_pool_stock_t * pttps,
    tx_ds_t * buffer, int total)
{
    u32 u32Ret = JF_ERR_NOT_READY;
    tx_rule_t * rule = NULL;
    tx_rule_rectangle_param_t trrp;
    tx_rule_min_num_of_day_summary_param_t trmnodsp;
    tx_rule_price_volatility_param_t trpvp;
    tx_rule_pressure_line_param_t trplp;
    tx_ds_t * end = buffer + total - 1;

    JF_LOGGER_INFO("stock: %s, total: %d", stockinfo->tsi_strCode, total);

    u32Ret = tx_rule_getRuleById(TX_RULE_ID_MIN_NUM_OF_DAY_SUMMARY, &rule);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        bzero(&trmnodsp, sizeof(trmnodsp));
        trmnodsp.trmnodsp_u32MinDay = TX_RULE_RECTANGLE_MAX_DAYS;

        u32Ret = rule->tr_fnExecRule(stockinfo, buffer, total, &trmnodsp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_rule_getRuleById(TX_RULE_ID_NO_HIGH_HIGH_LIMIT_DAY, &rule);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = rule->tr_fnExecRule(
            stockinfo, end - TX_RULE_RECTANGLE_MAX_DAYS + 1, TX_RULE_RECTANGLE_MAX_DAYS, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_rule_getRuleById(TX_RULE_ID_RECTANGLE, &rule);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&trrp, sizeof(trrp));
        trrp.trrp_u32MinDays = TX_RULE_RECTANGLE_MIN_DAYS;
        trrp.trrp_u32MaxDays = TX_RULE_RECTANGLE_MAX_DAYS;
        trrp.trrp_bCheckEdge = TRUE;
        trrp.trrp_dbPointThreshold = TX_RULE_RECTANGLE_POINT_THRESHOLD;

        u32Ret = rule->tr_fnExecRule(stockinfo, buffer, total, &trrp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_rule_getRuleById(TX_RULE_ID_PRICE_VOLATILITY, &rule);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&trpvp, sizeof(trpvp));
        trpvp.trpvp_u8Condition = TX_RULE_PRICE_VOLATILITY_CONDITION_GREATER_EQUAL;
        trpvp.trpvp_dbVolatility = TX_RULE_PRICE_VOLATILITY_RATIO;

        u32Ret = rule->tr_fnExecRule(
            stockinfo, trrp.trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_UPPER],
            end - trrp.trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_UPPER] + 1, &trpvp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_rule_getRuleById(TX_RULE_ID_PRESSURE_LINE, &rule);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&trplp, sizeof(trplp));
        trplp.trplp_ptdUpperLeft = trrp.trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_UPPER];
        trplp.trplp_ptdUpperRight = trrp.trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_UPPER];
        trplp.trplp_u8Condition = TX_RULE_PRESSURE_LINE_CONDITION_FAR;
        trplp.trplp_dbRatio = TX_RULE_PRICE_VOLATILITY_RATIO * 2 / 3;

        u32Ret = rule->tr_fnExecRule(
            stockinfo, trrp.trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_UPPER],
            end - trrp.trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_UPPER] + 1, &trplp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        tx_trade_initPoolStock(pttps, stockinfo->tsi_strCode, ptm->tm_strName);
        strcpy(pttps->ttps_strAddDate, end->td_strDate);
        snprintf(
            pttps->ttps_strModelParam, TX_TRADE_MAX_MODEL_PARAM_LEN,
            "%s=%s,%s=%s,%s=%s,%s=%s",
            ROI_SETTING_NAME_LEFT_UPPER,
            trrp.trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_UPPER]->td_strDate,
            ROI_SETTING_NAME_LEFT_LOWER,
            trrp.trrp_ptdRectangle[TX_RULE_RECTANGLE_LEFT_LOWER]->td_strDate,
            ROI_SETTING_NAME_RIGHT_UPPER,
            trrp.trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_UPPER]->td_strDate,
            ROI_SETTING_NAME_RIGHT_LOWER,
            trrp.trrp_ptdRectangle[TX_RULE_RECTANGLE_RIGHT_LOWER]->td_strDate);

        JF_LOGGER_INFO("setting string: %s", pttps->ttps_strModelParam);
    }

    return u32Ret;
}

static u32 _tradeTryBuyInRoi(
    struct tx_model * ptm, tx_stock_info_t * stockinfo, tx_trade_pool_stock_t * pttps,
    tx_model_trade_data_t * ptmtd, tx_ds_t * buffer, int total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * end = buffer + total - 1;
    char strNextDate[TX_TRADE_MAX_FIELD_LEN];
    olint_t nVolume = 0;

    tx_trade_getNextTradingDate(pttps->ttps_strAddDate, strNextDate);
    JF_LOGGER_INFO("addDate: %s, nextDate: %s", pttps->ttps_strAddDate, strNextDate);

    /*Buy stock the next day after adding the stock to pool.*/
    if (ol_strcmp(end->td_strDate, strNextDate) >= 0)
    {
        tx_trade_setPoolStockOpBuy(pttps);
        tx_trade_setPoolStockTradeDate(pttps, end->td_strDate);
        tx_trade_setPoolStockPositionFull(pttps);
        /*Buy stock with opening price.*/
        tx_trade_setPoolStockPrice(pttps, end->td_dbOpeningPrice);
        /*TODO: use all funds for the trade, improve this to reduce the risk.*/
        nVolume = ptmtd->tmtd_dbFund / pttps->ttps_dbPrice;
        JF_LOGGER_DEBUG("volume: %d", pttps->ttps_nVolume);
        /*One board lot is 100 shares.*/
        nVolume /= 100;
        nVolume *= 100;
        tx_trade_setPoolStockVolume(pttps, nVolume);

        ptmtd->tmtd_dbFund -= pttps->ttps_dbPrice * (oldouble_t)nVolume;
        JF_LOGGER_DEBUG("fund remains %.2f", ptmtd->tmtd_dbFund);
    }

    return u32Ret;
}

static u32 _getRoiRectanglePoint(
    tx_trade_pool_stock_t * pttps, tx_ds_t * buffer, int total,
    tx_ds_t ** ppLeftUpper, tx_ds_t ** ppLeftLower,
    tx_ds_t ** ppRightUpper, tx_ds_t ** ppRightLower)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    char strParam[TX_TRADE_MAX_MODEL_PARAM_LEN];
    char * pstrArray[8];
    olsize_t sArray = 8;
    char strDate[16];

    memcpy(strParam, pttps->ttps_strModelParam, TX_TRADE_MAX_MODEL_PARAM_LEN);

    u32Ret = jf_string_processSettings(strParam, pstrArray, &sArray);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_getSettingsString(
            pstrArray, sArray, ROI_SETTING_NAME_LEFT_UPPER, "\0",
            strDate, sizeof(strDate));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_ds_getDsWithDate(buffer, total, strDate, ppLeftUpper);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_getSettingsString(
            pstrArray, sArray, ROI_SETTING_NAME_LEFT_LOWER, "\0",
            strDate, sizeof(strDate));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_ds_getDsWithDate(buffer, total, strDate, ppLeftLower);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_getSettingsString(
            pstrArray, sArray, ROI_SETTING_NAME_RIGHT_UPPER, "\0",
            strDate, sizeof(strDate));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_ds_getDsWithDate(buffer, total, strDate, ppRightUpper);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_getSettingsString(
            pstrArray, sArray, ROI_SETTING_NAME_RIGHT_LOWER, "\0",
            strDate, sizeof(strDate));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_ds_getDsWithDate(buffer, total, strDate, ppRightLower);
    }

    return u32Ret;
}

static u32 _tradeTrySellInRoi(
    struct tx_model * ptm, tx_stock_info_t * stockinfo, tx_trade_pool_stock_t * pttps,
    tx_model_trade_data_t * ptmtd, tx_ds_t * buffer, int total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * pLeftUpper = NULL, * pLeftLower = NULL, * pRightUpper = NULL, * pRightLower = NULL;
    tx_ds_t * end = buffer + total - 1;
    tx_rule_pressure_line_param_t trplp;
    tx_rule_need_stop_loss_param_t trnslp;
    oldouble_t dbPrice = 0;
    tx_rule_t * rule = NULL;
    boolean_t bSell = FALSE;

    JF_LOGGER_DEBUG("stock: %s", stockinfo->tsi_strCode);
    
    if (ol_strcmp(end->td_strDate, pttps->ttps_strTradeDate) <= 0)
    {
        JF_LOGGER_DEBUG("T+1, cannot sell stock");
        return u32Ret;
    }

    /*Is the bought price is near pressure line.*/
    u32Ret = _getRoiRectanglePoint(
        pttps, buffer, total, &pLeftUpper, &pLeftLower, &pRightUpper, &pRightLower);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = tx_rule_getRuleById(TX_RULE_ID_PRESSURE_LINE, &rule);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&trplp, sizeof(trplp));
        trplp.trplp_ptdUpperLeft = pLeftUpper;
        trplp.trplp_ptdUpperRight = pRightUpper;
        trplp.trplp_u8Condition = TX_RULE_PRESSURE_LINE_CONDITION_NEAR;
        trplp.trplp_dbRatio = TX_RULE_PRESSURE_LINE_RATIO;

        u32Ret = rule->tr_fnExecRule(stockinfo, buffer, total, &trplp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Near the pressure line, sell the stock.*/
        dbPrice = trplp.trplp_dbPrice;
        bSell = TRUE;
        ol_strcpy(pttps->ttps_strOpRemark, "sell out");
    }
    else
    {
        /*Not near the pressure line.*/
        /*Check if need to stop loss.*/
        u32Ret = tx_rule_getRuleById(TX_RULE_ID_NEED_STOP_LOSS, &rule);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_bzero(&trnslp, sizeof(trnslp));
            trnslp.trnslp_dbBuyPrice = pttps->ttps_dbPrice;
            trnslp.trnslp_dbRatio = TX_RULE_NEED_STOP_LOSS_RATIO;

            u32Ret = rule->tr_fnExecRule(stockinfo, buffer, total, &trnslp);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Need to stop loss.*/
            dbPrice = trnslp.trnslp_dbStopLossPrice;
            bSell = TRUE;
            strcpy(pttps->ttps_strOpRemark, "stop loss");
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (bSell)
        {
            /*Sell stock.*/
            tx_trade_setPoolStockOpSell(pttps);
            tx_trade_setPoolStockTradeDate(pttps, end->td_strDate);
            tx_trade_setPoolStockPrice(pttps, dbPrice);
            /*Calculate the assert.*/
            ptmtd->tmtd_dbFund += dbPrice * (oldouble_t)pttps->ttps_nVolume;
            JF_LOGGER_INFO("fund remains %.2f", ptmtd->tmtd_dbFund);
        }
    }

    return u32Ret;
}

static u32 _tradeInRoi(
    struct tx_model * ptm, tx_stock_info_t * stockinfo, tx_trade_pool_stock_t * pttps,
    tx_model_trade_data_t * ptmtd, tx_ds_t * buffer, int total)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (tx_trade_isPoolStockOpNone(pttps))
    {
        /*Try to buy.*/
        u32Ret = _tradeTryBuyInRoi(ptm, stockinfo, pttps, ptmtd, buffer, total);
    }
    else if (tx_trade_isPoolStockOpBuy(pttps))
    {
        /*Try to sell.*/
        u32Ret = _tradeTrySellInRoi(ptm, stockinfo, pttps, ptmtd, buffer, total);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 tx_model_fillModel(tx_model_t * ptm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_strcpy(ptm->tm_strName, "ROI");
    ol_strcpy(ptm->tm_strLongName, "rectangle_over_index");
    ptm->tm_fnInitModel = _initTxModelRoi;
    ptm->tm_fnFiniModel = _finiTxModelRoi;
    ptm->tm_fnCanBeTraded = _canBeTradedInRoi;
    ptm->tm_fnTrade = _tradeInRoi;

    return u32Ret;
}


/*------------------------------------------------------------------------------------------------*/


