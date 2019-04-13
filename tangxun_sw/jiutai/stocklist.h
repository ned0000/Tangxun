/**
 *  @file stocklist.h
 *
 *  @brief list of stock
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_STOCKLIST_H
#define TANGXUN_JIUTAI_STOCKLIST_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"

/* --- constant definitions --------------------------------------------------------------------- */
#define SMALL_MEDIUM_STOCK_SHARE    (3000000000ULL)
#define GROWTH_STOCK_SHARE          (200000000ULL)
#define TINY_STOCK_SHARE            (30000000ULL)

enum stock_indu
{
    STOCK_INDU_AGRICULTURAL_PRODUCT = 1,
    STOCK_INDU_AGRICULTURAL_SERVICE,
    STOCK_INDU_AIRPORT_TRANSPORT,
    STOCK_INDU_AQUACULTURE, /*yang zhi ye*/
    STOCK_INDU_AUDIO_VISUAL_EQUIPMENT,
    STOCK_INDU_AUTO,
    STOCK_INDU_AUTO_ACCESSORY,

    STOCK_INDU_BANK,
    STOCK_INDU_BASIC_CHEMISTRY,
    STOCK_INDU_BEVERAGE,
    STOCK_INDU_BIOLOGY_PRODUCT,
    STOCK_INDU_BUILDING_DECORATION,
    STOCK_INDU_BUILDING_MATERIAL,
    STOCK_INDU_BUS,

    STOCK_INDU_CHEMISTRY_NEW_MATERIAL,
    STOCK_INDU_CHEMISTRY_PHARMACY, /*zhi yao*/
    STOCK_INDU_CHEMISTRY_PRODUCT,
    STOCK_INDU_CHEMISTRY_SYNTHETIC, /*he cheng*/
    STOCK_INDU_CHINESE_MEDICINE,
    STOCK_INDU_CLOTHING,
    STOCK_INDU_COAL, 
    STOCK_INDU_COLLIGATE,  /* zong he*/
    STOCK_INDU_COMMERCE,
    STOCK_INDU_COMPUTER_APPLICATION,
    STOCK_INDU_COMPUTER_EQUIPMENT,

    STOCK_INDU_ELECTRIC_EQUIPMENT,
    STOCK_INDU_ELECTRIC_POWER,
    STOCK_INDU_ELECTRONIC_MANUFACTURE,
    STOCK_INDU_ELECTRONIC_OTHER,
    STOCK_INDU_ENVIRONMENTAL_ENGINEERING,

    STOCK_INDU_FOOD_MANUFACTURE,
    STOCK_INDU_FORESTRY,

    STOCK_INDU_GAS_WATER,
    STOCK_INDU_GENERAL_EQUIPMENT,

    STOCK_INDU_HOUSEHOLD_LIGHT_INDUSTRY,
    STOCK_INDU_HOTEL_DINING,

    STOCK_INDU_INSTRUMENT_METER, /*yi qi yi biao*/
    STOCK_INDU_INSURANCE,

    STOCK_INDU_LOGISTICS,  /*wu liu*/

    STOCK_INDU_MEDIA,
    STOCK_INDU_MEDICAL_EQUIPMENT_SERVICE,
    STOCK_INDU_MEDICINE_COMMERCE,
    STOCK_INDU_METAL_PROCESSING,
    STOCK_INDU_MINING_SERVICE,

    STOCK_INDU_NAT_DEF_MIL_IND, /*guo fang jun gong*/
    STOCK_INDU_NEW_MATERIAL,
    STOCK_INDU_NOT_AUTO_TRANSPORT,

    STOCK_INDU_OIL_EXPLORATION,

    STOCK_INDU_PACKAGE_PRINTING,
    STOCK_INDU_PAPER_MAKING,
    STOCK_INDU_PARK_DEVELOP,
    STOCK_INDU_PHOTOELECTRON,  /*guang dian zi*/
    STOCK_INDU_PORT_TRANSPORT,

    STOCK_INDU_REAL_ESTATE,
    STOCK_INDU_RETAIL,
    STOCK_INDU_ROAD_RAILWAY_TRANSPORT,

    STOCK_INDU_SEMICONDUCTOR,
    STOCK_INDU_SPECIAL_EQUIPMENT,
    STOCK_INDU_STEEL,
    STOCK_INDU_STOCK,

    STOCK_INDU_TELE_EQUIPMENT,
    STOCK_INDU_TELE_SERVICE,
    STOCK_INDU_TEXTILE_MANUFACTURE,
    STOCK_INDU_TOUR,
    STOCK_INDU_TRANSPORT_EQUIPMENT_SERVICE,

    STOCK_INDU_WHITE_GOODS,

    STOCK_INDU_MAX,
};

/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    olchar_t si_strCode[16];
    u64 si_u64GeneralCapital;
    u64 si_u64TradableShare;
    olint_t si_nIndustry;
    olint_t si_nReserved;
    olchar_t si_strFirstTradeDate[16];

    /*runtime data*/
    boolean_t si_bCsvUptodate;
    u8 si_u8Reserved[7];
    oldouble_t si_dbValue;
} stock_info_t;

typedef struct
{
    stock_info_t * sl_psiStock;
    olint_t sl_nNumOfStock;
    olint_t sl_nMaxStock;
    boolean_t sl_bChanged;
    boolean_t sl_bReserved[7];
} stock_list_t;

typedef struct
{
    olchar_t sii_strChinese[16];
    olint_t sii_nChineseDesc;
    olint_t sii_nId;
    olchar_t * sii_pstrDesc;
    olint_t sii_nStock;
    olchar_t * sii_pstrStocks;
} stock_indu_info_t;

#define SH_COMPOSITE_INDEX              "sh000001"
#define SZ_COMPOSITIONAL_INDEX          "sz399001"
#define SME_COMPOSITIONAL_INDEX         "sz399005"

#define STOCK_QUOTATION_LIST_FILE_NAME  "StockQuotationList.txt"
#define STOCK_STAT_ARBI_LIST_FILE_NAME  "StockStatArbiList.txt"
#define STOCK_TOUGH_LIST_FILE_NAME      "StockToughList.txt"

/* --- functional routines ---------------------------------------------------------------------- */
u32 initStockList(void);
u32 finiStockList(void);

stock_info_t * getFirstStockInfo(void);
stock_info_t * getNextStockInfo(stock_info_t * info);
olint_t getNumOfStock(void);

u32 getIndustryInfo(olint_t id, stock_indu_info_t ** info);
stock_indu_info_t * getFirstIndustryInfo(void);
stock_indu_info_t * getNextIndustryInfo(stock_indu_info_t * info);
olint_t getNumOfIndustry(void);
olchar_t * getStringIndustry(olint_t nIndustry);

void printStockInfoVerbose(stock_info_t * info);
u32 getStockInfo(olchar_t * name, stock_info_t ** info);
olint_t getStockShareThres(stock_info_t * stockinfo);
boolean_t isSmallMediumStock(stock_info_t * stockinfo);

/*Stock Index Information*/
stock_info_t * getFirstStockInfoIndex(void);
stock_info_t * getNextStockInfoIndex(stock_info_t * info);
stock_info_t * getStockInfoIndex(olchar_t * name);
boolean_t isStockInfoIndex(olchar_t * stock);

boolean_t isShStockExchange(olchar_t * stock);

#endif /*TANGXUN_JIUTAI_STOCKLIST_H*/

/*------------------------------------------------------------------------------------------------*/


