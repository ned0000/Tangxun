/**
 *  @file sqlitejf_persistency.h
 *
 *  @brief The sqlite persistency
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TRADE_PERSISTENCY_SQLPERSISTENCY_H
#define TRADE_PERSISTENCY_SQLPERSISTENCY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "persistencycommon.h"
#include "sqlite3.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 initTpSqlite(tp_manager_t * pManager);


#endif /*TRADE_PERSISTENCY_SQLPERSISTENCY_H*/

/*------------------------------------------------------------------------------------------------*/


