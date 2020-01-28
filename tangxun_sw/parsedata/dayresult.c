/**
 *  @file dayresult.c
 *
 *  @brief Routine for parsing day result from files.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <math.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_file.h"
#include "jf_filestream.h"
#include "jf_dir.h"
#include "jf_string.h"
#include "jf_clieng.h"
#include "jf_mem.h"
#include "jf_time.h"
#include "jf_jiukun.h"

#include "tx_dayresult.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define TRADE_OP_BUY   0
#define TRADE_OP_SOLD  1
#define TRADE_OP_NA    2

typedef struct
{
    olchar_t * ti_pstrTime;
    oldouble_t ti_dbPrice;
    olint_t ti_nVolume;
    olint_t ti_nAmount;
    olint_t ti_nOp;
} trade_item_t;

/* --- private routine section ------------------------------------------------------------------ */

static void _convertResult(tx_dr_t * result)
{
    result->td_u64AllA /= TX_DR_AMOUNT_UNIT;
    result->td_u64BuyA /= TX_DR_AMOUNT_UNIT;
    result->td_u64SoldA /= TX_DR_AMOUNT_UNIT;
    result->td_u64NaA /= TX_DR_AMOUNT_UNIT;
    result->td_u64LambBuyA /= TX_DR_AMOUNT_UNIT;
    result->td_u64LambSoldA /= TX_DR_AMOUNT_UNIT;
    result->td_u64LambNaA /= TX_DR_AMOUNT_UNIT;
    result->td_u64CloseHighLimitSoldA /= TX_DR_AMOUNT_UNIT;
    result->td_u64CloseHighLimitLambSoldA /= TX_DR_AMOUNT_UNIT;
    result->td_u64CloseLowLimitBuyA /= TX_DR_AMOUNT_UNIT;
    result->td_u64CloseLowLimitLambBuyA /= TX_DR_AMOUNT_UNIT;
}

