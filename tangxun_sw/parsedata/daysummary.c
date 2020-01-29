/**
 *  @file daysummary.c
 *
 *  @brief Routine for parsing day summary from file.
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

#include "tx_daysummary.h"

/* --- private data/data structure section ------------------------------------------------------ */

static olchar_t * ls_pstrTradeSummaryFile = "Summary.csv";


/* --- private routine section ------------------------------------------------------------------ */

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

static u32 _calcOneTradeSummary(tx_ds_t * cur)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (cur->td_dbTurnoverRate == 0)
    {
        cur->td_dbTurnoverRate = cur->td_u64All * 100;
        cur->td_dbTurnoverRate /= cur->td_u64TradableShare;
    }

    if (cur->td_dbOpeningPrice > cur->td_dbClosingPrice)
    {
        cur->td_dbUpperShadowRatio =
            (cur->td_dbHighPrice - cur->td_dbOpeningPrice) /
            (cur->td_dbOpeningPrice - cur->td_dbClosingPrice);
        cur->td_dbLowerShadowRatio = 
            (cur->td_dbClosingPrice - cur->td_dbLowPrice) /
            (cur->td_dbOpeningPrice - cur->td_dbClosingPrice);
    }
    else if (cur->td_dbOpeningPrice < cur->td_dbClosingPrice)
    {
        cur->td_dbUpperShadowRatio =
            (cur->td_dbHighPrice - cur->td_dbClosingPrice) /
            (cur->td_dbClosingPrice - cur->td_dbOpeningPrice);
        cur->td_dbLowerShadowRatio =
            (cur->td_dbOpeningPrice - cur->td_dbLowPrice) /
            (cur->td_dbClosingPrice - cur->td_dbOpeningPrice);
    }
    else
    {
        if (cur->td_dbClosingPrice != cur->td_dbLowPrice)
            cur->td_dbUpperShadowRatio =
                (cur->td_dbHighPrice - cur->td_dbClosingPrice) /
                (cur->td_dbClosingPrice - cur->td_dbLowPrice);
        else
            cur->td_dbUpperShadowRatio =
                (cur->td_dbHighPrice - cur->td_dbClosingPrice) * 100; /* = / 0.01*/

        if (cur->td_dbHighPrice != cur->td_dbClosingPrice)
            cur->td_dbLowerShadowRatio =
                (cur->td_dbClosingPrice - cur->td_dbLowPrice) /
                (cur->td_dbHighPrice - cur->td_dbClosingPrice);
        else
            cur->td_dbLowerShadowRatio =
                (cur->td_dbClosingPrice - cur->td_dbLowPrice) * 100;
    }

    return u32Ret;
}

/* Date,Code,Name,ClosingPrice,HighPrice,LowPrice,OpeningPrice,LastClosingPrice,
   IncDec,IncDecPercent,TurnoverRate,Volumn,Amount,TotalMarketValue,TradableValue
 */
