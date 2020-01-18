/**
 *  @file tx_env.h
 *
 *  @brief The routines for environment variable
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_ENV_H
#define TANGXUN_JIUTAI_ENV_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define TX_ENV_VAR_DATA_PATH           "DataPath"
#define TX_ENV_VAR_DAYS_STOCK_POOL     "DaysForStockInPool"
#define TX_ENV_VAR_MAX_STOCK_IN_POOL   "MaxStockInPool"

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

char * tx_env_getVar(olchar_t * name);
olint_t tx_env_getVarDaysStockPool(void);
olint_t tx_env_getVarMaxStockInPool(void);

boolean_t tx_env_isNullVarDataPath(void);

u32 tx_env_setVar(olchar_t * data);
u32 tx_env_clearVar(olchar_t * name);

/*the envirionment variable persistency */
u32 tx_env_initPersistency(void);
u32 tx_env_finiPersistency(void);

#endif /*TANGXUN_JIUTAI_ENV_H*/

/*------------------------------------------------------------------------------------------------*/


