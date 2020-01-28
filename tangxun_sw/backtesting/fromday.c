/**
 *  @file fromday.c
 *
 *  @brief Backtesting model from trading day.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_process.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_jiukun.h"
#include "jf_file.h"
#include "jf_filestream.h"
#include "jf_dir.h"
#include "jf_time.h"
#include "jf_date.h"

#include "tx_stock.h"
#include "tx_model.h"
#include "tx_persistency.h"
#include "tx_backtesting.h"
#include "tx_trade.h"

#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */
static olchar_t * ls_pstrBacktestingStartDateDayByDay = "2014-01-01";

#define MAX_NUM_OF_TD_FOR_BACKTESTING   (800)

typedef struct
{
    tx_backtesting_result_t * bre_ptbrResult;
    jf_filestream_t * bre_pjfAsset;
    double bre_dbAsset;
    olint_t bre_nStock;
    boolean_t bre_bSaveAsset;
} backtesting_result_ext_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _saveAsset(
    tx_backtesting_eval_param_t * ptbep, backtesting_result_ext_t * pbre, olchar_t * pstrDate)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strLine[128];
    olsize_t sLine;

    assert(pbre->bre_pjfAsset != NULL);

    JF_LOGGER_DEBUG("save asset");
    
    sLine = ol_sprintf(
        strLine, "%s\t%.2f\t%d\n", pstrDate, pbre->bre_dbAsset, pbre->bre_nStock);
    u32Ret = jf_filestream_writen(pbre->bre_pjfAsset, strLine, sLine);

    return u32Ret;
}

static u32 _backtestingCalcAssetDayByDay(
    tx_backtesting_eval_param_t * ptbep, backtesting_result_ext_t * pbre, olchar_t * pstrBacktestDate)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_backtesting_result_t * ptbr = pbre->bre_ptbrResult;

    if (pbre->bre_bSaveAsset)
    {
        if (ptbr->tbr_dbMinAsset > pbre->bre_dbAsset)
            ptbr->tbr_dbMinAsset = pbre->bre_dbAsset;
        if (ptbr->tbr_dbMaxAsset < pbre->bre_dbAsset)
            ptbr->tbr_dbMaxAsset = pbre->bre_dbAsset;

        u32Ret = _saveAsset(ptbep, pbre, pstrBacktestDate);
    }

    return u32Ret;
}

static u32 _summaryBacktestingResult(tx_backtesting_result_t * ptbr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count = 0;
    tx_trade_trading_record_t * traderecord = NULL;
    tx_trade_trading_record_t * pBuy, * pSell, * pEnd;

    count = tx_persistency_getNumOfTradingRecord();

    if (count == 0)
        return u32Ret;

    JF_LOGGER_INFO("%d record.", count);

    jf_jiukun_allocMemory((void **)&traderecord, count * sizeof(tx_trade_trading_record_t));

    u32Ret = tx_persistency_getAllTradingRecord(traderecord, &count);
    if ((u32Ret == JF_ERR_NO_ERROR) && (count > 0))
    {
        pEnd = traderecord + count - 1;
        pBuy = traderecord;
        while (pBuy <= pEnd)
        {
            if (tx_trade_isTradingRecordOpBuy(pBuy))
            {
                ptbr->tbr_u32NumOfTrade ++;
                /*Buy operation, find sell*/
                pSell = pBuy + 1;
                while (pSell <= pEnd)
                {
                    if (tx_trade_isTradingRecordOpSell(pSell) &&
                        (ol_strcmp(pSell->tttr_strStock, pBuy->tttr_strStock) == 0) &&
                        (ol_strcmp(pSell->tttr_strModel, pBuy->tttr_strModel) == 0)) 
                        break;
                    pSell ++;
                }

                if (pSell <= pEnd)
                {
                    /*found the sell record*/
                    if (pSell->tttr_dbPrice > pBuy->tttr_dbPrice)
                        ptbr->tbr_u32NumOfTradeProfit ++;
                    else
                        ptbr->tbr_u32NumOfTradeLoss ++;
                }
                else
                {
                    /*no sell record for the buy operation*/
                    ptbr->tbr_u32NumOfTradeLoss ++;
                    ptbr->tbr_dbFund += pBuy->tttr_dbPrice * (oldouble_t)pBuy->tttr_nVolume;
                }
            }
            pBuy ++;
        }

    }

    jf_jiukun_freeMemory((void **)&traderecord);

    if (ptbr->tbr_dbMinAsset < ptbr->tbr_dbInitialFund)
        ptbr->tbr_dbMaxDrawdown =
            (ptbr->tbr_dbInitialFund - ptbr->tbr_dbMinAsset) * 100 / ptbr->tbr_dbInitialFund;

    ptbr->tbr_dbRateOfReturn =
        (ptbr->tbr_dbFund - ptbr->tbr_dbInitialFund) * 100 / ptbr->tbr_dbInitialFund;
    
    return u32Ret;
}

