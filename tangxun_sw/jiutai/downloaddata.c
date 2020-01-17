/**
 *  @file downloaddata.c
 *
 *  @brief download data
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
#include "jf_listhead.h"
#include "jf_ipaddr.h"
#include "jf_file.h"
#include "jf_dir.h"
#include "jf_jiukun.h"
#include "jf_network.h"
#include "jf_httpparser.h"
#include "jf_string.h"
#include "jf_time.h"
#include "jf_date.h"
#include "jf_process.h"

#include "downloaddata.h"
#include "stocklist.h"

/* --- private data/data structure section ------------------------------------------------------ */
static olchar_t * ls_pstrDataServer = "market.finance.sina.com.cn";
static olchar_t * ls_pstrDataServerNetease = "quotes.money.163.com";
static olchar_t * ls_pstrDataFileNetease = "Summary.csv";
static olchar_t * ls_pstrStartData = "19980101";

#define DATE_STRING_FORMAT "%4d%02d%02d"

/* --- private routine section ------------------------------------------------------------------ */
static u32 _recvOneXls(
    jf_network_socket_t * sock, olchar_t * stock, olchar_t * date,
    olchar_t * recvdata, olsize_t * srecv, download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t buffer[2048];
    olsize_t len, sends;

    len = ol_snprintf(
        buffer, 2048,
        "GET /downxls.php?date=%s&symbol=%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.28) Gecko/20120306 Firefox/3.6.28\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Language: en-us,en;q=0.5\r\n"
        "Accept-Encoding: gzip,deflate\r\n"
        "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
        "Keep-Alive: 115\r\n"
        "Connection: close\r\n"
        "\r\n", date, stock, ls_pstrDataServer);

    sends = len;
    u32Ret = jf_network_send(sock, buffer, &sends);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_recvn(sock, recvdata, srecv);
        if (u32Ret == JF_ERR_SOCKET_PEER_CLOSED)
            u32Ret = JF_ERR_NO_ERROR;
    }

    if (u32Ret != JF_ERR_NO_ERROR)
        jf_logger_logErrMsg(u32Ret, "recv error");

    return u32Ret;
}

/*
 *  chunk-size\r\n
 *  chunk......\r\n
 *  chunk-size\r\n
 *  chunk......\r\n
 *  0\r\n\r\n
 *
 *  chunk-size is a hexadecimal string
 */
static u32 _writeChunkedData(jf_file_t fd, olchar_t * pBody, olsize_t sBody)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pChunk;
    olsize_t schunk, sfile = 0;

    do
    {
        u32Ret = jf_string_locateSubString(pBody, "\r\n", &pChunk);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_string_getS32FromHexString(pBody, pChunk - pBody, (s32 *)&schunk);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (schunk == 0)
                break;

            pChunk += 2;
            pBody = pChunk + schunk + 2;
            sfile += schunk;
            u32Ret = jf_file_writen(fd, pChunk, schunk);
        }
    } while (u32Ret == JF_ERR_NO_ERROR);

    if (sfile < 100)
        u32Ret = JF_ERR_INVALID_DATA;

    return u32Ret;
}

