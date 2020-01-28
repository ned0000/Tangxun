/**
 *  @file main.c
 *
 *  @brief The main file of datransd
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
#include "jf_time.h"
#include "jf_jiukun.h"
#include "jf_network.h"
#include "jf_date.h"

#include "tx_stock.h"
#include "tx_config.h"

#include "datransd.h"

/* --- private data/data structure section ------------------------------------------------------ */
static datransd_t * ls_pgDatransd = NULL;
static boolean_t ls_bForeground = FALSE;

static const olchar_t * ls_pstrProgramName = "datransd";
static const olchar_t * ls_pstrVersion = "1.0.0";


/* --- private routine section ------------------------------------------------------------------ */
static void _printDatransdUsage(void)
{
    ol_printf("\
Usage: %s [-f] [-s setting file] [-V] [logger options]\n\
    -f ruuning in foreground.\n\
    -V show version information.\n\
logger options:\n\
    -T <0|1|2|3> the log level. 0: no log, 1: error only, 2: info, 3: all.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n",
           ls_pstrProgramName);

    ol_printf("\n");

    exit(0);
}

static u32 _parseDatransdCmdLineParam(
    olint_t argc, olchar_t ** argv, datransd_param_t * pdp, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv,
        "fVT:F:S:Oh")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'f':
            ls_bForeground = TRUE;
            break;
        case '?':
        case 'h':
            _printDatransdUsage();
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

    if (ls_pgDatransd != NULL)
        stopDatransd(ls_pgDatransd);
}

/* --- public routine section ------------------------------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_jiukun_init_param_t jjip;
    olint_t year, mon, day;
    olint_t dw;
    jf_logger_init_param_t ehpParam;
    datransd_param_t dp;
    olchar_t strExecutable[100];
    tx_stock_init_param_t tsipParam;

    jf_file_getFileName(strExecutable, 100, argv[0]);
    if (jf_process_isAlreadyRunning(strExecutable))
    {
        fprintf(stderr, "another %s is running\n", strExecutable);
        exit(-1);
    }

    jf_date_getDateToday(&year, &mon, &day);
    dw = 1; //getDayOfWeekFromDate(year, mon, day);
    if ((dw == 0) || (dw == 6))
    {
        /*Sunday and Saturday are not trading days */
        ol_printf("Today is not a trading day.\n");
        exit(-1);
    }

    memset(&ehpParam, 0, sizeof(jf_logger_init_param_t));
    ehpParam.jlip_pstrCallerName = "DATRANSD";
    ehpParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;
    ehpParam.jlip_bLogToStdout = TRUE;
    ehpParam.jlip_bLogToFile = TRUE;
    ehpParam.jlip_pstrLogFilePath = "datransd.log";

    setDefaultDatransdParam(&dp);
    dp.dp_pstrCmdLine = argv[0];

    u32Ret = _parseDatransdCmdLineParam(argc, argv, &dp, &ehpParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&ehpParam);
        jf_process_initSocket();

        memset(&jjip, 0, sizeof(jf_jiukun_init_param_t));
        jjip.jjip_sPool = 32 * 1024 * 1024;

        u32Ret = jf_jiukun_init(&jjip);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_bzero(&tsipParam, sizeof(tsipParam));
            tsipParam.tsip_pstrStockListFile = TX_CONFIG_STOCK_LIST_FILE;

            u32Ret = tx_stock_init(&tsipParam);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
//            if (! ls_bForeground)
//                u32Ret = switchToDaemon(strExecutable);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_process_registerSignalHandlers(_terminate);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = createDatransd(&ls_pgDatransd, &dp);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = startDatransd(ls_pgDatransd);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            sleep(3);
            destroyDatransd(&ls_pgDatransd);
        }

        tx_stock_fini();
        jf_jiukun_fini();

        jf_process_finiSocket();
        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        ol_printf("%s\n", jf_err_getDescription(u32Ret));
    }

    return 0;
}


/*------------------------------------------------------------------------------------------------*/


