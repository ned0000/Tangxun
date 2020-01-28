/**
 *  @file analysis.c
 *
 *  @brief The analysis command implementation.
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
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <stdlib.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_jiukun.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "jf_time.h"
#include "jf_date.h"

#include "tx_daysummary.h"
#include "tx_quo.h"
#include "tx_sector.h"
#include "tx_env.h"
#include "tx_datastat.h"
#include "tx_statarbi.h"

#include "clicmd.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define MAX_NUM_OF_RESULT  500
#define MAX_ANALYSIS_FIND_DAY_SUMMARY  500
#define MAX_ANALYSIS_DAY_SUMMARY  100

#define NUM_OF_STAT_ARBI_DAY_SUMMARY 40

/* --- private routine section ------------------------------------------------------------------ */
static u32 _analysisHelp(tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Analysis stock \n\
analysis [-s stock] [-v] [-i stock] [-q stock] [-a] [-n] [-t]\n\
  -a: stat arbitrage.\n\
  -s: specify stock, based on the inflexion point.\n\
  -b: analysis the stocks reach high limit or low limit.\n\
  -n: analysis sector.\n\
  -t: analysis stock, which is tough.\n\
  -q: analysis quotation of stock, use '-f' to specify the date, use last day if\n\
      '-f' is not specified.\n\
  -i: analysis index and stocks, the region is specified by \"-f\" and \"-l\".");
    jf_clieng_outputRawLine2("\
  -v: verbose.");
    jf_clieng_outputRawLine2("\
  -f: the first date with format \"yyyy-mm-dd\".\n\
  -l: the last date with format \"yyyy-mm-dd\".");
    jf_clieng_outputLine("");

    return u32Ret;
}

static oldouble_t _getClosingPriceInc(
    tx_ds_t * start, tx_ds_t * end)
{
    oldouble_t dbret = -9999.99;

    dbret = (end->td_dbClosingPrice - start->td_dbClosingPrice) * 100 /
        start->td_dbClosingPrice;

    return dbret;
}

static void _getStartEndDaySummary(
    tx_ds_t * buffer, olint_t num, olchar_t * startdate, olchar_t * enddate,
    tx_ds_t ** ppStart, tx_ds_t ** ppEnd)
{
    tx_ds_t * start, * end;

    start = buffer;
    end = buffer + num - 1;

    if (strncmp(end->td_strDate, startdate, 10) < 0)
    {
        *ppStart = *ppEnd = NULL;
        return;
    }

    while (start < end)
    {
        if (strncmp(start->td_strDate, startdate, 10) >= 0)
            break;

        start ++;
    }

    while (end > start)
    {
        if (strncmp(end->td_strDate, enddate, 10) <= 0)
            break;

        end --;
    }

    *ppStart = start;
    *ppEnd = end;
}

static boolean_t _getStockIndexClosingPriceInc(
    cli_analysis_param_t * pcap, tx_stock_info_t * stockinfo, olchar_t * lowdate, oldouble_t * dbInc)
{
    boolean_t bTough = FALSE;
    oldouble_t dbret = -9999.99;
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * strindex = NULL;
    tx_ds_t * buffer = NULL;
    olint_t total = MAX_NUM_OF_RESULT;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_ds_t * start, * end, * high, * low;

    if (tx_stock_isShStockExchange(stockinfo->tsi_strCode))
        strindex = TX_STOCK_SH_COMPOSITE_INDEX;
    else
        strindex = TX_STOCK_SZ_COMPOSITIONAL_INDEX;

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, strindex);

    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);

    u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _getStartEndDaySummary(
            buffer, total, pcap->cap_pstrStartDate, pcap->cap_pstrEndDate, &start, &end);

        if (start == NULL)
            dbret = 0.0;
        else
        {
            low = tx_ds_getDsWithLowestClosingPrice(start, end - start + 1);
            high = tx_ds_getDsWithHighestClosingPrice(low, end - low + 1);
            dbret = _getClosingPriceInc(low, high);
        }

        ol_printf(
            "Index %s, from %s to %s, %.2f inc\n",
            strindex, low->td_strDate, high->td_strDate, dbret);

        if (strncmp(lowdate, low->td_strDate, 10) <= 0)
            bTough = TRUE;
    }

    jf_jiukun_freeMemory((void **)&buffer);

    *dbInc = dbret;

    return bTough;
}

static u32 _analysisDaySummary_OLD(
    cli_analysis_param_t * pcap, tx_stock_info_t * stockinfo,
    tx_ds_t * buffer, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t inc1, inc2;
    tx_ds_t * start, * end, * high, * low;
    boolean_t bTough = FALSE;

    start = buffer;
    end = buffer + num - 1;

    if (pcap->cap_pstrStartDate == NULL)
        pcap->cap_pstrStartDate = start->td_strDate;
    if (pcap->cap_pstrEndDate == NULL)
        pcap->cap_pstrEndDate = end->td_strDate;

    _getStartEndDaySummary(
        buffer, num, pcap->cap_pstrStartDate, pcap->cap_pstrEndDate, &start, &end);

    if (start == NULL)
        inc1 = 0.0;
    else
    {
        low = tx_ds_getDsWithLowestClosingPrice(start, end - start + 1);
        high = tx_ds_getDsWithHighestClosingPrice(low, end - low + 1);
        inc1 = _getClosingPriceInc(low, high);
    }

    ol_printf(
        "Stock %s, from %s to %s, %.2f inc\n",
        stockinfo->tsi_strCode, low->td_strDate, high->td_strDate, inc1);

    bTough = _getStockIndexClosingPriceInc(pcap, stockinfo, low->td_strDate, &inc2);

    if (bTough && (inc1 <= inc2))
            bTough = FALSE;

    if (bTough)
        ol_printf(
            "Stock %s(%.2f) is Tougher than index(%.2f)\n\n", stockinfo->tsi_strCode, inc1, inc2);
    else
        ol_printf(
            "Stock %s(%.2f) is Weaker than index(%.2f)\n\n", stockinfo->tsi_strCode, inc1, inc2);

    return u32Ret;
}

static boolean_t _analysisDaySummaryRegion(
    tx_ds_t * buffer, olint_t total,
    tx_ds_t ** fp, olint_t nFp,
    tx_ds_t * idxStart, tx_ds_t * idxEnd)
{
    boolean_t bTough = FALSE;
    oldouble_t inc1, inc2;
    tx_ds_t * start1, * end1, * first, * last;

    inc1 = _getClosingPriceInc(idxStart, idxEnd);

    tx_ds_getDsWithDate(buffer, total, idxStart->td_strDate, &first);
    tx_ds_getDsWithDate(buffer, total, idxEnd->td_strDate, &last);
    if ((first == NULL) || (last == NULL))
    {
#if 0
        jf_clieng_outputLine(
            "Index from %s to %s, %.2f inc", idxStart->td_strDate, idxEnd->td_strDate, inc1);
        jf_clieng_outputLine(
            "Stock from %s to %s, no trade in the first day or the last day",
            idxStart->td_strDate, idxEnd->td_strDate);
#endif
        return bTough;
    }

    if (inc1 <= 0)
    {
        inc2 = _getClosingPriceInc(first, last);
        if (inc2 > 0)
            bTough = TRUE;
    }
    else
    {
        tx_ds_locateInflexionPointRegion(fp, nFp, first, &start1, &end1);
        if (end1->td_dbClosingPrice > start1->td_dbClosingPrice)
            first = start1 - 1;

        inc2 = _getClosingPriceInc(first, last);
        if (inc1 < inc2)
            bTough = TRUE;
    }
#if 0
    jf_clieng_outputLine(
        "Index from %s to %s, %.2f inc", idxStart->td_strDate, idxEnd->td_strDate, inc1);
    jf_clieng_outputLine(
        "Stock from %s to %s, %.2f inc", first->td_strDate, last->td_strDate, inc2);
#endif
    return bTough;
}

static u32 _analysisDaySummary(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, olint_t total,
    tx_ds_t * idxBuf, olint_t idxTotal, olint_t nCount, boolean_t * pbTough)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bTough = FALSE;
