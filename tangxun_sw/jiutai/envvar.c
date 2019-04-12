/**
 *  @file env.c
 *
 *  @brief The routines for environment variable
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_listhead.h"
#include "envvar.h"
#include "jf_string.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "jf_persistency.h"

/* --- private data/data structure section --------------------------------- */

#define MAX_STOCK_IN_POOL     (30)

typedef struct
{
    olchar_t ev_strDataPath[JF_LIMIT_MAX_PATH_LEN];
    olint_t ev_nDaysForStockInPool;
    olint_t ev_nMaxStockInPool;
} env_var_t;

env_var_t ls_evEnvVar = {
    {"/opt/stock"},
    0,
    MAX_STOCK_IN_POOL};

static jf_persistency_t * ls_jpEnvPersist = NULL;

/* --- private routine section---------------------------------------------- */

static u32 _clearEnvVar(env_var_t * pev, olchar_t * name)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_persistency_t * pPersist = ls_jpEnvPersist;
    olchar_t value[16];

    if (ol_strcasecmp(name, ENV_VAR_DATA_PATH) == 0)
    {
        pev->ev_strDataPath[0] = '\0';
        value[0] = '\0';
    }
    else if (ol_strcasecmp(name, ENV_VAR_DAYS_STOCK_POOL) == 0)
    {
        pev->ev_nDaysForStockInPool = 0;
        ol_strcpy(value, "0");
    }
    else if (ol_strcasecmp(name, ENV_VAR_MAX_STOCK_IN_POOL) == 0)
    {
        pev->ev_nMaxStockInPool = 0;
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

static u32 _setEnvVar(env_var_t * pev, olchar_t * data)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * name, * value;
    jf_persistency_t * pPersist = ls_jpEnvPersist;

    u32Ret = jf_string_processSettingString(data, &name, &value);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ol_strcasecmp(name, ENV_VAR_DATA_PATH) == 0)
        {
            ol_strcpy(pev->ev_strDataPath, value);
        }
        else if (ol_strcasecmp(name, ENV_VAR_DAYS_STOCK_POOL) == 0)
        {
            jf_string_getS32FromString(value, ol_strlen(value), &pev->ev_nDaysForStockInPool);
        }
        else if (ol_strcasecmp(name, ENV_VAR_MAX_STOCK_IN_POOL) == 0)
        {
            jf_string_getS32FromString(value, ol_strlen(value), &pev->ev_nMaxStockInPool);
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
    env_var_t * pev = &ls_evEnvVar;
    olchar_t value[JF_LIMIT_MAX_PATH_LEN];

    u32Ret = jf_persistency_getValue(pPersist, ENV_VAR_DATA_PATH, value, sizeof(value));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ol_strlen(value) > 0)
            ol_strcpy(pev->ev_strDataPath, value);

        u32Ret = jf_persistency_getValue(pPersist, ENV_VAR_DAYS_STOCK_POOL, value, sizeof(value));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ol_strlen(value) > 0)
            jf_string_getS32FromString(value, ol_strlen(value), &pev->ev_nDaysForStockInPool);

        u32Ret = jf_persistency_getValue(pPersist, ENV_VAR_MAX_STOCK_IN_POOL, value, sizeof(value));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ol_strlen(value) > 0)
            jf_string_getS32FromString(value, ol_strlen(value), &pev->ev_nMaxStockInPool);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olchar_t * getEnvVar(olchar_t * name)
{
    if (strcasecmp(name, ENV_VAR_DATA_PATH) == 0)
    {
        return ls_evEnvVar.ev_strDataPath;
    }

    return NULL;
}

boolean_t isNullEnvVarDataPath(void)
{
    boolean_t bRet = FALSE;

    if (ls_evEnvVar.ev_strDataPath[0] == '\0')
        bRet = TRUE;

    return bRet;
}

olint_t getEnvVarDaysStockPool(void)
{
    return ls_evEnvVar.ev_nDaysForStockInPool;
}

olint_t getEnvVarMaxStockInPool(void)
{
    return ls_evEnvVar.ev_nMaxStockInPool;
}

u32 setEnvVar(olchar_t * data)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = initEnvPersistency();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _setEnvVar(&ls_evEnvVar, data);

    return u32Ret;
}

u32 clearEnvVar(olchar_t * name)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = initEnvPersistency();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _clearEnvVar(&ls_evEnvVar, name);

    return u32Ret;
}

u32 initEnvPersistency(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_persistency_config_t config;

    if (ls_jpEnvPersist != NULL)
        return u32Ret;

    jf_logger_logInfoMsg("init env persist");

    ol_memset(&config, 0, sizeof(jf_persistency_config_t));
    ol_strcpy(config.jpc_pcsConfigSqlite.jpcs_strDbName, "../db/tangxun.db");
    ol_strcpy(config.jpc_pcsConfigSqlite.jpcs_strTableName, "env");
    ol_strcpy(config.jpc_pcsConfigSqlite.jpcs_strKeyColumnName, "key");
    ol_strcpy(config.jpc_pcsConfigSqlite.jpcs_strValueColumnName, "value");

    u32Ret = jf_persistency_create(JF_PERSISTENCY_TYPE_SQLITE, &config, &ls_jpEnvPersist);
    if (u32Ret == JF_ERR_NO_ERROR)
        _initEnvVar(ls_jpEnvPersist);

    return u32Ret;
}

u32 finiEnvPersistency(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_persistency_t * pPersist = ls_jpEnvPersist;

    if (pPersist == NULL)
        return JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("fini env persist");

    u32Ret = jf_persistency_destroy(&pPersist);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


