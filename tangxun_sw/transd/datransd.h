/**
 *  @file datransd.h
 *
 *  @brief Tangxun trasaction daemon
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_DATRANSD_H
#define JIUFENG_DATRANSD_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/*in seconds*/
#define DA_GET_QUO_INTERVAL  6

/* --- data structures -------------------------------------------------------------------------- */
typedef void  datransd_t;

typedef struct
{
    olchar_t * dp_pstrCmdLine;
    u8 dp_u8Reserved[64];
} datransd_param_t;

/* --- functional routines ---------------------------------------------------------------------- */
u32 setDefaultDatransdParam(datransd_param_t * pgp);

u32 createDatransd(datransd_t ** ppDatransd, datransd_param_t * pgp);

u32 destroyDatransd(datransd_t ** ppDatransd);

u32 startDatransd(datransd_t * pDatransd);

u32 stopDatransd(datransd_t * pDatransd);

#endif /*JIUFENG_DATRANSD_H*/

/*------------------------------------------------------------------------------------------------*/


