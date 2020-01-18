/**
 *  @file backtesting.c
 *
 *  @brief Backtesting module.
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
#include "parsedata.h"
#include "indicator.h"
#include "datastat.h"
#include "statarbitrage.h"
#include "tx_model.h"
#include "trade_persistency.h"
#include "tx_backtesting.h"
#include "tx_trade.h"

/* --- private data/data structure section ------------------------------------------------------ */
static olchar_t * ls_pstrBacktestingStartDateDayByDay = "2014-01-01";

#define MAX_NUM_OF_DDS_FOR_BACKTESTING   (800)
#define BACKTESTING_OUTPUT_DIR           "output"
#define BACKTESTING_OUTPUT_FILE          "asset.txt"

typedef struct
{
    tx_backtesting_result_t * bre_ptbrResult;
    jf_filestream_t * bre_pjfAsset;
    double bre_dbAsset;
    olint_t bre_nStock;
    boolean_t bre_bSaveAsset;
} backtesting_result_ext_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _backtestingFindAndTradeStockInModel(
    tx_backtesting_param_t * ptbp, backtesting_result_ext_t * pbre, tx_stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_model_t * model = NULL;
    da_day_summary_t * end;
    trade_pool_stock_t tps;
    trade_trading_record_t ttr;
    tx_model_trade_data_t tmtd;
    tx_backtesting_result_t * ptbr = pbre->bre_ptbrResult;

    end = buffer + num - 1;
    JF_LOGGER_INFO("total: %d, last day: %s", num, end->dds_strDate);

    model = tx_model_getFirstModel();
    while (model != NULL)
    {
        if ((ptbp->tbp_pstrModel != NULL) &&
            ol_strcmp(model->tm_strName, ptbp->tbp_pstrModel) != 0)
        {
            model = tx_model_getNextModel(model);
            continue;
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            initTradePoolStock(&tps, stockinfo->tsi_strCode, model->tm_strName);
            initTradeTradingRecord(
                &ttr, stockinfo->tsi_strCode, model->tm_strName);

            u32Ret = getPoolStockInTradePersistency(&tps);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                /*the stock is in pool*/
                JF_LOGGER_INFO("%s is already in pool", stockinfo->tsi_strCode);

                ol_bzero(&tmtd, sizeof(tmtd));

                if (isTradePoolStockOpNone(&tps))
                {
                    JF_LOGGER_INFO("before buy, fund: %.2f", ptbr->tbr_dbFund);
                    tmtd.tmtd_dbFund = ptbr->tbr_dbFund;
                    /*not bought yet*/
                    u32Ret = model->tm_fnTrade(model, stockinfo, &tps, &tmtd, buffer, num);
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
                    ptbr->tbr_dbFund = tmtd.tmtd_dbFund;
                    JF_LOGGER_INFO("after buy, fund: %.2f", ptbr->tbr_dbFund);
                }
                else if (isTradePoolStockOpBuy(&tps))
                {
                    JF_LOGGER_INFO("before sell, fund: %.2f", ptbr->tbr_dbFund);
                    /*Bought already, try to sell*/
                    u32Ret = model->tm_fnTrade(model, stockinfo, &tps, &tmtd, buffer, num);
                    if ((u32Ret == JF_ERR_NO_ERROR) && isTradePoolStockOpSell(&tps))
                    {
                        /*the stock is sold out*/
                        /*add trading record*/
                        setTradeTradingRecord(&ttr, &tps);
                        insertTradingRecordIntoTradePersistency(&ttr);
                        /*delete pool stock*/
                        removePoolStockFromTradePersistency(&tps);
                        ptbr->tbr_dbFund += tmtd.tmtd_dbFund;
                        pbre->bre_dbAsset += tmtd.tmtd_dbFund;
                        pbre->bre_bSaveAsset = TRUE;
                        JF_LOGGER_INFO("after sell, fund: %.2f", ptbr->tbr_dbFund);
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
                u32Ret = model->tm_fnCanBeTraded(model, stockinfo, &tps, buffer, num);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    u32Ret = insertPoolStockIntoTradePersistency(&tps);
                }
            }
        }

        model = tx_model_getNextModel(model);
    }

    return u32Ret;
}

