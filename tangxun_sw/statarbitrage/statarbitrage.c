/**
 *  @file statarbitrage.c
 *
 *  @brief statistic arbitrage
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
#include "jf_matrix.h"
#include "jf_jiukun.h"

#include "tx_indi.h"
#include "tx_statarbi.h"
#include "tx_datastat.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** 1 year
 */
#define SA_MIN_DS_CORRELATION  (250)

/** 3 months
 */
#define SA_MIN_DS_SPREAD       (60)

/* --- private routine section ------------------------------------------------------------------ */

static u32 _getClosingPricePair(
    tx_statarbi_stock_t * ptssa, tx_statarbi_stock_t * ptssb, oldouble_t * pdba, oldouble_t * pdbb,
    olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * cura, * enda, * curb, * endb;
    olint_t ret, count = 0, index = num - 1, mismatch = 0;

    cura = ptssa->tss_ptdSummary;
    enda = cura + ptssa->tss_nDaySummary - 1;
    curb = ptssb->tss_ptdSummary;
    endb = curb + ptssb->tss_nDaySummary - 1;

    if (ol_strncmp(enda->td_strDate, endb->td_strDate, 10) != 0)
        return JF_ERR_NOT_READY;

    while ((enda >= cura) && (endb >= curb))
    {
        ret = ol_strcmp(enda->td_strDate, endb->td_strDate);
        if (ret == 0)
        {
            pdba[index] = enda->td_dbClosingPrice;
            pdbb[index] = endb->td_dbClosingPrice;
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

static void _freeSaStock(u32 u32Stock, tx_statarbi_stock_t ** ppsastock)
{
	tx_statarbi_stock_t * sastock = *ppsastock;
	u32 index = 0;

	for (index = 0; index < u32Stock; index ++)
	{
		if (sastock[index].tss_ptdSummary != NULL)
			jf_mem_free((void **)&sastock[index].tss_ptdSummary);
	}

	jf_mem_free((void **)ppsastock);
}

static u32 _newSaStock(
	char * pstrDataPath, tx_stock_info_t ** ppStocks, u32 u32Stock, olint_t nDaySummary,
    tx_statarbi_stock_t ** ppsastock)
{
	u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
	tx_statarbi_stock_t * sastock = NULL;
	u32 index = 0;

	u32Ret = jf_jiukun_allocMemory((void **)&sastock, sizeof(tx_statarbi_stock_t) * u32Stock);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
		for (index = 0; (index < u32Stock) && (u32Ret == JF_ERR_NO_ERROR); index ++)
		{
            sastock[index].tss_ptsiStock = ppStocks[index];
			sastock[index].tss_nDaySummary = nDaySummary;
			u32Ret = jf_jiukun_allocMemory(
				(void **)&sastock[index].tss_ptdSummary,
                sizeof(tx_ds_t) * sastock[index].tss_nDaySummary);

			if (u32Ret == JF_ERR_NO_ERROR)
			{
				ol_snprintf(
					dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s", pstrDataPath, PATH_SEPARATOR,
					sastock[index].tss_ptsiStock->tsi_strCode);

				u32Ret = tx_ds_readDsWithFRoR(
					dirpath, sastock[index].tss_ptdSummary, &sastock[index].tss_nDaySummary);
			}
		}
	}

	if (u32Ret == JF_ERR_NO_ERROR)
		*ppsastock = sastock;
	else if (sastock != NULL)
		_freeSaStock(u32Stock, &sastock);

	return u32Ret;
}

#if 0
static u32 _statArbitrageIndustry_MAX(
    olchar_t * pstrDataPath, olint_t induid, tx_statarbi_eval_param_t * param,
    tx_statarbi_eval_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stock_indu_info_t * info;
    olint_t i, j;
    tx_statarbi_stock_t * sastock = NULL;
    oldouble_t dbr, maxdbr;
    oldouble_t * pdba = NULL, * pdbb = NULL;
    olint_t maxi, maxj;
    olint_t nDaySummary = param->tsep_nDaySummary;
    olint_t nArray = param->tsep_nCorrelationArray;
    tx_statarbi_eval_result_entry_t * entry;

    u32Ret = getIndustryInfo(induid, &info);
    if (u32Ret == JF_ERR_NO_ERROR)
		u32Ret = _newSaStock(
			pstrDataPath, info->tsii_pstrStocks, info->tsii_u32Stock,
            nDaySummary, &sastock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdba, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdbb, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        maxdbr = 0;
        maxi = maxj = 0;
        for (i = 0; i < info->tsii_u32Stock; i ++)
        {
            for (j = i + 1; j < info->tsii_u32Stock; j ++)
            {
                u32Ret = _getClosingPricePair(
                    &sastock[i], &sastock[j], pdba, pdbb, nArray);
                if (u32Ret == JF_ERR_NO_ERROR)
                    u32Ret = tx_datastat_getCorrelation(pdba, pdbb, nArray, &dbr);

                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    if (dbr > maxdbr)
                    {
                        maxi = i;
                        maxj = j;
                        maxdbr = dbr;
                    }
                }
            }
        }

        if (maxdbr >= param->tsep_dbMinCorrelation)
        {
            if (result->tser_nNumOfPair < result->tser_nMaxPair)
            {
                entry = &result->tser_ptsereEntry[result->tser_nNumOfPair];
                entry->tsere_dbCorrelation = maxdbr;
                entry->tsere_nInduId = induid;
                ol_strcpy(
                    entry->tsere_strStockPair, sastock[maxi].tss_ptsiStock->tsi_strCode);
                ol_strcat(entry->tsere_strStockPair, ",");
                ol_strcat(
                    entry->tsere_strStockPair, sastock[maxj].tss_ptsiStock->tsi_strCode);
                result->tser_nNumOfPair ++;
            }
        }
    }

    if (sastock != NULL)
        _freeSaStock(info->tsii_u32Stock, &sastock);
    if (pdba != NULL)
        jf_mem_free((void **)&pdba);
    if (pdbb != NULL)
        jf_mem_free((void **)&pdbb);

    return u32Ret;
}
#endif

static u32 _statArbitrageStock(
    olchar_t * pstrDataPath, tx_stock_info_t * stockinfo, tx_statarbi_eval_param_t * param,
    tx_statarbi_eval_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_indu_info_t * info;
    olint_t i, j;
    tx_statarbi_stock_t * sastock = NULL;
    oldouble_t dbr;
    oldouble_t * pdba = NULL, * pdbb = NULL;
    olint_t nDaySummary = param->tsep_nDaySummary;
    olint_t nArray = param->tsep_nCorrelationArray;
    tx_statarbi_eval_result_entry_t * entry;

    u32Ret = tx_stock_getInduInfo(stockinfo->tsi_nIndustry, &info);
    if (u32Ret == JF_ERR_NO_ERROR)
		u32Ret = _newSaStock(
			pstrDataPath, info->tsii_pptsiStocks, info->tsii_u32Stock, nDaySummary, &sastock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdba, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdbb, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < info->tsii_u32Stock; i ++)
            if (sastock[i].tss_ptsiStock == stockinfo)
                break;

        for (j = 0; j < info->tsii_u32Stock; j ++)
        {
            if (j == i)
                continue;

            u32Ret = _getClosingPricePair(
                &sastock[i], &sastock[j], pdba, pdbb, nArray);
            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = tx_datastat_getCorrelation(pdba, pdbb, nArray, &dbr);

            if ((u32Ret == JF_ERR_NO_ERROR) &&
                (dbr >= param->tsep_dbMinCorrelation))         
            {
                if (result->tser_nNumOfPair < result->tser_nMaxPair)
                {
                    entry = &result->tser_ptsereEntry[result->tser_nNumOfPair];
                    entry->tsere_dbCorrelation = dbr;
                    entry->tsere_nInduId = stockinfo->tsi_nIndustry;
                    ol_strcpy(
                        entry->tsere_strStockPair, sastock[i].tss_ptsiStock->tsi_strCode);
                    ol_strcat(entry->tsere_strStockPair, ",");
                    ol_strcat(
                        entry->tsere_strStockPair, sastock[j].tss_ptsiStock->tsi_strCode);
                    result->tser_nNumOfPair ++;
                }
                else
                    goto out;
            }
        }
    }
out:
    if (sastock != NULL)
        _freeSaStock(info->tsii_u32Stock, &sastock);
    if (pdba != NULL)
        jf_mem_free((void **)&pdba);
    if (pdbb != NULL)
        jf_mem_free((void **)&pdbb);

    return u32Ret;
}

