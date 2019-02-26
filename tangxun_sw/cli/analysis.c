/**
 *  @file analysis.c
 *
 *  @brief The analysis command
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <math.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <stdlib.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "clicmd.h"
#include "stringparse.h"
#include "files.h"
#include "xmalloc.h"
#include "parsedata.h"
#include "indicator.h"
#include "stocklist.h"
#include "damethod.h"
#include "jiukun.h"
#include "envvar.h"
#include "datastat.h"
#include "statarbitrage.h"
#include "xtime.h"

/* --- private data/data structure section --------------------------------- */

#define MAX_NUM_OF_RESULT  500
#define MAX_ANALYSIS_FIND_DAY_SUMMARY  500
#define MAX_ANALYSIS_DAY_SUMMARY  100

#define NUM_OF_STAT_ARBI_DAY_SUMMARY 40

/* --- private routine section---------------------------------------------- */
static u32 _analysisHelp(da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputRawLine2("\
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
    cliengOutputRawLine2("\
  -v: verbose.");
    cliengOutputRawLine2("\
  -f: the first date with format \"yyyy-mm-dd\".\n\
  -l: the last date with format \"yyyy-mm-dd\".");
    cliengOutputLine("");

    return u32Ret;
}

static oldouble_t _getClosingPriceInc(
    da_day_summary_t * start, da_day_summary_t * end)
{
    oldouble_t dbret = -9999.99;

    dbret = (end->dds_dbClosingPrice - start->dds_dbClosingPrice) * 100 /
        start->dds_dbClosingPrice;

    return dbret;
}

static void _getStartEndDaySummary(
    da_day_summary_t * buffer, olint_t num, olchar_t * startdate, olchar_t * enddate,
    da_day_summary_t ** ppStart, da_day_summary_t ** ppEnd)
{
    da_day_summary_t * start, * end;

    start = buffer;
    end = buffer + num - 1;

    if (strncmp(end->dds_strDate, startdate, 10) < 0)
    {
        *ppStart = *ppEnd = NULL;
        return;
    }

    while (start < end)
    {
        if (strncmp(start->dds_strDate, startdate, 10) >= 0)
            break;

        start ++;
    }

    while (end > start)
    {
        if (strncmp(end->dds_strDate, enddate, 10) <= 0)
            break;

        end --;
    }

    *ppStart = start;
    *ppEnd = end;
}

static boolean_t _getStockIndexClosingPriceInc(
    cli_analysis_param_t * pcap, stock_info_t * stockinfo, olchar_t * lowdate, oldouble_t * dbInc)
{
    boolean_t bTough = FALSE;
    oldouble_t dbret = -9999.99;
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t * strindex = NULL;
    da_day_summary_t * buffer = NULL;
    olint_t total = MAX_NUM_OF_RESULT;
    olchar_t dirpath[MAX_PATH_LEN];
    da_day_summary_t * start, * end, * high, * low;

    if (isShStockExchange(stockinfo->si_strCode))
        strindex = SH_COMPOSITE_INDEX;
    else
        strindex = SZ_COMPOSITIONAL_INDEX;

    ol_snprintf(
        dirpath, MAX_PATH_LEN, "%s%c%s",
        getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, strindex);

    allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);

    u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
    if (u32Ret == OLERR_NO_ERROR)
    {
        _getStartEndDaySummary(
            buffer, total, pcap->cap_pstrStartDate, pcap->cap_pstrEndDate, &start, &end);

        if (start == NULL)
            dbret = 0.0;
        else
        {
            low = getDaySummaryWithLowestClosingPrice(start, end - start + 1);
            high = getDaySummaryWithHighestClosingPrice(low, end - low + 1);
            dbret = _getClosingPriceInc(low, high);
        }

        ol_printf(
            "Index %s, from %s to %s, %.2f inc\n",
            strindex, low->dds_strDate, high->dds_strDate, dbret);

        if (strncmp(lowdate, low->dds_strDate, 10) <= 0)
            bTough = TRUE;
    }

    freeMemory((void **)&buffer);

    *dbInc = dbret;

    return bTough;
}

static u32 _analysisDaySummary_OLD(
    cli_analysis_param_t * pcap, stock_info_t * stockinfo,
    da_day_summary_t * buffer, olint_t num)
{
    u32 u32Ret = OLERR_NO_ERROR;
    oldouble_t inc1, inc2;
    da_day_summary_t * start, * end, * high, * low;
    boolean_t bTough = FALSE;

    start = buffer;
    end = buffer + num - 1;

    if (pcap->cap_pstrStartDate == NULL)
        pcap->cap_pstrStartDate = start->dds_strDate;
    if (pcap->cap_pstrEndDate == NULL)
        pcap->cap_pstrEndDate = end->dds_strDate;

    _getStartEndDaySummary(
        buffer, num, pcap->cap_pstrStartDate, pcap->cap_pstrEndDate, &start, &end);

    if (start == NULL)
        inc1 = 0.0;
    else
    {
        low = getDaySummaryWithLowestClosingPrice(start, end - start + 1);
        high = getDaySummaryWithHighestClosingPrice(low, end - low + 1);
        inc1 = _getClosingPriceInc(low, high);
    }

    ol_printf(
        "Stock %s, from %s to %s, %.2f inc\n",
        stockinfo->si_strCode, low->dds_strDate, high->dds_strDate, inc1);

    bTough = _getStockIndexClosingPriceInc(pcap, stockinfo, low->dds_strDate, &inc2);

    if (bTough && (inc1 <= inc2))
            bTough = FALSE;

    if (bTough)
        ol_printf(
            "Stock %s(%.2f) is Tougher than index(%.2f)\n\n",
            stockinfo->si_strCode, inc1, inc2);
    else
        ol_printf(
            "Stock %s(%.2f) is Weaker than index(%.2f)\n\n",
            stockinfo->si_strCode, inc1, inc2);

    return u32Ret;
}

static boolean_t _analysisDaySummaryRegion(
    da_day_summary_t * buffer, olint_t total,
    da_day_summary_t ** fp, olint_t nFp,
    da_day_summary_t * idxStart, da_day_summary_t * idxEnd)
{
    boolean_t bTough = FALSE;
    oldouble_t inc1, inc2;
    da_day_summary_t * start1, * end1, * first, * last;

    inc1 = _getClosingPriceInc(idxStart, idxEnd);

    getDaySummaryWithDate(buffer, total, idxStart->dds_strDate, &first);
    getDaySummaryWithDate(buffer, total, idxEnd->dds_strDate, &last);
    if ((first == NULL) || (last == NULL))
    {
#if 0
        cliengOutputLine(
            "Index from %s to %s, %.2f inc",
            idxStart->dds_strDate, idxEnd->dds_strDate, inc1);
        cliengOutputLine(
            "Stock from %s to %s, no trade in the first day or the last day",
            idxStart->dds_strDate, idxEnd->dds_strDate);
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
        locateInflexionPointRegion(fp, nFp, first, &start1, &end1);
        if (end1->dds_dbClosingPrice > start1->dds_dbClosingPrice)
            first = start1 - 1;

        inc2 = _getClosingPriceInc(first, last);
        if (inc1 < inc2)
            bTough = TRUE;
    }
#if 0
    cliengOutputLine(
        "Index from %s to %s, %.2f inc",
        idxStart->dds_strDate, idxEnd->dds_strDate, inc1);
    cliengOutputLine(
        "Stock from %s to %s, %.2f inc",
        first->dds_strDate, last->dds_strDate, inc2);
#endif
    return bTough;
}

