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

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "fixdata.h"
#include "jf_err.h"
#include "jf_file.h"
#include "jf_string.h"
#include "jf_mem.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define SAVE_LINE      (0)
#define DELETE_LINE    (1)

/* --- private routine section ------------------------------------------------------------------ */
static u32 _saveOneLine(
    fix_param_t * pfp, olchar_t * line,
    olsize_t llen, olchar_t * time, jf_file_t fdt, fix_result_t * pResult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * result;
    jf_string_parse_result_field_t * field;
    olint_t index;
    olchar_t * strtime;
    olint_t action = SAVE_LINE;

    u32Ret = jf_string_parse(&result, line, 0, llen, "\t", 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = result->jspr_pjsprfFirst;
        index = 1;
        while ((field != NULL) && (u32Ret == JF_ERR_NO_ERROR))
        {
            if (index == 1)
            {
                /*time of the trade*/
                if (field->jsprf_sData < 8)
                {
                    u32Ret = JF_ERR_INVALID_DATA;
                }
                else
                {
                    strtime = field->jsprf_pstrData;
                    if (ol_strncmp(field->jsprf_pstrData, time, 8) > 0)
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

            field = field->jsprf_pjsprfNext;
            index ++;
        }

        jf_string_destroyParseResult(&result);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (action == SAVE_LINE)
            jf_file_writen(fdt, line, llen);
        else if (action == DELETE_LINE)
            pResult->fr_nDeletedLine ++;
    }

    return u32Ret;
}

static u32 _verifyFirstLine(
    fix_param_t * pfp, olchar_t * line, olsize_t size, olchar_t * time)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ol_strncmp(line, "14:55", 5) < 0)
        u32Ret = JF_ERR_INVALID_DATA;

    if (ol_strncmp(line, time, 8) > 0)
        u32Ret = JF_ERR_INVALID_DATA;

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 fixDataFile(olchar_t * file, fix_param_t * pfp, fix_result_t * pResult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_t fd;
    jf_file_t fdt;
    olchar_t line[512];
    olsize_t llen;
    olchar_t filename[JF_LIMIT_MAX_PATH_LEN];
    olchar_t strTime[16]; /*the time when the last transaction occurs*/

    ol_memset(pResult, 0, sizeof(*pResult));
    jf_file_getDirectoryName(line, sizeof(line), file);
    ol_snprintf(filename, JF_LIMIT_MAX_PATH_LEN, "%s.fix", file);

    u32Ret = jf_file_open(file, O_RDONLY, &fd);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = jf_file_open(filename, O_WRONLY | O_CREAT | O_TRUNC, &fdt);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_file_close(&fd);
        return u32Ret;
    }

    ol_strcpy(strTime, "15:01:00");

    llen = sizeof(line);
    u32Ret = jf_file_readLine(fd, line, &llen);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_file_writen(fdt, line, llen);

        llen = sizeof(line);
        u32Ret = jf_file_readLine(fd, line, &llen);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _verifyFirstLine(pfp, line, llen, strTime);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _saveOneLine(pfp, line, llen, strTime, fdt, pResult);

    while (u32Ret == JF_ERR_NO_ERROR)
    {
        llen = sizeof(line);
        u32Ret = jf_file_readLine(fd, line, &llen);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _saveOneLine(pfp, line, llen, strTime, fdt, pResult);
        }
    } 

    if (u32Ret == JF_ERR_END_OF_FILE)
        u32Ret = JF_ERR_NO_ERROR;

    jf_file_close(&fd);
    jf_file_close(&fdt);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pfp->fp_bOverwrite)
        {
            jf_file_remove(file);
            jf_file_rename(filename, file);
            jf_logger_logInfoMsg("Fix data file %s, overwrite it", file);
        }
        else
        {
            jf_logger_logInfoMsg(
                "Fix data file %s, save data to %s", file, filename);
        }
    }

    return u32Ret;
}


/*------------------------------------------------------------------------------------------------*/


