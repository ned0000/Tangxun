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
#include "jf_listhead.h"
#include "jf_mem.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_time.h"
#include "jf_jiukun.h"

#include "stocklist.h"

/* --- private data/data structure section --------------------------------- */

static stock_list_t ls_slStockList;

#define MAX_STOCKS       (4000)
#define STOCK_LIST_FILE  "../config/StockList.txt"

static stock_indu_info_t ls_siiInduInfo[] =
{
    {{0xC5, 0xA9, 0xB2, 0xFA, 0xC6, 0xB7, 0xBC, 0xD3, 0xB9, 0xA4}, 10, STOCK_INDU_AGRICULTURAL_PRODUCT, "Agricultural product"},
    {{0xC5, 0xA9, 0xD2, 0xB5, 0xB7, 0xFE, 0xCE, 0xF1}, 8, STOCK_INDU_AGRICULTURAL_SERVICE, "Agricultural service"},
    {{0xBB, 0xFA, 0xB3, 0xA1, 0xBA, 0xBD, 0xD4, 0xCB}, 8, STOCK_INDU_AIRPORT_TRANSPORT, "Airport transport"},
    {{0xD1, 0xF8, 0xD6, 0xB3, 0xD2, 0xB5}, 6, STOCK_INDU_AQUACULTURE, "Aquaculture"},
    {{0xCA, 0xD3, 0xCC, 0xFD, 0xC6, 0xF7, 0xB2, 0xC4}, 8, STOCK_INDU_AUDIO_VISUAL_EQUIPMENT, "Audio and visual equipment"},
    {{0xC6, 0xFB, 0xB3, 0xB5, 0xD5, 0xFB, 0xB3, 0xB5}, 8, STOCK_INDU_AUTO, "Auto"},
    {{0xC6, 0xFB, 0xB3, 0xB5, 0xC1, 0xE3, 0xB2, 0xBF, 0xBC, 0xFE}, 10, STOCK_INDU_AUTO_ACCESSORY, "Auto accessory"},
    {{0xD2, 0xF8, 0xD0, 0xD0}, 4, STOCK_INDU_BANK, "Bank"},
    {{0xBB, 0xF9, 0xB4, 0xA1, 0xBB, 0xAF, 0xD1, 0xA7}, 8, STOCK_INDU_BASIC_CHEMISTRY, "Basic chemistry"},
    {{0xD2, 0xFB, 0xC1, 0xCF, 0xD6, 0xC6, 0xD4, 0xEC}, 8, STOCK_INDU_BEVERAGE, "Beverage"},
    {{0xC9, 0xFA, 0xCE, 0xEF, 0xD6, 0xC6, 0xC6, 0xB7}, 8, STOCK_INDU_BIOLOGY_PRODUCT, "Biology product"},
    {{0xBD, 0xA8, 0xD6, 0xFE, 0xD7, 0xB0, 0xCA, 0xCE}, 8, STOCK_INDU_BUILDING_DECORATION, "Building decoration"},
    {{0xBD, 0xA8, 0xD6, 0xFE, 0xB2, 0xC4, 0xC1, 0xCF}, 8, STOCK_INDU_BUILDING_MATERIAL, "Building material"},
    {{0xB9, 0xAB, 0xBD, 0xBB}, 4, STOCK_INDU_BUS, "Bus"},
    {{0xBB, 0xAF, 0xB9, 0xA4, 0xD0, 0xC2, 0xB2, 0xC4, 0xC1, 0xCF}, 10, STOCK_INDU_CHEMISTRY_NEW_MATERIAL, "Chemistry new material"},
    {{0xBB, 0xAF, 0xD1, 0xA7, 0xD6, 0xC6, 0xD2, 0xA9}, 8, STOCK_INDU_CHEMISTRY_PHARMACY, "Chemistry pharmacy"},
    {{0xBB, 0xAF, 0xD1, 0xA7, 0xD6, 0xC6, 0xC6, 0xB7}, 8, STOCK_INDU_CHEMISTRY_PRODUCT, "Chemistry product"},
    {{0xBB, 0xAF, 0xB9, 0xA4, 0xBA, 0xCF, 0xB3, 0xC9, 0xB2, 0xC4, 0xC1, 0xCF}, 12, STOCK_INDU_CHEMISTRY_SYNTHETIC, "Chemistry synthetic"},
    {{0xD6, 0xD0, 0xD2, 0xA9}, 4, STOCK_INDU_CHINESE_MEDICINE, "Chinese medicine"},
    {{0xB7, 0xFE, 0xD7, 0xB0, 0xBC, 0xD2, 0xB7, 0xC4}, 8, STOCK_INDU_CLOTHING, "Clothing"},
    {{0xC3, 0xBA, 0xCC, 0xBF, 0xBF, 0xAA, 0xB2, 0xC9, 0xBC, 0xD3, 0xB9, 0xA4}, 12, STOCK_INDU_COAL, "Coal"},
    {{0xD7, 0xDB, 0xBA, 0xCF}, 4, STOCK_INDU_COLLIGATE, "Colligate"},
    {{0xC3, 0xB3, 0xD2, 0xD7}, 4, STOCK_INDU_COMMERCE, "Commerce"},
    {{0xBC, 0xC6, 0xCB, 0xE3, 0xBB, 0xFA, 0xD3, 0xA6, 0xD3, 0xC3}, 10, STOCK_INDU_COMPUTER_APPLICATION, "Computer application"},
    {{0xBC, 0xC6, 0xCB, 0xE3, 0xBB, 0xFA, 0xC9, 0xE8, 0xB1, 0xB8}, 10, STOCK_INDU_COMPUTER_EQUIPMENT, "Computer equipment"},
    {{0xB5, 0xE7, 0xC6, 0xF8, 0xC9, 0xE8, 0xB1, 0xB8}, 8, STOCK_INDU_ELECTRIC_EQUIPMENT, "Electric equipment"},
    {{0xB5, 0xE7, 0xC1, 0xA6}, 4, STOCK_INDU_ELECTRIC_POWER, "Electric power"},
    {{0xB5, 0xE7, 0xD7, 0xD3, 0xD6, 0xC6, 0xD4, 0xEC}, 8, STOCK_INDU_ELECTRONIC_MANUFACTURE, "Electronic manufacture"},
    {{0xC6, 0xE4, 0xCB, 0xFB, 0xB5, 0xE7, 0xD7, 0xD3}, 8, STOCK_INDU_ELECTRONIC_OTHER, "Electronic other"},
    {{0xBB, 0xB7, 0xB1, 0xA3, 0xB9, 0xA4, 0xB3, 0xCC}, 8, STOCK_INDU_ENVIRONMENTAL_ENGINEERING, "Environmental engineering"},
    {{0xCA, 0xB3, 0xC6, 0xB7, 0xBC, 0xD3, 0xB9, 0xA4, 0xD6, 0xC6, 0xD4, 0xEC}, 12, STOCK_INDU_FOOD_MANUFACTURE, "Food manufacture"},
    {{0xD6, 0xD6, 0xD6, 0xB2, 0xD2, 0xB5, 0xD3, 0xEB, 0xC1, 0xD6, 0xD2, 0xB5}, 12, STOCK_INDU_FORESTRY, "Forestry"},
    {{0xC8, 0xBC, 0xC6, 0xF8, 0xCB, 0xAE, 0xCE, 0xF1}, 8, STOCK_INDU_GAS_WATER, "Gas and water"},
    {{0xCD, 0xA8, 0xD3, 0xC3, 0xC9, 0xE8, 0xB1, 0xB8}, 8, STOCK_INDU_GENERAL_EQUIPMENT, "General equipment"},
    {{0xBC, 0xD2, 0xD3, 0xC3, 0xC7, 0xE1, 0xB9, 0xA4}, 8, STOCK_INDU_HOUSEHOLD_LIGHT_INDUSTRY, "Household light industry"},
    {{0xBE, 0xC6, 0xB5, 0xEA, 0xBC, 0xB0, 0xB2, 0xCD, 0xD2, 0xFB}, 10, STOCK_INDU_HOTEL_DINING, "Hotel and dining"},
    {{0xD2, 0xC7, 0xC6, 0xF7, 0xD2, 0xC7, 0xB1, 0xED}, 8, STOCK_INDU_INSTRUMENT_METER, "Instrument and meter"},
    {{0xB1, 0xA3, 0xCF, 0xD5, 0xBC, 0xB0, 0xC6, 0xE4, 0xCB, 0xFB}, 10, STOCK_INDU_INSURANCE, "Insurance"},
    {{0xCE, 0xEF, 0xC1, 0xF7}, 4, STOCK_INDU_LOGISTICS, "Logistics"},
    {{0xB4, 0xAB, 0xC3, 0xBD}, 4, STOCK_INDU_MEDIA, "Media"},
    {{0xD2, 0xBD, 0xC1, 0xC6, 0xC6, 0xF7, 0xD0, 0xB5, 0xB7, 0xFE, 0xCE, 0xF1}, 12, STOCK_INDU_MEDICAL_EQUIPMENT_SERVICE, "Medical equipment service"},
    {{0xD2, 0xBD, 0xD2, 0xA9, 0xC9, 0xCC, 0xD2, 0xB5}, 8, STOCK_INDU_MEDICINE_COMMERCE, "Medicine commerce"},
    {{0xD3, 0xD0, 0xC9, 0xAB, 0xD2, 0xB1, 0xC1, 0xB6, 0xBC, 0xD3, 0xB9, 0xA4}, 12, STOCK_INDU_METAL_PROCESSING, "Metal processing"},
    {{0xB2, 0xC9, 0xBE, 0xF2, 0xB7, 0xFE, 0xCE, 0xF1}, 8, STOCK_INDU_MINING_SERVICE, "Mining service"},
    {{0xB9, 0xFA, 0xB7, 0xC0, 0xBE, 0xFC, 0xB9, 0xA4}, 8, STOCK_INDU_NAT_DEF_MIL_IND, "National defense and military industry"},
    {{0xD0, 0xC2, 0xB2, 0xC4, 0xC1, 0xCF}, 6, STOCK_INDU_NEW_MATERIAL, "New material"},
    {{0xB7, 0xC7, 0xC6, 0xFB, 0xB3, 0xB5, 0xBD, 0xBB, 0xD4, 0xCB}, 10, STOCK_INDU_NOT_AUTO_TRANSPORT, "Not auto transport"},
    {{0xCA, 0xAF, 0xD3, 0xCD, 0xBF, 0xF3, 0xD2, 0xB5, 0xBF, 0xAA, 0xB2, 0xC9}, 12, STOCK_INDU_OIL_EXPLORATION, "Oil exploration"},
    {{0xB0, 0xFC, 0xD7, 0xB0, 0xD3, 0xA1, 0xCB, 0xA2}, 8, STOCK_INDU_PACKAGE_PRINTING, "Package printing"},
    {{0xD4, 0xEC, 0xD6, 0xBD}, 4, STOCK_INDU_PAPER_MAKING, "Paper making"},
    {{0xD4, 0xB0, 0xC7, 0xF8, 0xBF, 0xAA, 0xB7, 0xA2}, 8, STOCK_INDU_PARK_DEVELOP, "Park develop"},
    {{0xB9, 0xE2, 0xD1, 0xA7, 0xB9, 0xE2, 0xB5, 0xE7, 0xD7, 0xD3}, 10, STOCK_INDU_PHOTOELECTRON, "Photoelectron"},
    {{0xB8, 0xDB, 0xBF, 0xDA, 0xBA, 0xBD, 0xD4, 0xCB}, 8, STOCK_INDU_PORT_TRANSPORT, "Port transport"},
    {{0xB7, 0xBF, 0xB5, 0xD8, 0xB2, 0xFA, 0xBF, 0xAA, 0xB7, 0xA2}, 10, STOCK_INDU_REAL_ESTATE, "Real estate"},
    {{0xC1, 0xE3, 0xCA, 0xDB}, 4, STOCK_INDU_RETAIL, "Retail"},
    {{0xB9, 0xAB, 0xC2, 0xB7, 0xCC, 0xFA, 0xC2, 0xB7, 0xD4, 0xCB, 0xCA, 0xE4}, 12, STOCK_INDU_ROAD_RAILWAY_TRANSPORT, "Road and railway transport"},
    {{0xB0, 0xEB, 0xB5, 0xBC, 0xCC, 0xE5, 0xBC, 0xB0, 0xD4, 0xAA, 0xBC, 0xFE}, 12, STOCK_INDU_SEMICONDUCTOR, "Semiconductor"},
    {{0xD7, 0xA8, 0xD3, 0xC3, 0xC9, 0xE8, 0xB1, 0xB8}, 8, STOCK_INDU_SPECIAL_EQUIPMENT, "Special equipment"},
    {{0xB8, 0xD6, 0xCC, 0xFA}, 4, STOCK_INDU_STEEL, "Steel"},
    {{0xD6, 0xA4, 0xC8, 0xAF}, 4, STOCK_INDU_STOCK, "Stock"},
    {{0xCD, 0xA8, 0xD0, 0xC5, 0xC9, 0xE8, 0xB1, 0xB8}, 8, STOCK_INDU_TELE_EQUIPMENT, "Telecommunication equipment"},
    {{0xCD, 0xA8, 0xD0, 0xC5, 0xB7, 0xFE, 0xCE, 0xF1}, 8, STOCK_INDU_TELE_SERVICE, "Telecommunication service"},
    {{0xB7, 0xC4, 0xD6, 0xAF, 0xD6, 0xC6, 0xD4, 0xEC}, 8, STOCK_INDU_TEXTILE_MANUFACTURE, "Textile manufacture"},
    {{0xBE, 0xB0, 0xB5, 0xE3, 0xBC, 0xB0, 0xC2, 0xC3, 0xD3, 0xCE}, 10, STOCK_INDU_TOUR, "Tour"},
    {{0xBD, 0xBB, 0xD4, 0xCB, 0xC9, 0xE8, 0xB1, 0xB8, 0xB7, 0xFE, 0xCE, 0xF1}, 12, STOCK_INDU_TRANSPORT_EQUIPMENT_SERVICE, "Transport equipment service"},
    {{0xB0, 0xD7, 0xC9, 0xAB, 0xBC, 0xD2, 0xB5, 0xE7}, 8, STOCK_INDU_WHITE_GOODS, "White goods"},
};