static u32 _parseTradeSummaryFileOneLine(
    olchar_t * buf, olsize_t sbuf, tx_ds_t * cur)
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
                    ol_strncpy(cur->td_strDate, field->jsprf_pstrData, 10);
                else
                    u32Ret = JF_ERR_INVALID_DATA;
            }
            else if (index == 3)
            {
                if (field->jsprf_pstrData[0] == 'X')
                {
                    if (field->jsprf_pstrData[1] == 'R')
                        cur->td_bXR = TRUE;
                    else if (field->jsprf_pstrData[1] == 'D')
                        cur->td_bXD = TRUE;
                }
                else if ((field->jsprf_pstrData[0] == 'D') && (field->jsprf_pstrData[1] == 'R'))
                    cur->td_bDR = TRUE;
                else if ((field->jsprf_pstrData[0] == 'S') && (field->jsprf_pstrData[1] == 'T'))
                    cur->td_bSt = TRUE;
                else if ((field->jsprf_pstrData[0] == '*') && (field->jsprf_pstrData[1] == 'S'))
                    cur->td_bStDelisting = TRUE;
            }
            else if (index == 4)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->td_dbClosingPrice);
                if ((u32Ret == JF_ERR_NO_ERROR) && (cur->td_dbClosingPrice == 0))
                {
                    u32Ret = JF_ERR_NOT_READY;
                    break;
                }
            }
            else if (index == 5)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->td_dbHighPrice);
            }
            else if (index == 6)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->td_dbLowPrice);
            }
            else if (index == 7)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->td_dbOpeningPrice);
            }
            else if (index == 8)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->td_dbLastClosingPrice);
            }
            else if (index == 10)
            {
                u32Ret = jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->td_dbClosingPriceRate);
            }
            else if (index == 11)
            {
                /*turnover rate string may be empty*/
                jf_string_getDoubleFromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->td_dbTurnoverRate);
            }
            else if (index == 12)
            {
                u32Ret = jf_string_getU64FromString(
                    field->jsprf_pstrData, field->jsprf_sData, &cur->td_u64All);
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
                    cur->td_u64AllA = (u64)dbamount;
                    cur->td_u64AllA /= TX_DR_AMOUNT_UNIT;
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
                    dbamount = dbtemp / cur->td_dbClosingPrice;
                    cur->td_u64GeneralCapital = (u64)dbamount;
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
                    dbamount = dbtemp / cur->td_dbClosingPrice;
                    cur->td_u64TradableShare = (u64)dbamount;
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
    tx_ds_t * buffer, olint_t num)
{
    tx_ds_t * result, * prev, * end, * start;
    oldouble_t dbt;
    olint_t i = 1;
    oldouble_t dbclose;

    result = prev = buffer;
    result->td_nIndex = i ++;
    end = buffer + num;
    result ++;
    while (result < end)
    {
        /*close high limit*/
        dbclose = round(prev->td_dbClosingPrice * 110) / 100;
        if (result->td_dbClosingPrice >= dbclose)
            result->td_bCloseHighLimit = TRUE;
        if (result->td_dbHighPrice >= dbclose)
            result->td_bHighHighLimit = TRUE;

        /*close low limit*/
        dbclose = round(prev->td_dbClosingPrice * 90) / 100;
        if (result->td_dbClosingPrice <= dbclose)
            result->td_bCloseLowLimit = TRUE; 
        if (result->td_dbLowPrice <= dbclose)
            result->td_bLowLowLimit = TRUE; 

        if (result->td_dbClosingPrice > prev->td_dbClosingPrice)
        {
            if (result->td_dbLowPrice > prev->td_dbHighPrice)
                result->td_bGap = TRUE;
        }
        else
        {            
            if (result->td_dbHighPrice < prev->td_dbLowPrice)
                result->td_bGap = TRUE;
        }
        /*closing price is read from summary file*/
//        result->td_dbClosingPriceRate =
//            (result->td_dbClosingPrice - prev->td_dbClosingPrice) * 100 /
//            prev->td_dbClosingPrice;
        result->td_dbHighPriceRate =
            (result->td_dbHighPrice - prev->td_dbClosingPrice) * 100 /
            prev->td_dbClosingPrice;

        if (result - buffer >= 5)
        {
            dbt = 0.0;
            start = result - 5;
            while (start < result)
            {
                dbt += start->td_u64All;
                start ++;
            }
            result->td_dbVolumeRatio = result->td_u64All * 5;
            result->td_dbVolumeRatio /= dbt;
        }
        result->td_nIndex = i ++;

        prev ++;
        result ++;
    }
}

static u32 _fillTradeDaySummary(
    olchar_t * buf, olsize_t sbuf, tx_ds_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, total;
    tx_ds_t * cur;
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
                    sizeof(tx_ds_t));
            }
        }

        _calcTradeDaySummary(buffer, total);
        *numofresult = total;
    }

    return u32Ret;
}

static void _restoreDaySummary(
    tx_ds_t * start, tx_ds_t * end,
    oldouble_t offset, oldouble_t rate)
{
    tx_ds_t * cur;

    cur = start;
    while (cur < end)
    {
        cur->td_dbOpeningPrice = (cur->td_dbOpeningPrice - offset) / rate;
        cur->td_dbClosingPrice = (cur->td_dbClosingPrice - offset) / rate;
        cur->td_dbHighPrice = (cur->td_dbHighPrice - offset) / rate;
        cur->td_dbLowPrice = (cur->td_dbLowPrice - offset) / rate;

        cur ++;
    }

}

