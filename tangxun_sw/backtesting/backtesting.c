/**
 *  @file backtesting.c
 *
 *  @brief Backtesting module
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
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

#include "stocklist.h"
#include "parsedata.h"
#include "indicator.h"
#include "datastat.h"
#include "statarbitrage.h"
#include "damodel.h"
#include "trade_persistency.h"
#include "backtesting.h"
#include "stocktrade.h"
#include "tradehelper.h"

/* --- private data/data structure section --------------------------------- */
static olchar_t * ls_pstrBacktestingStartDateDayByDay = "2014-01-01";

#define MAX_NUM_OF_DDS_FOR_BACKTESTING   (800)
#define BACKTESTING_OUTPUT_DIR           "output"
#define BACKTESTING_OUTPUT_FILE          "asset.txt"

typedef struct
{
    backtesting_result_t * bre_pbrResult;
    jf_filestream_t * bre_pjfAsset;
    double bre_dbAsset;
    olint_t bre_nStock;
    boolean_t bre_bSaveAsset;
} backtesting_result_ext_t;

/* --- private routine section---------------------------------------------- */

static u32 _backtestingFindAndTradeStockInModel(
    backtesting_param_t * pbp, backtesting_result_ext_t * pbre,
    stock_info_t * stockinfo, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index, max = DA_MODEL_MAX_ID;
    da_model_t * model = NULL;
    da_day_summary_t * end;
    trade_pool_stock_t tps;
    trade_trading_record_t ttr;
    da_model_trade_data_t dmtd;
    backtesting_result_t * pbr = pbre->bre_pbrResult;

    end = buffer + num - 1;
    jf_logger_logInfoMsg(
        "throw stock into model, total: %d, last day: %s", num, end->dds_strDate);

    for (index = 0; index < max; index ++)
    {
        u32Ret = getDaModel((da_model_id_t)index, &model);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if ((pbp->bp_pstrModel != NULL) &&
                ol_strcmp(model->dm_strName, pbp->bp_pstrModel) != 0)
                continue;
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            initTradePoolStock(&tps, stockinfo->si_strCode, model->dm_strName);
            initTradeTradingRecord(
                &ttr, stockinfo->si_strCode, model->dm_strName);

            u32Ret = getPoolStockInTradePersistency(&tps);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                /*the stock is in pool*/
                jf_logger_logInfoMsg("%s is already in pool", stockinfo->si_strCode);

                ol_bzero(&dmtd, sizeof(dmtd));

                if (isTradePoolStockOpNone(&tps))
                {
                    jf_logger_logInfoMsg(
                        "throw stock into model, before buy, fund: %.2f", pbr->br_dbFund);
                    dmtd.dmtd_dbFund = pbr->br_dbFund;
                    /*not bought yet*/
                    u32Ret = model->dm_fnTrade(model, stockinfo, &tps, &dmtd, buffer, num);
                    if ((u32Ret == JF_ERR_NO_ERROR) && isTradePoolStockOpBuy(&tps))
                    {
                        /*the stock is bought*/
                        /*update pool stock*/
                        updatePoolStockInTradePersistency(&tps);
                        /*add trading record*/
                        setTradeTradingRecord(&ttr, &tps);
                        insertTradingRecordIntoTradePersistency(&ttr);

                        pbre->bre_nStock ++;
                        pbre->bre_bSaveAsset = TRUE;
                    }
                    pbr->br_dbFund = dmtd.dmtd_dbFund;
                    jf_logger_logInfoMsg(
                        "throw stock into model, after buy, fund: %.2f", pbr->br_dbFund);
                }
                else if (isTradePoolStockOpBuy(&tps))
                {
                    jf_logger_logInfoMsg(
                        "throw stock into model, before sell, fund: %.2f", pbr->br_dbFund);
                    /*Bought already, try to sell*/
                    u32Ret = model->dm_fnTrade(model, stockinfo, &tps, &dmtd, buffer, num);
                    if ((u32Ret == JF_ERR_NO_ERROR) && isTradePoolStockOpSell(&tps))
                    {
                        /*the stock is sold out*/
                        /*add trading record*/
                        setTradeTradingRecord(&ttr, &tps);
                        insertTradingRecordIntoTradePersistency(&ttr);
                        /*delete pool stock*/
                        removePoolStockFromTradePersistency(&tps);
                        pbr->br_dbFund += dmtd.dmtd_dbFund;
                        pbre->bre_dbAsset += dmtd.dmtd_dbFund;
                        pbre->bre_bSaveAsset = TRUE;
                        jf_logger_logInfoMsg(
                            "throw stock into model, after sell, fund: %.2f", pbr->br_dbFund);
                    }
                    else
                    {
                        /*the stock is not sold out and still in pool*/
                        pbre->bre_nStock ++;
                        pbre->bre_dbAsset += end->dds_dbClosingPrice * (oldouble_t)tps.tps_nVolume;
                        pbre->bre_bSaveAsset = TRUE;
                    }
                }
            }
            else
            {
                /* The stock is not in pool */
                u32Ret = model->dm_fnCanBeTraded(model, stockinfo, &tps, buffer, num);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    u32Ret = insertPoolStockIntoTradePersistency(&tps);
                }
            }
        }
    }

    return u32Ret;
}