static u32 _backtestingFindStockInModelDayByDay(
    tx_model_t * model, tx_backtesting_eval_param_t * ptbep, olchar_t * pstrBacktestDate,
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_trade_pool_stock_t ttps;
    tx_ds_t * ptd;
    olint_t total = num;

    if (num == 0)
        return u32Ret;

    tx_trade_initPoolStock(&ttps, stockinfo->tsi_strCode, model->tm_strName);

    u32Ret = tx_persistency_getPoolStock(&ttps);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*the stock is in pool*/
        JF_LOGGER_INFO("%s is already in pool", stockinfo->tsi_strCode);
    }
    else
    {
        /* The stock is not in pool */
        u32Ret = tx_ds_getDsWithDate(buffer, total, pstrBacktestDate, &ptd);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            total = ptd - buffer + 1;

            u32Ret = model->tm_fnCanBeTraded(model, stockinfo, &ttps, buffer, total);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ttps.ttps_nNumOfDaySummary = num;
            ol_strcpy(ttps.ttps_strStartDateOfDaySummary, buffer->td_strDate);
            u32Ret = tx_persistency_insertPoolStock(&ttps);
        }
    }

    return u32Ret;
}

static u32 _backtestingFindStockDayByDay(
    tx_model_t * model, tx_backtesting_eval_param_t * ptbep, olchar_t * pstrBacktestDate,
    tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_info_t * stockinfo;
    olint_t total;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    boolean_t bAfter;

    JF_LOGGER_DEBUG("last day: %s", pstrBacktestDate);
    
    stockinfo = tx_stock_getFirstStockInfo();
    while (stockinfo != NULL)
    {
        bAfter = tx_trade_isAfterStockFirstTradeDate(stockinfo, pstrBacktestDate);
        if (! bAfter)
        {
            stockinfo = tx_stock_getNextStockInfo(stockinfo);
            continue;
        }

        ol_snprintf(
            strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
            ptbep->tbep_pstrStockPath, PATH_SEPARATOR, stockinfo->tsi_strCode);
        strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';
        total = num;

#define BACKTESTING_MAX_COUNT_AFTER_END_DATE    (100)    
        u32Ret = tx_ds_readDsUntilDateWithFRoR(
            strFullname, pstrBacktestDate, BACKTESTING_MAX_COUNT_AFTER_END_DATE, buffer, &total);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _backtestingFindStockInModelDayByDay(
                model, ptbep, pstrBacktestDate, stockinfo, buffer, total);
        }

        stockinfo = tx_stock_getNextStockInfo(stockinfo);
    }

    return JF_ERR_NO_ERROR;
}

