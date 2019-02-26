/**
 *  @file parse.c
 *
 *  @brief The parse command
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
#include "datastat.h"
#include "clieng.h"
#include "regression.h"
#include "damodel.h"
#include "stocklist.h"
#include "matrix.h"
#include "jiukun.h"
#include "envvar.h"

/* --- private data/data structure section --------------------------------- */

#define MAX_NUM_OF_RESULT  600
#define MAX_NUM_OF_TRADE_DETAIL_FILE  100

static clieng_caption_t ls_ccTradeDayDetailBrief[] =
{
    {"Day", 5}, 
    {"Date", 12},
    {"Buy", 16},
    {"Sold", 16},
    {"LambBuy", 10},
    {"LambSold", 10},
    {"Close", 6},
};

static clieng_caption_t ls_ccTradeDaySummaryBrief[] =
{
    {"Day", 5}, 
    {"Date", 12},
    {"OpenP", 7},
    {"CloseP", 7},
    {"HighP", 7},
    {"LowP", 7},
    {"Close", 6},
    {"Capital", 9},
    {"Share", 9},
	{"Status", 7},
};

static clieng_caption_t ls_ccQuoEntryBrief[] =
{
    {"Time", 11},
    {"CurP", 9}, 
    {"HighP", 9},
    {"LowP", 9},
    {"Volumn", 12},
    {"Amount", 12},
};

static clieng_caption_t ls_ccStockQuoVerbose[] =
{
    {"Name", CLIENG_CAP_HALF_LINE}, {"Date", CLIENG_CAP_HALF_LINE},
    {"OpeningPrice", CLIENG_CAP_HALF_LINE}, {"LastClosingPrice", CLIENG_CAP_HALF_LINE},
    {"MaxEntry", CLIENG_CAP_HALF_LINE}, {"NumOfEntry", CLIENG_CAP_HALF_LINE},
};

static clieng_caption_t ls_ccDataVerbose[] =
{
    {"Day", CLIENG_CAP_HALF_LINE}, {"Date", CLIENG_CAP_HALF_LINE},

    {"OpeningPrice", CLIENG_CAP_HALF_LINE}, {"ClosingPrice", CLIENG_CAP_HALF_LINE},
    {"HighPrice", CLIENG_CAP_HALF_LINE}, {"LowPrice", CLIENG_CAP_HALF_LINE},
    {"ClosingPriceInc", CLIENG_CAP_HALF_LINE}, {"ClosingPriceDec", CLIENG_CAP_HALF_LINE},
    {"HighPriceInc", CLIENG_CAP_HALF_LINE}, {"LowPriceDec", CLIENG_CAP_HALF_LINE},
    {"CloseHighLimit", CLIENG_CAP_HALF_LINE}, {"TimeCloseHighLimit", CLIENG_CAP_HALF_LINE},
    {"CloseLowLimit", CLIENG_CAP_HALF_LINE}, {"TimeCloseLowLimit", CLIENG_CAP_HALF_LINE},
    {"Gap", CLIENG_CAP_FULL_LINE},

    {"All", CLIENG_CAP_FULL_LINE},
    {"Buy", CLIENG_CAP_HALF_LINE}, {"PercentOfBuy", CLIENG_CAP_HALF_LINE},
    {"PercentOfBuyInAM", CLIENG_CAP_FULL_LINE},
    {"Sold", CLIENG_CAP_HALF_LINE}, {"PercentOfSold", CLIENG_CAP_HALF_LINE},
    {"PercentOfSoldInAM", CLIENG_CAP_FULL_LINE},
    {"LambBuy", CLIENG_CAP_HALF_LINE}, {"PercentOfLambBuy", CLIENG_CAP_HALF_LINE},
    {"PercentOfLambBuyBelow100", CLIENG_CAP_FULL_LINE},
    {"LambSold", CLIENG_CAP_HALF_LINE}, {"PercentOfLambSold", CLIENG_CAP_HALF_LINE},
    {"PercentOfLambSoldBelow100", CLIENG_CAP_FULL_LINE},
    {"Na", CLIENG_CAP_HALF_LINE}, {"LambNa", CLIENG_CAP_HALF_LINE},
    {"AboveLastClosingPrice", CLIENG_CAP_HALF_LINE}, {"AboveLastClosingPricePercent", CLIENG_CAP_HALF_LINE},
    {"CloseHighLimitSold", CLIENG_CAP_HALF_LINE}, {"PercentOfCloseHighLimitSold", CLIENG_CAP_HALF_LINE},
    {"CloseHighLimitLambSold", CLIENG_CAP_HALF_LINE}, {"PercentOfCloseHighLimitLambSold", CLIENG_CAP_HALF_LINE},
    {"CloseLowLimitBuy", CLIENG_CAP_HALF_LINE}, {"PercentOfCloseLowLimitBuy", CLIENG_CAP_HALF_LINE},
    {"CloseLowLimitLambBuy", CLIENG_CAP_HALF_LINE}, {"PercentOfCloseLowLimitLambBuy", CLIENG_CAP_HALF_LINE},
    {"VolumeRatio", CLIENG_CAP_FULL_LINE},
    {"SoldVolumeRatio", CLIENG_CAP_HALF_LINE}, {"LambSoldVolumeRatio", CLIENG_CAP_HALF_LINE},

    {"AllA", CLIENG_CAP_FULL_LINE},
    {"BuyA", CLIENG_CAP_HALF_LINE}, {"SoldA", CLIENG_CAP_HALF_LINE},
    {"LambBuyA", CLIENG_CAP_HALF_LINE}, {"LambSoldA", CLIENG_CAP_HALF_LINE},
    {"CloseHighLimitSoldA", CLIENG_CAP_HALF_LINE}, {"CloseHighLimitLambSoldA", CLIENG_CAP_HALF_LINE},
    {"CloseLowLimitBuyA", CLIENG_CAP_HALF_LINE}, {"CloseLowLimitLambBuyA", CLIENG_CAP_HALF_LINE},

    {"UpperShadowRatio", CLIENG_CAP_HALF_LINE}, {"LowerShadowRatio", CLIENG_CAP_HALF_LINE},

    {"LastTimeOfLowPrice", CLIENG_CAP_HALF_LINE}, {"LastLowPrice", CLIENG_CAP_HALF_LINE},
    {"LastLowPriceInc", CLIENG_CAP_FULL_LINE},
};

