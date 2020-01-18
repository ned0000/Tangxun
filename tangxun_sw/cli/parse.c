/**
 *  @file parse.c
 *
 *  @brief The parse command implementation.
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
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "jf_clieng.h"
#include "jf_matrix.h"
#include "jf_jiukun.h"

#include "parsedata.h"
#include "datastat.h"
#include "clicmd.h"
#include "regression.h"
#include "damodel.h"
#include "stocklist.h"
#include "tx_env.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define MAX_NUM_OF_RESULT  600
#define MAX_NUM_OF_TRADE_DETAIL_FILE  100

static jf_clieng_caption_t ls_ccTradeDayDetailBrief[] =
{
    {"Day", 5}, 
    {"Date", 12},
    {"Buy", 16},
    {"Sold", 16},
    {"LambBuy", 10},
    {"LambSold", 10},
    {"Close", 6},
};

static jf_clieng_caption_t ls_ccTradeDaySummaryBrief[] =
{
    {"Day", 5}, 
    {"Date", 12},
    {"OpenP", 7},
    {"CloseP", 7},
    {"HighP", 7},
    {"LowP", 7},
    {"Close", 6},
    {"GeneralCapital", 15},
    {"TradableShare", 14},
	{"Status", 10},
};

static jf_clieng_caption_t ls_ccQuoEntryBrief[] =
{
    {"Time", 11},
    {"CurP", 9}, 
    {"HighP", 9},
    {"LowP", 9},
    {"Volumn", 12},
    {"Amount", 12},
};

static jf_clieng_caption_t ls_ccStockQuoVerbose[] =
{
    {"Name", JF_CLIENG_CAP_HALF_LINE}, {"Date", JF_CLIENG_CAP_HALF_LINE},
    {"OpeningPrice", JF_CLIENG_CAP_HALF_LINE}, {"LastClosingPrice", JF_CLIENG_CAP_HALF_LINE},
    {"MaxEntry", JF_CLIENG_CAP_HALF_LINE}, {"NumOfEntry", JF_CLIENG_CAP_HALF_LINE},
};

static jf_clieng_caption_t ls_ccDataVerbose[] =
{
    {"Day", JF_CLIENG_CAP_HALF_LINE}, {"Date", JF_CLIENG_CAP_HALF_LINE},

    {"OpeningPrice", JF_CLIENG_CAP_HALF_LINE}, {"ClosingPrice", JF_CLIENG_CAP_HALF_LINE},
    {"HighPrice", JF_CLIENG_CAP_HALF_LINE}, {"LowPrice", JF_CLIENG_CAP_HALF_LINE},
    {"ClosingPriceInc", JF_CLIENG_CAP_HALF_LINE}, {"ClosingPriceDec", JF_CLIENG_CAP_HALF_LINE},
    {"HighPriceInc", JF_CLIENG_CAP_HALF_LINE}, {"LowPriceDec", JF_CLIENG_CAP_HALF_LINE},
    {"CloseHighLimit", JF_CLIENG_CAP_HALF_LINE}, {"TimeCloseHighLimit", JF_CLIENG_CAP_HALF_LINE},
    {"CloseLowLimit", JF_CLIENG_CAP_HALF_LINE}, {"TimeCloseLowLimit", JF_CLIENG_CAP_HALF_LINE},
    {"Gap", JF_CLIENG_CAP_FULL_LINE},

    {"All", JF_CLIENG_CAP_FULL_LINE},
    {"Buy", JF_CLIENG_CAP_HALF_LINE}, {"PercentOfBuy", JF_CLIENG_CAP_HALF_LINE},
    {"PercentOfBuyInAM", JF_CLIENG_CAP_FULL_LINE},
    {"Sold", JF_CLIENG_CAP_HALF_LINE}, {"PercentOfSold", JF_CLIENG_CAP_HALF_LINE},
    {"PercentOfSoldInAM", JF_CLIENG_CAP_FULL_LINE},
    {"LambBuy", JF_CLIENG_CAP_HALF_LINE}, {"PercentOfLambBuy", JF_CLIENG_CAP_HALF_LINE},
    {"PercentOfLambBuyBelow100", JF_CLIENG_CAP_FULL_LINE},
    {"LambSold", JF_CLIENG_CAP_HALF_LINE}, {"PercentOfLambSold", JF_CLIENG_CAP_HALF_LINE},
    {"PercentOfLambSoldBelow100", JF_CLIENG_CAP_FULL_LINE},
    {"Na", JF_CLIENG_CAP_HALF_LINE}, {"LambNa", JF_CLIENG_CAP_HALF_LINE},
    {"AboveLastClosingPrice", JF_CLIENG_CAP_HALF_LINE}, {"AboveLastClosingPricePercent", JF_CLIENG_CAP_HALF_LINE},
    {"CloseHighLimitSold", JF_CLIENG_CAP_HALF_LINE}, {"PercentOfCloseHighLimitSold", JF_CLIENG_CAP_HALF_LINE},
    {"CloseHighLimitLambSold", JF_CLIENG_CAP_HALF_LINE}, {"PercentOfCloseHighLimitLambSold", JF_CLIENG_CAP_HALF_LINE},
    {"CloseLowLimitBuy", JF_CLIENG_CAP_HALF_LINE}, {"PercentOfCloseLowLimitBuy", JF_CLIENG_CAP_HALF_LINE},
    {"CloseLowLimitLambBuy", JF_CLIENG_CAP_HALF_LINE}, {"PercentOfCloseLowLimitLambBuy", JF_CLIENG_CAP_HALF_LINE},
    {"VolumeRatio", JF_CLIENG_CAP_FULL_LINE},
    {"SoldVolumeRatio", JF_CLIENG_CAP_HALF_LINE}, {"LambSoldVolumeRatio", JF_CLIENG_CAP_HALF_LINE},

    {"AllA", JF_CLIENG_CAP_FULL_LINE},
    {"BuyA", JF_CLIENG_CAP_HALF_LINE}, {"SoldA", JF_CLIENG_CAP_HALF_LINE},
    {"LambBuyA", JF_CLIENG_CAP_HALF_LINE}, {"LambSoldA", JF_CLIENG_CAP_HALF_LINE},
    {"CloseHighLimitSoldA", JF_CLIENG_CAP_HALF_LINE}, {"CloseHighLimitLambSoldA", JF_CLIENG_CAP_HALF_LINE},
    {"CloseLowLimitBuyA", JF_CLIENG_CAP_HALF_LINE}, {"CloseLowLimitLambBuyA", JF_CLIENG_CAP_HALF_LINE},

    {"UpperShadowRatio", JF_CLIENG_CAP_HALF_LINE}, {"LowerShadowRatio", JF_CLIENG_CAP_HALF_LINE},

    {"LastTimeOfLowPrice", JF_CLIENG_CAP_HALF_LINE}, {"LastLowPrice", JF_CLIENG_CAP_HALF_LINE},
    {"LastLowPriceInc", JF_CLIENG_CAP_FULL_LINE},
};

static jf_clieng_caption_t ls_ccDaySummaryVerbose[] =
{
    {"Day", JF_CLIENG_CAP_HALF_LINE}, {"Date", JF_CLIENG_CAP_HALF_LINE},

    {"OpeningPrice", JF_CLIENG_CAP_HALF_LINE}, {"ClosingPrice", JF_CLIENG_CAP_HALF_LINE},
    {"HighPrice", JF_CLIENG_CAP_HALF_LINE}, {"LowPrice", JF_CLIENG_CAP_HALF_LINE},
    {"LastClosingPrice", JF_CLIENG_CAP_FULL_LINE},
    {"ClosingPriceRate", JF_CLIENG_CAP_HALF_LINE}, {"HighPriceRate", JF_CLIENG_CAP_HALF_LINE},
    {"CloseHighLimit", JF_CLIENG_CAP_HALF_LINE}, {"CloseLowLimit", JF_CLIENG_CAP_HALF_LINE},
    {"HighHighLimit", JF_CLIENG_CAP_HALF_LINE}, {"LowLowLimit", JF_CLIENG_CAP_HALF_LINE},
    {"Gap", JF_CLIENG_CAP_FULL_LINE},

    {"All", JF_CLIENG_CAP_HALF_LINE}, {"VolumeRatio", JF_CLIENG_CAP_HALF_LINE},
    {"TurnoverRate", JF_CLIENG_CAP_FULL_LINE},

    {"AllA", JF_CLIENG_CAP_FULL_LINE},

    {"UpperShadowRatio", JF_CLIENG_CAP_HALF_LINE}, {"LowerShadowRatio", JF_CLIENG_CAP_HALF_LINE},

    {"GeneralCapital", JF_CLIENG_CAP_HALF_LINE}, {"TradableShare", JF_CLIENG_CAP_HALF_LINE},
    {"Status", JF_CLIENG_CAP_FULL_LINE},
};

/* --- private routine section ------------------------------------------------------------------ */
static u32 _parseHelp(tx_cli_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputRawLine2("\
Parse stock data.\n\
parse [Trade summary file options] [Quotation file options] [Trade detail file options]\n\
  [Common options] ");
    jf_clieng_outputRawLine2("\
Common options.\n\
parse [-l count] [-d yyyy-mm-dd] [-v] [-h] \n\
  -l: read last N days.\n\
  -d: the start date.\n\
  -h: print the usage.\n\
  -v: verbose.");
    jf_clieng_outputRawLine2("\
Trade summary file options.\n\
parse [-s stock] [-f] [-u] [-o] [-c count] \n\
  -s: parse trade summary file.\n\
  -f: parse files from specified date, return NULL if the date is not found in the file.\n\
      If '-d' is not specified, the date will be the first day in the file.\n\
  -u: parse files until specified date, return NULL if the date is not found in the file.\n\
      If '-d' is not specified, the date will be the last day in the file.\n\
  -c: read maximum count trading days after specified date of option '-u'. The count is 0 if not\n\
      specifed.\n\
  -o: forward restoration of right.");
    jf_clieng_outputRawLine2("\
Quotation file options.\n\
parse [-q stock] [-f] \n\
  -q: parse quotation files, use '-f' to specify the date, use last day if '-f' is not specified.");
    jf_clieng_outputRawLine2("\
Trade detail file options.\n\
parse [-e stock] [-t threshold] \n\
  -e: parse trade detail files.\n\
  -t: specify the threshold.");
    jf_clieng_outputLine("");

    return u32Ret;
}