static u32 _saveAsset(
    tx_backtesting_param_t * ptbp, backtesting_result_ext_t * pbre, olchar_t * pstrDate)
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
    tx_backtesting_param_t * ptbp, backtesting_result_ext_t * pbre, olchar_t * pstrBacktestDate)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_backtesting_result_t * ptbr = pbre->bre_ptbrResult;

    if (pbre->bre_bSaveAsset)
    {
        if (ptbr->tbr_dbMinAsset > pbre->bre_dbAsset)
            ptbr->tbr_dbMinAsset = pbre->bre_dbAsset;
        if (ptbr->tbr_dbMaxAsset < pbre->bre_dbAsset)
            ptbr->tbr_dbMaxAsset = pbre->bre_dbAsset;

        u32Ret = _saveAsset(ptbp, pbre, pstrBacktestDate);
    }

    return u32Ret;
}

static u32 _backtestingFindAndTradeStock(
    tx_backtesting_param_t * ptbp, backtesting_result_ext_t * pbre,
    tx_stock_info_t * stockinfo, olchar_t * pstrStartDate, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t lyear, lmonth, lday;
    olint_t ldays, enddays;
    olchar_t strDate[16];
    olint_t total = num;
    da_day_summary_t * end = buffer + num - 1;

    JF_LOGGER_INFO("start date: %s", pstrStartDate);

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
        if (isHoliday(ldays))
        {
            ldays ++;
            continue;
        }

        jf_date_getStringDate2ForDaysFrom1970(strDate, ldays);

        total = daySummaryEndDataCount(buffer, num, strDate);
        if (total == 0)
        {
            ldays ++;
            continue;
        }

        pbre->bre_dbAsset = pbre->bre_ptbrResult->tbr_dbFund;
        pbre->bre_nStock = 0;
        pbre->bre_bSaveAsset = FALSE;

        u32Ret = _backtestingFindAndTradeStockInModel(
            ptbp, pbre, stockinfo, buffer, total);

        _backtestingCalcAssetDayByDay(ptbp, pbre, strDate);
        ldays ++;
    }

    getNextTradingDate(strDate, pstrStartDate);

    return u32Ret;
}