static void _calcResult(tx_dr_parse_param_t * ptdpp, tx_dr_t * result)
{
    result->td_dbBuyPercent = result->td_u64Buy * 100;
    if (result->td_u64All != 0)
        result->td_dbBuyPercent /= result->td_u64All;

    result->td_dbSoldPercent = result->td_u64Sold * 100;
    if (result->td_u64All != 0)
        result->td_dbSoldPercent /= result->td_u64All;

    result->td_dbLambBuyPercent = result->td_u64LambBuy * 100;
    if (result->td_u64All != 0)
        result->td_dbLambBuyPercent /= result->td_u64All;

    result->td_dbLambSoldPercent = result->td_u64LambSold * 100;
    if (result->td_u64All != 0)
        result->td_dbLambSoldPercent /= result->td_u64All;

    result->td_dbBuyInAmPercent = result->td_u64BuyInAm * 100;
    if (result->td_u64Buy != 0)
        result->td_dbBuyInAmPercent /= result->td_u64Buy;

    result->td_dbSoldInAmPercent = result->td_u64SoldInAm * 100;
    if (result->td_u64Sold != 0)
        result->td_dbSoldInAmPercent /= result->td_u64Sold;

    result->td_dbAboveLastClosingPricePercent = result->td_u64AboveLastClosingPrice * 100;
    if (result->td_u64All != 0)
        result->td_dbAboveLastClosingPricePercent /= result->td_u64All;

    result->td_dbLambBuyBelow100Percent = result->td_u64LambBuyBelow100 * 100;
    if (result->td_u64LambBuy != 0)
        result->td_dbLambBuyBelow100Percent /= result->td_u64LambBuy;

    result->td_dbLambSoldBelow100Percent = result->td_u64LambSoldBelow100 * 100;
    if (result->td_u64LambSold != 0)
        result->td_dbLambSoldBelow100Percent /= result->td_u64LambSold;

    result->td_dbCloseHighLimitSoldPercent = result->td_u64CloseHighLimitSold * 100;
    if (result->td_u64All != 0)
        result->td_dbCloseHighLimitSoldPercent /= result->td_u64All;

    result->td_dbCloseHighLimitLambSoldPercent = result->td_u64CloseHighLimitLambSold * 100;
    if (result->td_u64All != 0)
        result->td_dbCloseHighLimitLambSoldPercent /= result->td_u64All;

    result->td_dbCloseLowLimitBuyPercent = result->td_u64CloseLowLimitBuy * 100;
    if (result->td_u64All != 0)
        result->td_dbCloseLowLimitBuyPercent /= result->td_u64All;

    result->td_dbCloseLowLimitLambBuyPercent = result->td_u64CloseLowLimitLambBuy * 100;
    if (result->td_u64All != 0)
        result->td_dbCloseLowLimitLambBuyPercent /= result->td_u64All;

    if ((result->td_dbLastXLowPrice < result->td_dbClosingPrice) &&
        (result->td_dbLastXLowPrice != 0))
    {
        result->td_dbLastXInc =
            (result->td_dbClosingPrice - result->td_dbLastXLowPrice) * 100 /
            result->td_dbLastXLowPrice;
    }

    if (result->td_dbOpeningPrice > result->td_dbClosingPrice)
    {
        result->td_dbUpperShadowRatio =
            (result->td_dbHighPrice - result->td_dbOpeningPrice) /
            (result->td_dbOpeningPrice - result->td_dbClosingPrice);
        result->td_dbLowerShadowRatio = 
            (result->td_dbClosingPrice - result->td_dbLowPrice) /
            (result->td_dbOpeningPrice - result->td_dbClosingPrice);
    }
    else if (result->td_dbOpeningPrice < result->td_dbClosingPrice)
    {
        result->td_dbUpperShadowRatio =
            (result->td_dbHighPrice - result->td_dbClosingPrice) /
            (result->td_dbClosingPrice - result->td_dbOpeningPrice);
        result->td_dbLowerShadowRatio =
            (result->td_dbOpeningPrice - result->td_dbLowPrice) /
            (result->td_dbClosingPrice - result->td_dbOpeningPrice);
    }
    else
    {
        if (result->td_dbClosingPrice != result->td_dbLowPrice)
            result->td_dbUpperShadowRatio =
                (result->td_dbHighPrice - result->td_dbClosingPrice) /
                (result->td_dbClosingPrice - result->td_dbLowPrice);
        else
            result->td_dbUpperShadowRatio =
                (result->td_dbHighPrice - result->td_dbClosingPrice) * 100; /* = / 0.01*/

        if (result->td_dbHighPrice != result->td_dbClosingPrice)
            result->td_dbLowerShadowRatio =
                (result->td_dbClosingPrice - result->td_dbLowPrice) /
                (result->td_dbHighPrice - result->td_dbClosingPrice);
        else
            result->td_dbLowerShadowRatio =
                (result->td_dbClosingPrice - result->td_dbLowPrice) * 100;
    }

    _convertResult(result);
}

#if defined(WINDOWS)
static inline oldouble_t round(oldouble_t x)
{
    return floor(x + 0.5);
}
#endif

/*
static olint_t _strTimeToSec(olchar_t * pstr)
{
    olint_t hour, minute, second, tosec;

    sscanf(pstr, "%d:%d:%d", &hour, &minute, &second);
    tosec = (hour - 9) * 3600 + minute * 60 + second; 
    if (hour >= 13)
        tosec -= 90 * 60;

    return tosec;
}
*/