static u32 _analysisDaySummary(
    stock_info_t * stockinfo, da_day_summary_t * buffer, olint_t total,
    da_day_summary_t * idxBuf, olint_t idxTotal, olint_t nCount, boolean_t * pbTough)
{
    u32 u32Ret = OLERR_NO_ERROR;
    boolean_t bTough = FALSE;
#define MAX_INF_COUNT  60
    da_day_summary_t * fp[MAX_INF_COUNT];
    olint_t nFp = MAX_INF_COUNT;
    da_day_summary_t * idxFp[MAX_INF_COUNT];
    olint_t nIdxFp = MAX_INF_COUNT;
    olint_t i;
    da_day_summary_t * end, * last;

    *pbTough = FALSE;
    end = buffer + total - 1;
    last = idxBuf + idxTotal - 1;
    if (strncmp(end->dds_strDate, last->dds_strDate, 10) != 0)
    {
//        cliengOutputLine("Stock has no trade in the last day");
        return u32Ret;
    }

    getDaySummaryInflexionPoint(buffer, total, fp, &nFp);
    getDaySummaryInflexionPoint(idxBuf, idxTotal, idxFp, &nIdxFp);

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
        cliengOutputLine(
            "Stock %s is Tougher than index",
            stockinfo->si_strCode);
    else
        cliengOutputLine(
            "Stock %s is Weaker than index",
            stockinfo->si_strCode);
#endif
    *pbTough = bTough;

    return u32Ret;
}

static u32 _getVolumePair(
    stock_info_t * stockinfo, da_day_summary_t * buffer, olint_t total,
    da_day_summary_t * idxBuf, olint_t idxTotal,
    oldouble_t * pdba, oldouble_t * pdbb, olint_t num)
{
    u32 u32Ret = OLERR_NO_ERROR;
    da_day_summary_t * cura, * enda, * curb, * endb;
    olint_t ret, count = 0, index = num - 1, mismatch = 0;

    cura = buffer;
    enda = cura + total - 1;
    curb = idxBuf;
    endb = curb + idxTotal - 1;
    while ((enda >= cura) && (endb >= curb))
    {
        ret = strcmp(enda->dds_strDate, endb->dds_strDate);
        if (ret == 0)
        {
            pdba[index] = (oldouble_t)enda->dds_u64All;
            pdbb[index] = (oldouble_t)endb->dds_u64All;
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
        u32Ret = OLERR_NOT_READY;

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (mismatch >= num * 3 / 4)
            u32Ret = OLERR_NOT_READY;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {

    }

    return u32Ret;
}

static u32 _analysisStock(cli_analysis_param_t * pcap, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i;
    olchar_t dirpath[MAX_PATH_LEN];
    stock_info_t * stockinfo;
    da_day_summary_t * buffer = NULL;
    olint_t total = MAX_ANALYSIS_DAY_SUMMARY;
    da_day_summary_t * shiBuf = NULL;
    olint_t shiTotal = MAX_ANALYSIS_DAY_SUMMARY;
    da_day_summary_t * sziBuf = NULL;
    olint_t sziTotal = MAX_ANALYSIS_DAY_SUMMARY;
    da_day_summary_t * idxBuf = NULL;
    olint_t idxTotal;
    olint_t nCount = 20;
    oldouble_t * pdba = NULL, * pdbb = NULL, dbr;

    allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);
    allocMemory((void **)&shiBuf, sizeof(da_day_summary_t) * shiTotal, 0);
    allocMemory((void **)&sziBuf, sizeof(da_day_summary_t) * sziTotal, 0);
    allocMemory((void **)&pdba, sizeof(oldouble_t) * nCount, 0);
    allocMemory((void **)&pdbb, sizeof(oldouble_t) * nCount, 0);

    ol_snprintf(
        dirpath, MAX_PATH_LEN, "%s%c%s",
        getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, SH_COMPOSITE_INDEX);
    u32Ret = readTradeDaySummaryWithFRoR(dirpath, shiBuf, &shiTotal);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, SZ_COMPOSITIONAL_INDEX);
        u32Ret = readTradeDaySummaryWithFRoR(dirpath, sziBuf, &sziTotal);
    }

    for (i = 0; (i < pcap->cap_nDir) && (u32Ret == OLERR_NO_ERROR); i ++)
    {
        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = getStockInfo(pcap->cap_pstrDir[i], &stockinfo);

        if (u32Ret == OLERR_NO_ERROR)
        {
            ol_snprintf(
                dirpath, MAX_PATH_LEN, "%s%c%s",
                getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR,
                pcap->cap_pstrDir[i]);
            total = MAX_ANALYSIS_DAY_SUMMARY;
            u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            if (isShStockExchange(stockinfo->si_strCode))
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
            if (u32Ret == OLERR_NO_ERROR)
                u32Ret = getCorrelation(pdba, pdbb, nCount, &dbr);

            if (u32Ret == OLERR_NO_ERROR)
                cliengOutputLine("Correlation: %.2f", dbr);
        }
    }

    freeMemory((void **)&buffer);
    freeMemory((void **)&shiBuf);
    freeMemory((void **)&sziBuf);
    freeMemory((void **)&pdba);
    freeMemory((void **)&pdbb);

    return u32Ret;
}

static olint_t _analysisCompareStatArbiEntry(const void * a, const void * b)
{
    stat_arbi_indu_result_entry_t * ra, * rb;
    olint_t ret = 0;

    ra = (stat_arbi_indu_result_entry_t *)a;
    rb = (stat_arbi_indu_result_entry_t *)b;

    if (ra->saire_dbCorrelation > rb->saire_dbCorrelation)
        ret = -1;
    else if (ra->saire_dbCorrelation < rb->saire_dbCorrelation)
        ret = 1;

    return ret;
}

static u32 _analysisStockStatArbi(
    cli_analysis_param_t * pcap, stat_arbi_indu_result_t * result)
{
    u32 u32Ret = OLERR_NO_ERROR;
    stat_arbi_indu_param_t saip;

    memset(&saip, 0, sizeof(stat_arbi_indu_param_t));
    saip.saip_dbMinCorrelation = 0.98;
    saip.saip_nDaySummary = NUM_OF_STAT_ARBI_DAY_SUMMARY + 10;
    saip.saip_nCorrelationArray = NUM_OF_STAT_ARBI_DAY_SUMMARY;

    u32Ret = statArbiAllIndustry(
        getEnvVar(ENV_VAR_DATA_PATH), &saip, result);
    if (u32Ret == OLERR_NO_ERROR)
    {
        qsort(
            result->sair_psaireEntry, result->sair_nNumOfPair,
            sizeof(stat_arbi_indu_result_entry_t),
            _analysisCompareStatArbiEntry);
    }

    return u32Ret;
}

static oldouble_t _getStockVolatility(
    stock_info_t * stockinfo, da_day_summary_t * buffer, olint_t total, olint_t count)
{
    oldouble_t dbvol = 0.0;
    da_day_summary_t * high, * low;

    if (total < count)
        return dbvol;

    buffer = buffer + total - count;
    total = count;

    low = getDaySummaryWithLowestClosingPrice(buffer, total);
    high = getDaySummaryWithHighestClosingPrice(low, buffer + total - low);
    dbvol = (high->dds_dbClosingPrice - low->dds_dbClosingPrice) * 100 /
        low->dds_dbClosingPrice;

#if 0
    cliengOutputLine(
        "%s, volatility %.2f", stockinfo->si_strCode, dbvol);
#endif
    return dbvol;
}

static olint_t _analysisCompareStockValue(const void * a, const void * b)
{
    stock_info_t * ra, * rb;
    olint_t ret = 0;

    ra = *((stock_info_t **)a);
    rb = *((stock_info_t **)b);

    if (ra->si_dbValue > rb->si_dbValue)
        ret = -1;
    else if (ra->si_dbValue < rb->si_dbValue)
        ret = 1;

    return ret;
}

