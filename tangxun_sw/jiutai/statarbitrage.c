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
#include "statarbitrage.h"
#include "datastat.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** 1 year
 */
#define SA_MIN_DS_CORRELATION  (250)

/** 3 months
 */
#define SA_MIN_DS_SPREAD       (60)

/* --- private routine section ------------------------------------------------------------------ */

static u32 _getClosingPricePair(
    sa_stock_info_t * pssia, sa_stock_info_t * pssib,
    oldouble_t * pdba, oldouble_t * pdbb, olint_t num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_day_summary_t * cura, * enda, * curb, * endb;
    olint_t ret, count = 0, index = num - 1, mismatch = 0;

    cura = pssia->ssi_pddsSummary;
    enda = cura + pssia->ssi_nDaySummary - 1;
    curb = pssib->ssi_pddsSummary;
    endb = curb + pssib->ssi_nDaySummary - 1;

    if (strncmp(enda->dds_strDate, endb->dds_strDate, 10) != 0)
        return JF_ERR_NOT_READY;

    while ((enda >= cura) && (endb >= curb))
    {
        ret = strcmp(enda->dds_strDate, endb->dds_strDate);
        if (ret == 0)
        {
            pdba[index] = enda->dds_dbClosingPrice;
            pdbb[index] = endb->dds_dbClosingPrice;
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

static void _freeSaStock(olint_t nStock, sa_stock_info_t ** ppsastock)
{
	sa_stock_info_t * sastock = *ppsastock;
	olint_t i;

	for (i = 0; i < nStock; i ++)
	{
		if (sastock[i].ssi_pddsSummary != NULL)
			jf_mem_free((void **)&sastock[i].ssi_pddsSummary);
	}

	jf_mem_free((void **)ppsastock);
}

static u32 _newSaStock(
	char * pstrDataPath, olchar_t * pstrStocks, olint_t nStock,
    olint_t nDaySummary, sa_stock_info_t ** ppsastock)
{
	u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t dirpath[JF_LIMIT_MAX_PATH_LEN];
	sa_stock_info_t * sastock;
	char name[16];
	olint_t i;

	u32Ret = jf_mem_calloc((void **)&sastock, sizeof(sa_stock_info_t) * nStock);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
		ol_memset(name, 0, sizeof(name));
		for (i = 0; (i < nStock) && (u32Ret == JF_ERR_NO_ERROR); i ++)
		{
            ol_strncpy(name, pstrStocks + i * 9, 8);
			u32Ret = getStockInfo(name, &sastock[i].ssi_ptsiStock);
		}
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
		for (i = 0; (i < nStock) && (u32Ret == JF_ERR_NO_ERROR); i ++)
		{
			sastock[i].ssi_nDaySummary = nDaySummary;
			u32Ret = jf_mem_alloc(
				(void **)&sastock[i].ssi_pddsSummary,
				sizeof(da_day_summary_t) * sastock[i].ssi_nDaySummary);

			if (u32Ret == JF_ERR_NO_ERROR)
			{
				ol_snprintf(
					dirpath, JF_LIMIT_MAX_PATH_LEN, "%s%c%s",
					pstrDataPath, PATH_SEPARATOR,
					sastock[i].ssi_ptsiStock->tsi_strCode);

				u32Ret = readTradeDaySummaryWithFRoR(
					dirpath, sastock[i].ssi_pddsSummary, &sastock[i].ssi_nDaySummary);
			}
		}
	}

	if (u32Ret == JF_ERR_NO_ERROR)
		*ppsastock = sastock;
	else if (sastock != NULL)
		_freeSaStock(nStock, &sastock);

	return u32Ret;
}

#if 0
static u32 _statArbitrageIndustry_MAX(
    olchar_t * pstrDataPath, olint_t induid, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stock_indu_info_t * info;
    olint_t i, j;
    sa_stock_info_t * sastock = NULL;
    oldouble_t dbr, maxdbr;
    oldouble_t * pdba = NULL, * pdbb = NULL;
    olint_t maxi, maxj;
    olint_t nDaySummary = param->saip_nDaySummary;
    olint_t nArray = param->saip_nCorrelationArray;
    stat_arbi_indu_result_entry_t * entry;

    u32Ret = getIndustryInfo(induid, &info);
    if (u32Ret == JF_ERR_NO_ERROR)
		u32Ret = _newSaStock(
			pstrDataPath, info->tsii_pstrStocks, info->tsii_nStock,
            nDaySummary, &sastock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdba, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdbb, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        maxdbr = 0;
        maxi = maxj = 0;
        for (i = 0; i < info->tsii_nStock; i ++)
        {
            for (j = i + 1; j < info->tsii_nStock; j ++)
            {
                u32Ret = _getClosingPricePair(
                    &sastock[i], &sastock[j], pdba, pdbb, nArray);
                if (u32Ret == JF_ERR_NO_ERROR)
                    u32Ret = getCorrelation(pdba, pdbb, nArray, &dbr);

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

        if (maxdbr >= param->saip_dbMinCorrelation)
        {
            if (result->sair_nNumOfPair < result->sair_nMaxPair)
            {
                entry = &result->sair_psaireEntry[result->sair_nNumOfPair];
                entry->saire_dbCorrelation = maxdbr;
                entry->saire_nInduId = induid;
                ol_strcpy(
                    entry->saire_strStockPair, sastock[maxi].ssi_ptsiStock->tsi_strCode);
                ol_strcat(entry->saire_strStockPair, ",");
                ol_strcat(
                    entry->saire_strStockPair, sastock[maxj].ssi_ptsiStock->tsi_strCode);
                result->sair_nNumOfPair ++;
            }
        }
    }

    if (sastock != NULL)
        _freeSaStock(info->tsii_nStock, &sastock);
    if (pdba != NULL)
        jf_mem_free((void **)&pdba);
    if (pdbb != NULL)
        jf_mem_free((void **)&pdbb);

    return u32Ret;
}
#endif

static u32 _statArbitrageStock(
    olchar_t * pstrDataPath, tx_stock_info_t * stockinfo, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_indu_info_t * info;
    olint_t i, j;
    sa_stock_info_t * sastock = NULL;
    oldouble_t dbr;
    oldouble_t * pdba = NULL, * pdbb = NULL;
    olint_t nDaySummary = param->saip_nDaySummary;
    olint_t nArray = param->saip_nCorrelationArray;
    stat_arbi_indu_result_entry_t * entry;

    u32Ret = getIndustryInfo(stockinfo->tsi_nIndustry, &info);
    if (u32Ret == JF_ERR_NO_ERROR)
		u32Ret = _newSaStock(
			pstrDataPath, info->tsii_pstrStocks, info->tsii_nStock,
            nDaySummary, &sastock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdba, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdbb, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < info->tsii_nStock; i ++)
            if (sastock[i].ssi_ptsiStock == stockinfo)
                break;

        for (j = 0; j < info->tsii_nStock; j ++)
        {
            if (j == i)
                continue;

            u32Ret = _getClosingPricePair(
                &sastock[i], &sastock[j], pdba, pdbb, nArray);
            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = getCorrelation(pdba, pdbb, nArray, &dbr);

            if ((u32Ret == JF_ERR_NO_ERROR) &&
                (dbr >= param->saip_dbMinCorrelation))         
            {
                if (result->sair_nNumOfPair < result->sair_nMaxPair)
                {
                    entry = &result->sair_psaireEntry[result->sair_nNumOfPair];
                    entry->saire_dbCorrelation = dbr;
                    entry->saire_nInduId = stockinfo->tsi_nIndustry;
                    ol_strcpy(
                        entry->saire_strStockPair, sastock[i].ssi_ptsiStock->tsi_strCode);
                    ol_strcat(entry->saire_strStockPair, ",");
                    ol_strcat(
                        entry->saire_strStockPair, sastock[j].ssi_ptsiStock->tsi_strCode);
                    result->sair_nNumOfPair ++;
                }
                else
                    goto out;
            }
        }
    }
out:
    if (sastock != NULL)
        _freeSaStock(info->tsii_nStock, &sastock);
    if (pdba != NULL)
        jf_mem_free((void **)&pdba);
    if (pdbb != NULL)
        jf_mem_free((void **)&pdbb);

    return u32Ret;
}

static u32 _statArbitrageStockList(
    olchar_t * pstrDataPath, olchar_t * stocklist, olint_t nStock, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, j;
    sa_stock_info_t * sastock = NULL;
    oldouble_t dbr;
    oldouble_t * pdba = NULL, * pdbb = NULL;
    olint_t nDaySummary = param->saip_nDaySummary;
    olint_t nArray = param->saip_nCorrelationArray;
    stat_arbi_indu_result_entry_t * entry;

    u32Ret = _newSaStock(
        pstrDataPath, stocklist, nStock, nDaySummary, &sastock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdba, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdbb, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < nStock; i ++)
        {
            for (j = i + 1; j < nStock; j ++)
            {
                u32Ret = _getClosingPricePair(
                    &sastock[i], &sastock[j], pdba, pdbb, nArray);
                if (u32Ret == JF_ERR_NO_ERROR)
                    u32Ret = getCorrelation(pdba, pdbb, nArray, &dbr);

                if ((u32Ret == JF_ERR_NO_ERROR) &&
                    (dbr >= param->saip_dbMinCorrelation))         
                {
                    if (result->sair_nNumOfPair < result->sair_nMaxPair)
                    {
                        entry = &result->sair_psaireEntry[result->sair_nNumOfPair];
                        entry->saire_dbCorrelation = dbr;
                        entry->saire_nInduId = 0;
                        ol_strcpy(
                            entry->saire_strStockPair, sastock[i].ssi_ptsiStock->tsi_strCode);
                        ol_strcat(entry->saire_strStockPair, ",");
                        ol_strcat(
                            entry->saire_strStockPair, sastock[j].ssi_ptsiStock->tsi_strCode);
                        result->sair_nNumOfPair ++;
                    }
                    else
                        goto out;
                }
            }
        }
        u32Ret = JF_ERR_NO_ERROR;        
    }
out:
    if (sastock != NULL)
        _freeSaStock(nStock, &sastock);
    if (pdba != NULL)
        jf_mem_free((void **)&pdba);
    if (pdbb != NULL)
        jf_mem_free((void **)&pdbb);

    return u32Ret;
}

static u32 _statArbitrageIndustry(
    olchar_t * pstrDataPath, olint_t induid, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_indu_info_t * info;
    olint_t i, j;
    sa_stock_info_t * sastock = NULL;
    oldouble_t dbr;
    oldouble_t * pdba = NULL, * pdbb = NULL;
    olint_t nDaySummary = param->saip_nDaySummary;
    olint_t nArray = param->saip_nCorrelationArray;
    stat_arbi_indu_result_entry_t * entry;

    u32Ret = getIndustryInfo(induid, &info);
    if (u32Ret == JF_ERR_NO_ERROR)
		u32Ret = _newSaStock(
			pstrDataPath, info->tsii_pstrStocks, info->tsii_nStock,
            nDaySummary, &sastock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdba, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdbb, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < info->tsii_nStock; i ++)
        {
            for (j = i + 1; j < info->tsii_nStock; j ++)
            {
                u32Ret = _getClosingPricePair(
                    &sastock[i], &sastock[j], pdba, pdbb, nArray);
                if (u32Ret == JF_ERR_NO_ERROR)
                    u32Ret = getCorrelation(pdba, pdbb, nArray, &dbr);

                if ((u32Ret == JF_ERR_NO_ERROR) &&
                    (dbr >= param->saip_dbMinCorrelation))         
                {
                    if (result->sair_nNumOfPair < result->sair_nMaxPair)
                    {
                        entry = &result->sair_psaireEntry[result->sair_nNumOfPair];
                        entry->saire_dbCorrelation = dbr;
                        entry->saire_nInduId = induid;
                        ol_strcpy(
                            entry->saire_strStockPair, sastock[i].ssi_ptsiStock->tsi_strCode);
                        ol_strcat(entry->saire_strStockPair, ",");
                        ol_strcat(
                            entry->saire_strStockPair, sastock[j].ssi_ptsiStock->tsi_strCode);
                        result->sair_nNumOfPair ++;
                    }
                    else
                        goto out;
                }
            }
        }
    }
out:
    if (sastock != NULL)
        _freeSaStock(info->tsii_nStock, &sastock);
    if (pdba != NULL)
        jf_mem_free((void **)&pdba);
    if (pdbb != NULL)
        jf_mem_free((void **)&pdbb);

    return u32Ret;
}

static void _getStringSpreadBoundaryParam(
    struct stat_arbi_desc * arbi, const stat_arbi_param_t * psap, olchar_t * buf)
{



}

static void _getSpreadBoundaryParamFromString(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * psap, const olchar_t * buf)
{




}

static void _getStringDescStatParam(
    struct stat_arbi_desc * arbi, const stat_arbi_param_t * psap, olchar_t * buf)
{



}

static void _getDescStatParamFromString(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * psap, const olchar_t * buf)
{




}

static stat_arbi_desc_t ls_sadStatArbiDesc[] =
{
    {STAT_ARBI_SPREAD_BOUNDARY, "SASB", "Spread Boundary",
     _getStringSpreadBoundaryParam, _getSpreadBoundaryParamFromString},
    {STAT_ARBI_SPREAD_BOUNDARY, "SADS", "Descriptive Statistic",
     _getStringDescStatParam, _getDescStatParamFromString},
};

/* --- public routine section ------------------------------------------------------------------- */

stat_arbi_desc_t * getStatArbiDesc(olint_t id)
{
    if ((id == 0) || (id >= STAT_ARBI_MAX))
        return NULL;

    return &ls_sadStatArbiDesc[id - 1];
}

u32 statArbiStock(
    olchar_t * pstrDataPath, tx_stock_info_t * stockinfo, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result)
{
	u32 u32Ret = JF_ERR_NO_ERROR;

	u32Ret = _statArbitrageStock(pstrDataPath, stockinfo, param, result);

    return u32Ret;
}

u32 statArbiStockList(
    olchar_t * pstrDataPath, olchar_t * stocklist, olint_t nStock, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result)
{
	u32 u32Ret = JF_ERR_NO_ERROR;

	u32Ret = _statArbitrageStockList(pstrDataPath, stocklist, nStock, param, result);

    return u32Ret;
}

u32 statArbiIndustry(
    olchar_t * pstrDataPath, olint_t induid, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result)
{
	u32 u32Ret = JF_ERR_NO_ERROR;

	u32Ret = _statArbitrageIndustry(pstrDataPath, induid, param, result);

    return u32Ret;
}

u32 statArbiAllIndustry(
    olchar_t * pstrDataPath, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t id;

    for (id = TX_STOCK_INDU_AGRICULTURAL_PRODUCT; id < TX_STOCK_INDU_MAX; id ++)
    {
        u32Ret = _statArbitrageIndustry(pstrDataPath, id, param, result);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (result->sair_nNumOfPair == result->sair_nMaxPair)
                break;
        }
    }

    return u32Ret;
}