static u32 _saveAsset(
    backtesting_param_t * pbp, backtesting_result_ext_t * pbre,
    olchar_t * pstrDate)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strLine[128];
    olsize_t sLine;

    assert(pbre->bre_pjfAsset != NULL);

    jf_logger_logDebugMsg("save asset");
    
    sLine = ol_sprintf(
        strLine, "%s\t%.2f\t%d\n", pstrDate, pbre->bre_dbAsset, pbre->bre_nStock);
    u32Ret = jf_filestream_writen(pbre->bre_pjfAsset, strLine, sLine);

    return u32Ret;
}

static u32 _backtestingCalcAssetDayByDay(
    backtesting_param_t * pbp, backtesting_result_ext_t * pbre,
    olchar_t * pstrBacktestDate)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    backtesting_result_t * pbr = pbre->bre_pbrResult;

    if (pbre->bre_bSaveAsset)
    {
        if (pbr->br_dbMinAsset > pbre->bre_dbAsset)
            pbr->br_dbMinAsset = pbre->bre_dbAsset;
        if (pbr->br_dbMaxAsset < pbre->bre_dbAsset)
            pbr->br_dbMaxAsset = pbre->bre_dbAsset;

        u32Ret = _saveAsset(pbp, pbre, pstrBacktestDate);
    }

    return u32Ret;
}

static u32 _backtestingFindAndTradeStock(
    backtesting_param_t * pbp, backtesting_result_ext_t * pbre,
    stock_info_t * stockinfo, olchar_t * pstrStartDate,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t lyear, lmonth, lday;
    olint_t ldays, enddays;
    olchar_t strDate[16];
    olint_t total = num;
    da_day_summary_t * end = buffer + num - 1;

    jf_logger_logInfoMsg("backtesting find trade stock, startdate: %s", pstrStartDate);

    ol_strcpy(strDate, end->dds_strDate);
    jf_date_getDate2FromString(end->dds_strDate, &lyear, &lmonth, &lday);
    enddays = jf_date_convertDateToDaysFrom1970(lyear, lmonth, lday);

    if (ol_strlen(pstrStartDate) == 0)
    {
        /*the first day*/
        jf_date_getDate2FromString(buffer->dds_strDate, &lyear, &lmonth, &lday);
    }
    else
    {
        jf_date_getDate2FromString(pstrStartDate, &lyear, &lmonth, &lday);
    }
    ldays = jf_date_convertDateToDaysFrom1970(lyear, lmonth, lday);

    while (ldays <= enddays)
    {
        jf_date_convertDaysFrom1970ToDate(ldays, &lyear, &lmonth, &lday);
        if (jf_date_isWeekendForDate(lyear, lmonth, lday) ||
            isHoliday(lyear, lmonth, lday))
        {
            ldays ++;
            continue;
        }

        jf_date_getStringDate2(strDate, lyear, lmonth, lday);

        total = daySummaryEndDataCount(buffer, num, strDate);
        if (total == 0)
        {
            ldays ++;
            continue;
        }

        pbre->bre_dbAsset = pbre->bre_pbrResult->br_dbFund;
        pbre->bre_nStock = 0;
        pbre->bre_bSaveAsset = FALSE;

        u32Ret = _backtestingFindAndTradeStockInModel(
            pbp, pbre, stockinfo, buffer, total);

        _backtestingCalcAssetDayByDay(pbp, pbre, strDate);
        ldays ++;
    }

    getNextTradingDate(strDate, pstrStartDate);

    return u32Ret;
}