#define MAX_INF_COUNT  60
    tx_ds_t * fp[MAX_INF_COUNT];
    olint_t nFp = MAX_INF_COUNT;
    tx_ds_t * idxFp[MAX_INF_COUNT];
    olint_t nIdxFp = MAX_INF_COUNT;
    olint_t i;
    tx_ds_t * end, * last;

    *pbTough = FALSE;
    end = buffer + total - 1;
    last = idxBuf + idxTotal - 1;
    if (strncmp(end->td_strDate, last->td_strDate, 10) != 0)
    {
//        jf_clieng_outputLine("Stock has no trade in the last day");
        return u32Ret;
    }

    tx_ds_getDsInflexionPoint(buffer, total, fp, &nFp);
    tx_ds_getDsInflexionPoint(idxBuf, idxTotal, idxFp, &nIdxFp);

    i = nIdxFp - nCount - 2;
    while (nCount > 0)
    {
        bTough = _analysisDaySummaryRegion(
            buffer, total, fp, nFp, idxFp[i + nCount], idxFp[i + nCount + 1]);
        if (! bTough)
            break;

        nCount --;
    }
#if 0
    if (bTough)
        jf_clieng_outputLine("Stock %s is Tougher than index", stockinfo->tsi_strCode);
    else
        jf_clieng_outputLine("Stock %s is Weaker than index", stockinfo->tsi_strCode);
#endif
    *pbTough = bTough;

    return u32Ret;
}

static u32 _getVolumePair(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, olint_t total,
    tx_ds_t * idxBuf, olint_t idxTotal,
    oldouble_t * pdba, oldouble_t * pdbb, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * cura, * enda, * curb, * endb;
    olint_t ret, count = 0, index = num - 1, mismatch = 0;

    cura = buffer;
    enda = cura + total - 1;
    curb = idxBuf;
    endb = curb + idxTotal - 1;
    while ((enda >= cura) && (endb >= curb))
    {
        ret = strcmp(enda->td_strDate, endb->td_strDate);
        if (ret == 0)
        {
            pdba[index] = (oldouble_t)enda->td_u64All;
            pdbb[index] = (oldouble_t)endb->td_u64All;
            count ++;
            index --;
            enda --;
            endb --;
        }
        else if (ret < 0)
        {
            endb --;
            mismatch ++;
        }
        else
        {
            enda --;
            mismatch ++;
        }

        if (index < 0)
            break;
    }

    if (count != num)
        u32Ret = JF_ERR_NOT_READY;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (mismatch >= num * 3 / 4)
            u32Ret = JF_ERR_NOT_READY;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    return u32Ret;
}

static u32 _analysisStock(cli_analysis_param_t * pcap, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_stock_info_t * stockinfo;
    tx_ds_t * buffer = NULL;
    olint_t total = MAX_ANALYSIS_DAY_SUMMARY;
    tx_ds_t * shiBuf = NULL;
    olint_t shiTotal = MAX_ANALYSIS_DAY_SUMMARY;
    tx_ds_t * sziBuf = NULL;
    olint_t sziTotal = MAX_ANALYSIS_DAY_SUMMARY;
    tx_ds_t * idxBuf = NULL;
    olint_t idxTotal;
    olint_t nCount = 20;
    oldouble_t * pdba = NULL, * pdbb = NULL, dbr;

    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);
    jf_jiukun_allocMemory((void **)&shiBuf, sizeof(tx_ds_t) * shiTotal);
    jf_jiukun_allocMemory((void **)&sziBuf, sizeof(tx_ds_t) * sziTotal);
    jf_jiukun_allocMemory((void **)&pdba, sizeof(oldouble_t) * nCount);
    jf_jiukun_allocMemory((void **)&pdbb, sizeof(oldouble_t) * nCount);

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, TX_STOCK_SH_COMPOSITE_INDEX);
    u32Ret = tx_ds_readDsWithFRoR(dirpath, shiBuf, &shiTotal);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, TX_STOCK_SZ_COMPOSITIONAL_INDEX);
        u32Ret = tx_ds_readDsWithFRoR(dirpath, sziBuf, &sziTotal);
    }

    for (i = 0; (i < pcap->cap_nDir) && (u32Ret == JF_ERR_NO_ERROR); i ++)
    {
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = tx_stock_getStockInfo(pcap->cap_pstrDir[i], &stockinfo);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_snprintf(
                dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
                tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcap->cap_pstrDir[i]);
            total = MAX_ANALYSIS_DAY_SUMMARY;
            u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (tx_stock_isShStockExchange(stockinfo->tsi_strCode))
            {
                idxBuf = shiBuf;
                idxTotal = shiTotal;
            }
            else
            {
                idxBuf = sziBuf;
                idxTotal = sziTotal;
            }

            u32Ret = _getVolumePair(
                stockinfo, buffer, total, idxBuf, idxTotal, pdba, pdbb, nCount);
            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = tx_datastat_getCorrelation(pdba, pdbb, nCount, &dbr);

            if (u32Ret == JF_ERR_NO_ERROR)
                jf_clieng_outputLine("Correlation: %.2f", dbr);
        }
    }

    jf_jiukun_freeMemory((void **)&buffer);
    jf_jiukun_freeMemory((void **)&shiBuf);
    jf_jiukun_freeMemory((void **)&sziBuf);
    jf_jiukun_freeMemory((void **)&pdba);
    jf_jiukun_freeMemory((void **)&pdbb);

    return u32Ret;
}

static olint_t _analysisCompareStatArbiEntry(const void * a, const void * b)
{
    tx_statarbi_eval_result_entry_t * ra, * rb;
    olint_t ret = 0;

    ra = (tx_statarbi_eval_result_entry_t *)a;
    rb = (tx_statarbi_eval_result_entry_t *)b;

    if (ra->tsere_dbCorrelation > rb->tsere_dbCorrelation)
        ret = -1;
    else if (ra->tsere_dbCorrelation < rb->tsere_dbCorrelation)
        ret = 1;

    return ret;
}

static u32 _analysisStockStatArbi(
    cli_analysis_param_t * pcap, tx_statarbi_eval_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_statarbi_eval_param_t tsep;

    ol_memset(&tsep, 0, sizeof(tx_statarbi_eval_param_t));
    tsep.tsep_dbMinCorrelation = 0.98;
    tsep.tsep_nDaySummary = NUM_OF_STAT_ARBI_DAY_SUMMARY + 10;
    tsep.tsep_nCorrelationArray = NUM_OF_STAT_ARBI_DAY_SUMMARY;

    u32Ret = tx_statarbi_evalAllIndu(tx_env_getVar(TX_ENV_VAR_DATA_PATH), &tsep, result);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_qsort(
            result->tser_ptsereEntry, result->tser_nNumOfPair,
            sizeof(tx_statarbi_eval_result_entry_t), _analysisCompareStatArbiEntry);
    }

    return u32Ret;
}

static oldouble_t _getStockVolatility(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, olint_t total, olint_t count)
{
    oldouble_t dbvol = 0.0;
    tx_ds_t * high, * low;

    if (total < count)
        return dbvol;

    buffer = buffer + total - count;
    total = count;

    low = tx_ds_getDsWithLowestClosingPrice(buffer, total);
    high = tx_ds_getDsWithHighestClosingPrice(low, buffer + total - low);
    dbvol = (high->td_dbClosingPrice - low->td_dbClosingPrice) * 100 /
        low->td_dbClosingPrice;

#if 0
    jf_clieng_outputLine(
        "%s, volatility %.2f", stockinfo->tsi_strCode, dbvol);
#endif
    return dbvol;
}

static olint_t _analysisCompareStockValue(const void * a, const void * b)
{
    tx_stock_info_t * ra, * rb;
    olint_t ret = 0;

    ra = *((tx_stock_info_t **)a);
    rb = *((tx_stock_info_t **)b);

    if (ra->tsi_dbValue > rb->tsi_dbValue)
        ret = -1;
    else if (ra->tsi_dbValue < rb->tsi_dbValue)
        ret = 1;

    return ret;
}

static u32 _analysisStockGraph(
    tx_stock_info_t * stockinfo, tx_ds_t * buffer, olint_t total,
    boolean_t * pbTough, olint_t count)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * end, * high, * start;
    olint_t year, mon, day, days;
    olint_t year2, mon2, day2, days2;