static u32 _fillDayResult(
    tx_dr_parse_param_t * ptdpp, tx_dr_t * daresult,
    trade_item_t * item, tx_dr_t * prev, olchar_t * time,
    boolean_t bFirstLine)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bCloseHighLimit = FALSE, bCloseLowLimit= FALSE;
    oldouble_t dbclose;

    if (strncmp(item->ti_pstrTime, time, 8) < 0)
        return JF_ERR_INVALID_DATA;
    else
        ol_strcpy(time, item->ti_pstrTime);

    if (prev != NULL)
    {
        /*high limit*/
        dbclose = round(prev->td_dbClosingPrice * 110) / 100;
        if (item->ti_dbPrice >= dbclose)
            bCloseHighLimit = TRUE; 

        /*low limit*/
        dbclose = round(prev->td_dbClosingPrice * 90) / 100;
        if (item->ti_dbPrice <= dbclose)
            bCloseLowLimit = TRUE; 
    }

    /*closing price is of the last trade*/
    daresult->td_dbClosingPrice = item->ti_dbPrice;

    /*opening price is of the first trade*/
    if (bFirstLine)
        daresult->td_dbOpeningPrice = item->ti_dbPrice;

    if (daresult->td_dbHighPrice < item->ti_dbPrice)
        daresult->td_dbHighPrice = item->ti_dbPrice;

    if (daresult->td_dbLowPrice > item->ti_dbPrice)
        daresult->td_dbLowPrice = item->ti_dbPrice;

    if (strncmp(item->ti_pstrTime, ptdpp->tdpp_strLastX,
                ol_strlen(ptdpp->tdpp_strLastX)) >= 0)
    {
        if (daresult->td_dbLastXLowPrice > item->ti_dbPrice)
        {
            daresult->td_dbLastXLowPrice = item->ti_dbPrice;
            ol_strcpy(daresult->td_strLastTimeLowPrice, item->ti_pstrTime);
        }
    }

    daresult->td_u64All += item->ti_nVolume;
    daresult->td_u64AllA += item->ti_nAmount;

    if (bCloseHighLimit && ! daresult->td_bCloseHighLimit)
    {
        daresult->td_bCloseHighLimit = TRUE;
        ol_strcpy(daresult->td_strTimeCloseHighLimit, item->ti_pstrTime);
    }

    if (bCloseLowLimit && ! daresult->td_bCloseLowLimit)
    {
        daresult->td_bCloseLowLimit = TRUE;
        ol_strcpy(daresult->td_strTimeCloseLowLimit, item->ti_pstrTime);
    }

    if (item->ti_nOp == TRADE_OP_BUY)
    {
        if (item->ti_nVolume >= ptdpp->tdpp_nThres)
        {
            if (bCloseLowLimit)
            {
                daresult->td_u64CloseLowLimitBuy += item->ti_nVolume;
                daresult->td_u64CloseLowLimitBuyA += item->ti_nAmount;
            }
            else
            {
                daresult->td_u64Buy += item->ti_nVolume;
                daresult->td_u64BuyA += item->ti_nAmount;
                if (strncmp(item->ti_pstrTime, "12", 2) < 0)
                    daresult->td_u64BuyInAm += item->ti_nVolume;
            }
        }
        else
        {
            if (bCloseLowLimit)
            {
                daresult->td_u64CloseLowLimitLambBuy += item->ti_nVolume;
                daresult->td_u64CloseLowLimitLambBuyA += item->ti_nAmount;
            }
            else
            {
                daresult->td_u64LambBuy += item->ti_nVolume;
                daresult->td_u64LambBuyA += item->ti_nAmount;
                if (item->ti_nVolume < 100)
                    daresult->td_u64LambBuyBelow100 += item->ti_nVolume;
            }
        }
    }
    else if (item->ti_nOp == TRADE_OP_SOLD)
    {
        if (item->ti_nVolume >= ptdpp->tdpp_nThres)
        {
            if (bCloseHighLimit)
            {
                daresult->td_u64CloseHighLimitSold += item->ti_nVolume;
                daresult->td_u64CloseHighLimitSoldA += item->ti_nAmount;
            }
            else
            {
                daresult->td_u64Sold += item->ti_nVolume;
                daresult->td_u64SoldA += item->ti_nAmount;
                if (strncmp(item->ti_pstrTime, "12", 2) < 0)
                    daresult->td_u64SoldInAm += item->ti_nVolume;
            }
        }
        else
        {
            if (bCloseHighLimit)
            {
                daresult->td_u64CloseHighLimitLambSold += item->ti_nVolume;
                daresult->td_u64CloseHighLimitLambSoldA += item->ti_nAmount;
            }
            else
            {
                daresult->td_u64LambSold += item->ti_nVolume;
                daresult->td_u64LambSoldA += item->ti_nAmount;
                if (item->ti_nVolume < 100)
                    daresult->td_u64LambSoldBelow100 += item->ti_nVolume;
            }
        }
    }
    else if (item->ti_nOp == TRADE_OP_NA)
    {
        if (item->ti_nVolume >= ptdpp->tdpp_nThres)
        {
            daresult->td_u64Na += item->ti_nVolume;
            daresult->td_u64NaA += item->ti_nAmount;
        }
        else
        {
            daresult->td_u64LambNa += item->ti_nVolume;
            daresult->td_u64LambNaA += item->ti_nAmount;
        }
    }

    if (prev != NULL)
    {
        if (item->ti_dbPrice >= prev->td_dbClosingPrice)
            daresult->td_u64AboveLastClosingPrice += item->ti_nVolume;
    }

    return u32Ret;
}

