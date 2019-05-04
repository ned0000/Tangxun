/**
 *  @file parsedata.c
 *
 *  @brief routine for parsing data, the data from files or ...
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

#include "parsedata.h"

/* --- private data/data structure section ------------------------------------------------------ */

static olchar_t * ls_pstrTradeSummaryFile = "Summary.csv";

typedef struct
{
    parse_sector_info_t * sector;
    olint_t maxsector;
    olint_t numofsector;
    olchar_t * data;
    olsize_t sdata;
} tmp_sector_file_t;

#define MAX_NUM_OF_RESULT  600

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
static void _convertResult(da_day_result_t * result)
{
    result->ddr_u64AllA /= AMOUNT_UNIT;
    result->ddr_u64BuyA /= AMOUNT_UNIT;
    result->ddr_u64SoldA /= AMOUNT_UNIT;
    result->ddr_u64NaA /= AMOUNT_UNIT;
    result->ddr_u64LambBuyA /= AMOUNT_UNIT;
    result->ddr_u64LambSoldA /= AMOUNT_UNIT;
    result->ddr_u64LambNaA /= AMOUNT_UNIT;
    result->ddr_u64CloseHighLimitSoldA /= AMOUNT_UNIT;
    result->ddr_u64CloseHighLimitLambSoldA /= AMOUNT_UNIT;
    result->ddr_u64CloseLowLimitBuyA /= AMOUNT_UNIT;
    result->ddr_u64CloseLowLimitLambBuyA /= AMOUNT_UNIT;
}

static void _calcResult(parse_param_t * ppp, da_day_result_t * result)
{
    result->ddr_dbBuyPercent = result->ddr_u64Buy * 100;
    if (result->ddr_u64All != 0)
        result->ddr_dbBuyPercent /= result->ddr_u64All;

    result->ddr_dbSoldPercent = result->ddr_u64Sold * 100;
    if (result->ddr_u64All != 0)
        result->ddr_dbSoldPercent /= result->ddr_u64All;

    result->ddr_dbLambBuyPercent = result->ddr_u64LambBuy * 100;
    if (result->ddr_u64All != 0)
        result->ddr_dbLambBuyPercent /= result->ddr_u64All;

    result->ddr_dbLambSoldPercent = result->ddr_u64LambSold * 100;
    if (result->ddr_u64All != 0)
        result->ddr_dbLambSoldPercent /= result->ddr_u64All;

    result->ddr_dbBuyInAmPercent = result->ddr_u64BuyInAm * 100;
    if (result->ddr_u64Buy != 0)
        result->ddr_dbBuyInAmPercent /= result->ddr_u64Buy;

    result->ddr_dbSoldInAmPercent = result->ddr_u64SoldInAm * 100;
    if (result->ddr_u64Sold != 0)
        result->ddr_dbSoldInAmPercent /= result->ddr_u64Sold;

    result->ddr_dbAboveLastClosingPricePercent = result->ddr_u64AboveLastClosingPrice * 100;
    if (result->ddr_u64All != 0)
        result->ddr_dbAboveLastClosingPricePercent /= result->ddr_u64All;

    result->ddr_dbLambBuyBelow100Percent = result->ddr_u64LambBuyBelow100 * 100;
    if (result->ddr_u64LambBuy != 0)
        result->ddr_dbLambBuyBelow100Percent /= result->ddr_u64LambBuy;

    result->ddr_dbLambSoldBelow100Percent = result->ddr_u64LambSoldBelow100 * 100;
    if (result->ddr_u64LambSold != 0)
        result->ddr_dbLambSoldBelow100Percent /= result->ddr_u64LambSold;

    result->ddr_dbCloseHighLimitSoldPercent = result->ddr_u64CloseHighLimitSold * 100;
    if (result->ddr_u64All != 0)
        result->ddr_dbCloseHighLimitSoldPercent /= result->ddr_u64All;

    result->ddr_dbCloseHighLimitLambSoldPercent = result->ddr_u64CloseHighLimitLambSold * 100;
    if (result->ddr_u64All != 0)
        result->ddr_dbCloseHighLimitLambSoldPercent /= result->ddr_u64All;

    result->ddr_dbCloseLowLimitBuyPercent = result->ddr_u64CloseLowLimitBuy * 100;
    if (result->ddr_u64All != 0)
        result->ddr_dbCloseLowLimitBuyPercent /= result->ddr_u64All;

    result->ddr_dbCloseLowLimitLambBuyPercent = result->ddr_u64CloseLowLimitLambBuy * 100;
    if (result->ddr_u64All != 0)
        result->ddr_dbCloseLowLimitLambBuyPercent /= result->ddr_u64All;

    if ((result->ddr_dbLastXLowPrice < result->ddr_dbClosingPrice) &&
        (result->ddr_dbLastXLowPrice != 0))
    {
        result->ddr_dbLastXInc =
            (result->ddr_dbClosingPrice - result->ddr_dbLastXLowPrice) * 100 /
            result->ddr_dbLastXLowPrice;
    }

    if (result->ddr_dbOpeningPrice > result->ddr_dbClosingPrice)
    {
        result->ddr_dbUpperShadowRatio =
            (result->ddr_dbHighPrice - result->ddr_dbOpeningPrice) /
            (result->ddr_dbOpeningPrice - result->ddr_dbClosingPrice);
        result->ddr_dbLowerShadowRatio = 
            (result->ddr_dbClosingPrice - result->ddr_dbLowPrice) /
            (result->ddr_dbOpeningPrice - result->ddr_dbClosingPrice);
    }
    else if (result->ddr_dbOpeningPrice < result->ddr_dbClosingPrice)
    {
        result->ddr_dbUpperShadowRatio =
            (result->ddr_dbHighPrice - result->ddr_dbClosingPrice) /
            (result->ddr_dbClosingPrice - result->ddr_dbOpeningPrice);
        result->ddr_dbLowerShadowRatio =
            (result->ddr_dbOpeningPrice - result->ddr_dbLowPrice) /
            (result->ddr_dbClosingPrice - result->ddr_dbOpeningPrice);
    }
    else
    {
        if (result->ddr_dbClosingPrice != result->ddr_dbLowPrice)
            result->ddr_dbUpperShadowRatio =
                (result->ddr_dbHighPrice - result->ddr_dbClosingPrice) /
                (result->ddr_dbClosingPrice - result->ddr_dbLowPrice);
        else
            result->ddr_dbUpperShadowRatio =
                (result->ddr_dbHighPrice - result->ddr_dbClosingPrice) * 100; /* = / 0.01*/

        if (result->ddr_dbHighPrice != result->ddr_dbClosingPrice)
            result->ddr_dbLowerShadowRatio =
                (result->ddr_dbClosingPrice - result->ddr_dbLowPrice) /
                (result->ddr_dbHighPrice - result->ddr_dbClosingPrice);
        else
            result->ddr_dbLowerShadowRatio =
                (result->ddr_dbClosingPrice - result->ddr_dbLowPrice) * 100;
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
    parse_param_t * ppp, da_day_result_t * daresult,
    trade_item_t * item, da_day_result_t * prev, olchar_t * time,
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
        dbclose = round(prev->ddr_dbClosingPrice * 110) / 100;
        if (item->ti_dbPrice >= dbclose)
            bCloseHighLimit = TRUE; 

        /*low limit*/
        dbclose = round(prev->ddr_dbClosingPrice * 90) / 100;
        if (item->ti_dbPrice <= dbclose)
            bCloseLowLimit = TRUE; 
    }

    /*closing price is of the last trade*/
    daresult->ddr_dbClosingPrice = item->ti_dbPrice;

    /*opening price is of the first trade*/
    if (bFirstLine)
        daresult->ddr_dbOpeningPrice = item->ti_dbPrice;

    if (daresult->ddr_dbHighPrice < item->ti_dbPrice)
        daresult->ddr_dbHighPrice = item->ti_dbPrice;

    if (daresult->ddr_dbLowPrice > item->ti_dbPrice)
        daresult->ddr_dbLowPrice = item->ti_dbPrice;

    if (strncmp(item->ti_pstrTime, ppp->pp_strLastX,
                ol_strlen(ppp->pp_strLastX)) >= 0)
    {
        if (daresult->ddr_dbLastXLowPrice > item->ti_dbPrice)
        {
            daresult->ddr_dbLastXLowPrice = item->ti_dbPrice;
            ol_strcpy(daresult->ddr_strLastTimeLowPrice, item->ti_pstrTime);
        }
    }

    daresult->ddr_u64All += item->ti_nVolume;
    daresult->ddr_u64AllA += item->ti_nAmount;

    if (bCloseHighLimit && ! daresult->ddr_bCloseHighLimit)
    {
        daresult->ddr_bCloseHighLimit = TRUE;
        ol_strcpy(daresult->ddr_strTimeCloseHighLimit, item->ti_pstrTime);
    }

    if (bCloseLowLimit && ! daresult->ddr_bCloseLowLimit)
    {
        daresult->ddr_bCloseLowLimit = TRUE;
        ol_strcpy(daresult->ddr_strTimeCloseLowLimit, item->ti_pstrTime);
    }

    if (item->ti_nOp == TRADE_OP_BUY)
    {
        if (item->ti_nVolume >= ppp->pp_nThres)
        {
            if (bCloseLowLimit)
            {
                daresult->ddr_u64CloseLowLimitBuy += item->ti_nVolume;
                daresult->ddr_u64CloseLowLimitBuyA += item->ti_nAmount;
            }
            else
            {
                daresult->ddr_u64Buy += item->ti_nVolume;
                daresult->ddr_u64BuyA += item->ti_nAmount;
                if (strncmp(item->ti_pstrTime, "12", 2) < 0)
                    daresult->ddr_u64BuyInAm += item->ti_nVolume;
            }
        }
        else
        {
            if (bCloseLowLimit)
            {
                daresult->ddr_u64CloseLowLimitLambBuy += item->ti_nVolume;
                daresult->ddr_u64CloseLowLimitLambBuyA += item->ti_nAmount;
            }
            else
            {
                daresult->ddr_u64LambBuy += item->ti_nVolume;
                daresult->ddr_u64LambBuyA += item->ti_nAmount;
                if (item->ti_nVolume < 100)
                    daresult->ddr_u64LambBuyBelow100 += item->ti_nVolume;
            }
        }
    }
    else if (item->ti_nOp == TRADE_OP_SOLD)
    {
        if (item->ti_nVolume >= ppp->pp_nThres)
        {
            if (bCloseHighLimit)
            {
                daresult->ddr_u64CloseHighLimitSold += item->ti_nVolume;
                daresult->ddr_u64CloseHighLimitSoldA += item->ti_nAmount;
            }
            else
            {
                daresult->ddr_u64Sold += item->ti_nVolume;
                daresult->ddr_u64SoldA += item->ti_nAmount;
                if (strncmp(item->ti_pstrTime, "12", 2) < 0)
                    daresult->ddr_u64SoldInAm += item->ti_nVolume;
            }
        }
        else
        {
            if (bCloseHighLimit)
            {
                daresult->ddr_u64CloseHighLimitLambSold += item->ti_nVolume;
                daresult->ddr_u64CloseHighLimitLambSoldA += item->ti_nAmount;
            }
            else
            {
                daresult->ddr_u64LambSold += item->ti_nVolume;
                daresult->ddr_u64LambSoldA += item->ti_nAmount;
                if (item->ti_nVolume < 100)
                    daresult->ddr_u64LambSoldBelow100 += item->ti_nVolume;
            }
        }
    }
    else if (item->ti_nOp == TRADE_OP_NA)
    {
        if (item->ti_nVolume >= ppp->pp_nThres)
        {
            daresult->ddr_u64Na += item->ti_nVolume;
            daresult->ddr_u64NaA += item->ti_nAmount;
        }
        else
        {
            daresult->ddr_u64LambNa += item->ti_nVolume;
            daresult->ddr_u64LambNaA += item->ti_nAmount;
        }
    }

    if (prev != NULL)
    {
        if (item->ti_dbPrice >= prev->ddr_dbClosingPrice)
            daresult->ddr_u64AboveLastClosingPrice += item->ti_nVolume;
    }

    return u32Ret;
}