#define MAX_INF_COUNT  60
    tx_ds_t * fp[MAX_INF_COUNT];
    olint_t nFp = MAX_INF_COUNT;
    olint_t nCount = 1, i;

    *pbTough = TRUE;

    end = buffer + total - 1;
    if (end->td_bSt)
    {
        *pbTough = FALSE;
        return u32Ret;
    }

    end = buffer + total - 1;
    start = buffer + total - NUM_OF_STAT_ARBI_DAY_SUMMARY;

    while (end > start)
    {
        jf_date_getDate2FromString(end->td_strDate, &year, &mon, &day);
        days = jf_date_convertDateToDaysFrom1970(year, mon, day);

        jf_date_getDate2FromString((end - 1)->td_strDate, &year2, &mon2, &day2);
        days2 = jf_date_convertDateToDaysFrom1970(year2, mon2, day2);

        if (days - days2 > 30)
        {
            *pbTough = FALSE;
            return u32Ret;
        }

        end --;
    }

    end = buffer + total - 1;
    start = buffer + total - count;

    high = tx_ds_getDsWithHighestClosingPrice(start, count);
    if ((high != end) /*&& (high != end - 1)*/)
    {
        *pbTough = FALSE;
        return u32Ret;
    }

    tx_ds_getDsInflexionPoint(buffer, total, fp, &nFp);
    i = nFp - nCount - 2;
    while (nCount > 0)
    {
        end = fp[i + nCount + 1];
        start = fp[i + nCount];

        while (end > start)
        {
            if (end->td_bLowLowLimit)
            {
                *pbTough = FALSE;
                return u32Ret;
            }
            end --;
        }

        nCount --;
    }

    return u32Ret;
}

static u32 _analysisStockToughness(
    cli_analysis_param_t * pcap, tx_stock_info_t ** ptsiStockInfo, olint_t * pnStockInfo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_stock_info_t * stockinfo;
    tx_ds_t * buffer = NULL;
    olint_t total = MAX_ANALYSIS_DAY_SUMMARY;
    tx_ds_t * shiBuf = NULL;
    olint_t shiTotal = MAX_ANALYSIS_DAY_SUMMARY;
    tx_ds_t * sziBuf = NULL;
    olint_t sziTotal = MAX_ANALYSIS_DAY_SUMMARY;
    tx_ds_t * idxBuf;
    olint_t idxTotal;
    boolean_t bTough = FALSE;
    olint_t nStockInfo = 0;
    tx_stock_info_t ** ptsiStock = NULL;
    olint_t nStock = 0, i;
#define VOLLTILITY_DAY_SUMMARY_COUNT  20

//    *pnStockInfo = 0;
//    return u32Ret;

    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);
    jf_jiukun_allocMemory((void **)&shiBuf, sizeof(tx_ds_t) * shiTotal);
    jf_jiukun_allocMemory((void **)&sziBuf, sizeof(tx_ds_t) * sziTotal);
    jf_jiukun_allocMemory(
        (void **)&ptsiStock, sizeof(tx_stock_info_t *) * tx_stock_getNumOfStock());

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, TX_STOCK_SH_COMPOSITE_INDEX);
    u32Ret = tx_ds_readDsWithFRoR(dirpath, shiBuf, &shiTotal);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, TX_STOCK_SZ_COMPOSITIONAL_INDEX);
        u32Ret = tx_ds_readDsWithFRoR(dirpath, sziBuf, &sziTotal);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        stockinfo = tx_stock_getFirstStockInfo();
        while (stockinfo != NULL)
        {
            ol_snprintf(
                dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s", tx_env_getVar(TX_ENV_VAR_DATA_PATH),
                PATH_SEPARATOR, stockinfo->tsi_strCode);
            total = MAX_ANALYSIS_DAY_SUMMARY;
            u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                stockinfo->tsi_dbValue = _getStockVolatility(
                    stockinfo, buffer, total, VOLLTILITY_DAY_SUMMARY_COUNT);
                ptsiStock[nStock] = stockinfo;
                nStock ++;
            }

            stockinfo = tx_stock_getNextStockInfo(stockinfo);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        qsort(ptsiStock, nStock, sizeof(tx_stock_info_t *), _analysisCompareStockValue);

        for (i = 0; i < nStock; i ++)
        {
            bTough = FALSE;
            ol_snprintf(
                dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
                tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, ptsiStock[i]->tsi_strCode);
            total = MAX_ANALYSIS_DAY_SUMMARY;
            u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                if (tx_stock_isShStockExchange(ptsiStock[i]->tsi_strCode))
                {
                    idxBuf = shiBuf;
                    idxTotal = shiTotal;
                }
                else
                {
                    idxBuf = sziBuf;
                    idxTotal = sziTotal;
                }

                u32Ret = _analysisDaySummary(
                    ptsiStock[i], buffer, total, idxBuf, idxTotal, 1, &bTough);
            }

            if ((u32Ret == JF_ERR_NO_ERROR) && bTough)
            {
                u32Ret = _analysisStockGraph(
                    ptsiStock[i], buffer, total, &bTough,
                    VOLLTILITY_DAY_SUMMARY_COUNT);
            }

            /*continue if bTouch is true*/
            if ((u32Ret == JF_ERR_NO_ERROR) && bTough)
            {
                ptsiStockInfo[nStockInfo] = ptsiStock[i];
                nStockInfo ++;
                if (nStockInfo == *pnStockInfo)
                    break;
            }
        }

        *pnStockInfo = nStockInfo;
    }

    jf_jiukun_freeMemory((void **)&buffer);
    jf_jiukun_freeMemory((void **)&shiBuf);
    jf_jiukun_freeMemory((void **)&sziBuf);
    jf_jiukun_freeMemory((void **)&ptsiStock);

    return u32Ret;
}

static boolean_t _findInStockStatArbi(
    tx_stock_info_t * stockinfo, tx_statarbi_eval_result_t * result,
    tx_statarbi_eval_result_entry_t ** ppEntry)
{
    olint_t i;
    tx_statarbi_eval_result_entry_t * entry;

    for (i = 0; i < result->tser_nNumOfPair; i ++)
    {
        entry = &result->tser_ptsereEntry[i];
        if (jf_string_locateSubString(entry->tsere_strStockPair, stockinfo->tsi_strCode, NULL) ==
            JF_ERR_NO_ERROR)
        {
            *ppEntry = entry;
            return TRUE;
        }
    }

    return FALSE;
}

static boolean_t _isDuplicateEntry(
    tx_statarbi_eval_result_entry_t * entry, tx_statarbi_eval_result_entry_t * pEntry[], olint_t nEntry)
{
    olint_t i;
    olchar_t code1[16], code2[16];

    ol_memset(code1, 0, sizeof(code1));
    ol_memset(code2, 0, sizeof(code2));

    ol_strncpy(code1, entry->tsere_strStockPair, 8);
    ol_strncpy(code2, entry->tsere_strStockPair + 9, 8);

    for (i = 0; i < nEntry; i ++)
    {
        if (jf_string_locateSubString(
                pEntry[i]->tsere_strStockPair, code1, NULL) == JF_ERR_NO_ERROR)
            return TRUE;
        if (jf_string_locateSubString(
                pEntry[i]->tsere_strStockPair, code2, NULL) == JF_ERR_NO_ERROR)
            return TRUE;
    }

    return FALSE;
}

static u32 _analysisStockPool(cli_analysis_param_t * pcap)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_info_t ** ptsiStockInfo = NULL;
    olint_t nStockInfo = 0, i;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    tx_statarbi_eval_result_t result;
    tx_statarbi_eval_result_entry_t * entry;
    tx_stock_indu_info_t * induinfo;
#define MAX_ARBI_INDU_ENTRY 5
    olint_t nEntry = 0;
    tx_statarbi_eval_result_entry_t * pEntry[MAX_ARBI_INDU_ENTRY];
