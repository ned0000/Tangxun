/**
 *  @file clicmd.h
 *
 *  @brief CLI command processor
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

#ifndef TANGXUN_CLI_CLICMD_H
#define TANGXUN_CLI_CLICMD_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_clieng.h"
#include "jf_listhead.h"
#include "main.h"

/* --- constant definitions ------------------------------------------------ */
#define CLI_ACTION_UNKNOWN    0
#define CLI_ACTION_SHOW_HELP  1


/* --- data structures ----------------------------------------------------- */

typedef struct
{
    u8 chp_u8Action;
    u8 chp_u8Reserved[15];
} cli_help_param_t;

typedef struct
{
    u8 cep_u8Action;
    u8 cep_u8Reserved[15];
} cli_exit_param_t;

typedef struct
{
#define CLI_ACTION_ANALYSIS_STOCK            0x80
#define CLI_ACTION_ANALYSIS_STOCK_POOL       0x81
#define CLI_ACTION_ANALYSIS_INDEX_STOCKS     0x82
#define CLI_ACTION_ANALYSIS_STOCK_STAT_ARBI  0x83
#define CLI_ACTION_ANALYSIS_STOCK_LIMIT      0x84
#define CLI_ACTION_ANALYSIS_SECTOR           0x85
#define CLI_ACTION_ANALYSIS_STOCK_TOUGH      0x86
#define CLI_ACTION_ANALYSIS_STOCK_QUOTATION  0x87
    u8 cap_u8Action;
    boolean_t cap_bVerbose;
    u8 cap_u8Reserved[14];
    u32 cap_u32Reserved[8];
    olint_t cap_nDir;
#define MAX_ANALYSIS_DIR   10
    olchar_t * cap_pstrDir[MAX_ANALYSIS_DIR];
    olchar_t * cap_pstrStartDate;
    olchar_t * cap_pstrEndDate;
} cli_analysis_param_t;

typedef struct
{
#define CLI_ACTION_CREATE_EXDR_FILE  0x80
    u8 cmp_u8Action;
    boolean_t cmp_bVerbose;
    u8 cmp_u8Reserved[14];
    u32 cmp_u32Reserved[8];
} cli_misc_param_t;

typedef struct
{
#define CLI_ACTION_MODEL_LIST_ALL  0x80
#define CLI_ACTION_MODEL_LIST      0x81
    u8 cmp_u8Action;
    boolean_t cmp_bVerbose;
    u8 cmp_u8Reserved[14];
    u32 cmp_u32ModelId;
    u32 cmp_u32Reserved[7];
} cli_model_param_t;

typedef struct
{
#define CLI_ACTION_INFLEXION_POINT    0x80
#define CLI_ACTION_MARKOV_DIR         0x81
#define CLI_ACTION_DESCRIPTIVE_STAT   0x82
#define CLI_ACTION_STAT_ARBI_INDUSTRY 0x83
#define CLI_ACTION_STAT_ARBI_STOCKS   0x84
#define CLI_ACTION_STAT_ARBI_FIND     0x85
#define CLI_ACTION_INFLEXION_POINT_2  0x86
    u8 csp_u8Action;
    boolean_t csp_bVerbose;
    u8 csp_u8Reserved[14];
    olchar_t * csp_pstrData;
    olchar_t * csp_pstrIndustry;
    olint_t csp_nLastCount;
    u32 csp_u32Reserved[3];
} cli_stat_param_t;

typedef struct
{
#define CLI_ACTION_ENV_LIST_ALL   0x80
#define CLI_ACTION_ENV_LIST       0x81
#define CLI_ACTION_ENV_SET        0x82
#define CLI_ACTION_ENV_CLEAR      0x83
    u8 cep_u8Action;
    u8 cep_u8Reserved[15];
    olchar_t * cep_pstrData;
    u32 cep_u32Reserved[4];
} cli_env_param_t;

typedef struct
{
#define CLI_ACTION_FIX_FILE  0x80
    u8 cfp_u8Action;
    boolean_t cfp_bVerbose;
    boolean_t cfp_bOverwrite;
    u8 cfp_u8Reserved[13];
    olchar_t * cfp_pstrData;
    u32 cfp_u32Reserved[4];
} cli_fix_param_t;

