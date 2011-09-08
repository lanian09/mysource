/*
*  _  _   _  ____ ____    _   __     __ _    _    _    _____  __
* | || \ | ||  __|  _ \  / \  \ \   / // \  | |  | |  |  _\ \/ /
* | ||  \| || |__| |_) |/ _ \  \ \ / // _ \ | |  | |  | |__\  /
* | || \   ||  __|    // ___ \  \   // ___ \| |__| |__|  __ | |
* |_||_|\__||_|  |_|\_\_/   \_\  \_//_/   \_\____|____|____||_|
*
* Copyright 2004 Infravalley, Inc. All Rights Reserved
*
* ------------------------------------------------------------------------------
* MODULE NAME : srchmsg_proc.c
* DESCRIPTION :
* REVISION    : DATE       VER NAME                   DESCRIPTION
*               2004/06/08 1.0 poopee                 Created
* COMMENTS    :
*
*
* ------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include "srchmsg.h"
extern int              errno;

int srch_file(st_SrchInfo_t *pSrchInfo, FILE *Outfd);
int is_headlog(char *filename);
time_t get_ctime(char *filename);
int srch_msg(char *filename, FILE *OutFd, st_SrchInfo_t   *pSrchInfo);
int write_title(FILE *OutFd, st_SrchInfo_t *pSrchInfo);
//int write_msg(st_AAAREQ *pAAAReq, SRCH_INFO *pSRCHINFO);

extern char *str_time(time_t t);


void ConvertAllMsg(st_AAAREQ *pstAAA)
{
    int i = 0;
    long long tmpLongLong = 0;
    
    pstAAA->dUDRCount = CVT_INT_CP(pstAAA->dUDRCount);
    
    /* Start of ACCInfo */
    pstAAA->stInfo.uiUDRSeq = CVT_INT_CP(pstAAA->stInfo.uiUDRSeq);
    pstAAA->stInfo.uiTimeStamp = CVT_INT_CP(pstAAA->stInfo.uiTimeStamp);
    pstAAA->stInfo.uiAAAIP = CVT_INT_CP(pstAAA->stInfo.uiAAAIP);
    pstAAA->stInfo.uiKey = CVT_INT_CP(pstAAA->stInfo.uiKey);
    pstAAA->stInfo.uiFramedIP = CVT_INT_CP(pstAAA->stInfo.uiFramedIP);
    pstAAA->stInfo.uiNASIP = CVT_INT_CP(pstAAA->stInfo.uiNASIP);
    pstAAA->stInfo.uiPCFIP = CVT_INT_CP(pstAAA->stInfo.uiPCFIP);
    pstAAA->stInfo.uiHAIP = CVT_INT_CP(pstAAA->stInfo.uiHAIP);
    pstAAA->stInfo.uiRADIUSLen = CVT_INT_CP(pstAAA->stInfo.uiRADIUSLen);
    pstAAA->stInfo.uiSessContinue = CVT_INT_CP(pstAAA->stInfo.uiSessContinue);
    pstAAA->stInfo.uiBeginnigSess = CVT_INT_CP(pstAAA->stInfo.uiBeginnigSess);
    pstAAA->stInfo.dSvcOpt = CVT_INT_CP(pstAAA->stInfo.dSvcOpt);
    pstAAA->stInfo.dAcctStatType = CVT_INT_CP(pstAAA->stInfo.dAcctStatType);
    pstAAA->stInfo.dCompTunnelInd = CVT_INT_CP(pstAAA->stInfo.dCompTunnelInd);
    pstAAA->stInfo.dNumAct = CVT_INT_CP(pstAAA->stInfo.dNumAct);
    pstAAA->stInfo.dSvcType = CVT_INT_CP(pstAAA->stInfo.dSvcType);
    pstAAA->stInfo.dFwdFCHMux = CVT_INT_CP(pstAAA->stInfo.dFwdFCHMux);
    pstAAA->stInfo.dRevFCHMux = CVT_INT_CP(pstAAA->stInfo.dRevFCHMux);
    pstAAA->stInfo.dFwdTrafType = CVT_INT_CP(pstAAA->stInfo.dFwdTrafType);
    pstAAA->stInfo.dRevTrafType = CVT_INT_CP(pstAAA->stInfo.dRevTrafType);
    pstAAA->stInfo.dFCHSize = CVT_INT_CP(pstAAA->stInfo.dFCHSize);
    pstAAA->stInfo.dFwdFCHRC = CVT_INT_CP(pstAAA->stInfo.dFwdFCHRC);
    pstAAA->stInfo.dRevFCHRC = CVT_INT_CP(pstAAA->stInfo.dRevFCHRC);
    pstAAA->stInfo.dIPTech = CVT_INT_CP(pstAAA->stInfo.dIPTech);
    pstAAA->stInfo.dDCCHSize = CVT_INT_CP(pstAAA->stInfo.dDCCHSize);
    pstAAA->stInfo.dNASPort = CVT_INT_CP(pstAAA->stInfo.dNASPort);
    pstAAA->stInfo.dNASPortType = CVT_INT_CP(pstAAA->stInfo.dNASPortType);
    pstAAA->stInfo.dReleaseInd = CVT_INT_CP(pstAAA->stInfo.dReleaseInd);
    pstAAA->stInfo.dAcctInOct = CVT_INT_CP(pstAAA->stInfo.dAcctInOct);
    pstAAA->stInfo.dAcctOutOct = CVT_INT_CP(pstAAA->stInfo.dAcctOutOct);
    pstAAA->stInfo.dAcctInPkt = CVT_INT_CP(pstAAA->stInfo.dAcctInPkt);
    pstAAA->stInfo.dAcctOutPkt = CVT_INT_CP(pstAAA->stInfo.dAcctOutPkt);
    pstAAA->stInfo.uiEventTime = CVT_INT_CP(pstAAA->stInfo.uiEventTime);

    pstAAA->stInfo.uiActTime = CVT_INT_CP(pstAAA->stInfo.uiActTime);
    pstAAA->stInfo.uiAcctSessTime = CVT_INT_CP(pstAAA->stInfo.uiAcctSessTime);
    pstAAA->stInfo.uiAcctDelayTime = CVT_INT_CP(pstAAA->stInfo.uiAcctDelayTime);
    
    pstAAA->stInfo.dTermSDBOctCnt = CVT_INT_CP(pstAAA->stInfo.dTermSDBOctCnt);
    pstAAA->stInfo.dOrgSDBOctCnt = CVT_INT_CP(pstAAA->stInfo.dOrgSDBOctCnt);
    pstAAA->stInfo.dTermNumSDB = CVT_INT_CP(pstAAA->stInfo.dTermNumSDB);
    pstAAA->stInfo.dOrgNumSDB = CVT_INT_CP(pstAAA->stInfo.dOrgNumSDB);
    
    pstAAA->stInfo.dRcvHDLCOct = CVT_INT_CP(pstAAA->stInfo.dRcvHDLCOct);
    pstAAA->stInfo.dIPQoS = CVT_INT_CP(pstAAA->stInfo.dIPQoS);
    pstAAA->stInfo.dAirQoS = CVT_INT_CP(pstAAA->stInfo.dAirQoS);
    pstAAA->stInfo.dRPConnectID = CVT_INT_CP(pstAAA->stInfo.dRPConnectID);
    pstAAA->stInfo.dBadPPPFrameCnt = CVT_INT_CP(pstAAA->stInfo.dBadPPPFrameCnt);
    pstAAA->stInfo.dAcctAuth = CVT_INT_CP(pstAAA->stInfo.dAcctAuth);
    pstAAA->stInfo.dAcctTermCause = CVT_INT_CP(pstAAA->stInfo.dAcctTermCause);
    pstAAA->stInfo.dAlwaysOn = CVT_INT_CP(pstAAA->stInfo.dAlwaysOn);
    pstAAA->stInfo.dUserID = CVT_INT_CP(pstAAA->stInfo.dUserID);
    pstAAA->stInfo.dInMIPSigCnt = CVT_INT_CP(pstAAA->stInfo.dInMIPSigCnt);
    pstAAA->stInfo.dOutMIPSigCnt = CVT_INT_CP(pstAAA->stInfo.dOutMIPSigCnt);
    pstAAA->stInfo.dAcctInterim = CVT_INT_CP(pstAAA->stInfo.dAcctInterim);
    
    tmpLongLong = 0;
    tmpLongLong = pstAAA->stInfo.llAcctSessID;
    //fprintf(stdout, "Before Cvt tmp[%lld] stInfo.sessid[%lld]\n", tmpLongLong, pstAAA->stInfo.llAcctSessID);
    CVT_INT64_CP(&tmpLongLong, pstAAA->stInfo.llAcctSessID);
    pstAAA->stInfo.llAcctSessID = tmpLongLong;
    //fprintf(stdout, "After Cvt tmp[%lld] stInfo.sessid[%lld]\n", tmpLongLong, pstAAA->stInfo.llAcctSessID);
    //pstAAA->stInfo.llAcctSessID = CVT_INT64_CP(pstAAA->stInfo.llAcctSessID);
    
    tmpLongLong = 0;
    tmpLongLong = pstAAA->stInfo.llCorrelID;
    CVT_INT64_CP(&tmpLongLong, pstAAA->stInfo.llCorrelID);
    pstAAA->stInfo.llCorrelID = tmpLongLong;
    
    pstAAA->stInfo.uiRetryFlg = CVT_INT_CP(pstAAA->stInfo.uiRetryFlg);
    pstAAA->stInfo.dReserved = CVT_INT_CP(pstAAA->stInfo.dReserved);
    
    /* End of ACCInfo */
    
    
    /* Start of UDRInfo */
    for(i = 0; i<pstAAA->dUDRCount; i++)
    {
        //pstAAA->stUDRInfo[i].llAcctSessID = CVT_INT_CP(pstAAA->stUDRInfo[i].llAcctSessID);
        tmpLongLong = 0;
        tmpLongLong = pstAAA->stUDRInfo[i].llAcctSessID;
        CVT_INT64_CP(&tmpLongLong, pstAAA->stUDRInfo[i].llAcctSessID);
        pstAAA->stUDRInfo[i].llAcctSessID = tmpLongLong;
        
        pstAAA->stUDRInfo[i].dDataSvcType = CVT_INT_CP(pstAAA->stUDRInfo[i].dDataSvcType);
        pstAAA->stUDRInfo[i].uiTranID = CVT_INT_CP(pstAAA->stUDRInfo[i].uiTranID);
        pstAAA->stUDRInfo[i].tReqTime = CVT_INT_CP(pstAAA->stUDRInfo[i].tReqTime);
        pstAAA->stUDRInfo[i].tResTime = CVT_INT_CP(pstAAA->stUDRInfo[i].tResTime);
        pstAAA->stUDRInfo[i].tSessionTime = CVT_INT_CP(pstAAA->stUDRInfo[i].tSessionTime);
        pstAAA->stUDRInfo[i].uiDestIP = CVT_INT_CP(pstAAA->stUDRInfo[i].uiDestIP);
        pstAAA->stUDRInfo[i].dDestPort = CVT_INT_CP(pstAAA->stUDRInfo[i].dDestPort);
        pstAAA->stUDRInfo[i].dSrcPort = CVT_INT_CP(pstAAA->stUDRInfo[i].dSrcPort);
        pstAAA->stUDRInfo[i].dCType = CVT_INT_CP(pstAAA->stUDRInfo[i].dCType);
        pstAAA->stUDRInfo[i].dAppID = CVT_INT_CP(pstAAA->stUDRInfo[i].dAppID);
        pstAAA->stUDRInfo[i].dContentCode = CVT_INT_CP(pstAAA->stUDRInfo[i].dContentCode);
        pstAAA->stUDRInfo[i].dMethodType = CVT_INT_CP(pstAAA->stUDRInfo[i].dMethodType);
        pstAAA->stUDRInfo[i].dResultCode= CVT_INT_CP(pstAAA->stUDRInfo[i].dResultCode);
        pstAAA->stUDRInfo[i].dIPUpSize= CVT_INT_CP(pstAAA->stUDRInfo[i].dIPUpSize);
        pstAAA->stUDRInfo[i].dIPDownSize = CVT_INT_CP(pstAAA->stUDRInfo[i].dIPDownSize);
        pstAAA->stUDRInfo[i].dRetransInSize = CVT_INT_CP(pstAAA->stUDRInfo[i].dRetransInSize);
        pstAAA->stUDRInfo[i].dRetransOutSize = CVT_INT_CP(pstAAA->stUDRInfo[i].dRetransOutSize);
        pstAAA->stUDRInfo[i].dContentLen = CVT_INT_CP(pstAAA->stUDRInfo[i].dContentLen);
        pstAAA->stUDRInfo[i].dTranComplete = CVT_INT_CP(pstAAA->stUDRInfo[i].dTranComplete);
        pstAAA->stUDRInfo[i].dTranTermReason = CVT_INT_CP(pstAAA->stUDRInfo[i].dTranTermReason);
    }
    /* End of UDRInfo */
    
    return;

}



