/**
 *  @file main.c
 *
 *  @brief The main file of CLI utility
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <fcntl.h>
    #include <termios.h>
    #include <sys/ioctl.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_clieng.h"
#include "jf_jiukun.h"
#include "jf_mem.h"
#include "jf_network.h"
#include "jf_process.h"

#include "stocklist.h"
#include "envvar.h"
#include "trade_persistency.h"
#include "damodel.h"
#include "clicmd.h"
#include "main.h"

/* --- private data/data structure section ------------------------------------------------------ */
da_master_t * ls_pdmMaster = NULL;

static const olchar_t * ls_pstrProgramName = "oldata-analysis";
static const olchar_t * ls_pstrVersion = "1.0.0";
static const olchar_t * ls_pstrBuildData = "7/21/2018";

/* --- private routine section ------------------------------------------------------------------ */
static void _printUsage(void)
{
    ol_printf("\
Usage: %s [logger options] \n\
logger options:\n\
    -T <0|1|2|3> the log level. 0: no log, 1: error only, 2: info, 3: all.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n",
           ls_pstrProgramName);

    ol_printf("\n");

    exit(0);
}

static u32 _parseCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_clieng_init_param_t * pjcip, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv,
        "T:F:S:Oh")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            break;
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

static u32 _printShellGreeting(void * pMaster)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputLine("-------------------------------------------------------------");
    jf_clieng_outputLine("Data Analysis Command Line Interface (CLI) Utility");
    jf_clieng_outputLine("Version: %s Build Date: %s",
                     ls_pstrVersion, ls_pstrBuildData);
    jf_clieng_outputLine("All rights reserved.");
    jf_clieng_outputLine("-------------------------------------------------------------");

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_clieng_init_param_t jcip;
    jf_logger_init_param_t jlipParam;
    da_cli_param_t dcpParam;
    jf_jiukun_init_param_t jjip;

    u32Ret = jf_mem_calloc((void **)&ls_pdmMaster, sizeof(da_master_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
        jlipParam.jlip_pstrCallerName = "DA";
        jlipParam.jlip_bLogToStdout = TRUE;
        jlipParam.jlip_bLogToFile = TRUE;
        jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DATA;
        jlipParam.jlip_pstrLogFilePath = "cli.log";

        ol_memset(&jcip, 0, sizeof(jf_clieng_init_param_t));

        jcip.jcip_sMaxCmdLine = JF_CLIENG_MAX_COMMAND_LINE_SIZE;
        jcip.jcip_sCmdHistroyBuf = 20;
        ol_strcpy(jcip.jcip_strCliName, "TANGXUN-CLI");
        jcip.jcip_pstrNewLine = "\n";
        jcip.jcip_pMaster = ls_pdmMaster;
        jcip.jcip_fnPrintGreeting = _printShellGreeting;

        u32Ret = _parseCmdLineParam(argc, argv, &jcip, &jlipParam);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);
        jf_process_initSocket();

        ol_memset(&jjip, 0, sizeof(jf_jiukun_init_param_t));
        jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;
        jjip.jjip_bNoGrow = FALSE;

        u32Ret = jf_jiukun_init(&jjip);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_clieng_init(&jcip);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = initStockList();

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = initEnvPersistency();

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = initTradePersistency();

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = initDaModel();

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = addDaCmd(ls_pdmMaster, &dcpParam);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_clieng_run();

        jf_clieng_fini();
        finiEnvPersistency();
        finiStockList();
        jf_jiukun_fini();

        jf_process_finiSocket();
        jf_logger_fini();
    }

    if (ls_pdmMaster != NULL)
        jf_mem_free((void **)&ls_pdmMaster);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