#define MAX_STOCK_PAIR  2000

    nStockInfo = 100; //getNumOfStock();
    jf_jiukun_allocMemory((void **)&ptsiStockInfo, sizeof(tx_stock_info_t *) * nStockInfo);

    ol_memset(&result, 0, sizeof(tx_statarbi_eval_result_t));
    result.tser_nMaxPair = MAX_STOCK_PAIR;
    jf_jiukun_allocMemory(
        (void **)&result.tser_ptsereEntry,
        sizeof(tx_statarbi_eval_result_entry_t) * result.tser_nMaxPair);

    u32Ret = _analysisStockToughness(pcap, ptsiStockInfo, &nStockInfo);
    if ((u32Ret == JF_ERR_NO_ERROR) && (nStockInfo > 0))
    {
        for (i = 0; i < nStockInfo; i ++)
            jf_clieng_outputLine("%s", ptsiStockInfo[i]->tsi_strCode);
        jf_clieng_outputLine("Found %d stocks\n", nStockInfo);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _analysisStockStatArbi(pcap, &result);
    if ((u32Ret == JF_ERR_NO_ERROR) && (result.tser_nNumOfPair > 0))
    {
        for (i = 0; i < result.tser_nNumOfPair; i ++)
        {
            entry = &result.tser_ptsereEntry[i];
            tx_stock_getInduInfo(entry->tsere_nInduId, &induinfo);
            jf_clieng_outputLine(
                "%s, %.2f, %s", entry->tsere_strStockPair, entry->tsere_dbCorrelation,
                induinfo->tsii_pstrDesc);
        }
        jf_clieng_outputLine("Found %d pairs stock", result.tser_nNumOfPair);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (nStockInfo > 0))
    {
        for (i = 0; i < nStockInfo; i ++)
        {
            if (_findInStockStatArbi(ptsiStockInfo[i], &result, &entry) &&
                ! _isDuplicateEntry(entry, pEntry, nEntry))
            {
                tx_stock_getInduInfo(entry->tsere_nInduId, &induinfo);
                jf_clieng_outputLine(
                    "%s, %.2f, %s, %s", entry->tsere_strStockPair, entry->tsere_dbCorrelation,
                    induinfo->tsii_pstrDesc, ptsiStockInfo[i]->tsi_strCode);
                pEntry[nEntry] = entry;
                nEntry ++;
                if (nEntry == MAX_ARBI_INDU_ENTRY)
                    break;
            }
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_file_openWithMode(
            TX_STOCK_QUOTATION_LIST_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC,
            JF_FILE_DEFAULT_CREATE_MODE, &fd);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            for (i = 0; i < nEntry; i ++)
            {
                jf_file_writen(fd, pEntry[i]->tsere_strStockPair, 17);
                jf_file_writen(fd, "\n", 1);
            }

            jf_file_close(&fd);
        }
    }

    jf_jiukun_freeMemory((void **)&result.tser_ptsereEntry);
    jf_jiukun_freeMemory((void **)&ptsiStockInfo);

    jf_clieng_outputLine("");

    return u32Ret;
}

static oldouble_t _getIndexClosingPriceInc(
    cli_analysis_param_t * pcap, olchar_t * strstock)
{
    oldouble_t dbret = -9999.99;
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * buffer = NULL;
    olint_t total = MAX_NUM_OF_RESULT;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_ds_t * start, * end;

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, strstock);

    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);

    u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _getStartEndDaySummary(
            buffer, total, pcap->cap_pstrStartDate, pcap->cap_pstrEndDate, &start, &end);
        if (start == NULL)
            dbret = 0.0;
        else
            dbret = _getClosingPriceInc(start, end);
    }

    jf_jiukun_freeMemory((void **)&buffer);

    return dbret;
}

static oldouble_t _getStockClosingPriceInc(
    cli_analysis_param_t * pcap, tx_ds_t * buffer, olint_t total)
{
    oldouble_t dbret = -9999.99;
    tx_ds_t * start, * end;

    _getStartEndDaySummary(
        buffer, total, pcap->cap_pstrStartDate, pcap->cap_pstrEndDate, &start, &end);
    if (start == NULL)
        dbret = 0.0;
    else
        dbret = _getClosingPriceInc(start, end);

    return dbret;
}

static boolean_t _isStockHighlimit(
    cli_analysis_param_t * pcap, tx_ds_t * buffer, olint_t total)
{
    boolean_t bRet = FALSE;
    tx_ds_t * start, * end;

    _getStartEndDaySummary(
        buffer, total, pcap->cap_pstrStartDate, pcap->cap_pstrEndDate, &start, &end);
    if (start != NULL)
    {
        start = end;
        end = buffer + total - 1;
        while (start <= end)
        {
            if (start->td_bHighHighLimit)
                return TRUE;

            start ++;
        }
    }

    return bRet;
}

static u32 _analysisIndexStock1(cli_analysis_param_t * pcap)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_info_t * stockinfo;
    oldouble_t shIndexInc, szIndexInc, stockInc;
    olint_t aboveSh, belowSh, aboveSz, belowSz;
    olint_t aboveShHl, belowShHl, aboveSzHl, belowSzHl;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_ds_t * buffer = NULL;
    olint_t total = MAX_NUM_OF_RESULT;
    oldouble_t thres = 1.3;



    if ((pcap->cap_pstrStartDate == NULL) || (pcap->cap_pstrEndDate == NULL))
        return JF_ERR_MISSING_PARAM;

    shIndexInc = _getIndexClosingPriceInc(pcap, TX_STOCK_SH_COMPOSITE_INDEX);
    szIndexInc = _getIndexClosingPriceInc(pcap, TX_STOCK_SZ_COMPOSITIONAL_INDEX);

    aboveSh = belowSh = aboveSz = belowSz = 0;
    aboveShHl = belowShHl = aboveSzHl = belowSzHl = 0;
    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);

    stockinfo = tx_stock_getFirstStockInfo();
    while ((stockinfo != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, stockinfo->tsi_strCode);
        total = MAX_NUM_OF_RESULT;
        u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            stockInc = _getStockClosingPriceInc(pcap, buffer, total);
            if (strncmp(stockinfo->tsi_strCode, "sh", 2) == 0)
            {
                if (stockInc == 0.0)
                    ;
                else if (stockInc > shIndexInc * thres)
                {
                    aboveSh ++;
                    if (_isStockHighlimit(pcap, buffer, total))
                        aboveShHl++;
                }
                else
                {
                    belowSh ++;
                    if (_isStockHighlimit(pcap, buffer, total))
                        belowShHl++;
                }
            }
            else
            {
                if (stockInc == 0.0)
                    ;
                else if (stockInc > szIndexInc * thres)
                {
                    aboveSz ++;
                    if (_isStockHighlimit(pcap, buffer, total))
                        aboveSzHl ++;
                }
                else
                {
                    belowSz ++;
                    if (_isStockHighlimit(pcap, buffer, total))
                        belowSzHl ++;
                }
            }
        }

        stockinfo = tx_stock_getNextStockInfo(stockinfo);
    }

    jf_jiukun_freeMemory((void **)&buffer);

    ol_printf(
        "From %s to %s, thres %.2f\n",
        pcap->cap_pstrStartDate, pcap->cap_pstrEndDate, thres);
    ol_printf(
        "Index %s, %.2f inc. %d stocks above, %d high limit. %d stocks below, %d high limit\n",
        TX_STOCK_SH_COMPOSITE_INDEX, shIndexInc, aboveSh, aboveShHl, belowSh, belowShHl);
    ol_printf(
        "Index %s, %.2f inc. %d stocks above, %d high limit. %d stocks below, %d high limit\n",
        TX_STOCK_SZ_COMPOSITIONAL_INDEX, szIndexInc, aboveSz, aboveSzHl, belowSz, belowSzHl);

    jf_clieng_outputLine("");

    return u32Ret;
}

static u32 _analysisIndexStock2(cli_analysis_param_t * pcap)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_info_t * stockinfo;
    oldouble_t stockInc;
    olint_t aboveSh, aboveSz;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_ds_t * buffer = NULL;
    olint_t total = MAX_NUM_OF_RESULT;

    return u32Ret;

    if ((pcap->cap_pstrStartDate == NULL) || (pcap->cap_pstrEndDate == NULL))
        return JF_ERR_MISSING_PARAM;

    aboveSh = aboveSz = 0;
    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);

    stockinfo = tx_stock_getFirstStockInfo();
    while ((stockinfo != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, stockinfo->tsi_strCode);
        total = MAX_NUM_OF_RESULT;
        u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            stockInc = _getStockClosingPriceInc(pcap, buffer, total);
            if (strncmp(stockinfo->tsi_strCode, "sh", 2) == 0)
            {
                if (stockInc == 0.0)
                    ;
                else if (stockInc > 0)
                {
                    aboveSh ++;
                    ol_printf("%s\n", stockinfo->tsi_strCode);
                }
            }
            else
            {
                if (stockInc == 0.0)
                    ;
                else if (stockInc > 0)
                {
                    aboveSz ++;
                    ol_printf("%s\n", stockinfo->tsi_strCode);
                }
            }
        }

        stockinfo = tx_stock_getNextStockInfo(stockinfo);
    }

    jf_jiukun_freeMemory((void **)&buffer);

    ol_printf(
        "From %s to %s\n", pcap->cap_pstrStartDate, pcap->cap_pstrEndDate);
    ol_printf(
        "Index %s, %d stocks above\n", TX_STOCK_SH_COMPOSITE_INDEX, aboveSh);
    ol_printf(
        "Index %s, %d stocks above\n", TX_STOCK_SZ_COMPOSITIONAL_INDEX, aboveSz);

    jf_clieng_outputLine("");

    return u32Ret;
}