static u32 _parseOneLine(
    tx_dr_parse_param_t * ptdpp, tx_dr_t * daresult,
    olchar_t * line, olsize_t llen, tx_dr_t * prev, olchar_t * time,
    boolean_t bFirstLine)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * result;
    jf_string_parse_result_field_t * field;
    olint_t index;
    trade_item_t item;
    oldouble_t dbvalue;

    memset(&item, 0, sizeof(item));

    u32Ret = jf_string_parse(&result, line, 0, llen, "\t", 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = result->jspr_pjsprfFirst;
        index = 1;
        while ((field != NULL) && (u32Ret == JF_ERR_NO_ERROR))
        {
            if (index == 1)
            {
                /*time of the trade*/
                if (field->jsprf_sData < 8)
                    u32Ret = JF_ERR_INVALID_DATA;
                else
                {
                    field->jsprf_pstrData[field->jsprf_sData] = '\0';
                    item.ti_pstrTime = field->jsprf_pstrData;
                }
            }
            else if (index == 2)
            {
                /*jsprice of the trade*/
                field->jsprf_pstrData[field->jsprf_sData] = '\0';
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &item.ti_dbPrice);
            }
            else if (index == 4)
            {
                /*volume*/
                if (field->jsprf_pstrData[0] == '-')
                {
                    u32Ret = jf_string_getS32FromString(
                        field->jsprf_pstrData + 1, field->jsprf_sData - 1, &item.ti_nVolume);
                    if (u32Ret == JF_ERR_NO_ERROR)
                        item.ti_nVolume = -item.ti_nVolume;
                }
                else
                {
                    u32Ret = jf_string_getS32FromString(
                        field->jsprf_pstrData, field->jsprf_sData, &item.ti_nVolume);
                }
            }
            else if (index == 5)
            {
                /*amount*/
                if (field->jsprf_pstrData[0] == '-')
                {
                    u32Ret = jf_string_getDoubleFromString(
                        field->jsprf_pstrData + 1, field->jsprf_sData - 1, &dbvalue);
                    if (u32Ret == JF_ERR_NO_ERROR)
                        item.ti_nAmount = -(olint_t)dbvalue;
                }
                else
                {
                    u32Ret = jf_string_getDoubleFromString(
                        field->jsprf_pstrData, field->jsprf_sData, &dbvalue);
                    if (u32Ret == JF_ERR_NO_ERROR)
                        item.ti_nAmount = (olint_t)dbvalue;
                }
            }
            else if (index == 6)
            {
                if ((((u8)field->jsprf_pstrData[0] == 0xC2) &&
                     ((u8)field->jsprf_pstrData[1] == 0xF2) &&
                     ((u8)field->jsprf_pstrData[2] == 0xC5) &&
                     ((u8)field->jsprf_pstrData[3] == 0xCC)) ||
                    (ol_strncmp(field->jsprf_pstrData, "buy", 3) == 0))
                    item.ti_nOp = TRADE_OP_BUY;
                else if ((((u8)field->jsprf_pstrData[0] == 0xC2) &&
                          ((u8)field->jsprf_pstrData[1] == 0xF4) &&
                          ((u8)field->jsprf_pstrData[2] == 0xC5) &&
                          ((u8)field->jsprf_pstrData[3] == 0xCC)) ||
                         (ol_strncmp(field->jsprf_pstrData, "sold", 4) == 0))
                    item.ti_nOp = TRADE_OP_SOLD;
                else if  ((((u8)field->jsprf_pstrData[0] == 0xD6) &&
                           ((u8)field->jsprf_pstrData[1] == 0xD0) &&
                           ((u8)field->jsprf_pstrData[2] == 0xD0) &&
                           ((u8)field->jsprf_pstrData[3] == 0xD4) &&
                           ((u8)field->jsprf_pstrData[4] == 0xC5) &&
                           ((u8)field->jsprf_pstrData[5] == 0xCC)) ||
                          (ol_strncmp(field->jsprf_pstrData, "na", 2) == 0))
                    item.ti_nOp = TRADE_OP_NA;
                else
                    u32Ret = JF_ERR_INVALID_DATA;
            }

            field = field->jsprf_pjsprfNext;
            index ++;
        }

        jf_string_destroyParseResult(&result);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (item.ti_nVolume < 0)
        {
            if (item.ti_nOp == TRADE_OP_SOLD)
                item.ti_nOp = TRADE_OP_BUY;
            else if (item.ti_nOp == TRADE_OP_BUY)
                item.ti_nOp = TRADE_OP_SOLD;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _fillDayResult(
            ptdpp, daresult, &item, prev, time, bFirstLine);

    return u32Ret;
}

