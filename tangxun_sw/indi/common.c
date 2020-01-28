/**
 *  @file tangxun_sw/indi/common.c
 *
 *  @brief Implementation file for indicator common definition and routines.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_jiukun.h"

#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 freeIndicatorDataById(tx_ds_t * buffer, olint_t total, olint_t id)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * end = buffer + total - 1;
    olint_t index = 0;

    while (index < total)
    {
        switch (id)
        {
        case TX_INDI_ID_DMI:
            if (end->td_ptidDmi != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptidDmi);
            if (end->td_ptidpDmiParam != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptidpDmiParam);
            break;
        case TX_INDI_ID_MACD:
            if (end->td_ptimMacd != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptimMacd);
            if (end->td_ptimpMacdParam != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptimpMacdParam);
            break;
        case TX_INDI_ID_MTM:
            if (end->td_ptimMtm != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptimMtm);
            if (end->td_ptimpMtmParam != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptimpMtmParam);
            break;
        case TX_INDI_ID_KDJ:
            if (end->td_ptikKdj != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptikKdj);
            if (end->td_ptikpKdjParam != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptikpKdjParam);
            break;
        case TX_INDI_ID_RSI:
            if (end->td_ptirRsi != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptirRsi);
            if (end->td_ptirpRsiParam != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptirpRsiParam);
            break;
        case TX_INDI_ID_ASI:
            if (end->td_ptiaAsi != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptiaAsi);
            if (end->td_ptiapAsiParam != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptiapAsiParam);
            break;
        case TX_INDI_ID_ATR:
            if (end->td_ptiaAtr != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptiaAtr);
            if (end->td_ptiapAtrParam != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptiapAtrParam);
            break;
        case TX_INDI_ID_OBV:
            if (end->td_ptioObv != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptioObv);
            if (end->td_ptiopObvParam != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptiopObvParam);
            break;
        default:
            break;
        }

        end --;
        index ++;
    }

    return u32Ret;
}

#if 0
u32 freeIndicatorParamById(tx_ds_t * buffer, olint_t total, olint_t id)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tx_ds_t * end = buffer + total - 1;
    olint_t index = 0;

    while (index < total)
    {
        switch (id)
        {
        case TX_INDI_ID_MACD:
            if (end->td_ptimpMacdParam != NULL)
                jf_jiukun_freeMemory((void **)&end->td_ptimpMacdParam);
            break;
        default:
            break;
        }

        end --;
        index ++;
    }

    return u32Ret;
}
#endif

/*------------------------------------------------------------------------------------------------*/