static u32 _analysisStockGraph(
    stock_info_t * stockinfo, da_day_summary_t * buffer, olint_t total,
    boolean_t * pbTough, olint_t count)
{
    u32 u32Ret = OLERR_NO_ERROR;
    da_day_summary_t * end, * high, * start;
    olint_t year, mon, day, days;
    olint_t year2, mon2, day2, days2;
#define MAX_INF_COUNT  60
    da_day_summary_t * fp[MAX_INF_COUNT];
    olint_t nFp = MAX_INF_COUNT;
    olint_t nCount = 1, i;

    *pbTough = TRUE;

    end = buffer + total - 1;
    if (end->dds_bS)
    {
        *pbTough = FALSE;
        return u32Ret;
    }

    end = buffer + total - 1;
    start = buffer + total - NUM_OF_STAT_ARBI_DAY_SUMMARY;

    while (end > start)
    {
        getDate2FromString(end->dds_strDate, &year, &mon, &day);
        days = convertDateToDaysFrom1970(year, mon, day);

        getDate2FromString((end - 1)->dds_strDate, &year2, &mon2, &day2);
        days2 = convertDateToDaysFrom1970(year2, mon2, day2);

        if (days - days2 > 30)
        {
            *pbTough = FALSE;
            return u32Ret;
        }

        end --;
    }

    end = buffer + total - 1;
    start = buffer + total - count;

    high = getDaySummaryWithHighestClosingPrice(start, count);
    if ((high != end) /*&& (high != end - 1)*/)
    {
        *pbTough = FALSE;
        return u32Ret;
    }

    getDaySummaryInflexionPoint(buffer, total, fp, &nFp);
    i = nFp - nCount - 2;
    while (nCount > 0)
    {
        end = fp[i + nCount + 1];
        start = fp[i + nCount];

        while (end > start)
        {
            if (end->dds_bLowLowLimit)
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
    cli_analysis_param_t * pcap, stock_info_t ** ppsiStockInfo, olint_t * pnStockInfo)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t dirpath[MAX_PATH_LEN];
    stock_info_t * stockinfo;
    da_day_summary_t * buffer = NULL;
    olint_t total = MAX_ANALYSIS_DAY_SUMMARY;
    da_day_summary_t * shiBuf = NULL;
    olint_t shiTotal = MAX_ANALYSIS_DAY_SUMMARY;
    da_day_summary_t * sziBuf = NULL;
    olint_t sziTotal = MAX_ANALYSIS_DAY_SUMMARY;
    da_day_summary_t * idxBuf;
    olint_t idxTotal;
    boolean_t bTough = FALSE;
    olint_t nStockInfo = 0;
    stock_info_t ** ppsiStock = NULL;
    olint_t nStock = 0, i;
#define VOLLTILITY_DAY_SUMMARY_COUNT  20

//    *pnStockInfo = 0;
//    return u32Ret;

    allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);
    allocMemory((void **)&shiBuf, sizeof(da_day_summary_t) * shiTotal, 0);
    allocMemory((void **)&sziBuf, sizeof(da_day_summary_t) * sziTotal, 0);
    allocMemory((void **)&ppsiStock, sizeof(stock_info_t *) * getNumOfStock(), 0);

    ol_snprintf(
        dirpath, MAX_PATH_LEN, "%s%c%s",
        getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, SH_COMPOSITE_INDEX);
    u32Ret = readTradeDaySummaryWithFRoR(dirpath, shiBuf, &shiTotal);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, SZ_COMPOSITIONAL_INDEX);
        u32Ret = readTradeDaySummaryWithFRoR(dirpath, sziBuf, &sziTotal);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        stockinfo = getFirstStockInfo();
        while (stockinfo != NULL)
        {
            ol_snprintf(
                dirpath, MAX_PATH_LEN, "%s%c%s",
                getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR,
                stockinfo->si_strCode);
            total = MAX_ANALYSIS_DAY_SUMMARY;
            u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);

            if (u32Ret == OLERR_NO_ERROR)
            {
                stockinfo->si_dbValue = _getStockVolatility(
                    stockinfo, buffer, total, VOLLTILITY_DAY_SUMMARY_COUNT);
                ppsiStock[nStock] = stockinfo;
                nStock ++;
            }

            stockinfo = getNextStockInfo(stockinfo);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        qsort(ppsiStock, nStock, sizeof(stock_info_t *), _analysisCompareStockValue);

        for (i = 0; i < nStock; i ++)
        {
            bTough = FALSE;
            ol_snprintf(
                dirpath, MAX_PATH_LEN, "%s%c%s",
                getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR,
                ppsiStock[i]->si_strCode);
            total = MAX_ANALYSIS_DAY_SUMMARY;
            u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);

            if (u32Ret == OLERR_NO_ERROR)
            {
                if (isShStockExchange(ppsiStock[i]->si_strCode))
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
                    ppsiStock[i], buffer, total, idxBuf, idxTotal, 1, &bTough);
            }

            if ((u32Ret == OLERR_NO_ERROR) && bTough)
            {
                u32Ret = _analysisStockGraph(
                    ppsiStock[i], buffer, total, &bTough,
                    VOLLTILITY_DAY_SUMMARY_COUNT);
            }

            /*continue if bTouch is true*/
            if ((u32Ret == OLERR_NO_ERROR) && bTough)
            {
                ppsiStockInfo[nStockInfo] = ppsiStock[i];
                nStockInfo ++;
                if (nStockInfo == *pnStockInfo)
                    break;
            }
        }

        *pnStockInfo = nStockInfo;
    }

    freeMemory((void **)&buffer);
    freeMemory((void **)&shiBuf);
    freeMemory((void **)&sziBuf);
    freeMemory((void **)&ppsiStock);

    return u32Ret;
}

static boolean_t _findInStockStatArbi(
    stock_info_t * stockinfo, stat_arbi_indu_result_t * result,
    stat_arbi_indu_result_entry_t ** ppEntry)
{
    olint_t i;
    stat_arbi_indu_result_entry_t * entry;

    for (i = 0; i < result->sair_nNumOfPair; i ++)
    {
        entry = &result->sair_psaireEntry[i];
        if (locateSubString(
                entry->saire_strStockPair, stockinfo->si_strCode, NULL) ==
            OLERR_NO_ERROR)
        {
            *ppEntry = entry;
            return TRUE;
        }
    }

    return FALSE;
}

static boolean_t _isDuplicateEntry(
    stat_arbi_indu_result_entry_t * entry,
    stat_arbi_indu_result_entry_t * pEntry[], olint_t nEntry)
{
    olint_t i;
    olchar_t code1[16], code2[16];

    memset(code1, 0, sizeof(code1));
    memset(code2, 0, sizeof(code2));

    ol_strncpy(code1, entry->saire_strStockPair, 8);
    ol_strncpy(code2, entry->saire_strStockPair + 9, 8);

    for (i = 0; i < nEntry; i ++)
    {
        if (locateSubString(
                pEntry[i]->saire_strStockPair, code1, NULL) == OLERR_NO_ERROR)
            return TRUE;
        if (locateSubString(
                pEntry[i]->saire_strStockPair, code2, NULL) == OLERR_NO_ERROR)
            return TRUE;
    }

    return FALSE;
}