static u32 _analysisIndexStock(cli_analysis_param_t * pcap)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    _analysisIndexStock1(pcap);

    _analysisIndexStock2(pcap);

    return u32Ret;
}

static u32 _analysisStockStatArbi2(cli_analysis_param_t * pcap)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    tx_statarbi_eval_result_t result;
    tx_statarbi_eval_result_entry_t * entry;
    tx_stock_indu_info_t * induinfo;
#define MAX_STOCK_PAIR  2000

    ol_memset(&result, 0, sizeof(tx_statarbi_eval_result_t));
    result.tser_nMaxPair = MAX_STOCK_PAIR;
    jf_jiukun_allocMemory(
        (void **)&result.tser_ptsereEntry,
        sizeof(tx_statarbi_eval_result_entry_t) * result.tser_nMaxPair);

    u32Ret = _analysisStockStatArbi(pcap, &result);
    if ((u32Ret == JF_ERR_NO_ERROR) && (result.tser_nNumOfPair > 0))
    {
        for (i = 0; i < result.tser_nNumOfPair; i ++)
        {
            entry = &result.tser_ptsereEntry[i];
            tx_stock_getInduInfo(entry->tsere_nInduId, &induinfo);
            jf_clieng_outputLine(
                "%s, %.2f, %s", entry->tsere_strStockPair, entry->tsere_dbCorrelation,
                induinfo->tsii_pstrDesc);
        }
        jf_clieng_outputLine("Found %d pairs stock", result.tser_nNumOfPair);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_file_openWithMode(
            TX_STOCK_STAT_ARBI_LIST_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC,
            JF_FILE_DEFAULT_CREATE_MODE, &fd);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            for (i = 0; i < result.tser_nNumOfPair; i ++)
            {
                entry = &result.tser_ptsereEntry[i];
                jf_file_writen(fd, entry->tsere_strStockPair, 17);
                jf_file_writen(fd, "\n", 1);
            }

            jf_file_close(&fd);
        }
    }

    jf_jiukun_freeMemory((void **)&result.tser_ptsereEntry);

    jf_clieng_outputLine("");

    return u32Ret;
}

static u32 _analysisStockLimit(cli_analysis_param_t * pcap)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;
    tx_statarbi_eval_result_t result;
    tx_statarbi_eval_result_entry_t * entry;
    tx_stock_indu_info_t * induinfo;
    tx_statarbi_eval_param_t tsep;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_ds_t * end, * buffer;
    tx_stock_info_t * stockinfo;
    olint_t total = MAX_NUM_OF_RESULT;

    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);

    ol_memset(&result, 0, sizeof(tx_statarbi_eval_result_t));
    result.tser_nMaxPair = 100;
    jf_jiukun_allocMemory(
        (void **)&result.tser_ptsereEntry,
        sizeof(tx_statarbi_eval_result_entry_t) * result.tser_nMaxPair);

    ol_memset(&tsep, 0, sizeof(tx_statarbi_eval_param_t));
    tsep.tsep_dbMinCorrelation = 0.95;
    tsep.tsep_nDaySummary = NUM_OF_STAT_ARBI_DAY_SUMMARY + 10;
    tsep.tsep_nCorrelationArray = NUM_OF_STAT_ARBI_DAY_SUMMARY;

    stockinfo = tx_stock_getFirstStockInfo();
    while (stockinfo != NULL)
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s", tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR,
            stockinfo->tsi_strCode);
        total = MAX_ANALYSIS_DAY_SUMMARY;
        u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            end = buffer + total - 1;
            if (! end->td_bHighHighLimit)
            {
                stockinfo = tx_stock_getNextStockInfo(stockinfo);
                continue;
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            result.tser_nNumOfPair = 0;
            u32Ret = tx_statarbi_evalStock(
                tx_env_getVar(TX_ENV_VAR_DATA_PATH), stockinfo, &tsep, &result);
        }

        if ((u32Ret == JF_ERR_NO_ERROR) && (result.tser_nNumOfPair > 0))
        {
            for (i = 0; i < result.tser_nNumOfPair; i ++)
            {
                entry = &result.tser_ptsereEntry[i];
                ol_snprintf(
                    dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s", tx_env_getVar(TX_ENV_VAR_DATA_PATH),
                    PATH_SEPARATOR, entry->tsere_strStockPair + 9);
                total = MAX_ANALYSIS_DAY_SUMMARY;
                u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    end = buffer + total - 1;
                    tx_stock_getInduInfo(entry->tsere_nInduId, &induinfo);
                    jf_clieng_outputLine(
                        "%s, %.2f, %s, %.2f", entry->tsere_strStockPair, entry->tsere_dbCorrelation,
                        induinfo->tsii_pstrDesc, end->td_dbClosingPriceRate);
                }
            }
            jf_clieng_outputLine("Found %d pairs stock", result.tser_nNumOfPair);
        }

        stockinfo = tx_stock_getNextStockInfo(stockinfo);
    }

    jf_jiukun_freeMemory((void **)&result.tser_ptsereEntry);
    jf_jiukun_freeMemory((void **)&buffer);

    jf_clieng_outputLine("");

    return u32Ret;
}

static void _atdstockToList(
    olchar_t ** stocklist, olint_t * numoflist, olchar_t * stock, olchar_t * pair)
{
    olint_t i;

    for (i = 0; i < *numoflist; i ++)
    {
        if (strncmp(stocklist[i], stock, 8) == 0)
        {
            ol_strcat(stocklist[i], pair);
            ol_strcat(stocklist[i], ",");
            return;
        }
    }

    ol_strcpy(stocklist[*numoflist], stock);
    ol_strcat(stocklist[*numoflist], ",");
    ol_strcat(stocklist[*numoflist], pair);
    ol_strcat(stocklist[*numoflist], ",");
    *numoflist = *numoflist + 1;
}

static void _strcatStock(olchar_t ** stocklist, olint_t * numoflist, olchar_t * stockpair)
{
    stockpair[8] = '\0';
    _atdstockToList(stocklist, numoflist, stockpair, &stockpair[9]);
    _atdstockToList(stocklist, numoflist, &stockpair[9], stockpair);
}

static void _allocMemoryForStockList(olchar_t ** stocklist, olint_t num)
{
    olint_t i;

    for (i = 0; i < num; i ++)
    {
        jf_jiukun_allocMemory((void **)&stocklist[i], 200);
    }

}

static void _freeMemoryForStockList(olchar_t ** stocklist, olint_t num)
{
    olint_t i;

    for (i = 0; i < num; i ++)
    {
        jf_jiukun_freeMemory((void **)&stocklist[i]);
    }

}

static boolean_t _analysisSectorStockToughness(
    cli_analysis_param_t * pcap, olchar_t * pstrStock,
    tx_ds_t * shiBuf, olint_t shiTotal,
    tx_ds_t * sziBuf, olint_t sziTotal)
{
    boolean_t bTough = FALSE;
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_stock_info_t * stockinfo;
    tx_ds_t * buffer = NULL;
    olint_t total = MAX_ANALYSIS_DAY_SUMMARY;
    tx_ds_t * idxBuf;
    olint_t idxTotal;
#define VOLLTILITY_DAY_SUMMARY_COUNT  20

    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);

    u32Ret = tx_stock_getStockInfo(pstrStock, &stockinfo);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s", tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR,
            stockinfo->tsi_strCode);
        total = MAX_ANALYSIS_DAY_SUMMARY;
        u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            stockinfo->tsi_dbValue = _getStockVolatility(
                stockinfo, buffer, total, VOLLTILITY_DAY_SUMMARY_COUNT);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (tx_stock_isShStockExchange(stockinfo->tsi_strCode))
        {
            idxBuf = shiBuf;
            idxTotal = shiTotal;
        }
        else
        {
            idxBuf = sziBuf;
            idxTotal = sziTotal;
        }

        u32Ret = _analysisDaySummary(
            stockinfo, buffer, total, idxBuf, idxTotal, 1, &bTough);

        if ((u32Ret == JF_ERR_NO_ERROR) && bTough)
        {
            u32Ret = _analysisStockGraph(
                stockinfo, buffer, total, &bTough, VOLLTILITY_DAY_SUMMARY_COUNT);
        }
    }

    jf_jiukun_freeMemory((void **)&buffer);

    return bTough;
}

