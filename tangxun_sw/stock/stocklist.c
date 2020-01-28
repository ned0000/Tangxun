/**
 *  @file stocklist.c
 *
 *  @brief manage stock information
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
#include "jf_listhead.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_time.h"
#include "jf_jiukun.h"

#include "tx_err.h"

#include "common.h"
#include "stocklist.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define MAX_STOCKS       (4000)

typedef struct
{
    tx_stock_info_t * tsl_ptsiStock;
    olint_t tsl_nNumOfStock;
    olint_t tsl_nMaxStock;
    boolean_t tsl_bChanged;
    boolean_t tsl_bReserved[7];
} tx_stock_list_t;

static tx_stock_list_t ls_tslStockList;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _findStock(tx_stock_list_t * ptsl, const olchar_t * name, tx_stock_info_t ** info)
{
    u32 u32Ret = TX_ERR_STOCK_NOT_FOUND;
    olint_t begin, end, index, ret;

    begin = 0;
    end = ptsl->tsl_nNumOfStock - 1;
    
    /*binary search*/
    while (begin <= end)
    {
        index = (begin + end) / 2;
        ret = strncmp(
            ptsl->tsl_ptsiStock[index].tsi_strCode, name,
            ol_strlen(ptsl->tsl_ptsiStock[index].tsi_strCode));
        if (ret == 0)
        {
            *info = &ptsl->tsl_ptsiStock[index];
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }
        else
        {
            if (ret < 0)
            {
                begin = index + 1;
            }
            else
            {
                end = index - 1;
            }
        }
    }

    return u32Ret;
}

static u32 _fillStockInfo(
    olchar_t * line, olsize_t sline, tx_stock_info_t * stockinfo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * result = NULL;
    jf_string_parse_result_field_t * field = NULL;

    ol_bzero(stockinfo, sizeof(tx_stock_info_t));

    u32Ret = jf_string_parse(&result, line, 0, sline, "\t", 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (result->jspr_u32NumOfResult != 4)
            u32Ret = JF_ERR_INVALID_DATA;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = result->jspr_pjsprfFirst;
        if (field->jsprf_sData < 8)
            u32Ret = JF_ERR_INVALID_DATA;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_strncpy(stockinfo->tsi_strCode, field->jsprf_pstrData, 8);
        jf_string_lower(stockinfo->tsi_strCode);

        field = field->jsprf_pjsprfNext;
        u32Ret = setStockIndustry(field->jsprf_pstrData, field->jsprf_sData, stockinfo);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = field->jsprf_pjsprfNext;
        if (field->jsprf_pstrData[0] != '-')
            u32Ret = jf_string_getU64FromString(
                field->jsprf_pstrData, field->jsprf_sData, &stockinfo->tsi_u64GeneralCapital);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = field->jsprf_pjsprfNext;
        if (field->jsprf_pstrData[0] != '-')
            /*Remove the '\n'.*/
            u32Ret = jf_string_getU64FromString(
                field->jsprf_pstrData, field->jsprf_sData - 1, &stockinfo->tsi_u64TradableShare);
    }

    if (result != NULL)
        jf_string_destroyParseResult(&result);

    return u32Ret;
}

static u32 _readStockList(olchar_t * pstrStockListFile, tx_stock_list_t * stocklist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_t fd = 0;
    olchar_t line[512];
    olsize_t sline = 0;
    tx_stock_info_t * stockinfo;
    olint_t lineno = 1;

    stockinfo = stocklist->tsl_ptsiStock;

    u32Ret = jf_file_open(pstrStockListFile, O_RDONLY, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        do
        {
            sline = sizeof(line);
            u32Ret = jf_file_readLine(fd, line, &sline);
            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _fillStockInfo(line, sline, stockinfo);

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                stocklist->tsl_nNumOfStock ++;
                if (stocklist->tsl_nNumOfStock >= stocklist->tsl_nMaxStock)
                    u32Ret = JF_ERR_BUFFER_TOO_SMALL;
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                stockinfo ++;
                lineno ++;
            }
        } while (u32Ret == JF_ERR_NO_ERROR);

        if (u32Ret == JF_ERR_END_OF_FILE)
            u32Ret = JF_ERR_NO_ERROR;
        jf_file_close(&fd);
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
		JF_LOGGER_ERR(u32Ret, "found ERROR at line %d", lineno);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 findStockInfo(const olchar_t * name, tx_stock_info_t ** info)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    *info = NULL;
    u32Ret = _findStock(&ls_tslStockList, name, info);

    return u32Ret;
}

u32 initStockList(tx_stock_init_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t size = 0;
    tx_stock_list_t * ptsl = &ls_tslStockList;

    ol_bzero(ptsl, sizeof(tx_stock_list_t));

    ptsl->tsl_nMaxStock = MAX_STOCKS;
    size = sizeof(tx_stock_info_t) * ptsl->tsl_nMaxStock;
    u32Ret = jf_jiukun_allocMemory((void **)&ptsl->tsl_ptsiStock, size);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(ptsl->tsl_ptsiStock, size);

        u32Ret = _readStockList(param->tsip_pstrStockListFile, ptsl);
    }

    if (u32Ret != JF_ERR_NO_ERROR)
	{
		JF_LOGGER_ERR(u32Ret, "Failed to initiate stock list");
        finiStockList();
	}

    return u32Ret;
}

u32 finiStockList(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_list_t * ptsl = &ls_tslStockList;

    if (ptsl->tsl_ptsiStock != NULL)
    {
        jf_jiukun_freeMemory((void **)&ptsl->tsl_ptsiStock);
    }

    return u32Ret;
}

tx_stock_info_t * tx_stock_getFirstStockInfo(void)
{
    return ls_tslStockList.tsl_ptsiStock;
}

tx_stock_info_t * tx_stock_getNextStockInfo(tx_stock_info_t * info)
{
    tx_stock_info_t * ptsi = NULL;
    tx_stock_list_t * ptsl = &ls_tslStockList;

    if (info < ptsl->tsl_ptsiStock + ptsl->tsl_nNumOfStock - 1)
        ptsi = info + 1;

    return ptsi;
}

olint_t tx_stock_getNumOfStock(void)
{
    tx_stock_list_t * ptsl = &ls_tslStockList;

    return ptsl->tsl_nNumOfStock;
}

olint_t tx_stock_getStockShareThres(tx_stock_info_t * stockinfo)
{
    olint_t ret;

    if (stockinfo->tsi_u64TradableShare > TX_STOCK_SMALL_MEDIUM_STOCK_SHARE)
        ret = 800;
    else if (stockinfo->tsi_u64TradableShare > TX_STOCK_GROWTH_STOCK_SHARE)
        ret = 400;
    else if (stockinfo->tsi_u64TradableShare > TX_STOCK_TINY_STOCK_SHARE)
        ret = 200;
    else
        ret = 100;

    return ret;
}

boolean_t tx_stock_isSmallMediumStock(tx_stock_info_t * stockinfo)
{
    boolean_t bRet = TRUE;

    if (stockinfo->tsi_u64TradableShare > TX_STOCK_SMALL_MEDIUM_STOCK_SHARE)
        bRet = FALSE;

    return bRet;
}

boolean_t tx_stock_isShStockExchange(olchar_t * stock)
{
   if (ol_strncmp(stock, "sh", 2) == 0)
       return TRUE;

   return FALSE;
}

/*------------------------------------------------------------------------------------------------*/