static u32 _summaryBacktestingResult(backtesting_result_t * pbr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count = 0;
    trade_trading_record_t * traderecord = NULL;
    trade_trading_record_t * pBuy, * pSell, * pEnd;

    count = getNumOfTradingRecordInTradePersistency();

    if (count == 0)
        return u32Ret;

    jf_logger_logInfoMsg("summary backtesting result, %d record.", count);

    jf_jiukun_allocMemory(
        (void **)&traderecord, count * sizeof(trade_trading_record_t), 0);

    u32Ret = getAllTradingRecordInTradePersistency(traderecord, &count);
    if ((u32Ret == JF_ERR_NO_ERROR) && (count > 0))
    {
        pEnd = traderecord + count - 1;
        pBuy = traderecord;
        while (pBuy <= pEnd)
        {
            if (isTradeTradingRecordOpBuy(pBuy))
            {
                pbr->br_u32NumOfTrade ++;
                /*Buy operation, find sell*/
                pSell = pBuy + 1;
                while (pSell <= pEnd)
                {
                    if (isTradeTradingRecordOpSell(pSell) &&
                        (ol_strcmp(pSell->ttr_strStock, pBuy->ttr_strStock) == 0) &&
                        (ol_strcmp(pSell->ttr_strModel, pBuy->ttr_strModel) == 0)) 
                        break;
                    pSell ++;
                }

                if (pSell <= pEnd)
                {
                    /*found the sell record*/
                    if (pSell->ttr_dbPrice > pBuy->ttr_dbPrice)
                        pbr->br_u32NumOfTradeProfit ++;
                    else
                        pbr->br_u32NumOfTradeLoss ++;
                }
                else
                {
                    /*no sell record for the buy operation*/
                    pbr->br_u32NumOfTradeLoss ++;
                    pbr->br_dbFund += pBuy->ttr_dbPrice * (oldouble_t)pBuy->ttr_nVolume;
                }
            }
            pBuy ++;
        }

    }

    jf_jiukun_freeMemory((void **)&traderecord);

    if (pbr->br_dbMinAsset < pbr->br_dbInitialFund)
        pbr->br_dbMaxDrawdown =
            (pbr->br_dbInitialFund - pbr->br_dbMinAsset) * 100 / pbr->br_dbInitialFund;

    pbr->br_dbRateOfReturn =
        (pbr->br_dbFund - pbr->br_dbInitialFund) * 100 / pbr->br_dbInitialFund;
    
    return u32Ret;
}