static u32 _summaryBacktestingResult(tx_backtesting_result_t * ptbr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count = 0;
    trade_trading_record_t * traderecord = NULL;
    trade_trading_record_t * pBuy, * pSell, * pEnd;

    count = getNumOfTradingRecordInTradePersistency();

    if (count == 0)
        return u32Ret;

    JF_LOGGER_INFO("%d record.", count);

    jf_jiukun_allocMemory((void **)&traderecord, count * sizeof(trade_trading_record_t));

    u32Ret = getAllTradingRecordInTradePersistency(traderecord, &count);
    if ((u32Ret == JF_ERR_NO_ERROR) && (count > 0))
    {
        pEnd = traderecord + count - 1;
        pBuy = traderecord;
        while (pBuy <= pEnd)
        {
            if (isTradeTradingRecordOpBuy(pBuy))
            {
                ptbr->tbr_u32NumOfTrade ++;
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
                        ptbr->tbr_u32NumOfTradeProfit ++;
                    else
                        ptbr->tbr_u32NumOfTradeLoss ++;
                }
                else
                {
                    /*no sell record for the buy operation*/
                    ptbr->tbr_u32NumOfTradeLoss ++;
                    ptbr->tbr_dbFund += pBuy->ttr_dbPrice * (oldouble_t)pBuy->ttr_nVolume;
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

static u32 _wrapupPoolStock(tx_backtesting_result_t * ptbr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index = 0, count = 0;
    trade_pool_stock_t * stockinpool = NULL, * pStock = NULL;
    trade_trading_record_t ttr;

    count = getNumOfPoolStockInTradePersistency();
    if (count == 0)
        return u32Ret;

    JF_LOGGER_INFO("%d stocks", count);

    jf_jiukun_allocMemory((void **)&stockinpool, count * sizeof(trade_pool_stock_t));

    u32Ret = getAllPoolStockInTradePersistency(stockinpool, &count);
    if ((u32Ret == JF_ERR_NO_ERROR) && (count > 0))
    {
        pStock = stockinpool;
        for (index = 0; index < count; index ++)
        {
            if (isTradePoolStockOpBuy(pStock))
            {
                /*found a bought stock without sold */
                JF_LOGGER_INFO("stock: %s", pStock->tps_strStock);
                setTradePoolStockOpSell(pStock);
                ol_strcpy(pStock->tps_strOpRemark, "wrap up");
                setTradeTradingRecord(&ttr, pStock);
                insertTradingRecordIntoTradePersistency(&ttr);

                ptbr->tbr_dbFund += pStock->tps_dbPrice * (oldouble_t)pStock->tps_nVolume;
            }

            removePoolStockFromTradePersistency(pStock);
            pStock ++;
        }
    }

    jf_jiukun_freeMemory((void **)&stockinpool);

    return u32Ret;
}

static u32 _startBacktestingModel(
    tx_backtesting_param_t * ptbp, backtesting_result_ext_t * pbre)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = MAX_NUM_OF_DDS_FOR_BACKTESTING;
    da_day_summary_t * buffer = NULL, * middle = NULL;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    tx_stock_info_t * stockinfo;
    olchar_t strStartDate[16] = {'\0'};
    olchar_t strDate[16] = {'\0'};
    olchar_t * pstrStartDate = NULL;
    boolean_t bLast = FALSE;

    clearDataInTradePersistency();
    strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';

    jf_jiukun_allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total);

    stockinfo = getFirstStockInfo();
    while (stockinfo != NULL)
    {
        JF_LOGGER_INFO("stock: %s", stockinfo->tsi_strCode);

        ol_snprintf(
            strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s.txt",
            BACKTESTING_OUTPUT_DIR, PATH_SEPARATOR, stockinfo->tsi_strCode);
        u32Ret = jf_filestream_open(strFullname, "w", &pbre->bre_pjfAsset);
        if (u32Ret != JF_ERR_NO_ERROR)
            break;

        ol_snprintf(
            strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
            ptbp->tbp_pstrStockPath, PATH_SEPARATOR, stockinfo->tsi_strCode);

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
                pbre->bre_dbAsset = pbre->bre_ptbrResult->tbr_dbFund;
                pbre->bre_nStock = 0;
                _saveAsset(ptbp, pbre, buffer->dds_strDate);
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = _backtestingFindAndTradeStock(
                    ptbp, pbre, stockinfo, strDate, buffer, total);
            }

            middle = buffer + total / 2;
            ol_strcpy(strStartDate, middle->dds_strDate);
            JF_LOGGER_INFO("startdate: %s", strStartDate);
            pstrStartDate = strStartDate;

        } while (! bLast);

        /*in case the stock is bought but not sold out, we should sell it out forcely*/
        _wrapupPoolStock(pbre->bre_ptbrResult);
        jf_filestream_close(&pbre->bre_pjfAsset);

        stockinfo = getNextStockInfo(stockinfo);
    }

    jf_jiukun_freeMemory((void **)&buffer);

    return u32Ret;
}

static u32 _backtestingFindStockInModelDayByDay(
    tx_backtesting_param_t * ptbp, olchar_t * pstrBacktestDate, tx_stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_model_t * model = NULL;
    trade_pool_stock_t tps;
    da_day_summary_t * pdds;
    olint_t total = num;

    if (num == 0)
        return u32Ret;

    model = tx_model_getFirstModel();
    while (model != NULL)
    {
        if ((ptbp->tbp_pstrModel != NULL) &&
            ol_strcasecmp(model->tm_strName, ptbp->tbp_pstrModel) != 0)
        {
            model = tx_model_getNextModel(model);
            continue;
        }

        initTradePoolStock(&tps, stockinfo->tsi_strCode, model->tm_strName);

        u32Ret = getPoolStockInTradePersistency(&tps);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*the stock is in pool*/
            JF_LOGGER_INFO("%s is already in pool", stockinfo->tsi_strCode);
        }
        else
        {
            /* The stock is not in pool */
            u32Ret = getDaySummaryWithDate(buffer, total, pstrBacktestDate, &pdds);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                total = pdds - buffer + 1;

                u32Ret = model->tm_fnCanBeTraded(model, stockinfo, &tps, buffer, total);
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                tps.tps_nNumOfDaySummary = num;
                ol_strcpy(tps.tps_strStartDateOfDaySummary, buffer->dds_strDate);
                u32Ret = insertPoolStockIntoTradePersistency(&tps);
            }
        }

        model = tx_model_getNextModel(model);
    }

    return u32Ret;
}

