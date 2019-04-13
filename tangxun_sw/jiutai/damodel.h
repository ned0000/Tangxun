/**
 *  @file damodel.h
 *
 *  @brief Model definition
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_DAMODEL_H
#define TANGXUN_DAMODEL_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"

#include "stocklist.h"
#include "parsedata.h"
#include "stocktrade.h"

/* --- constant definitions --------------------------------------------------------------------- */



/* --- data structures -------------------------------------------------------------------------- */

typedef enum da_model_id
{
    DA_MODEL_ROI = 0,
    DA_MODEL_MAX_ID,
} da_model_id_t;

typedef struct
{
    oldouble_t dmtd_dbFund;
    u32 dmtd_u32Reserved[4];
} da_model_trade_data_t;

struct da_model;

typedef u32 (* fnInitDaModel_t)(struct da_model * pdm);
typedef u32 (* fnFiniDaModel_t)(struct da_model * pdm);
typedef u32 (* fnCanBeTradedInDaModel_t)(
    struct da_model * pdm, stock_info_t * stockinfo, trade_pool_stock_t * ptps,
    da_day_summary_t * buffer, olint_t total);
typedef u32 (* fnTradeInDaModel_t)(
    struct da_model * pdm, stock_info_t * stockinfo, trade_pool_stock_t * ptps,
    da_model_trade_data_t * pdmtd, da_day_summary_t * buffer, olint_t total);

typedef struct da_model
{
    da_model_id_t dm_dmiId;
    olchar_t dm_strName[8];
    olchar_t dm_strLongName[64];

    fnInitDaModel_t dm_fnInitModel;
    fnFiniDaModel_t dm_fnFiniModel;
    fnCanBeTradedInDaModel_t dm_fnCanBeTraded;
    fnTradeInDaModel_t dm_fnTrade;

    void * dm_pData;

    jf_listhead_t dm_jlList;
} da_model_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 initDaModel(void);
u32 finiDaModel(void);

u32 getDaModel(da_model_id_t id, da_model_t ** model);
u32 getDaModelByName(olchar_t * name, da_model_t ** model);

/* ROI model*/
u32 addDaModelRoi(jf_listhead_t * pjl);



#endif /*TANGXUN_DAMODEL_H*/

/*------------------------------------------------------------------------------------------------*/