static u32 ls_u32NumberOfInduInfo = sizeof(ls_siiInduInfo) / sizeof(stock_indu_info_t);

static stock_info_t ls_siStockInfoIndex[] =
{
    {SH_COMPOSITE_INDEX},
    {SZ_COMPOSITIONAL_INDEX},
    {SME_COMPOSITIONAL_INDEX}
};

static olint_t ls_nNumOfStockInfoIndex =
    sizeof(ls_siStockInfoIndex) / sizeof(stock_info_t);

/* --- private routine section---------------------------------------------- */
static u32 _findStock(stock_list_t * psl, olchar_t * name, stock_info_t ** info)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    olint_t begin, end, index, ret;
    olint_t i;

    begin = 0;
    end = psl->sl_nNumOfStock - 1;
    
    /*binary search*/
    while (begin <= end)
    {
        index = (begin + end) / 2;
        ret = strncmp(
            psl->sl_psiStock[index].si_strCode, name,
            ol_strlen(psl->sl_psiStock[index].si_strCode));
        if (ret == 0)
        {
            *info = &psl->sl_psiStock[index];
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

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        for (i = 0; i < ls_nNumOfStockInfoIndex; i ++)
        {
            if (strncmp(name, ls_siStockInfoIndex[i].si_strCode,
                        ol_strlen(ls_siStockInfoIndex[i].si_strCode)) == 0)
            {
                *info = &ls_siStockInfoIndex[i];
                u32Ret = JF_ERR_NO_ERROR;
                break;
            }
        }
    }

    return u32Ret;
}