static void _printTradeDayDetailBrief(da_day_result_t * cur)
{
    jf_clieng_caption_t * pcc = &ls_ccTradeDayDetailBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Day */
    ol_sprintf(strField, "%d", cur->ddr_nIndex);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Date */
    jf_clieng_appendBriefColumn(pcc, strInfo, cur->ddr_strDate);
    pcc++;

    /* Buy */
    ol_sprintf(
        strField, "%llu",
        cur->ddr_u64Buy + cur->ddr_u64CloseHighLimitSold + cur->ddr_u64CloseHighLimitLambSold);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Sold */
    ol_sprintf(
        strField, "%llu",
        cur->ddr_u64Sold + cur->ddr_u64CloseLowLimitBuy + cur->ddr_u64CloseLowLimitLambBuy);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambBuy */
    ol_sprintf(strField, "%llu", cur->ddr_u64LambBuy);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambSold */
    ol_sprintf(strField, "%llu", cur->ddr_u64LambSold);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosePriceInc */
    ol_sprintf(strField, "%.2f", cur->ddr_dbClosingPriceInc - cur->ddr_dbClosingPriceDec);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printTradeDayDetailVerbose(da_day_result_t * result)
{
    jf_clieng_caption_t * pcc = &ls_ccDataVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Day*/
    ol_sprintf(strLeft, "%d", result->ddr_nIndex);
    jf_clieng_printTwoHalfLine(pcc, strLeft, result->ddr_strDate);
    pcc += 2;

    /*OpeningPrice*/
    ol_sprintf(strLeft, "%.2f", result->ddr_dbOpeningPrice);
    ol_sprintf(strRight, "%.2f", result->ddr_dbClosingPrice);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*HighPrice*/
    ol_sprintf(strLeft, "%.2f", result->ddr_dbHighPrice);
    ol_sprintf(strRight, "%.2f", result->ddr_dbLowPrice);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*ClosingPriceInc*/
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbClosingPriceInc);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbClosingPriceDec);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*HighPriceInc*/
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbHighPriceInc);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbLowPriceDec);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimit*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(result->ddr_bCloseHighLimit));
    jf_clieng_printTwoHalfLine(pcc, strLeft, result->ddr_strTimeCloseHighLimit);
    pcc += 2;

    /*CloseLowLimit*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(result->ddr_bCloseLowLimit));
    jf_clieng_printTwoHalfLine(pcc, strLeft, result->ddr_strTimeCloseLowLimit);
    pcc += 2;

    /*Gap*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(result->ddr_bGap));
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*All*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64All);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Buy*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64Buy);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbBuyPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*PercentOfBuyInAM*/
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbBuyInAmPercent);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Sold*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64Sold);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbSoldPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*PercentOfSoldInAM*/
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbSoldInAmPercent);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*LambBuy*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64LambBuy);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbLambBuyPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*PercentOfLambBuyBelow100*/
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbLambBuyBelow100Percent);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*LambSold*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64LambSold);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbLambSoldPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*PercentOfLambSoldBelow100*/
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbLambSoldBelow100Percent);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Na*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64Na);
    ol_sprintf(strRight, "%llu", result->ddr_u64LambNa);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*AboveLastClosingPrice*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64AboveLastClosingPrice);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbAboveLastClosingPricePercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimitSold*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64CloseHighLimitSold);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbCloseHighLimitSoldPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimitLambSold*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64CloseHighLimitLambSold);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbCloseHighLimitLambSoldPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseLowLimitBuy*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64CloseLowLimitBuy);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbCloseLowLimitBuyPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseLowLimitLambBuy*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64CloseLowLimitLambBuy);
    ol_sprintf(strRight, "%.2f%%", result->ddr_dbCloseLowLimitLambBuyPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*VolumeRatio*/
    ol_sprintf(strLeft, "%.2f", result->ddr_dbVolumeRatio);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*SoldVolumeRatio*/
    ol_sprintf(strLeft, "%.2f", result->ddr_dbSoldVolumeRatio);
    ol_sprintf(strRight, "%.2f", result->ddr_dbLambSoldVolumeRatio);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*AllA*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64AllA);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*BuyA*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64BuyA);
    ol_sprintf(strRight, "%llu", result->ddr_u64SoldA);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*LambBuyA*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64LambBuyA);
    ol_sprintf(strRight, "%llu", result->ddr_u64LambSoldA);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimitSoldA*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64CloseHighLimitSoldA);
    ol_sprintf(strRight, "%llu", result->ddr_u64CloseHighLimitLambSoldA);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseLowLimitBuyA*/
    ol_sprintf(strLeft, "%llu", result->ddr_u64CloseLowLimitBuyA);
    ol_sprintf(strRight, "%llu", result->ddr_u64CloseLowLimitLambBuyA);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*UpperShadowRatio*/
    ol_sprintf(strLeft, "%.2f", result->ddr_dbUpperShadowRatio);
    ol_sprintf(strRight, "%.2f", result->ddr_dbLowerShadowRatio);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* LastTimeOfLowPrice */
    ol_sprintf(strRight, "%.2f", result->ddr_dbLastXLowPrice);
    jf_clieng_printTwoHalfLine(pcc, result->ddr_strLastTimeLowPrice, strRight);
    pcc += 2;

    /* LastLowPriceInc */
    ol_sprintf(strLeft, "%.2f%%", result->ddr_dbLastXInc);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

}