static u32 _backtestingFindStockDayByDay(
    tx_backtesting_param_t * ptbp, olchar_t * pstrBacktestDate, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_info_t * stockinfo;
    olint_t total;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    boolean_t bAfter;

    JF_LOGGER_DEBUG("last day: %s", pstrBacktestDate);
    
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
            ptbp->tbp_pstrStockPath, PATH_SEPARATOR, stockinfo->tsi_strCode);
        strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';
        total = num;

#define BACKTESTING_MAX_COUNT_AFTER_END_DATE    (100)    
        u32Ret = readTradeDaySummaryUntilDateWithFRoR(
            strFullname, pstrBacktestDate, BACKTESTING_MAX_COUNT_AFTER_END_DATE, buffer, &total);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _backtestingFindStockInModelDayByDay(
                ptbp, pstrBacktestDate, stockinfo, buffer, total);
        }

        stockinfo = getNextStockInfo(stockinfo);
    }

    return JF_ERR_NO_ERROR;
}

static u32 _backtestingSellOneStockDayByDay(
    tx_backtesting_param_t * ptbp, backtesting_result_ext_t * pbre, tx_stock_info_t * stockinfo,
    tx_model_t * model, trade_pool_stock_t * ptps, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    trade_trading_record_t ttr;
    tx_model_trade_data_t tmtd;
    tx_backtesting_result_t * ptbr = pbre->bre_ptbrResult;
    da_day_summary_t * end;

    ol_bzero(&tmtd, sizeof(tmtd));
    
    /*Bought already, try to sell*/
    u32Ret = model->tm_fnTrade(model, stockinfo, ptps, &tmtd, buffer, num);
    if ((u32Ret == JF_ERR_NO_ERROR) && isTradePoolStockOpSell(ptps))
    {
        /*the stock is sold out*/
        /*add trading record*/
        setTradeTradingRecord(&ttr, ptps);
        insertTradingRecordIntoTradePersistency(&ttr);
        /*delete pool stock*/
        removePoolStockFromTradePersistency(ptps);
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
        if (ol_strcmp(end->dds_strDate, ptps->tps_strTradeDate) > 0)
        {
            pbre->bre_dbAsset += end->dds_dbClosingPrice * (oldouble_t)ptps->tps_nVolume;
        }
    }
    jf_logger_logInfoMsg(
        "backtesting sell one stock day by day, fund: %.2f, asset: %.2f",
        ptbr->tbr_dbFund, pbre->bre_dbAsset);
    
    return u32Ret;
}

static u32 _backtestingSellStockDayByDay(
    tx_backtesting_param_t * ptbp, backtesting_result_ext_t * pbre, trade_pool_stock_t ** ptpsOp,
    olint_t opCount, olchar_t * pstrBacktestDate, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index;
    tx_model_t * model = NULL;
    tx_stock_info_t * stockinfo;
    trade_pool_stock_t * pStock = NULL;
    olint_t total = num;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    da_day_summary_t * pdds;

    if (opCount == 0)
        return u32Ret;

    JF_LOGGER_INFO("count: %d", opCount);

    for (index = 0; index < opCount; index ++)
    {
        pStock = ptpsOp[index];

        u32Ret = tx_model_getModel(pStock->tps_strModel, &model);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = getStockInfo(pStock->tps_strStock, &stockinfo);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_snprintf(
                strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
                ptbp->tbp_pstrStockPath, PATH_SEPARATOR, stockinfo->tsi_strCode);
            strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';
            total = pStock->tps_nNumOfDaySummary;

            u32Ret = readTradeDaySummaryFromDateWithFRoR(
                strFullname, pStock->tps_strStartDateOfDaySummary, buffer, &total);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = getDaySummaryWithDate(buffer, total, pstrBacktestDate, &pdds);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            total = pdds - buffer + 1;

            u32Ret = _backtestingSellOneStockDayByDay(
                ptbp, pbre, stockinfo, model, pStock, buffer, total);
        }
    }

    return u32Ret;
}

static u32 sortPoolStockForBuyOp(
    tx_backtesting_param_t * ptbp, trade_pool_stock_t ** ptpsOp, olint_t opCount,
    olchar_t * pstrBacktestDate, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    /*ToDo: select the most valuable stocks */
    
    return u32Ret;
}