static u32 _backtestingSellOneStockDayByDay(
    tx_backtesting_eval_param_t * ptbep, backtesting_result_ext_t * pbre, tx_stock_info_t * stockinfo,
    tx_model_t * model, tx_trade_pool_stock_t * pttps, tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_trade_trading_record_t tttr;
    tx_model_trade_data_t tmtd;
    tx_backtesting_result_t * ptbr = pbre->bre_ptbrResult;
    tx_ds_t * end;

    ol_bzero(&tmtd, sizeof(tmtd));
    
    /*Bought already, try to sell*/
    u32Ret = model->tm_fnTrade(model, stockinfo, pttps, &tmtd, buffer, num);
    if ((u32Ret == JF_ERR_NO_ERROR) && tx_trade_isPoolStockOpSell(pttps))
    {
        /*the stock is sold out*/
        /*add trading record*/
        tx_trade_setTradingRecord(&tttr, pttps);
        tx_persistency_insertTradingRecord(&tttr);
        /*delete pool stock*/
        tx_persistency_removePoolStock(pttps);
        ptbr->tbr_dbFund += tmtd.tmtd_dbFund;
        pbre->bre_dbAsset += tmtd.tmtd_dbFund;
        pbre->bre_bSaveAsset = TRUE;
    }
    else
    {
        pbre->bre_nStock ++;
        end = buffer + num - 1;
        pbre->bre_bSaveAsset = TRUE;
        /*the asset already include the stock bought today*/
        if (ol_strcmp(end->td_strDate, pttps->ttps_strTradeDate) > 0)
        {
            pbre->bre_dbAsset += end->td_dbClosingPrice * (oldouble_t)pttps->ttps_nVolume;
        }
    }
    jf_logger_logInfoMsg(
        "backtesting sell one stock day by day, fund: %.2f, asset: %.2f",
        ptbr->tbr_dbFund, pbre->bre_dbAsset);
    
    return u32Ret;
}

static u32 _backtestingSellStockDayByDay(
    tx_backtesting_eval_param_t * ptbep, backtesting_result_ext_t * pbre, tx_trade_pool_stock_t ** pttpsOp,
    olint_t opCount, olchar_t * pstrBacktestDate, tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index;
    tx_model_t * model = NULL;
    tx_stock_info_t * stockinfo;
    tx_trade_pool_stock_t * pStock = NULL;
    olint_t total = num;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    tx_ds_t * ptd;

    if (opCount == 0)
        return u32Ret;

    JF_LOGGER_INFO("count: %d", opCount);

    for (index = 0; index < opCount; index ++)
    {
        pStock = pttpsOp[index];

        u32Ret = tx_model_getModel(pStock->ttps_strModel, &model);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = tx_stock_getStockInfo(pStock->ttps_strStock, &stockinfo);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_snprintf(
                strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
                ptbep->tbep_pstrStockPath, PATH_SEPARATOR, stockinfo->tsi_strCode);
            strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';
            total = pStock->ttps_nNumOfDaySummary;

            u32Ret = tx_ds_readDsFromDateWithFRoR(
                strFullname, pStock->ttps_strStartDateOfDaySummary, buffer, &total);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = tx_ds_getDsWithDate(buffer, total, pstrBacktestDate, &ptd);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            total = ptd - buffer + 1;

            u32Ret = _backtestingSellOneStockDayByDay(
                ptbep, pbre, stockinfo, model, pStock, buffer, total);
        }
    }

    return u32Ret;
}

static u32 sortPoolStockForBuyOp(
    tx_backtesting_eval_param_t * ptbep, tx_trade_pool_stock_t ** pttpsOp, olint_t opCount,
    olchar_t * pstrBacktestDate, tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    /*ToDo: select the most valuable stocks */
    
    return u32Ret;
}

static u32 _backtestingBuyOneStockDayByDay(
    tx_backtesting_eval_param_t * ptbep, backtesting_result_ext_t * pbre, tx_stock_info_t * stockinfo,
    tx_model_t * model, tx_trade_pool_stock_t * pttps, tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_trade_trading_record_t tttr;
    tx_model_trade_data_t tmtd;
    oldouble_t maxfund;
    tx_backtesting_result_t * ptbr = pbre->bre_ptbrResult;

    ol_bzero(&tmtd, sizeof(tmtd));
    
    /*ToDo: determine fund which can be used for one stock*/
#define MAX_PERCENT_OF_FUND_FOR_ONE_STOCK     (0.2)
#define MAX_FUND_FOR_ONE_STOCK                (10000)
    tmtd.tmtd_dbFund = ptbr->tbr_dbFund;
    maxfund = ptbr->tbr_dbInitialFund * MAX_PERCENT_OF_FUND_FOR_ONE_STOCK;
    if (tmtd.tmtd_dbFund > maxfund)
        tmtd.tmtd_dbFund = maxfund;
    if (tmtd.tmtd_dbFund < MAX_FUND_FOR_ONE_STOCK)
    {
        jf_logger_logErrMsg(JF_ERR_OPERATION_FAIL, "No enough fund");
        /*delete pool stock*/
        tx_persistency_removePoolStock(pttps);
        return u32Ret;
    }

    ptbr->tbr_dbFund -= tmtd.tmtd_dbFund;
    /*not bought yet*/
    u32Ret = model->tm_fnTrade(model, stockinfo, pttps, &tmtd, buffer, num);
    if ((u32Ret == JF_ERR_NO_ERROR) && tx_trade_isPoolStockOpBuy(pttps))
    {
        /*the stock is bought*/
        /*update pool stock*/
        tx_persistency_updatePoolStock(pttps);
        /*add trading record*/
        tx_trade_setTradingRecord(&tttr, pttps);
        tx_persistency_insertTradingRecord(&tttr);
    }

    ptbr->tbr_dbFund += tmtd.tmtd_dbFund;
    jf_logger_logInfoMsg("backtesting buy one stock day by day, fund: %.2f", ptbr->tbr_dbFund);

    return u32Ret;
}