static u32 _wrapupPoolStock(backtesting_result_t * pbr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index = 0, count = 0;
    trade_pool_stock_t * stockinpool = NULL, * pStock = NULL;
    trade_trading_record_t ttr;

    count = getNumOfPoolStockInTradePersistency();
    if (count == 0)
        return u32Ret;

    jf_logger_logInfoMsg("wrapup pool stock, %d stocks", count);

    jf_jiukun_allocMemory(
        (void **)&stockinpool, count * sizeof(trade_pool_stock_t), 0);

    u32Ret = getAllPoolStockInTradePersistency(stockinpool, &count);
    if ((u32Ret == JF_ERR_NO_ERROR) && (count > 0))
    {
        pStock = stockinpool;
        for (index = 0; index < count; index ++)
        {
            if (isTradePoolStockOpBuy(pStock))
            {
                /*found a bought stock without sold */
                jf_logger_logInfoMsg("wrapup pool stock, stock: %s", pStock->tps_strStock);
                setTradePoolStockOpSell(pStock);
                ol_strcpy(pStock->tps_strOpRemark, "wrap up");
                setTradeTradingRecord(&ttr, pStock);
                insertTradingRecordIntoTradePersistency(&ttr);

                pbr->br_dbFund += pStock->tps_dbPrice * (oldouble_t)pStock->tps_nVolume;
            }

            removePoolStockFromTradePersistency(pStock);
            pStock ++;
        }
    }

    jf_jiukun_freeMemory((void **)&stockinpool);

    return u32Ret;
}

static u32 _startBacktestingModel(
    backtesting_param_t * pbp, backtesting_result_ext_t * pbre)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = MAX_NUM_OF_DDS_FOR_BACKTESTING;
    da_day_summary_t * buffer = NULL, * middle = NULL;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    stock_info_t * stockinfo;
    olchar_t strStartDate[16] = {'\0'};
    olchar_t strDate[16] = {'\0'};
    olchar_t * pstrStartDate = NULL;
    boolean_t bLast = FALSE;

    clearDataInTradePersistency();
    strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';

    jf_jiukun_allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);

    stockinfo = getFirstStockInfo();
    while (stockinfo != NULL)
    {
        jf_logger_logInfoMsg("start backtesting model, stock: %s", stockinfo->si_strCode);

        ol_snprintf(
            strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s.txt",
            BACKTESTING_OUTPUT_DIR,
            PATH_SEPARATOR, stockinfo->si_strCode);
        u32Ret = jf_filestream_open(strFullname, "w", &pbre->bre_pjfAsset);
        if (u32Ret != JF_ERR_NO_ERROR)
            break;

        ol_snprintf(
            strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
            pbp->bp_pstrStockPath,
            PATH_SEPARATOR, stockinfo->si_strCode);

        pstrStartDate = NULL;
        strDate[0] = '\0';

        do
        {
            bLast = FALSE;
            total = MAX_NUM_OF_DDS_FOR_BACKTESTING;

            u32Ret = readTradeDaySummaryFromDateWithFRoR(
                strFullname, pstrStartDate, buffer, &total);
            if ((u32Ret == JF_ERR_NO_ERROR) &&
                (total != MAX_NUM_OF_DDS_FOR_BACKTESTING))
            {
                bLast = TRUE;
            }

            if ((u32Ret == JF_ERR_NO_ERROR) && (pstrStartDate == NULL))
            {
                /*the first day*/
                pbre->bre_dbAsset = pbre->bre_pbrResult->br_dbFund;
                pbre->bre_nStock = 0;
                _saveAsset(pbp, pbre, buffer->dds_strDate);
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = _backtestingFindAndTradeStock(
                    pbp, pbre, stockinfo, strDate, buffer, total);
            }

            middle = buffer + total / 2;
            ol_strcpy(strStartDate, middle->dds_strDate);
            jf_logger_logInfoMsg("start backtesting model, startdate: %s", strStartDate);
            pstrStartDate = strStartDate;

        } while (! bLast);

        /*in case the stock is bought but not sold out, we should sell it out forcely*/
        _wrapupPoolStock(pbre->bre_pbrResult);
        jf_filestream_close(&pbre->bre_pjfAsset);

        stockinfo = getNextStockInfo(stockinfo);
    }

    jf_jiukun_freeMemory((void **)&buffer);

    return u32Ret;
}