static u32 _backtestingBuyOneStockDayByDay(
    tx_backtesting_param_t * ptbp, backtesting_result_ext_t * pbre, tx_stock_info_t * stockinfo,
    tx_model_t * model, trade_pool_stock_t * ptps, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    trade_trading_record_t ttr;
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
        removePoolStockFromTradePersistency(ptps);
        return u32Ret;
    }

    ptbr->tbr_dbFund -= tmtd.tmtd_dbFund;
    /*not bought yet*/
    u32Ret = model->tm_fnTrade(model, stockinfo, ptps, &tmtd, buffer, num);
    if ((u32Ret == JF_ERR_NO_ERROR) && isTradePoolStockOpBuy(ptps))
    {
        /*the stock is bought*/
        /*update pool stock*/
        updatePoolStockInTradePersistency(ptps);
        /*add trading record*/
        setTradeTradingRecord(&ttr, ptps);
        insertTradingRecordIntoTradePersistency(&ttr);
    }

    ptbr->tbr_dbFund += tmtd.tmtd_dbFund;
    jf_logger_logInfoMsg("backtesting buy one stock day by day, fund: %.2f", ptbr->tbr_dbFund);

    return u32Ret;
}

static u32 _backtestingBuyStockDayByDay(
    tx_backtesting_param_t * ptbp, backtesting_result_ext_t * pbre, trade_pool_stock_t ** ptpsOp,
    olint_t opCount, olchar_t * pstrBacktestDate, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index;
    tx_model_t * model = NULL;
    tx_stock_info_t * stockinfo;
    trade_pool_stock_t * pStock = NULL;
    olint_t total = num;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    da_day_summary_t * pdds;

    if (opCount == 0)
        return u32Ret;

    jf_logger_logInfoMsg("backtesting buy stock day by day, count: %d", opCount);
    
    for (index = 0; index < opCount; index ++)
    {
        pStock = ptpsOp[index];

        u32Ret = tx_model_getModel(pStock->tps_strModel, &model);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = getStockInfo(pStock->tps_strStock, &stockinfo);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_snprintf(
                strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
                ptbp->tbp_pstrStockPath, PATH_SEPARATOR, stockinfo->tsi_strCode);
            strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';
            total = pStock->tps_nNumOfDaySummary;

            u32Ret = readTradeDaySummaryFromDateWithFRoR(
                strFullname, pStock->tps_strStartDateOfDaySummary, buffer, &total);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = getDaySummaryWithDate(buffer, total, pstrBacktestDate, &pdds);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            total = pdds - buffer + 1;
        
            u32Ret = _backtestingBuyOneStockDayByDay(
                ptbp, pbre, stockinfo, model, pStock, buffer, total);
        }
    }

    return u32Ret;
}

static u32 _backtestingTradeStockDayByDay(
    tx_backtesting_param_t * ptbp, backtesting_result_ext_t * pbre,
    olchar_t * pstrBacktestDate, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count;
    trade_pool_stock_t * ptps = NULL;
    olint_t opCount;
    trade_pool_stock_t ** ptpsOp = NULL;

    pbre->bre_dbAsset = pbre->bre_ptbrResult->tbr_dbFund;
    pbre->bre_nStock = 0;
    pbre->bre_bSaveAsset = FALSE;

    count = getNumOfPoolStockInTradePersistency();

    if (count == 0)
        return u32Ret;

    jf_logger_logInfoMsg("backtesting trade stock day by day, count: %d", count);

    jf_jiukun_allocMemory((void **)&ptps, count * sizeof(trade_pool_stock_t));

    u32Ret = getAllPoolStockInTradePersistency(ptps, &count);
    if ((u32Ret == JF_ERR_NO_ERROR) && (count > 0))
    {
        opCount = count;
        jf_jiukun_allocMemory((void **)&ptpsOp, opCount * sizeof(trade_pool_stock_t *));
        /*1. try to buy stock*/
        filterPoolStockByOp(ptps, count, STOCK_OP_NONE, ptpsOp, &opCount);
        sortPoolStockForBuyOp(ptbp, ptpsOp, opCount, pstrBacktestDate, buffer, num);
        u32Ret = _backtestingBuyStockDayByDay(
            ptbp, pbre, ptpsOp, opCount, pstrBacktestDate, buffer, num);

        /*2. try to sell stock*/
        opCount = count;
        filterPoolStockByOp(ptps, count, STOCK_OP_BUY, ptpsOp, &opCount);
        u32Ret = _backtestingSellStockDayByDay(
            ptbp, pbre, ptpsOp, opCount, pstrBacktestDate, buffer, num);

        jf_jiukun_freeMemory((void **)&ptpsOp);

        _backtestingCalcAssetDayByDay(ptbp, pbre, pstrBacktestDate);
    }

    jf_jiukun_freeMemory((void **)&ptps);
    
    return u32Ret;
}