/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/

int srch_msg(char *filename, FILE *OutFd, st_SrchInfo_t   *pSrchInfo)
{
    int         fd, len, ret;
    int         tot_msg=0, tot_found=0, tot_written=0;
    char        tmp_filename[MAX_FILENAME_LEN+1], command[128], *ptr;

    st_MsgQ             qmsg;
    st_AAAREQ           stAAA;
    int                 iRet = 0;
    char                tmpStart[256];
    char                tmpStop[256];
    struct stat         statbuf;
    time_t              start_time;
    st_LogFileHead      file_head;
    st_LogDataHead2     data_head;
    st_DbFetchInfo_t    stDbFetch;
    st_LogParse_t       stLogParse;

    memset( &qmsg, 0x00, sizeof(st_MsgQ) );
    memset(&file_head, 0x00, sizeof(st_LogFileHead));
    memset(&data_head, 0x00, sizeof(st_LogDataHead2));
    memset(&stDbFetch, 0x00, sizeof(st_DbFetchInfo_t));
    memset(&stLogParse, 0x00, sizeof(st_LogParse_t));

    PASS_EXCEPTION(stat(filename,&statbuf) < 0, STAT_ERROR);
    

    len = strlen(filename);
    strcpy(tmp_filename,filename);

    PASS_EXCEPTION((fd=open(tmp_filename,O_RDONLY)) < 0, FILE_OPEN_ERR);
    /*
    PASS_EXCEPTION((len=read(fd,(void *)&file_head,sizeof(st_LogFileHead))) <= 0,
        FILE_READ_ERR);
    */
    if (pSrchInfo->iSrchType == SRCHTYPE_TIME)
    {
        PASS_EXCEPTION(statbuf.st_mtime < pSrchInfo->tStartTime,
            FILE_END_ERR);
        PASS_EXCEPTION((start_time = get_ctime(filename)) > pSrchInfo->tEndTime,
            FILE_START_ERR);
    }
    
    while(1)
    {
        memset(&data_head, 0x00, sizeof(st_LogDataHead2));
        memset(&stDbFetch, 0x00, sizeof(st_DbFetchInfo_t));
        memset(&stLogParse, 0x00, sizeof(st_LogParse_t));

        len = read(fd, (void *)&data_head, sizeof(st_LogDataHead2));
		
        if( len <= 0 )
        {
            if( len < 0 )
            {
                fprintf(stderr,"ERROR: file(%s) read error(%s)!!!\n",
                    filename,strerror(errno));
            }
            close(fd);
            break;

        }
        len = read(fd, (void *)&stDbFetch, sizeof(st_DbFetchInfo_t));
        if( pSrchInfo->iSrchType == SRCHTYPE_TIME )
        {
            if( (CVT_INT_CP(data_head.stTmval.tv_sec) < pSrchInfo->tStartTime)
                || (CVT_INT_CP(data_head.stTmval.tv_sec) > pSrchInfo->tEndTime))
            {
                continue;
            }
        }

        
        tot_msg++;
        
        ParsingDbData(stDbFetch.sTLOG, &stLogParse);

        switch (pSrchInfo->iSrchKeyType)
        {
            case SRCHKEY_LOGID:
				if (stDbFetch.iLogId == 0)
                {
					fprintf(stderr,"ERROR: MSID not exist in log header\n");
                }
				else if (stDbFetch.iLogId == pSrchInfo->uiLogId )
				{
                    tot_found++;
                    if ((ret = print_txnlog_msg(&stDbFetch, &stLogParse, OutFd, pSrchInfo, data_head.ucADR)) == 0)
                    {
                        tot_written++;
                    }
				}
                break;

            case SRCHKEY_ALL:
            default:

            tot_found++;
            if ((ret = print_txnlog_msg(&stDbFetch, &stLogParse, OutFd, pSrchInfo, data_head.ucADR)) == 0)
            {
                tot_written++;
            }
            break;

        }

    }


    close(fd);


    if ((ptr=strrchr(filename,'/')) == NULL)
    {
        ptr = filename;
    }
    else
    {
        ptr += 1;
    }
    fprintf(stderr,"     : %-15s   : MESSAGE=%d FOUND=%d WRITTEN=%d\n",
            ptr,tot_msg,tot_found,tot_written);           
    
    
    PASS_CATCH(STAT_ERROR)
    fprintf(stderr,"ERROR: stat(%s) error(%s)!!!\n",filename,strerror(errno));
    iRet = -1;
    
    PASS_CATCH(FILE_OPEN_ERR)
    fprintf(stderr,"ERROR: file(%s) open error(%s)!!!\n",tmp_filename,strerror(errno));
    iRet = -1;
    
    PASS_CATCH(FILE_READ_ERR)
    fprintf(stderr,"ERROR: file(%s) read error(%s)!!!\n",tmp_filename,strerror(errno));
    iRet = -1;
        
    PASS_CATCH(FILE_END_ERR)
    fprintf(stderr,"ERROR: filestat.st_mtime(%d) is Smaller than pSrchInfo->tStartTime(%d)!!\n",
        statbuf.st_mtime, pSrchInfo->tStartTime);
    iRet = -1;
    
    PASS_CATCH(FILE_START_ERR)
    fprintf(stderr,"ERROR: get_ctime(filename) (%d) is Bigger than pSrchInfo->tEndTime (%d)!!\n",
        start_time, pSrchInfo->tEndTime);
    iRet = -1;
    
    
    PASS_CATCH_END;

    return iRet;
}


