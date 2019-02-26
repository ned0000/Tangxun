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

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "logger.h"
#include "process.h"
#include "dabgad.h"
#include "files.h"
#include "xmalloc.h"
#include "network.h"

/* --- private data structures --------------------------------------------- */

typedef struct
{
    olchar_t * id_pstrSettingFile;
    u32 id_u8Reserved[8];

    basic_chain_t * id_pbcChain;
    utimer_t * id_putUtimer;

    boolean_t id_bDownloaded;
} internal_dabgad_t;

/*10 minutes*/
#define DOWNLOAD_TIMER_INTERVAL  600

/* --- private routine section --------------------------------------------- */
static u32 _startDownload(void * pData)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_dabgad_t * pid;
    struct tm * ptm;
    time_t tCur;

    pid = (internal_dabgad_t *)pData;

    ol_printf("hello, world\n");

    tCur = time(NULL);
    ptm = localtime(&tCur);

    if (ptm->tm_hour < 16)
    {
        addUtimerItem(
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

/* --- public routine section ---------------------------------------------- */
u32 createDabgad(dabgad_t ** ppDabgad, dabgad_param_t * pdp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_dabgad_t * pid;
    olchar_t strExecutablePath[MAX_PATH_LEN];

    logInfoMsg("create dabgad");

    u32Ret = xcalloc((void **)&pid, sizeof(internal_dabgad_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        pid->id_pstrSettingFile = pdp->dp_pstrSettingFile;

        /*change the working directory*/
        getDirectoryName(strExecutablePath, MAX_PATH_LEN, pdp->dp_pstrCmdLine);
        if (strlen(strExecutablePath) > 0)
            u32Ret = setCurrentWorkingDirectory(strExecutablePath);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = createBasicChain(&pid->id_pbcChain);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = createUtimer(pid->id_pbcChain, &(pid->id_putUtimer));

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = addUtimerItem(
            pid->id_putUtimer, pid, DOWNLOAD_TIMER_INTERVAL, _startDownload, NULL);

    if (u32Ret == OLERR_NO_ERROR)
        *ppDabgad = pid;
    else if (pid != NULL)
        destroyDabgad((dabgad_t **)&pid);

    return u32Ret;
}

u32 destroyDabgad(dabgad_t ** ppDabgad)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_dabgad_t * pid;

    assert((ppDabgad != NULL) && (*ppDabgad != NULL));

    pid = (internal_dabgad_t *)*ppDabgad;

    if (pid->id_pbcChain != NULL)
        destroyBasicChain(&pid->id_pbcChain);

    xfree(ppDabgad);

    return u32Ret;
}

u32 startDabgad(dabgad_t * pDabgad)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_dabgad_t * pid;

    assert(pDabgad != NULL);

    pid = (internal_dabgad_t *)pDabgad;

    u32Ret = startBasicChain(pid->id_pbcChain);

    return u32Ret;
}

u32 stopDabgad(dabgad_t * pDabgad)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_dabgad_t * pid;

    assert(pDabgad != NULL);

    pid = (internal_dabgad_t *)pDabgad;

    u32Ret = stopBasicChain(pid->id_pbcChain);

    return u32Ret;
}

u32 setDefaultDabgadParam(dabgad_param_t * pdp)
{
    u32 u32Ret = OLERR_NO_ERROR;

    memset(pdp, 0, sizeof(dabgad_param_t));


    return u32Ret;
}

/*---------------------------------------------------------------------------*/


