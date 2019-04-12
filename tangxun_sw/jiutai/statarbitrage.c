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

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <math.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <stdlib.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_clieng.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "statarbitrage.h"
#include "datastat.h"
#include "jf_clieng.h"
#include "stocklist.h"
#include "jf_matrix.h"
#include "jf_jiukun.h"
#include "indicator.h"

/* --- private data/data structure section --------------------------------- */

/** 1 year
 */
#define SA_MIN_DS_CORRELATION  (250)

/** 3 months
 */
#define SA_MIN_DS_SPREAD       (60)

static jf_clieng_caption_t ls_jccStatArbiDescVerbose[] =
{
    {"Id", JF_CLIENG_CAP_HALF_LINE}, {"Name", JF_CLIENG_CAP_HALF_LINE},
    {"Desc", JF_CLIENG_CAP_FULL_LINE},
};

/* --- private routine section---------------------------------------------- */

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
			u32Ret = getStockInfo(name, &sastock[i].ssi_psiStock);
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
					sastock[i].ssi_psiStock->si_strCode);

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
			pstrDataPath, info->sii_pstrStocks, info->sii_nStock,
            nDaySummary, &sastock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdba, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdbb, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        maxdbr = 0;
        maxi = maxj = 0;
        for (i = 0; i < info->sii_nStock; i ++)
        {
            for (j = i + 1; j < info->sii_nStock; j ++)
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
                    entry->saire_strStockPair, sastock[maxi].ssi_psiStock->si_strCode);
                ol_strcat(entry->saire_strStockPair, ",");
                ol_strcat(
                    entry->saire_strStockPair, sastock[maxj].ssi_psiStock->si_strCode);
                result->sair_nNumOfPair ++;
            }
        }
    }

    if (sastock != NULL)
        _freeSaStock(info->sii_nStock, &sastock);
    if (pdba != NULL)
        jf_mem_free((void **)&pdba);
    if (pdbb != NULL)
        jf_mem_free((void **)&pdbb);

    return u32Ret;
}
#endif

static u32 _statArbitrageStock(
    olchar_t * pstrDataPath, stock_info_t * stockinfo, stat_arbi_indu_param_t * param,
    stat_arbi_indu_result_t * result)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stock_indu_info_t * info;
    olint_t i, j;
    sa_stock_info_t * sastock = NULL;
    oldouble_t dbr;
    oldouble_t * pdba = NULL, * pdbb = NULL;
    olint_t nDaySummary = param->saip_nDaySummary;
    olint_t nArray = param->saip_nCorrelationArray;
    stat_arbi_indu_result_entry_t * entry;

    u32Ret = getIndustryInfo(stockinfo->si_nIndustry, &info);
    if (u32Ret == JF_ERR_NO_ERROR)
		u32Ret = _newSaStock(
			pstrDataPath, info->sii_pstrStocks, info->sii_nStock,
            nDaySummary, &sastock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdba, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdbb, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < info->sii_nStock; i ++)
            if (sastock[i].ssi_psiStock == stockinfo)
                break;

        for (j = 0; j < info->sii_nStock; j ++)
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
                    entry->saire_nInduId = stockinfo->si_nIndustry;
                    ol_strcpy(
                        entry->saire_strStockPair, sastock[i].ssi_psiStock->si_strCode);
                    ol_strcat(entry->saire_strStockPair, ",");
                    ol_strcat(
                        entry->saire_strStockPair, sastock[j].ssi_psiStock->si_strCode);
                    result->sair_nNumOfPair ++;
                }
                else
                    goto out;
            }
        }
    }