static u32 _analysisStockPool(cli_analysis_param_t * pcap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    stock_info_t ** ppsiStockInfo = NULL;
    olint_t nStockInfo = 0, i;
    file_t fd = INVALID_FILE_VALUE;
    stat_arbi_indu_result_t result;
    stat_arbi_indu_result_entry_t * entry;
    stock_indu_info_t * induinfo;
#define MAX_ARBI_INDU_ENTRY 5
    olint_t nEntry = 0;
    stat_arbi_indu_result_entry_t * pEntry[MAX_ARBI_INDU_ENTRY];
#define MAX_STOCK_PAIR  2000

    nStockInfo = 100; //getNumOfStock();
    allocMemory(
        (void **)&ppsiStockInfo, sizeof(stock_info_t *) * nStockInfo, 0);

    memset(&result, 0, sizeof(stat_arbi_indu_result_t));
    result.sair_nMaxPair = MAX_STOCK_PAIR;
    allocMemory(
        (void **)&result.sair_psaireEntry,
        sizeof(stat_arbi_indu_result_entry_t) * result.sair_nMaxPair, 0);

    u32Ret = _analysisStockToughness(pcap, ppsiStockInfo, &nStockInfo);
    if ((u32Ret == OLERR_NO_ERROR) && (nStockInfo > 0))
    {
        for (i = 0; i < nStockInfo; i ++)
            cliengOutputLine("%s", ppsiStockInfo[i]->si_strCode);
        cliengOutputLine("Found %d stocks\n", nStockInfo);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _analysisStockStatArbi(pcap, &result);
    if ((u32Ret == OLERR_NO_ERROR) && (result.sair_nNumOfPair > 0))
    {
        for (i = 0; i < result.sair_nNumOfPair; i ++)
        {
            entry = &result.sair_psaireEntry[i];
            getIndustryInfo(entry->saire_nInduId, &induinfo);
            cliengOutputLine(
                "%s, %.2f, %s", entry->saire_strStockPair,
                entry->saire_dbCorrelation, induinfo->sii_pstrDesc);
        }
        cliengOutputLine("Found %d pairs stock", result.sair_nNumOfPair);
    }

    if ((u32Ret == OLERR_NO_ERROR) && (nStockInfo > 0))
    {
        for (i = 0; i < nStockInfo; i ++)
        {
            if (_findInStockStatArbi(ppsiStockInfo[i], &result, &entry) &&
                ! _isDuplicateEntry(entry, pEntry, nEntry))
            {
                getIndustryInfo(entry->saire_nInduId, &induinfo);
                cliengOutputLine(
                    "%s, %.2f, %s, %s", entry->saire_strStockPair,
                    entry->saire_dbCorrelation, induinfo->sii_pstrDesc,
                    ppsiStockInfo[i]->si_strCode);
                pEntry[nEntry] = entry;
                nEntry ++;
                if (nEntry == MAX_ARBI_INDU_ENTRY)
                    break;
            }
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = openFile2(
            STOCK_QUOTATION_LIST_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC,
            DEFAULT_CREATE_FILE_MODE, &fd);
        if (u32Ret == OLERR_NO_ERROR)
        {
            for (i = 0; i < nEntry; i ++)
            {
                writen(fd, pEntry[i]->saire_strStockPair, 17);
                writen(fd, "\n", 1);
            }

            closeFile(&fd);
        }
    }

    freeMemory((void **)&result.sair_psaireEntry);
    freeMemory((void **)&ppsiStockInfo);

    cliengOutputLine("");

    return u32Ret;
}

static oldouble_t _getIndexClosingPriceInc(
    cli_analysis_param_t * pcap, olchar_t * strstock)
{
    oldouble_t dbret = -9999.99;
    u32 u32Ret = OLERR_NO_ERROR;
    da_day_summary_t * buffer = NULL;
    olint_t total = MAX_NUM_OF_RESULT;
    olchar_t dirpath[MAX_PATH_LEN];
    da_day_summary_t * start, * end;

    ol_snprintf(
        dirpath, MAX_PATH_LEN, "%s%c%s",
        getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, strstock);

    allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);

    u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
    if (u32Ret == OLERR_NO_ERROR)
    {
        _getStartEndDaySummary(
            buffer, total, pcap->cap_pstrStartDate, pcap->cap_pstrEndDate, &start, &end);
        if (start == NULL)
            dbret = 0.0;
        else
            dbret = _getClosingPriceInc(start, end);
    }

    freeMemory((void **)&buffer);

    return dbret;
}

static oldouble_t _getStockClosingPriceInc(
    cli_analysis_param_t * pcap, da_day_summary_t * buffer, olint_t total)
{
    oldouble_t dbret = -9999.99;
    da_day_summary_t * start, * end;

    _getStartEndDaySummary(
        buffer, total, pcap->cap_pstrStartDate, pcap->cap_pstrEndDate, &start, &end);
    if (start == NULL)
        dbret = 0.0;
    else
        dbret = _getClosingPriceInc(start, end);

    return dbret;
}

static boolean_t _isStockHighlimit(
    cli_analysis_param_t * pcap, da_day_summary_t * buffer, olint_t total)
{
    boolean_t bRet = FALSE;
    da_day_summary_t * start, * end;

    _getStartEndDaySummary(
        buffer, total, pcap->cap_pstrStartDate, pcap->cap_pstrEndDate, &start, &end);
    if (start != NULL)
    {
        start = end;
        end = buffer + total - 1;
        while (start <= end)
        {
            if (start->dds_bHighHighLimit)
                return TRUE;

            start ++;
        }
    }

    return bRet;
}

static u32 _analysisIndexStock1(cli_analysis_param_t * pcap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    stock_info_t * stockinfo;
    oldouble_t shIndexInc, szIndexInc, stockInc;
    olint_t aboveSh, belowSh, aboveSz, belowSz;
    olint_t aboveShHl, belowShHl, aboveSzHl, belowSzHl;
    olchar_t dirpath[MAX_PATH_LEN];
    da_day_summary_t * buffer = NULL;
    olint_t total = MAX_NUM_OF_RESULT;
    oldouble_t thres = 1.3;



    if ((pcap->cap_pstrStartDate == NULL) || (pcap->cap_pstrEndDate == NULL))
        return OLERR_MISSING_PARAM;

    shIndexInc = _getIndexClosingPriceInc(pcap, SH_COMPOSITE_INDEX);
    szIndexInc = _getIndexClosingPriceInc(pcap, SZ_COMPOSITIONAL_INDEX);

    aboveSh = belowSh = aboveSz = belowSz = 0;
    aboveShHl = belowShHl = aboveSzHl = belowSzHl = 0;
    allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);

    stockinfo = getFirstStockInfo();
    while ((stockinfo != NULL) && (u32Ret == OLERR_NO_ERROR))
    {
        ol_snprintf(
            dirpath, MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, stockinfo->si_strCode);
        total = MAX_NUM_OF_RESULT;
        u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
        if (u32Ret == OLERR_NO_ERROR)
        {
            stockInc = _getStockClosingPriceInc(pcap, buffer, total);
            if (strncmp(stockinfo->si_strCode, "sh", 2) == 0)
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

        stockinfo = getNextStockInfo(stockinfo);
    }

    freeMemory((void **)&buffer);

    ol_printf(
        "From %s to %s, thres %.2f\n",
        pcap->cap_pstrStartDate, pcap->cap_pstrEndDate, thres);
    ol_printf(
        "Index %s, %.2f inc. %d stocks above, %d high limit. %d stocks below, %d high limit\n",
        SH_COMPOSITE_INDEX, shIndexInc, aboveSh, aboveShHl, belowSh, belowShHl);
    ol_printf(
        "Index %s, %.2f inc. %d stocks above, %d high limit. %d stocks below, %d high limit\n",
        SZ_COMPOSITIONAL_INDEX, szIndexInc, aboveSz, aboveSzHl, belowSz, belowSzHl);

    cliengOutputLine("");

    return u32Ret;
}

static u32 _analysisIndexStock2(cli_analysis_param_t * pcap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    stock_info_t * stockinfo;
    oldouble_t stockInc;
    olint_t aboveSh, aboveSz;
    olchar_t dirpath[MAX_PATH_LEN];
    da_day_summary_t * buffer = NULL;
    olint_t total = MAX_NUM_OF_RESULT;

    return u32Ret;

    if ((pcap->cap_pstrStartDate == NULL) || (pcap->cap_pstrEndDate == NULL))
        return OLERR_MISSING_PARAM;

    aboveSh = aboveSz = 0;
    allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);

    stockinfo = getFirstStockInfo();
    while ((stockinfo != NULL) && (u32Ret == OLERR_NO_ERROR))
    {
        ol_snprintf(
            dirpath, MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, stockinfo->si_strCode);
        total = MAX_NUM_OF_RESULT;
        u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
        if (u32Ret == OLERR_NO_ERROR)
        {
            stockInc = _getStockClosingPriceInc(pcap, buffer, total);
            if (strncmp(stockinfo->si_strCode, "sh", 2) == 0)
            {
                if (stockInc == 0.0)
                    ;
                else if (stockInc > 0)
                {
                    aboveSh ++;
                    ol_printf("%s\n", stockinfo->si_strCode);
                }
            }
            else
            {
                if (stockInc == 0.0)
                    ;
                else if (stockInc > 0)
                {
                    aboveSz ++;
                    ol_printf("%s\n", stockinfo->si_strCode);
                }
            }
        }

        stockinfo = getNextStockInfo(stockinfo);
    }

    freeMemory((void **)&buffer);

    ol_printf(
        "From %s to %s\n",
        pcap->cap_pstrStartDate, pcap->cap_pstrEndDate);
    ol_printf(
        "Index %s, %d stocks above\n",
        SH_COMPOSITE_INDEX, aboveSh);
    ol_printf(
        "Index %s, %d stocks above\n",
        SZ_COMPOSITIONAL_INDEX, aboveSz);

    cliengOutputLine("");

    return u32Ret;
}