/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
int write_title(FILE *OutFd, st_SrchInfo_t *pSrchInfo)
{
    char        buffer[1024*20];
    int         len;
    int         iRet = 0;
    
    sprintf(buffer,"LOG-ID; LOG-TIME; ");
    len = strlen(buffer);

    sprintf(&buffer[len],"FLAG; FAIL-REASON; ");
    len = strlen(buffer);

    sprintf(&buffer[len],"MIN; TERMINAL-IP-ADDRESS; TERMINAL-PORT; ");
    len = strlen(buffer);
    
    sprintf(&buffer[len],"SUB-NUMBER; REQUEST-TIME; RESPONSE-TIME; ");
    len = strlen(buffer);
    
    sprintf(&buffer[len],"WAP-REQUEST-SIZE; WAP-RESPONSE-SIZE; CONTENTS-LENGTH; ");
    len = strlen(buffer);

    sprintf(&buffer[len],"RESULT-CODE; STATUS; URL; ");
    len = strlen(buffer);
    
    sprintf(&buffer[len],"METHOD-TYPE; USER-AGENT; TLOG; \n");
    len = strlen(buffer);


    PASS_EXCEPTION(fprintf(OutFd,"%s",buffer) < 0, FWRITE_FAIL);
    

    PASS_CATCH(FWRITE_FAIL)
    fprintf(stderr,"ERROR: file(%s) write error(%s)!!!\n",
            pSrchInfo->szOutFile,strerror(errno));
    
    iRet = -1;
    
    PASS_CATCH_END;

    return iRet;
}


