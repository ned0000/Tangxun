/**
 *  @file template.h
 *
 *  @brief template header file
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef JIUFENG_TEMPLATE_H
#define JIUFENG_TEMPLATE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 addDaModel(jf_listhead_t * pjl, const char * pstrLibDir);

u32 removeDaModel(jf_listhead_t * pjl);

#endif /*JIUFENG_TEMPLATE_H*/

/*------------------------------------------------------------------------------------------------*/


