/**
 *  @file tx_env.c
 *
 *  @brief The routines for environment variable.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_listhead.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "jf_persistency.h"

#include "tx_env.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define MAX_STOCK_IN_POOL     (30)

typedef struct
{
    olchar_t tev_strDataPath[JF_LIMIT_MAX_PATH_LEN];
    olint_t tev_nDaysForStockInPool;
    olint_t tev_nMaxStockInPool;
} tx_env_var_t;

tx_env_var_t ls_tevEnvVar = {
    {"/opt/stock"},
    0,
    MAX_STOCK_IN_POOL};

static jf_persistency_t * ls_pjpEnvPersist = NULL;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _clearEnvVar(tx_env_var_t * ptev, olchar_t * name)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_persistency_t * pPersist = ls_pjpEnvPersist;
    olchar_t value[16];

    if (ol_strcasecmp(name, TX_ENV_VAR_DATA_PATH) == 0)
    {
        ptev->tev_strDataPath[0] = '\0';
        value[0] = '\0';
    }
    else if (ol_strcasecmp(name, TX_ENV_VAR_DAYS_STOCK_POOL) == 0)
    {
        ptev->tev_nDaysForStockInPool = 0;
        ol_strcpy(value, "0");
    }
    else if (ol_strcasecmp(name, TX_ENV_VAR_MAX_STOCK_IN_POOL) == 0)
    {
        ptev->tev_nMaxStockInPool = 0;
        ol_strcpy(value, "0");
    }
    else
    {
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_persistency_setValue(pPersist, name, value);


    return u32Ret;
}

static u32 _setEnvVar(tx_env_var_t * ptev, olchar_t * data)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * name, * value;
    jf_persistency_t * pPersist = ls_pjpEnvPersist;

    u32Ret = jf_string_processSettingString(data, &name, &value);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ol_strcasecmp(name, TX_ENV_VAR_DATA_PATH) == 0)
        {
            ol_strcpy(ptev->tev_strDataPath, value);
        }
        else if (ol_strcasecmp(name, TX_ENV_VAR_DAYS_STOCK_POOL) == 0)
        {
            jf_string_getS32FromString(value, ol_strlen(value), &ptev->tev_nDaysForStockInPool);
        }
        else if (ol_strcasecmp(name, TX_ENV_VAR_MAX_STOCK_IN_POOL) == 0)
        {
            jf_string_getS32FromString(value, ol_strlen(value), &ptev->tev_nMaxStockInPool);
        }
        else
        {
            u32Ret = JF_ERR_INVALID_PARAM;
        }

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_persistency_setValue(pPersist, name, value);
    }

    return u32Ret;
}

static u32 _initEnvVar(jf_persistency_t * pPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_env_var_t * ptev = &ls_tevEnvVar;
    olchar_t value[JF_LIMIT_MAX_PATH_LEN];

    u32Ret = jf_persistency_getValue(pPersist, TX_ENV_VAR_DATA_PATH, value, sizeof(value));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ol_strlen(value) > 0)
            ol_strcpy(ptev->tev_strDataPath, value);

        u32Ret = jf_persistency_getValue(pPersist, TX_ENV_VAR_DAYS_STOCK_POOL, value, sizeof(value));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ol_strlen(value) > 0)
            jf_string_getS32FromString(value, ol_strlen(value), &ptev->tev_nDaysForStockInPool);

        u32Ret = jf_persistency_getValue(pPersist, TX_ENV_VAR_MAX_STOCK_IN_POOL, value, sizeof(value));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ol_strlen(value) > 0)
            jf_string_getS32FromString(value, ol_strlen(value), &ptev->tev_nMaxStockInPool);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olchar_t * tx_env_getVar(olchar_t * name)
{
    if (strcasecmp(name, TX_ENV_VAR_DATA_PATH) == 0)
    {
        return ls_tevEnvVar.tev_strDataPath;
    }

    return NULL;
}

boolean_t tx_env_isNullVarDataPath(void)
{
    boolean_t bRet = FALSE;

    if (ls_tevEnvVar.tev_strDataPath[0] == '\0')
        bRet = TRUE;

    return bRet;
}

olint_t tx_env_getVarDaysStockPool(void)
{
    return ls_tevEnvVar.tev_nDaysForStockInPool;
}

olint_t tx_env_getVarMaxStockInPool(void)
{
    return ls_tevEnvVar.tev_nMaxStockInPool;
}

u32 tx_env_setVar(olchar_t * data)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = tx_env_initPersistency();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _setEnvVar(&ls_tevEnvVar, data);

    return u32Ret;
}

u32 tx_env_clearVar(olchar_t * name)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = tx_env_initPersistency();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _clearEnvVar(&ls_tevEnvVar, name);

    return u32Ret;
}

u32 tx_env_initPersistency(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_persistency_config_t config;

    if (ls_pjpEnvPersist != NULL)
        return u32Ret;

    JF_LOGGER_INFO("init persist");

    ol_memset(&config, 0, sizeof(jf_persistency_config_t));
    ol_strcpy(config.jpc_pcsConfigSqlite.jpcs_strDbName, "../db/tangxun.db");
    ol_strcpy(config.jpc_pcsConfigSqlite.jpcs_strTableName, "env");
    ol_strcpy(config.jpc_pcsConfigSqlite.jpcs_strKeyColumnName, "key");
    ol_strcpy(config.jpc_pcsConfigSqlite.jpcs_strValueColumnName, "value");

    u32Ret = jf_persistency_create(JF_PERSISTENCY_TYPE_SQLITE, &config, &ls_pjpEnvPersist);
    if (u32Ret == JF_ERR_NO_ERROR)
        _initEnvVar(ls_pjpEnvPersist);

    return u32Ret;
}

u32 tx_env_finiPersistency(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_persistency_t * pPersist = ls_pjpEnvPersist;

    if (pPersist == NULL)
        return JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("fini env persist");

    u32Ret = jf_persistency_destroy(&pPersist);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