static u32 _parseOneLine(
    parse_param_t * ppp, da_day_result_t * daresult,
    olchar_t * line, olsize_t llen, da_day_result_t * prev, olchar_t * time,
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
            ppp, daresult, &item, prev, time, bFirstLine);

    return u32Ret;
}

static void _calcData(parse_param_t * ppp, da_day_result_t * buffer, olint_t num)
{
    da_day_result_t * result, * prev, * end, * start;
    oldouble_t dbt, dbts, dbtls;

    result = prev = buffer;
    end = buffer + num;
    result ++;
    while (result < end)
    {
        if (result->ddr_dbHighPrice > prev->ddr_dbClosingPrice)
            result->ddr_dbHighPriceInc =
                (result->ddr_dbHighPrice - prev->ddr_dbClosingPrice) * 100 /
                prev->ddr_dbClosingPrice;

        if (result->ddr_dbLowPrice < prev->ddr_dbClosingPrice)
            result->ddr_dbLowPriceDec =
                (prev->ddr_dbClosingPrice - result->ddr_dbLowPrice) * 100 /
                prev->ddr_dbClosingPrice;

        if (result->ddr_dbClosingPrice > prev->ddr_dbClosingPrice)
        {
            result->ddr_dbClosingPriceInc =
                (result->ddr_dbClosingPrice - prev->ddr_dbClosingPrice) * 100 /
                prev->ddr_dbClosingPrice;
            if (result->ddr_dbLowPrice > prev->ddr_dbHighPrice)
                result->ddr_bGap = TRUE;
        }
        else
        {            
            result->ddr_dbClosingPriceDec =
                (prev->ddr_dbClosingPrice - result->ddr_dbClosingPrice) * 100 /
                prev->ddr_dbClosingPrice;
            if (result->ddr_dbHighPrice < prev->ddr_dbLowPrice)
                result->ddr_bGap = TRUE;
        }

        if (result - buffer >= 5)
        {
            dbts = dbtls = dbt = 0.0;
            start = result - 5;
            while (start < result)
            {
                dbt += start->ddr_u64All;
                dbts += start->ddr_u64CloseHighLimitSold + start->ddr_u64Sold;
                dbtls += start->ddr_u64CloseHighLimitLambSold + start->ddr_u64LambSold;
                start ++;
            }
            result->ddr_dbVolumeRatio = result->ddr_u64All * 5 / dbt;
            result->ddr_dbSoldVolumeRatio =
                (result->ddr_u64CloseHighLimitSold + result->ddr_u64Sold) * 5 / dbts;
            result->ddr_dbLambSoldVolumeRatio =
                (result->ddr_u64CloseHighLimitLambSold + result->ddr_u64LambSold) * 5 / dbtls;
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
    olchar_t * file, olchar_t *data, olsize_t sdata, parse_param_t * ppp,
    da_day_result_t * daresult, da_day_result_t * prev)
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
            ppp, daresult, line, llen, prev, strTime, TRUE);

    while (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _readDataFileLine(data, &line, &llen);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _parseOneLine(
                ppp, daresult, line, llen, prev, strTime, FALSE);
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
    _calcResult(ppp, daresult);

    return u32Ret;
}

static u32 _parseDataFile(
    olchar_t * file, parse_param_t * ppp,
    olint_t daindex, da_day_result_t * daresult, da_day_result_t * prev)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_t fd;
    olchar_t * data = NULL;
#define MAX_DATAFILE_SIZE  (512 * 1024)
    olsize_t sread;
    olchar_t filename[JF_LIMIT_MAX_PATH_LEN];

    ol_memset(daresult, 0, sizeof(*daresult));
    daresult->ddr_nIndex = daindex;
#define MAX_PRICE 9999.99
    daresult->ddr_dbLowPrice = MAX_PRICE;
    daresult->ddr_dbLastXLowPrice = MAX_PRICE;

    jf_file_getFileName(filename, JF_LIMIT_MAX_PATH_LEN, file);
    ol_strncpy(daresult->ddr_strDate, filename, 10);

    u32Ret = jf_file_open(file, O_RDONLY, &fd);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = jf_jiukun_allocMemory((void **)&data, MAX_DATAFILE_SIZE, 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sread = MAX_DATAFILE_SIZE;
        u32Ret = jf_file_readn(fd, data, &sread);
        if ((u32Ret == JF_ERR_NO_ERROR) && (sread == MAX_DATAFILE_SIZE))
            u32Ret = JF_ERR_BUFFER_TOO_SMALL;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseDataFileLine(
            file, data, sread, ppp, daresult, prev);

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

static u32 _calcOneTradeSummary(da_day_summary_t * cur)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (cur->dds_dbTurnoverRate == 0)
    {
        cur->dds_dbTurnoverRate = cur->dds_u64All * 100;
        cur->dds_dbTurnoverRate /= cur->dds_u64TradableShare;
    }

    if (cur->dds_dbOpeningPrice > cur->dds_dbClosingPrice)
    {
        cur->dds_dbUpperShadowRatio =
            (cur->dds_dbHighPrice - cur->dds_dbOpeningPrice) /
            (cur->dds_dbOpeningPrice - cur->dds_dbClosingPrice);
        cur->dds_dbLowerShadowRatio = 
            (cur->dds_dbClosingPrice - cur->dds_dbLowPrice) /
            (cur->dds_dbOpeningPrice - cur->dds_dbClosingPrice);
    }
    else if (cur->dds_dbOpeningPrice < cur->dds_dbClosingPrice)
    {
        cur->dds_dbUpperShadowRatio =
            (cur->dds_dbHighPrice - cur->dds_dbClosingPrice) /
            (cur->dds_dbClosingPrice - cur->dds_dbOpeningPrice);
        cur->dds_dbLowerShadowRatio =
            (cur->dds_dbOpeningPrice - cur->dds_dbLowPrice) /
            (cur->dds_dbClosingPrice - cur->dds_dbOpeningPrice);
    }
    else
    {
        if (cur->dds_dbClosingPrice != cur->dds_dbLowPrice)
            cur->dds_dbUpperShadowRatio =
                (cur->dds_dbHighPrice - cur->dds_dbClosingPrice) /
                (cur->dds_dbClosingPrice - cur->dds_dbLowPrice);
        else
            cur->dds_dbUpperShadowRatio =
                (cur->dds_dbHighPrice - cur->dds_dbClosingPrice) * 100; /* = / 0.01*/

        if (cur->dds_dbHighPrice != cur->dds_dbClosingPrice)
            cur->dds_dbLowerShadowRatio =
                (cur->dds_dbClosingPrice - cur->dds_dbLowPrice) /
                (cur->dds_dbHighPrice - cur->dds_dbClosingPrice);
        else
            cur->dds_dbLowerShadowRatio =
                (cur->dds_dbClosingPrice - cur->dds_dbLowPrice) * 100;
    }

    return u32Ret;
}

/* Date,Code,Name,ClosingPrice,HighPrice,LowPrice,OpeningPrice,LastClosingPrice,
   IncDec,IncDecPercent,TurnoverRate,Volumn,Amount,TotalMarketValue,TradableValue
 */
static u32 _parseTradeSummaryFileOneLine(
    olchar_t * buf, olsize_t sbuf, da_day_summary_t * cur)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t dbamount, dbtemp;
    jf_string_parse_result_t * result;
    jf_string_parse_result_field_t * field;
    olint_t index;
    olsize_t sdata;

    u32Ret = jf_string_parse(&result, buf, 0, sbuf, ",", 1);
    if ((u32Ret == JF_ERR_NO_ERROR) &&
        (result->jspr_u32NumOfResult != 15) && (result->jspr_u32NumOfResult != 13))
        u32Ret = JF_ERR_INVALID_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = result->jspr_pjsprfFirst;
        index = 1;
        while ((field != NULL) && (u32Ret == JF_ERR_NO_ERROR))
        {
            if (index == 1)
            {
                if (field->jsprf_sData >= 10)
                    ol_strncpy(cur->dds_strDate, field->jsprf_pstrData, 10);
                else
                    u32Ret = JF_ERR_INVALID_DATA;
            }
            else if (index == 3)
            {
                if (field->jsprf_pstrData[0] == 'X')
                {
                    if (field->jsprf_pstrData[1] == 'R')
                        cur->dds_bXR = TRUE;
                    else if (field->jsprf_pstrData[1] == 'D')
                        cur->dds_bXD = TRUE;
                }
                else if ((field->jsprf_pstrData[0] == 'D') && (field->jsprf_pstrData[1] == 'R'))
                    cur->dds_bDR = TRUE;
                else if ((field->jsprf_pstrData[0] == 'S') && (field->jsprf_pstrData[1] == 'T'))
                    cur->dds_bSt = TRUE;
                else if ((field->jsprf_pstrData[0] == '*') && (field->jsprf_pstrData[1] == 'S'))
                    cur->dds_bStDelisting = TRUE;
            }
            else if (index == 4)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->dds_dbClosingPrice);
                if ((u32Ret == JF_ERR_NO_ERROR) && (cur->dds_dbClosingPrice == 0))
                {
                    u32Ret = JF_ERR_NOT_READY;
                    break;
                }
            }
            else if (index == 5)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->dds_dbHighPrice);
            }
            else if (index == 6)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->dds_dbLowPrice);
            }
            else if (index == 7)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->dds_dbOpeningPrice);
            }
            else if (index == 8)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->dds_dbLastClosingPrice);
            }
            else if (index == 10)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->dds_dbClosingPriceRate);
            }
            else if (index == 11)
            {
                /*turnover rate string may be empty*/
                jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->dds_dbTurnoverRate);
            }
            else if (index == 12)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->dds_u64All);
            }
            else if (index == 13)
            {
                sdata = field->jsprf_sData;
                if (field->jsprf_pstrData[field->jsprf_sData - 1] == '\r')
                    sdata --;
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, sdata, &dbamount);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    cur->dds_u64AllA = (u64)dbamount;
                    cur->dds_u64AllA /= AMOUNT_UNIT;
                }
            }
            else if (index == 14)
            {
#if 0
                if (field->jsprf_sData > 16)
                {
                    jf_clieng_outputLine("The oldouble_t is overflow");
                    u32Ret = JF_ERR_PROGRAM_ERROR;
                }
#endif
                if (u32Ret == JF_ERR_NO_ERROR)
                    u32Ret = jf_string_getDoubleFromString(
                        field->jsprf_pstrData, field->jsprf_sData, &dbtemp);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    dbamount = dbtemp / cur->dds_dbClosingPrice;
                    cur->dds_u64GeneralCapital = (u64)dbamount;
                }
            }
            else if (index == 15)
            {
                sdata = field->jsprf_sData;
                if (field->jsprf_pstrData[field->jsprf_sData - 1] == '\r')
                    sdata --;
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, sdata, &dbtemp);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    dbamount = dbtemp / cur->dds_dbClosingPrice;
                    cur->dds_u64TradableShare = (u64)dbamount;
                }
            }

            field = field->jsprf_pjsprfNext;
            index ++;
        }

        jf_string_destroyParseResult(&result);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _calcOneTradeSummary(cur);

    return u32Ret;
}