static u32 _statArbitrageStockList(
    olchar_t * pstrDataPath, tx_stock_info_t ** ppStocks, u32 u32Stock,
    tx_statarbi_eval_param_t * param, tx_statarbi_eval_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, j;
    tx_statarbi_stock_t * sastock = NULL;
    oldouble_t dbr;
    oldouble_t * pdba = NULL, * pdbb = NULL;
    olint_t nDaySummary = param->tsep_nDaySummary;
    olint_t nArray = param->tsep_nCorrelationArray;
    tx_statarbi_eval_result_entry_t * entry;

    u32Ret = _newSaStock(pstrDataPath, ppStocks, u32Stock, nDaySummary, &sastock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdba, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdbb, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < u32Stock; i ++)
        {
            for (j = i + 1; j < u32Stock; j ++)
            {
                u32Ret = _getClosingPricePair(&sastock[i], &sastock[j], pdba, pdbb, nArray);
                if (u32Ret == JF_ERR_NO_ERROR)
                    u32Ret = tx_datastat_getCorrelation(pdba, pdbb, nArray, &dbr);

                if ((u32Ret == JF_ERR_NO_ERROR) &&
                    (dbr >= param->tsep_dbMinCorrelation))         
                {
                    if (result->tser_nNumOfPair < result->tser_nMaxPair)
                    {
                        entry = &result->tser_ptsereEntry[result->tser_nNumOfPair];
                        entry->tsere_dbCorrelation = dbr;
                        entry->tsere_nInduId = 0;
                        ol_strcpy(entry->tsere_strStockPair, sastock[i].tss_ptsiStock->tsi_strCode);
                        ol_strcat(entry->tsere_strStockPair, ",");
                        ol_strcat(entry->tsere_strStockPair, sastock[j].tss_ptsiStock->tsi_strCode);
                        result->tser_nNumOfPair ++;
                    }
                    else
                    {
                        goto out;
                    }
                }
            }
        }
        u32Ret = JF_ERR_NO_ERROR;        
    }