static u32 _analysisIndexStock(cli_analysis_param_t * pcap)
{
    u32 u32Ret = OLERR_NO_ERROR;

    _analysisIndexStock1(pcap);

    _analysisIndexStock2(pcap);

    return u32Ret;
}

static u32 _analysisStockStatArbi2(cli_analysis_param_t * pcap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i;
    file_t fd = INVALID_FILE_VALUE;
    stat_arbi_indu_result_t result;
    stat_arbi_indu_result_entry_t * entry;
    stock_indu_info_t * induinfo;
#define MAX_STOCK_PAIR  2000

    memset(&result, 0, sizeof(stat_arbi_indu_result_t));
    result.sair_nMaxPair = MAX_STOCK_PAIR;
    allocMemory(
        (void **)&result.sair_psaireEntry,
        sizeof(stat_arbi_indu_result_entry_t) * result.sair_nMaxPair, 0);

    u32Ret = _analysisStockStatArbi(pcap, &result);
    if ((u32Ret == OLERR_NO_ERROR) && (result.sair_nNumOfPair > 0))
    {
        for (i = 0; i < result.sair_nNumOfPair; i ++)
        {
            entry = &result.sair_psaireEntry[i];
            getIndustryInfo(entry->saire_nInduId, &induinfo);
            cliengOutputLine(
                "%s, %.2f, %s", entry->saire_strStockPair,
                entry->saire_dbCorrelation, induinfo->sii_pstrDesc);
        }
        cliengOutputLine("Found %d pairs stock", result.sair_nNumOfPair);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = openFile2(
            STOCK_STAT_ARBI_LIST_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC,
            DEFAULT_CREATE_FILE_MODE, &fd);
        if (u32Ret == OLERR_NO_ERROR)
        {
            for (i = 0; i < result.sair_nNumOfPair; i ++)
            {
                entry = &result.sair_psaireEntry[i];
                writen(fd, entry->saire_strStockPair, 17);
                writen(fd, "\n", 1);
            }

            closeFile(&fd);
        }
    }

    freeMemory((void **)&result.sair_psaireEntry);

    cliengOutputLine("");

    return u32Ret;
}

static u32 _analysisStockLimit(cli_analysis_param_t * pcap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i;
    stat_arbi_indu_result_t result;
    stat_arbi_indu_result_entry_t * entry;
    stock_indu_info_t * induinfo;
    stat_arbi_indu_param_t saip;
    olchar_t dirpath[MAX_PATH_LEN];
    da_day_summary_t * end, * buffer;
    stock_info_t * stockinfo;
    olint_t total = MAX_NUM_OF_RESULT;

    allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);

    memset(&result, 0, sizeof(stat_arbi_indu_result_t));
    result.sair_nMaxPair = 100;
    allocMemory(
        (void **)&result.sair_psaireEntry,
        sizeof(stat_arbi_indu_result_entry_t) * result.sair_nMaxPair, 0);

    memset(&saip, 0, sizeof(stat_arbi_indu_param_t));
    saip.saip_dbMinCorrelation = 0.95;
    saip.saip_nDaySummary = NUM_OF_STAT_ARBI_DAY_SUMMARY + 10;
    saip.saip_nCorrelationArray = NUM_OF_STAT_ARBI_DAY_SUMMARY;

    stockinfo = getFirstStockInfo();
    while (stockinfo != NULL)
    {
        ol_snprintf(
            dirpath, MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR,
            stockinfo->si_strCode);
        total = MAX_ANALYSIS_DAY_SUMMARY;
        u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
        if (u32Ret == OLERR_NO_ERROR)
        {
            end = buffer + total - 1;
            if (! end->dds_bHighHighLimit)
            {
                stockinfo = getNextStockInfo(stockinfo);
                continue;
            }
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            result.sair_nNumOfPair = 0;
            u32Ret = statArbiStock(
                getEnvVar(ENV_VAR_DATA_PATH), stockinfo, &saip, &result);
        }

        if ((u32Ret == OLERR_NO_ERROR) && (result.sair_nNumOfPair > 0))
        {
            for (i = 0; i < result.sair_nNumOfPair; i ++)
            {
                entry = &result.sair_psaireEntry[i];
                ol_snprintf(
                    dirpath, MAX_PATH_LEN, "%s%c%s",
                    getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR,
                    entry->saire_strStockPair + 9);
                total = MAX_ANALYSIS_DAY_SUMMARY;
                u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
                if (u32Ret == OLERR_NO_ERROR)
                {
                    end = buffer + total - 1;
                    getIndustryInfo(entry->saire_nInduId, &induinfo);
                    cliengOutputLine(
                        "%s, %.2f, %s, %.2f", entry->saire_strStockPair,
                        entry->saire_dbCorrelation, induinfo->sii_pstrDesc,
                        end->dds_dbClosingPriceRate);
                }
            }
            cliengOutputLine("Found %d pairs stock", result.sair_nNumOfPair);
        }

        stockinfo = getNextStockInfo(stockinfo);
    }

    freeMemory((void **)&result.sair_psaireEntry);
    freeMemory((void **)&buffer);

    cliengOutputLine("");

    return u32Ret;
}

static void _addStockToList(
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
    _addStockToList(stocklist, numoflist, stockpair, &stockpair[9]);
    _addStockToList(stocklist, numoflist, &stockpair[9], stockpair);
}

