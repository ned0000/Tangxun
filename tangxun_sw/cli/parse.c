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

#include "tx_daysummary.h"
#include "tx_quo.h"
#include "tx_datastat.h"
#include "tx_regression.h"
#include "tx_stock.h"
#include "tx_env.h"

#include "clicmd.h"

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

static void _printTradeDayDetailBrief(tx_dr_t * cur)
{
    jf_clieng_caption_t * pcc = &ls_ccTradeDayDetailBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Day */
    ol_sprintf(strField, "%d", cur->td_nIndex);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Date */
    jf_clieng_appendBriefColumn(pcc, strInfo, cur->td_strDate);
    pcc++;

    /* Buy */
    ol_sprintf(
        strField, "%llu",
        cur->td_u64Buy + cur->td_u64CloseHighLimitSold + cur->td_u64CloseHighLimitLambSold);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Sold */
    ol_sprintf(
        strField, "%llu",
        cur->td_u64Sold + cur->td_u64CloseLowLimitBuy + cur->td_u64CloseLowLimitLambBuy);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambBuy */
    ol_sprintf(strField, "%llu", cur->td_u64LambBuy);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LambSold */
    ol_sprintf(strField, "%llu", cur->td_u64LambSold);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosePriceInc */
    ol_sprintf(strField, "%.2f", cur->td_dbClosingPriceInc - cur->td_dbClosingPriceDec);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printTradeDayDetailVerbose(tx_dr_t * result)
{
    jf_clieng_caption_t * pcc = &ls_ccDataVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Day*/
    ol_sprintf(strLeft, "%d", result->td_nIndex);
    jf_clieng_printTwoHalfLine(pcc, strLeft, result->td_strDate);
    pcc += 2;

    /*OpeningPrice*/
    ol_sprintf(strLeft, "%.2f", result->td_dbOpeningPrice);
    ol_sprintf(strRight, "%.2f", result->td_dbClosingPrice);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*HighPrice*/
    ol_sprintf(strLeft, "%.2f", result->td_dbHighPrice);
    ol_sprintf(strRight, "%.2f", result->td_dbLowPrice);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*ClosingPriceInc*/
    ol_sprintf(strLeft, "%.2f%%", result->td_dbClosingPriceInc);
    ol_sprintf(strRight, "%.2f%%", result->td_dbClosingPriceDec);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*HighPriceInc*/
    ol_sprintf(strLeft, "%.2f%%", result->td_dbHighPriceInc);
    ol_sprintf(strRight, "%.2f%%", result->td_dbLowPriceDec);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimit*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(result->td_bCloseHighLimit));
    jf_clieng_printTwoHalfLine(pcc, strLeft, result->td_strTimeCloseHighLimit);
    pcc += 2;

    /*CloseLowLimit*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(result->td_bCloseLowLimit));
    jf_clieng_printTwoHalfLine(pcc, strLeft, result->td_strTimeCloseLowLimit);
    pcc += 2;

    /*Gap*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(result->td_bGap));
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*All*/
    ol_sprintf(strLeft, "%llu", result->td_u64All);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Buy*/
    ol_sprintf(strLeft, "%llu", result->td_u64Buy);
    ol_sprintf(strRight, "%.2f%%", result->td_dbBuyPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*PercentOfBuyInAM*/
    ol_sprintf(strLeft, "%.2f%%", result->td_dbBuyInAmPercent);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Sold*/
    ol_sprintf(strLeft, "%llu", result->td_u64Sold);
    ol_sprintf(strRight, "%.2f%%", result->td_dbSoldPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*PercentOfSoldInAM*/
    ol_sprintf(strLeft, "%.2f%%", result->td_dbSoldInAmPercent);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*LambBuy*/
    ol_sprintf(strLeft, "%llu", result->td_u64LambBuy);
    ol_sprintf(strRight, "%.2f%%", result->td_dbLambBuyPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*PercentOfLambBuyBelow100*/
    ol_sprintf(strLeft, "%.2f%%", result->td_dbLambBuyBelow100Percent);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*LambSold*/
    ol_sprintf(strLeft, "%llu", result->td_u64LambSold);
    ol_sprintf(strRight, "%.2f%%", result->td_dbLambSoldPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*PercentOfLambSoldBelow100*/
    ol_sprintf(strLeft, "%.2f%%", result->td_dbLambSoldBelow100Percent);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*Na*/
    ol_sprintf(strLeft, "%llu", result->td_u64Na);
    ol_sprintf(strRight, "%llu", result->td_u64LambNa);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*AboveLastClosingPrice*/
    ol_sprintf(strLeft, "%llu", result->td_u64AboveLastClosingPrice);
    ol_sprintf(strRight, "%.2f%%", result->td_dbAboveLastClosingPricePercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimitSold*/
    ol_sprintf(strLeft, "%llu", result->td_u64CloseHighLimitSold);
    ol_sprintf(strRight, "%.2f%%", result->td_dbCloseHighLimitSoldPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimitLambSold*/
    ol_sprintf(strLeft, "%llu", result->td_u64CloseHighLimitLambSold);
    ol_sprintf(strRight, "%.2f%%", result->td_dbCloseHighLimitLambSoldPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseLowLimitBuy*/
    ol_sprintf(strLeft, "%llu", result->td_u64CloseLowLimitBuy);
    ol_sprintf(strRight, "%.2f%%", result->td_dbCloseLowLimitBuyPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseLowLimitLambBuy*/
    ol_sprintf(strLeft, "%llu", result->td_u64CloseLowLimitLambBuy);
    ol_sprintf(strRight, "%.2f%%", result->td_dbCloseLowLimitLambBuyPercent);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*VolumeRatio*/
    ol_sprintf(strLeft, "%.2f", result->td_dbVolumeRatio);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*SoldVolumeRatio*/
    ol_sprintf(strLeft, "%.2f", result->td_dbSoldVolumeRatio);
    ol_sprintf(strRight, "%.2f", result->td_dbLambSoldVolumeRatio);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*AllA*/
    ol_sprintf(strLeft, "%llu", result->td_u64AllA);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*BuyA*/
    ol_sprintf(strLeft, "%llu", result->td_u64BuyA);
    ol_sprintf(strRight, "%llu", result->td_u64SoldA);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*LambBuyA*/
    ol_sprintf(strLeft, "%llu", result->td_u64LambBuyA);
    ol_sprintf(strRight, "%llu", result->td_u64LambSoldA);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimitSoldA*/
    ol_sprintf(strLeft, "%llu", result->td_u64CloseHighLimitSoldA);
    ol_sprintf(strRight, "%llu", result->td_u64CloseHighLimitLambSoldA);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseLowLimitBuyA*/
    ol_sprintf(strLeft, "%llu", result->td_u64CloseLowLimitBuyA);
    ol_sprintf(strRight, "%llu", result->td_u64CloseLowLimitLambBuyA);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*UpperShadowRatio*/
    ol_sprintf(strLeft, "%.2f", result->td_dbUpperShadowRatio);
    ol_sprintf(strRight, "%.2f", result->td_dbLowerShadowRatio);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /* LastTimeOfLowPrice */
    ol_sprintf(strRight, "%.2f", result->td_dbLastXLowPrice);
    jf_clieng_printTwoHalfLine(pcc, result->td_strLastTimeLowPrice, strRight);
    pcc += 2;

    /* LastLowPriceInc */
    ol_sprintf(strLeft, "%.2f%%", result->td_dbLastXInc);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

}

static void _printTradeDetailFile(
    cli_parse_param_t * pcpp, tx_dr_t * buffer, olint_t total)
{
    tx_dr_t * cur;
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
    tx_dr_t * buffer = NULL;
    tx_dr_parse_param_t tdpp;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_stock_info_t * stockinfo;

    u32Ret = jf_mem_alloc((void **)&buffer, sizeof(tx_dr_t) * total);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = tx_stock_getStockInfo(pcpp->cpp_pstrStock, &stockinfo);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(&tdpp, 0, sizeof(tdpp));
        tdpp.tdpp_nThres = pcpp->cpp_nThres;
        if (tdpp.tdpp_nThres == 0)
            tdpp.tdpp_nThres = tx_stock_getStockShareThres(stockinfo);
        tdpp.tdpp_nStart = 1; //pcpp->cpp_nStart;
        tdpp.tdpp_nEnd = tdpp.tdpp_nStart + total - 1; //pcpp->cpp_nEnd;
        tdpp.tdpp_nLastCount = pcpp->cpp_nLastCount;
        tdpp.tdpp_pstrDateFrom = pcpp->cpp_pstrDate;
        ol_strcpy(tdpp.tdpp_strLastX, "14:45:00");
        ol_snprintf(
            dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
            tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcpp->cpp_pstrStock);

        u32Ret = tx_dr_readDrDir(dirpath, &tdpp, buffer, &total);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _printTradeDetailFile(pcpp, buffer, total);
    }

    if (buffer != NULL)
        jf_mem_free((void **)&buffer);

    return u32Ret;
}

static void _printTradeDaySummaryBrief(tx_ds_t * cur)
{
    jf_clieng_caption_t * pcc = &ls_ccTradeDaySummaryBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Day */
    ol_sprintf(strField, "%d", cur->td_nIndex);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Date */
    jf_clieng_appendBriefColumn(pcc, strInfo, cur->td_strDate);
    pcc++;

    /* OpeningPrice */
    ol_sprintf(strField, "%.2f", cur->td_dbOpeningPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosingPrice */
    ol_sprintf(strField, "%.2f", cur->td_dbClosingPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* HighPrice */
    ol_sprintf(strField, "%.2f", cur->td_dbHighPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LowPrice */
    ol_sprintf(strField, "%.2f", cur->td_dbLowPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* ClosePriceRate */
    ol_sprintf(strField, "%.2f", cur->td_dbClosingPriceRate);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* GeneralCapital */
    ol_sprintf(strField, "%lld", cur->td_u64GeneralCapital);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* TradableShare */
    ol_sprintf(strField, "%lld", cur->td_u64TradableShare);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /*Status*/
    tx_ds_getStringDsStatus(cur, strField);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    jf_clieng_outputLine(strInfo);
}

static void _printDaDaySummaryVerbose(tx_ds_t * summary)
{
    jf_clieng_caption_t * pcc = &ls_ccDaySummaryVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Day*/
    ol_sprintf(strLeft, "%d", summary->td_nIndex);
    jf_clieng_printTwoHalfLine(pcc, strLeft, summary->td_strDate);
    pcc += 2;

    /*OpeningPrice*/
    ol_sprintf(strLeft, "%.2f", summary->td_dbOpeningPrice);
    ol_sprintf(strRight, "%.2f", summary->td_dbClosingPrice);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*HighPrice*/
    ol_sprintf(strLeft, "%.2f", summary->td_dbHighPrice);
    ol_sprintf(strRight, "%.2f", summary->td_dbLowPrice);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*LastClosingPrice*/
    ol_sprintf(strLeft, "%.2f", summary->td_dbLastClosingPrice);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*ClosingPriceRate*/
    ol_sprintf(strLeft, "%.2f%%", summary->td_dbClosingPriceRate);
    ol_sprintf(strRight, "%.2f%%", summary->td_dbHighPriceRate);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*CloseHighLimit*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(summary->td_bCloseHighLimit));
    ol_sprintf(strRight, "%s", jf_string_getStringPositive(summary->td_bCloseLowLimit));
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*HighHighLimit*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(summary->td_bHighHighLimit));
    ol_sprintf(strRight, "%s", jf_string_getStringPositive(summary->td_bLowLowLimit));
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*Gap*/
    ol_sprintf(strLeft, "%s", jf_string_getStringPositive(summary->td_bGap));
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*All*/
    ol_sprintf(strLeft, "%llu", summary->td_u64All);
    ol_sprintf(strRight, "%.2f", summary->td_dbVolumeRatio);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*TurnoverRate*/
    ol_sprintf(strLeft, "%.2f", summary->td_dbTurnoverRate);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*AllA*/
    ol_sprintf(strLeft, "%llu", summary->td_u64AllA);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;

    /*UpperShadowRatio*/
    ol_sprintf(strLeft, "%.2f", summary->td_dbUpperShadowRatio);
    ol_sprintf(strRight, "%.2f", summary->td_dbLowerShadowRatio);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*GeneralCapital*/
    ol_sprintf(strLeft, "%lld", summary->td_u64GeneralCapital);
    ol_sprintf(strRight, "%lld", summary->td_u64TradableShare);
    jf_clieng_printTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;

    /*Status*/
    tx_ds_getStringDsStatus(summary, strLeft);
    jf_clieng_printOneFullLine(pcc, strLeft);
    pcc += 1;
}

static void _printTradeDaySummary(
    cli_parse_param_t * pcpp, tx_ds_t * buffer, olint_t total)
{
    tx_ds_t * cur;
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
    tx_ds_t * buffer = NULL;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];

    if (pcpp->cpp_nLastCount != 0)
        total = pcpp->cpp_nLastCount;

    u32Ret = jf_mem_alloc((void **)&buffer, sizeof(tx_ds_t) * total);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    ol_snprintf(
        dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
        tx_env_getVar(TX_ENV_VAR_DATA_PATH), PATH_SEPARATOR, pcpp->cpp_pstrStock);

	if (pcpp->cpp_bFRoR)
    {
        if (pcpp->cpp_bStartDate)
            u32Ret = tx_ds_readDsFromDateWithFRoR(
                dirpath, pcpp->cpp_pstrDate, buffer, &total);
        else if (pcpp->cpp_bEndDate)
            u32Ret = tx_ds_readDsUntilDateWithFRoR(
                dirpath, pcpp->cpp_pstrDate, pcpp->cpp_u32Count, buffer, &total);
        else
            u32Ret = tx_ds_readDsWithFRoR(dirpath, buffer, &total);
    }
	else
    {
        if (pcpp->cpp_bStartDate)
            u32Ret = tx_ds_readDsFromDate(
                dirpath, pcpp->cpp_pstrDate, buffer, &total);
        else if (pcpp->cpp_bEndDate)
            u32Ret = tx_ds_readDsUntilDate(
                dirpath, pcpp->cpp_pstrDate, pcpp->cpp_u32Count, buffer, &total);
        else
            u32Ret = tx_ds_readDs(dirpath, buffer, &total);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _printTradeDaySummary(pcpp, buffer, total);
    }

    jf_mem_free((void **)&buffer);

    return u32Ret;
}

static void _printQuoEntryBrief(tx_quo_entry_t * entry)
{
    jf_clieng_caption_t * pcc = &ls_ccQuoEntryBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* time */
    ol_sprintf(strField, "%s", entry->tqe_strTime);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* CurP */
    ol_sprintf(strField, "%.2f", entry->tqe_dbCurPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* HighP */
    ol_sprintf(strField, "%.2f", entry->tqe_dbHighPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* LowP */
    ol_sprintf(strField, "%.2f", entry->tqe_dbLowPrice);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Volumn */
    ol_sprintf(strField, "%llu", entry->tqe_u64Volume);
    jf_clieng_appendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Amount */
    ol_sprintf(strField, "%.2f", entry->tqe_dbAmount);
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

static void _analysisStockQuo(stock_quo_t * pstockquo, tx_quo_entry_t ** ptqe, olint_t total)
{
//    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;
    u64 u64Vol;
    olint_t start, end;

    jf_clieng_outputLine(
        "%s, %.2f, %lld", ptqe[0]->tqe_strTime, ptqe[0]->tqe_dbCurPrice, ptqe[0]->tqe_u64Volume);
    u64Vol = ptqe[0]->tqe_u64Volume;
    for (i = 1; i < total; i ++)
    {
        if (ptqe[i]->tqe_dbCurPrice > ptqe[i - 1]->tqe_dbCurPrice)
        {
            u64Vol = u64Vol + (ptqe[i]->tqe_u64Volume - ptqe[i - 1]->tqe_u64Volume);
        }
        else
        {
            u64Vol = u64Vol - (ptqe[i]->tqe_u64Volume - ptqe[i - 1]->tqe_u64Volume);
        }
        jf_clieng_outputLine("%s, %.2f, %lld", ptqe[i]->tqe_strTime, ptqe[i]->tqe_dbCurPrice, u64Vol);
    }

    jf_clieng_outputLine("");
    start = 0;
    jf_clieng_outputLine("%s, %.2f", ptqe[start]->tqe_strTime, ptqe[start]->tqe_dbCurPrice);
    while (start < total)
    {
        end = getNextTopBottomQuoEntry(ptqe, total, start);

        jf_clieng_outputLine(
            "%s(%.2f) --> %s(%.2f)", ptqe[start]->tqe_strTime, ptqe[start]->tqe_dbCurPrice,
            ptqe[end]->tqe_strTime, ptqe[end]->tqe_dbCurPrice);

        /*Do something from start to end*/

        if (end == total - 1)
            break;
        start = end;
    }
}

#if 0
static void _analysisStockQuo(
    stock_quo_t * pstockquo, tx_quo_entry_t ** ptqe, olint_t total)
{
    oldouble_t * pdba = NULL, * pdbb = NULL, dbr;
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, j, max;
    tx_quo_entry_t * start;

    jf_jiukun_allocMemory((void **)&pdba, sizeof(oldouble_t) * pstockquo->sq_nNumOfEntry);
    jf_jiukun_allocMemory((void **)&pdbb, sizeof(oldouble_t) * pstockquo->sq_nNumOfEntry);

    for (i = 1; i < total; i ++)
    {

        start = &pstockquo->sq_ptqeEntry[2];
        if (ptqe[i] <= start)
            dbr = 0;
        else
        {
            max = ptqe[i] - &pstockquo->sq_ptqeEntry[1];

            for (j = 0; j < max; j ++)
            {
                pdba[j] = start->tqe_dbCurPrice;
                pdbb[j] = (oldouble_t)(start->tqe_u64Volume - (start - 1)->tqe_u64Volume);
                start ++;
//                jf_clieng_outputLine("%.2f, %.2f", pdba[j], pdbb[j]);
            }
            u32Ret = getCorrelation(pdba, pdbb, max, &dbr);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_clieng_outputLine("%s, Price & Volumn: %.2f", ptqe[i]->tqe_strTime, dbr);
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
    tx_quo_entry_t ** ptqe = NULL;
    stock_quo_t stockquo;
    tx_stock_info_t * stockinfo;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
    tx_quo_entry_t * entry;

    memset(&stockquo, 0, sizeof(stock_quo_t));

    u32Ret = tx_stock_getStockInfo(pcpp->cpp_pstrStock, &stockinfo);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        stockquo.sq_nMaxEntry = 3500; //4 * 60 * 60 / 6; /*6 items per minutes*/
        ol_strcpy(stockquo.sq_strCode, stockinfo->tsi_strCode);
        if (pcpp->cpp_pstrDate != NULL)
            ol_strcpy(stockquo.sq_strDate, pcpp->cpp_pstrDate);

        u32Ret = jf_mem_alloc(
            (void **)&stockquo.sq_ptqeEntry, sizeof(tx_quo_entry_t) * stockquo.sq_nMaxEntry);
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

        entry = stockquo.sq_ptqeEntry;
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
        jf_jiukun_allocMemory((void **)&ptqe, sizeof(tx_quo_entry_t *) * total);
        jf_clieng_outputLine("");
        getQuoEntryInflexionPoint(stockquo.sq_ptqeEntry, stockquo.sq_nNumOfEntry, ptqe, &total);
        jf_clieng_outputLine("Total %d points", total);

        for (i = 0; i < total; i ++)
        {
            jf_clieng_outputLine("%s %.2f", ptqe[i]->tqe_strTime, ptqe[i]->tqe_dbCurPrice);
        }
        jf_clieng_outputLine("");

        _analysisStockQuo(&stockquo, ptqe, total);
        jf_clieng_outputLine("");

        jf_jiukun_freeMemory((void **)&ptqe);
    }

    if (stockquo.sq_ptqeEntry != NULL)
        jf_mem_free((void **)&stockquo.sq_ptqeEntry);

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