static void _calcTradeDaySummary(
    da_day_summary_t * buffer, olint_t num)
{
    da_day_summary_t * result, * prev, * end, * start;
    oldouble_t dbt;
    olint_t i = 1;
    oldouble_t dbclose;

    result = prev = buffer;
    result->dds_nIndex = i ++;
    end = buffer + num;
    result ++;
    while (result < end)
    {
        /*close high limit*/
        dbclose = round(prev->dds_dbClosingPrice * 110) / 100;
        if (result->dds_dbClosingPrice >= dbclose)
            result->dds_bCloseHighLimit = TRUE;
        if (result->dds_dbHighPrice >= dbclose)
            result->dds_bHighHighLimit = TRUE;

        /*close low limit*/
        dbclose = round(prev->dds_dbClosingPrice * 90) / 100;
        if (result->dds_dbClosingPrice <= dbclose)
            result->dds_bCloseLowLimit = TRUE; 
        if (result->dds_dbLowPrice <= dbclose)
            result->dds_bLowLowLimit = TRUE; 

        if (result->dds_dbClosingPrice > prev->dds_dbClosingPrice)
        {
            if (result->dds_dbLowPrice > prev->dds_dbHighPrice)
                result->dds_bGap = TRUE;
        }
        else
        {            
            if (result->dds_dbHighPrice < prev->dds_dbLowPrice)
                result->dds_bGap = TRUE;
        }
        /*closing price is read from summary file*/
//        result->dds_dbClosingPriceRate =
//            (result->dds_dbClosingPrice - prev->dds_dbClosingPrice) * 100 /
//            prev->dds_dbClosingPrice;
        result->dds_dbHighPriceRate =
            (result->dds_dbHighPrice - prev->dds_dbClosingPrice) * 100 /
            prev->dds_dbClosingPrice;

        if (result - buffer >= 5)
        {
            dbt = 0.0;
            start = result - 5;
            while (start < result)
            {
                dbt += start->dds_u64All;
                start ++;
            }
            result->dds_dbVolumeRatio = result->dds_u64All * 5;
            result->dds_dbVolumeRatio /= dbt;
        }
        result->dds_nIndex = i ++;

        prev ++;
        result ++;
    }
}

static u32 _fillTradeDaySummary(
    olchar_t * buf, olsize_t sbuf, da_day_summary_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, total;
    da_day_summary_t * cur;
    olchar_t * line, *start, * end;

    cur = buffer + *numofresult - 1;
    total = 0;

    start = line = buf;
    end = buf + sbuf;
    while (start < end)
    {
        if (*start == '\n')
        {
            memset(cur, 0, sizeof(*cur));
            *start = '\0';

            u32Ret = _parseTradeSummaryFileOneLine(line, start - line, cur);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                total ++;
                cur --;
                if (total == *numofresult)
                    break;
            }
            else if (u32Ret == JF_ERR_NOT_READY)
                u32Ret = JF_ERR_NO_ERROR; /*not a trading day*/

            line = start + 1;
        }

        start ++;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (total < *numofresult)
        {
            for (i = 0; i < total; i ++)
            {
                memcpy(
                    buffer + i, buffer + *numofresult - total + i,
                    sizeof(da_day_summary_t));
            }
        }

        _calcTradeDaySummary(buffer, total);
        *numofresult = total;
    }

    return u32Ret;
}

