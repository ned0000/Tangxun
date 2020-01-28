/**
 *  @file parsedata.c
 *
 *  @brief Routine for parsing data, the data from files.
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
#include "tx_quo.h"
#include "tx_sector.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

#if 0
static u32 _parseQuotationFileOneLine_OLD(
    olchar_t * buf, olsize_t sbuf, tx_quo_entry_t * cur)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t count;

    count = sscanf(
        buf,
        "%s\t%lf\t%llu\t%lf\t"
        "%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t"
        "%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t",
        cur->tqe_strTime,
        &cur->tqe_dbCurPrice,
        &cur->tqe_u64Volume, &cur->tqe_dbAmount,
        &cur->tqe_tqpBuy[0].tqp_dbPrice,
        &cur->tqe_tqpBuy[0].tqp_u64Volume,
        &cur->tqe_tqpBuy[1].tqp_dbPrice,
        &cur->tqe_tqpBuy[1].tqp_u64Volume,
        &cur->tqe_tqpBuy[2].tqp_dbPrice,
        &cur->tqe_tqpBuy[2].tqp_u64Volume,
        &cur->tqe_tqpBuy[3].tqp_dbPrice,
        &cur->tqe_tqpBuy[3].tqp_u64Volume,
        &cur->tqe_tqpBuy[4].tqp_dbPrice,
        &cur->tqe_tqpBuy[4].tqp_u64Volume,
        &cur->tqe_tqpSold[0].tqp_dbPrice,
        &cur->tqe_tqpSold[0].tqp_u64Volume,
        &cur->tqe_tqpSold[1].tqp_dbPrice,
        &cur->tqe_tqpSold[1].tqp_u64Volume,
        &cur->tqe_tqpSold[2].tqp_dbPrice,
        &cur->tqe_tqpSold[2].tqp_u64Volume,
        &cur->tqe_tqpSold[3].tqp_dbPrice,
        &cur->tqe_tqpSold[3].tqp_u64Volume,
        &cur->tqe_tqpSold[4].tqp_dbPrice,
        &cur->tqe_tqpSold[4].tqp_u64Volume);

    return u32Ret;
}

static u32 _parseQuotationFile_OLD(
    olchar_t * buf, olsize_t sbuf, tx_quo_entry_t * buffer, olint_t * numofresult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total;
    tx_quo_entry_t * cur;
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

static u32 _parseQuotationFileOneLine(
    olchar_t * line, olsize_t llen, stock_quo_t * psq, boolean_t bFirstLine)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_quo_entry_t * cur, * end = NULL;
    olint_t count;

//    assert(psq->sq_nNumOfEntry < psq->sq_nMaxEntry);

    if (psq->sq_nNumOfEntry > 0)
        end = &psq->sq_ptqeEntry[psq->sq_nNumOfEntry - 1];
    cur = &psq->sq_ptqeEntry[psq->sq_nNumOfEntry];

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        count = sscanf(
            line,
            "%s\t%lf\t%lf\t%lf\t%llu\t%lf\t"
            "%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t"
            "%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu\t%lf\t%llu",
            cur->tqe_strTime,
            &cur->tqe_dbCurPrice,
            &cur->tqe_dbHighPrice, &cur->tqe_dbLowPrice,
            &cur->tqe_u64Volume, &cur->tqe_dbAmount,
            &cur->tqe_tqpBuy[0].tqp_dbPrice,
            &cur->tqe_tqpBuy[0].tqp_u64Volume,
            &cur->tqe_tqpBuy[1].tqp_dbPrice,
            &cur->tqe_tqpBuy[1].tqp_u64Volume,
            &cur->tqe_tqpBuy[2].tqp_dbPrice,
            &cur->tqe_tqpBuy[2].tqp_u64Volume,
            &cur->tqe_tqpBuy[3].tqp_dbPrice,
            &cur->tqe_tqpBuy[3].tqp_u64Volume,
            &cur->tqe_tqpBuy[4].tqp_dbPrice,
            &cur->tqe_tqpBuy[4].tqp_u64Volume,
            &cur->tqe_tqpSold[0].tqp_dbPrice,
            &cur->tqe_tqpSold[0].tqp_u64Volume,
            &cur->tqe_tqpSold[1].tqp_dbPrice,
            &cur->tqe_tqpSold[1].tqp_u64Volume,
            &cur->tqe_tqpSold[2].tqp_dbPrice,
            &cur->tqe_tqpSold[2].tqp_u64Volume,
            &cur->tqe_tqpSold[3].tqp_dbPrice,
            &cur->tqe_tqpSold[3].tqp_u64Volume,
            &cur->tqe_tqpSold[4].tqp_dbPrice,
            &cur->tqe_tqpSold[4].tqp_u64Volume);

        if (count != 26)
            u32Ret = JF_ERR_INVALID_DATA;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (bFirstLine)
            psq->sq_dbOpeningPrice = cur->tqe_dbCurPrice;

        if ((end == NULL) ||
            (end->tqe_u64Volume != cur->tqe_u64Volume))
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

    u32Ret = jf_jiukun_allocMemory((void **)&data, MAX_DATAFILE_SIZE);
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

static olint_t getQuoEntryIdxWithHighestPrice(tx_quo_entry_t ** ppq, olint_t start, olint_t end)
{
    olint_t highest = start;

    while (start <= end)
    {
        if (ppq[highest]->tqe_dbCurPrice < ppq[start]->tqe_dbCurPrice)
            highest = start;
        start ++;
    }

    return highest;
}

static olint_t getQuoEntryIdxWithLowestPrice(tx_quo_entry_t ** ppq, olint_t start, olint_t end)
{
    olint_t lowest = start;

    while (start <= end)
    {
        if (ppq[lowest]->tqe_dbCurPrice >= ppq[start]->tqe_dbCurPrice)
            lowest = start;
        start ++;
    }

    return lowest;
}

static olint_t _getMaxDupQuoEntry(tx_quo_entry_t ** ppq, olint_t maxp, olint_t idx)
{
    olint_t con = 0;
    boolean_t bUp;
    olint_t hour, min, sec, seconds1, seconds2;
    olint_t start, end, high, low;

    if (idx + 1 >= maxp)
        return con;

    start = idx;
    end = idx + 1;
    jf_time_getTimeFromString(ppq[idx]->tqe_strTime, &hour, &min, &sec);
    seconds1 = jf_time_convertTimeToSeconds(hour, min, sec);
    while (end <= maxp - 2)
    {
        jf_time_getTimeFromString(ppq[end]->tqe_strTime, &hour, &min, &sec);
        seconds2 = jf_time_convertTimeToSeconds(hour, min, sec);
        if (seconds2 - seconds1 > 60)
            break;
        end ++;
    }

    if (end <= idx + 1)
        return con;

    if (ppq[idx]->tqe_dbCurPrice < ppq[idx + 1]->tqe_dbCurPrice)
        bUp = TRUE;
    else
        bUp = FALSE;

    if (bUp)
    {
        low = getQuoEntryIdxWithLowestPrice(ppq, start + 1, end);
        if (ppq[low]->tqe_dbCurPrice > ppq[start]->tqe_dbCurPrice)
            high = getQuoEntryIdxWithHighestPrice(ppq, start + 1, end);
        else
            high = getQuoEntryIdxWithHighestPrice(ppq, start + 1, low);
        con = high - start - 1;
    }
    else
    {
        high = getQuoEntryIdxWithHighestPrice(ppq, start + 1, end);
        if (ppq[high]->tqe_dbCurPrice <= ppq[start]->tqe_dbCurPrice)
            low = getQuoEntryIdxWithLowestPrice(ppq, start + 1, end);
        else
            low = getQuoEntryIdxWithLowestPrice(ppq, start + 1, high);
        con = low - start - 1;
    }

    return con;
}

static void _removeQuoEntry(tx_quo_entry_t ** ppq, olint_t * nump, olint_t start, olint_t count)
{
    olsize_t size;

    size = sizeof(tx_quo_entry_t **) * (*nump - start - count);
    memmove(&ppq[start], &ppq[start + count], size);
    *nump = *nump - count;
}

static void _removeDupQuoEntry(
    tx_quo_entry_t * buffer, olint_t num, tx_quo_entry_t ** ppq, olint_t * nump)
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
    tx_quo_entry_t ** ppq, olint_t maxp, olint_t idx)
{
    olint_t con = 0;
    boolean_t bUp;
    olint_t start, end;

    if (idx + 1 >= maxp)
        return con;

    if (ppq[idx]->tqe_dbCurPrice < ppq[idx + 1]->tqe_dbCurPrice)
        bUp = TRUE;
    else
        bUp = FALSE;

    start = idx;
    end = idx + 1;
    while (end <= maxp - 2)
    {
        if (bUp)
        {
            if (ppq[end + 1]->tqe_dbCurPrice < ppq[end]->tqe_dbCurPrice)
                break;
        }
        else
        {
            if (ppq[end + 1]->tqe_dbCurPrice > ppq[end]->tqe_dbCurPrice)
                break;
        }
        end ++;
    }
    con = end - start - 1;

    return con;
}

/*remove those entries between lowest and highest entry*/
static boolean_t _removeExtraQuoEntry1(tx_quo_entry_t ** ppq, olint_t * nump)
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