out:
    if (sastock != NULL)
        _freeSaStock(info->sii_nStock, &sastock);
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
                            entry->saire_strStockPair, sastock[i].ssi_psiStock->si_strCode);
                        ol_strcat(entry->saire_strStockPair, ",");
                        ol_strcat(
                            entry->saire_strStockPair, sastock[j].ssi_psiStock->si_strCode);
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
    stock_indu_info_t * info;
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
			pstrDataPath, info->sii_pstrStocks, info->sii_nStock,
            nDaySummary, &sastock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdba, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mem_alloc((void **)&pdbb, sizeof(oldouble_t) * nArray);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < info->sii_nStock; i ++)
        {
            for (j = i + 1; j < info->sii_nStock; j ++)
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
                            entry->saire_strStockPair, sastock[i].ssi_psiStock->si_strCode);
                        ol_strcat(entry->saire_strStockPair, ",");
                        ol_strcat(
                            entry->saire_strStockPair, sastock[j].ssi_psiStock->si_strCode);
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
        _freeSaStock(info->sii_nStock, &sastock);
    if (pdba != NULL)
        jf_mem_free((void **)&pdba);
    if (pdbb != NULL)
        jf_mem_free((void **)&pdbb);

    return u32Ret;
}

static u32 _optStatArbi(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * param,
    sa_stock_info_t * stocks, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;
    da_conc_t daconc;

    memset(&daconc, 0, sizeof(da_conc_t));
    memset(conc, 0, sizeof(da_conc_sum_t));

    for (i = SA_MIN_DS_CORRELATION;
         (i <= stocks[0].ssi_nDaySummary) && (u32Ret == JF_ERR_NO_ERROR); i ++)
    {
        u32Ret = arbi->sad_fnStatArbiStocks(
            arbi, param, stocks, &daconc);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (daconc.dc_nAction == CONC_ACTION_BULL_OPENING)
            {

            }
            else if (daconc.dc_nAction == CONC_ACTION_BULL_CLOSEOUT)
            {
                addToDaConcSum(conc, &daconc);
            }
        }
        else if (u32Ret == JF_ERR_NOT_READY)
            u32Ret = JF_ERR_NO_ERROR;
    }

    return u32Ret;
}

static boolean_t _isPureArray(
    oldouble_t * pdba, oldouble_t * pdbb, olint_t nArray)
{
    boolean_t bRet = TRUE;
    olint_t i;
    boolean_t bPos;

    if (pdba[0] >= pdbb[0])
        bPos = TRUE;
    else
        bPos = FALSE;

    for (i = 0; i < nArray; i ++)
    {
        if ((bPos && (pdba[i] < pdbb[i])) ||
            (!bPos && (pdba[i] >= pdbb[i])))
        {
            bRet = FALSE;
            break;
        }
    }

    return bRet;
}

static u32 _statArbiSpreadBoundaryOpening(
    stat_arbi_sb_param_t * psasp, sa_stock_info_t * sastock,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t * pdba, * pdbb, * pdbc;
    olint_t nArray = SA_MIN_DS_SPREAD;
    olint_t i;
    oldouble_t dbmax, dbmin, region, dbupper = 0; //, dblower = 0;

    jf_jiukun_allocMemory((void **)&pdba, sizeof(oldouble_t) * nArray, 0);
    jf_jiukun_allocMemory((void **)&pdbb, sizeof(oldouble_t) * nArray, 0);
    jf_jiukun_allocMemory((void **)&pdbc, sizeof(oldouble_t) * nArray, 0);
    pdba[nArray - 1] = pdbb[nArray - 1] = pdbc[nArray - 1] = 0;

    u32Ret = _getClosingPricePair(
        sastock, sastock + 1, pdba, pdbb, nArray);
    if ((u32Ret == JF_ERR_NO_ERROR) && ! _isPureArray(pdba, pdbb, nArray))
        u32Ret = JF_ERR_NOT_READY;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        dbmax = -99999.99;
        dbmin = 99999.99;
        for (i = 0; i < nArray; i ++)
        {
            pdbc[i] = ABS(pdba[i] - pdbb[i]);
            if (dbmax < pdbc[i])
                dbmax = pdbc[i];
            if (dbmin > pdbc[i])
                dbmin = pdbc[i];
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        region = (dbmax - dbmin) * psasp->sasp_dbBoundary / 100;
        dbupper = dbmax - region;
//        dblower = dbmin + region;

        if (pdbc[nArray - 1] >= dbupper)
        {
            if (pdba[nArray - 1] > pdbb[nArray - 1])
            {
                conc[1].dc_nAction = CONC_ACTION_BULL_OPENING;
                conc[1].dc_nPosition = CONC_POSITION_FULL;
                conc[1].dc_dbPrice = pdbb[nArray - 1];
                conc[1].dc_pddsOpening =
                    sastock[1].ssi_pddsSummary + sastock[1].ssi_nDaySummary - 1;
            }
            else
            {
                conc[0].dc_nAction = CONC_ACTION_BULL_OPENING;
                conc[0].dc_nPosition = CONC_POSITION_FULL;
                conc[0].dc_dbPrice = pdba[nArray - 1];
                conc[0].dc_pddsOpening =
                    sastock[0].ssi_pddsSummary + sastock[0].ssi_nDaySummary - 1;
            }
        }
    }
#if 0
    ol_printf("%s %.2f %.2f %.2f %.2f %.2f\n",
           (sastock[0].ssi_pddsSummary + sastock[0].ssi_nDaySummary - 1)->dds_strDate,
           pdba[nArray - 1], pdbb[nArray - 1], pdbc[nArray - 1], dbupper, dblower);
#endif

    jf_jiukun_freeMemory((void **)&pdba);
    jf_jiukun_freeMemory((void **)&pdbb);
    jf_jiukun_freeMemory((void **)&pdbc);

    return u32Ret;
}