static void _calcData(tx_dr_parse_param_t * ptdpp, tx_dr_t * buffer, olint_t num)
{
    tx_dr_t * result, * prev, * end, * start;
    oldouble_t dbt, dbts, dbtls;

    result = prev = buffer;
    end = buffer + num;
    result ++;
    while (result < end)
    {
        if (result->td_dbHighPrice > prev->td_dbClosingPrice)
            result->td_dbHighPriceInc =
                (result->td_dbHighPrice - prev->td_dbClosingPrice) * 100 /
                prev->td_dbClosingPrice;

        if (result->td_dbLowPrice < prev->td_dbClosingPrice)
            result->td_dbLowPriceDec =
                (prev->td_dbClosingPrice - result->td_dbLowPrice) * 100 /
                prev->td_dbClosingPrice;

        if (result->td_dbClosingPrice > prev->td_dbClosingPrice)
        {
            result->td_dbClosingPriceInc =
                (result->td_dbClosingPrice - prev->td_dbClosingPrice) * 100 /
                prev->td_dbClosingPrice;
            if (result->td_dbLowPrice > prev->td_dbHighPrice)
                result->td_bGap = TRUE;
        }
        else
        {            
            result->td_dbClosingPriceDec =
                (prev->td_dbClosingPrice - result->td_dbClosingPrice) * 100 /
                prev->td_dbClosingPrice;
            if (result->td_dbHighPrice < prev->td_dbLowPrice)
                result->td_bGap = TRUE;
        }

        if (result - buffer >= 5)
        {
            dbts = dbtls = dbt = 0.0;
            start = result - 5;
            while (start < result)
            {
                dbt += start->td_u64All;
                dbts += start->td_u64CloseHighLimitSold + start->td_u64Sold;
                dbtls += start->td_u64CloseHighLimitLambSold + start->td_u64LambSold;
                start ++;
            }
            result->td_dbVolumeRatio = result->td_u64All * 5 / dbt;
            result->td_dbSoldVolumeRatio =
                (result->td_u64CloseHighLimitSold + result->td_u64Sold) * 5 / dbts;
            result->td_dbLambSoldVolumeRatio =
                (result->td_u64CloseHighLimitLambSold + result->td_u64LambSold) * 5 / dbtls;
        }

        prev ++;
        result ++;
    }
}