static void _printTradeDetailFile(
    cli_parse_param_t * pcpp, da_day_result_t * buffer, olint_t total)
{
    da_day_result_t * cur;
    olint_t i;

    if (! pcpp->cpp_bVerbose)
    {
        jf_clieng_printDivider();

        jf_clieng_printHeader(
            ls_ccTradeDayDetailBrief,
            sizeof(ls_ccTradeDayDetailBrief) / sizeof(jf_clieng_caption_t));
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

    jf_clieng_outputLine("");
}

static u32 _readTradeDetailFile(cli_parse_param_t * pcpp, tx_cli_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = MAX_NUM_OF_TRADE_DETAIL_FILE;
    da_day_result_t * buffer = NULL;
    parse_param_t pp;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    stock_info_t * stockinfo;

    u32Ret = jf_mem_alloc((void **)&buffer, sizeof(da_day_result_t) * total);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = getStockInfo(pcpp->cpp_pstrStock, &stockinfo);
    if (u32Ret == JF_ERR_NO_ERROR)
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
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcpp->cpp_pstrStock);

        u32Ret = readTradeDayDetail(dirpath, &pp, buffer, &total);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _printTradeDetailFile(pcpp, buffer, total);
    }

    if (buffer != NULL)
        jf_mem_free((void **)&buffer);

    return u32Ret;
}