static u32 _analysisSector(cli_analysis_param_t * pcap)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, j;
    tx_statarbi_eval_result_t result;
    tx_statarbi_eval_result_entry_t * entry;
    tx_statarbi_eval_param_t tsep;
    olint_t numofsector = 100;
    tx_sector_info_t tsi[100];
    olint_t fd = JF_FILE_INVALID_FILE_VALUE;
    olchar_t str[128];
    olsize_t size;
#define MAX_STOCKLIST  200
    olint_t numoflist = 0;
    olchar_t * stocklist[MAX_STOCKLIST];
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_ds_t * shiBuf = NULL;
    olint_t shiTotal = MAX_ANALYSIS_DAY_SUMMARY;
    tx_ds_t * sziBuf = NULL;
    olint_t sziTotal = MAX_ANALYSIS_DAY_SUMMARY;

    jf_jiukun_allocMemory((void **)&shiBuf, sizeof(tx_ds_t) * shiTotal);
    jf_jiukun_allocMemory((void **)&sziBuf, sizeof(tx_ds_t) * sziTotal);
    _allocMemoryForStockList(stocklist, MAX_STOCKLIST);

    ol_memset(&result, 0, sizeof(tx_statarbi_eval_result_t));
    result.tser_nMaxPair = 300;
    jf_jiukun_allocMemory(
        (void **)&result.tser_ptsereEntry,
        sizeof(tx_statarbi_eval_result_entry_t) * result.tser_nMaxPair);

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, TX_STOCK_SH_COMPOSITE_INDEX);
    u32Ret = tx_ds_readDsWithFRoR(dirpath, shiBuf, &shiTotal);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, TX_STOCK_SZ_COMPOSITIONAL_INDEX);
        u32Ret = tx_ds_readDsWithFRoR(dirpath, sziBuf, &sziTotal);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(tsi, sizeof(tx_sector_info_t) * numofsector);

        ol_bzero(&tsep, sizeof(tx_statarbi_eval_param_t));
        tsep.tsep_dbMinCorrelation = 0.95;
        tsep.tsep_nDaySummary = NUM_OF_STAT_ARBI_DAY_SUMMARY + 10;
        tsep.tsep_nCorrelationArray = NUM_OF_STAT_ARBI_DAY_SUMMARY;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_file_openWithMode(
            TX_STOCK_STAT_ARBI_LIST_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC,
            JF_FILE_DEFAULT_CREATE_MODE, &fd);

    u32Ret = tx_sector_parseDir("sector", tsi, &numofsector);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < numofsector; i ++)
        {
            jf_clieng_outputLine("%s: %s", tsi[i].tsi_strName, tsi[i].tsi_pstrStocks);

            result.tser_nNumOfPair = 0;

            u32Ret = tx_statarbi_evalStockList(
                tx_env_getVar(TX_ENV_VAR_DATA_PATH), tsi[i].tsi_pstrStocks,
                ol_strlen(tsi[i].tsi_pstrStocks) / 9, &tsep, &result);
            if ((u32Ret == JF_ERR_NO_ERROR) && (result.tser_nNumOfPair > 0))
            {
/*
  qsort(
  result.tser_ptsereEntry, result.tser_nNumOfPair,
  sizeof(tx_statarbi_eval_result_entry_t),
  _analysisCompareStatArbiEntry);
*/
                numoflist = 0;
                for (j = 0; j < result.tser_nNumOfPair; j ++)
                {
                    entry = &result.tser_ptsereEntry[j];
                    if (! _analysisSectorStockToughness(
                            pcap, entry->tsere_strStockPair, shiBuf, shiTotal, sziBuf, sziTotal))
                        continue;
                    if (! _analysisSectorStockToughness(
                            pcap, &entry->tsere_strStockPair[9], shiBuf, shiTotal, sziBuf, sziTotal))
                        continue;
                    jf_clieng_outputLine(
                        "%s, %.2f", entry->tsere_strStockPair, entry->tsere_dbCorrelation);
                    _strcatStock(stocklist, &numoflist, entry->tsere_strStockPair);
                    /*get the first at least 10 stocks*/
                    if (numoflist >= MAX_STOCKLIST)
                        break;
                }
                jf_clieng_outputLine("Found %d pairs stock", result.tser_nNumOfPair);

                if (numoflist > 0)
                {
                    size = ol_sprintf(str, "[%s]\n", tsi[i].tsi_strName);
                    jf_file_writen(fd, str, size);
                }

                for (j = 0; j < numoflist; j ++)
                {
                    jf_clieng_outputLine("%s", stocklist[j]);
                    jf_file_writen(fd, stocklist[j], ol_strlen(stocklist[j]));
                    jf_file_writen(fd, "\n", 1);
                }
                jf_clieng_outputLine("Found %d stocks", numoflist);
            }
            jf_clieng_outputLine("");
            tx_sector_freeSectorInfo(&tsi[i]);
        }
    }

    if (fd != JF_FILE_INVALID_FILE_VALUE)
        jf_file_close(&fd);

    jf_jiukun_freeMemory((void **)&result.tser_ptsereEntry);
    _freeMemoryForStockList(stocklist, MAX_STOCKLIST);

    jf_jiukun_freeMemory((void **)&shiBuf);
    jf_jiukun_freeMemory((void **)&sziBuf);

    jf_clieng_outputLine("");

    return u32Ret;
}

static u32 _analysisStockTough(cli_analysis_param_t * pcap)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_info_t ** ptsiStockInfo = NULL;
    olint_t nStockInfo = 0, i;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;

    nStockInfo = 10; //getNumOfStock();
    jf_jiukun_allocMemory((void **)&ptsiStockInfo, sizeof(tx_stock_info_t *) * nStockInfo);

    u32Ret = _analysisStockToughness(pcap, ptsiStockInfo, &nStockInfo);
    if ((u32Ret == JF_ERR_NO_ERROR) && (nStockInfo > 0))
    {
        for (i = 0; i < nStockInfo; i ++)
            jf_clieng_outputLine("%s", ptsiStockInfo[i]->tsi_strCode);
        jf_clieng_outputLine("Found %d stocks\n", nStockInfo);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_file_openWithMode(
            TX_STOCK_TOUGH_LIST_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC,
            JF_FILE_DEFAULT_CREATE_MODE, &fd);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            for (i = 0; i < nStockInfo; i ++)
            {
                jf_file_writen(fd, ptsiStockInfo[i]->tsi_strCode, 8);
                jf_file_writen(fd, "\n", 1);
            }

            jf_file_close(&fd);
        }
    }

    jf_jiukun_freeMemory((void **)&ptsiStockInfo);

//    jf_clieng_outputLine("");

    return u32Ret;
}

#if 0
static oldouble_t _caluStockPriceVolumnCorrelation(
    stock_quo_t * psq, olint_t num, oldouble_t * pdba, oldouble_t * pdbb)
{
    oldouble_t dbr;
    olint_t i, max;
    tx_quo_entry_t * start;

    max = num - 1;
    start = &psq->sq_ptqeEntry[1];
    for (i = 0; i < max; i ++)
    {
        pdba[i] = start->tqe_dbCurPrice;
        pdbb[i] = (oldouble_t)(start->tqe_u64Volume - (start - 1)->tqe_u64Volume);
        start ++;
    }

    getCorrelation(pdba, pdbb, max, &dbr);

    return dbr;
}

static void _analysisPrintQuotation(stock_quo_t * psq, tx_quo_entry_t ** ptqe, olint_t total)
{
    olint_t i;

    jf_clieng_outputLine("Total %d points", total);
    for (i = 0; i < total; i ++)
    {
        jf_clieng_outputLine("%s %.2f", ptqe[i]->tqe_strTime, ptqe[i]->tqe_dbCurPrice);
    }
    jf_clieng_outputLine("");
}
#endif