static u32 _backtestingFindStockInModelDayByDay(
    backtesting_param_t * pbp,
    stock_info_t * stockinfo, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index, max = DA_MODEL_MAX_ID;
    da_model_t * model = NULL;
    trade_pool_stock_t tps;

    if (num == 0)
        return u32Ret;

    for (index = 0; index < max; index ++)
    {
        u32Ret = getDaModel((da_model_id_t)index, &model);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if ((pbp->bp_pstrModel != NULL) &&
                ol_strcmp(model->dm_strName, pbp->bp_pstrModel) != 0)
                continue;
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            initTradePoolStock(&tps, stockinfo->si_strCode, model->dm_strName);

            u32Ret = getPoolStockInTradePersistency(&tps);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                /*the stock is in pool*/
                jf_logger_logInfoMsg("%s is already in pool", stockinfo->si_strCode);
            }
            else
            {
                /* The stock is not in pool */
                u32Ret = model->dm_fnCanBeTraded(model, stockinfo, &tps, buffer, num);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    u32Ret = insertPoolStockIntoTradePersistency(&tps);
                }
            }
        }
    }

    return u32Ret;
}

static u32 _backtestingFindStockDayByDay(
    backtesting_param_t * pbp,
    olchar_t * pstrBacktestDate, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stock_info_t * stockinfo;
    olint_t total = num;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    boolean_t bAfter;

    jf_logger_logDebugMsg(
        "backtesting find stock day by day, last day: %s", pstrBacktestDate);
    
    stockinfo = getFirstStockInfo();
    while (stockinfo != NULL)
    {
        bAfter = isAfterStockFirstTradeDate(stockinfo, pstrBacktestDate);
        if (! bAfter)
        {
            stockinfo = getNextStockInfo(stockinfo);
            continue;
        }

        ol_snprintf(
            strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
            pbp->bp_pstrStockPath,
            PATH_SEPARATOR, stockinfo->si_strCode);
        strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';
        total = num;

        u32Ret = readTradeDaySummaryUntilDateWithFRoR(
            strFullname, pstrBacktestDate, buffer, &total);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _backtestingFindStockInModelDayByDay(
                pbp, stockinfo, buffer, total);
        }

        stockinfo = getNextStockInfo(stockinfo);
    }

    return JF_ERR_NO_ERROR;
}

static u32 _backtestingSellOneStockDayByDay(
    backtesting_param_t * pbp, backtesting_result_ext_t * pbre,
    stock_info_t * stockinfo, da_model_t * model,
    trade_pool_stock_t * ptps,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    trade_trading_record_t ttr;
    da_model_trade_data_t dmtd;
    backtesting_result_t * pbr = pbre->bre_pbrResult;
    da_day_summary_t * end;

    ol_bzero(&dmtd, sizeof(dmtd));
    
    /*Bought already, try to sell*/
    u32Ret = model->dm_fnTrade(model, stockinfo, ptps, &dmtd, buffer, num);
    if ((u32Ret == JF_ERR_NO_ERROR) && isTradePoolStockOpSell(ptps))
    {
        /*the stock is sold out*/
        /*add trading record*/
        setTradeTradingRecord(&ttr, ptps);
        insertTradingRecordIntoTradePersistency(&ttr);
        /*delete pool stock*/
        removePoolStockFromTradePersistency(ptps);
        pbr->br_dbFund += dmtd.dmtd_dbFund;
        pbre->bre_dbAsset += dmtd.dmtd_dbFund;
        pbre->bre_bSaveAsset = TRUE;
    }
    else
    {
        pbre->bre_nStock ++;
        end = buffer + num - 1;
        pbre->bre_bSaveAsset = TRUE;
        /*the asset already include the stock bought today*/
        if (ol_strcmp(end->dds_strDate, ptps->tps_strTradeDate) > 0)
        {
            pbre->bre_dbAsset += end->dds_dbClosingPrice * (oldouble_t)ptps->tps_nVolume;
        }
    }
    jf_logger_logInfoMsg(
        "backtesting sell one stock day by day, fund: %.2f, asset: %.2f",
        pbr->br_dbFund, pbre->bre_dbAsset);
    
    return u32Ret;
}

