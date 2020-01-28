/**
 *  @file tx_sector.h
 *
 *  @brief Routine for parsing data into day result data structure.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef TANGXUN_JIUTAI_SECTOR_H
#define TANGXUN_JIUTAI_SECTOR_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    olchar_t tsi_strName[64];
    olchar_t * tsi_pstrStocks;
} tx_sector_info_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Allocate memory for tsi_pstrStocks, call tx_sector_freeSectorInfo() to free the memory.
 */
u32 tx_sector_parseDir(
    olchar_t * pstrDir, tx_sector_info_t * sector, olint_t * numofsector);

void tx_sector_freeSectorInfo(tx_sector_info_t * sector);


#endif /*TANGXUN_JIUTAI_SECTOR_H*/

/*------------------------------------------------------------------------------------------------*/