void print_hex_log(FILE* fp, char *buffer, int len)
{
    int     i;

    for (i=0; i<len; i++)
    {
        if (i % 10 == 0) fprintf(fp,"\n");
        fprintf(fp,"%02x ",0x000000FF & buffer[i]);
    }
    fprintf(fp,"\n");

    return;
}

/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
time_t get_ctime(char *filename)
{
    int         len, i, tmp_val;
    char        *ptr, tmp_str[5];
    struct tm   bdtime;
    struct tm   tmp_time;
    char        *tmp_filename = NULL;
    time_t      tmp_timet = 0;

    /* YYYYmmdd_HHMMSS */
    ptr = strrchr(filename,'/');
    ptr += 1;
    
    tmp_timet = time(NULL);

    localtime_r(&tmp_timet, &tmp_time);
    
    //tmp_filename = ptr + 8;
    tmp_filename = ptr;
    

    bdtime.tm_year = tmp_time.tm_year;

    memcpy(tmp_str,tmp_filename,2);    // month
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 1 || tmp_val > 12)
    {
        fprintf(stderr,"ERROR: invalid month value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_mon = tmp_val - 1;

    memcpy(tmp_str,tmp_filename+2,2);    // day
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 1 || tmp_val > 31)
    {
        fprintf(stderr,"ERROR: invalid day value(%d)!!!\n",tmp_val);
        return 0;
    }

    bdtime.tm_mday = tmp_val;

    return (mktime(&bdtime));

}