typedef struct
{
#define CLI_ACTION_DOWNLOAD_TRADE_SUMMARY     0x80
#define CLI_ACTION_DOWNLOAD_TRADE_DETAIL      0x81
#define CLI_ACTION_DOWNLOAD_EXDR              0x82
#define CLI_ACTION_DOWNLOAD_STOCK_INFO_INDEX  0x83
    u8 cdp_u8Action;
    boolean_t cdp_bVerbose;
    boolean_t cdp_bOverwrite;
    u8 cdp_u8IterativeCount;
    u8 cdp_u8Reserved[12];
    olchar_t * cdp_pstrStock;
    u32 cdp_u32Reserved[3];
    olint_t cdp_nSleep;
    olchar_t * cdp_pstrStartDate;
    olchar_t * cdp_pstrEndDate;
    olchar_t * cdp_pstrDataDir;
} cli_download_param_t;

typedef struct
{
#define CLI_ACTION_FIND_STOCK                0x80
#define CLI_ACTION_FIND_STOCK_IN_PERIOD      0x81
#define CLI_ACTION_FIND_LIST_POOL            0x82
#define CLI_ACTION_FIND_LIST_POOL_BY_MODEL   0x83
#define CLI_ACTION_CLEAN_STOCK_POOL          0x84
    u8 cfp_u8Action;
    boolean_t cfp_bVerbose;
    boolean_t cfp_bAllStocks;
    u8 cfp_u8Reserved[13];
    u32 cfp_u32Reserved[4];
    olchar_t * cfp_pstrPeriod;
} cli_find_param_t;

typedef struct
{
#define CLI_ACTION_TRADE_STOCK         0x80
#define CLI_ACTION_TRADE_LIST          0x81
#define CLI_ACTION_TRADE_LIST_RECORD   0x82
    u8 ctp_u8Action;
    boolean_t ctp_bVerbose;
    u8 ctp_u8Reserved[14];
    u32 ctp_u32Reserved[4];
} cli_trade_param_t;

typedef struct
{
#define CLI_ACTION_LIST_STOCK     0x80
#define CLI_ACTION_LIST_INDUSTRY  0x81
    u8 csp_u8Action;
    boolean_t csp_bVerbose;
    u8 csp_u8Reserved[14];
    u32 csp_u32Reserved[4];
    olchar_t * csp_pstrStock;
} cli_stock_param_t;

typedef struct
{
#define CLI_ACTION_PARSE_TRADE_SUMMARY  0x80
#define CLI_ACTION_PARSE_TRADE_DETAIL   0x81
#define CLI_ACTION_PARSE_QUOTATION      0x82
    u8 cpp_u8Action;
    boolean_t cpp_bVerbose;
	boolean_t cpp_bFRoR;
    boolean_t cpp_bStartDate;
    boolean_t cpp_bEndDate;
    u8 cpp_u8Reserved[11];
    olchar_t * cpp_pstrStock;
    olchar_t * cpp_pstrDate;
    olint_t cpp_nLastCount;
    olint_t cpp_nThres;
    u32 cpp_u32Reserved[6];
} cli_parse_param_t;

typedef struct
{
#define CLI_ACTION_INDI_LIST       0x80
#define CLI_ACTION_INDI_FIND       0x81
#define CLI_ACTION_INDI_TEST       0x82
#define CLI_ACTION_INDI_EVALUATE   0x83
#define CLI_ACTION_INDIS_EVALUATE  0x84
#define CLI_ACTION_INDI_ADXR       0x85
    u8 cip_u8Action;
    boolean_t cip_bVerbose;
    boolean_t cip_bPrintData;
    u8 cip_u8Reserved[13];
    u32 cip_u32Reserved[8];
    olchar_t * cip_pstrStock;
    olchar_t * cip_pstrIndicator;
    olchar_t * cip_pstrParam;
    olchar_t * cip_pstrEndDate;
} cli_indi_param_t;