static u32 _backtestingBuyStockDayByDay(
    tx_backtesting_eval_param_t * ptbep, backtesting_result_ext_t * pbre, tx_trade_pool_stock_t ** pttpsOp,
    olint_t opCount, olchar_t * pstrBacktestDate, tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index;
    tx_model_t * model = NULL;
    tx_stock_info_t * stockinfo;
    tx_trade_pool_stock_t * pStock = NULL;
    olint_t total = num;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    tx_ds_t * ptd;

    if (opCount == 0)
        return u32Ret;

    jf_logger_logInfoMsg("backtesting buy stock day by day, count: %d", opCount);
    
    for (index = 0; index < opCount; index ++)
    {
        pStock = pttpsOp[index];

        u32Ret = tx_model_getModel(pStock->ttps_strModel, &model);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = tx_stock_getStockInfo(pStock->ttps_strStock, &stockinfo);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_snprintf(
                strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
                ptbep->tbep_pstrStockPath, PATH_SEPARATOR, stockinfo->tsi_strCode);
            strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';
            total = pStock->ttps_nNumOfDaySummary;

            u32Ret = tx_ds_readDsFromDateWithFRoR(
                strFullname, pStock->ttps_strStartDateOfDaySummary, buffer, &total);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = tx_ds_getDsWithDate(buffer, total, pstrBacktestDate, &ptd);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            total = ptd - buffer + 1;
        
            u32Ret = _backtestingBuyOneStockDayByDay(
                ptbep, pbre, stockinfo, model, pStock, buffer, total);
        }
    }

    return u32Ret;
}

static u32 _backtestingTradeStockDayByDay(
    tx_backtesting_eval_param_t * ptbep, backtesting_result_ext_t * pbre,
    olchar_t * pstrBacktestDate, tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count = 0;
    tx_trade_pool_stock_t * pttps = NULL;
    olint_t opCount = 0;
    tx_trade_pool_stock_t ** pttpsOp = NULL;

    pbre->bre_dbAsset = pbre->bre_ptbrResult->tbr_dbFund;
    pbre->bre_nStock = 0;
    pbre->bre_bSaveAsset = FALSE;

    count = tx_persistency_getNumOfPoolStock();

    if (count == 0)
        return u32Ret;

    JF_LOGGER_INFO("count: %d", count);

    jf_jiukun_allocMemory((void **)&pttps, count * sizeof(tx_trade_pool_stock_t));

    u32Ret = tx_persistency_getAllPoolStock(pttps, &count);
    if ((u32Ret == JF_ERR_NO_ERROR) && (count > 0))
    {
        opCount = count;
        jf_jiukun_allocMemory((void **)&pttpsOp, opCount * sizeof(tx_trade_pool_stock_t *));
        /*1. try to buy stock*/
        tx_trade_filterPoolStockByOp(pttps, count, TX_TRADE_STOCK_OP_NONE, pttpsOp, &opCount);
        sortPoolStockForBuyOp(ptbep, pttpsOp, opCount, pstrBacktestDate, buffer, num);
        u32Ret = _backtestingBuyStockDayByDay(
            ptbep, pbre, pttpsOp, opCount, pstrBacktestDate, buffer, num);

        /*2. try to sell stock*/
        opCount = count;
        tx_trade_filterPoolStockByOp(pttps, count, TX_TRADE_STOCK_OP_BUY, pttpsOp, &opCount);
        u32Ret = _backtestingSellStockDayByDay(
            ptbep, pbre, pttpsOp, opCount, pstrBacktestDate, buffer, num);

        jf_jiukun_freeMemory((void **)&pttpsOp);

        _backtestingCalcAssetDayByDay(ptbep, pbre, pstrBacktestDate);
    }

    jf_jiukun_freeMemory((void **)&pttps);
    
    return u32Ret;
}