static u32 _saveOneXls(
    olchar_t * stock, olchar_t * fname,
    olchar_t * recvdata, olsize_t srecv, download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_httpparser_packet_header_t * pjhph = NULL;
    olchar_t filepath[JF_LIMIT_MAX_PATH_LEN];
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    olchar_t * pstrEncoding = "Transfer-Encoding";
    olchar_t * pstrContentLength = "Content-Length";
    jf_httpparser_packet_header_field_t * pjhphf;
    boolean_t bChunked = FALSE;
    olchar_t * pBody;
    olsize_t sheader, sbody;
    olint_t nContentLength = 0;

    jf_logger_logInfoMsg("recv %d", srecv);

    u32Ret = jf_string_locateSubString(recvdata, "\r\n\r\n", &pBody);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sheader = pBody - recvdata;
        sbody = srecv - sheader - 4;
        pBody += 4;
        if ((sheader <= 0) || (sbody <= 0))
            u32Ret = JF_ERR_INVALID_DATA;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logDataMsgWithAscii(
            (u8 *)recvdata, sheader, "HTTP Header");
        u32Ret = jf_httpparser_parsePacketHeader(&pjhph, recvdata, 0, sheader);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pjhph->jhph_nStatusCode != 200)
            u32Ret = JF_ERR_HTTP_STATUS_NOT_OK;
        else
        {
            if (jf_httpparser_getHeaderLine(
                    pjhph, pstrEncoding, ol_strlen(pstrEncoding), &pjhphf) ==
                JF_ERR_NO_ERROR)
            {
                if (ol_strcmp(pjhphf->jhphf_pstrData, "chunked") == 0)
                    bChunked = TRUE;
            }
            else if (jf_httpparser_getHeaderLine(
                         pjhph, pstrContentLength,
                         ol_strlen(pstrContentLength), &pjhphf) ==
                     JF_ERR_NO_ERROR)
            {
                u32Ret = jf_string_getS32FromString(
                    pjhphf->jhphf_pstrData, pjhphf->jhphf_sData, &nContentLength);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    if (nContentLength <= 100) /*can't be so little data*/
                    {
                        u32Ret = JF_ERR_INVALID_DATA;
                        jf_logger_logErrMsg(
                            u32Ret, "Invalid data for %s, stock %s",
                            fname, stock);
                    }
                }
                else
                {
                    jf_logger_logErrMsg(
                        u32Ret, "Invalid HTTP header for %s, stock %s",
                        fname, stock);
                }
            }
            else
                u32Ret = JF_ERR_INVALID_DATA;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logDebugMsg("Save trade detail %s for stock %s", fname, stock);
        ol_sprintf(
            filepath, "%s%c%s%c%s", param->ddp_pstrDataDir, PATH_SEPARATOR, stock,
            PATH_SEPARATOR, fname);

        u32Ret = jf_file_openWithMode(
            filepath, O_WRONLY | O_CREAT | O_TRUNC,
            JF_FILE_DEFAULT_CREATE_MODE, &fd);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (bChunked)
                u32Ret = _writeChunkedData(fd, pBody, sbody);
            else
                u32Ret = jf_file_writen(fd, pBody, sbody);

            jf_file_close(&fd);

            if (u32Ret != JF_ERR_NO_ERROR)
            {
                /*remove data file so we can try again later*/
                jf_file_remove(filepath);
                jf_logger_logErrMsg(u32Ret, "Remove data file %s", fname);
            }
        }
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_logger_logErrMsg(u32Ret, "Failed to save %s for stock %s", fname, stock);
        u32Ret = JF_ERR_NO_ERROR;
    }

    if (pjhph != NULL)
        jf_httpparser_destroyPacketHeader(&pjhph);

    return u32Ret;
}

/*create the directory if it's not existing*/
static u32 _createStockDir(olchar_t * stock, download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t dirname[JF_LIMIT_MAX_PATH_LEN];
    jf_dir_t * pDir;

    ol_sprintf(
        dirname, "%s%c%s", param->ddp_pstrDataDir, PATH_SEPARATOR, stock);

    u32Ret = jf_dir_open(dirname, &pDir);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*already there*/
        jf_dir_close(&pDir);
    }
    else
    {
        u32Ret = jf_dir_create(dirname, JF_DIR_DEFAULT_CREATE_MODE);
    }

    return u32Ret;
}

static boolean_t _skipDataDownload(
    olchar_t * stock, olchar_t * strfile, download_data_param_t * param)
{
    boolean_t bRet = FALSE;
    olchar_t filepath[JF_LIMIT_MAX_PATH_LEN];
    jf_file_stat_t filestat;

    if (param->ddp_bOverwrite)
        return bRet;

    ol_sprintf(
        filepath, "%s%c%s%c%s", param->ddp_pstrDataDir, PATH_SEPARATOR, stock,
        PATH_SEPARATOR, strfile);

    if (jf_file_getStat(filepath, &filestat) == JF_ERR_NO_ERROR)
        bRet = TRUE;

    return bRet;
}