u32 freeSaStockInfo(sa_stock_info_t ** ppsastock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    _freeSaStock(2, ppsastock);

    return u32Ret;
}

/*pstrStocks is the stock pair, with format sh600000,sh600001*/
u32 newSaStockInfo(
    olchar_t * pstrDataPath, olchar_t * pstrStocks,
    sa_stock_info_t ** ppsastock, olint_t nDaySummary)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sa_stock_info_t * sastock = NULL;

	u32Ret = _newSaStock(pstrDataPath, pstrStocks, 2, nDaySummary, &sastock);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppsastock = sastock;
    else if (sastock != NULL)
        freeSaStockInfo(&sastock);

    return u32Ret;
}

oldouble_t getSaStockInfoCorrelation(
    sa_stock_info_t * sastock, olint_t nDaySummary)
{
    oldouble_t dbret = -9999.99;
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t * pdba = NULL, * pdbb = NULL;

    jf_jiukun_allocMemory((void **)&pdba, sizeof(oldouble_t) * nDaySummary);
    jf_jiukun_allocMemory((void **)&pdbb, sizeof(oldouble_t) * nDaySummary);

    u32Ret = _getClosingPricePair(
        &sastock[0], &sastock[1], pdba, pdbb, nDaySummary);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = getCorrelation(pdba, pdbb, nDaySummary, &dbret);

    jf_jiukun_freeMemory((void **)&pdba);
    jf_jiukun_freeMemory((void **)&pdbb);

    return dbret;
}

/*------------------------------------------------------------------------------------------------*/


