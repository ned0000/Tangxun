/**
 *  @file fromstock.c
 *
 *  @brief Backtesting model from stock.
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

/** Maximum number of day summary for backtesting from stock.
 */
#define BT_FROM_STOCK_MAX_NUM_OF_DS   (800)

typedef struct
{
    tx_backtesting_result_t * bsre_ptbrResult;
    jf_filestream_t * bsre_pjfAsset;
    /**Fund + Stock.*/
    double bsre_dbAsset;
    olint_t bsre_nStock;
    boolean_t bsre_bSaveAsset;
} bt_stock_result_ext_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _findAndTradeBtEvalStockInModel(
    tx_model_t * model, tx_backtesting_eval_param_t * ptbep, bt_stock_result_ext_t * pbsre,
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * end = NULL;
    tx_trade_pool_stock_t ttps;
    tx_trade_trading_record_t tttr;
    tx_model_trade_data_t tmtd;
    tx_backtesting_result_t * ptbr = pbsre->bsre_ptbrResult;

    end = buffer + num - 1;
    JF_LOGGER_INFO("total: %d, last day: %s", num, end->td_strDate);

    tx_trade_initPoolStock(&ttps, stockinfo->tsi_strCode, model->tm_strName);
    tx_trade_initTradingRecord(&tttr, stockinfo->tsi_strCode, model->tm_strName);

    u32Ret = tx_persistency_getPoolStock(&ttps);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*The stock is in pool.*/
        JF_LOGGER_INFO("%s is already in pool", stockinfo->tsi_strCode);

        ol_bzero(&tmtd, sizeof(tmtd));
        tmtd.tmtd_dbFund = ptbr->tbr_dbFund;

        if (tx_trade_isPoolStockOpNone(&ttps))
        {
            JF_LOGGER_INFO("before buy, fund: %.2f", ptbr->tbr_dbFund);
            /*Not bought yet.*/
            u32Ret = model->tm_fnTrade(model, stockinfo, &ttps, &tmtd, buffer, num);
            if ((u32Ret == JF_ERR_NO_ERROR) && tx_trade_isPoolStockOpBuy(&ttps))
            {
                /*The stock is bought.*/
                /*Update pool stock.*/
                tx_persistency_updatePoolStock(&ttps);
                /*Add trading record.*/
                tx_trade_setTradingRecord(&tttr, &ttps);
                tx_persistency_insertTradingRecord(&tttr);

                pbsre->bsre_nStock ++;
                pbsre->bsre_bSaveAsset = TRUE;
            }
            ptbr->tbr_dbFund = tmtd.tmtd_dbFund;
            JF_LOGGER_INFO("after buy, fund: %.2f", ptbr->tbr_dbFund);
        }
        else if (tx_trade_isPoolStockOpBuy(&ttps))
        {
            JF_LOGGER_INFO("before sell, fund: %.2f", ptbr->tbr_dbFund);
            /*Bought already, try to sell.*/
            u32Ret = model->tm_fnTrade(model, stockinfo, &ttps, &tmtd, buffer, num);
            if ((u32Ret == JF_ERR_NO_ERROR) && tx_trade_isPoolStockOpSell(&ttps))
            {
                /*The stock is sold out.*/
                /*Add trading record.*/
                tx_trade_setTradingRecord(&tttr, &ttps);
                tx_persistency_insertTradingRecord(&tttr);
                /*Delete pool stock.*/
                tx_persistency_removePoolStock(&ttps);
                /*Calculate total fund.*/
                ptbr->tbr_dbFund += tmtd.tmtd_dbFund;

                pbsre->bsre_nStock --;
                pbsre->bsre_dbAsset = tmtd.tmtd_dbFund;
                pbsre->bsre_bSaveAsset = TRUE;
                JF_LOGGER_INFO("after sell, fund: %.2f", ptbr->tbr_dbFund);
            }
            else
            {
                /*Stock is not sold out and still in pool.*/

            }
        }
    }
    else
    {
        /*Stock is not in pool.*/
        u32Ret = model->tm_fnCanBeTraded(model, stockinfo, &ttps, buffer, num);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            JF_LOGGER_DEBUG("insert stock %s to pool", stockinfo->tsi_strCode);
            u32Ret = tx_persistency_insertPoolStock(&ttps);
        }
    }

    return u32Ret;
}