static u32 _readDataFileLine(
    olchar_t * data, olchar_t ** prevline, olsize_t * llen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * line;
    olsize_t len = 0;

    if (*prevline <= data)
        return JF_ERR_END_OF_FILE;

    line = *prevline - 2;

    while (data < line)
    {
        if (*line == '\n')
        {
            line ++;
            break;
        }

        line --;
        len ++;
    }

    *prevline = line;
    *llen = len;

    if (((u8)line[0] == 0xB3) && ((u8)line[1] == 0xC9) &&
        ((u8)line[2] == 0xBD) && ((u8)line[3] == 0xBB))
        u32Ret = JF_ERR_END_OF_FILE;

    return u32Ret;
}

static u32 _checkDataFile(olchar_t *data, olsize_t sdata)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (((u8)data[0] != 0xB3) || ((u8)data[1] != 0xC9) ||
        ((u8)data[2] != 0xBD) || ((u8)data[3] != 0xBB))
        u32Ret = JF_ERR_INVALID_FILE;

    return u32Ret;
}

static u32 _parseDataFileLine(
    olchar_t * file, olchar_t *data, olsize_t sdata, tx_dr_parse_param_t * ptdpp,
    tx_dr_t * daresult, tx_dr_t * prev)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * line;
    olsize_t llen;
    olchar_t strTime[16]; /*the time when the last transaction occurs*/

    ol_strcpy(strTime, "09:15:00");

    line = data + sdata;

    u32Ret = _checkDataFile(data, sdata);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _readDataFileLine(data, &line, &llen);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseOneLine(
            ptdpp, daresult, line, llen, prev, strTime, TRUE);

    while (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _readDataFileLine(data, &line, &llen);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _parseOneLine(
                ptdpp, daresult, line, llen, prev, strTime, FALSE);
        }
    } 

    if (u32Ret == JF_ERR_END_OF_FILE)
        u32Ret = JF_ERR_NO_ERROR;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (strncmp(strTime, "11", 2) == 0)
        {
            if (ol_strncmp(strTime, "11:20", 5) < 0)
                u32Ret = JF_ERR_INVALID_DATA;
        }
        else
        {
            if (ol_strncmp(strTime, "13:43", 5) < 0)
                u32Ret = JF_ERR_INVALID_DATA;
        }
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_clieng_outputLine("Error analysising file %s, time %s", file, strTime);
    }

    /*count*/
    _calcResult(ptdpp, daresult);

    return u32Ret;
}

static u32 _parseDataFile(
    olchar_t * file, tx_dr_parse_param_t * ptdpp,
    olint_t daindex, tx_dr_t * daresult, tx_dr_t * prev)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_t fd;
    olchar_t * data = NULL;
#define MAX_DATAFILE_SIZE  (512 * 1024)
    olsize_t sread;
    olchar_t filename[JF_LIMIT_MAX_PATH_LEN];

    ol_memset(daresult, 0, sizeof(*daresult));
    daresult->td_nIndex = daindex;