static u32 _forwardRestorationOfRight(
    olchar_t * pstrDataDir, tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t rate;
    oldouble_t offset;
    olint_t i;
    tx_ds_t * cur, * prev;

    prev = buffer;
    cur = buffer + 1;
    for (i = 1; i < num; i ++)
    {
        if (prev->td_dbClosingPrice != cur->td_dbLastClosingPrice)
        {
            rate = (oldouble_t)cur->td_u64GeneralCapital / 100;
            rate /= prev->td_u64GeneralCapital / 100;

            offset = prev->td_dbClosingPrice - cur->td_dbLastClosingPrice * rate;

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
//        cur->td_dbClosingPriceRate =
//            (cur->td_dbClosingPrice - prev->td_dbClosingPrice) * 100 / prev->td_dbClosingPrice;

        cur->td_bGap = FALSE;
        if (cur->td_dbClosingPrice > prev->td_dbClosingPrice)
        {
            if (cur->td_dbLowPrice > prev->td_dbHighPrice)
                cur->td_bGap = TRUE;
        }
        else
        {            
            if (cur->td_dbHighPrice < prev->td_dbLowPrice)
                cur->td_bGap = TRUE;
        }

        prev ++;
        cur ++;
    }

    return u32Ret;
}

static u32 _fillTradeDaySummaryFromDate(
    olchar_t * buf, olsize_t sbuf, olchar_t * pstrDateFrom,
    tx_ds_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total;
    tx_ds_t * cur;
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

tx_ds_t * tx_ds_getDsWithHighestClosingPrice(tx_ds_t * buffer, olint_t num)
{
    olint_t i;
    tx_ds_t * result, * high = NULL;

    high = result = buffer + num - 1;
    for (i = 0; i < num; i ++)
    {
        if (high->td_dbClosingPrice < result->td_dbClosingPrice)
            high = result;

        result --;
    }

    return high;
}

tx_ds_t * tx_ds_getDsWithHighestHighPrice(tx_ds_t * buffer, olint_t num)
{
    olint_t i;
    tx_ds_t * result, * high = NULL;

    high = result = buffer + num - 1;
    for (i = 0; i < num; i ++)
    {
        if (high->td_dbHighPrice < result->td_dbHighPrice)
            high = result;

        result --;
    }

    return high;
}

/*return NULL if the date has no trade*/
u32 tx_ds_getDsWithDate(
    tx_ds_t * buffer, olint_t num, olchar_t * strdate, tx_ds_t ** ret)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    olint_t i;
    tx_ds_t * result;

    result = buffer + num - 1;
    for (i = 0; i < num; i ++)
    {
        if (ol_strncmp(result->td_strDate, strdate, 10) == 0)
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
tx_ds_t * tx_ds_getDsWithDate2(
    tx_ds_t * buffer, olint_t num, olchar_t * strdate)
{
    olint_t i;
    tx_ds_t * result;

    result = buffer;
    for (i = 0; i < num; i ++)
    {
        if (strncmp(result->td_strDate, strdate, 10) >= 0)
            return result;

        result ++;
    }

    return NULL;
}

/*return the prior day if the date has no trade*/
tx_ds_t * tx_ds_getDsWithDate3(
    tx_ds_t * buffer, olint_t num, olchar_t * strdate)
{
    olint_t i;
    tx_ds_t * result;

    result = buffer + num - 1;
    for (i = 0; i < num; i ++)
    {
        if (ol_strncmp(result->td_strDate, strdate, 10) <= 0)
            return result;

        result --;
    }

    return NULL;
}

tx_ds_t * tx_ds_getDsWithLowestClosingPrice(tx_ds_t * buffer, olint_t num)
{
    olint_t i;
    tx_ds_t * result, * low = NULL;

    low = result = buffer + num - 1;
    for (i = 0; i < num; i ++)
    {
        if (low->td_dbClosingPrice >= result->td_dbClosingPrice)
            low = result;

        result --;
    }

    return low;
}

tx_ds_t * tx_ds_getDsWithLowestLowPrice(tx_ds_t * buffer, olint_t num)
{
    olint_t i;
    tx_ds_t * result, * low = NULL;

    low = result = buffer + num - 1;
    for (i = 0; i < num; i ++)
    {
        if (low->td_dbLowPrice >= result->td_dbLowPrice)
            low = result;

        result --;
    }

    return low;
}

void tx_ds_getDsInflexionPoint(
    tx_ds_t * buffer, olint_t num, tx_ds_t ** ppp, olint_t * nump)
{
    tx_ds_t * prior, * next, * cur, * end;
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
        if ((cur->td_dbClosingPrice >= 
             prior->td_dbClosingPrice * (1 + TX_DS_STRAIGHT_LINE_MOTION)) ||
            (cur->td_dbClosingPrice <= 
             prior->td_dbClosingPrice * (1 - TX_DS_STRAIGHT_LINE_MOTION)))
            break;

        cur ++;
    }

    next = cur;
    if (cur->td_dbClosingPrice >= prior->td_dbClosingPrice)
        bUp = TRUE;
    else
        bUp = FALSE;
    cur ++;

    while (cur <= end)
    {
        if (bUp)
        {
            if (cur->td_dbClosingPrice >= next->td_dbClosingPrice)
            {
                next = cur;
            }
            else
            {
                dbp = next->td_dbClosingPrice - prior->td_dbClosingPrice;
                dbp2 = next->td_dbClosingPrice - cur->td_dbClosingPrice;

                if ((dbp2 > dbp * INFLEXION_THRES) &&
                    (dbp2 > next->td_dbClosingPrice * TX_DS_STRAIGHT_LINE_MOTION))
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
            if (cur->td_dbClosingPrice <= next->td_dbClosingPrice)
            {
                next = cur;
            }
            else
            {
                dbp = prior->td_dbClosingPrice - next->td_dbClosingPrice;
                dbp2 = cur->td_dbClosingPrice - next->td_dbClosingPrice;
                if ((dbp2 > dbp * INFLEXION_THRES) &&
                    (dbp2 > next->td_dbClosingPrice * TX_DS_STRAIGHT_LINE_MOTION))
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

void tx_ds_adjustDsInflexionPoint(
    tx_ds_t ** ppp, olint_t * nump, olint_t adjust)
{
    tx_ds_t * next;
    olint_t i;
    olsize_t size;

    for (i = 1; i < *nump; i ++)
    {
        if (ppp[i]->td_dbClosingPrice > ppp[i - 1]->td_dbClosingPrice)
        {
            if (i + 2 >= *nump)
                break;

            if ((ppp[i + 2]->td_dbClosingPrice > ppp[i]->td_dbClosingPrice) &&
                (ppp[i + 1]->td_dbClosingPrice > ppp[i - 1]->td_dbClosingPrice))
            {
                next = ppp[i] + adjust;
                if (next->td_dbClosingPrice > ppp[i]->td_dbClosingPrice)
                {
                    size = (*nump - i - 2) * sizeof(tx_ds_t *);
                    if (size != 0)
                        ol_memcpy(&ppp[i], &ppp[i + 2], size);
                    *nump -= 2;
                    i --;
                }
            }
        }
    }


}

void tx_ds_getDsInflexionPoint2(
    tx_ds_t * buffer, olint_t num, tx_ds_t ** ppp, olint_t * nump)
{
    tx_ds_t * prior, * next, * cur, * end;
    tx_ds_t * low, * start;
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
        if ((cur->td_dbClosingPrice >= prior->td_dbClosingPrice) &&
            ((cur == end) || (next->td_dbClosingPriceRate < 0)))
        {
            start = prior + 1;
            while (start <= cur)
            {
                if (start->td_dbClosingPriceRate < 0)
                {
                    /*find decreasing day*/
                    low = tx_ds_getDsWithLowestClosingPrice(prior, cur - prior + 1);
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
            (end->td_dbClosingPrice < prior->td_dbClosingPrice))
            ppp[count ++] = prior;
        if (count < *nump)
            ppp[count ++] = end;
    }

    *nump = count;
}

void tx_ds_locateInflexionPointRegion(
    tx_ds_t ** ppp, olint_t nump, tx_ds_t * dest,
    tx_ds_t ** start, tx_ds_t ** end)
{
    olint_t i;
    tx_ds_t * dastart, * daend;

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

tx_ds_t * tx_ds_getInflexionPointWithHighestClosingPrice(
    tx_ds_t ** ppp, olint_t nump)
{
    tx_ds_t * high;
    olint_t i;

    high = ppp[0];
    for (i = 0; i < nump; i ++)
    {
        if (high->td_dbClosingPrice < ppp[i]->td_dbClosingPrice)
            high = ppp[i];
    }

    return high;
}

u32 tx_ds_readDs(
    olchar_t * pstrDataDir, tx_ds_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    FILE * fp = NULL;
    olchar_t * buf = NULL;
    olsize_t sbuf = 1024 * 1024;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN];

    ol_snprintf(
        strFullname, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c%s",
        pstrDataDir, PATH_SEPARATOR, ls_pstrTradeSummaryFile);

    u32Ret = jf_jiukun_allocMemory((void **)&buf, sbuf);

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

u32 tx_ds_readDsWithFRoR(
    olchar_t * pstrDataDir, tx_ds_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = tx_ds_readDs(
        pstrDataDir, buffer, numofresult);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _forwardRestorationOfRight(pstrDataDir, buffer, *numofresult);

    return u32Ret;
}

u32 tx_ds_readDsFromDate(
    olchar_t * pstrDataDir, olchar_t * pstrStartDate, tx_ds_t * buffer, olint_t * numofresult)
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

    u32Ret = jf_jiukun_allocMemory((void **)&buf, sbuf);

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

u32 tx_ds_readDsFromDateWithFRoR(
    olchar_t * pstrDataDir, olchar_t * pstrStartDate, tx_ds_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = tx_ds_readDsFromDate(pstrDataDir, pstrStartDate, buffer, numofresult);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("read td from date %s, total %d days", pstrStartDate, *numofresult);
        u32Ret = _forwardRestorationOfRight(pstrDataDir, buffer, *numofresult);
    }

    return u32Ret;
}

u32 tx_ds_readDsUntilDate(
    olchar_t * pstrDataDir, olchar_t * pstrEndDate, u32 u32Count, tx_ds_t * buffer,
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

    u32Ret = jf_jiukun_allocMemory((void **)&buf, sbuf);

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

u32 tx_ds_readDsUntilDateWithFRoR(
    olchar_t * pstrDataDir, olchar_t * pstrEndDate, u32 u32Count, tx_ds_t * buffer,
    olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = tx_ds_readDsUntilDate(
        pstrDataDir, pstrEndDate, u32Count, buffer, numofresult);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("read td until date %s, total %d days", pstrEndDate, *numofresult);
        u32Ret = _forwardRestorationOfRight(pstrDataDir, buffer, *numofresult);
    }

    return u32Ret;
}

olint_t tx_ds_countDsWithEndData(tx_ds_t * buffer, olint_t num, olchar_t * pstrEndDate)
{
    olint_t total = num;
    tx_ds_t * end;

    if (num == 0)
        return num;

    end = buffer + num - 1;
    while (end >= buffer)
    {
        if (strcmp(end->td_strDate, pstrEndDate) == 0)
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

boolean_t tx_ds_isStraightLineMotion(
    tx_ds_t * buffer, olint_t total, olint_t ndays)
{
    boolean_t bRet = TRUE;
    olint_t i;
    tx_ds_t * end, * high;
    oldouble_t dblow;

    if (total < ndays)
        return FALSE;

    end = buffer + total - 1;
    high = tx_ds_getDsWithHighestClosingPrice(buffer + total - ndays, ndays);
    dblow = high->td_dbClosingPrice * (1 - TX_DS_STRAIGHT_LINE_MOTION);
    for (i = 0; i < ndays; i ++)
    {
        if (end->td_dbClosingPrice < dblow)
            break;
        end --;
    }
    if (i == ndays)
        return bRet;

    bRet = FALSE;

    return bRet;
}

boolean_t tx_ds_isStraightHighLimitDay(tx_ds_t * ptd)
{
    boolean_t bRet = FALSE;

    if (ptd->td_bCloseHighLimit &&
        (ptd->td_dbOpeningPrice == ptd->td_dbClosingPrice))
        bRet = TRUE;

    return bRet;
}

void tx_ds_getStringDsStatus(tx_ds_t * summary, olchar_t * pstr)
{
    pstr[0] = '\0';

    if (summary->td_bXR)
        ol_strcpy(pstr, "XR");
    else if (summary->td_bXD)
        ol_strcpy(pstr, "XD");
    else if (summary->td_bDR)
        ol_strcpy(pstr, "DR");
    else if (summary->td_bSt)
        ol_strcpy(pstr, "ST");
    else if (summary->td_bStDelisting)
        ol_strcpy(pstr, "ST, Delisting");
}

/*------------------------------------------------------------------------------------------------*/


