/**
 *  @file tx_model.h
 *
 *  @brief Model definition.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_MODEL_H
#define TANGXUN_MODEL_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"
#include "jf_dynlib.h"

#include "tx_stock.h"
#include "tx_daysummary.h"
#include "tx_trade.h"

/* --- constant definitions --------------------------------------------------------------------- */



/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    oldouble_t tmtd_dbFund;
    u32 tmtd_u32Reserved[4];
} tx_model_trade_data_t;

struct tx_model;

typedef u32 (* fnInitDaModel_t)(struct tx_model * ptm);
typedef u32 (* fnFiniDaModel_t)(struct tx_model * ptm);
typedef u32 (* fnCanBeTradedInDaModel_t)(
    struct tx_model * ptm, tx_stock_info_t * stockinfo, trade_pool_stock_t * ptps,
    da_day_summary_t * buffer, olint_t total);
typedef u32 (* fnTradeInDaModel_t)(
    struct tx_model * ptm, tx_stock_info_t * stockinfo, trade_pool_stock_t * ptps,
    tx_model_trade_data_t * ptmtd, da_day_summary_t * buffer, olint_t total);

typedef struct tx_model
{
    olchar_t tm_strName[8];
    olchar_t tm_strLongName[64];

    fnInitDaModel_t tm_fnInitModel;
    fnFiniDaModel_t tm_fnFiniModel;
    fnCanBeTradedInDaModel_t tm_fnCanBeTraded;
    fnTradeInDaModel_t tm_fnTrade;

    void * tm_pData;

    jf_dynlib_t * tm_pjdLib;

    jf_listhead_t tm_jlList;
} tx_model_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 tx_model_initModelFramework(void);
u32 tx_model_finiModelFramework(void);

u32 tx_model_getModel(const olchar_t * name, tx_model_t ** ppModel);

tx_model_t * tx_model_getFirstModel(void);
tx_model_t * tx_model_getNextModel(tx_model_t * ptm);

#endif /*TANGXUN_MODEL_H*/

/*------------------------------------------------------------------------------------------------*/