static u32 _startBacktestingModelDayByDay(
    tx_backtesting_param_t * ptbp, backtesting_result_ext_t * pbre)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t lyear, lmonth, lday;
    olint_t ldays, enddays;
    olchar_t strBacktestDate[16];
    da_day_summary_t * buffer = NULL;
    olint_t total = 400; //MAX_NUM_OF_DDS_FOR_BACKTESTING;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];

    ol_sprintf(
        strFullname, "%s%c%s", BACKTESTING_OUTPUT_DIR, PATH_SEPARATOR, BACKTESTING_OUTPUT_FILE);
    
    u32Ret = jf_filestream_open(strFullname, "w", &pbre->bre_pjfAsset);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;
    
    jf_logger_logInfoMsg("backtesting model day by day");
    clearDataInTradePersistency();

    jf_jiukun_allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total);

    jf_date_getDate2FromString(ls_pstrBacktestingStartDateDayByDay, &lyear, &lmonth, &lday);
    ldays = jf_date_convertDateToDaysFrom1970(lyear, lmonth, lday);
    ol_strcpy(pbre->bre_ptbrResult->tbr_strStartDate, ls_pstrBacktestingStartDateDayByDay);

    enddays = jf_date_convertTodayToDaysFrom1970();
    jf_date_getStringDate2ForDaysFrom1970(pbre->bre_ptbrResult->tbr_strEndDate, enddays);

    pbre->bre_dbAsset = pbre->bre_ptbrResult->tbr_dbInitialFund;
    pbre->bre_nStock = 0;
    _saveAsset(ptbp, pbre, ls_pstrBacktestingStartDateDayByDay);
    
    while (ldays <= enddays)
    {
        /*determine the start date for reading day summary*/
        if (isHoliday(ldays))
        {
            ldays ++;
            continue;
        }

        jf_date_getStringDate2ForDaysFrom1970(strBacktestDate, ldays);
        jf_logger_logDebugMsg("backtesting model day by day, %s", strBacktestDate);

        u32Ret = _backtestingFindStockDayByDay(ptbp, strBacktestDate, buffer, total);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _backtestingTradeStockDayByDay(ptbp, pbre, strBacktestDate, buffer, total);
        }

        ldays ++;
    }

    jf_jiukun_freeMemory((void **)&buffer);

    /*in case the stock is bought but not sold out, we should sell it out forcely*/
    _wrapupPoolStock(pbre->bre_ptbrResult);

    jf_filestream_close(&pbre->bre_pjfAsset);
    
    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 backtestingModel(tx_backtesting_param_t * ptbp, tx_backtesting_result_t * ptbr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_model_t * model = NULL;
    backtesting_result_ext_t bre;

    if (! ptbp->tbp_bAllModel)
    {
        assert(ptbp->tbp_pstrModel != NULL);

        u32Ret = tx_model_getModel(ptbp->tbp_pstrModel, &model);
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
        u32Ret = setStockFirstTradeDate(ptbp->tbp_pstrStockPath);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(ptbr, sizeof(tx_backtesting_result_t));
        ptbr->tbr_dbInitialFund = ptbp->tbp_dbInitialFund;
        ptbr->tbr_dbFund = ptbp->tbp_dbInitialFund;
        ptbr->tbr_dbMinAsset = ptbr->tbr_dbMaxAsset = ptbr->tbr_dbFund;

        ol_bzero(&bre, sizeof(backtesting_result_ext_t));
        bre.bre_ptbrResult = ptbr;

        if (ptbp->tbp_u8Method == TX_BACKTESTING_METHOD_DAY_BY_DAY)
            u32Ret = _startBacktestingModelDayByDay(ptbp, &bre);
        else
            u32Ret = _startBacktestingModel(ptbp, &bre);

        _summaryBacktestingResult(ptbr);
    }

    return u32Ret;
}


/*------------------------------------------------------------------------------------------------*/