static void _restoreDaySummary(
    da_day_summary_t * start, da_day_summary_t * end,
    oldouble_t offset, oldouble_t rate)
{
    da_day_summary_t * cur;

    cur = start;
    while (cur < end)
    {
        cur->dds_dbOpeningPrice = (cur->dds_dbOpeningPrice - offset) / rate;
        cur->dds_dbClosingPrice = (cur->dds_dbClosingPrice - offset) / rate;
        cur->dds_dbHighPrice = (cur->dds_dbHighPrice - offset) / rate;
        cur->dds_dbLowPrice = (cur->dds_dbLowPrice - offset) / rate;

        cur ++;
    }

}

static u32 _forwardRestorationOfRight(
    olchar_t * pstrDataDir, da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t rate;
    oldouble_t offset;
    olint_t i;
    da_day_summary_t * cur, * prev;

    prev = buffer;
    cur = buffer + 1;
    for (i = 1; i < num; i ++)
    {
        if (prev->dds_dbClosingPrice != cur->dds_dbLastClosingPrice)
        {
            rate = (oldouble_t)cur->dds_u64GeneralCapital / 100;
            rate /= prev->dds_u64GeneralCapital / 100;

            offset = prev->dds_dbClosingPrice - cur->dds_dbLastClosingPrice * rate;

            _restoreDaySummary(buffer, cur, offset, rate);
        }

        prev ++;
        cur ++;
    }

    prev = buffer;
    cur = buffer + 1;
    for (i = 1; i < num; i ++)
    {
        /*closing price is read from summary file*/
//        cur->dds_dbClosingPriceRate =
//            (cur->dds_dbClosingPrice - prev->dds_dbClosingPrice) * 100 / prev->dds_dbClosingPrice;

        cur->dds_bGap = FALSE;
        if (cur->dds_dbClosingPrice > prev->dds_dbClosingPrice)
        {
            if (cur->dds_dbLowPrice > prev->dds_dbHighPrice)
                cur->dds_bGap = TRUE;
        }
        else
        {            
            if (cur->dds_dbHighPrice < prev->dds_dbLowPrice)
                cur->dds_bGap = TRUE;
        }

        prev ++;
        cur ++;
    }

    return u32Ret;
}

#if 0
static u32 _parseQuotationFileOneLine_OLD(
    olchar_t * buf, olsize_t sbuf, quo_entry_t * cur)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count;

    count = sscanf(
        buf,
        "%s\t%lf\t%llu\t%lf\t"
        "%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t"
        "%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t",
        cur->qe_strTime,
        &cur->qe_dbCurPrice,
        &cur->qe_u64Volume, &cur->qe_dbAmount,
        &cur->qe_qdpBuy[0].qdp_dbPrice,
        &cur->qe_qdpBuy[0].qdp_u64Volume,
        &cur->qe_qdpBuy[1].qdp_dbPrice,
        &cur->qe_qdpBuy[1].qdp_u64Volume,
        &cur->qe_qdpBuy[2].qdp_dbPrice,
        &cur->qe_qdpBuy[2].qdp_u64Volume,
        &cur->qe_qdpBuy[3].qdp_dbPrice,
        &cur->qe_qdpBuy[3].qdp_u64Volume,
        &cur->qe_qdpBuy[4].qdp_dbPrice,
        &cur->qe_qdpBuy[4].qdp_u64Volume,
        &cur->qe_qdpSold[0].qdp_dbPrice,
        &cur->qe_qdpSold[0].qdp_u64Volume,
        &cur->qe_qdpSold[1].qdp_dbPrice,
        &cur->qe_qdpSold[1].qdp_u64Volume,
        &cur->qe_qdpSold[2].qdp_dbPrice,
        &cur->qe_qdpSold[2].qdp_u64Volume,
        &cur->qe_qdpSold[3].qdp_dbPrice,
        &cur->qe_qdpSold[3].qdp_u64Volume,
        &cur->qe_qdpSold[4].qdp_dbPrice,
        &cur->qe_qdpSold[4].qdp_u64Volume);

    return u32Ret;
}

static u32 _parseQuotationFile_OLD(
    olchar_t * buf, olsize_t sbuf, quo_entry_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total;
    quo_entry_t * cur;
    olchar_t * line, *start, * end;

    cur = buffer;
    total = 0;

    start = line = buf;
    end = buf + sbuf;
    while (start < end)
    {
        if (*start == '\n')
        {
            memset(cur, 0, sizeof(*cur));
            *start = '\0';

            u32Ret = _parseQuotationFileOneLine(line, start - line, cur);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                total ++;
                cur ++;
                if (total == *numofresult)
                    break;
            }

            line = start + 1;
        }

        start ++;
    }

    *numofresult = total;

    return u32Ret;
}
#endif

static u32 _parseSectorFile(
    const olchar_t * pstrFullpath, olchar_t * buf, olsize_t sbuf, tmp_sector_file_t * ptsf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    olint_t total;
    olchar_t * line, *start, * end;
    olchar_t stocklist[2048];
    parse_sector_info_t * ppsi;

    stocklist[0] = '\0';
//    total = 0;
    start = line = buf;
    end = buf + sbuf;
    while (start < end)
    {
        if (*start == '\r')
        {
            strncat(stocklist, line, 8);
            ol_strcat(stocklist, ",");

            line = start + 1;
        }

        start ++;
    }

    jf_string_lower(stocklist);
    ppsi = &ptsf->sector[ptsf->numofsector];
    u32Ret = jf_string_duplicate(&ppsi->psi_pstrStocks, stocklist);
    if (u32Ret == JF_ERR_NO_ERROR)
        jf_file_getFileName(ppsi->psi_strName, sizeof(ppsi->psi_strName), pstrFullpath);

    if (u32Ret == JF_ERR_NO_ERROR)
        ptsf->numofsector ++;

    return u32Ret;
}

static u32 _handleSectorFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tmp_sector_file_t * ptsf = (tmp_sector_file_t *)pArg;
    olsize_t sread;
    jf_file_t fd;

    if (ptsf->numofsector == ptsf->maxsector)
        return u32Ret;

    u32Ret = jf_file_open(pstrFullpath, O_RDONLY, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sread = ptsf->sdata;
        u32Ret = jf_file_readn(fd, ptsf->data, &sread);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            _parseSectorFile(
                pstrFullpath, ptsf->data, sread, ptsf);
        }

        jf_file_close(&fd);
    }

    return u32Ret;
}

static u32 _parseQuotationFileOneLine(
    olchar_t * line, olsize_t llen, stock_quo_t * psq, boolean_t bFirstLine)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    quo_entry_t * cur, * end = NULL;
    olint_t count;

//    assert(psq->sq_nNumOfEntry < psq->sq_nMaxEntry);

    if (psq->sq_nNumOfEntry > 0)
        end = &psq->sq_pqeEntry[psq->sq_nNumOfEntry - 1];
    cur = &psq->sq_pqeEntry[psq->sq_nNumOfEntry];

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        count = sscanf(
            line,
            "%s\t%lf\t%lf\t%lf\t%llu\t%lf\t"
            "%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t"
            "%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu",
            cur->qe_strTime,
            &cur->qe_dbCurPrice,
            &cur->qe_dbHighPrice, &cur->qe_dbLowPrice,
            &cur->qe_u64Volume, &cur->qe_dbAmount,
            &cur->qe_qdpBuy[0].qdp_dbPrice,
            &cur->qe_qdpBuy[0].qdp_u64Volume,
            &cur->qe_qdpBuy[1].qdp_dbPrice,
            &cur->qe_qdpBuy[1].qdp_u64Volume,
            &cur->qe_qdpBuy[2].qdp_dbPrice,
            &cur->qe_qdpBuy[2].qdp_u64Volume,
            &cur->qe_qdpBuy[3].qdp_dbPrice,
            &cur->qe_qdpBuy[3].qdp_u64Volume,
            &cur->qe_qdpBuy[4].qdp_dbPrice,
            &cur->qe_qdpBuy[4].qdp_u64Volume,
            &cur->qe_qdpSold[0].qdp_dbPrice,
            &cur->qe_qdpSold[0].qdp_u64Volume,
            &cur->qe_qdpSold[1].qdp_dbPrice,
            &cur->qe_qdpSold[1].qdp_u64Volume,
            &cur->qe_qdpSold[2].qdp_dbPrice,
            &cur->qe_qdpSold[2].qdp_u64Volume,
            &cur->qe_qdpSold[3].qdp_dbPrice,
            &cur->qe_qdpSold[3].qdp_u64Volume,
            &cur->qe_qdpSold[4].qdp_dbPrice,
            &cur->qe_qdpSold[4].qdp_u64Volume);

        if (count != 26)
            u32Ret = JF_ERR_INVALID_DATA;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (bFirstLine)
            psq->sq_dbOpeningPrice = cur->qe_dbCurPrice;

        if ((end == NULL) ||
            (end->qe_u64Volume != cur->qe_u64Volume))
            psq->sq_nNumOfEntry ++;
    }

    return u32Ret;
}