static clieng_caption_t ls_ccDaySummaryVerbose[] =
{
    {"Day", CLIENG_CAP_HALF_LINE}, {"Date", CLIENG_CAP_HALF_LINE},

    {"OpeningPrice", CLIENG_CAP_HALF_LINE}, {"ClosingPrice", CLIENG_CAP_HALF_LINE},
    {"HighPrice", CLIENG_CAP_HALF_LINE}, {"LowPrice", CLIENG_CAP_HALF_LINE},
    {"LastClosingPrice", CLIENG_CAP_FULL_LINE},
    {"ClosingPriceRate", CLIENG_CAP_HALF_LINE}, {"HighPriceRate", CLIENG_CAP_HALF_LINE},
    {"CloseHighLimit", CLIENG_CAP_HALF_LINE}, {"CloseLowLimit", CLIENG_CAP_HALF_LINE},
    {"HighHighLimit", CLIENG_CAP_HALF_LINE}, {"LowLowLimit", CLIENG_CAP_HALF_LINE},
    {"Gap", CLIENG_CAP_FULL_LINE},

    {"All", CLIENG_CAP_HALF_LINE}, {"VolumeRatio", CLIENG_CAP_HALF_LINE},
    {"TurnoverRate", CLIENG_CAP_FULL_LINE},

    {"AllA", CLIENG_CAP_FULL_LINE},

    {"UpperShadowRatio", CLIENG_CAP_HALF_LINE}, {"LowerShadowRatio", CLIENG_CAP_HALF_LINE},

    {"GeneralCapital", CLIENG_CAP_HALF_LINE}, {"TradableShare", CLIENG_CAP_HALF_LINE},
    {"Status", CLIENG_CAP_FULL_LINE},
};