static u32 _downloadOneXls(
    jf_ipaddr_t * serveraddr, olchar_t * stock, olchar_t * buf, olsize_t sbuf,
    download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ipaddr_t ipaddr;
    u16 u16Port;
    olint_t i, retry_count = 3;
    jf_network_socket_t * sock = NULL;
    olsize_t srecv;
    olchar_t strDate[64], strFile[96], strServer[64];
    olint_t syear, smonth, sday, eyear, emonth, eday;
    olint_t sdays, edays;
    olint_t dw;

    jf_ipaddr_getStringIpAddr(strServer, serveraddr);
    jf_date_getDate2FromString(param->ddp_pstrStartDate, &syear, &smonth, &sday);
    jf_date_getDate2FromString(param->ddp_pstrEndDate, &eyear, &emonth, &eday);

    sdays = jf_date_convertDateToDaysFrom1970(syear, smonth, sday);
    edays = jf_date_convertDateToDaysFrom1970(eyear, emonth, eday);

    while (sdays <= edays)
    {
        jf_date_convertDaysFrom1970ToDate(sdays, &syear, &smonth, &sday);
        jf_date_getStringDate2(strDate, syear, smonth, sday);
        ol_sprintf(strFile, "%s.xls", strDate);
        dw = jf_date_getDayOfWeekFromDate(syear, smonth, sday);
        if ((dw == 0) || (dw == 6))
        {
            /*Sunday and Saturday are not trading days */
            sdays ++;
            continue;
        }

        if ((u32Ret == JF_ERR_NO_ERROR) &&
            _skipDataDownload(stock, strFile, param))
        {
            /*the data file is already there and 'overwrite' option is not set*/
            sdays ++;
            continue;
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_ipaddr_setIpV4AddrToInaddrAny(&ipaddr);
            u16Port = 0;
            u32Ret = jf_network_createStreamSocket(&ipaddr, &u16Port, &sock);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*sometimes, it fails to connect to server. So try more times*/
            i = 0;
            while (i < retry_count)
            {
                u32Ret = jf_network_connectWithTimeout(sock, serveraddr, 80, 5);
                if (u32Ret == JF_ERR_NO_ERROR)
                    break;
                i ++;
                sleep(5);
            }
        }
        if (u32Ret == JF_ERR_NO_ERROR)
            jf_logger_logInfoMsg("Server %s connected", strServer);
        else
            jf_logger_logErrMsg(
                u32Ret, "Failed to connect to server %s, retry %d times", strServer, i);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            srecv = sbuf;
            u32Ret = _recvOneXls(
                sock, stock, strDate, buf, &srecv, param);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _saveOneXls(
                stock, strFile, buf, srecv, param);
        }

        if (sock != NULL)
            jf_network_destroySocket(&sock);

        sdays ++;
        jf_time_milliSleep(500); /*sleep 0.5 second or the server hate you, maybe*/
    }

    return u32Ret;
}

static u32 _downloadSinaXls(download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ipaddr_t serveraddr;
    olchar_t * recvdata = NULL;
    olsize_t srecv = 256 * 1024;
    struct hostent * servp;
    stock_info_t * stockinfo;

    u32Ret = jf_network_getHostByName(ls_pstrDataServer, &servp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ipaddr_setIpV4Addr(&serveraddr, *(long *) (servp->h_addr));
//        getIpAddrFromString("58.63.237.237", JF_IPADDR_TYPE_V4, &serveraddr);
        jf_jiukun_allocMemory((void **)&recvdata, srecv);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (param->ddp_pstrStock == NULL)
        {
            stockinfo = getFirstStockInfo();
            while ((stockinfo != NULL) && (u32Ret == JF_ERR_NO_ERROR))
            {
                u32Ret = _createStockDir(stockinfo->si_strCode, param);
                if (u32Ret == JF_ERR_NO_ERROR)
                    u32Ret = _downloadOneXls(
                        &serveraddr, stockinfo->si_strCode, recvdata, srecv, param);


                stockinfo = getNextStockInfo(stockinfo);
            }
        }
        else
        {
            u32Ret = getStockInfo(param->ddp_pstrStock, &stockinfo);
            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _createStockDir(param->ddp_pstrStock, param);

            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _downloadOneXls(
                    &serveraddr, param->ddp_pstrStock, recvdata, srecv, param);
        }
    }

    if (recvdata != NULL)
        jf_jiukun_freeMemory((void **)&recvdata);

    return u32Ret;
}