static u32 _backtestingSellStockDayByDay(
    backtesting_param_t * pbp, backtesting_result_ext_t * pbre,
    trade_pool_stock_t ** ptpsOp, olint_t opCount,
    olchar_t * pstrBacktestDate, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index;
    da_model_t * model = NULL;
    stock_info_t * stockinfo;
    trade_pool_stock_t * pStock = NULL;
    olint_t total = num;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];

    if (opCount == 0)
        return u32Ret;

    jf_logger_logInfoMsg(
        "backtesting sell stock day by day, count: %d", opCount);

    for (index = 0; index < opCount; index ++)
    {
        pStock = ptpsOp[index];

        u32Ret = getDaModelByName(pStock->tps_strModel, &model);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = getStockInfo(pStock->tps_strStock, &stockinfo);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_snprintf(
                strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
                pbp->bp_pstrStockPath,
                PATH_SEPARATOR, stockinfo->si_strCode);
            strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';
            total = num;

            u32Ret = readTradeDaySummaryUntilDateWithFRoR(
                strFullname, pstrBacktestDate, buffer, &total);
        }
            
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _backtestingSellOneStockDayByDay(
                pbp, pbre, stockinfo, model, pStock, buffer, total);
        }
    }

    return u32Ret;
}

static u32 sortPoolStockForBuyOp(
    backtesting_param_t * pbp,
    trade_pool_stock_t ** ptpsOp, olint_t opCount,
    olchar_t * pstrBacktestDate, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    /*ToDo: select the most valuable stocks */
    
    return u32Ret;
}

static u32 _backtestingBuyOneStockDayByDay(
    backtesting_param_t * pbp, backtesting_result_ext_t * pbre,
    stock_info_t * stockinfo, da_model_t * model,
    trade_pool_stock_t * ptps,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    trade_trading_record_t ttr;
    da_model_trade_data_t dmtd;
    oldouble_t maxfund;
    backtesting_result_t * pbr = pbre->bre_pbrResult;

    ol_bzero(&dmtd, sizeof(dmtd));
    
    /*ToDo: determine fund which can be used for one stock*/
#define MAX_PERCENT_OF_FUND_FOR_ONE_STOCK     (0.2)
#define MAX_FUND_FOR_ONE_STOCK                (10000)
    dmtd.dmtd_dbFund = pbr->br_dbFund;
    maxfund = pbr->br_dbInitialFund * MAX_PERCENT_OF_FUND_FOR_ONE_STOCK;
    if (dmtd.dmtd_dbFund > maxfund)
        dmtd.dmtd_dbFund = maxfund;
    if (dmtd.dmtd_dbFund < MAX_FUND_FOR_ONE_STOCK)
    {
        jf_logger_logErrMsg(JF_ERR_OPERATION_FAIL, "No enough fund");
        /*delete pool stock*/
        removePoolStockFromTradePersistency(ptps);
        return u32Ret;
    }

    pbr->br_dbFund -= dmtd.dmtd_dbFund;
    /*not bought yet*/
    u32Ret = model->dm_fnTrade(model, stockinfo, ptps, &dmtd, buffer, num);
    if ((u32Ret == JF_ERR_NO_ERROR) && isTradePoolStockOpBuy(ptps))
    {
        /*the stock is bought*/
        /*update pool stock*/
        updatePoolStockInTradePersistency(ptps);
        /*add trading record*/
        setTradeTradingRecord(&ttr, ptps);
        insertTradingRecordIntoTradePersistency(&ttr);

        
    }
    pbr->br_dbFund += dmtd.dmtd_dbFund;
    jf_logger_logInfoMsg(
        "backtesting buy one stock day by day, fund: %.2f", pbr->br_dbFund);

    return u32Ret;
}

