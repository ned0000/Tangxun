/**
 *  @file fixdata.c
 *
 *  @brief routine for fix data, the data from files or ...
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "fixdata.h"
#include "errcode.h"
#include "files.h"
#include "stringparse.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */

#define SAVE_LINE      (0)
#define DELETE_LINE    (1)

/* --- private routine section---------------------------------------------- */
static u32 _saveOneLine(
    fix_param_t * pfp, olchar_t * line,
    olsize_t llen, olchar_t * time, file_t fdt, fix_result_t * pResult)
{
    u32 u32Ret = OLERR_NO_ERROR;
    parse_result_t * result;
    parse_result_field_t * field;
    olint_t index;
    olchar_t * strtime;
    olint_t action = SAVE_LINE;

    u32Ret = parseString(&result, line, 0, llen, "\t", 1);
    if (u32Ret == OLERR_NO_ERROR)
    {
        field = result->pr_pprfFirst;
        index = 1;
        while ((field != NULL) && (u32Ret == OLERR_NO_ERROR))
        {
            if (index == 1)
            {
                /*time of the trade*/
                if (field->prf_sData < 8)
                {
                    u32Ret = OLERR_INVALID_DATA;
                }
                else
                {
                    strtime = field->prf_pstrData;
                    if (strncmp(field->prf_pstrData, time, 8) > 0)
                        action = DELETE_LINE;
                    else
                        ol_strncpy(time, strtime, 8);
                }
            }
            else if (index == 2)
            {
                /*price of the trade*/
            }
            else if (index == 4)
            {
                /*volume*/
            }
            else if (index == 5)
            {
                /*amount*/
            }
            else if (index == 6)
            {
                /*op*/

            }

            field = field->prf_pprfNext;
            index ++;
        }

        destroyParseResult(&result);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (action == SAVE_LINE)
            writen(fdt, line, llen);
        else if (action == DELETE_LINE)
            pResult->fr_nDeletedLine ++;
    }

    return u32Ret;
}

static u32 _verifyFirstLine(
    fix_param_t * pfp, olchar_t * line, olsize_t size, olchar_t * time)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (strncmp(line, "14:55", 5) < 0)
        u32Ret = OLERR_INVALID_DATA;

    if (strncmp(line, time, 8) > 0)
        u32Ret = OLERR_INVALID_DATA;

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 fixDataFile(olchar_t * file, fix_param_t * pfp, fix_result_t * pResult)
{
    u32 u32Ret = OLERR_NO_ERROR;
    file_t fd;
    file_t fdt;
    olchar_t line[512];
    olsize_t llen;
    olchar_t filename[MAX_PATH_LEN];
    olchar_t strTime[16]; /*the time when the last transaction occurs*/

    ol_memset(pResult, 0, sizeof(*pResult));
    getDirectoryName(line, sizeof(line), file);
    ol_snprintf(filename, MAX_PATH_LEN, "%s.fix", file);

    u32Ret = openFile(file, O_RDONLY, &fd);
    if (u32Ret != OLERR_NO_ERROR)
        return u32Ret;

    u32Ret = openFile(filename, O_WRONLY | O_CREAT | O_TRUNC, &fdt);
    if (u32Ret != OLERR_NO_ERROR)
    {
        closeFile(&fd);
        return u32Ret;
    }

    ol_strcpy(strTime, "15:01:00");

    llen = sizeof(line);
    u32Ret = readLine(fd, line, &llen);
    if (u32Ret == OLERR_NO_ERROR)
    {
        writen(fdt, line, llen);

        llen = sizeof(line);
        u32Ret = readLine(fd, line, &llen);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _verifyFirstLine(pfp, line, llen, strTime);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _saveOneLine(pfp, line, llen, strTime, fdt, pResult);

    while (u32Ret == OLERR_NO_ERROR)
    {
        llen = sizeof(line);
        u32Ret = readLine(fd, line, &llen);
        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = _saveOneLine(pfp, line, llen, strTime, fdt, pResult);
        }
    } 

    if (u32Ret == OLERR_END_OF_FILE)
        u32Ret = OLERR_NO_ERROR;

    closeFile(&fd);
    closeFile(&fdt);

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (pfp->fp_bOverwrite)
        {
            removeFile(file);
            renameFile(filename, file);
            logInfoMsg("Fix data file %s, overwrite it", file);
        }
        else
        {
            logInfoMsg(
                "Fix data file %s, save data to %s", file, filename);
        }
    }

    return u32Ret;
}


/*---------------------------------------------------------------------------*/