static boolean_t _analysisStockPriceVolumnUp(
    stock_quo_t * psq, olint_t num, tx_quo_entry_t ** ptqe, olint_t total, oldouble_t * pdba)
{
    tx_quo_entry_t * end;
    oldouble_t db1, db2;

    return FALSE;

    if (total < 4)
        return FALSE;

    if (ptqe[total - 1]->tqe_dbCurPrice <= ptqe[total - 3]->tqe_dbCurPrice)
        return FALSE;
    if (ptqe[total - 2]->tqe_dbCurPrice <= ptqe[total - 4]->tqe_dbCurPrice)
        return FALSE;
    if (ptqe[total - 1]->tqe_dbCurPrice > ptqe[total - 2]->tqe_dbCurPrice)
        return FALSE;

    if (pdba[total - 2] <= pdba[total - 4])
        return FALSE;

    end = &psq->sq_ptqeEntry[num - 1];
    if (end == ptqe[total - 1])
        return FALSE;

    db1 = ptqe[total - 1]->tqe_dbCurPrice - ptqe[total - 2]->tqe_dbCurPrice;
    db2 = end->tqe_dbCurPrice - ptqe[total - 1]->tqe_dbCurPrice;
    if ((db2 < db1 * 0.1) || (db2 > db1 * 0.2))
        return FALSE;

    return TRUE;
}

static boolean_t _analysisStockPriceVolumnBottom(
    stock_quo_t * psq, olint_t num, tx_quo_entry_t ** ptqe, olint_t total, oldouble_t * pdba)
{
    tx_quo_entry_t * end;
    oldouble_t db1, db2;

    if (total < 6)
        return FALSE;

    if (ptqe[total - 1]->tqe_dbCurPrice > ptqe[total - 2]->tqe_dbCurPrice)
        return FALSE;
    if (ptqe[total - 1]->tqe_dbCurPrice <= ptqe[total - 3]->tqe_dbCurPrice)
        return FALSE;

    if (pdba[total - 1] <= pdba[total - 3])
        return FALSE;

    end = &psq->sq_ptqeEntry[num - 1];
    if (end == ptqe[total - 1])
        return FALSE;

//    db1 = ptqe[total - 2]->tqe_dbCurPrice - ptqe[total - 1]->tqe_dbCurPrice;
    db1 = end->tqe_dbHighPrice - end->tqe_dbLowPrice;
    db2 = end->tqe_dbCurPrice - ptqe[total - 1]->tqe_dbCurPrice;
    if ((db2 < db1 * 0.1) || (db2 > db1 * 0.2))
        return FALSE;
/*
    if ((ptqe[total - 2]->tqe_dbCurPrice > ptqe[total - 4]->tqe_dbCurPrice) &&
        (pdba[total - 2] <= pdba[total - 4]))
        return FALSE;
*/
    /*the inflextion poolint_t before the bottom*/
    if (ptqe[total - 5]->tqe_dbCurPrice < ptqe[total - 3]->tqe_dbCurPrice)
        return FALSE;
    if (ptqe[total - 6]->tqe_dbCurPrice < ptqe[total - 4]->tqe_dbCurPrice)
        return FALSE;

    if (pdba[total - 3] > pdba[total - 5])
        return FALSE;

    return TRUE;
}

/* ((close - low) - (high - close)) / (high - low) * V */
static void _analysisStockPriceVolumnObv(
    stock_quo_t * psq, olint_t num, tx_quo_entry_t ** ptqe, olint_t total, oldouble_t * pdba)
{
    olint_t i;

    pdba[0] = (oldouble_t)ptqe[0]->tqe_u64Volume;
    for (i = 1; i < total; i ++)
    {
        pdba[i] = (oldouble_t)(ptqe[i]->tqe_u64Volume - ptqe[i - 1]->tqe_u64Volume);
        if (ptqe[i]->tqe_dbCurPrice > ptqe[i - 1]->tqe_dbCurPrice)
        {
            pdba[i] = pdba[i - 1] + pdba[i];
        }
        else
        {
            pdba[i] = pdba[i - 1] - pdba[i];
        }
    }
#if 1
    for (i = 0; i < total; i ++)
    {
        jf_clieng_outputLine(
            "%s, %.2f, %.2f", ptqe[i]->tqe_strTime, ptqe[i]->tqe_dbCurPrice, pdba[i]);
    }
#endif
}

static olint_t _getNextTopBottomQuoEntry(
    tx_quo_entry_t ** ptqe, olint_t total, olint_t start, boolean_t bUp)
{
    olint_t next, cur;
#if 0
    oldouble_t db;

    if (bUp)
        db = ptqe[start]->tqe_dbCurPrice * (1.0 + 0.1);
    else
        db = ptqe[start]->tqe_dbCurPrice * (1.0 - 0.1);
#endif
    cur = start;
    next = cur + 1;
    while (cur <= total)
    {
        if (bUp)
        {
            if (ptqe[cur]->tqe_dbCurPrice > ptqe[next]->tqe_dbCurPrice)
                next = start;

        }

        cur ++;
    }

    return next;
}

static boolean_t _analysisStockPriceVolumn(
    stock_quo_t * psq, olint_t num, tx_quo_entry_t ** ptqe, olint_t total)
{
    oldouble_t pdba[500];
    tx_quo_entry_t * low;
    olint_t cur, next;
    boolean_t bUp;

    assert(total < 500);

    if (total < 3)
        return FALSE;

    _analysisStockPriceVolumnObv(psq, num, ptqe, total, pdba);

    cur = next = 1;
    while (next <= total)
    {
        if (ptqe[cur - 1]->tqe_dbCurPrice <= ptqe[cur]->tqe_dbCurPrice)
            bUp = TRUE;
        else
            bUp = FALSE;


        next = _getNextTopBottomQuoEntry(ptqe, total, cur - 1, bUp);

        cur ++;
    }



    return TRUE;

    low = getQuoEntryWithLowestPrice(psq->sq_ptqeEntry, ptqe[total - 1]);
    if (low == ptqe[total - 3])
    {
        if (! _analysisStockPriceVolumnBottom(psq, num, ptqe, total, pdba))
            return FALSE;
    }
    else if (low != ptqe[total - 3])
    {
        if (! _analysisStockPriceVolumnUp(psq, num, ptqe, total, pdba))
            return FALSE;
    }
    else
        return FALSE;

    return TRUE;
}

static u32 _analysisStockIndexQuotation(
    stock_quo_t * psq, stock_quo_t * psqi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, total;
    oldouble_t * pdba = NULL, * pdbb = NULL; //, dbr1, dbr2;
    tx_quo_entry_t ** ptqe = NULL;
//    tx_quo_entry_t * low;

    jf_jiukun_allocMemory((void **)&pdba, sizeof(oldouble_t) * psq->sq_nMaxEntry);
    jf_jiukun_allocMemory((void **)&pdbb, sizeof(oldouble_t) * psq->sq_nMaxEntry);
    jf_jiukun_allocMemory((void **)&ptqe, sizeof(tx_quo_entry_t *) * psqi->sq_nNumOfEntry);

    for (i = 469; i < psqi->sq_nNumOfEntry; i ++)
    {
        if (strcmp(psqi->sq_ptqeEntry[i].tqe_strTime, "09:50:00") < 0)
            continue;

//        if (psq->sq_ptqeEntry[i].tqe_dbCurPrice <= psq->sq_dbLastClosingPrice)
//            continue;

//        dbr1 = _caluStockPriceVolumnCorrelation(psq, i + 1, pdba, pdbb);
//        if (dbr1 <= 0)
//            continue;

        total = psqi->sq_nNumOfEntry;
        getQuoEntryInflexionPoint(
            psqi->sq_ptqeEntry, i + 1, ptqe, &total);

        if (! _analysisStockPriceVolumn(psqi, i + 1, ptqe, total))
            continue;

/*
        dbr2 = _caluStockPriceVolumnCorrelation(psqi, i + 1, pdba, pdbb);
        if (dbr2 <= 0)
        {
            low = getQuoEntryWithLowestPrice(psq->sq_ptqeEntry, ptqe[total - 3]);
            if (low != ptqe[total - 3])
                continue;
        }
*/
        jf_clieng_printDivider();
        jf_clieng_outputLine(
            "%s, %.2f, %d", psqi->sq_ptqeEntry[i].tqe_strTime, psqi->sq_ptqeEntry[i].tqe_dbCurPrice, i);
//        jf_clieng_outputLine("Stock price and volumn: %.2f", dbr1);
//        jf_clieng_outputLine("Index price and volumn: %.2f", dbr2);
//        _analysisPrintQuotation(psqi, ptqe, total);
        break;
    }

    jf_jiukun_freeMemory((void **)&pdba);
    jf_jiukun_freeMemory((void **)&pdbb);
    jf_jiukun_freeMemory((void **)&ptqe);

    return u32Ret;
}

