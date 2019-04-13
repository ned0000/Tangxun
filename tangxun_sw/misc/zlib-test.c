/**
 *  @file zlib-test.c
 *
 *  @brief The test file for zlib library
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
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_file.h"

/* --- private data/data structure section ------------------------------------------------------ */
static boolean_t ls_bDecompress = FALSE;

static boolean_t ls_bCompress = FALSE;
static boolean_t ls_bDecompressBuffer = FALSE;
static boolean_t ls_bBestSpeed = FALSE;
static boolean_t ls_bBestCompression = FALSE;
static char * ls_pstrFile = NULL;

/* --- private routine section ------------------------------------------------------------------ */

static void _printUsage(void)
{
    printf("\
Usage: compress-test [-d file] [-s] [-c] [-e file] \n\
    [-T <trace level>] [-F <trace log file>] [-S <trace file size>]\n\
    [-d] decompress file.\n\
    [-e] decompress buffer.\n\
    [-s] best speed when compressing file\n\
    [-c] best compression when compressing file\n");

    printf("\n");
}

static u32 _parseCmdLineParam(int argc, char ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    int nOpt;

    while (((nOpt = getopt(argc, argv,
        "dsce:f:T:F:S:h")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'd':
            ls_bDecompress = TRUE;
            break;
        case 's':
            ls_bBestSpeed = TRUE;
            ls_bCompress = TRUE;
            break;
        case 'c':
            ls_bBestCompression = TRUE;
            ls_bCompress = TRUE;
            break;
        case 'e':
            ls_bDecompressBuffer = TRUE;
            ls_pstrFile = optarg;
            break;
        case '?':
        case 'h':
            _printUsage();
            exit(0);
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

int main(int argc, char ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (argc < 2)
    {
        _printUsage();
        u32Ret = JF_ERR_MISSING_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _parseCmdLineParam(argc, argv);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bDecompressBuffer)
        {

        }
        else if (ls_bCompress)
        {

        }
        else if (ls_bDecompress)
        {

        }
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        printf("Err (0x%X) %s\n", u32Ret, jf_err_getDescription(u32Ret));
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