#define MAX_PRICE 9999.99
    daresult->td_dbLowPrice = MAX_PRICE;
    daresult->td_dbLastXLowPrice = MAX_PRICE;

    jf_file_getFileName(filename, JF_LIMIT_MAX_PATH_LEN, file);
    ol_strncpy(daresult->td_strDate, filename, 10);

    u32Ret = jf_file_open(file, O_RDONLY, &fd);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = jf_jiukun_allocMemory((void **)&data, MAX_DATAFILE_SIZE);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sread = MAX_DATAFILE_SIZE;
        u32Ret = jf_file_readn(fd, data, &sread);
        if ((u32Ret == JF_ERR_NO_ERROR) && (sread == MAX_DATAFILE_SIZE))
            u32Ret = JF_ERR_BUFFER_TOO_SMALL;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseDataFileLine(
            file, data, sread, ptdpp, daresult, prev);

    if (data != NULL)
        jf_jiukun_freeMemory((void **)&data);

    jf_file_close(&fd);

    return u32Ret;
}

static boolean_t _filterDirEntry(jf_dir_entry_t * entry)
{
    boolean_t bRet = TRUE;

    if (ol_strncmp(entry->jde_strName + 10, ".xls", 4) == 0)
        bRet = FALSE;

    return bRet;
}

/* --- public routine section ------------------------------------------------------------------- */
u32 tx_dr_parseDataFile(
    olchar_t * file, tx_dr_parse_param_t * ptdpp, olint_t daindex, tx_dr_t * daresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _parseDataFile(file, ptdpp, daindex, daresult, NULL);

    return u32Ret;
}

u32 tx_dr_readDrDir(
    olchar_t * dirpath, tx_dr_parse_param_t * ptdpp, tx_dr_t * buffer,
    olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_dir_entry_t * entrylist = NULL, *entry;
    olint_t i, count, total = *numofresult;
    olchar_t file[JF_LIMIT_MAX_PATH_LEN * 2];
    tx_dr_t * result, * prev = NULL;

    u32Ret = jf_jiukun_allocMemory(
        (void **)&entrylist, sizeof(jf_dir_entry_t) * total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(entrylist, sizeof(jf_dir_entry_t) * total);

        u32Ret = jf_dir_scan(
            dirpath, entrylist, &total, _filterDirEntry, jf_dir_compareDirEntry);
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        if (entrylist != NULL)
            jf_jiukun_freeMemory((void **)&entrylist);
        return u32Ret;
    }

    if (ptdpp->tdpp_nLastCount != 0)
    {
        ptdpp->tdpp_nEnd = total;
        if (total > *numofresult)
            total = *numofresult;
        if (total > ptdpp->tdpp_nLastCount)
            total = ptdpp->tdpp_nLastCount;
        ptdpp->tdpp_nStart = ptdpp->tdpp_nEnd - total + 1;
    }
    else
    {
        if (total > *numofresult)
            total = *numofresult;

        total = total - ptdpp->tdpp_nStart + 1;
        if (total > ptdpp->tdpp_nEnd - ptdpp->tdpp_nStart + 1)
            total = ptdpp->tdpp_nEnd - ptdpp->tdpp_nStart + 1;
    }

    entry = entrylist + ptdpp->tdpp_nStart - 1;
    result = buffer;
    count = 0;
    for(i = ptdpp->tdpp_nStart;
        (i < ptdpp->tdpp_nStart + total) && (u32Ret == JF_ERR_NO_ERROR); i++)
    {
        ol_snprintf(file, sizeof(file), "%s%c%s",
                 dirpath, PATH_SEPARATOR, entry->jde_strName);

        if ((ptdpp->tdpp_pstrDateFrom == NULL) ||
            ol_strncmp(entry->jde_strName, ptdpp->tdpp_pstrDateFrom,
                    ol_strlen(ptdpp->tdpp_pstrDateFrom)) >= 0)
        {
            u32Ret = _parseDataFile(file, ptdpp, i, result, prev);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                count ++;
                prev = result;
                result ++;
            }
        }

        entry ++;
    }

    jf_jiukun_freeMemory((void **)&entrylist);

    if (u32Ret == JF_ERR_NO_ERROR)
        _calcData(ptdpp, buffer, total);

    *numofresult = count;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