static u32 _readQuotationFileLine(
    olchar_t * data, olsize_t sdata, olchar_t ** prevline, olsize_t * llen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * line, * end;

    line = *prevline + *llen;
    if (*line == '\0')
        line ++; 

    if (line >= data + sdata)
        return JF_ERR_END_OF_FILE;

    end = line;
    while (end < data + sdata)
    {
        if (*end == '\n')
        {
            *end = '\0';
            break;
        }

        end ++;
    }

    *prevline = line;
    *llen = end - line;

    return u32Ret;
}

static u32 _parseQuotationFileLine(
    olchar_t * file, olchar_t * data, olsize_t sdata, stock_quo_t * psq)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * line;
    olsize_t llen = 0;

    line = data;

    u32Ret = _readQuotationFileLine(data, sdata, &line, &llen);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseQuotationFileOneLine(
            line, llen, psq, TRUE);

    while ((u32Ret == JF_ERR_NO_ERROR) && (psq->sq_nNumOfEntry < psq->sq_nMaxEntry))
    {
        u32Ret = _readQuotationFileLine(data, sdata, &line, &llen);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _parseQuotationFileOneLine(
                line, llen, psq, FALSE);
        }
    } 

    if (u32Ret == JF_ERR_END_OF_FILE)
        u32Ret = JF_ERR_NO_ERROR;

    if (u32Ret != JF_ERR_NO_ERROR)
        jf_logger_logErrMsg(u32Ret, "Error analysising file %s", file);

    return u32Ret;
}

static u32 _parseQuotationFile(
    olchar_t * file, stock_quo_t * psq)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_t fd;
    olchar_t * data = NULL;
#define MAX_DATAFILE_SIZE  (512 * 1024)
    olsize_t sread;

    u32Ret = jf_file_open(file, O_RDONLY, &fd);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = jf_jiukun_allocMemory((void **)&data, MAX_DATAFILE_SIZE, 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sread = MAX_DATAFILE_SIZE;
        u32Ret = jf_file_readn(fd, data, &sread);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        assert(sread != MAX_DATAFILE_SIZE);
        u32Ret = _parseQuotationFileLine(
            file, data, sread, psq);
    }

    if (data != NULL)
        jf_jiukun_freeMemory((void **)&data);

    jf_file_close(&fd);

    return u32Ret;
}

static boolean_t _filterQuotationFileEntry(jf_dir_entry_t * entry)
{
    boolean_t bRet = TRUE;

    if (ol_strncmp(entry->jde_strName, "quotation-", 10) == 0)
        bRet = FALSE;

    return bRet;
}

static u32 _fillTradeDaySummaryFromDate(
    olchar_t * buf, olsize_t sbuf, olchar_t * pstrDateFrom,
    da_day_summary_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total;
    da_day_summary_t * cur;
    olchar_t * line, * start, * end;

    cur = buffer;
    total = 0;

    if (pstrDateFrom == NULL)
    {
        end = buf + sbuf - 1;
        while (*end != '\n')
            end --;
    }
    else
    {
        u32Ret = jf_string_locateSubString(buf, pstrDateFrom, &end);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            while (*end != '\n')
                end ++;
        }
        else
        {
            return JF_ERR_NOT_FOUND;
        }
    }

    start = line = end;
    while (start >= buf)
    {
        *end = '\0';
        if ((*start == '\n') || (start == buf))
        {
            ol_memset(cur, 0, sizeof(*cur));
            if (start == buf)
                line = start;
            else
                line = start + 1;

            u32Ret = _parseTradeSummaryFileOneLine(line, end - line, cur);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                total ++;
                cur ++;
                if (total == *numofresult)
                    break;
            }
            else if (u32Ret == JF_ERR_NOT_READY)
                u32Ret = JF_ERR_NO_ERROR; /*not a trading day*/

            end = start;
        }

        start --;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _calcTradeDaySummary(buffer, total);
        *numofresult = total;
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */
u32 parseDataFile(olchar_t * file, parse_param_t * ppp,
    olint_t daindex, da_day_result_t * daresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _parseDataFile(file, ppp, daindex, daresult, NULL);

    return u32Ret;
}

u32 readTradeDayDetail(olchar_t * dirpath, parse_param_t * ppp,
    da_day_result_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_dir_entry_t * entrylist = NULL, *entry;
    olint_t i, count, total = *numofresult;
    olchar_t file[JF_LIMIT_MAX_PATH_LEN * 2];
    da_day_result_t * result, * prev = NULL;

    u32Ret = jf_jiukun_allocMemory(
        (void **)&entrylist, sizeof(jf_dir_entry_t) * total,
        JF_FLAG_MASK(JF_JIUKUN_MEM_ALLOC_FLAG_ZERO));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_dir_scan(
            dirpath, entrylist, &total, _filterDirEntry, jf_dir_compareDirEntry);
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        if (entrylist != NULL)
            jf_jiukun_freeMemory((void **)&entrylist);
        return u32Ret;
    }

    if (ppp->pp_nLastCount != 0)
    {
        ppp->pp_nEnd = total;
        if (total > *numofresult)
            total = *numofresult;
        if (total > ppp->pp_nLastCount)
            total = ppp->pp_nLastCount;
        ppp->pp_nStart = ppp->pp_nEnd - total + 1;
    }
    else
    {
        if (total > *numofresult)
            total = *numofresult;

        total = total - ppp->pp_nStart + 1;
        if (total > ppp->pp_nEnd - ppp->pp_nStart + 1)
            total = ppp->pp_nEnd - ppp->pp_nStart + 1;
    }

    entry = entrylist + ppp->pp_nStart - 1;
    result = buffer;
    count = 0;
    for(i = ppp->pp_nStart;
        (i < ppp->pp_nStart + total) && (u32Ret == JF_ERR_NO_ERROR); i++)
    {
        ol_snprintf(file, sizeof(file), "%s%c%s",
                 dirpath, PATH_SEPARATOR, entry->jde_strName);

        if ((ppp->pp_pstrDateFrom == NULL) ||
            ol_strncmp(entry->jde_strName, ppp->pp_pstrDateFrom,
                    ol_strlen(ppp->pp_pstrDateFrom)) >= 0)
        {
            u32Ret = _parseDataFile(file, ppp, i, result, prev);
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
        _calcData(ppp, buffer, total);

    *numofresult = count;

    return u32Ret;
}

da_day_summary_t * getDaySummaryWithHighestClosingPrice(
    da_day_summary_t * buffer, olint_t num)
{
    olint_t i;
    da_day_summary_t * result, * high = NULL;

    high = result = buffer + num - 1;
    for (i = 0; i < num; i ++)
    {
        if (high->dds_dbClosingPrice < result->dds_dbClosingPrice)
            high = result;

        result --;
    }

    return high;
}

da_day_summary_t * getDaySummaryWithHighestHighPrice(
    da_day_summary_t * buffer, olint_t num)
{
    olint_t i;
    da_day_summary_t * result, * high = NULL;

    high = result = buffer + num - 1;
    for (i = 0; i < num; i ++)
    {
        if (high->dds_dbHighPrice < result->dds_dbHighPrice)
            high = result;

        result --;
    }

    return high;
}

/*return NULL if the date has no trade*/
u32 getDaySummaryWithDate(
    da_day_summary_t * buffer, olint_t num, olchar_t * strdate, da_day_summary_t ** ret)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    olint_t i;
    da_day_summary_t * result;

    result = buffer + num - 1;
    for (i = 0; i < num; i ++)
    {
        if (ol_strncmp(result->dds_strDate, strdate, 10) == 0)
        {
            *ret = result;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }
        result --;
    }

    return u32Ret;
}

/*return the next day if the date has no trade*/
da_day_summary_t * getDaySummaryWithDate2(
    da_day_summary_t * buffer, olint_t num, olchar_t * strdate)
{
    olint_t i;
    da_day_summary_t * result;

    result = buffer;
    for (i = 0; i < num; i ++)
    {
        if (strncmp(result->dds_strDate, strdate, 10) >= 0)
            return result;

        result ++;
    }

    return NULL;
}

/*return the prior day if the date has no trade*/
da_day_summary_t * getDaySummaryWithDate3(
    da_day_summary_t * buffer, olint_t num, olchar_t * strdate)
{
    olint_t i;
    da_day_summary_t * result;

    result = buffer + num - 1;
    for (i = 0; i < num; i ++)
    {
        if (ol_strncmp(result->dds_strDate, strdate, 10) <= 0)
            return result;

        result --;
    }

    return NULL;
}

da_day_summary_t * getDaySummaryWithLowestClosingPrice(
    da_day_summary_t * buffer, olint_t num)
{
    olint_t i;
    da_day_summary_t * result, * low = NULL;

    low = result = buffer + num - 1;
    for (i = 0; i < num; i ++)
    {
        if (low->dds_dbClosingPrice >= result->dds_dbClosingPrice)
            low = result;

        result --;
    }

    return low;
}

