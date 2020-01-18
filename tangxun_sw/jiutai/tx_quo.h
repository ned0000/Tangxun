/**
 *  @file parsedata.h
 *
 *  @brief routine for parsing data
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
    u64 qdp_u64Volume;
    oldouble_t qdp_dbPrice;
} quo_data_price_t;

typedef struct
{
    oldouble_t qe_dbCurPrice;
    oldouble_t qe_dbHighPrice;
    oldouble_t qe_dbLowPrice;
    u64 qe_u64Volume;
    oldouble_t qe_dbAmount;
    quo_data_price_t qe_qdpBuy[5];
    quo_data_price_t qe_qdpSold[5];

    olchar_t qe_strTime[16];
} quo_entry_t;

typedef struct stock_quo
{
    olchar_t sq_strCode[16];
    oldouble_t sq_dbOpeningPrice;
    oldouble_t sq_dbLastClosingPrice;
    olchar_t sq_strDate[16];

    olint_t sq_nMaxEntry;
    olint_t sq_nNumOfEntry;
    quo_entry_t * sq_pqeEntry;

//    struct stock_quo * sq_psqPair;
} stock_quo_t;

/* --- functional routines ---------------------------------------------------------------------- */

quo_entry_t * getQuoEntryWithHighestPrice(quo_entry_t * start, quo_entry_t * end);

quo_entry_t * getQuoEntryWithLowestPrice(quo_entry_t * start, quo_entry_t * end);

void getQuoEntryInflexionPoint(
    quo_entry_t * buffer, olint_t num, quo_entry_t ** ppq, olint_t * nump);

u32 readStockQuotationFile(olchar_t * pstrDataDir, stock_quo_t * psq);

olint_t getNextTopBottomQuoEntry(quo_entry_t ** pqe, olint_t total, olint_t start);

#endif /*TANGXUN_JIUTAI_QUO_H*/

/*------------------------------------------------------------------------------------------------*/


