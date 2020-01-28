/**
 *  @file parsedata.c
 *
 *  @brief Routine for parsing data, the data from files.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <math.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_file.h"
#include "jf_filestream.h"
#include "jf_dir.h"
#include "jf_string.h"
#include "jf_clieng.h"
#include "jf_mem.h"
#include "jf_time.h"
#include "jf_jiukun.h"

#include "tx_daysummary.h"
#include "tx_quo.h"
#include "tx_sector.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    tx_sector_info_t * sector;
    olint_t maxsector;
    olint_t numofsector;
    olchar_t * data;
    olsize_t sdata;
} tmp_sector_file_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _parseSectorFile(
    const olchar_t * pstrFullpath, olchar_t * buf, olsize_t sbuf, tmp_sector_file_t * ptsf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    olint_t total;
    olchar_t * line, *start, * end;
    olchar_t stocklist[2048];
    tx_sector_info_t * ptsi;

    stocklist[0] = '\0';
//    total = 0;
    start = line = buf;
    end = buf + sbuf;
    while (start < end)
    {
        if (*start == '\r')
        {
            strncat(stocklist, line, 8);
            ol_strcat(stocklist, ",");

            line = start + 1;
        }

        start ++;
    }

    jf_string_lower(stocklist);
    ptsi = &ptsf->sector[ptsf->numofsector];
    u32Ret = jf_string_duplicate(&ptsi->tsi_pstrStocks, stocklist);
    if (u32Ret == JF_ERR_NO_ERROR)
        jf_file_getFileName(ptsi->tsi_strName, sizeof(ptsi->tsi_strName), pstrFullpath);

    if (u32Ret == JF_ERR_NO_ERROR)
        ptsf->numofsector ++;

    return u32Ret;
}

static u32 _handleSectorFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tmp_sector_file_t * ptsf = (tmp_sector_file_t *)pArg;
    olsize_t sread;
    jf_file_t fd;

    if (ptsf->numofsector == ptsf->maxsector)
        return u32Ret;

    u32Ret = jf_file_open(pstrFullpath, O_RDONLY, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sread = ptsf->sdata;
        u32Ret = jf_file_readn(fd, ptsf->data, &sread);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            _parseSectorFile(
                pstrFullpath, ptsf->data, sread, ptsf);
        }

        jf_file_close(&fd);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 tx_sector_parseDir(
    olchar_t * pstrDir, tx_sector_info_t * sector, olint_t * numofsector)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    tmp_sector_file_t tsf;
#define MAX_SECTOR_FILE_SIZE  (32 * 1024)

    ol_bzero(&tsf, sizeof(tsf));
    tsf.sector = sector;
    tsf.maxsector = *numofsector;
    tsf.sdata = MAX_SECTOR_FILE_SIZE;

    jf_jiukun_allocMemory((void **)&tsf.data, tsf.sdata);

    u32Ret = jf_dir_parse(pstrDir, _handleSectorFile, &tsf);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *numofsector = tsf.numofsector;
    }

    jf_jiukun_freeMemory((void **)&tsf.data);

    return u32Ret;
}

void tx_sector_freeSectorInfo(tx_sector_info_t * sector)
{
    if (sector->tsi_pstrStocks != NULL)
        jf_string_free(&sector->tsi_pstrStocks);
}

/*------------------------------------------------------------------------------------------------*/