da_day_summary_t * getDaySummaryWithLowestLowPrice(
    da_day_summary_t * buffer, olint_t num)
{
    olint_t i;
    da_day_summary_t * result, * low = NULL;

    low = result = buffer + num - 1;
    for (i = 0; i < num; i ++)
    {
        if (low->dds_dbLowPrice >= result->dds_dbLowPrice)
            low = result;

        result --;
    }

    return low;
}

void getDaySummaryInflexionPoint(
    da_day_summary_t * buffer, olint_t num, da_day_summary_t ** ppp, olint_t * nump)
{
    da_day_summary_t * prior, * next, * cur, * end;
    olint_t count = 0;
    boolean_t bUp = TRUE;
    oldouble_t dbp, dbp2;
#define INFLEXION_THRES  0.1

    if (num < 2)
    {
        *nump = 0;
        return;
    }

    if (*nump < 2)
    {
        *nump = 0;
        return;
    }

    prior = buffer;
    cur = prior + 1;
    end = buffer + num - 1;
    while (cur < end)
    {
        if ((cur->dds_dbClosingPrice >= 
             prior->dds_dbClosingPrice * (1 + STRAIGHT_LINE_MOTION)) ||
            (cur->dds_dbClosingPrice <= 
             prior->dds_dbClosingPrice * (1 - STRAIGHT_LINE_MOTION)))
            break;

        cur ++;
    }

    next = cur;
    if (cur->dds_dbClosingPrice >= prior->dds_dbClosingPrice)
        bUp = TRUE;
    else
        bUp = FALSE;
    cur ++;

    while (cur <= end)
    {
        if (bUp)
        {
            if (cur->dds_dbClosingPrice >= next->dds_dbClosingPrice)
            {
                next = cur;
            }
            else
            {
                dbp = next->dds_dbClosingPrice - prior->dds_dbClosingPrice;
                dbp2 = next->dds_dbClosingPrice - cur->dds_dbClosingPrice;

                if ((dbp2 > dbp * INFLEXION_THRES) &&
                    (dbp2 > next->dds_dbClosingPrice * STRAIGHT_LINE_MOTION))
                {
                    ppp[count ++] = prior;
                    prior = next;
                    next = cur;
                    bUp = FALSE;
                }
            }
        }
        else
        {
            if (cur->dds_dbClosingPrice <= next->dds_dbClosingPrice)
            {
                next = cur;
            }
            else
            {
                dbp = prior->dds_dbClosingPrice - next->dds_dbClosingPrice;
                dbp2 = cur->dds_dbClosingPrice - next->dds_dbClosingPrice;
                if ((dbp2 > dbp * INFLEXION_THRES) &&
                    (dbp2 > next->dds_dbClosingPrice * STRAIGHT_LINE_MOTION))
                {
                    ppp[count ++] = prior;
                    prior = next;
                    next = cur;
                    bUp = TRUE;
                }
            }
        }

        if (count == *nump)
            break;

        cur ++;
    }

    if (count == 0)
    {
        ppp[count ++] = buffer;
        ppp[count ++] = end;
    }
    else
    {
        if (count < *nump)
            ppp[count ++] = prior;
        if (count < *nump)
            ppp[count ++] = end;
    }

    *nump = count;
}

void adjustDaySummaryInflexionPoint(
    da_day_summary_t ** ppp, olint_t * nump, olint_t adjust)
{
    da_day_summary_t * next;
    olint_t i;
    olsize_t size;

    for (i = 1; i < *nump; i ++)
    {
        if (ppp[i]->dds_dbClosingPrice > ppp[i - 1]->dds_dbClosingPrice)
        {
            if (i + 2 >= *nump)
                break;

            if ((ppp[i + 2]->dds_dbClosingPrice > ppp[i]->dds_dbClosingPrice) &&
                (ppp[i + 1]->dds_dbClosingPrice > ppp[i - 1]->dds_dbClosingPrice))
            {
                next = ppp[i] + adjust;
                if (next->dds_dbClosingPrice > ppp[i]->dds_dbClosingPrice)
                {
                    size = (*nump - i - 2) * sizeof(da_day_summary_t *);
                    if (size != 0)
                        ol_memcpy(&ppp[i], &ppp[i + 2], size);
                    *nump -= 2;
                    i --;
                }
            }
        }
    }


}

void getDaySummaryInflexionPoint2(
    da_day_summary_t * buffer, olint_t num, da_day_summary_t ** ppp, olint_t * nump)
{
    da_day_summary_t * prior, * next, * cur, * end;
    da_day_summary_t * low, * start;
    olint_t count = 0;

    if (num < 2)
    {
        *nump = 0;
        return;
    }

    if (*nump < 2)
    {
        *nump = 0;
        return;
    }

    prior = buffer;
    end = buffer + num - 1;
    cur = prior + 1;
    next = cur + 1;

    while (cur <= end)
    {
        if ((cur->dds_dbClosingPrice >= prior->dds_dbClosingPrice) &&
            ((cur == end) || (next->dds_dbClosingPriceRate < 0)))
        {
            start = prior + 1;
            while (start <= cur)
            {
                if (start->dds_dbClosingPriceRate < 0)
                {
                    /*find decreasing day*/
                    low = getDaySummaryWithLowestClosingPrice(prior, cur - prior + 1);
                    ppp[count ++] = prior;
                    ppp[count ++] = low;
                    prior = cur;

                    break;
                }

                start ++;
            }

            if (start > cur)
            {
                /*can't find decreasing day*/
                ppp[count ++] = prior;
                prior = cur;
            }
        }

        if (count == *nump)
            break;

        cur ++;
        next ++;
    }

    if (count == 0)
    {
        ppp[count ++] = buffer;
        ppp[count ++] = end;
    }
    else
    {
        if ((count < *nump) &&
            (end->dds_dbClosingPrice < prior->dds_dbClosingPrice))
            ppp[count ++] = prior;
        if (count < *nump)
            ppp[count ++] = end;
    }

    *nump = count;
}

void locateInflexionPointRegion(
    da_day_summary_t ** ppp, olint_t nump, da_day_summary_t * dest,
    da_day_summary_t ** start, da_day_summary_t ** end)
{
    olint_t i;
    da_day_summary_t * dastart, * daend;

    if (dest == ppp[0])
    {
        *start = ppp[0];
        *end = ppp[1];
        return;
    }
    else if (dest == ppp[nump - 1])
    {
        if (nump - 2 == 0)
            *start = ppp[nump - 2];
        else
            *start = ppp[nump - 2] + 1;
        *end = ppp[nump - 1];
        return;
    }

    *start = *end = NULL;

    for (i = 1; i < nump; i ++)
    {
        dastart = ppp[i - 1] + 1;
        daend = ppp[i];
        while (dastart <= daend)
        {
            if (dest == dastart)
            {
                *start = ppp[i - 1] + 1;
                *end = ppp[i];
                return;
            }

            dastart ++;
        }
    }
}

da_day_summary_t * getInflexionPointWithHighestClosingPrice(
    da_day_summary_t ** ppp, olint_t nump)
{
    da_day_summary_t * high;
    olint_t i;

    high = ppp[0];
    for (i = 0; i < nump; i ++)
    {
        if (high->dds_dbClosingPrice < ppp[i]->dds_dbClosingPrice)
            high = ppp[i];
    }

    return high;
}

u32 readTradeDaySummary(
    olchar_t * pstrDataDir, da_day_summary_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    FILE * fp = NULL;
    olchar_t * buf = NULL;
    olsize_t sbuf = 1024 * 1024;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];

    ol_snprintf(
        strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
        pstrDataDir, PATH_SEPARATOR, ls_pstrTradeSummaryFile);

    u32Ret = jf_jiukun_allocMemory((void **)&buf, sbuf, 0);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_filestream_open(strFullname, "r", &fp);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_filestream_readn(fp, buf, &sbuf);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (sbuf == 0)
            *numofresult = 0;
        else
            u32Ret = _fillTradeDaySummary(buf, sbuf, buffer, numofresult);
    }

    if (buf != NULL)
        jf_jiukun_freeMemory((void **)&buf);
    if (fp != NULL)
        jf_filestream_close(&fp);

    if (u32Ret != JF_ERR_NO_ERROR)
        jf_logger_logErrMsg(u32Ret, "Error reading trading summary day result for %s", pstrDataDir);

    return u32Ret;
}

u32 readTradeDaySummaryWithFRoR(
    olchar_t * pstrDataDir, da_day_summary_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = readTradeDaySummary(
        pstrDataDir, buffer, numofresult);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _forwardRestorationOfRight(pstrDataDir, buffer, *numofresult);

    return u32Ret;
}

