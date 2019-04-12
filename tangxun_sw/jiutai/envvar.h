/**
 *  @file envvar.h
 *
 *  @brief The routines for environment variable
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_ENVVAR_H
#define TANGXUN_JIUTAI_ENVVAR_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions ------------------------------------------------ */

#define ENV_VAR_DATA_PATH           "DataPath"
#define ENV_VAR_DAYS_STOCK_POOL     "DaysForStockInPool"
#define ENV_VAR_MAX_STOCK_IN_POOL   "MaxStockInPool"

/* --- data structures ----------------------------------------------------- */


/* --- functional routines ------------------------------------------------- */

char * getEnvVar(olchar_t * name);
olint_t getEnvVarDaysStockPool(void);
olint_t getEnvVarMaxStockInPool(void);

boolean_t isNullEnvVarDataPath(void);

u32 setEnvVar(olchar_t * data);
u32 clearEnvVar(olchar_t * name);

/*the envirionment variable persistency */
u32 initEnvPersistency(void);
u32 finiEnvPersistency(void);

#endif /*TANGXUN_JIUTAI_ENVVAR_H*/

/*---------------------------------------------------------------------------*/


