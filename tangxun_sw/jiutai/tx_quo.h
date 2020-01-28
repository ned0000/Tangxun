/**
 *  @file tx_quo.h
 *
 *  @brief Routine for parsing data
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_QUO_H
#define TANGXUN_JIUTAI_QUO_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    u64 tqp_u64Volume;
    oldouble_t tqp_dbPrice;
} tx_quo_price_t;

typedef struct
{
    oldouble_t tqe_dbCurPrice;
    oldouble_t tqe_dbHighPrice;
    oldouble_t tqe_dbLowPrice;
    u64 tqe_u64Volume;
    oldouble_t tqe_dbAmount;
    tx_quo_price_t tqe_tqpBuy[5];
    tx_quo_price_t tqe_tqpSold[5];

    olchar_t tqe_strTime[16];
} tx_quo_entry_t;

typedef struct stock_quo
{
    olchar_t sq_strCode[16];
    oldouble_t sq_dbOpeningPrice;
    oldouble_t sq_dbLastClosingPrice;
    olchar_t sq_strDate[16];

    olint_t sq_nMaxEntry;
    olint_t sq_nNumOfEntry;
    tx_quo_entry_t * sq_ptqeEntry;

//    struct stock_quo * sq_psqPair;
} stock_quo_t;

/* --- functional routines ---------------------------------------------------------------------- */

tx_quo_entry_t * getQuoEntryWithHighestPrice(tx_quo_entry_t * start, tx_quo_entry_t * end);

tx_quo_entry_t * getQuoEntryWithLowestPrice(tx_quo_entry_t * start, tx_quo_entry_t * end);

void getQuoEntryInflexionPoint(
    tx_quo_entry_t * buffer, olint_t num, tx_quo_entry_t ** ppq, olint_t * nump);

u32 readStockQuotationFile(olchar_t * pstrDataDir, stock_quo_t * psq);

olint_t getNextTopBottomQuoEntry(tx_quo_entry_t ** ptqe, olint_t total, olint_t start);

#endif /*TANGXUN_JIUTAI_QUO_H*/

/*------------------------------------------------------------------------------------------------*/


