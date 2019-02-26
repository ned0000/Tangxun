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

/* --- standard C lib header files ----------------------------------------- */
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

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "clieng.h"
#include "clicmd.h"
#include "main.h"
#include "jiukun.h"
#include "xmalloc.h"
#include "network.h"
#include "stocklist.h"
#include "envvar.h"
#include "trade_persistency.h"
#include "damodel.h"

/* --- private data/data structure section --------------------------------- */
da_master_t * ls_pdmMaster = NULL;

static const olchar_t * ls_pstrProgramName = "oldata-analysis";
static const olchar_t * ls_pstrVersion = "1.0.0";
static const olchar_t * ls_pstrBuildData = "7/21/2018";

/* --- private routine section---------------------------------------------- */
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

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv, 
    clieng_param_t * pcp, logger_param_t * plp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv,
        "T:F:S:Oh")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            break;
        case 'T':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                plp->lp_u8TraceLevel = (u8)u32Value;
            else
                u32Ret = OLERR_INVALID_PARAM;
            break;
        case 'F':
            plp->lp_bLogToFile = TRUE;
            plp->lp_pstrLogFilePath = optarg;
            break;
        case 'O':
            plp->lp_bLogToStdout = TRUE;
            break;
        case 'S':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                plp->lp_sLogFile = u32Value;
            else
                u32Ret = OLERR_INVALID_PARAM;
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _printShellGreeting(void * pMaster)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputLine("-------------------------------------------------------------");
    cliengOutputLine("Data Analysis Command Line Interface (CLI) Utility");
    cliengOutputLine("Version: %s Build Date: %s",
                     ls_pstrVersion, ls_pstrBuildData);
    cliengOutputLine("All rights reserved.");
    cliengOutputLine("-------------------------------------------------------------");

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    clieng_param_t cp;
    logger_param_t lpParam;
    da_cli_param_t dcpParam;
    jiukun_param_t jp;

    u32Ret = xcalloc((void **)&ls_pdmMaster, sizeof(da_master_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(&lpParam, 0, sizeof(logger_param_t));
        lpParam.lp_pstrCallerName = "DA";
        lpParam.lp_bLogToStdout = TRUE;
        lpParam.lp_bLogToFile = TRUE;
        lpParam.lp_u8TraceLevel = LOGGER_TRACE_DATA;
        lpParam.lp_pstrLogFilePath = "cli.log";

        memset(&cp, 0, sizeof(clieng_param_t));

        cp.cp_sMaxCmdLine = MAX_COMMAND_LINE_SIZE;
        cp.cp_sCmdHistroyBuf = 20;
        ol_strcpy(cp.cp_strCliName, "TANGXUN-CLI");
        cp.cp_pstrNewLine = "\n";
        cp.cp_pMaster = ls_pdmMaster;
        cp.cp_fnPrintGreeting = _printShellGreeting;

        u32Ret = _parseCmdLineParam(argc, argv, &cp, &lpParam);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        initLogger(&lpParam);
        initNetworkLib();

        memset(&jp, 0, sizeof(jiukun_param_t));
        jp.jp_sPool = MAX_JIUKUN_POOL_SIZE;
        jp.jp_bNoGrow = FALSE;

        u32Ret = initJiukun(&jp);

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = initClieng(&cp);

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = initStockList();

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = initEnvPersistency();

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = initTradePersistency();

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = initDaModel();

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = addDaCmd(ls_pdmMaster, &dcpParam);

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = runClieng();

        finiClieng();
        finiEnvPersistency();
        finiStockList();
        finiJiukun();

        finiNetworkLib();
        finiLogger();
    }

    if (ls_pdmMaster != NULL)
        xfree((void **)&ls_pdmMaster);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

