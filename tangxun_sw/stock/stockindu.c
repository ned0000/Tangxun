/**
 *  @file stockindu.c
 *
 *  @brief Implementation file for stock industry.
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

#include "common.h"
#include "stockindu.h"

/* --- private data/data structure section ------------------------------------------------------ */

static tx_stock_indu_info_t ls_tsiiInduInfo[] =
{
    {{0xC5, 0xA9, 0xB2, 0xFA, 0xC6, 0xB7, 0xBC, 0xD3, 0xB9, 0xA4}, 10, TX_STOCK_INDU_AGRICULTURAL_PRODUCT, "Agricultural product"},
    {{0xC5, 0xA9, 0xD2, 0xB5, 0xB7, 0xFE, 0xCE, 0xF1}, 8, TX_STOCK_INDU_AGRICULTURAL_SERVICE, "Agricultural service"},
    {{0xBB, 0xFA, 0xB3, 0xA1, 0xBA, 0xBD, 0xD4, 0xCB}, 8, TX_STOCK_INDU_AIRPORT_TRANSPORT, "Airport transport"},
    {{0xD1, 0xF8, 0xD6, 0xB3, 0xD2, 0xB5}, 6, TX_STOCK_INDU_AQUACULTURE, "Aquaculture"},
    {{0xCA, 0xD3, 0xCC, 0xFD, 0xC6, 0xF7, 0xB2, 0xC4}, 8, TX_STOCK_INDU_AUDIO_VISUAL_EQUIPMENT, "Audio and visual equipment"},
    {{0xC6, 0xFB, 0xB3, 0xB5, 0xD5, 0xFB, 0xB3, 0xB5}, 8, TX_STOCK_INDU_AUTO, "Auto"},
    {{0xC6, 0xFB, 0xB3, 0xB5, 0xC1, 0xE3, 0xB2, 0xBF, 0xBC, 0xFE}, 10, TX_STOCK_INDU_AUTO_ACCESSORY, "Auto accessory"},
    {{0xD2, 0xF8, 0xD0, 0xD0}, 4, TX_STOCK_INDU_BANK, "Bank"},
    {{0xBB, 0xF9, 0xB4, 0xA1, 0xBB, 0xAF, 0xD1, 0xA7}, 8, TX_STOCK_INDU_BASIC_CHEMISTRY, "Basic chemistry"},
    {{0xD2, 0xFB, 0xC1, 0xCF, 0xD6, 0xC6, 0xD4, 0xEC}, 8, TX_STOCK_INDU_BEVERAGE, "Beverage"},
    {{0xC9, 0xFA, 0xCE, 0xEF, 0xD6, 0xC6, 0xC6, 0xB7}, 8, TX_STOCK_INDU_BIOLOGY_PRODUCT, "Biology product"},
    {{0xBD, 0xA8, 0xD6, 0xFE, 0xD7, 0xB0, 0xCA, 0xCE}, 8, TX_STOCK_INDU_BUILDING_DECORATION, "Building decoration"},
    {{0xBD, 0xA8, 0xD6, 0xFE, 0xB2, 0xC4, 0xC1, 0xCF}, 8, TX_STOCK_INDU_BUILDING_MATERIAL, "Building material"},
    {{0xB9, 0xAB, 0xBD, 0xBB}, 4, TX_STOCK_INDU_BUS, "Bus"},
    {{0xBB, 0xAF, 0xB9, 0xA4, 0xD0, 0xC2, 0xB2, 0xC4, 0xC1, 0xCF}, 10, TX_STOCK_INDU_CHEMISTRY_NEW_MATERIAL, "Chemistry new material"},
    {{0xBB, 0xAF, 0xD1, 0xA7, 0xD6, 0xC6, 0xD2, 0xA9}, 8, TX_STOCK_INDU_CHEMISTRY_PHARMACY, "Chemistry pharmacy"},
    {{0xBB, 0xAF, 0xD1, 0xA7, 0xD6, 0xC6, 0xC6, 0xB7}, 8, TX_STOCK_INDU_CHEMISTRY_PRODUCT, "Chemistry product"},
    {{0xBB, 0xAF, 0xB9, 0xA4, 0xBA, 0xCF, 0xB3, 0xC9, 0xB2, 0xC4, 0xC1, 0xCF}, 12, TX_STOCK_INDU_CHEMISTRY_SYNTHETIC, "Chemistry synthetic"},
    {{0xD6, 0xD0, 0xD2, 0xA9}, 4, TX_STOCK_INDU_CHINESE_MEDICINE, "Chinese medicine"},
    {{0xB7, 0xFE, 0xD7, 0xB0, 0xBC, 0xD2, 0xB7, 0xC4}, 8, TX_STOCK_INDU_CLOTHING, "Clothing"},
    {{0xC3, 0xBA, 0xCC, 0xBF, 0xBF, 0xAA, 0xB2, 0xC9, 0xBC, 0xD3, 0xB9, 0xA4}, 12, TX_STOCK_INDU_COAL, "Coal"},
    {{0xD7, 0xDB, 0xBA, 0xCF}, 4, TX_STOCK_INDU_COLLIGATE, "Colligate"},
    {{0xC3, 0xB3, 0xD2, 0xD7}, 4, TX_STOCK_INDU_COMMERCE, "Commerce"},
    {{0xBC, 0xC6, 0xCB, 0xE3, 0xBB, 0xFA, 0xD3, 0xA6, 0xD3, 0xC3}, 10, TX_STOCK_INDU_COMPUTER_APPLICATION, "Computer application"},
    {{0xBC, 0xC6, 0xCB, 0xE3, 0xBB, 0xFA, 0xC9, 0xE8, 0xB1, 0xB8}, 10, TX_STOCK_INDU_COMPUTER_EQUIPMENT, "Computer equipment"},
    {{0xB5, 0xE7, 0xC6, 0xF8, 0xC9, 0xE8, 0xB1, 0xB8}, 8, TX_STOCK_INDU_ELECTRIC_EQUIPMENT, "Electric equipment"},
    {{0xB5, 0xE7, 0xC1, 0xA6}, 4, TX_STOCK_INDU_ELECTRIC_POWER, "Electric power"},
    {{0xB5, 0xE7, 0xD7, 0xD3, 0xD6, 0xC6, 0xD4, 0xEC}, 8, TX_STOCK_INDU_ELECTRONIC_MANUFACTURE, "Electronic manufacture"},
    {{0xC6, 0xE4, 0xCB, 0xFB, 0xB5, 0xE7, 0xD7, 0xD3}, 8, TX_STOCK_INDU_ELECTRONIC_OTHER, "Electronic other"},
    {{0xBB, 0xB7, 0xB1, 0xA3, 0xB9, 0xA4, 0xB3, 0xCC}, 8, TX_STOCK_INDU_ENVIRONMENTAL_ENGINEERING, "Environmental engineering"},
    {{0xCA, 0xB3, 0xC6, 0xB7, 0xBC, 0xD3, 0xB9, 0xA4, 0xD6, 0xC6, 0xD4, 0xEC}, 12, TX_STOCK_INDU_FOOD_MANUFACTURE, "Food manufacture"},
    {{0xD6, 0xD6, 0xD6, 0xB2, 0xD2, 0xB5, 0xD3, 0xEB, 0xC1, 0xD6, 0xD2, 0xB5}, 12, TX_STOCK_INDU_FORESTRY, "Forestry"},
    {{0xC8, 0xBC, 0xC6, 0xF8, 0xCB, 0xAE, 0xCE, 0xF1}, 8, TX_STOCK_INDU_GAS_WATER, "Gas and water"},
    {{0xCD, 0xA8, 0xD3, 0xC3, 0xC9, 0xE8, 0xB1, 0xB8}, 8, TX_STOCK_INDU_GENERAL_EQUIPMENT, "General equipment"},
    {{0xBC, 0xD2, 0xD3, 0xC3, 0xC7, 0xE1, 0xB9, 0xA4}, 8, TX_STOCK_INDU_HOUSEHOLD_LIGHT_INDUSTRY, "Household light industry"},
    {{0xBE, 0xC6, 0xB5, 0xEA, 0xBC, 0xB0, 0xB2, 0xCD, 0xD2, 0xFB}, 10, TX_STOCK_INDU_HOTEL_DINING, "Hotel and dining"},
    {{0xD2, 0xC7, 0xC6, 0xF7, 0xD2, 0xC7, 0xB1, 0xED}, 8, TX_STOCK_INDU_INSTRUMENT_METER, "Instrument and meter"},
    {{0xB1, 0xA3, 0xCF, 0xD5, 0xBC, 0xB0, 0xC6, 0xE4, 0xCB, 0xFB}, 10, TX_STOCK_INDU_INSURANCE, "Insurance"},
    {{0xCE, 0xEF, 0xC1, 0xF7}, 4, TX_STOCK_INDU_LOGISTICS, "Logistics"},
    {{0xB4, 0xAB, 0xC3, 0xBD}, 4, TX_STOCK_INDU_MEDIA, "Media"},
    {{0xD2, 0xBD, 0xC1, 0xC6, 0xC6, 0xF7, 0xD0, 0xB5, 0xB7, 0xFE, 0xCE, 0xF1}, 12, TX_STOCK_INDU_MEDICAL_EQUIPMENT_SERVICE, "Medical equipment service"},
    {{0xD2, 0xBD, 0xD2, 0xA9, 0xC9, 0xCC, 0xD2, 0xB5}, 8, TX_STOCK_INDU_MEDICINE_COMMERCE, "Medicine commerce"},
    {{0xD3, 0xD0, 0xC9, 0xAB, 0xD2, 0xB1, 0xC1, 0xB6, 0xBC, 0xD3, 0xB9, 0xA4}, 12, TX_STOCK_INDU_METAL_PROCESSING, "Metal processing"},
    {{0xB2, 0xC9, 0xBE, 0xF2, 0xB7, 0xFE, 0xCE, 0xF1}, 8, TX_STOCK_INDU_MINING_SERVICE, "Mining service"},
    {{0xB9, 0xFA, 0xB7, 0xC0, 0xBE, 0xFC, 0xB9, 0xA4}, 8, TX_STOCK_INDU_NAT_DEF_MIL_IND, "National defense and military industry"},
    {{0xD0, 0xC2, 0xB2, 0xC4, 0xC1, 0xCF}, 6, TX_STOCK_INDU_NEW_MATERIAL, "New material"},
    {{0xB7, 0xC7, 0xC6, 0xFB, 0xB3, 0xB5, 0xBD, 0xBB, 0xD4, 0xCB}, 10, TX_STOCK_INDU_NOT_AUTO_TRANSPORT, "Not auto transport"},
    {{0xCA, 0xAF, 0xD3, 0xCD, 0xBF, 0xF3, 0xD2, 0xB5, 0xBF, 0xAA, 0xB2, 0xC9}, 12, TX_STOCK_INDU_OIL_EXPLORATION, "Oil exploration"},
    {{0xB0, 0xFC, 0xD7, 0xB0, 0xD3, 0xA1, 0xCB, 0xA2}, 8, TX_STOCK_INDU_PACKAGE_PRINTING, "Package printing"},
    {{0xD4, 0xEC, 0xD6, 0xBD}, 4, TX_STOCK_INDU_PAPER_MAKING, "Paper making"},
    {{0xD4, 0xB0, 0xC7, 0xF8, 0xBF, 0xAA, 0xB7, 0xA2}, 8, TX_STOCK_INDU_PARK_DEVELOP, "Park develop"},
    {{0xB9, 0xE2, 0xD1, 0xA7, 0xB9, 0xE2, 0xB5, 0xE7, 0xD7, 0xD3}, 10, TX_STOCK_INDU_PHOTOELECTRON, "Photoelectron"},
    {{0xB8, 0xDB, 0xBF, 0xDA, 0xBA, 0xBD, 0xD4, 0xCB}, 8, TX_STOCK_INDU_PORT_TRANSPORT, "Port transport"},
    {{0xB7, 0xBF, 0xB5, 0xD8, 0xB2, 0xFA, 0xBF, 0xAA, 0xB7, 0xA2}, 10, TX_STOCK_INDU_REAL_ESTATE, "Real estate"},
    {{0xC1, 0xE3, 0xCA, 0xDB}, 4, TX_STOCK_INDU_RETAIL, "Retail"},
    {{0xB9, 0xAB, 0xC2, 0xB7, 0xCC, 0xFA, 0xC2, 0xB7, 0xD4, 0xCB, 0xCA, 0xE4}, 12, TX_STOCK_INDU_ROAD_RAILWAY_TRANSPORT, "Road and railway transport"},
    {{0xB0, 0xEB, 0xB5, 0xBC, 0xCC, 0xE5, 0xBC, 0xB0, 0xD4, 0xAA, 0xBC, 0xFE}, 12, TX_STOCK_INDU_SEMICONDUCTOR, "Semiconductor"},
    {{0xD7, 0xA8, 0xD3, 0xC3, 0xC9, 0xE8, 0xB1, 0xB8}, 8, TX_STOCK_INDU_SPECIAL_EQUIPMENT, "Special equipment"},
    {{0xB8, 0xD6, 0xCC, 0xFA}, 4, TX_STOCK_INDU_STEEL, "Steel"},
    {{0xD6, 0xA4, 0xC8, 0xAF}, 4, TX_STOCK_INDU_STOCK, "Stock"},
    {{0xCD, 0xA8, 0xD0, 0xC5, 0xC9, 0xE8, 0xB1, 0xB8}, 8, TX_STOCK_INDU_TELE_EQUIPMENT, "Telecommunication equipment"},
    {{0xCD, 0xA8, 0xD0, 0xC5, 0xB7, 0xFE, 0xCE, 0xF1}, 8, TX_STOCK_INDU_TELE_SERVICE, "Telecommunication service"},
    {{0xB7, 0xC4, 0xD6, 0xAF, 0xD6, 0xC6, 0xD4, 0xEC}, 8, TX_STOCK_INDU_TEXTILE_MANUFACTURE, "Textile manufacture"},
    {{0xBE, 0xB0, 0xB5, 0xE3, 0xBC, 0xB0, 0xC2, 0xC3, 0xD3, 0xCE}, 10, TX_STOCK_INDU_TOUR, "Tour"},
    {{0xBD, 0xBB, 0xD4, 0xCB, 0xC9, 0xE8, 0xB1, 0xB8, 0xB7, 0xFE, 0xCE, 0xF1}, 12, TX_STOCK_INDU_TRANSPORT_EQUIPMENT_SERVICE, "Transport equipment service"},
    {{0xB0, 0xD7, 0xC9, 0xAB, 0xBC, 0xD2, 0xB5, 0xE7}, 8, TX_STOCK_INDU_WHITE_GOODS, "White goods"},
};