static oldouble_t _getMaxPriceRange(tx_quo_entry_t ** ppq, olint_t num)
{
    olint_t i;
    oldouble_t db, dbmax = 0;

    for (i = 0; i < num - 1; i ++)
    {
        db = ABS(ppq[i + 1]->tqe_dbCurPrice - ppq[i]->tqe_dbCurPrice);
        if (dbmax < db)
            dbmax = db;
    }

    return dbmax;
}

static void _removeExtraQuoEntry(
    tx_quo_entry_t * buffer, olint_t num, tx_quo_entry_t ** ppq, olint_t * nump, oldouble_t thres)
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
            dbp1 = ABS(ppq[i + 2]->tqe_dbCurPrice - ppq[i + 1]->tqe_dbCurPrice);
            dbp2 = ABS(ppq[i + 1]->tqe_dbCurPrice - ppq[i]->tqe_dbCurPrice);

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

/* --- public routine section ------------------------------------------------------------------- */

void getQuoEntryInflexionPoint(
    tx_quo_entry_t * buffer, olint_t num, tx_quo_entry_t ** ppq, olint_t * nump)
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

olint_t getNextTopBottomQuoEntry(tx_quo_entry_t ** ptqe, olint_t total, olint_t start)
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

    if (ptqe[cur]->tqe_dbCurPrice <= ptqe[next]->tqe_dbCurPrice)
        bUp = TRUE;
    else
        bUp = FALSE;

    cur = next + 1;
    while (cur < total)
    {
        if (bUp)
        {
            if (ptqe[cur]->tqe_dbCurPrice > ptqe[next]->tqe_dbCurPrice)
                next = cur;
            db = ptqe[start]->tqe_dbCurPrice + (ptqe[next]->tqe_dbCurPrice - ptqe[start]->tqe_dbCurPrice) * 0.1;
            if (ptqe[cur]->tqe_dbCurPrice <= db)
                break;
        }
        else
        {
            if (ptqe[cur]->tqe_dbCurPrice <= ptqe[next]->tqe_dbCurPrice)
                next = cur;
            db = ptqe[start]->tqe_dbCurPrice - (ptqe[start]->tqe_dbCurPrice - ptqe[next]->tqe_dbCurPrice) * 0.1;
            if (ptqe[cur]->tqe_dbCurPrice > db)
                break;
        }

        cur ++;
    }

    return next;
}

u32 readStockQuotationFile(olchar_t * pstrDataDir, stock_quo_t * psq)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_dir_entry_t * entrylist = NULL, *entry;
    olint_t i, total = 300;
    olchar_t file[JF_LIMIT_MAX_PATH_LEN * 2];

    u32Ret = jf_jiukun_allocMemory(
        (void **)&entrylist, sizeof(jf_dir_entry_t) * total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(entrylist, sizeof(jf_dir_entry_t) * total);

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

tx_quo_entry_t * getQuoEntryWithHighestPrice(tx_quo_entry_t * start, tx_quo_entry_t * end)
{
    tx_quo_entry_t * highest = start;

    while (start <= end)
    {
        if (highest->tqe_dbCurPrice < start->tqe_dbCurPrice)
            highest = start;
        start ++;
    }

    return highest;
}

tx_quo_entry_t * getQuoEntryWithLowestPrice(tx_quo_entry_t * start, tx_quo_entry_t * end)
{
    tx_quo_entry_t * lowest = start;

    while (start <= end)
    {
        if (lowest->tqe_dbCurPrice >= start->tqe_dbCurPrice)
            lowest = start;
        start ++;
    }

    return lowest;
}

/*------------------------------------------------------------------------------------------------*/