static void _allocMemoryForStockList(olchar_t ** stocklist, olint_t num)
{
    olint_t i;

    for (i = 0; i < num; i ++)
    {
        allocMemory((void **)&stocklist[i], 200, 0);
    }

}

static void _freeMemoryForStockList(olchar_t ** stocklist, olint_t num)
{
    olint_t i;

    for (i = 0; i < num; i ++)
    {
        freeMemory((void **)&stocklist[i]);
    }

}

static boolean_t _analysisSectorStockToughness(
    cli_analysis_param_t * pcap, olchar_t * pstrStock,
    da_day_summary_t * shiBuf, olint_t shiTotal,
    da_day_summary_t * sziBuf, olint_t sziTotal)
{
    boolean_t bTough = FALSE;
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t dirpath[MAX_PATH_LEN];
    stock_info_t * stockinfo;
    da_day_summary_t * buffer = NULL;
    olint_t total = MAX_ANALYSIS_DAY_SUMMARY;
    da_day_summary_t * idxBuf;
    olint_t idxTotal;
#define VOLLTILITY_DAY_SUMMARY_COUNT  20

    allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);

    u32Ret = getStockInfo(pstrStock, &stockinfo);

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR,
            stockinfo->si_strCode);
        total = MAX_ANALYSIS_DAY_SUMMARY;
        u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);

        if (u32Ret == OLERR_NO_ERROR)
        {
            stockinfo->si_dbValue = _getStockVolatility(
                stockinfo, buffer, total, VOLLTILITY_DAY_SUMMARY_COUNT);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (isShStockExchange(stockinfo->si_strCode))
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

        if ((u32Ret == OLERR_NO_ERROR) && bTough)
        {
            u32Ret = _analysisStockGraph(
                stockinfo, buffer, total, &bTough,
                VOLLTILITY_DAY_SUMMARY_COUNT);
        }
    }

    freeMemory((void **)&buffer);

    return bTough;
}

static u32 _analysisSector(cli_analysis_param_t * pcap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i, j;
    stat_arbi_indu_result_t result;
    stat_arbi_indu_result_entry_t * entry;
    stat_arbi_indu_param_t saip;
    olint_t numofsector = 100;
    parse_sector_info_t psi[100];
    olint_t fd = INVALID_FILE_VALUE;
    olchar_t str[128];
    olsize_t size;
#define MAX_STOCKLIST  200
    olint_t numoflist = 0;
    olchar_t * stocklist[MAX_STOCKLIST];
    olchar_t dirpath[MAX_PATH_LEN];
    da_day_summary_t * shiBuf = NULL;
    olint_t shiTotal = MAX_ANALYSIS_DAY_SUMMARY;
    da_day_summary_t * sziBuf = NULL;
    olint_t sziTotal = MAX_ANALYSIS_DAY_SUMMARY;

    allocMemory((void **)&shiBuf, sizeof(da_day_summary_t) * shiTotal, 0);
    allocMemory((void **)&sziBuf, sizeof(da_day_summary_t) * sziTotal, 0);
    _allocMemoryForStockList(stocklist, MAX_STOCKLIST);

    memset(&result, 0, sizeof(stat_arbi_indu_result_t));
    result.sair_nMaxPair = 300;
    allocMemory(
        (void **)&result.sair_psaireEntry,
        sizeof(stat_arbi_indu_result_entry_t) * result.sair_nMaxPair, 0);

    ol_snprintf(
        dirpath, MAX_PATH_LEN, "%s%c%s",
        getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, SH_COMPOSITE_INDEX);
    u32Ret = readTradeDaySummaryWithFRoR(dirpath, shiBuf, &shiTotal);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, SZ_COMPOSITIONAL_INDEX);
        u32Ret = readTradeDaySummaryWithFRoR(dirpath, sziBuf, &sziTotal);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(psi, 0, sizeof(parse_sector_info_t) * numofsector);

        memset(&saip, 0, sizeof(stat_arbi_indu_param_t));
        saip.saip_dbMinCorrelation = 0.95;
        saip.saip_nDaySummary = NUM_OF_STAT_ARBI_DAY_SUMMARY + 10;
        saip.saip_nCorrelationArray = NUM_OF_STAT_ARBI_DAY_SUMMARY;
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = openFile2(
            STOCK_STAT_ARBI_LIST_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC,
            DEFAULT_CREATE_FILE_MODE, &fd);

    u32Ret = parseSectorDir("sector", psi, &numofsector);
    if (u32Ret == OLERR_NO_ERROR)
    {
        for (i = 0; i < numofsector; i ++)
        {
            cliengOutputLine("%s: %s", psi[i].psi_strName, psi[i].psi_pstrStocks);

            result.sair_nNumOfPair = 0;

            u32Ret = statArbiStockList(
                getEnvVar(ENV_VAR_DATA_PATH), psi[i].psi_pstrStocks,
                ol_strlen(psi[i].psi_pstrStocks) / 9, &saip, &result);
            if ((u32Ret == OLERR_NO_ERROR) && (result.sair_nNumOfPair > 0))
            {
/*
  qsort(
  result.sair_psaireEntry, result.sair_nNumOfPair,
  sizeof(stat_arbi_indu_result_entry_t),
  _analysisCompareStatArbiEntry);
*/
                numoflist = 0;
                for (j = 0; j < result.sair_nNumOfPair; j ++)
                {
                    entry = &result.sair_psaireEntry[j];
                    if (! _analysisSectorStockToughness(
                            pcap, entry->saire_strStockPair, shiBuf, shiTotal, sziBuf, sziTotal))
                        continue;
                    if (! _analysisSectorStockToughness(
                            pcap, &entry->saire_strStockPair[9], shiBuf, shiTotal, sziBuf, sziTotal))
                        continue;
                    cliengOutputLine(
                        "%s, %.2f", entry->saire_strStockPair,
                        entry->saire_dbCorrelation);
                    _strcatStock(stocklist, &numoflist, entry->saire_strStockPair);
                    /*get the first at least 10 stocks*/
                    if (numoflist >= MAX_STOCKLIST)
                        break;
                }
                cliengOutputLine("Found %d pairs stock", result.sair_nNumOfPair);

                if (numoflist > 0)
                {
                    size = ol_sprintf(str, "[%s]\n", psi[i].psi_strName);
                    writen(fd, str, size);
                }

                for (j = 0; j < numoflist; j ++)
                {
                    cliengOutputLine("%s", stocklist[j]);
                    writen(fd, stocklist[j], ol_strlen(stocklist[j]));
                    writen(fd, "\n", 1);
                }
                cliengOutputLine("Found %d stocks", numoflist);
            }
            cliengOutputLine("");
            freeSectorInfo(&psi[i]);
        }
    }

    if (fd != INVALID_FILE_VALUE)
        closeFile(&fd);

    freeMemory((void **)&result.sair_psaireEntry);
    _freeMemoryForStockList(stocklist, MAX_STOCKLIST);

    freeMemory((void **)&shiBuf);
    freeMemory((void **)&sziBuf);

    cliengOutputLine("");

    return u32Ret;
}