static u32 _statArbiSpreadBoundaryCloseout(
    stat_arbi_sb_param_t * psasp, sa_stock_info_t * sastock,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t * pdba, * pdbb, * pdbc;
    olint_t nArray = SA_MIN_DS_SPREAD;
    olint_t i;
    oldouble_t dbmax, dbmin, region, dblower = 0; //dbupper = 0;
    boolean_t bCloseout = FALSE;
    da_day_summary_t * end[2];

    end[0] = sastock[0].ssi_pddsSummary + sastock[0].ssi_nDaySummary - 1;
    end[1] = sastock[1].ssi_pddsSummary + sastock[1].ssi_nDaySummary - 1;

    jf_jiukun_allocMemory((void **)&pdba, sizeof(oldouble_t) * nArray, 0);
    jf_jiukun_allocMemory((void **)&pdbb, sizeof(oldouble_t) * nArray, 0);
    jf_jiukun_allocMemory((void **)&pdbc, sizeof(oldouble_t) * nArray, 0);
    pdbc[nArray - 1] = 0;

    u32Ret = _getClosingPricePair(
        sastock, sastock + 1, pdba, pdbb, nArray);
    if ((u32Ret == JF_ERR_NO_ERROR) && ! _isPureArray(pdba, pdbb, nArray))
        u32Ret = JF_ERR_NOT_READY;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        dbmax = -99999.99;
        dbmin = 99999.99;
        for (i = 0; i < nArray; i ++)
        {
            pdbc[i] = ABS(pdba[i] - pdbb[i]);
            if (dbmax < pdbc[i])
                dbmax = pdbc[i];
            if (dbmin > pdbc[i])
                dbmin = pdbc[i];
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        region = (dbmax - dbmin) * psasp->sasp_dbBoundary / 100;
//        dbupper = dbmax - region;
        dblower = dbmin + region;

        if (pdbc[nArray - 1] <= dblower)
            bCloseout = TRUE;
    }

    if ((u32Ret != JF_ERR_NO_ERROR) || bCloseout)
    {
        if (conc[0].dc_nPosition == CONC_POSITION_FULL)
        {
            conc[0].dc_nAction = CONC_ACTION_BULL_CLOSEOUT;
            conc[0].dc_nPosition = CONC_POSITION_SHORT;
            conc[0].dc_dbYield =
                (end[0]->dds_dbClosingPrice - conc[0].dc_dbPrice) * 100 / conc[0].dc_dbPrice;
            conc[0].dc_dbPrice = end[0]->dds_dbClosingPrice;
            conc[0].dc_pddsCloseout = end[0];
            conc[0].dc_nHoldDays = conc[0].dc_pddsCloseout - conc[0].dc_pddsOpening;
        }

        if (conc[1].dc_nPosition == CONC_POSITION_FULL)
        {
            conc[1].dc_nAction = CONC_ACTION_BULL_CLOSEOUT;
            conc[1].dc_nPosition = CONC_POSITION_SHORT;
            conc[1].dc_dbYield =
                (end[1]->dds_dbClosingPrice - conc[1].dc_dbPrice) * 100 / conc[1].dc_dbPrice;
            conc[1].dc_dbPrice = end[1]->dds_dbClosingPrice;
            conc[1].dc_pddsCloseout = end[1];
            conc[1].dc_nHoldDays = conc[1].dc_pddsCloseout - conc[1].dc_pddsOpening;
        }
    }
#if 0
        ol_printf("%s %.2f %.2f %.2f %.2f %.2f\n",
               end[0]->dds_strDate,
               end[0]->dds_dbClosingPrice, end[1]->dds_dbClosingPrice, pdbc[nArray - 1], dbupper, dblower);
#endif

    return u32Ret;
}