static u32 _saveBtStockAsset(
    tx_backtesting_eval_param_t * ptbep, bt_stock_result_ext_t * pbsre, olchar_t * pstrDate)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strLine[128];
    olsize_t sLine;

    assert(pbsre->bsre_pjfAsset != NULL);

    JF_LOGGER_DEBUG("save asset");
    
    sLine = ol_sprintf(
        strLine, "%s\t%.2f\t%d\n", pstrDate, pbsre->bsre_dbAsset, pbsre->bsre_nStock);
    u32Ret = jf_filestream_writen(pbsre->bsre_pjfAsset, strLine, sLine);

    return u32Ret;
}

static u32 _trySaveBtStockAsset(
    tx_backtesting_eval_param_t * ptbep, bt_stock_result_ext_t * pbsre, olchar_t * pstrDate)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_backtesting_result_t * ptbr = pbsre->bsre_ptbrResult;

    if (pbsre->bsre_bSaveAsset)
    {
        pbsre->bsre_bSaveAsset = FALSE;

        if (ptbr->tbr_dbMinAsset > pbsre->bsre_dbAsset)
            ptbr->tbr_dbMinAsset = pbsre->bsre_dbAsset;
        if (ptbr->tbr_dbMaxAsset < pbsre->bsre_dbAsset)
            ptbr->tbr_dbMaxAsset = pbsre->bsre_dbAsset;

        u32Ret = _saveBtStockAsset(ptbep, pbsre, pstrDate);
    }

    return u32Ret;
}

static u32 _findAndTradeBtEvalStock(
    tx_model_t * model, tx_backtesting_eval_param_t * ptbep, bt_stock_result_ext_t * pbsre,
    tx_stock_info_t * stockinfo, olchar_t * pstrStartDate, tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t lyear = 0, lmonth = 0, lday = 0;
    olint_t ldays = 0, enddays = 0;
    olchar_t strDate[16] = {'\0'};
    olint_t total = num;
    tx_ds_t * end = buffer + num - 1;

    JF_LOGGER_INFO("start date: %s", pstrStartDate);

    ol_strcpy(strDate, end->td_strDate);

    /*The end date is from the last day summary.*/
    jf_date_getDate2FromString(end->td_strDate, &lyear, &lmonth, &lday);
    enddays = jf_date_convertDateToDaysFrom1970(lyear, lmonth, lday);

    if (ol_strlen(pstrStartDate) == 0)
        /*The start date is empty, use the date from the first day summary.*/
        jf_date_getDate2FromString(buffer->td_strDate, &lyear, &lmonth, &lday);
    else
        /*Use the start date if it's not empty.*/
        jf_date_getDate2FromString(pstrStartDate, &lyear, &lmonth, &lday);

    ldays = jf_date_convertDateToDaysFrom1970(lyear, lmonth, lday);

    /*Loop from start day to the end date.*/
    while (ldays <= enddays)
    {
        /*Ignore the holiday.*/
        if (tx_trade_isHoliday(ldays))
        {
            ldays ++;
            continue;
        }

        /*Convert the current day to string.*/
        jf_date_getStringDate2ForDaysFrom1970(strDate, ldays);

        /*Count number of day summary until the current day.*/
        total = tx_ds_countDsWithEndData(buffer, num, strDate);
        if (total == 0)
        {
            ldays ++;
            continue;
        }

        /*Find stock and trade.*/
        _findAndTradeBtEvalStockInModel(model, ptbep, pbsre, stockinfo, buffer, total);

        /*Save the asset to file.*/
        _trySaveBtStockAsset(ptbep, pbsre, strDate);

        ldays ++;
    }

    tx_trade_getNextTradingDate(strDate, pstrStartDate);

    return u32Ret;
}

static u32 _summaryBtEvalStockResult(tx_backtesting_result_t * ptbr)
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