static u32 _analysisStockTough(cli_analysis_param_t * pcap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    stock_info_t ** ppsiStockInfo = NULL;
    olint_t nStockInfo = 0, i;
    file_t fd = INVALID_FILE_VALUE;

    nStockInfo = 10; //getNumOfStock();
    allocMemory(
        (void **)&ppsiStockInfo, sizeof(stock_info_t *) * nStockInfo, 0);

    u32Ret = _analysisStockToughness(pcap, ppsiStockInfo, &nStockInfo);
    if ((u32Ret == OLERR_NO_ERROR) && (nStockInfo > 0))
    {
        for (i = 0; i < nStockInfo; i ++)
            cliengOutputLine("%s", ppsiStockInfo[i]->si_strCode);
        cliengOutputLine("Found %d stocks\n", nStockInfo);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = openFile2(
            STOCK_TOUGH_LIST_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC,
            DEFAULT_CREATE_FILE_MODE, &fd);
        if (u32Ret == OLERR_NO_ERROR)
        {
            for (i = 0; i < nStockInfo; i ++)
            {
                writen(fd, ppsiStockInfo[i]->si_strCode, 8);
                writen(fd, "\n", 1);
            }

            closeFile(&fd);
        }
    }

    freeMemory((void **)&ppsiStockInfo);

//    cliengOutputLine("");

    return u32Ret;
}

#if 0
static oldouble_t _caluStockPriceVolumnCorrelation(
    stock_quo_t * psq, olint_t num, oldouble_t * pdba, oldouble_t * pdbb)
{
    oldouble_t dbr;
    olint_t i, max;
    quo_entry_t * start;

    max = num - 1;
    start = &psq->sq_pqeEntry[1];
    for (i = 0; i < max; i ++)
    {
        pdba[i] = start->qe_dbCurPrice;
        pdbb[i] = (oldouble_t)(start->qe_u64Volume - (start - 1)->qe_u64Volume);
        start ++;
    }

    getCorrelation(pdba, pdbb, max, &dbr);

    return dbr;
}

static void _analysisPrintQuotation(stock_quo_t * psq, quo_entry_t ** pqe, olint_t total)
{
    olint_t i;

    cliengOutputLine("Total %d points", total);
    for (i = 0; i < total; i ++)
    {
        cliengOutputLine(
            "%s %.2f", pqe[i]->qe_strTime, pqe[i]->qe_dbCurPrice);
    }
    cliengOutputLine("");
}
#endif

static boolean_t _analysisStockPriceVolumnUp(
    stock_quo_t * psq, olint_t num, quo_entry_t ** pqe, olint_t total, oldouble_t * pdba)
{
    quo_entry_t * end;
    oldouble_t db1, db2;

    return FALSE;

    if (total < 4)
        return FALSE;

    if (pqe[total - 1]->qe_dbCurPrice <= pqe[total - 3]->qe_dbCurPrice)
        return FALSE;
    if (pqe[total - 2]->qe_dbCurPrice <= pqe[total - 4]->qe_dbCurPrice)
        return FALSE;
    if (pqe[total - 1]->qe_dbCurPrice > pqe[total - 2]->qe_dbCurPrice)
        return FALSE;

    if (pdba[total - 2] <= pdba[total - 4])
        return FALSE;

    end = &psq->sq_pqeEntry[num - 1];
    if (end == pqe[total - 1])
        return FALSE;

    db1 = pqe[total - 1]->qe_dbCurPrice - pqe[total - 2]->qe_dbCurPrice;
    db2 = end->qe_dbCurPrice - pqe[total - 1]->qe_dbCurPrice;
    if ((db2 < db1 * 0.1) || (db2 > db1 * 0.2))
        return FALSE;

    return TRUE;
}

static boolean_t _analysisStockPriceVolumnBottom(
    stock_quo_t * psq, olint_t num, quo_entry_t ** pqe, olint_t total, oldouble_t * pdba)
{
    quo_entry_t * end;
    oldouble_t db1, db2;

    if (total < 6)
        return FALSE;

    if (pqe[total - 1]->qe_dbCurPrice > pqe[total - 2]->qe_dbCurPrice)
        return FALSE;
    if (pqe[total - 1]->qe_dbCurPrice <= pqe[total - 3]->qe_dbCurPrice)
        return FALSE;

    if (pdba[total - 1] <= pdba[total - 3])
        return FALSE;

    end = &psq->sq_pqeEntry[num - 1];
    if (end == pqe[total - 1])
        return FALSE;

//    db1 = pqe[total - 2]->qe_dbCurPrice - pqe[total - 1]->qe_dbCurPrice;
    db1 = end->qe_dbHighPrice - end->qe_dbLowPrice;
    db2 = end->qe_dbCurPrice - pqe[total - 1]->qe_dbCurPrice;
    if ((db2 < db1 * 0.1) || (db2 > db1 * 0.2))
        return FALSE;
/*
    if ((pqe[total - 2]->qe_dbCurPrice > pqe[total - 4]->qe_dbCurPrice) &&
        (pdba[total - 2] <= pdba[total - 4]))
        return FALSE;
*/
    /*the inflextion poolint_t before the bottom*/
    if (pqe[total - 5]->qe_dbCurPrice < pqe[total - 3]->qe_dbCurPrice)
        return FALSE;
    if (pqe[total - 6]->qe_dbCurPrice < pqe[total - 4]->qe_dbCurPrice)
        return FALSE;

    if (pdba[total - 3] > pdba[total - 5])
        return FALSE;

    return TRUE;
}

/* ((close - low) - (high - close)) / (high - low) * V */
static void _analysisStockPriceVolumnObv(
    stock_quo_t * psq, olint_t num, quo_entry_t ** pqe, olint_t total, oldouble_t * pdba)
{
    olint_t i;

    pdba[0] = (oldouble_t)pqe[0]->qe_u64Volume;
    for (i = 1; i < total; i ++)
    {
        pdba[i] = (oldouble_t)(pqe[i]->qe_u64Volume - pqe[i - 1]->qe_u64Volume);
        if (pqe[i]->qe_dbCurPrice > pqe[i - 1]->qe_dbCurPrice)
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
        cliengOutputLine(
            "%s, %.2f, %.2f", pqe[i]->qe_strTime, pqe[i]->qe_dbCurPrice, pdba[i]);
    }
#endif
}

static olint_t _getNextTopBottomQuoEntry(
    quo_entry_t ** pqe, olint_t total, olint_t start, boolean_t bUp)
{
    olint_t next, cur;
#if 0
    oldouble_t db;

    if (bUp)
        db = pqe[start]->qe_dbCurPrice * (1.0 + 0.1);
    else
        db = pqe[start]->qe_dbCurPrice * (1.0 - 0.1);
#endif
    cur = start;
    next = cur + 1;
    while (cur <= total)
    {
        if (bUp)
        {
            if (pqe[cur]->qe_dbCurPrice > pqe[next]->qe_dbCurPrice)
                next = start;

        }

        cur ++;
    }

    return next;
}

static boolean_t _analysisStockPriceVolumn(
    stock_quo_t * psq, olint_t num, quo_entry_t ** pqe, olint_t total)
{
    oldouble_t pdba[500];
    quo_entry_t * low;
    olint_t cur, next;
    boolean_t bUp;

    assert(total < 500);

    if (total < 3)
        return FALSE;

    _analysisStockPriceVolumnObv(psq, num, pqe, total, pdba);

    cur = next = 1;
    while (next <= total)
    {
        if (pqe[cur - 1]->qe_dbCurPrice <= pqe[cur]->qe_dbCurPrice)
            bUp = TRUE;
        else
            bUp = FALSE;


        next = _getNextTopBottomQuoEntry(pqe, total, cur - 1, bUp);

        cur ++;
    }



    return TRUE;

    low = getQuoEntryWithLowestPrice(psq->sq_pqeEntry, pqe[total - 1]);
    if (low == pqe[total - 3])
    {
        if (! _analysisStockPriceVolumnBottom(psq, num, pqe, total, pdba))
            return FALSE;
    }
    else if (low != pqe[total - 3])
    {
        if (! _analysisStockPriceVolumnUp(psq, num, pqe, total, pdba))
            return FALSE;
    }
    else
        return FALSE;

    return TRUE;
}