u32 readTradeDaySummaryFromDate(
    olchar_t * pstrDataDir, olchar_t * pstrStartDate, da_day_summary_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    FILE * fp = NULL;
    olchar_t * buf = NULL;
    olsize_t sbuf = 1024 * 1024;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];

    if (*numofresult == 0)
        return JF_ERR_INVALID_PARAM;
    
    ol_snprintf(
        strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
        pstrDataDir, PATH_SEPARATOR, ls_pstrTradeSummaryFile);

    u32Ret = jf_jiukun_allocMemory((void **)&buf, sbuf, 0);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_filestream_open(strFullname, "r", &fp);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_filestream_readn(fp, buf, &sbuf);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (sbuf == 0)
            *numofresult = 0;
        else
            u32Ret = _fillTradeDaySummaryFromDate(buf, sbuf, pstrStartDate, buffer, numofresult);
    }

    if (buf != NULL)
        jf_jiukun_freeMemory((void **)&buf);
    if (fp != NULL)
        jf_filestream_close(&fp);

    if (u32Ret != JF_ERR_NO_ERROR)
        jf_clieng_outputLine("Error reading trading summary day result for %s", pstrDataDir);

    return u32Ret;
}

u32 readTradeDaySummaryFromDateWithFRoR(
    olchar_t * pstrDataDir, olchar_t * pstrStartDate, da_day_summary_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = readTradeDaySummaryFromDate(
        pstrDataDir, pstrStartDate, buffer, numofresult);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("read dds from date %s, total %d days", pstrStartDate, *numofresult);
        u32Ret = _forwardRestorationOfRight(pstrDataDir, buffer, *numofresult);
    }

    return u32Ret;
}

u32 readTradeDaySummaryUntilDate(
    olchar_t * pstrDataDir, olchar_t * pstrEndDate, u32 u32Count, da_day_summary_t * buffer,
    olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    FILE * fp = NULL;
    olchar_t * buf = NULL;
    olsize_t sbuf = 1024 * 1024;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];
    olchar_t * start;
    u32 u32Line = 0;

    if ((*numofresult == 0) || (pstrEndDate == NULL))
        return JF_ERR_INVALID_PARAM;
    
    ol_snprintf(
        strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
        pstrDataDir, PATH_SEPARATOR, ls_pstrTradeSummaryFile);

    u32Ret = jf_jiukun_allocMemory((void **)&buf, sbuf, 0);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_filestream_open(strFullname, "r", &fp);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_filestream_readn(fp, buf, &sbuf);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        start = buf;

        u32Ret = jf_string_locateSubString(buf, pstrEndDate, &start);
        if (u32Ret != JF_ERR_NO_ERROR)
            u32Ret = JF_ERR_NOT_FOUND;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Read maximum u32Count trading day after end date*/
        if ((start > buf) && (u32Count > 0))
        {
            start -= 2;
            u32Line = 0;
            while ((u32Line < u32Count) && (start != buf))
            {
                if (*start == '\n')
                    u32Line ++;

                start --;
            }

            if (start != buf)
                start ++;
        }

        sbuf -= start - buf;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (sbuf == 0)
            *numofresult = 0;
        else
            u32Ret = _fillTradeDaySummary(start, sbuf, buffer, numofresult);
    }

    if (buf != NULL)
        jf_jiukun_freeMemory((void **)&buf);
    if (fp != NULL)
        jf_filestream_close(&fp);

    if (u32Ret != JF_ERR_NO_ERROR)
        jf_logger_logErrMsg(u32Ret, "Error reading trade day summary until date for %s", pstrDataDir);

    return u32Ret;
}

u32 readTradeDaySummaryUntilDateWithFRoR(
    olchar_t * pstrDataDir, olchar_t * pstrEndDate, u32 u32Count, da_day_summary_t * buffer,
    olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = readTradeDaySummaryUntilDate(
        pstrDataDir, pstrEndDate, u32Count, buffer, numofresult);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("read dds until date %s, total %d days", pstrEndDate, *numofresult);
        u32Ret = _forwardRestorationOfRight(pstrDataDir, buffer, *numofresult);
    }

    return u32Ret;
}

olint_t daySummaryEndDataCount(da_day_summary_t * buffer, olint_t num, olchar_t * pstrEndDate)
{
    olint_t total = num;
    da_day_summary_t * end;

    if (num == 0)
        return num;

    end = buffer + num - 1;
    while (end >= buffer)
    {
        if (strcmp(end->dds_strDate, pstrEndDate) == 0)
        {
            total = end - buffer + 1;
            break;
        }

        end --;
    }

    if (end < buffer)
        total = 0;

    return total;
}

boolean_t isStraightLineMotion(
    da_day_summary_t * buffer, olint_t total, olint_t ndays)
{
    boolean_t bRet = TRUE;
    olint_t i;
    da_day_summary_t * end, * high;
    oldouble_t dblow;

    if (total < ndays)
        return FALSE;

    end = buffer + total - 1;
    high = getDaySummaryWithHighestClosingPrice(buffer + total - ndays, ndays);
    dblow = high->dds_dbClosingPrice * (1 - STRAIGHT_LINE_MOTION);
    for (i = 0; i < ndays; i ++)
    {
        if (end->dds_dbClosingPrice < dblow)
            break;
        end --;
    }
    if (i == ndays)
        return bRet;

    bRet = FALSE;

    return bRet;
}

boolean_t isStraightHighLimitDay(da_day_summary_t * pdds)
{
    boolean_t bRet = FALSE;

    if (pdds->dds_bCloseHighLimit &&
        (pdds->dds_dbOpeningPrice == pdds->dds_dbClosingPrice))
        bRet = TRUE;

    return bRet;
}

void getStringDaySummaryStatus(da_day_summary_t * summary, olchar_t * pstr)
{
    pstr[0] = '\0';

    if (summary->dds_bXR)
        ol_strcpy(pstr, "XR");
    else if (summary->dds_bXD)
        ol_strcpy(pstr, "XD");
    else if (summary->dds_bDR)
        ol_strcpy(pstr, "DR");
    else if (summary->dds_bSt)
        ol_strcpy(pstr, "ST");
    else if (summary->dds_bStDelisting)
        ol_strcpy(pstr, "ST, Delisting");
}

quo_entry_t * getQuoEntryWithHighestPrice(
    quo_entry_t * start, quo_entry_t * end)
{
    quo_entry_t * highest = start;

    while (start <= end)
    {
        if (highest->qe_dbCurPrice < start->qe_dbCurPrice)
            highest = start;
        start ++;
    }

    return highest;
}

quo_entry_t * getQuoEntryWithLowestPrice(
    quo_entry_t * start, quo_entry_t * end)
{
    quo_entry_t * lowest = start;

    while (start <= end)
    {
        if (lowest->qe_dbCurPrice >= start->qe_dbCurPrice)
            lowest = start;
        start ++;
    }

    return lowest;
}

static olint_t getQuoEntryIdxWithHighestPrice(
    quo_entry_t ** ppq, olint_t start, olint_t end)
{
    olint_t highest = start;

    while (start <= end)
    {
        if (ppq[highest]->qe_dbCurPrice < ppq[start]->qe_dbCurPrice)
            highest = start;
        start ++;
    }

    return highest;
}

static olint_t getQuoEntryIdxWithLowestPrice(
    quo_entry_t ** ppq, olint_t start, olint_t end)
{
    olint_t lowest = start;

    while (start <= end)
    {
        if (ppq[lowest]->qe_dbCurPrice >= ppq[start]->qe_dbCurPrice)
            lowest = start;
        start ++;
    }

    return lowest;
}

static olint_t _getMaxDupQuoEntry(
    quo_entry_t ** ppq, olint_t maxp, olint_t idx)
{
    olint_t con = 0;
    boolean_t bUp;
    olint_t hour, min, sec, seconds1, seconds2;
    olint_t start, end, high, low;

    if (idx + 1 >= maxp)
        return con;

    start = idx;
    end = idx + 1;
    jf_time_getTimeFromString(ppq[idx]->qe_strTime, &hour, &min, &sec);
    seconds1 = jf_time_convertTimeToSeconds(hour, min, sec);
    while (end <= maxp - 2)
    {
        jf_time_getTimeFromString(ppq[end]->qe_strTime, &hour, &min, &sec);
        seconds2 = jf_time_convertTimeToSeconds(hour, min, sec);
        if (seconds2 - seconds1 > 60)
            break;
        end ++;
    }

    if (end <= idx + 1)
        return con;

    if (ppq[idx]->qe_dbCurPrice < ppq[idx + 1]->qe_dbCurPrice)
        bUp = TRUE;
    else
        bUp = FALSE;

    if (bUp)
    {
        low = getQuoEntryIdxWithLowestPrice(ppq, start + 1, end);
        if (ppq[low]->qe_dbCurPrice > ppq[start]->qe_dbCurPrice)
            high = getQuoEntryIdxWithHighestPrice(ppq, start + 1, end);
        else
            high = getQuoEntryIdxWithHighestPrice(ppq, start + 1, low);
        con = high - start - 1;
    }
    else
    {
        high = getQuoEntryIdxWithHighestPrice(ppq, start + 1, end);
        if (ppq[high]->qe_dbCurPrice <= ppq[start]->qe_dbCurPrice)
            low = getQuoEntryIdxWithLowestPrice(ppq, start + 1, end);
        else
            low = getQuoEntryIdxWithLowestPrice(ppq, start + 1, high);
        con = low - start - 1;
    }

    return con;
}