out:
    if (sastock != NULL)
        _freeSaStock(u32Stock, &sastock);
    if (pdba != NULL)
        jf_mem_free((void **)&pdba);
    if (pdbb != NULL)
        jf_mem_free((void **)&pdbb);

    return u32Ret;
}

static u32 _statArbitrageIndustry(
    olchar_t * pstrDataPath, olint_t induid, tx_statarbi_eval_param_t * param,
    tx_statarbi_eval_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_indu_info_t * info;
    olint_t i, j;
    tx_statarbi_stock_t * sastock = NULL;
    oldouble_t dbr;
    oldouble_t * pdba = NULL, * pdbb = NULL;
    olint_t nDaySummary = param->tsep_nDaySummary;
    olint_t nArray = param->tsep_nCorrelationArray;
    tx_statarbi_eval_result_entry_t * entry;

    u32Ret = tx_stock_getInduInfo(induid, &info);
    if (u32Ret == JF_ERR_NO_ERROR)
		u32Ret = _newSaStock(
			pstrDataPath, info->tsii_pptsiStocks, info->tsii_u32Stock, nDaySummary, &sastock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdba, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdbb, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < info->tsii_u32Stock; i ++)
        {
            for (j = i + 1; j < info->tsii_u32Stock; j ++)
            {
                u32Ret = _getClosingPricePair(&sastock[i], &sastock[j], pdba, pdbb, nArray);
                if (u32Ret == JF_ERR_NO_ERROR)
                    u32Ret = tx_datastat_getCorrelation(pdba, pdbb, nArray, &dbr);

                if ((u32Ret == JF_ERR_NO_ERROR) &&
                    (dbr >= param->tsep_dbMinCorrelation))         
                {
                    if (result->tser_nNumOfPair < result->tser_nMaxPair)
                    {
                        entry = &result->tser_ptsereEntry[result->tser_nNumOfPair];
                        entry->tsere_dbCorrelation = dbr;
                        entry->tsere_nInduId = induid;
                        ol_strcpy(entry->tsere_strStockPair, sastock[i].tss_ptsiStock->tsi_strCode);
                        ol_strcat(entry->tsere_strStockPair, ",");
                        ol_strcat(entry->tsere_strStockPair, sastock[j].tss_ptsiStock->tsi_strCode);
                        result->tser_nNumOfPair ++;
                    }
                    else
                    {
                        goto out;
                    }
                }
            }
        }
    }
