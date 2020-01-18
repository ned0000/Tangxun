/**
 *  @file stock.h
 *
 *  @brief list of stock
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_STOCK_H
#define TANGXUN_STOCK_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define TX_STOCK_SMALL_MEDIUM_STOCK_SHARE    (3000000000ULL)
#define TX_STOCK_GROWTH_STOCK_SHARE          (200000000ULL)
#define TX_STOCK_TINY_STOCK_SHARE            (30000000ULL)

enum tx_stock_indu
{
    TX_STOCK_INDU_AGRICULTURAL_PRODUCT = 1,
    TX_STOCK_INDU_AGRICULTURAL_SERVICE,
    TX_STOCK_INDU_AIRPORT_TRANSPORT,
    TX_STOCK_INDU_AQUACULTURE, /*yang zhi ye*/
    TX_STOCK_INDU_AUDIO_VISUAL_EQUIPMENT,
    TX_STOCK_INDU_AUTO,
    TX_STOCK_INDU_AUTO_ACCESSORY,

    TX_STOCK_INDU_BANK,
    TX_STOCK_INDU_BASIC_CHEMISTRY,
    TX_STOCK_INDU_BEVERAGE,
    TX_STOCK_INDU_BIOLOGY_PRODUCT,
    TX_STOCK_INDU_BUILDING_DECORATION,
    TX_STOCK_INDU_BUILDING_MATERIAL,
    TX_STOCK_INDU_BUS,

    TX_STOCK_INDU_CHEMISTRY_NEW_MATERIAL,
    TX_STOCK_INDU_CHEMISTRY_PHARMACY, /*zhi yao*/
    TX_STOCK_INDU_CHEMISTRY_PRODUCT,
    TX_STOCK_INDU_CHEMISTRY_SYNTHETIC, /*he cheng*/
    TX_STOCK_INDU_CHINESE_MEDICINE,
    TX_STOCK_INDU_CLOTHING,
    TX_STOCK_INDU_COAL, 
    TX_STOCK_INDU_COLLIGATE,  /* zong he*/
    TX_STOCK_INDU_COMMERCE,
    TX_STOCK_INDU_COMPUTER_APPLICATION,
    TX_STOCK_INDU_COMPUTER_EQUIPMENT,

    TX_STOCK_INDU_ELECTRIC_EQUIPMENT,
    TX_STOCK_INDU_ELECTRIC_POWER,
    TX_STOCK_INDU_ELECTRONIC_MANUFACTURE,
    TX_STOCK_INDU_ELECTRONIC_OTHER,
    TX_STOCK_INDU_ENVIRONMENTAL_ENGINEERING,

    TX_STOCK_INDU_FOOD_MANUFACTURE,
    TX_STOCK_INDU_FORESTRY,

    TX_STOCK_INDU_GAS_WATER,
    TX_STOCK_INDU_GENERAL_EQUIPMENT,

    TX_STOCK_INDU_HOUSEHOLD_LIGHT_INDUSTRY,
    TX_STOCK_INDU_HOTEL_DINING,

    TX_STOCK_INDU_INSTRUMENT_METER, /*yi qi yi biao*/
    TX_STOCK_INDU_INSURANCE,

    TX_STOCK_INDU_LOGISTICS,  /*wu liu*/

    TX_STOCK_INDU_MEDIA,
    TX_STOCK_INDU_MEDICAL_EQUIPMENT_SERVICE,
    TX_STOCK_INDU_MEDICINE_COMMERCE,
    TX_STOCK_INDU_METAL_PROCESSING,
    TX_STOCK_INDU_MINING_SERVICE,

    TX_STOCK_INDU_NAT_DEF_MIL_IND, /*guo fang jun gong*/
    TX_STOCK_INDU_NEW_MATERIAL,
    TX_STOCK_INDU_NOT_AUTO_TRANSPORT,

    TX_STOCK_INDU_OIL_EXPLORATION,

    TX_STOCK_INDU_PACKAGE_PRINTING,
    TX_STOCK_INDU_PAPER_MAKING,
    TX_STOCK_INDU_PARK_DEVELOP,
    TX_STOCK_INDU_PHOTOELECTRON,  /*guang dian zi*/
    TX_STOCK_INDU_PORT_TRANSPORT,

    TX_STOCK_INDU_REAL_ESTATE,
    TX_STOCK_INDU_RETAIL,
    TX_STOCK_INDU_ROAD_RAILWAY_TRANSPORT,

    TX_STOCK_INDU_SEMICONDUCTOR,
    TX_STOCK_INDU_SPECIAL_EQUIPMENT,
    TX_STOCK_INDU_STEEL,
    TX_STOCK_INDU_STOCK,

    TX_STOCK_INDU_TELE_EQUIPMENT,
    TX_STOCK_INDU_TELE_SERVICE,
    TX_STOCK_INDU_TEXTILE_MANUFACTURE,
    TX_STOCK_INDU_TOUR,
    TX_STOCK_INDU_TRANSPORT_EQUIPMENT_SERVICE,

    TX_STOCK_INDU_WHITE_GOODS,

    TX_STOCK_INDU_MAX,
};