static u32 _statArbiSpreadBoundary(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * psap,
    sa_stock_info_t * sastock, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stat_arbi_sb_param_t * psasp = &psap->sap_saspSb;
    da_day_summary_t * end[2];
    olint_t nMinDaySummary;

    conc[0].dc_nAction = CONC_ACTION_NONE;
    conc[1].dc_nAction = CONC_ACTION_NONE;

    if (psasp->sasp_dbBoundary == 0)
        psasp->sasp_dbBoundary = DEF_SA_DB_BOUNDARY;
    nMinDaySummary = SA_MIN_DS_SPREAD;
    if ((sastock[0].ssi_nDaySummary < nMinDaySummary) ||
        (sastock[1].ssi_nDaySummary < nMinDaySummary))
        return JF_ERR_NOT_READY;

    end[0] = sastock[0].ssi_pddsSummary + sastock[0].ssi_nDaySummary - 1;
    end[1] = sastock[1].ssi_pddsSummary + sastock[1].ssi_nDaySummary - 1;
    if (strcmp(end[0]->dds_strDate, end[1]->dds_strDate) != 0)
        u32Ret = JF_ERR_NOT_READY;

    if ((conc[0].dc_nPosition == CONC_POSITION_SHORT) &&
        (conc[1].dc_nPosition == CONC_POSITION_SHORT))
        u32Ret = _statArbiSpreadBoundaryOpening(
            psasp, sastock, conc);
    else
        u32Ret = _statArbiSpreadBoundaryCloseout(
            psasp, sastock, conc);

    return u32Ret;
}

static u32 _optimizeSpreadBoundary(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * psap,
    sa_stock_info_t * stocks, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t db;
    stat_arbi_param_t myparam;
    da_conc_sum_t myconc;

    memset(conc, 0, sizeof(da_conc_sum_t));
#define DEF_SA_DB_BOUNDARY_MIN  10
#define DEF_SA_DB_BOUNDARY_MAX  20

    for (db = DEF_SA_DB_BOUNDARY_MIN; db <= DEF_SA_DB_BOUNDARY_MAX; db = db + 1)
    {
        myparam.sap_saspSb.sasp_dbBoundary = db;
        u32Ret = _optStatArbi(arbi, &myparam, stocks, &myconc);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (compareDaConcSum(&myconc, conc) > 0)
            {
                memcpy(psap, &myparam, sizeof(stat_arbi_param_t));
                memcpy(conc, &myconc, sizeof(da_conc_sum_t));
            }
        }

        if (u32Ret != JF_ERR_NO_ERROR)
            goto out;
    }
out:
    return u32Ret;
}

static void _printSpreadBoundaryParamVerbose(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * pdip)
{




}

static void _getStringSpreadBoundaryParam(
    struct stat_arbi_desc * arbi, const stat_arbi_param_t * psap, olchar_t * buf)
{



}

static void _getSpreadBoundaryParamFromString(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * psap, const olchar_t * buf)
{




}