static olchar_t * _getStringIndustry(olint_t nIndustry)
{
    if ((nIndustry > 0) && (nIndustry <= ls_u32NumberOfInduInfo))
        return ls_siiInduInfo[nIndustry - 1].sii_pstrDesc;

    return (olchar_t *)jf_string_getStringNotApplicable();
}

static u32 _getStockIndustry(
    jf_string_parse_result_field_t * field, stock_info_t * stockinfo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stock_indu_info_t * desc;
    olint_t i, j;

    if (field->jsprf_sData < 4)
        return JF_ERR_INVALID_DATA;

    for (i = 0; i < ls_u32NumberOfInduInfo; i ++)
    {
        desc = &ls_siiInduInfo[i];
        if (desc->sii_nChineseDesc == field->jsprf_sData)
        {
            for (j = 0; j < desc->sii_nChineseDesc; j ++)
            {
                if (desc->sii_strChinese[j] != field->jsprf_pstrData[j])
                    break;
            }

            if (j == desc->sii_nChineseDesc)
            {
                stockinfo->si_nIndustry = desc->sii_nId;
                desc->sii_nStock ++;
            }
        }

    }

    if (stockinfo->si_nIndustry == 0)
    {
        u32Ret = JF_ERR_INVALID_DATA;
        jf_logger_logErrMsg(
            u32Ret, "Unknown industry for stock %s", stockinfo->si_strCode);
    }

    return u32Ret;
}