typedef struct
{
#define CLI_ACTION_RULE_LIST_ALL  0x80
#define CLI_ACTION_RULE_LIST      0x81
    u8 crp_u8Action;
    boolean_t crp_bVerbose;
    u8 crp_u8Reserved[14];
    u32 crp_u32RuleId;
} cli_rule_param_t;

typedef struct
{
#define CLI_ACTION_BACKTEST_ALL              0x80
#define CLI_ACTION_BACKTEST_MODEL            0x81
    u8 cbp_u8Action;
    boolean_t cbp_bVerbose;
    boolean_t cbp_bStockByStock;
    u8 cbp_u8Reserved[13];
    u32 cbp_u32Reserved[4];
    olchar_t * cbp_pstrModel;
} cli_backtest_param_t;

typedef union
{
    cli_exit_param_t dcp_cepExit;
    cli_env_param_t dcp_cepEnv;
    cli_help_param_t dcp_chpHelp;
    cli_analysis_param_t dcp_capAnalysis;
    cli_model_param_t dcp_cmpModel;
    cli_stat_param_t dcp_cspStat;
    cli_fix_param_t dcp_cfpFix;
    cli_download_param_t dcp_cfpDownload;
    cli_find_param_t dcp_cfpFind;
    cli_stock_param_t dcp_cspStock;
    cli_parse_param_t dcp_cppParse;
    cli_misc_param_t dcp_cmpMisc;
    cli_indi_param_t dcp_cipIndi;
    cli_rule_param_t dcp_crpRule;
    cli_trade_param_t dcp_ctpTrade;
    cli_backtest_param_t dcp_cbpBacktest;
} da_cli_param_t;

/* --- functional routines ------------------------------------------------- */

u32 addDaCmd(da_master_t * pdm, da_cli_param_t * dcp);

u32 parseAnalysis(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);
u32 setDefaultParamAnalysis(void * pMaster, void * pParam);
u32 processAnalysis(void * pMaster, void * pParam);

u32 processModel(void * pMaster, void * pParam);
u32 setDefaultParamModel(void * pMaster, void * pParam);
u32 parseModel(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

u32 processStat(void * pMaster, void * pParam);
u32 setDefaultParamStat(void * pMaster, void * pParam);
u32 parseStat(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

u32 processFix(void * pMaster, void * pParam);
u32 setDefaultParamFix(void * pMaster, void * pParam);
u32 parseFix(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

u32 processEnv(void * pMaster, void * pParam);
u32 setDefaultParamEnv(void * pMaster, void * pParam);
u32 parseEnv(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);
char * getEnvVar(olchar_t * name);
olint_t getEnvVarDaysStockPool(void);
olint_t getEnvVarMaxStockInPool(void);

u32 processDownload(void * pMaster, void * pParam);
u32 setDefaultParamDownload(void * pMaster, void * pParam);
u32 parseDownload(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

u32 processFind(void * pMaster, void * pParam);
u32 setDefaultParamFind(void * pMaster, void * pParam);
u32 parseFind(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

u32 processStock(void * pMaster, void * pParam);
u32 setDefaultParamStock(void * pMaster, void * pParam);
u32 parseStock(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

u32 processParse(void * pMaster, void * pParam);
u32 setDefaultParamParse(void * pMaster, void * pParam);
u32 parseParse(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

u32 processMisc(void * pMaster, void * pParam);
u32 setDefaultParamMisc(void * pMaster, void * pParam);
u32 parseMisc(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

u32 processIndi(void * pMaster, void * pParam);
u32 setDefaultParamIndi(void * pMaster, void * pParam);
u32 parseIndi(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

u32 processRule(void * pMaster, void * pParam);
u32 setDefaultParamRule(void * pMaster, void * pParam);
u32 parseRule(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

u32 processTrade(void * pMaster, void * pParam);
u32 setDefaultParamTrade(void * pMaster, void * pParam);
u32 parseTrade(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

u32 processBacktest(void * pMaster, void * pParam);
u32 setDefaultParamBacktest(void * pMaster, void * pParam);
u32 parseBacktest(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

#endif /*TANGXUN_CLI_CLICMD_H*/

/*---------------------------------------------------------------------------*/