/* --- private routine section---------------------------------------------- */
static u32 _parseHelp(da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputRawLine2("\
Parse stock data.\n\
parse [-s stock] [-f] [-u] [-d date] [-o] \n\
      [-e stock] [-t threshold] \n\
      [-q stock] \n\
      [-d start-date] [-l count] [-h] [-v]");
    cliengOutputRawLine2("\
  -s: parse trade summary files.\n\
  -f: parse files from specified date, return NULL if the date is not found\n\
      in the file. If '-d' is not specified, the date will be the first day\n\
      in the file.\n\
  -u: parse files until specified date, return NULL if the date is not found\n\
      in the file. If '-d' is not specified, the date will be the last day \n\
      in the file.\n\
  -o: forward restoration of right, used with '-s'.");
    cliengOutputRawLine2("\
  -q: parse quotation files, use '-f' to specify the date, use last day if\n\
      '-f' is not specified.");
    cliengOutputRawLine2("\
  -e: parse trade detail files.\n\
  -t: specify the threshold, used with '-e'.");
    cliengOutputRawLine2("\
  -l: read last N days.\n\
  -d: the start date.\n\
  -v: verbose.");
    cliengOutputLine("");

    return u32Ret;
}

static void _printTradeDayDetailBrief(da_day_result_t * cur)
{
    clieng_caption_t * pcc = &ls_ccTradeDayDetailBrief[0];
    olchar_t strInfo[MAX_OUTPUT_LINE_LEN], strField[MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Day */
    ol_sprintf(strField, "%d", cur->ddr_nIndex);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Date */
    cliengAppendBriefColumn(pcc, strInfo, cur->ddr_strDate);
    pcc++;

    /* Buy */
    ol_sprintf(strField, "%llu",
            cur->ddr_u64Buy + cur->ddr_u64CloseHighLimitSold +
            cur->ddr_u64CloseHighLimitLambSold);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Sold */
    ol_sprintf(strField, "%llu",
            cur->ddr_u64Sold + cur->ddr_u64CloseLowLimitBuy +
            cur->ddr_u64CloseLowLimitLambBuy);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambBuy */
    ol_sprintf(strField, "%llu", cur->ddr_u64LambBuy);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambSold */
    ol_sprintf(strField, "%llu", cur->ddr_u64LambSold);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosePriceInc */
    ol_sprintf(strField, "%.2f",
            cur->ddr_dbClosingPriceInc - cur->ddr_dbClosingPriceDec);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    cliengOutputLine(strInfo);
}

static void _printTradeDayDetailVerbose(da_day_result_t * result)
{
    clieng_caption_t * pcc = &ls_ccDataVerbose[0];
    olchar_t strLeft[MAX_OUTPUT_LINE_LEN], strRight[MAX_OUTPUT_LINE_LEN];

    cliengPrintDivider();

    /*Day*/
    ol_sprintf(strLeft, "%d", result->ddr_nIndex);
    cliengPrintTwoHalfLine(pcc, strLeft, result->ddr_strDate);
    pcc += 2;

    /*OpeningPrice*/
    ol_sprintf(strLeft, "%.2f", result->ddr_dbOpeningPrice);
    ol_sprintf(strRight, "%.2f", result->ddr_dbClosingPrice);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*HighPrice*/
    ol_sprintf(strLeft, "%.2f", result->ddr_dbHighPrice);
    ol_sprintf(strRight, "%.2f", result->ddr_dbLowPrice);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*ClosingPriceInc*/
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbClosingPriceInc);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbClosingPriceDec);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*HighPriceInc*/
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbHighPriceInc);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbLowPriceDec);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimit*/
    ol_sprintf(strLeft, "%s", getStringPositive(result->ddr_bCloseHighLimit));
    cliengPrintTwoHalfLine(pcc, strLeft, result->ddr_strTimeCloseHighLimit);
    pcc += 2;

    /*CloseLowLimit*/
    ol_sprintf(strLeft, "%s", getStringPositive(result->ddr_bCloseLowLimit));
    cliengPrintTwoHalfLine(pcc, strLeft, result->ddr_strTimeCloseLowLimit);
    pcc += 2;

    /*Gap*/
    ol_sprintf(strLeft, "%s", getStringPositive(result->ddr_bGap));
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*All*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64All);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Buy*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64Buy);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbBuyPercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*PercentOfBuyInAM*/
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbBuyInAmPercent);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Sold*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64Sold);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbSoldPercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*PercentOfSoldInAM*/
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbSoldInAmPercent);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*LambBuy*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64LambBuy);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbLambBuyPercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*PercentOfLambBuyBelow100*/
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbLambBuyBelow100Percent);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*LambSold*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64LambSold);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbLambSoldPercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*PercentOfLambSoldBelow100*/
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbLambSoldBelow100Percent);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Na*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64Na);
    ol_sprintf(strRight, "%llu", result->ddr_u64LambNa);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*AboveLastClosingPrice*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64AboveLastClosingPrice);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbAboveLastClosingPricePercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimitSold*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64CloseHighLimitSold);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbCloseHighLimitSoldPercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimitLambSold*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64CloseHighLimitLambSold);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbCloseHighLimitLambSoldPercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseLowLimitBuy*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64CloseLowLimitBuy);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbCloseLowLimitBuyPercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseLowLimitLambBuy*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64CloseLowLimitLambBuy);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbCloseLowLimitLambBuyPercent);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*VolumeRatio*/
    ol_sprintf(strLeft, "%.2f", result->ddr_dbVolumeRatio);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*SoldVolumeRatio*/
    ol_sprintf(strLeft, "%.2f", result->ddr_dbSoldVolumeRatio);
    ol_sprintf(strRight, "%.2f", result->ddr_dbLambSoldVolumeRatio);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*AllA*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64AllA);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*BuyA*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64BuyA);
    ol_sprintf(strRight, "%llu", result->ddr_u64SoldA);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*LambBuyA*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64LambBuyA);
    ol_sprintf(strRight, "%llu", result->ddr_u64LambSoldA);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimitSoldA*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64CloseHighLimitSoldA);
    ol_sprintf(strRight, "%llu", result->ddr_u64CloseHighLimitLambSoldA);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseLowLimitBuyA*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64CloseLowLimitBuyA);
    ol_sprintf(strRight, "%llu", result->ddr_u64CloseLowLimitLambBuyA);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*UpperShadowRatio*/
    ol_sprintf(strLeft, "%.2f", result->ddr_dbUpperShadowRatio);
    ol_sprintf(strRight, "%.2f", result->ddr_dbLowerShadowRatio);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* LastTimeOfLowPrice */
    ol_sprintf(strRight, "%.2f", result->ddr_dbLastXLowPrice);
    cliengPrintTwoHalfLine(pcc, result->ddr_strLastTimeLowPrice, strRight);
    pcc += 2;

    /* LastLowPriceInc */
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbLastXInc);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

}