out:
    if (sastock != NULL)
        _freeSaStock(info->tsii_u32Stock, &sastock);
    if (pdba != NULL)
        jf_mem_free((void **)&pdba);
    if (pdbb != NULL)
        jf_mem_free((void **)&pdbb);

    return u32Ret;
}

static void _getStringSpreadBoundaryParam(
    struct tx_statarbi_desc * arbi, const tx_statarbi_desc_param_t * ptsdp, olchar_t * buf)
{

}

static void _getSpreadBoundaryParamFromString(
    struct tx_statarbi_desc * arbi, tx_statarbi_desc_param_t * ptsdp, const olchar_t * buf)
{

}

static void _getStringDescStatParam(
    struct tx_statarbi_desc * arbi, const tx_statarbi_desc_param_t * ptsdp, olchar_t * buf)
{

}

static void _getDescStatParamFromString(
    struct tx_statarbi_desc * arbi, tx_statarbi_desc_param_t * ptsdp, const olchar_t * buf)
{

}

static tx_statarbi_desc_t ls_tsdStatArbiDesc[] =
{
    {TX_STATARBI_SPREAD_BOUNDARY, "SASB", "Spread Boundary",
     _getStringSpreadBoundaryParam, _getSpreadBoundaryParamFromString},
    {TX_STATARBI_SPREAD_BOUNDARY, "SADS", "Descriptive Statistic",
     _getStringDescStatParam, _getDescStatParamFromString},
};

static u32 _convertStockStringToArray(
    olchar_t * pstrStocks, u32 u32Stock, tx_stock_info_t ** ppStocks)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    char name[16];
    u32 index;

    ol_bzero(name, sizeof(name));
    for (index = 0; (index < u32Stock) && (u32Ret == JF_ERR_NO_ERROR); index ++)
    {
        ol_strncpy(name, pstrStocks + index * 9, 8);
        u32Ret = tx_stock_getStockInfo(name, &ppStocks[index]);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

tx_statarbi_desc_t * tx_statarbi_getDesc(olint_t id)
{
    if ((id == 0) || (id >= TX_STATARBI_MAX))
        return NULL;

    return &ls_tsdStatArbiDesc[id - 1];
}

u32 tx_statarbi_evalStock(
    olchar_t * pstrDataPath, tx_stock_info_t * stockinfo, tx_statarbi_eval_param_t * param,
    tx_statarbi_eval_result_t * result)
{
	u32 u32Ret = JF_ERR_NO_ERROR;

	u32Ret = _statArbitrageStock(pstrDataPath, stockinfo, param, result);

    return u32Ret;
}

u32 tx_statarbi_evalStockArray(
    olchar_t * pstrDataPath, tx_stock_info_t ** ppStocks, u32 u32Stock,
    tx_statarbi_eval_param_t * param, tx_statarbi_eval_result_t * result)
{
	u32 u32Ret = JF_ERR_NO_ERROR;

	u32Ret = _statArbitrageStockList(pstrDataPath, ppStocks, u32Stock, param, result);

    return u32Ret;
}

u32 tx_statarbi_evalStockList(
    olchar_t * pstrDataPath, olchar_t * stocklist, u32 u32Stock, tx_statarbi_eval_param_t * param,
    tx_statarbi_eval_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_info_t ** ppStocks = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&ppStocks, sizeof(tx_stock_info_t *) * u32Stock);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(ppStocks, sizeof(tx_stock_info_t *) * u32Stock);

        u32Ret = _convertStockStringToArray(stocklist, u32Stock, ppStocks);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _statArbitrageStockList(pstrDataPath, ppStocks, u32Stock, param, result);

    if (ppStocks != NULL)
        jf_jiukun_freeMemory((void **)&ppStocks);

    return u32Ret;
}