static void _removeQuoEntry(quo_entry_t ** ppq, olint_t * nump, olint_t start, olint_t count)
{
    olsize_t size;

    size = sizeof(quo_entry_t **) * (*nump - start - count);
    memmove(&ppq[start], &ppq[start + count], size);
    *nump = *nump - count;
}

static void _removeDupQuoEntry(
    quo_entry_t * buffer, olint_t num, quo_entry_t ** ppq, olint_t * nump)
{
    olint_t i, con;
    boolean_t bRemove;

    *nump = num;
    for (i = 0; i < *nump; i ++)
    {
        ppq[i] = &buffer[i];
    }

    do
    {
        bRemove = FALSE;
        for (i = 0; i < *nump; i ++)
        {
            con = _getMaxDupQuoEntry(ppq, *nump, i);
            if (con > 0)
            {
                _removeQuoEntry(ppq, nump, i + 1, con);

                bRemove = TRUE;
                break;
            }
        }
    } while (bRemove);

}

static olint_t _getMaxExtraQuoEntry(
    quo_entry_t ** ppq, olint_t maxp, olint_t idx)
{
    olint_t con = 0;
    boolean_t bUp;
    olint_t start, end;

    if (idx + 1 >= maxp)
        return con;

    if (ppq[idx]->qe_dbCurPrice < ppq[idx + 1]->qe_dbCurPrice)
        bUp = TRUE;
    else
        bUp = FALSE;

    start = idx;
    end = idx + 1;
    while (end <= maxp - 2)
    {
        if (bUp)
        {
            if (ppq[end + 1]->qe_dbCurPrice < ppq[end]->qe_dbCurPrice)
                break;
        }
        else
        {
            if (ppq[end + 1]->qe_dbCurPrice > ppq[end]->qe_dbCurPrice)
                break;
        }
        end ++;
    }
    con = end - start - 1;

    return con;
}

/*remove those entries between lowest and highest entry*/
static boolean_t _removeExtraQuoEntry1(
    quo_entry_t ** ppq, olint_t * nump)
{
    boolean_t bChange = FALSE, bRemove = FALSE;
    olint_t i, con;

    do
    {
        bRemove = FALSE;
        for (i = 0; i < *nump; i ++)
        {
            con = _getMaxExtraQuoEntry(ppq, *nump, i);
            if (con > 0)
            {
                _removeQuoEntry(ppq, nump, i + 1, con);

                bRemove = TRUE;
                bChange = TRUE;
                break;
            }
        }
    } while (bRemove);

    return bChange;
}

static oldouble_t _getMaxPriceRange(quo_entry_t ** ppq, olint_t num)
{
    olint_t i;
    oldouble_t db, dbmax = 0;

    for (i = 0; i < num - 1; i ++)
    {
        db = ABS(ppq[i + 1]->qe_dbCurPrice - ppq[i]->qe_dbCurPrice);
        if (dbmax < db)
            dbmax = db;
    }

    return dbmax;
}

static void _removeExtraQuoEntry(
    quo_entry_t * buffer, olint_t num, quo_entry_t ** ppq, olint_t * nump, oldouble_t thres)
{
    boolean_t bRemove = FALSE;
    olint_t i;
    oldouble_t dbp1, dbp2, dbmax;

    do
    {
        _removeExtraQuoEntry1(ppq, nump);

        /*remove those entry which didn't roll back larger than thres*/
        bRemove = FALSE;
        for (i = 0; i < *nump - 2; i ++)
        {
            dbmax = _getMaxPriceRange(ppq, *nump);
            dbp1 = ABS(ppq[i + 2]->qe_dbCurPrice - ppq[i + 1]->qe_dbCurPrice);
            dbp2 = ABS(ppq[i + 1]->qe_dbCurPrice - ppq[i]->qe_dbCurPrice);

            if ((dbp2 < dbp1) && (dbp2 < dbmax * thres))
            {
                _removeQuoEntry(ppq, nump, i, 1);

                bRemove = TRUE;
                break;
            }
            else if ((dbp1 < dbp2) && (dbp1 < dbmax * thres))
            {
                if (i + 3 == *nump)
                    *nump = *nump - 1;
                else
                    _removeQuoEntry(ppq, nump, i + 2, 1);

                bRemove = TRUE;
                break;
            }
        }

    } while (bRemove);

}

void getQuoEntryInflexionPoint(
    quo_entry_t * buffer, olint_t num, quo_entry_t ** ppq, olint_t * nump)
{
#define QUO_INFLEXION_THRES  0.2

    if (num < 2)
    {
        *nump = 0;
        return;
    }

    if (*nump < 2)
    {
        *nump = 0;
        return;
    }

    assert(*nump >= num);
    _removeDupQuoEntry(buffer, num, ppq, nump);
    _removeExtraQuoEntry(buffer, num, ppq, nump, QUO_INFLEXION_THRES);

    return;
}

olint_t getNextTopBottomQuoEntry(
    quo_entry_t ** pqe, olint_t total, olint_t start)
{
    boolean_t bUp;
    olint_t next, cur;
    oldouble_t db;

    if (start == total - 1)
        return start;
    if (start + 1 == total - 1)
        return start + 1;

    cur = start;
    next = cur + 1;

    if (pqe[cur]->qe_dbCurPrice <= pqe[next]->qe_dbCurPrice)
        bUp = TRUE;
    else
        bUp = FALSE;

    cur = next + 1;
    while (cur < total)
    {
        if (bUp)
        {
            if (pqe[cur]->qe_dbCurPrice > pqe[next]->qe_dbCurPrice)
                next = cur;
            db = pqe[start]->qe_dbCurPrice + (pqe[next]->qe_dbCurPrice - pqe[start]->qe_dbCurPrice) * 0.1;
            if (pqe[cur]->qe_dbCurPrice <= db)
                break;
        }
        else
        {
            if (pqe[cur]->qe_dbCurPrice <= pqe[next]->qe_dbCurPrice)
                next = cur;
            db = pqe[start]->qe_dbCurPrice - (pqe[start]->qe_dbCurPrice - pqe[next]->qe_dbCurPrice) * 0.1;
            if (pqe[cur]->qe_dbCurPrice > db)
                break;
        }

        cur ++;
    }

    return next;
}

u32 readStockQuotationFile(
    olchar_t * pstrDataDir, stock_quo_t * psq)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_dir_entry_t * entrylist = NULL, *entry;
    olint_t i, total = 300;
    olchar_t file[JF_LIMIT_MAX_PATH_LEN * 2];

    u32Ret = jf_jiukun_allocMemory(
        (void **)&entrylist, sizeof(jf_dir_entry_t) * total,
        JF_FLAG_MASK(JF_JIUKUN_MEM_ALLOC_FLAG_ZERO));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_dir_scan(
            pstrDataDir, entrylist, &total, _filterQuotationFileEntry, jf_dir_compareDirEntry);
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        if (entrylist != NULL)
            jf_jiukun_freeMemory((void **)&entrylist);
        return u32Ret;
    }

    if (total == 0)
        return JF_ERR_NOT_FOUND;

    entry = NULL;
    if (psq->sq_strDate[0] == '\0')
    {
        entry = entrylist + total - 1;
        ol_strncpy(psq->sq_strDate, entry->jde_strName + 10, 10);
    }
    else
    {
        for(i = 0; i < total; i ++)
        {
            if (strncmp(psq->sq_strDate, entrylist[i].jde_strName + 10, 10) == 0)
            {
                entry = &entrylist[i];
                break;
            }
        }
    }

    if (entry == NULL)
        return JF_ERR_NOT_FOUND;

    ol_snprintf(
        file, sizeof(file), "%s%c%s",
        pstrDataDir, PATH_SEPARATOR, entry->jde_strName);
    u32Ret = _parseQuotationFile(file, psq);

    jf_jiukun_freeMemory((void **)&entrylist);

    return u32Ret;
}

u32 parseSectorDir(
    olchar_t * pstrDir, parse_sector_info_t * sector, olint_t * numofsector)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tmp_sector_file_t tsf;
#define MAX_SECTOR_FILE_SIZE  (32 * 1024)

    ol_bzero(&tsf, sizeof(tsf));
    tsf.sector = sector;
    tsf.maxsector = *numofsector;
    tsf.sdata = MAX_SECTOR_FILE_SIZE;

    jf_jiukun_allocMemory((void **)&tsf.data, tsf.sdata, 0);

    u32Ret = jf_dir_parse(pstrDir, _handleSectorFile, &tsf);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *numofsector = tsf.numofsector;
    }

    jf_jiukun_freeMemory((void **)&tsf.data);

    return u32Ret;
}

void freeSectorInfo(parse_sector_info_t * sector)
{
    if (sector->psi_pstrStocks != NULL)
        jf_string_free(&sector->psi_pstrStocks);
}

/*------------------------------------------------------------------------------------------------*/