static void _printTradeDaySummaryBrief(da_day_summary_t * cur)
{
    jf_clieng_caption_t * pcc = &ls_ccTradeDaySummaryBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Day */
    ol_sprintf(strField, "%d", cur->dds_nIndex);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Date */
    jf_clieng_appendBriefColumn(pcc, strInfo, cur->dds_strDate);
    pcc++;

    /* OpeningPrice */
    ol_sprintf(strField, "%.2f", cur->dds_dbOpeningPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosingPrice */
    ol_sprintf(strField, "%.2f", cur->dds_dbClosingPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* HighPrice */
    ol_sprintf(strField, "%.2f", cur->dds_dbHighPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LowPrice */
    ol_sprintf(strField, "%.2f", cur->dds_dbLowPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosePriceRate */
    ol_sprintf(strField, "%.2f", cur->dds_dbClosingPriceRate);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* GeneralCapital */
    ol_sprintf(strField, "%lld", cur->dds_u64GeneralCapital);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* TradableShare */
    ol_sprintf(strField, "%lld", cur->dds_u64TradableShare);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Status*/
    getStringDaySummaryStatus(cur, strField);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printDaDaySummaryVerbose(da_day_summary_t * summary)
{
    jf_clieng_caption_t * pcc = &ls_ccDaySummaryVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Day*/
    ol_sprintf(strLeft, "%d", summary->dds_nIndex);
    jf_clieng_printTwoHalfLine(pcc, strLeft, summary->dds_strDate);
    pcc += 2;

    /*OpeningPrice*/
    ol_sprintf(strLeft, "%.2f", summary->dds_dbOpeningPrice);
    ol_sprintf(strRight, "%.2f", summary->dds_dbClosingPrice);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*HighPrice*/
    ol_sprintf(strLeft, "%.2f", summary->dds_dbHighPrice);
    ol_sprintf(strRight, "%.2f", summary->dds_dbLowPrice);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*LastClosingPrice*/
    ol_sprintf(strLeft, "%.2f", summary->dds_dbLastClosingPrice);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*ClosingPriceRate*/
    ol_sprintf(strLeft, "%.2f%%", summary->dds_dbClosingPriceRate);
    ol_sprintf(strRight, "%.2f%%", summary->dds_dbHighPriceRate);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimit*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(summary->dds_bCloseHighLimit));
    ol_sprintf(strRight, "%s", jf_string_getStringPositive(summary->dds_bCloseLowLimit));
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*HighHighLimit*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(summary->dds_bHighHighLimit));
    ol_sprintf(strRight, "%s", jf_string_getStringPositive(summary->dds_bLowLowLimit));
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*Gap*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(summary->dds_bGap));
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*All*/
    ol_sprintf(strLeft, "%llu", summary->dds_u64All);
    ol_sprintf(strRight, "%.2f", summary->dds_dbVolumeRatio);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*TurnoverRate*/
    ol_sprintf(strLeft, "%.2f", summary->dds_dbTurnoverRate);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*AllA*/
    ol_sprintf(strLeft, "%llu", summary->dds_u64AllA);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*UpperShadowRatio*/
    ol_sprintf(strLeft, "%.2f", summary->dds_dbUpperShadowRatio);
    ol_sprintf(strRight, "%.2f", summary->dds_dbLowerShadowRatio);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*GeneralCapital*/
    ol_sprintf(strLeft, "%lld", summary->dds_u64GeneralCapital);
    ol_sprintf(strRight, "%lld", summary->dds_u64TradableShare);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*Status*/
    getStringDaySummaryStatus(summary, strLeft);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;
}

static void _printTradeDaySummary(
    cli_parse_param_t * pcpp, da_day_summary_t * buffer, olint_t total)
{
    da_day_summary_t * cur;
    olint_t i;

    if (! pcpp->cpp_bVerbose)
    {
        jf_clieng_printDivider();

        jf_clieng_printHeader(
            ls_ccTradeDaySummaryBrief,
            sizeof(ls_ccTradeDaySummaryBrief) / sizeof(jf_clieng_caption_t));
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

    jf_clieng_outputLine("");
}

static u32 _readTradeSummaryFile(cli_parse_param_t * pcpp, tx_cli_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total = 400; //MAX_NUM_OF_RESULT;
    da_day_summary_t * buffer = NULL;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];

    if (pcpp->cpp_nLastCount != 0)
        total = pcpp->cpp_nLastCount;

    u32Ret = jf_mem_alloc((void **)&buffer, sizeof(da_day_summary_t) * total);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcpp->cpp_pstrStock);

	if (pcpp->cpp_bFRoR)
    {
        if (pcpp->cpp_bStartDate)
            u32Ret = readTradeDaySummaryFromDateWithFRoR(
                dirpath, pcpp->cpp_pstrDate, buffer, &total);
        else if (pcpp->cpp_bEndDate)
            u32Ret = readTradeDaySummaryUntilDateWithFRoR(
                dirpath, pcpp->cpp_pstrDate, pcpp->cpp_u32Count, buffer, &total);
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
                dirpath, pcpp->cpp_pstrDate, pcpp->cpp_u32Count, buffer, &total);
        else
            u32Ret = readTradeDaySummary(dirpath, buffer, &total);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _printTradeDaySummary(pcpp, buffer, total);
    }

    jf_mem_free((void **)&buffer);

    return u32Ret;
}

