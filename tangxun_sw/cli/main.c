/**
 *  @file main.c
 *
 *  @brief The main file of CLI utility.
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
#include "jf_option.h"

#include "stocklist.h"
#include "tx_env.h"
#include "trade_persistency.h"
#include "damodel.h"
#include "clicmd.h"
#include "main.h"
#include "tx_rule.h"

/* --- private data/data structure section ------------------------------------------------------ */

static tx_cli_master_t ls_tcmCliMaster;

static const olchar_t * ls_pstrTxCliProgramName = "tx_cli";
static const olchar_t * ls_pstrTxCliVersion = "1.0.0";
static const olchar_t * ls_pstrTxCliBuildData = "7/21/2019";

/* --- private routine section ------------------------------------------------------------------ */

static void _printTxCliUsage(void)
{
    ol_printf("\
Usage: %s [logger options] \n\
logger options:\n\
    -T <0|1|2|3> the log level. 0: no log, 1: error only, 2: info, 3: all.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n",
           ls_pstrTxCliProgramName);

    ol_printf("\n");

}

static u32 _parseCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_clieng_init_param_t * pjcip, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "T:F:S:Oh")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printTxCliUsage();
            exit(0);
            break;
        case 'T':
            u32Ret = jf_option_getU8FromString(optarg, &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
            break;
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
            break;
        case 'S':
            u32Ret = jf_option_getS32FromString(optarg, &pjlip->jlip_sLogFile);
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
    jf_clieng_outputLine("Tangxun Command Line Interface (CLI) Utility");
    jf_clieng_outputLine("Version: %s Build Date: %s", ls_pstrTxCliVersion, ls_pstrTxCliBuildData);
    jf_clieng_outputLine("All rights reserved.");
    jf_clieng_outputLine("-------------------------------------------------------------");

    return u32Ret;
}

static u32 _initAndRunTxCli(tx_cli_master_t * ptcmCli, jf_clieng_init_param_t * pjcip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    da_cli_param_t dcpParam;

        u32Ret = jf_clieng_init(pjcip);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = initStockList();

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = tx_env_initPersistency();

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = initTradePersistency();

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = initDaModelFramework();

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = tx_rule_init();

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = addDaCmd(ptcmCli, &dcpParam);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_clieng_run();

        jf_clieng_fini();
        tx_env_finiPersistency();
        finiStockList();
        finiDaModelFramework();
        tx_rule_fini();

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_clieng_init_param_t jcip;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_pstrCallerName = "TX_CLI";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_bLogToFile = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;
    jlipParam.jlip_pstrLogFilePath = "tx_cli.log";

    ol_bzero(&jjip, sizeof(jf_jiukun_init_param_t));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    ol_bzero(&jcip, sizeof(jf_clieng_init_param_t));
    jcip.jcip_sMaxCmdLine = JF_CLIENG_MAX_COMMAND_LINE_SIZE;
    jcip.jcip_sCmdHistroyBuf = 20;
    ol_strcpy(jcip.jcip_strCliName, "Tangxun CLI");
    jcip.jcip_pstrNewLine = "\n";
    jcip.jcip_pMaster = &ls_tcmCliMaster;
    jcip.jcip_fnPrintGreeting = _printShellGreeting;

    u32Ret = _parseCmdLineParam(argc, argv, &jcip, &jlipParam);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_process_initSocket();
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_jiukun_init(&jjip);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = _initAndRunTxCli(&ls_tcmCliMaster, &jcip);

                jf_jiukun_fini();
            }

            jf_process_finiSocket();
        }

        jf_logger_fini();
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