static u32 _startBacktestingModelDayByDay(
    tx_model_t * model, tx_backtesting_eval_param_t * ptbep, backtesting_result_ext_t * pbre)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t lyear, lmonth, lday;
    olint_t ldays, enddays;
    olchar_t strBacktestDate[16];
    tx_ds_t * buffer = NULL;
    olint_t total = 400; //MAX_NUM_OF_TD_FOR_BACKTESTING;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];

    ol_sprintf(
        strFullname, "%s%c%s", BACKTESTING_OUTPUT_DIR, PATH_SEPARATOR, BACKTESTING_OUTPUT_FILE);
    
    u32Ret = jf_filestream_open(strFullname, "w", &pbre->bre_pjfAsset);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;
    
    JF_LOGGER_INFO("start");
    tx_persistency_clearData();

    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);

    jf_date_getDate2FromString(ls_pstrBacktestingStartDateDayByDay, &lyear, &lmonth, &lday);
    ldays = jf_date_convertDateToDaysFrom1970(lyear, lmonth, lday);
    ol_strcpy(pbre->bre_ptbrResult->tbr_strStartDate, ls_pstrBacktestingStartDateDayByDay);

    enddays = jf_date_convertTodayToDaysFrom1970();
    jf_date_getStringDate2ForDaysFrom1970(pbre->bre_ptbrResult->tbr_strEndDate, enddays);

    pbre->bre_dbAsset = pbre->bre_ptbrResult->tbr_dbInitialFund;
    pbre->bre_nStock = 0;
    _saveAsset(ptbep, pbre, ls_pstrBacktestingStartDateDayByDay);
    
    while (ldays <= enddays)
    {
        /*determine the start date for reading day summary*/
        if (tx_trade_isHoliday(ldays))
        {
            ldays ++;
            continue;
        }

        jf_date_getStringDate2ForDaysFrom1970(strBacktestDate, ldays);
        JF_LOGGER_DEBUG("date: %s", strBacktestDate);

        u32Ret = _backtestingFindStockDayByDay(model, ptbep, strBacktestDate, buffer, total);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _backtestingTradeStockDayByDay(ptbep, pbre, strBacktestDate, buffer, total);
        }

        ldays ++;
    }

    jf_jiukun_freeMemory((void **)&buffer);

    /*in case the stock is bought but not sold out, we should sell it out forcely*/
    wrapupBtPoolStock(pbre->bre_ptbrResult);

    jf_filestream_close(&pbre->bre_pjfAsset);
    
    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 tx_backtesting_evalModelFromDay(
    const olchar_t * pstrModel, tx_backtesting_eval_param_t * ptbep, tx_backtesting_result_t * ptbr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_model_t * model = NULL;
    backtesting_result_ext_t bre;

    u32Ret = tx_model_getModel(pstrModel, &model);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_dir_create(BACKTESTING_OUTPUT_DIR, JF_DIR_DEFAULT_CREATE_MODE);
        if (u32Ret == JF_ERR_DIR_ALREADY_EXIST)
            u32Ret = JF_ERR_NO_ERROR;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(ptbr, sizeof(tx_backtesting_result_t));
        ptbr->tbr_dbInitialFund = ptbep->tbep_dbInitialFund;
        ptbr->tbr_dbFund = ptbep->tbep_dbInitialFund;
        ptbr->tbr_dbMinAsset = ptbr->tbr_dbMaxAsset = ptbr->tbr_dbFund;

        ol_bzero(&bre, sizeof(backtesting_result_ext_t));
        bre.bre_ptbrResult = ptbr;

        u32Ret = _startBacktestingModelDayByDay(model, ptbep, &bre);

        _summaryBacktestingResult(ptbr);
    }

    return u32Ret;
}


/*------------------------------------------------------------------------------------------------*/