static u32 _statArbiDescStatOpening(
    stat_arbi_ds_param_t * psasp, sa_stock_info_t * sastock,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t * pdba, * pdbb, * pdbc;
    olint_t nArray = SA_MIN_DS_SPREAD;
    olint_t i;
    oldouble_t dbmax, dbmin, dbupper = 0, dblower = 0;
    desc_stat_t descstat;

    jf_jiukun_allocMemory((void **)&pdba, sizeof(oldouble_t) * nArray, 0);
    jf_jiukun_allocMemory((void **)&pdbb, sizeof(oldouble_t) * nArray, 0);
    jf_jiukun_allocMemory((void **)&pdbc, sizeof(oldouble_t) * nArray, 0);
    pdba[nArray - 1] = pdbb[nArray - 1] = pdbc[nArray - 1] = 0;

    u32Ret = _getClosingPricePair(
        sastock, sastock + 1, pdba, pdbb, nArray);
    if ((u32Ret == JF_ERR_NO_ERROR) && ! _isPureArray(pdba, pdbb, nArray))
        u32Ret = JF_ERR_NOT_READY;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        dbmax = -99999.99;
        dbmin = 99999.99;
        for (i = 0; i < nArray; i ++)
        {
            pdbc[i] = ABS(pdba[i] - pdbb[i]);
            if (dbmax < pdbc[i])
                dbmax = pdbc[i];
            if (dbmin > pdbc[i])
                dbmin = pdbc[i];
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = descStatFromData(&descstat, pdbc, nArray);
        dbupper = descstat.ds_dbMean + 2 * descstat.ds_dbStDev;
        dblower = descstat.ds_dbMean - 1 * descstat.ds_dbStDev;
        if (pdbc[nArray - 1] >= dbupper)
        {
            if (pdba[nArray - 1] > pdbb[nArray - 1])
            {
                conc[1].dc_nAction = CONC_ACTION_BULL_OPENING;
                conc[1].dc_nPosition = CONC_POSITION_FULL;
                conc[1].dc_dbPrice = pdbb[nArray - 1];
                conc[1].dc_pddsOpening =
                    sastock[1].ssi_pddsSummary + sastock[1].ssi_nDaySummary - 1;
            }
            else
            {
                conc[0].dc_nAction = CONC_ACTION_BULL_OPENING;
                conc[0].dc_nPosition = CONC_POSITION_FULL;
                conc[0].dc_dbPrice = pdba[nArray - 1];
                conc[0].dc_pddsOpening =
                    sastock[0].ssi_pddsSummary + sastock[0].ssi_nDaySummary - 1;
            }
        }
    }
#if 1
    ol_printf("%s %.2f %.2f %.2f %.2f %.2f\n",
           (sastock[0].ssi_pddsSummary + sastock[0].ssi_nDaySummary - 1)->dds_strDate,
           pdba[nArray - 1], pdbb[nArray - 1], pdbc[nArray - 1], dbupper, dblower);
#endif

    jf_jiukun_freeMemory((void **)&pdba);
    jf_jiukun_freeMemory((void **)&pdbb);
    jf_jiukun_freeMemory((void **)&pdbc);

    return u32Ret;
}