/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    olchar_t tsi_strCode[16];
    u64 tsi_u64GeneralCapital;
    u64 tsi_u64TradableShare;
    olint_t tsi_nIndustry;
    olint_t tsi_nReserved;
    olchar_t tsi_strFirstTradeDate[16];

    /*runtime data*/
    boolean_t tsi_bCsvUptodate;
    u8 tsi_u8Reserved[7];
    oldouble_t tsi_dbValue;
} tx_stock_info_t;

typedef struct
{
    tx_stock_info_t * tsl_ptsiStock;
    olint_t tsl_nNumOfStock;
    olint_t tsl_nMaxStock;
    boolean_t tsl_bChanged;
    boolean_t tsl_bReserved[7];
} tx_stock_list_t;

typedef struct
{
    olchar_t tsii_strChinese[16];
    olint_t tsii_nChineseDesc;
    olint_t tsii_nId;
    olchar_t * tsii_pstrDesc;
    olint_t tsii_nStock;
    olchar_t * tsii_pstrStocks;
} tx_stock_indu_info_t;

#define TX_STOCK_SH_COMPOSITE_INDEX              "sh000001"
#define TX_STOCK_SZ_COMPOSITIONAL_INDEX          "sz399001"
#define TX_STOCK_SME_COMPOSITIONAL_INDEX         "sz399005"

#define TX_STOCK_QUOTATION_LIST_FILE_NAME  "StockQuotationList.txt"
#define TX_STOCK_STAT_ARBI_LIST_FILE_NAME  "StockStatArbiList.txt"
#define TX_STOCK_TOUGH_LIST_FILE_NAME      "StockToughList.txt"

/* --- functional routines ---------------------------------------------------------------------- */
u32 initStockList(void);
u32 finiStockList(void);

tx_stock_info_t * getFirstStockInfo(void);
tx_stock_info_t * getNextStockInfo(tx_stock_info_t * info);
olint_t getNumOfStock(void);

u32 getIndustryInfo(olint_t id, tx_stock_indu_info_t ** info);
tx_stock_indu_info_t * getFirstIndustryInfo(void);
tx_stock_indu_info_t * getNextIndustryInfo(tx_stock_indu_info_t * info);
olint_t getNumOfIndustry(void);
olchar_t * getStringIndustry(olint_t nIndustry);

void printStockInfoVerbose(tx_stock_info_t * info);
u32 getStockInfo(olchar_t * name, tx_stock_info_t ** info);
olint_t getStockShareThres(tx_stock_info_t * stockinfo);
boolean_t isSmallMediumStock(tx_stock_info_t * stockinfo);

/*Stock Index Information*/
tx_stock_info_t * getFirstStockInfoIndex(void);
tx_stock_info_t * getNextStockInfoIndex(tx_stock_info_t * info);
tx_stock_info_t * getStockInfoIndex(olchar_t * name);
boolean_t isStockInfoIndex(olchar_t * stock);

boolean_t isShStockExchange(olchar_t * stock);

#endif /*TANGXUN_STOCK_H*/

/*------------------------------------------------------------------------------------------------*/


