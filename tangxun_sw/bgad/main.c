/**
 *  @file main.c
 *
 *  @brief The main file of dabgad
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
#include "jf_process.h"
#include "jf_file.h"

#include "dabgad.h"

/* --- private data/data structure section ------------------------------------------------------ */
static dabgad_t * ls_pgDabgad = NULL;
static boolean_t ls_bForeground = FALSE;

static const olchar_t * ls_pstrProgramName = "dabgad";
static const olchar_t * ls_pstrVersion = "1.0.0";

#if defined(LINUX)
    #define SERVICE_RETURN_VALUE  int
    #define SERVICE_RETURN(value) return value
#elif defined(WINDOWS)
    #define SERVICE_RETURN_VALUE  void
    #define SERVICE_RETURN(value) return

    static u8 ls_u8ServiceName[] = "Jiufeng Service Manager";
    static SERVICE_STATUS ls_ssServiceStatus;
    static SERVICE_STATUS_HANDLE ls_sshServiceStatus;
#endif

/* --- private routine section ------------------------------------------------------------------ */
static void _printDabgadUsage(void)
{
    ol_printf("\
Usage: %s [-f] [-s setting file] [-V] [logger options]\n\
    -f ruuning in foreground.\n\
    -s setting file: specify the setting file.\n\
    -V show version information.\n\
logger options:\n\
    -T <0|1|2|3> the log level. 0: no log, 1: error only, 2: info, 3: all.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n",
           ls_pstrProgramName);

    ol_printf("\n");

    exit(0);
}

static u32 _parseDabgadCmdLineParam(
    olint_t argc, olchar_t ** argv, dabgad_param_t * pgp, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv,
        "fs:VT:F:S:Oh")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'f':
            ls_bForeground = TRUE;
            break;
        case 's':
            pgp->dp_pstrSettingFile = optarg;
            break;
        case '?':
        case 'h':
            _printDabgadUsage();
            break;
        case 'V':
            ol_printf("%s %s\n", ls_pstrProgramName, ls_pstrVersion);
            exit(0);
        case 'T':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                pjlip->jlip_u8TraceLevel = (u8)u32Value;
            else
                u32Ret = JF_ERR_INVALID_PARAM;
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
            break;
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
            break;
        case 'S':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                pjlip->jlip_sLogFile = u32Value;
            else
                u32Ret = JF_ERR_INVALID_PARAM;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static void _terminate(olint_t signal)
{
    ol_printf("get signal\n");

    if (ls_pgDabgad != NULL)
        stopDabgad(ls_pgDabgad);
}

static u32 _startDabgad(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = startDabgad(ls_pgDabgad);
    if (u32Ret != JF_ERR_NO_ERROR)
        jf_logger_logErrMsg(u32Ret, "quit dabgad");

    if (ls_pgDabgad != NULL)
        destroyDabgad(&ls_pgDabgad);

    jf_logger_fini();

    return u32Ret;
}

static u32 _initDabgad(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dabgad_param_t dp;
    jf_logger_init_param_t ehpParam;
    olchar_t strExecutable[100];

    memset(&ehpParam, 0, sizeof(jf_logger_init_param_t));
    ehpParam.jlip_pstrCallerName = "DABGAD";
    ehpParam.jlip_u8TraceLevel = 0;

    setDefaultDabgadParam(&dp);
    dp.dp_pstrCmdLine = argv[0];

    u32Ret = _parseDabgadCmdLineParam(argc, argv, &dp, &ehpParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (! ls_bForeground)
            u32Ret = jf_process_switchToDaemon();
    }        

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (jf_process_isAlreadyRunning(strExecutable))
        {
            fprintf(stderr, "another %s is ruuning\n", strExecutable);
            exit(-1);
        }
    }        

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&ehpParam);

        jf_file_getFileName(strExecutable, 100, argv[0]);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_process_registerSignalHandlers(_terminate);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = createDabgad(&ls_pgDabgad, &dp);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _initDabgad(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _startDabgad();

    return u32Ret;
}


/*------------------------------------------------------------------------------------------------*/