static u32 _statArbiDescStatCloseout(
    stat_arbi_ds_param_t * psasp, sa_stock_info_t * sastock,
    da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t * pdba, * pdbb, * pdbc;
    olint_t nArray = SA_MIN_DS_SPREAD;
    olint_t i;
    oldouble_t dbmax, dbmin, dbupper = 0, dblower = 0;
    boolean_t bCloseout = FALSE;
    da_day_summary_t * end[2];
    desc_stat_t descstat;

    end[0] = sastock[0].ssi_pddsSummary + sastock[0].ssi_nDaySummary - 1;
    end[1] = sastock[1].ssi_pddsSummary + sastock[1].ssi_nDaySummary - 1;

    jf_jiukun_allocMemory((void **)&pdba, sizeof(oldouble_t) * nArray, 0);
    jf_jiukun_allocMemory((void **)&pdbb, sizeof(oldouble_t) * nArray, 0);
    jf_jiukun_allocMemory((void **)&pdbc, sizeof(oldouble_t) * nArray, 0);
    pdbc[nArray - 1] = 0;

    u32Ret = _getClosingPricePair(
        sastock, sastock + 1, pdba, pdbb, nArray);
    if ((u32Ret == JF_ERR_NO_ERROR) && ! _isPureArray(pdba, pdbb, nArray))
        u32Ret = JF_ERR_NOT_READY;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        dbmax = -99999.99;
        dbmin = 99999.99;
        for (i = 0; i < nArray; i ++)
        {
            pdbc[i] = ABS(pdba[i] - pdbb[i]);
            if (dbmax < pdbc[i])
                dbmax = pdbc[i];
            if (dbmin > pdbc[i])
                dbmin = pdbc[i];
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = descStatFromData(&descstat, pdbc, nArray);
        dbupper = descstat.ds_dbMean + 2 * descstat.ds_dbStDev;
        dblower = descstat.ds_dbMean; // - 1 * descstat.ds_dbStDev;
        if (pdbc[nArray - 1] <= dblower)
            bCloseout = TRUE;
    }

    if ((u32Ret != JF_ERR_NO_ERROR) || bCloseout)
    {
        if (conc[0].dc_nPosition == CONC_POSITION_FULL)
        {
            conc[0].dc_nAction = CONC_ACTION_BULL_CLOSEOUT;
            conc[0].dc_nPosition = CONC_POSITION_SHORT;
            conc[0].dc_dbYield =
                (end[0]->dds_dbClosingPrice - conc[0].dc_dbPrice) * 100 / conc[0].dc_dbPrice;
            conc[0].dc_dbPrice = end[0]->dds_dbClosingPrice;
            conc[0].dc_pddsCloseout = end[0];
            conc[0].dc_nHoldDays = conc[0].dc_pddsCloseout - conc[0].dc_pddsOpening;
        }

        if (conc[1].dc_nPosition == CONC_POSITION_FULL)
        {
            conc[1].dc_nAction = CONC_ACTION_BULL_CLOSEOUT;
            conc[1].dc_nPosition = CONC_POSITION_SHORT;
            conc[1].dc_dbYield =
                (end[1]->dds_dbClosingPrice - conc[1].dc_dbPrice) * 100 / conc[1].dc_dbPrice;
            conc[1].dc_dbPrice = end[1]->dds_dbClosingPrice;
            conc[1].dc_pddsCloseout = end[1];
            conc[1].dc_nHoldDays = conc[1].dc_pddsCloseout - conc[1].dc_pddsOpening;
        }
    }
#if 1
        ol_printf("%s %.2f %.2f %.2f %.2f %.2f\n",
               end[0]->dds_strDate,
               end[0]->dds_dbClosingPrice, end[1]->dds_dbClosingPrice, pdbc[nArray - 1], dbupper, dblower);
#endif

    return u32Ret;
}

static u32 _statArbiDescStat(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * psap,
    sa_stock_info_t * sastock, da_conc_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stat_arbi_ds_param_t * psadp = &psap->sap_sadpDs;
    da_day_summary_t * end[2];
    olint_t nMinDaySummary;

    conc[0].dc_nAction = CONC_ACTION_NONE;
    conc[1].dc_nAction = CONC_ACTION_NONE;

    if (psadp->sadp_dbBoundary == 0)
        psadp->sadp_dbBoundary = DEF_SA_DS_BOUNDARY;
    nMinDaySummary = SA_MIN_DS_SPREAD;
    if ((sastock[0].ssi_nDaySummary < nMinDaySummary) ||
        (sastock[1].ssi_nDaySummary < nMinDaySummary))
        return JF_ERR_NOT_READY;

    end[0] = sastock[0].ssi_pddsSummary + sastock[0].ssi_nDaySummary - 1;
    end[1] = sastock[1].ssi_pddsSummary + sastock[1].ssi_nDaySummary - 1;
    if (strcmp(end[0]->dds_strDate, end[1]->dds_strDate) != 0)
        u32Ret = JF_ERR_NOT_READY;

    if ((conc[0].dc_nPosition == CONC_POSITION_SHORT) &&
        (conc[1].dc_nPosition == CONC_POSITION_SHORT))
        u32Ret = _statArbiDescStatOpening(
            psadp, sastock, conc);
    else
        u32Ret = _statArbiDescStatCloseout(
            psadp, sastock, conc);

    return u32Ret;
}