static u32 _analysisStockIndexQuotation(
    stock_quo_t * psq, stock_quo_t * psqi)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i, total;
    oldouble_t * pdba = NULL, * pdbb = NULL; //, dbr1, dbr2;
    quo_entry_t ** pqe = NULL;
//    quo_entry_t * low;

    allocMemory((void **)&pdba, sizeof(oldouble_t) * psq->sq_nMaxEntry, 0);
    allocMemory((void **)&pdbb, sizeof(oldouble_t) * psq->sq_nMaxEntry, 0);
    allocMemory((void **)&pqe, sizeof(quo_entry_t *) * psqi->sq_nNumOfEntry, 0);

    for (i = 469; i < psqi->sq_nNumOfEntry; i ++)
    {
        if (strcmp(psqi->sq_pqeEntry[i].qe_strTime, "09:50:00") < 0)
            continue;

//        if (psq->sq_pqeEntry[i].qe_dbCurPrice <= psq->sq_dbLastClosingPrice)
//            continue;

//        dbr1 = _caluStockPriceVolumnCorrelation(psq, i + 1, pdba, pdbb);
//        if (dbr1 <= 0)
//            continue;

        total = psqi->sq_nNumOfEntry;
        getQuoEntryInflexionPoint(
            psqi->sq_pqeEntry, i + 1, pqe, &total);

        if (! _analysisStockPriceVolumn(psqi, i + 1, pqe, total))
            continue;

/*
        dbr2 = _caluStockPriceVolumnCorrelation(psqi, i + 1, pdba, pdbb);
        if (dbr2 <= 0)
        {
            low = getQuoEntryWithLowestPrice(psq->sq_pqeEntry, pqe[total - 3]);
            if (low != pqe[total - 3])
                continue;
        }
*/
        cliengPrintDivider();
        cliengOutputLine(
            "%s, %.2f, %d", psqi->sq_pqeEntry[i].qe_strTime, psqi->sq_pqeEntry[i].qe_dbCurPrice, i);
//        cliengOutputLine("Stock price and volumn: %.2f", dbr1);
//        cliengOutputLine("Index price and volumn: %.2f", dbr2);
//        _analysisPrintQuotation(psqi, pqe, total);
        break;
    }

    freeMemory((void **)&pdba);
    freeMemory((void **)&pdbb);
    freeMemory((void **)&pqe);

    return u32Ret;
}

static u32 _analysisFillStockQuotation(
    stock_quo_t * psq, stock_info_t * stockinfo)
{
    u32 u32Ret = OLERR_NO_ERROR;
    da_day_summary_t * buffer = NULL;
    olint_t total = 100;
    olchar_t dirpath[MAX_PATH_LEN];
    da_day_summary_t * end;

    ol_snprintf(
        dirpath, MAX_PATH_LEN, "%s%c%s",
        getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, stockinfo->si_strCode);

    allocMemory((void **)&buffer, sizeof(da_day_summary_t) * total, 0);

    u32Ret = readTradeDaySummary(dirpath, buffer, &total);
    if (u32Ret == OLERR_NO_ERROR)
    {
        getDaySummaryWithDate(buffer, total, psq->sq_strDate, &end);
        end --;

        psq->sq_dbLastClosingPrice = end->dds_dbClosingPrice;
    }

    freeMemory((void **)&buffer);

    return u32Ret;
}

static u32 _analysisStockQuotation(cli_analysis_param_t * pcap, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t * pstrIndex;
    stock_quo_t stockquo, stockquoindex;
    olchar_t dirpath[MAX_PATH_LEN];
    stock_info_t * stockinfo;

    if (isStockInfoIndex(pcap->cap_pstrDir[0]))
    {
        cliengOutputLine("Cannot analysis the quotation of stock index");
        return OLERR_INVALID_PARAM;
    }

    memset(&stockquo, 0, sizeof(stock_quo_t));
    memset(&stockquoindex, 0, sizeof(stock_quo_t));

    if (isShStockExchange(pcap->cap_pstrDir[0]))
        pstrIndex = SH_COMPOSITE_INDEX;
    else
        pstrIndex = SZ_COMPOSITIONAL_INDEX;

    u32Ret = getStockInfo(pcap->cap_pstrDir[0], &stockinfo);
    if (u32Ret == OLERR_NO_ERROR)
    {
        stockquo.sq_nMaxEntry = 3500;
        ol_strcpy(stockquo.sq_strCode, stockinfo->si_strCode);
        if (pcap->cap_pstrStartDate != NULL)
            ol_strcpy(stockquo.sq_strDate, pcap->cap_pstrStartDate);

        u32Ret = xmalloc(
            (void **)&stockquo.sq_pqeEntry,
            sizeof(quo_entry_t) * stockquo.sq_nMaxEntry);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcap->cap_pstrDir[0]);

        u32Ret = readStockQuotationFile(
            dirpath, &stockquo);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _analysisFillStockQuotation(&stockquo, stockinfo);

    if (u32Ret == OLERR_NO_ERROR)
    {
        stockquoindex.sq_nMaxEntry = 3500;
        ol_strcpy(stockquoindex.sq_strCode, pstrIndex);
        if (pcap->cap_pstrStartDate != NULL)
            ol_strcpy(stockquoindex.sq_strDate, pcap->cap_pstrStartDate);

        u32Ret = xmalloc(
            (void **)&stockquoindex.sq_pqeEntry,
            sizeof(quo_entry_t) * stockquoindex.sq_nMaxEntry);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, pstrIndex);

        u32Ret = readStockQuotationFile(
            dirpath, &stockquoindex);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _analysisStockIndexQuotation(&stockquo, &stockquoindex);
    }

    if (stockquo.sq_pqeEntry != NULL)
        xfree((void **)&stockquo.sq_pqeEntry);
    if (stockquoindex.sq_pqeEntry != NULL)
        xfree((void **)&stockquoindex.sq_pqeEntry);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 processAnalysis(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_analysis_param_t * pcap = (cli_analysis_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcap->cap_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _analysisHelp(pdm);
    else if (*getEnvVar(ENV_VAR_DATA_PATH) == '\0')
    {
        cliengOutputLine("Data path is not set.");
        u32Ret = OLERR_NOT_READY;
    }
    else if (pcap->cap_u8Action == CLI_ACTION_ANALYSIS_STOCK)
    {
        if (pcap->cap_nDir == 0)
            u32Ret = OLERR_MISSING_PARAM;
        else
            u32Ret = _analysisStock(pcap, pdm);
    }
    else if (pcap->cap_u8Action == CLI_ACTION_ANALYSIS_STOCK_QUOTATION)
        u32Ret = _analysisStockQuotation(pcap, pdm);
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
        u32Ret = OLERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamAnalysis(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_analysis_param_t * pcap = (cli_analysis_param_t *)pParam;

    memset(pcap, 0, sizeof(cli_analysis_param_t));
    pcap->cap_u8Action = CLI_ACTION_ANALYSIS_SECTOR;

    return u32Ret;
}

u32 parseAnalysis(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_analysis_param_t * pcap = (cli_analysis_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "atbns:q:if:l:hv?")) != -1) && (u32Ret == OLERR_NO_ERROR))
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
            u32Ret = OLERR_MISSING_PARAM;
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
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

void __toSuppressWarnMsg(void)
{
    cli_analysis_param_t * pcap = NULL;
    stock_info_t * stockinfo = NULL; 
    da_day_summary_t * buffer = NULL;
    olint_t total = MAX_NUM_OF_RESULT;

    _analysisDaySummary_OLD(pcap, stockinfo, buffer, total);
}

/*---------------------------------------------------------------------------*/