static u32 _fillStockInfo(
    olchar_t * line, olsize_t sline, stock_info_t * stockinfo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * result = NULL;
    jf_string_parse_result_field_t * field;

    memset(stockinfo, 0, sizeof(stock_info_t));

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
        ol_strncpy(stockinfo->si_strCode, field->jsprf_pstrData, 8);
        jf_string_lower(stockinfo->si_strCode);

        field = field->jsprf_pjsprfNext;
        u32Ret = _getStockIndustry(field, stockinfo);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = field->jsprf_pjsprfNext;
        if (field->jsprf_pstrData[0] != '-')
            u32Ret = jf_string_getU64FromString(
                field->jsprf_pstrData, field->jsprf_sData,
                &stockinfo->si_u64GeneralCapital);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = field->jsprf_pjsprfNext;
        if (field->jsprf_pstrData[0] != '-')
            u32Ret = jf_string_getU64FromString(
                field->jsprf_pstrData, field->jsprf_sData - 1, /*remove the '\n'*/
                &stockinfo->si_u64TradableShare);
    }

    if (result != NULL)
        jf_string_destroyParseResult(&result);

    return u32Ret;
}

static u32 _readStockList(olchar_t * pstrStockListFile, stock_list_t * stocklist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_t fd;
    olchar_t line[512];
    olsize_t sline;
    stock_info_t * stockinfo;
    olint_t lineno = 1;

    stockinfo = stocklist->sl_psiStock;

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
                stocklist->sl_nNumOfStock ++;
                if (stocklist->sl_nNumOfStock >= stocklist->sl_nMaxStock)
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
		jf_logger_logErrMsg(
			u32Ret, "Found ERROR at line %d, %s", lineno, jf_err_getDescription(u32Ret));
    }

    return u32Ret;
}