#if 0
int srch_file(st_SrchInfo_t *pSrchInfo)
{
    DIR         *dirp;
    struct dirent   *direntp;
    char        filename[MAX_FILENAME_LEN+1];
    int         iRet = 0;
    int         iFileCnt = 0;
    
    PASS_EXCEPTION((dirp=opendir(pSrchInfo->szDirectory)) == NULL, OPEN_DIR_FAIL);
    
    direntp = readdir(dirp);

    while ((direntp=readdir(dirp)) != 0)
    {
        if (!strcmp(direntp->d_name,".") || !strcmp(direntp->d_name,"..")) 
            continue;
        
        /*
        if (is_headlog(direntp->d_name) < 0) 
            continue;
        */
        snprintf(pSrchInfo->szFileInfo[iFileCnt++], MAX_FILENAME_LEN, "%s/%s",pSrchInfo->szDirectory,direntp->d_name);
    }

    pSrchInfo->iFileCnt = iFileCnt;
    
    PASS_CATCH(OPEN_DIR_FAIL)
    fprintf(stderr,"ERROR: directory(%s) open error(%s)!!!\n",pSrchInfo->szDirectory,strerror(errno));
    iRet = -1;

    PASS_CATCH_END;

    closedir(dirp);

    return iRet;
}
#endif
int srch_file(st_SrchInfo_t *pSrchInfo, FILE *Outfd)
{
    DIR         *dirp;
    struct dirent   *direntp;
    char        filename[MAX_FILENAME_LEN+1];
    int         iRet = 0;
    int         iFileCnt = 0;
    struct tm   tmStart, tmEnd, tmInc;
    char        startDate[15], endDate[15];
    char        searchDir[128];
    
    localtime_r(&pSrchInfo->tStartTime, &tmStart);
    localtime_r(&pSrchInfo->tEndTime, &tmEnd);
    
    sprintf(endDate, "%02d%02d", tmEnd.tm_mon+1, tmEnd.tm_mday);
    sprintf(startDate, "%02d%02d", tmStart.tm_mon+1, tmStart.tm_mday);
    
    sprintf(searchDir, "%s", "/BSD/LOG/HEADLOG/UAWAP_FAIL");
    dirp = opendir(searchDir);
        
    while( dirp && (direntp = readdir(dirp)) != 0 )
    {
        if (!strcmp(direntp->d_name,".") || !strcmp(direntp->d_name,"..")) 
            continue;
        if (strlen(direntp->d_name) != 4)
            continue;
        /*
        if ( strcmp(startDate, direntp->d_name) > 0 )
            continue;
        if ( strcmp(endDate, direntp->d_name) < 0 )
            break;
        */
        memset( filename, 0x00, sizeof(filename));
        sprintf(filename, "%s/%s", searchDir, direntp->d_name);
        srch_msg(filename, Outfd, pSrchInfo);
    }

    
    PASS_CATCH(OPEN_DIR_FAIL)
    fprintf(stderr,"ERROR: directory(%s) open error(%s)!!!\n",pSrchInfo->szDirectory,strerror(errno));
    iRet = -1;
    
    PASS_CATCH(STAT_ERROR)
    iRet = -1;

    PASS_CATCH_END;

    closedir(dirp);

    return iRet;
}