static u32 _analysisFillStockQuotation(
    stock_quo_t * psq, tx_stock_info_t * stockinfo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * buffer = NULL;
    olint_t total = 100;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_ds_t * end;

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s", tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR,
        stockinfo->tsi_strCode);

    jf_jiukun_allocMemory((void **)&buffer, sizeof(tx_ds_t) * total);

    u32Ret = tx_ds_readDs(dirpath, buffer, &total);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        tx_ds_getDsWithDate(buffer, total, psq->sq_strDate, &end);
        end --;

        psq->sq_dbLastClosingPrice = end->td_dbClosingPrice;
    }

    jf_jiukun_freeMemory((void **)&buffer);

    return u32Ret;
}

static u32 _analysisStockQuotation(cli_analysis_param_t * pcap, tx_cli_master_t * ptcm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrIndex;
    stock_quo_t stockquo, stockquoindex;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_stock_info_t * stockinfo;

    if (tx_stock_isStockIndex(pcap->cap_pstrDir[0]))
    {
        jf_clieng_outputLine("Cannot analysis the quotation of stock index");
        return JF_ERR_INVALID_PARAM;
    }

    ol_memset(&stockquo, 0, sizeof(stock_quo_t));
    ol_memset(&stockquoindex, 0, sizeof(stock_quo_t));

    if (tx_stock_isShStockExchange(pcap->cap_pstrDir[0]))
        pstrIndex = TX_STOCK_SH_COMPOSITE_INDEX;
    else
        pstrIndex = TX_STOCK_SZ_COMPOSITIONAL_INDEX;

    u32Ret = tx_stock_getStockInfo(pcap->cap_pstrDir[0], &stockinfo);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        stockquo.sq_nMaxEntry = 3500;
        ol_strcpy(stockquo.sq_strCode, stockinfo->tsi_strCode);
        if (pcap->cap_pstrStartDate != NULL)
            ol_strcpy(stockquo.sq_strDate, pcap->cap_pstrStartDate);

        u32Ret = jf_mem_alloc(
            (void **)&stockquo.sq_ptqeEntry, sizeof(tx_quo_entry_t) * stockquo.sq_nMaxEntry);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s", tx_env_getVar(TX_ENV_VAR_DATA_PATH),
            PATH_SEPARATOR, pcap->cap_pstrDir[0]);

        u32Ret = readStockQuotationFile(dirpath, &stockquo);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _analysisFillStockQuotation(&stockquo, stockinfo);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        stockquoindex.sq_nMaxEntry = 3500;
        ol_strcpy(stockquoindex.sq_strCode, pstrIndex);
        if (pcap->cap_pstrStartDate != NULL)
            ol_strcpy(stockquoindex.sq_strDate, pcap->cap_pstrStartDate);

        u32Ret = jf_mem_alloc(
            (void **)&stockquoindex.sq_ptqeEntry, sizeof(tx_quo_entry_t) * stockquoindex.sq_nMaxEntry);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s", tx_env_getVar(TX_ENV_VAR_DATA_PATH),
            PATH_SEPARATOR, pstrIndex);

        u32Ret = readStockQuotationFile(dirpath, &stockquoindex);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _analysisStockIndexQuotation(&stockquo, &stockquoindex);
    }

    if (stockquo.sq_ptqeEntry != NULL)
        jf_mem_free((void **)&stockquo.sq_ptqeEntry);
    if (stockquoindex.sq_ptqeEntry != NULL)
        jf_mem_free((void **)&stockquoindex.sq_ptqeEntry);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processAnalysis(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_analysis_param_t * pcap = (cli_analysis_param_t *)pParam;
    tx_cli_master_t * ptcm = (tx_cli_master_t *)pMaster;

    if (pcap->cap_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _analysisHelp(ptcm);
    else if (tx_env_isNullVarDataPath())
    {
        jf_clieng_outputLine("Data path is not set.");
        u32Ret = JF_ERR_NOT_READY;
    }
    else if (pcap->cap_u8Action == CLI_ACTION_ANALYSIS_STOCK)
    {
        if (pcap->cap_nDir == 0)
            u32Ret = JF_ERR_MISSING_PARAM;
        else
            u32Ret = _analysisStock(pcap, ptcm);
    }
    else if (pcap->cap_u8Action == CLI_ACTION_ANALYSIS_STOCK_QUOTATION)
        u32Ret = _analysisStockQuotation(pcap, ptcm);
    else if (pcap->cap_u8Action == CLI_ACTION_ANALYSIS_STOCK_POOL)
        u32Ret = _analysisStockPool(pcap);
    else if (pcap->cap_u8Action == CLI_ACTION_ANALYSIS_STOCK_TOUGH)
        u32Ret = _analysisStockTough(pcap);
    else if (pcap->cap_u8Action == CLI_ACTION_ANALYSIS_STOCK_STAT_ARBI)
        u32Ret = _analysisStockStatArbi2(pcap);
    else if (pcap->cap_u8Action == CLI_ACTION_ANALYSIS_INDEX_STOCKS)
        u32Ret = _analysisIndexStock(pcap);
    else if (pcap->cap_u8Action == CLI_ACTION_ANALYSIS_STOCK_LIMIT)
        u32Ret = _analysisStockLimit(pcap);
    else if (pcap->cap_u8Action == CLI_ACTION_ANALYSIS_SECTOR)
        u32Ret = _analysisSector(pcap);
    else
        u32Ret = JF_ERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamAnalysis(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_analysis_param_t * pcap = (cli_analysis_param_t *)pParam;

    ol_memset(pcap, 0, sizeof(cli_analysis_param_t));
    pcap->cap_u8Action = CLI_ACTION_ANALYSIS_SECTOR;

    return u32Ret;
}

u32 parseAnalysis(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_analysis_param_t * pcap = (cli_analysis_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv, "atbns:q:if:l:hv?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 's':
            pcap->cap_u8Action = CLI_ACTION_ANALYSIS_STOCK;
            if (pcap->cap_nDir < MAX_ANALYSIS_DIR)
            {
                pcap->cap_pstrDir[pcap->cap_nDir] = (olchar_t *)optarg;
                pcap->cap_nDir ++;
            }
            break;
        case 'q':
            pcap->cap_u8Action = CLI_ACTION_ANALYSIS_STOCK_QUOTATION;
            pcap->cap_pstrDir[0] = (olchar_t *)optarg;
            pcap->cap_nDir = 1;
            break;
        case 'a':
            pcap->cap_u8Action = CLI_ACTION_ANALYSIS_STOCK_STAT_ARBI;
            break;
        case 't':
            pcap->cap_u8Action = CLI_ACTION_ANALYSIS_STOCK_TOUGH;
            break;
        case 'b':
            pcap->cap_u8Action = CLI_ACTION_ANALYSIS_STOCK_LIMIT;
            break;
        case 'n':
            pcap->cap_u8Action = CLI_ACTION_ANALYSIS_SECTOR;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case 'i':
            pcap->cap_u8Action = CLI_ACTION_ANALYSIS_INDEX_STOCKS;
            break;
        case 'v':
            pcap->cap_bVerbose = TRUE;
            break;
        case 'f':
            pcap->cap_pstrStartDate = (olchar_t *)optarg;
            break;
        case 'l':
            pcap->cap_pstrEndDate = (olchar_t *)optarg;
            break;
        case '?':
        case 'h':
            pcap->cap_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

void __toSuppressWarnMsg(void)
{
    cli_analysis_param_t * pcap = NULL;
    tx_stock_info_t * stockinfo = NULL; 
    tx_ds_t * buffer = NULL;
    olint_t total = MAX_NUM_OF_RESULT;

    _analysisDaySummary_OLD(pcap, stockinfo, buffer, total);
}

/*------------------------------------------------------------------------------------------------*/

