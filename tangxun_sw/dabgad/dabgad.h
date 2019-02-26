/**
 *  @file dabgad.h
 *
 *  @brief Tangxun background activity daemon 
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_DABGAD_H
#define JIUFENG_DABGAD_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */
typedef void  dabgad_t;

typedef struct
{
    olchar_t * dp_pstrCmdLine;
    olchar_t * dp_pstrSettingFile;
    u8 dp_u8Reserved[16];
} dabgad_param_t;

/* --- functional routines ------------------------------------------------- */
u32 setDefaultDabgadParam(dabgad_param_t * pgp);

u32 createDabgad(dabgad_t ** ppDabgad, dabgad_param_t * pgp);

u32 destroyDabgad(dabgad_t ** ppDabgad);

u32 startDabgad(dabgad_t * pDabgad);

u32 stopDabgad(dabgad_t * pDabgad);

#endif /*JIUFENG_DABGAD_H*/

/*---------------------------------------------------------------------------*/