u32 tx_statarbi_evalIndu(
    olchar_t * pstrDataPath, olint_t induid, tx_statarbi_eval_param_t * param,
    tx_statarbi_eval_result_t * result)
{
	u32 u32Ret = JF_ERR_NO_ERROR;

	u32Ret = _statArbitrageIndustry(pstrDataPath, induid, param, result);

    return u32Ret;
}

u32 tx_statarbi_evalAllIndu(
    olchar_t * pstrDataPath, tx_statarbi_eval_param_t * param,
    tx_statarbi_eval_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t id;

    for (id = TX_STOCK_INDU_AGRICULTURAL_PRODUCT; id < TX_STOCK_INDU_MAX; id ++)
    {
        u32Ret = _statArbitrageIndustry(pstrDataPath, id, param, result);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (result->tser_nNumOfPair == result->tser_nMaxPair)
                break;
        }
    }

    return u32Ret;
}

u32 tx_statarbi_freeStockInfo(tx_statarbi_stock_t ** ppsastock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    _freeSaStock(2, ppsastock);

    return u32Ret;
}

/*pstrStocks is the stock pair, with format sh600000,sh600001*/
u32 tx_statarbi_newStockInfo(
    olchar_t * pstrDataPath, olchar_t * pstrStocks, tx_statarbi_stock_t ** ppsastock,
    olint_t nDaySummary)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_statarbi_stock_t * sastock = NULL;
    tx_stock_info_t ** ppStocks = NULL;
    u32 u32Stock = 2;

    u32Ret = jf_jiukun_allocMemory((void **)&ppStocks, sizeof(tx_stock_info_t *) * u32Stock);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(ppStocks, sizeof(tx_stock_info_t *) * u32Stock);

        u32Ret = _convertStockStringToArray(pstrStocks, u32Stock, ppStocks);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _newSaStock(pstrDataPath, ppStocks, u32Stock, nDaySummary, &sastock);

    if (ppStocks != NULL)
        jf_jiukun_freeMemory((void **)ppStocks);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppsastock = sastock;
    else if (sastock != NULL)
        tx_statarbi_freeStockInfo(&sastock);

    return u32Ret;
}

oldouble_t tx_statarbi_getCorrelation(tx_statarbi_stock_t * sastock, olint_t nDaySummary)
{
    oldouble_t dbret = -9999.99;
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t * pdba = NULL, * pdbb = NULL;

    jf_jiukun_allocMemory((void **)&pdba, sizeof(oldouble_t) * nDaySummary);
    jf_jiukun_allocMemory((void **)&pdbb, sizeof(oldouble_t) * nDaySummary);

    u32Ret = _getClosingPricePair(&sastock[0], &sastock[1], pdba, pdbb, nDaySummary);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = tx_datastat_getCorrelation(pdba, pdbb, nDaySummary, &dbret);

    jf_jiukun_freeMemory((void **)&pdba);
    jf_jiukun_freeMemory((void **)&pdbb);

    return dbret;
}

/*------------------------------------------------------------------------------------------------*/