static u32 _recvOneCsv(
    jf_network_socket_t * sock, olchar_t * stock, olchar_t * startdate, olchar_t * enddate,
    olchar_t * recvdata, olsize_t * srecv, download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t buffer[2048];
    olsize_t len, sends;
    olchar_t code[16];
    olchar_t * pstrFields;

    if (ol_strncmp(stock, "sh", 2) == 0)
        ol_strcpy(code, "0");
    else
        ol_strcpy(code, "1");
    ol_strcat(code, stock + 2);

    if (isStockInfoIndex(stock))
    {
        pstrFields = "TCLOSE;HIGH;LOW;TOPEN;LCLOSE;CHG;PCHG;TURNOVER;VOTURNOVER;VATURNOVER";
    }
    else
    {
        pstrFields = "TCLOSE;HIGH;LOW;TOPEN;LCLOSE;CHG;PCHG;TURNOVER;VOTURNOVER;VATURNOVER;TCAP;MCAP";
    }

    len = ol_snprintf(
        buffer, 2048,
        "GET /service/chddata.html?code=%s&start=%s&end=%s&"
        "fields=%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 6.1; rv:12.0) Gecko/20100101 Firefox/12.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Language: en-us,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Connection: close\r\n"
        "\r\n", code, startdate, enddate, pstrFields, ls_pstrDataServerNetease);

    sends = len;
    u32Ret = jf_network_send(sock, buffer, &sends);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_recvnWithTimeout(sock, recvdata, srecv, 10);
        if (u32Ret == JF_ERR_SOCKET_PEER_CLOSED)
            u32Ret = JF_ERR_NO_ERROR;
    }

    if (u32Ret != JF_ERR_NO_ERROR)
        jf_logger_logErrMsg(u32Ret, "recv error");

    return u32Ret;
}

static u32 _writeChunkedCsvData(jf_file_t fd, olchar_t * pBody, olsize_t sBody)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pChunk, * pEnd;
    olsize_t schunk, sfile = 0;
    boolean_t bTitleSkipped = FALSE;

    do
    {
        u32Ret = jf_string_locateSubString(pBody, "\r\n", &pChunk);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_string_getS32FromHexString(pBody, pChunk - pBody, (s32 *)&schunk);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (schunk == 0)
                break;

            pChunk += 2;
            pBody = pChunk + schunk + 2;
            sfile += schunk;

            if (! bTitleSkipped)
            {
                pEnd = pChunk + schunk;
                while (pChunk < pEnd)
                {
                    if (*pChunk == '\n')
                    {
                        pChunk ++;
                        schunk = pEnd - pChunk;
                        break;
                    }
                    pChunk ++;
                } 

                bTitleSkipped = TRUE;
            }

            if (schunk > 0)
                u32Ret = jf_file_writen(fd, pChunk, schunk);
        }
    } while (u32Ret == JF_ERR_NO_ERROR);

    return u32Ret;
}

static u32 _writeCsvData(jf_file_t fd, olchar_t * pBody, olsize_t sBody)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pEnd;

    pEnd = pBody + sBody;
    while (pBody < pEnd)
    {
        if (*pBody == '\n')
        {
            pBody ++;
            break;
        }
        pBody ++;
    } 

    if (pBody < pEnd)
    {
        sBody = pEnd - pBody;

        u32Ret = jf_file_writen(fd, pBody, sBody);
    }

    return u32Ret;
}

static olsize_t _cvsChunkedDataReady(olchar_t * pBody, olsize_t sBody)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pChunk, * pEnd;
    olsize_t schunk, sfile = 0;
    boolean_t bTitleSkipped = FALSE;

    do
    {
        u32Ret = jf_string_locateSubString(pBody, "\r\n", &pChunk);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_string_getS32FromHexString(pBody, pChunk - pBody, (s32 *)&schunk);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (schunk == 0)
                break;

            pChunk += 2;
            pBody = pChunk + schunk + 2;

            if (! bTitleSkipped)
            {
                pEnd = pChunk + schunk;
                while (pChunk < pEnd)
                {
                    if (*pChunk == '\n')
                    {
                        pChunk ++;
                        schunk = pEnd - pChunk;
                        break;
                    }
                    pChunk ++;
                } 

                bTitleSkipped = TRUE;
            }
            
            sfile += schunk;
            if (sfile > 0)
                break;
        }
    } while (u32Ret == JF_ERR_NO_ERROR);

    return sfile;
}

static olsize_t _cvsDataReady(
    boolean_t bChunked, olint_t nContentLength, olchar_t * pBody, olsize_t sBody)
{
    olchar_t * pEnd;
    olsize_t sfile = 0;

    if (bChunked)
        sfile = _cvsChunkedDataReady(pBody, sBody);
    else if (nContentLength > 0)
    {
        pEnd = pBody + sBody;
        while (pBody < pEnd)
        {
            if (*pBody == '\n')
            {
                pBody ++;
                break;
            }
            pBody ++;
        } 

        if (pBody < pEnd)
        {
            sBody = pEnd - pBody;
            sfile = sBody;
        }
    }

    return sfile;
}