static u32 _startBtEvalModelFromOneStock(
    tx_model_t * model, tx_backtesting_eval_param_t * ptbep, bt_stock_result_ext_t * pbsre,
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = 0;
    tx_ds_t * middle = NULL;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    olchar_t strStartDate[16] = {'\0'};
    olchar_t strDate[16] = {'\0'};
    olchar_t * pstrStartDate = NULL;
    boolean_t bLast = FALSE;

    JF_LOGGER_INFO("stock: %s", stockinfo->tsi_strCode);

    /*Set the name of the day summary file.*/
    strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';
    ol_snprintf(
        strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
        ptbep->tbep_pstrStockPath, PATH_SEPARATOR, stockinfo->tsi_strCode);

    /*In case the buffer is not large enough to hold all day summary, read the file several times.*/
    do
    {
        total = num;

        /*Read day summary from file.*/
        u32Ret = tx_ds_readDsFromDateWithFRoR(strFullname, pstrStartDate, buffer, &total);
        if ((u32Ret == JF_ERR_NO_ERROR) && (total != BT_FROM_STOCK_MAX_NUM_OF_DS))
        {
            /*If the number of day summary read from file is not expected, it's the last part.*/
            bLast = TRUE;
        }

        /*Save the asset to the file at the first time.*/
        if ((u32Ret == JF_ERR_NO_ERROR) && (pstrStartDate == NULL))
        {
            _saveBtStockAsset(ptbep, pbsre, buffer->td_strDate);
            pbsre->bsre_bSaveAsset = FALSE;
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _findAndTradeBtEvalStock(
                model, ptbep, pbsre, stockinfo, strDate, buffer, total);
        }

        middle = buffer + total / 2;
        ol_strcpy(strStartDate, middle->td_strDate);
        JF_LOGGER_INFO("startdate: %s", strStartDate);
        pstrStartDate = strStartDate;

    } while (! bLast);

    /*In case the stock is bought but not sold out, we should sell it out forcely.*/
    wrapupBtPoolStock(pbsre->bsre_ptbrResult);

    return u32Ret;
}

static u32 _startBtEvalModelFromStock(
    tx_model_t * model, tx_backtesting_eval_param_t * ptbep, bt_stock_result_ext_t * pbsre)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = BT_FROM_STOCK_MAX_NUM_OF_DS;
    tx_ds_t * buffer = NULL;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    tx_stock_info_t * stockinfo;

    tx_persistency_clearData();
    strFullname[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';

    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);

    stockinfo = tx_stock_getFirstStockInfo();
    while ((stockinfo != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        JF_LOGGER_INFO("stock: %s", stockinfo->tsi_strCode);

        ol_snprintf(
            strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s.txt",
            BACKTESTING_OUTPUT_DIR, PATH_SEPARATOR, stockinfo->tsi_strCode);

        u32Ret = jf_filestream_open(strFullname, "w", &pbsre->bsre_pjfAsset);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _startBtEvalModelFromOneStock(model, ptbep, pbsre, stockinfo, buffer, total);

        if (pbsre->bsre_pjfAsset != NULL)
            jf_filestream_close(&pbsre->bsre_pjfAsset);

        if (u32Ret == JF_ERR_NO_ERROR)
            stockinfo = tx_stock_getNextStockInfo(stockinfo);
    }

    jf_jiukun_freeMemory((void **)&buffer);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 tx_backtesting_evalModelFromStock(
    const olchar_t * pstrModel, tx_backtesting_eval_param_t * ptbep, tx_backtesting_result_t * ptbr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_model_t * model = NULL;
    bt_stock_result_ext_t bsre;

    assert(pstrModel != NULL);

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

        ol_bzero(&bsre, sizeof(bt_stock_result_ext_t));
        bsre.bsre_ptbrResult = ptbr;
        bsre.bsre_dbAsset = ptbr->tbr_dbFund;

        u32Ret = _startBtEvalModelFromStock(model, ptbep, &bsre);

        _summaryBtEvalStockResult(ptbr);
    }

    return u32Ret;
}


/*------------------------------------------------------------------------------------------------*/