static void _printTradeDetailFile(
    cli_parse_param_t * pcpp, da_day_result_t * buffer, olint_t total)
{
    da_day_result_t * cur;
    olint_t i;

    if (! pcpp->cpp_bVerbose)
    {
        cliengPrintDivider();

        cliengPrintHeader(
            ls_ccTradeDayDetailBrief,
            sizeof(ls_ccTradeDayDetailBrief) / sizeof(clieng_caption_t));
    }

    cur = buffer;
    for (i = 0; i < total; i++)
    {
        if (pcpp->cpp_bVerbose)
            _printTradeDayDetailVerbose(cur);
        else
            _printTradeDayDetailBrief(cur);
        cur ++;
    }

    cliengOutputLine("");
}

static u32 _readTradeDetailFile(cli_parse_param_t * pcpp, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t total = MAX_NUM_OF_TRADE_DETAIL_FILE;
    da_day_result_t * buffer = NULL;
    parse_param_t pp;
    olchar_t dirpath[MAX_PATH_LEN];
    stock_info_t * stockinfo;

    u32Ret = xmalloc((void **)&buffer, sizeof(da_day_result_t) * total);
    if (u32Ret != OLERR_NO_ERROR)
        return u32Ret;

    u32Ret = getStockInfo(pcpp->cpp_pstrStock, &stockinfo);
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(&pp, 0, sizeof(pp));
        pp.pp_nThres = pcpp->cpp_nThres;
        if (pp.pp_nThres == 0)
            pp.pp_nThres = getStockShareThres(stockinfo);
        pp.pp_nStart = 1; //pcpp->cpp_nStart;
        pp.pp_nEnd = pp.pp_nStart + total - 1; //pcpp->cpp_nEnd;
        pp.pp_nLastCount = pcpp->cpp_nLastCount;
        pp.pp_pstrDateFrom = pcpp->cpp_pstrDate;
        ol_strcpy(pp.pp_strLastX, "14:45:00");
        ol_snprintf(
            dirpath, MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcpp->cpp_pstrStock);

        u32Ret = readTradeDayDetail(dirpath, &pp, buffer, &total);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        _printTradeDetailFile(pcpp, buffer, total);
    }

    if (buffer != NULL)
        xfree((void **)&buffer);

    return u32Ret;
}