static void _printQuoEntryBrief(quo_entry_t * entry)
{
    jf_clieng_caption_t * pcc = &ls_ccQuoEntryBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* time */
    ol_sprintf(strField, "%s", entry->qe_strTime);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* CurP */
    ol_sprintf(strField, "%.2f", entry->qe_dbCurPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* HighP */
    ol_sprintf(strField, "%.2f", entry->qe_dbHighPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LowP */
    ol_sprintf(strField, "%.2f", entry->qe_dbLowPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Volumn */
    ol_sprintf(strField, "%llu", entry->qe_u64Volume);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Amount */
    ol_sprintf(strField, "%.2f", entry->qe_dbAmount);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printStockQuoVerbose(stock_quo_t * psq)
{
    jf_clieng_caption_t * pcc = &ls_ccStockQuoVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Name*/
    ol_sprintf(strLeft, "%s", psq->sq_strCode);
    ol_sprintf(strRight, "%s", psq->sq_strDate);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*OpeningPrice*/
    ol_sprintf(strLeft, "%.2f", psq->sq_dbOpeningPrice);
    ol_sprintf(strRight, "%.2f", psq->sq_dbLastClosingPrice);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*MaxEntry*/
    ol_sprintf(strLeft, "%d", psq->sq_nMaxEntry);
    ol_sprintf(strRight, "%d", psq->sq_nNumOfEntry);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    jf_clieng_outputLine("");
}

static void _analysisStockQuo(stock_quo_t * pstockquo, quo_entry_t ** pqe, olint_t total)
{
//    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;
    u64 u64Vol;
    olint_t start, end;

    jf_clieng_outputLine(
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
        jf_clieng_outputLine("%s, %.2f, %lld", pqe[i]->qe_strTime, pqe[i]->qe_dbCurPrice, u64Vol);
    }

    jf_clieng_outputLine("");
    start = 0;
    jf_clieng_outputLine("%s, %.2f", pqe[start]->qe_strTime, pqe[start]->qe_dbCurPrice);
    while (start < total)
    {
        end = getNextTopBottomQuoEntry(pqe, total, start);

        jf_clieng_outputLine(
            "%s(%.2f) --> %s(%.2f)", pqe[start]->qe_strTime, pqe[start]->qe_dbCurPrice,
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
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, j, max;
    quo_entry_t * start;

    jf_jiukun_allocMemory((void **)&pdba, sizeof(oldouble_t) * pstockquo->sq_nNumOfEntry);
    jf_jiukun_allocMemory((void **)&pdbb, sizeof(oldouble_t) * pstockquo->sq_nNumOfEntry);

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
//                jf_clieng_outputLine("%.2f, %.2f", pdba[j], pdbb[j]);
            }
            u32Ret = getCorrelation(pdba, pdbb, max, &dbr);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_clieng_outputLine("%s, Price & Volumn: %.2f", pqe[i]->qe_strTime, dbr);
        }
    }

    jf_jiukun_freeMemory((void **)&pdba);
    jf_jiukun_freeMemory((void **)&pdbb);

}
#endif

static u32 _readQuotationFile(cli_parse_param_t * pcpp, tx_cli_master_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t total, i;
    quo_entry_t ** pqe = NULL;
    stock_quo_t stockquo;
    stock_info_t * stockinfo;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    quo_entry_t * entry;

    memset(&stockquo, 0, sizeof(stock_quo_t));

    u32Ret = getStockInfo(pcpp->cpp_pstrStock, &stockinfo);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        stockquo.sq_nMaxEntry = 3500; //4 * 60 * 60 / 6; /*6 items per minutes*/
        ol_strcpy(stockquo.sq_strCode, stockinfo->si_strCode);
        if (pcpp->cpp_pstrDate != NULL)
            ol_strcpy(stockquo.sq_strDate, pcpp->cpp_pstrDate);

        u32Ret = jf_mem_alloc(
            (void **)&stockquo.sq_pqeEntry, sizeof(quo_entry_t) * stockquo.sq_nMaxEntry);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcpp->cpp_pstrStock);

        u32Ret = readStockQuotationFile(dirpath, &stockquo);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_clieng_printHeader(
            ls_ccQuoEntryBrief, sizeof(ls_ccQuoEntryBrief) / sizeof(jf_clieng_caption_t));

        entry = stockquo.sq_pqeEntry;
        for (i = 0; i < stockquo.sq_nNumOfEntry; i++)
        {
            _printQuoEntryBrief(entry);
            entry ++;
        }

        _printStockQuoVerbose(&stockquo);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        total = stockquo.sq_nNumOfEntry;
        jf_jiukun_allocMemory((void **)&pqe, sizeof(quo_entry_t *) * total);
        jf_clieng_outputLine("");
        getQuoEntryInflexionPoint(stockquo.sq_pqeEntry, stockquo.sq_nNumOfEntry, pqe, &total);
        jf_clieng_outputLine("Total %d points", total);

        for (i = 0; i < total; i ++)
        {
            jf_clieng_outputLine("%s %.2f", pqe[i]->qe_strTime, pqe[i]->qe_dbCurPrice);
        }
        jf_clieng_outputLine("");

        _analysisStockQuo(&stockquo, pqe, total);
        jf_clieng_outputLine("");

        jf_jiukun_freeMemory((void **)&pqe);
    }

    if (stockquo.sq_pqeEntry != NULL)
        jf_mem_free((void **)&stockquo.sq_pqeEntry);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processParse(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_parse_param_t * pcpp = (cli_parse_param_t *)pParam;
    tx_cli_master_t * pdm = (tx_cli_master_t *)pMaster;

    if (pcpp->cpp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _parseHelp(pdm);
    else if (tx_env_isNullVarDataPath())
    {
        jf_clieng_outputLine("Data path is not set.");
        u32Ret = JF_ERR_NOT_READY;
    }
    else if (pcpp->cpp_u8Action == CLI_ACTION_PARSE_TRADE_SUMMARY)
		u32Ret = _readTradeSummaryFile(pcpp, pdm);
    else if (pcpp->cpp_u8Action == CLI_ACTION_PARSE_QUOTATION)
		u32Ret = _readQuotationFile(pcpp, pdm);
    else if (pcpp->cpp_u8Action == CLI_ACTION_PARSE_TRADE_DETAIL)
		u32Ret = _readTradeDetailFile(pcpp, pdm);
    else
        u32Ret = JF_ERR_MISSING_PARAM;

    return u32Ret;
}

u32 setDefaultParamParse(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_parse_param_t * pcpp = (cli_parse_param_t *)pParam;
//    da_model_t * model = getCurrentDaModel();

    memset(pcpp, 0, sizeof(cli_parse_param_t));

    pcpp->cpp_nThres = 400;

    return u32Ret;
}

u32 parseParse(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_parse_param_t * pcpp = (cli_parse_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv, "s:e:c:t:d:fuq:l:ohv?")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
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
            u32Ret = jf_string_getS32FromString(optarg, ol_strlen(optarg), &pcpp->cpp_nThres);
            if ((u32Ret == JF_ERR_NO_ERROR) && (pcpp->cpp_nThres <= 0))
            {
                jf_clieng_reportInvalidOpt('t');
                u32Ret = JF_ERR_INVALID_PARAM;
            }
            break;
        case 'c':
            u32Ret = jf_string_getU32FromString(optarg, ol_strlen(optarg), &pcpp->cpp_u32Count);
            if (u32Ret != JF_ERR_NO_ERROR)
            {
                jf_clieng_reportInvalidOpt('c');
                u32Ret = JF_ERR_INVALID_PARAM;
            }
            break;
        case 'd':
            pcpp->cpp_pstrDate = (olchar_t *)optarg;
            break;
		case 'o':
			pcpp->cpp_bFRoR = TRUE;
			break;
        case 'l':
            u32Ret = jf_string_getS32FromString(optarg, ol_strlen(optarg), &pcpp->cpp_nLastCount);
            if ((u32Ret == JF_ERR_NO_ERROR) && (pcpp->cpp_nLastCount <= 0))
            {
                jf_clieng_reportInvalidOpt('l');
                u32Ret = JF_ERR_INVALID_PARAM;
            }
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case 'v':
            pcpp->cpp_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcpp->cpp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