static u32 _backtestingBuyStockDayByDay(
    backtesting_param_t * pbp, backtesting_result_ext_t * pbre,
    trade_pool_stock_t ** ptpsOp, olint_t opCount,
    olchar_t * pstrBacktestDate, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index;
    da_model_t * model = NULL;
    stock_info_t * stockinfo;
    trade_pool_stock_t * pStock = NULL;
    olint_t total = num;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];

    if (opCount == 0)
        return u32Ret;

    jf_logger_logInfoMsg(
        "backtesting buy stock day by day, count: %d", opCount);
    
    for (index = 0; index < opCount; index ++)
    {
        pStock = ptpsOp[index];

        u32Ret = getDaModelByName(pStock->tps_strModel, &model);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = getStockInfo(pStock->tps_strStock, &stockinfo);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_snprintf(
                strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
                pbp->bp_pstrStockPath,
                PATH_SEPARATOR, stockinfo->si_strCode);
            strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';
            total = num;

            u32Ret = readTradeDaySummaryUntilDateWithFRoR(
                strFullname, pstrBacktestDate, buffer, &total);
        }
            
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _backtestingBuyOneStockDayByDay(
                pbp, pbre, stockinfo, model, pStock, buffer, total);
        }
    }

    return u32Ret;
}

static u32 _backtestingTradeStockDayByDay(
    backtesting_param_t * pbp, backtesting_result_ext_t * pbre,
    olchar_t * pstrBacktestDate,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count;
    trade_pool_stock_t * ptps = NULL;
    olint_t opCount;
    trade_pool_stock_t ** ptpsOp = NULL;

    pbre->bre_dbAsset = pbre->bre_pbrResult->br_dbFund;
    pbre->bre_nStock = 0;
    pbre->bre_bSaveAsset = FALSE;

    count = getNumOfPoolStockInTradePersistency();

    if (count == 0)
        return u32Ret;

    jf_logger_logInfoMsg(
        "backtesting trade stock day by day, count: %d", count);

    jf_jiukun_allocMemory(
        (void **)&ptps, count * sizeof(trade_pool_stock_t), 0);

    u32Ret = getAllPoolStockInTradePersistency(ptps, &count);
    if ((u32Ret == JF_ERR_NO_ERROR) && (count > 0))
    {
        opCount = count;
        jf_jiukun_allocMemory((void **)&ptpsOp, opCount * sizeof(trade_pool_stock_t *), 0);
        /*1. try to buy stock*/
        filterPoolStockByOp(ptps, count, STOCK_OP_NONE, ptpsOp, &opCount);
        sortPoolStockForBuyOp(pbp, ptpsOp, opCount, pstrBacktestDate, buffer, num);
        u32Ret = _backtestingBuyStockDayByDay(
            pbp, pbre, ptpsOp, opCount, pstrBacktestDate, buffer, num);

        /*2. try to sell stock*/
        opCount = count;
        filterPoolStockByOp(ptps, count, STOCK_OP_BUY, ptpsOp, &opCount);
        u32Ret = _backtestingSellStockDayByDay(
            pbp, pbre, ptpsOp, opCount, pstrBacktestDate, buffer, num);

        jf_jiukun_freeMemory((void **)&ptpsOp);

        _backtestingCalcAssetDayByDay(pbp, pbre, pstrBacktestDate);
    }

    jf_jiukun_freeMemory((void **)&ptps);
    
    return u32Ret;
}