static u32 _saveOneCsv(
    olchar_t * stock, olchar_t * fname, olchar_t * origcsv, olsize_t sorig,
    olchar_t * recvdata, olsize_t srecv, download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_httpparser_packet_header_t * pjhph = NULL;
    olchar_t filepath[JF_LIMIT_MAX_PATH_LEN];
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    olchar_t * pstrEncoding = "Transfer-Encoding";
    olchar_t * pstrContentLength = "Content-Length";
    jf_httpparser_packet_header_field_t * pjhphf;
    boolean_t bChunked = FALSE;
    olchar_t * pBody;
    olsize_t sheader, sbody;
    olint_t nContentLength = 0;

    jf_logger_logInfoMsg("recv %d", srecv);

    u32Ret = jf_string_locateSubString(recvdata, "\r\n\r\n", &pBody);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sheader = pBody - recvdata;
        sbody = srecv - sheader - 4;
        pBody += 4;
        if ((sheader <= 0) || (sbody <= 0))
            u32Ret = JF_ERR_INVALID_DATA;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logDataMsgWithAscii(
            (u8 *)recvdata, sheader, "HTTP Header");
        u32Ret = jf_httpparser_parsePacketHeader(&pjhph, recvdata, 0, sheader);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pjhph->jhph_nStatusCode != 200)
        {
            u32Ret = JF_ERR_HTTP_STATUS_NOT_OK;
        }
        else
        {
            u32Ret = jf_httpparser_getHeaderLine(
                pjhph, pstrEncoding, ol_strlen(pstrEncoding), &pjhphf);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                if (ol_strncmp(pjhphf->jhphf_pstrData, "chunked", 7) == 0)
                    bChunked = TRUE;
            }
            else
            {
                u32Ret = jf_httpparser_getHeaderLine(
                    pjhph, pstrContentLength, ol_strlen(pstrContentLength), &pjhphf);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    u32Ret = jf_string_getS32FromString(
                        pjhphf->jhphf_pstrData, pjhphf->jhphf_sData, &nContentLength);
                    if (u32Ret == JF_ERR_NO_ERROR)
                    {
                        if (nContentLength <= 10) /*can't be so little data*/
                        {
                            u32Ret = JF_ERR_INVALID_DATA;
                            jf_logger_logErrMsg(
                                u32Ret, "Invalid data for %s, stock %s", fname, stock);
                        }
                    }
                    else
                    {
                        jf_logger_logErrMsg(
                            u32Ret, "Invalid HTTP header for %s, stock %s", fname, stock);
                    }
                }
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                if ((! bChunked) && (nContentLength == 0))
                {
                    u32Ret = JF_ERR_INVALID_DATA;
                }
            }
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logDebugMsg("Save trade summary for stock %s", stock);
        ol_sprintf(
            filepath, "%s%c%s%c%s", param->ddp_pstrDataDir, PATH_SEPARATOR, stock,
            PATH_SEPARATOR, fname);

        if (_cvsDataReady(bChunked, nContentLength, pBody, sbody) > 0)
        {
            u32Ret = jf_file_openWithMode(
                filepath, O_WRONLY | O_CREAT | O_TRUNC, JF_FILE_DEFAULT_CREATE_MODE, &fd);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                if (bChunked)
                    u32Ret = _writeChunkedCsvData(fd, pBody, sbody);
                else
                    u32Ret = _writeCsvData(fd, pBody, sbody);

                if ((u32Ret == JF_ERR_NO_ERROR) && (sorig > 0))
                    u32Ret = jf_file_writen(fd, origcsv, sorig);

                jf_file_close(&fd);

                if (u32Ret != JF_ERR_NO_ERROR)
                {
                    /*remove data file so we can try again later*/
                    jf_file_remove(filepath);
                    jf_logger_logErrMsg(u32Ret, "Remove trade summary file");
                }
            }
        }
    }

    if (pjhph != NULL)
        jf_httpparser_destroyPacketHeader(&pjhph);

    return u32Ret;
}