static void _printTradeDaySummaryBrief(da_day_summary_t * cur)
{
    clieng_caption_t * pcc = &ls_ccTradeDaySummaryBrief[0];
    olchar_t strInfo[MAX_OUTPUT_LINE_LEN], strField[MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Day */
    ol_sprintf(strField, "%d", cur->dds_nIndex);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Date */
    cliengAppendBriefColumn(pcc, strInfo, cur->dds_strDate);
    pcc++;

    /* OpeningPrice */
    ol_sprintf(strField, "%.2f", cur->dds_dbOpeningPrice);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosingPrice */
    ol_sprintf(strField, "%.2f", cur->dds_dbClosingPrice);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* HighPrice */
    ol_sprintf(strField, "%.2f", cur->dds_dbHighPrice);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LowPrice */
    ol_sprintf(strField, "%.2f", cur->dds_dbLowPrice);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosePriceRate */
    ol_sprintf(strField, "%.2f", cur->dds_dbClosingPriceRate);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* GeneralCapital */
    ol_sprintf(strField, "%lld", cur->dds_u64GeneralCapital / 10000);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* TradableShare */
    ol_sprintf(strField, "%lld", cur->dds_u64TradableShare / 10000);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Status*/
    getStringDaySummaryStatus(cur, strField);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    cliengOutputLine(strInfo);
}

static void _printDaDaySummaryVerbose(da_day_summary_t * summary)
{
    clieng_caption_t * pcc = &ls_ccDaySummaryVerbose[0];
    olchar_t strLeft[MAX_OUTPUT_LINE_LEN], strRight[MAX_OUTPUT_LINE_LEN];

    cliengPrintDivider();

    /*Day*/
    ol_sprintf(strLeft, "%d", summary->dds_nIndex);
    cliengPrintTwoHalfLine(pcc, strLeft, summary->dds_strDate);
    pcc += 2;

    /*OpeningPrice*/
    ol_sprintf(strLeft, "%.2f", summary->dds_dbOpeningPrice);
    ol_sprintf(strRight, "%.2f", summary->dds_dbClosingPrice);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*HighPrice*/
    ol_sprintf(strLeft, "%.2f", summary->dds_dbHighPrice);
    ol_sprintf(strRight, "%.2f", summary->dds_dbLowPrice);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*LastClosingPrice*/
    ol_sprintf(strLeft, "%.2f", summary->dds_dbLastClosingPrice);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*ClosingPriceRate*/
    ol_sprintf(strLeft, "%.2f%%", summary->dds_dbClosingPriceRate);
    ol_sprintf(strRight, "%.2f%%", summary->dds_dbHighPriceRate);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimit*/
    ol_sprintf(strLeft, "%s", getStringPositive(summary->dds_bCloseHighLimit));
    ol_sprintf(strRight, "%s", getStringPositive(summary->dds_bCloseLowLimit));
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*HighHighLimit*/
    ol_sprintf(strLeft, "%s", getStringPositive(summary->dds_bHighHighLimit));
    ol_sprintf(strRight, "%s", getStringPositive(summary->dds_bLowLowLimit));
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*Gap*/
    ol_sprintf(strLeft, "%s", getStringPositive(summary->dds_bGap));
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*All*/
    ol_sprintf(strLeft, "%llu", summary->dds_u64All);
    ol_sprintf(strRight, "%.2f", summary->dds_dbVolumeRatio);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*TurnoverRate*/
    ol_sprintf(strLeft, "%.2f", summary->dds_dbTurnoverRate);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*AllA*/
    ol_sprintf(strLeft, "%llu", summary->dds_u64AllA);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;

    /*UpperShadowRatio*/
    ol_sprintf(strLeft, "%.2f", summary->dds_dbUpperShadowRatio);
    ol_sprintf(strRight, "%.2f", summary->dds_dbLowerShadowRatio);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*GeneralCapital*/
    ol_sprintf(strLeft, "%lld", summary->dds_u64GeneralCapital);
    ol_sprintf(strRight, "%lld", summary->dds_u64TradableShare);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*Status*/
    getStringDaySummaryStatus(summary, strLeft);
    cliengPrintOneFullLine(pcc, strLeft);
    pcc += 1;
}

static void _printTradeDaySummary(
    cli_parse_param_t * pcpp, da_day_summary_t * buffer, olint_t total)
{
    da_day_summary_t * cur;
    olint_t i;

    if (! pcpp->cpp_bVerbose)
    {
        cliengPrintDivider();

        cliengPrintHeader(
            ls_ccTradeDaySummaryBrief,
            sizeof(ls_ccTradeDaySummaryBrief) / sizeof(clieng_caption_t));
    }

    cur = buffer;
    for (i = 0; i < total; i++)
    {
        if (pcpp->cpp_bVerbose)
            _printDaDaySummaryVerbose(cur);
        else
            _printTradeDaySummaryBrief(cur);
        cur ++;
    }

    cliengOutputLine("");
}

static u32 _readTradeSummaryFile(cli_parse_param_t * pcpp, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t total = 400; //MAX_NUM_OF_RESULT;
    da_day_summary_t * buffer = NULL;
    olchar_t dirpath[MAX_PATH_LEN];

    if (pcpp->cpp_nLastCount != 0)
        total = pcpp->cpp_nLastCount;

    u32Ret = xmalloc((void **)&buffer, sizeof(da_day_summary_t) * total);
    if (u32Ret != OLERR_NO_ERROR)
        return u32Ret;

    ol_snprintf(
        dirpath, MAX_PATH_LEN, "%s%c%s",
        getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcpp->cpp_pstrStock);

	if (pcpp->cpp_bFRoR)
    {
        if (pcpp->cpp_bStartDate)
            u32Ret = readTradeDaySummaryFromDateWithFRoR(
                dirpath, pcpp->cpp_pstrDate, buffer, &total);
        else if (pcpp->cpp_bEndDate)
            u32Ret = readTradeDaySummaryUntilDateWithFRoR(
                dirpath, pcpp->cpp_pstrDate, buffer, &total);
        else
            u32Ret = readTradeDaySummaryWithFRoR(dirpath, buffer, &total);
    }
	else
    {
        if (pcpp->cpp_bStartDate)
            u32Ret = readTradeDaySummaryFromDate(
                dirpath, pcpp->cpp_pstrDate, buffer, &total);
        else if (pcpp->cpp_bEndDate)
            u32Ret = readTradeDaySummaryUntilDate(
                dirpath, pcpp->cpp_pstrDate, buffer, &total);
        else
            u32Ret = readTradeDaySummary(dirpath, buffer, &total);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        _printTradeDaySummary(pcpp, buffer, total);
    }

    xfree((void **)&buffer);

    return u32Ret;
}

static void _printQuoEntryBrief(quo_entry_t * entry)
{
    clieng_caption_t * pcc = &ls_ccQuoEntryBrief[0];
    olchar_t strInfo[MAX_OUTPUT_LINE_LEN], strField[MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* time */
    ol_sprintf(strField, "%s", entry->qe_strTime);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* CurP */
    ol_sprintf(strField, "%.2f", entry->qe_dbCurPrice);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* HighP */
    ol_sprintf(strField, "%.2f", entry->qe_dbHighPrice);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LowP */
    ol_sprintf(strField, "%.2f", entry->qe_dbLowPrice);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Volumn */
    ol_sprintf(strField, "%llu", entry->qe_u64Volume);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Amount */
    ol_sprintf(strField, "%.2f", entry->qe_dbAmount);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    cliengOutputLine(strInfo);
}

static void _printStockQuoVerbose(stock_quo_t * psq)
{
    clieng_caption_t * pcc = &ls_ccStockQuoVerbose[0];
    olchar_t strLeft[MAX_OUTPUT_LINE_LEN], strRight[MAX_OUTPUT_LINE_LEN];

    cliengPrintDivider();

    /*Name*/
    ol_sprintf(strLeft, "%s", psq->sq_strCode);
    ol_sprintf(strRight, "%s", psq->sq_strDate);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*OpeningPrice*/
    ol_sprintf(strLeft, "%.2f", psq->sq_dbOpeningPrice);
    ol_sprintf(strRight, "%.2f", psq->sq_dbLastClosingPrice);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*MaxEntry*/
    ol_sprintf(strLeft, "%d", psq->sq_nMaxEntry);
    ol_sprintf(strRight, "%d", psq->sq_nNumOfEntry);
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    cliengOutputLine("");
}

static void _analysisStockQuo(
    stock_quo_t * pstockquo, quo_entry_t ** pqe, olint_t total)
{
//    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i;
    u64 u64Vol;
    olint_t start, end;

    cliengOutputLine(
        "%s, %.2f, %lld", pqe[0]->qe_strTime, pqe[0]->qe_dbCurPrice, pqe[0]->qe_u64Volume);
    u64Vol = pqe[0]->qe_u64Volume;
    for (i = 1; i < total; i ++)
    {
        if (pqe[i]->qe_dbCurPrice > pqe[i - 1]->qe_dbCurPrice)
        {
            u64Vol = u64Vol + (pqe[i]->qe_u64Volume - pqe[i - 1]->qe_u64Volume);
        }
        else
        {
            u64Vol = u64Vol - (pqe[i]->qe_u64Volume - pqe[i - 1]->qe_u64Volume);
        }
        cliengOutputLine(
            "%s, %.2f, %lld", pqe[i]->qe_strTime, pqe[i]->qe_dbCurPrice, u64Vol);
    }

    cliengOutputLine("");
    start = 0;
    cliengOutputLine(
        "%s, %.2f", pqe[start]->qe_strTime, pqe[start]->qe_dbCurPrice);
    while (start < total)
    {
        end = getNextTopBottomQuoEntry(pqe, total, start);

        cliengOutputLine(
            "%s(%.2f) --> %s(%.2f)",
            pqe[start]->qe_strTime, pqe[start]->qe_dbCurPrice,
            pqe[end]->qe_strTime, pqe[end]->qe_dbCurPrice);

        /*Do something from start to end*/

        if (end == total - 1)
            break;
        start = end;
    }
}

#if 0
static void _analysisStockQuo(
    stock_quo_t * pstockquo, quo_entry_t ** pqe, olint_t total)
{
    oldouble_t * pdba = NULL, * pdbb = NULL, dbr;
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i, j, max;
    quo_entry_t * start;

    allocMemory((void **)&pdba, sizeof(oldouble_t) * pstockquo->sq_nNumOfEntry, 0);
    allocMemory((void **)&pdbb, sizeof(oldouble_t) * pstockquo->sq_nNumOfEntry, 0);

    for (i = 1; i < total; i ++)
    {

        start = &pstockquo->sq_pqeEntry[2];
        if (pqe[i] <= start)
            dbr = 0;
        else
        {
            max = pqe[i] - &pstockquo->sq_pqeEntry[1];

            for (j = 0; j < max; j ++)
            {
                pdba[j] = start->qe_dbCurPrice;
                pdbb[j] = (oldouble_t)(start->qe_u64Volume - (start - 1)->qe_u64Volume);
                start ++;
//                cliengOutputLine("%.2f, %.2f", pdba[j], pdbb[j]);
            }
            u32Ret = getCorrelation(pdba, pdbb, max, &dbr);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            cliengOutputLine("%s, Price & Volumn: %.2f", pqe[i]->qe_strTime, dbr);
        }
    }

    freeMemory((void **)&pdba);
    freeMemory((void **)&pdbb);

}
#endif

static u32 _readQuotationFile(cli_parse_param_t * pcpp, da_master_t * pdm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t total, i;
    quo_entry_t ** pqe = NULL;
    stock_quo_t stockquo;
    stock_info_t * stockinfo;
    olchar_t dirpath[MAX_PATH_LEN];
    quo_entry_t * entry;

    memset(&stockquo, 0, sizeof(stock_quo_t));

    u32Ret = getStockInfo(pcpp->cpp_pstrStock, &stockinfo);
    if (u32Ret == OLERR_NO_ERROR)
    {
        stockquo.sq_nMaxEntry = 3500; //4 * 60 * 60 / 6; /*6 items per minutes*/
        ol_strcpy(stockquo.sq_strCode, stockinfo->si_strCode);
        if (pcpp->cpp_pstrDate != NULL)
            ol_strcpy(stockquo.sq_strDate, pcpp->cpp_pstrDate);

        u32Ret = xmalloc(
            (void **)&stockquo.sq_pqeEntry,
            sizeof(quo_entry_t) * stockquo.sq_nMaxEntry);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, MAX_PATH_LEN, "%s%c%s",
            getEnvVar(ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcpp->cpp_pstrStock);

        u32Ret = readStockQuotationFile(
            dirpath, &stockquo);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        cliengPrintHeader(
            ls_ccQuoEntryBrief,
            sizeof(ls_ccQuoEntryBrief) / sizeof(clieng_caption_t));

        entry = stockquo.sq_pqeEntry;
        for (i = 0; i < stockquo.sq_nNumOfEntry; i++)
        {
            _printQuoEntryBrief(entry);
            entry ++;
        }

        _printStockQuoVerbose(&stockquo);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        total = stockquo.sq_nNumOfEntry;
        allocMemory((void **)&pqe, sizeof(quo_entry_t *) * total, 0);
        cliengOutputLine("");
        getQuoEntryInflexionPoint(
            stockquo.sq_pqeEntry, stockquo.sq_nNumOfEntry, pqe, &total);
        cliengOutputLine("Total %d points", total);

        for (i = 0; i < total; i ++)
        {
            cliengOutputLine(
                "%s %.2f", pqe[i]->qe_strTime, pqe[i]->qe_dbCurPrice);
        }
        cliengOutputLine("");

        _analysisStockQuo(&stockquo, pqe, total);
        cliengOutputLine("");

        freeMemory((void **)&pqe);
    }

    if (stockquo.sq_pqeEntry != NULL)
        xfree((void **)&stockquo.sq_pqeEntry);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 processParse(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_parse_param_t * pcpp = (cli_parse_param_t *)pParam;
    da_master_t * pdm = (da_master_t *)pMaster;

    if (pcpp->cpp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _parseHelp(pdm);
    else if (*getEnvVar(ENV_VAR_DATA_PATH) == '\0')
    {
        cliengOutputLine("Data path is not set.");
        u32Ret = OLERR_NOT_READY;
    }
    else if (pcpp->cpp_u8Action == CLI_ACTION_PARSE_TRADE_SUMMARY)
		u32Ret = _readTradeSummaryFile(pcpp, pdm);
    else if (pcpp->cpp_u8Action == CLI_ACTION_PARSE_QUOTATION)
		u32Ret = _readQuotationFile(pcpp, pdm);
    else if (pcpp->cpp_u8Action == CLI_ACTION_PARSE_TRADE_DETAIL)
		u32Ret = _readTradeDetailFile(pcpp, pdm);
    else
        u32Ret = OLERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamParse(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_parse_param_t * pcpp = (cli_parse_param_t *)pParam;
//    da_model_t * model = getCurrentDaModel();

    memset(pcpp, 0, sizeof(cli_parse_param_t));

    pcpp->cpp_nThres = 400;

    return u32Ret;
}

u32 parseParse(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_parse_param_t * pcpp = (cli_parse_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "s:e:t:d:fuq:l:ohv?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 's':
            pcpp->cpp_u8Action = CLI_ACTION_PARSE_TRADE_SUMMARY;
            pcpp->cpp_pstrStock = (olchar_t *)optarg;
            break;		
        case 'f':
            pcpp->cpp_bStartDate = TRUE;
            break;
        case 'u':
            pcpp->cpp_bEndDate = TRUE;
            break;
        case 'q':
            pcpp->cpp_u8Action = CLI_ACTION_PARSE_QUOTATION;
            pcpp->cpp_pstrStock = (olchar_t *)optarg;
            break;		
        case 'e':
            pcpp->cpp_u8Action = CLI_ACTION_PARSE_TRADE_DETAIL;
            pcpp->cpp_pstrStock = (olchar_t *)optarg;
            break;
        case 't':
            u32Ret = getS32FromString(
                optarg, ol_strlen(optarg), &pcpp->cpp_nThres);
            if ((u32Ret == OLERR_NO_ERROR) && (pcpp->cpp_nThres <= 0))
            {
                cliengReportInvalidOpt('t');
                u32Ret = OLERR_INVALID_PARAM;
            }
            break;
        case 'd':
            pcpp->cpp_pstrDate = (olchar_t *)optarg;
            break;
		case 'o':
			pcpp->cpp_bFRoR = TRUE;
			break;
        case 'l':
            u32Ret = getS32FromString(
                optarg, ol_strlen(optarg), &pcpp->cpp_nLastCount);
            if ((u32Ret == OLERR_NO_ERROR) && (pcpp->cpp_nLastCount <= 0))
            {
                cliengReportInvalidOpt('l');
                u32Ret = OLERR_INVALID_PARAM;
            }
            break;
        case ':':
            u32Ret = OLERR_MISSING_PARAM;
            break;
        case 'v':
            pcpp->cpp_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcpp->cpp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