static u32 _startBacktestingModelDayByDay(
    backtesting_param_t * pbp, backtesting_result_ext_t * pbre)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t lyear, lmonth, lday;
    olint_t ldays, enddays;
    olchar_t strBacktestDate[16];
    da_day_summary_t * buffer = NULL;
    olint_t total = 400; //MAX_NUM_OF_DDS_FOR_BACKTESTING;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];

    ol_sprintf(
        strFullname, "%s%c%s",
        BACKTESTING_OUTPUT_DIR, PATH_SEPARATOR, BACKTESTING_OUTPUT_FILE);
    
    u32Ret = jf_filestream_open(strFullname, "w", &pbre->bre_pjfAsset);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;
    
    jf_logger_logInfoMsg("backtesting model day by day");
    clearDataInTradePersistency();

    jf_jiukun_allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);

    jf_date_getDate2FromString(
        ls_pstrBacktestingStartDateDayByDay, &lyear, &lmonth, &lday);
    ldays = jf_date_convertDateToDaysFrom1970(lyear, lmonth, lday);

    jf_date_getDateToday(&lyear, &lmonth, &lday);
    enddays = jf_date_convertDateToDaysFrom1970(lyear, lmonth, lday);

    pbre->bre_dbAsset = pbre->bre_pbrResult->br_dbInitialFund;
    pbre->bre_nStock = 0;
    _saveAsset(pbp, pbre, ls_pstrBacktestingStartDateDayByDay);
    
    while (ldays <= enddays)
    {
        /*determine the start date for reading day summary*/
        jf_date_convertDaysFrom1970ToDate(ldays, &lyear, &lmonth, &lday);
        if (jf_date_isWeekendForDate(lyear, lmonth, lday) ||
            isHoliday(lyear, lmonth, lday))
        {
            ldays ++;
            continue;
        }

        jf_date_getStringDate2(strBacktestDate, lyear, lmonth, lday);
        jf_logger_logDebugMsg("backtesting model day by day, %s", strBacktestDate);

        u32Ret = _backtestingFindStockDayByDay(
            pbp, strBacktestDate, buffer, total);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _backtestingTradeStockDayByDay(
                pbp, pbre, strBacktestDate, buffer, total);
        }

        ldays ++;
    }

    jf_jiukun_freeMemory((void **)&buffer);

    /*in case the stock is bought but not sold out, we should sell it out forcely*/
    _wrapupPoolStock(pbre->bre_pbrResult);

    jf_filestream_close(&pbre->bre_pjfAsset);
    
    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 backtestingModel(backtesting_param_t * pbp, backtesting_result_t * pbr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_model_t * model = NULL;
    backtesting_result_ext_t bre;

    if (! pbp->bp_bAllModel)
    {
        assert(pbp->bp_pstrModel != NULL);

        u32Ret = getDaModelByName(pbp->bp_pstrModel, &model);
        if (u32Ret != JF_ERR_NO_ERROR)
            u32Ret = JF_ERR_INVALID_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_dir_create(BACKTESTING_OUTPUT_DIR, JF_DIR_DEFAULT_CREATE_MODE);
        if (u32Ret == JF_ERR_DIR_ALREADY_EXIST)
            u32Ret = JF_ERR_NO_ERROR;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = setStockFirstTradeDate(pbp->bp_pstrStockPath);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pbr, sizeof(backtesting_result_t));
        pbr->br_dbInitialFund = pbp->bp_dbInitialFund;
        pbr->br_dbFund = pbp->bp_dbInitialFund;
        pbr->br_dbMinAsset = pbr->br_dbMaxAsset = pbr->br_dbFund;

        ol_bzero(&bre, sizeof(backtesting_result_ext_t));
        bre.bre_pbrResult = pbr;

        if (pbp->bp_u8Method == BACKTESTING_METHOD_DAY_BY_DAY)
            u32Ret = _startBacktestingModelDayByDay(pbp, &bre);
        else
            u32Ret = _startBacktestingModel(pbp, &bre);

        _summaryBacktestingResult(pbr);
    }

    return u32Ret;
}


/*---------------------------------------------------------------------------*/