static u32 _getStartEndDate(
    stock_info_t * stockinfo, olchar_t * strFile, olchar_t * origcsv,
    olsize_t * rorig, olchar_t * startdate, olchar_t * enddate, boolean_t * pbUptodate,
    download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t filepath[JF_LIMIT_MAX_PATH_LEN];
    jf_file_t fd;
    olint_t year, mon, day, days;
    olchar_t date[16];
    olchar_t * stock = stockinfo->si_strCode;

    ol_strcpy(startdate, ls_pstrStartData);
    jf_date_getDateToday(&year, &mon, &day);
    ol_sprintf(enddate, DATE_STRING_FORMAT, year, mon, day);

    if (param->ddp_bOverwrite)
    {
        *rorig = 0;
        *pbUptodate = FALSE;
        return u32Ret;
    }

    if (stockinfo->si_bCsvUptodate)
    {
        *pbUptodate = TRUE;
        return u32Ret;
    }

    ol_sprintf(
        filepath, "%s%c%s%c%s", param->ddp_pstrDataDir, PATH_SEPARATOR, stock,
        PATH_SEPARATOR, strFile);

    u32Ret = jf_file_open(filepath, O_RDONLY, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_file_readn(fd, origcsv, rorig);
        if ((u32Ret == JF_ERR_NO_ERROR) && (*rorig > 10))
        {
            ol_strncpy(date, origcsv, 10);
            date[10] = '\0';
            jf_date_getDate2FromString(date, &year, &mon, &day);
            days = jf_date_convertDateToDaysFrom1970(year, mon, day);
            days ++;
            jf_date_convertDaysFrom1970ToDate(days, &year, &mon, &day);
            ol_sprintf(startdate, DATE_STRING_FORMAT, year, mon, day);
        }

        jf_file_close(&fd);
    }
    else
        *rorig = 0;

    if (ol_strcmp(startdate, enddate) > 0)
        *pbUptodate = TRUE;
/*
    else
    {
        getDateToday(&year, &mon, &day);
        days = convertDateToDaysFrom1970(year, mon, day);
        days ++;
        convertDaysFrom1970ToDate(days, &year, &mon, &day);
        ol_sprintf(enddate, DATE_STRING_FORMAT, year, mon, day);
    }
*/
    return u32Ret;
}

static u32 _downloadOneCsv(
    jf_ipaddr_t * serveraddr, stock_info_t * stockinfo, olchar_t * strFile,
    olchar_t * origcsv, olsize_t sorig, olchar_t * buf, olsize_t sbuf,
    download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i, retry_count = 3;
    jf_network_socket_t * sock = NULL;
    olsize_t srecv;
    olchar_t strServer[64];
    olchar_t startdate[16], enddate[16];
    olsize_t rorig;
    boolean_t bUptodate = FALSE;
    olchar_t * stock = stockinfo->si_strCode;

    jf_ipaddr_getStringIpAddr(strServer, serveraddr);

    rorig = sorig;
    _getStartEndDate(
        stockinfo, strFile, origcsv, &rorig, startdate, enddate, &bUptodate, param);
 
    if (bUptodate)
    {
        stockinfo->si_bCsvUptodate = TRUE;
//        cliengOutputLine("The CSV file for %s is up to date", stock);
        return u32Ret;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_createSocket(AF_INET, SOCK_STREAM, 0, &sock);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*sometimes, it fails to connect to server. So try more times*/
        i = 0;
        while (i < retry_count)
        {
            u32Ret = jf_network_connectWithTimeout(sock, serveraddr, 80, 5);
            if (u32Ret == JF_ERR_NO_ERROR)
                break;
            i ++;
            sleep(5);
        }
    }
    if (u32Ret == JF_ERR_NO_ERROR)
        jf_logger_logInfoMsg("Server %s connected", strServer);
    else
        jf_logger_logErrMsg(
            u32Ret, "Failed to connect to server %s, retry %d times",
            strServer, i);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(buf, 0, sbuf);
        srecv = sbuf;
        u32Ret = _recvOneCsv(
            sock, stock, startdate, enddate, buf, &srecv, param);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _saveOneCsv(
                stock, strFile, origcsv, rorig, buf, srecv, param);
        }

        if (u32Ret != JF_ERR_NO_ERROR)
        {
            jf_logger_logErrMsg(u32Ret, "Failed to save trade summary for stock %s", stock);
            u32Ret = JF_ERR_NO_ERROR;
        }
    }

    if (sock != NULL)
        jf_network_destroySocket(&sock);

    return u32Ret;
}

