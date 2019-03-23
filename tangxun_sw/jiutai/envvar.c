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
#include "olbasic.h"
#include "ollimit.h"
#include "bases.h"
#include "envvar.h"
#include "stringparse.h"
#include "files.h"
#include "xmalloc.h"
#include "persistency.h"

/* --- private data/data structure section --------------------------------- */

#define MAX_STOCK_IN_POOL     (30)

typedef struct
{
    olchar_t ev_strDataPath[MAX_PATH_LEN];
    olint_t ev_nDaysForStockInPool;
    olint_t ev_nMaxStockInPool;
} env_var_t;

env_var_t ls_evEnvVar = {
    {"/opt/stock"},
    0,
    MAX_STOCK_IN_POOL};

static persistency_t * ls_pEnvPersist = NULL;

/* --- private routine section---------------------------------------------- */

static u32 _clearEnvVar(env_var_t * pev, olchar_t * name)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_t * pPersist = ls_pEnvPersist;
    olchar_t value[16];

    if (strcasecmp(name, ENV_VAR_DATA_PATH) == 0)
    {
        pev->ev_strDataPath[0] = '\0';
        value[0] = '\0';
    }
    else if (strcasecmp(name, ENV_VAR_DAYS_STOCK_POOL) == 0)
    {
        pev->ev_nDaysForStockInPool = 0;
        ol_strcpy(value, "0");
    }
    else if (strcasecmp(name, ENV_VAR_MAX_STOCK_IN_POOL) == 0)
    {
        pev->ev_nMaxStockInPool = 0;
        ol_strcpy(value, "0");
    }
    else
    {
        u32Ret = OLERR_INVALID_PARAM;
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = setPersistencyValue(pPersist, name, value);


    return u32Ret;
}

static u32 _setEnvVar(env_var_t * pev, olchar_t * data)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t * name, * value;
    persistency_t * pPersist = ls_pEnvPersist;

    u32Ret = processSettingString(data, &name, &value);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (strcasecmp(name, ENV_VAR_DATA_PATH) == 0)
        {
            ol_strcpy(pev->ev_strDataPath, value);
        }
        else if (strcasecmp(name, ENV_VAR_DAYS_STOCK_POOL) == 0)
        {
            getS32FromString(value, ol_strlen(value), &pev->ev_nDaysForStockInPool);
        }
        else if (strcasecmp(name, ENV_VAR_MAX_STOCK_IN_POOL) == 0)
        {
            getS32FromString(value, ol_strlen(value), &pev->ev_nMaxStockInPool);
        }
        else
        {
            u32Ret = OLERR_INVALID_PARAM;
        }

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = setPersistencyValue(pPersist, name, value);
    }

    return u32Ret;
}

static u32 _initEnvVar(persistency_t * pPersist)
{
    u32 u32Ret = OLERR_NO_ERROR;
    env_var_t * pev = &ls_evEnvVar;
    olchar_t value[MAX_PATH_LEN];

    u32Ret = getPersistencyValue(pPersist, ENV_VAR_DATA_PATH, value, sizeof(value));
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (strlen(value) > 0)
            ol_strcpy(pev->ev_strDataPath, value);

        u32Ret = getPersistencyValue(pPersist, ENV_VAR_DAYS_STOCK_POOL, value, sizeof(value));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (strlen(value) > 0)
            getS32FromString(value, ol_strlen(value), &pev->ev_nDaysForStockInPool);

        u32Ret = getPersistencyValue(pPersist, ENV_VAR_MAX_STOCK_IN_POOL, value, sizeof(value));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (strlen(value) > 0)
            getS32FromString(value, ol_strlen(value), &pev->ev_nMaxStockInPool);
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
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = initEnvPersistency();

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _setEnvVar(&ls_evEnvVar, data);

    return u32Ret;
}

u32 clearEnvVar(olchar_t * name)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = initEnvPersistency();

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _clearEnvVar(&ls_evEnvVar, name);

    return u32Ret;
}

u32 initEnvPersistency(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_config_t config;

    if (ls_pEnvPersist != NULL)
        return u32Ret;

    logInfoMsg("init env persist");

    memset(&config, 0, sizeof(persistency_config_t));
    ol_strcpy(config.pc_pcsConfigSqlite.pcs_strDbName, "../db/tangxun.db");
    ol_strcpy(config.pc_pcsConfigSqlite.pcs_strTableName, "env");
    ol_strcpy(config.pc_pcsConfigSqlite.pcs_strKeyColumnName, "key");
    ol_strcpy(config.pc_pcsConfigSqlite.pcs_strValueColumnName, "value");

    u32Ret = createPersistency(SQLITE_PERSISTENCY, &config, &ls_pEnvPersist);
    if (u32Ret == OLERR_NO_ERROR)
        _initEnvVar(ls_pEnvPersist);

    return u32Ret;
}

u32 finiEnvPersistency(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_t * pPersist = ls_pEnvPersist;

    if (pPersist == NULL)
        return OLERR_NO_ERROR;

    logInfoMsg("fini env persist");

    u32Ret = destroyPersistency(&pPersist);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