static u32 _optimizeDescStat(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * psap,
    sa_stock_info_t * stocks, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static void _printDescStatParamVerbose(
    struct stat_arbi_desc * arbi, stat_arbi_param_t * pdip)
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

static void _printStatArbiDescVerbose(
    struct stat_arbi_desc * arbi)
{
    jf_clieng_caption_t * pcc = &ls_jccStatArbiDescVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN]; //, strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /*Id*/
    ol_sprintf(strLeft, "%d", arbi->sad_nId);
    jf_clieng_printTwoHalfLine(pcc, strLeft, arbi->sad_pstrName);
    pcc += 2;

    /*Desc*/
    jf_clieng_printOneFullLine(pcc, arbi->sad_pstrDesc);
    pcc += 1;

}

static stat_arbi_desc_t ls_sadStatArbiDesc[] =
{
    {STAT_ARBI_SPREAD_BOUNDARY, "SASB", "Spread Boundary",
     _statArbiSpreadBoundary, _optimizeSpreadBoundary,
     _printSpreadBoundaryParamVerbose, _getStringSpreadBoundaryParam,
     _getSpreadBoundaryParamFromString, _printStatArbiDescVerbose},
    {STAT_ARBI_SPREAD_BOUNDARY, "SADS", "Descriptive Statistic",
     _statArbiDescStat, _optimizeDescStat,
     _printDescStatParamVerbose, _getStringDescStatParam,
     _getDescStatParamFromString, _printStatArbiDescVerbose},
};

/* --- public routine section ---------------------------------------------- */

stat_arbi_desc_t * getStatArbiDesc(olint_t id)
{
    if ((id == 0) || (id >= STAT_ARBI_MAX))
        return NULL;

    return &ls_sadStatArbiDesc[id - 1];
}

u32 statArbiStock(
    olchar_t * pstrDataPath, stock_info_t * stockinfo, stat_arbi_indu_param_t * param,
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

    for (id = STOCK_INDU_AGRICULTURAL_PRODUCT; id < STOCK_INDU_MAX; id ++)
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

u32 getBestStatArbiMethod(
    sa_stock_info_t * stocks, get_best_stat_arbi_param_t * pgbsap,
    olint_t * pnId, stat_arbi_param_t * psap, da_conc_sum_t * conc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stat_arbi_param_t myparam;
    da_conc_sum_t myconc;
    olint_t id;
    stat_arbi_desc_t * arbi;
    olchar_t buf[512];

    memset(conc, 0, sizeof(da_conc_sum_t));

    for (id = STOCK_INDICATOR_DMI; id < STOCK_INDICATOR_MAX; id ++)
    {
        arbi = getStatArbiDesc(id);
        u32Ret = arbi->sad_fnOptimize(arbi, &myparam, stocks, &myconc);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (pgbsap->gbsap_bVerbose)
            {
                jf_clieng_printBanner(JF_CLIENG_MAX_OUTPUT_LINE_LEN);
                arbi->sad_fnGetStringParam(arbi, &myparam, buf);
                jf_clieng_outputLine("%s System, %s", arbi->sad_pstrName, buf);
                arbi->sad_fnPrintParamVerbose(arbi, &myparam);
                printDaConcSumVerbose(&myconc);
                jf_clieng_outputLine("");
            }

            if (compareDaConcSum(&myconc, conc) > 0)
            {
                ol_memcpy(psap, &myparam, sizeof(stat_arbi_param_t));
                ol_memcpy(conc, &myconc, sizeof(da_conc_sum_t));
                *pnId = id;
            }
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((conc->dcs_dbOverallYield <= 0) ||
            (conc->dcs_dbProfitTimeRatio < pgbsap->gbsap_dbMinProfitTimeRatio))
            u32Ret = JF_ERR_NOT_FOUND;
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

    jf_jiukun_allocMemory((void **)&pdba, sizeof(oldouble_t) * nDaySummary, 0);
    jf_jiukun_allocMemory((void **)&pdbb, sizeof(oldouble_t) * nDaySummary, 0);

    u32Ret = _getClosingPricePair(
        &sastock[0], &sastock[1], pdba, pdbb, nDaySummary);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = getCorrelation(pdba, pdbb, nDaySummary, &dbret);

    jf_jiukun_freeMemory((void **)&pdba);
    jf_jiukun_freeMemory((void **)&pdbb);

    return dbret;
}

/*---------------------------------------------------------------------------*/


