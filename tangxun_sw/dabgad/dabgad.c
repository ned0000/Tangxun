/**
 *  @file dabgad.c
 *
 *  @brief Tangxun background activity daemon
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_process.h"
#include "jf_file.h"
#include "jf_mem.h"
#include "jf_network.h"

#include "dabgad.h"

/* --- private data structures ------------------------------------------------------------------ */

typedef struct
{
    olchar_t * id_pstrSettingFile;
    u32 id_u8Reserved[8];

    jf_network_chain_t * id_pjncChain;
    jf_network_utimer_t * id_putUtimer;

    boolean_t id_bDownloaded;
} internal_dabgad_t;

/*10 minutes*/
#define DOWNLOAD_TIMER_INTERVAL  600

/* --- private routine section ------------------------------------------------------------------ */
static u32 _startDownload(void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dabgad_t * pid;
    struct tm * ptm;
    time_t tCur;

    pid = (internal_dabgad_t *)pData;

    ol_printf("hello, world\n");

    tCur = time(NULL);
    ptm = localtime(&tCur);

    if (ptm->tm_hour < 16)
    {
        jf_network_addUtimerItem(
            pid->id_putUtimer, pid, DOWNLOAD_TIMER_INTERVAL, _startDownload, NULL);
        return u32Ret;
    }

    if (! pid->id_bDownloaded)
    {
        if ((ptm->tm_hour == 15) && (ptm->tm_min <= DOWNLOAD_TIMER_INTERVAL / 60))
        {
            pid->id_bDownloaded = TRUE;
        }
    }


    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */
u32 createDabgad(dabgad_t ** ppDabgad, dabgad_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dabgad_t * pid;
    olchar_t strExecutablePath[JF_LIMIT_MAX_PATH_LEN];

    jf_logger_logInfoMsg("create dabgad");

    u32Ret = jf_mem_calloc((void **)&pid, sizeof(internal_dabgad_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pid->id_pstrSettingFile = pdp->dp_pstrSettingFile;

        /*change the working directory*/
        jf_file_getDirectoryName(strExecutablePath, JF_LIMIT_MAX_PATH_LEN, pdp->dp_pstrCmdLine);
        if (strlen(strExecutablePath) > 0)
            u32Ret = jf_process_setCurrentWorkingDirectory(strExecutablePath);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_createChain(&pid->id_pjncChain);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_createUtimer(pid->id_pjncChain, &(pid->id_putUtimer));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_addUtimerItem(
            pid->id_putUtimer, pid, DOWNLOAD_TIMER_INTERVAL, _startDownload, NULL);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppDabgad = pid;
    else if (pid != NULL)
        destroyDabgad((dabgad_t **)&pid);

    return u32Ret;
}

u32 destroyDabgad(dabgad_t ** ppDabgad)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dabgad_t * pid;

    assert((ppDabgad != NULL) && (*ppDabgad != NULL));

    pid = (internal_dabgad_t *)*ppDabgad;

    if (pid->id_pjncChain != NULL)
        jf_network_destroyChain(&pid->id_pjncChain);

    jf_mem_free(ppDabgad);

    return u32Ret;
}

u32 startDabgad(dabgad_t * pDabgad)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dabgad_t * pid;

    assert(pDabgad != NULL);

    pid = (internal_dabgad_t *)pDabgad;

    u32Ret = jf_network_startChain(pid->id_pjncChain);

    return u32Ret;
}

u32 stopDabgad(dabgad_t * pDabgad)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dabgad_t * pid;

    assert(pDabgad != NULL);

    pid = (internal_dabgad_t *)pDabgad;

    u32Ret = jf_network_stopChain(pid->id_pjncChain);

    return u32Ret;
}

u32 setDefaultDabgadParam(dabgad_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    memset(pdp, 0, sizeof(dabgad_param_t));


    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