static u32 ls_u32NumberOfInduInfo = sizeof(ls_tsiiInduInfo) / sizeof(tx_stock_indu_info_t);


/* --- private routine section ------------------------------------------------------------------ */

static olchar_t * _getStringIndustry(olint_t nIndustry)
{
    if ((nIndustry > 0) && (nIndustry <= ls_u32NumberOfInduInfo))
        return ls_tsiiInduInfo[nIndustry - 1].tsii_pstrDesc;

    return (olchar_t *)jf_string_getStringNotApplicable();
}

/** Add stocks to stock indu data structure.
 */
static u32 _addStockToIndu(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_indu_info_t * info = NULL;
    olint_t index = 0;
    olsize_t sbuf = 0;
    tx_stock_info_t * stockinfo = NULL;

    for (index = 0; (index < ls_u32NumberOfInduInfo) && (u32Ret == JF_ERR_NO_ERROR); index ++)
    {
        info = &ls_tsiiInduInfo[index];
        sbuf = info->tsii_u32Stock * sizeof(tx_stock_info_t *);

        u32Ret = jf_jiukun_allocMemory((void **)&info->tsii_pptsiStocks, sbuf);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_bzero(info->tsii_pptsiStocks, sbuf);
            /*Clear the counter temporarily, the counter is increased in the next step.*/
            info->tsii_u32Stock = 0;
        }
    }

    stockinfo = tx_stock_getFirstStockInfo();
    while ((stockinfo != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        info = &ls_tsiiInduInfo[stockinfo->tsi_nIndustry - 1];
        /*Add the stock info into the pointer array.*/
        info->tsii_pptsiStocks[info->tsii_u32Stock] = stockinfo;
        info->tsii_u32Stock ++;

        stockinfo = tx_stock_getNextStockInfo(stockinfo);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initStockIndu(tx_stock_init_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("init stock indu");

    u32Ret = _addStockToIndu();

    if (u32Ret != JF_ERR_NO_ERROR)
	{
		JF_LOGGER_ERR(u32Ret, "Failed to initiate stock indu");
        finiStockIndu();
	}

    return u32Ret;
}

u32 finiStockIndu(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_indu_info_t * info = NULL;
    olint_t i = 0;

    JF_LOGGER_INFO("fini stock indu");

    for (i = 0; i < ls_u32NumberOfInduInfo; i ++)
    {
        info = &ls_tsiiInduInfo[i];

        if (info->tsii_pptsiStocks != NULL)
            jf_jiukun_freeMemory((void **)&info->tsii_pptsiStocks);
    }

    return u32Ret;
}

u32 tx_stock_getInduInfo(olint_t id, tx_stock_indu_info_t ** info)
{
    if ((id > 0) && (id <= ls_u32NumberOfInduInfo))
    {
        *info = &ls_tsiiInduInfo[id - 1];
        return JF_ERR_NO_ERROR;
    }

    return JF_ERR_NOT_FOUND;
}

tx_stock_indu_info_t * tx_stock_getFirstInduInfo(void)
{
    return &ls_tsiiInduInfo[0];
}

tx_stock_indu_info_t * tx_stock_getNextInduInfo(tx_stock_indu_info_t * info)
{
    tx_stock_indu_info_t * ptsi = NULL;

    if (info < ls_tsiiInduInfo + ls_u32NumberOfInduInfo - 1)
        ptsi = info + 1;

    return ptsi;
}

olint_t tx_stock_getNumOfIndu(void)
{
    return ls_u32NumberOfInduInfo;
}

olchar_t * tx_stock_getStringIndu(olint_t nIndustry)
{
    return _getStringIndustry(nIndustry);
}

u32 setStockIndustry(
    const olchar_t * pstrData, const olsize_t sData, tx_stock_info_t * stockinfo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_stock_indu_info_t * desc = NULL;
    olint_t i = 0, j = 0;

    if (sData < 4)
        return JF_ERR_INVALID_DATA;

    for (i = 0; i < ls_u32NumberOfInduInfo; i ++)
    {
        desc = &ls_tsiiInduInfo[i];
        if (desc->tsii_sChineseDesc == sData)
        {
            for (j = 0; j < desc->tsii_sChineseDesc; j ++)
            {
                if (desc->tsii_strChinese[j] != pstrData[j])
                    break;
            }

            if (j == desc->tsii_sChineseDesc)
            {
                stockinfo->tsi_nIndustry = desc->tsii_nId;
                /*Increase the counter to record number of stock.*/
                desc->tsii_u32Stock ++;
            }
        }

    }

    if (stockinfo->tsi_nIndustry == 0)
    {
        u32Ret = JF_ERR_INVALID_DATA;
        JF_LOGGER_ERR(u32Ret, "unknown industry for stock %s", stockinfo->tsi_strCode);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