static u32 _classifyStock(stock_list_t * psl)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stock_indu_info_t * info;
    olint_t i;
    olsize_t sbuf;
    stock_info_t * stockinfo;

    for (i = 0;
         (i < ls_u32NumberOfInduInfo) && (u32Ret == JF_ERR_NO_ERROR);
         i ++)
    {
        info = &ls_siiInduInfo[i];
        sbuf = info->sii_nStock * 9 + 1;

        u32Ret = jf_mem_alloc((void **)&info->sii_pstrStocks, sbuf);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            info->sii_pstrStocks[0] = '\0';
        }
    }

    stockinfo = psl->sl_psiStock;
    for (i = 0; i < psl->sl_nNumOfStock; i ++)
    {
        info = &ls_siiInduInfo[stockinfo->si_nIndustry - 1];
        ol_strcat(info->sii_pstrStocks, stockinfo->si_strCode);
        ol_strcat(info->sii_pstrStocks, " ");

        stockinfo ++;
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 initStockList(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t size;
    stock_list_t * psl = &ls_slStockList;

    memset(psl, 0, sizeof(stock_list_t));

    psl->sl_nMaxStock = MAX_STOCKS;
    size = sizeof(stock_info_t) * psl->sl_nMaxStock;
    u32Ret = jf_mem_alloc((void **)&psl->sl_psiStock, size);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _readStockList(STOCK_LIST_FILE, psl);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _classifyStock(psl);

    if (u32Ret != JF_ERR_NO_ERROR)
	{
		jf_logger_logErrMsg(
			u32Ret, "Failed to initiate stock list. %s",
            jf_err_getDescription(u32Ret));
        finiStockList();
	}

    return u32Ret;
}

u32 finiStockList(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stock_list_t * psl = &ls_slStockList;
    stock_indu_info_t * info;
    olint_t i;

    if (psl->sl_psiStock != NULL)
    {
        jf_mem_free((void **)&psl->sl_psiStock);
    }

    for (i = 0; i < ls_u32NumberOfInduInfo; i ++)
    {
        info = &ls_siiInduInfo[i];

        if (info->sii_pstrStocks != NULL)
            jf_mem_free((void **)&info->sii_pstrStocks);
    }

    return u32Ret;
}

u32 getIndustryInfo(olint_t id, stock_indu_info_t ** info)
{
    if ((id > 0) && (id <= ls_u32NumberOfInduInfo))
    {
        *info = &ls_siiInduInfo[id - 1];
        return JF_ERR_NO_ERROR;
    }

    return JF_ERR_NOT_FOUND;
}

stock_indu_info_t * getFirstIndustryInfo(void)
{
    return &ls_siiInduInfo[0];
}

stock_indu_info_t * getNextIndustryInfo(stock_indu_info_t * info)
{
    stock_indu_info_t * psi = NULL;

    if (info < ls_siiInduInfo + ls_u32NumberOfInduInfo - 1)
        psi = info + 1;

    return psi;
}

olint_t getNumOfIndustry(void)
{
    return ls_u32NumberOfInduInfo;
}

stock_info_t * getFirstStockInfo(void)
{
    return ls_slStockList.sl_psiStock;
}

stock_info_t * getNextStockInfo(stock_info_t * info)
{
    stock_info_t * psi = NULL;
    stock_list_t * psl = &ls_slStockList;

    if (info < psl->sl_psiStock + psl->sl_nNumOfStock - 1)
        psi = info + 1;

    return psi;
}

olint_t getNumOfStock(void)
{
    stock_list_t * psl = &ls_slStockList;

    return psl->sl_nNumOfStock;
}

stock_info_t * getFirstStockInfoIndex(void)
{
    return &ls_siStockInfoIndex[0];
}

stock_info_t * getNextStockInfoIndex(stock_info_t * info)
{
    stock_info_t * psi = NULL;

    if (info < &ls_siStockInfoIndex[0] + ls_nNumOfStockInfoIndex - 1)
        psi = info + 1;

    return psi;
}

stock_info_t * getStockInfoIndex(olchar_t * name)
{
    olint_t i;

    for (i = 0; i < ls_nNumOfStockInfoIndex; i ++)
    {
        if (ol_strncmp(ls_siStockInfoIndex[i].si_strCode, name, 8) == 0)
            return &ls_siStockInfoIndex[i];
    }

    return NULL;
}

boolean_t isStockInfoIndex(olchar_t * code)
{
    boolean_t bRet = FALSE;
    stock_info_t * psi = &ls_siStockInfoIndex[0];
    olint_t i;

    for (i = 0; i < ls_nNumOfStockInfoIndex; i ++)
    {
        if (ol_strncmp(code, psi->si_strCode, ol_strlen(psi->si_strCode)) == 0)
            return TRUE;

        psi ++;
    }

    return bRet;
}

u32 getStockInfo(olchar_t * name, stock_info_t ** info)
{
    *info = NULL;
    if (name == NULL)
        return JF_ERR_INVALID_PARAM;

    return _findStock(&ls_slStockList, name, info);
}

olint_t getStockShareThres(stock_info_t * stockinfo)
{
    olint_t ret;

    if (stockinfo->si_u64TradableShare > SMALL_MEDIUM_STOCK_SHARE)
        ret = 800;
    else if (stockinfo->si_u64TradableShare > GROWTH_STOCK_SHARE)
        ret = 400;
    else if (stockinfo->si_u64TradableShare > TINY_STOCK_SHARE)
        ret = 200;
    else
        ret = 100;

    return ret;
}

boolean_t isSmallMediumStock(stock_info_t * stockinfo)
{
    boolean_t bRet = TRUE;

    if (stockinfo->si_u64TradableShare > SMALL_MEDIUM_STOCK_SHARE)
        bRet = FALSE;

    return bRet;
}

boolean_t isShStockExchange(olchar_t * stock)
{
   if (ol_strncmp(stock, "sh", 2) == 0)
       return TRUE;

   return FALSE;
}

olchar_t * getStringIndustry(olint_t nIndustry)
{
    return _getStringIndustry(nIndustry);
}

/*---------------------------------------------------------------------------*/