static u32 _downloadNeteaseCSV(download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ipaddr_t serveraddr;
    olchar_t * recvdata = NULL;
    olsize_t srecv = 1024 * 1024;
    olchar_t * origcsv = NULL;
    olsize_t sorig = 1024 * 1024;
    struct hostent * servp;
    stock_info_t * stockinfo;

    u32Ret = jf_network_getHostByName(ls_pstrDataServerNetease, &servp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ipaddr_setIpV4Addr(&serveraddr, *(long *) (servp->h_addr));
//        getIpAddrFromString("58.63.237.237", JF_IPADDR_TYPE_V4, &serveraddr);
        jf_jiukun_allocMemory((void **)&origcsv, sorig);
        jf_jiukun_allocMemory((void **)&recvdata, srecv);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (param->ddp_pstrStock == NULL)
        {
            stockinfo = getFirstStockInfo();
            while ((stockinfo != NULL) && (u32Ret == JF_ERR_NO_ERROR))
            {
                u32Ret = _createStockDir(stockinfo->si_strCode, param);
                if (u32Ret == JF_ERR_NO_ERROR)
                    u32Ret = _downloadOneCsv(
                        &serveraddr, stockinfo,
                        ls_pstrDataFileNetease, origcsv, sorig,
                        recvdata, srecv, param);

                jf_time_milliSleep(500);

                stockinfo = getNextStockInfo(stockinfo);
            }
        }
        else
        {
            u32Ret = getStockInfo(param->ddp_pstrStock, &stockinfo);
            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _createStockDir(param->ddp_pstrStock, param);

            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _downloadOneCsv(
                    &serveraddr, stockinfo,
                    ls_pstrDataFileNetease, origcsv, sorig,
                    recvdata, srecv, param);
        }
    }

    if (recvdata != NULL)
        jf_jiukun_freeMemory((void **)&recvdata);
    if (origcsv != NULL)
        jf_jiukun_freeMemory((void **)&origcsv);

    return u32Ret;
}

static u32 _checkDlDataParam(download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t year, month, day;

    if ((param->ddp_pstrDataDir == NULL) ||
        (param->ddp_pstrDataDir[0] == '\0'))
    {
        return JF_ERR_INVALID_PARAM;
    }

    if (param->ddp_bTradeDetail)
    {
        if ((param->ddp_pstrStartDate == NULL) || (param->ddp_pstrEndDate == NULL))
        {
            return JF_ERR_INVALID_PARAM;
        }

        u32Ret = jf_date_getDate2FromString(param->ddp_pstrStartDate, &year, &month, &day);
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_date_getDate2FromString(param->ddp_pstrEndDate, &year, &month, &day);
    }

    return u32Ret;
}

static u32 _downloadNeteaseIndexCSV(download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ipaddr_t serveraddr;
    olchar_t * recvdata = NULL;
    olsize_t srecv = 1024 * 1024;
    olchar_t * origcsv = NULL;
    olsize_t sorig = 1024 * 1024;
    struct hostent * servp;
    stock_info_t * stockinfo;

    u32Ret = jf_network_getHostByName(ls_pstrDataServerNetease, &servp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ipaddr_setIpV4Addr(&serveraddr, *(long *) (servp->h_addr));
//        getIpAddrFromString("58.63.237.237", JF_IPADDR_TYPE_V4, &serveraddr);
        jf_jiukun_allocMemory((void **)&origcsv, sorig);
        jf_jiukun_allocMemory((void **)&recvdata, srecv);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        stockinfo = getFirstStockInfoIndex();
        while ((stockinfo != NULL) && (u32Ret == JF_ERR_NO_ERROR))
        {
            u32Ret = _createStockDir(stockinfo->si_strCode, param);
            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _downloadOneCsv(
                    &serveraddr, stockinfo,
                    ls_pstrDataFileNetease, origcsv, sorig,
                    recvdata, srecv, param);

            jf_time_milliSleep(500);

            stockinfo = getNextStockInfoIndex(stockinfo);
        }
    }

    if (recvdata != NULL)
        jf_jiukun_freeMemory((void **)&recvdata);
    if (origcsv != NULL)
        jf_jiukun_freeMemory((void **)&origcsv);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */
u32 downloadData(download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _checkDlDataParam(param);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (param->ddp_bTradeSummary)
            u32Ret = _downloadNeteaseCSV(param);
        else if (param->ddp_bTradeDetail)
            u32Ret = _downloadSinaXls(param);
        else
            u32Ret = JF_ERR_INVALID_PARAM;
    }


    return u32Ret;
}

u32 downloadStockInfoIndex(download_data_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _downloadNeteaseIndexCSV(param);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


