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

#define UDR_FILE_LEN 36
#define UDR_GZFILE_LEN 39


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
	int	tmpInt = 0;
    
	//fprintf(stderr,"### POOPEE %s ",pstAAA->stInfo.szMIN);
    tmpInt = CVT_INT_CP(pstAAA->dUDRCount);
	//fprintf(stderr,"CNT %d ",tmpInt);
    pstAAA->dUDRCount = tmpInt;
    
    pstAAA->dReserved = CVT_INT_CP(pstAAA->dReserved);
	//fprintf(stderr,"%d ",pstAAA->dReserved);
    /* Start of ACCInfo */
    pstAAA->stInfo.uiUDRSeq = CVT_INT_CP(pstAAA->stInfo.uiUDRSeq);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiUDRSeq);
    pstAAA->stInfo.uiTimeStamp = CVT_INT_CP(pstAAA->stInfo.uiTimeStamp);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiTimeStamp);
    pstAAA->stInfo.uiAAAIP = CVT_INT_CP(pstAAA->stInfo.uiAAAIP);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiAAAIP);
    pstAAA->stInfo.uiKey = CVT_INT_CP(pstAAA->stInfo.uiKey);
	//fprintf(stderr,"2 %d ",pstAAA->stInfo.uiKey);
    pstAAA->stInfo.uiFramedIP = CVT_INT_CP(pstAAA->stInfo.uiFramedIP);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiFramedIP);
    pstAAA->stInfo.uiNASIP = CVT_INT_CP(pstAAA->stInfo.uiNASIP);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiNASIP);
    pstAAA->stInfo.uiPCFIP = CVT_INT_CP(pstAAA->stInfo.uiPCFIP);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiPCFIP);
    pstAAA->stInfo.uiHAIP = CVT_INT_CP(pstAAA->stInfo.uiHAIP);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiHAIP);
    pstAAA->stInfo.uiRADIUSLen = CVT_INT_CP(pstAAA->stInfo.uiRADIUSLen);
	//fprintf(stderr,"3 %d ",pstAAA->stInfo.uiRADIUSLen);
    pstAAA->stInfo.uiSessContinue = CVT_INT_CP(pstAAA->stInfo.uiSessContinue);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiSessContinue);
    pstAAA->stInfo.uiBeginnigSess = CVT_INT_CP(pstAAA->stInfo.uiBeginnigSess);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiBeginnigSess);
    pstAAA->stInfo.dSvcOpt = CVT_INT_CP(pstAAA->stInfo.dSvcOpt);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dSvcOpt);
    pstAAA->stInfo.dAcctStatType = CVT_INT_CP(pstAAA->stInfo.dAcctStatType);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dAcctStatType);
    pstAAA->stInfo.dCompTunnelInd = CVT_INT_CP(pstAAA->stInfo.dCompTunnelInd);
	//fprintf(stderr,"4 %d ",pstAAA->stInfo.dCompTunnelInd);
    pstAAA->stInfo.dNumAct = CVT_INT_CP(pstAAA->stInfo.dNumAct);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dNumAct);
    pstAAA->stInfo.dSvcType = CVT_INT_CP(pstAAA->stInfo.dSvcType);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dSvcType);
    pstAAA->stInfo.dFwdFCHMux = CVT_INT_CP(pstAAA->stInfo.dFwdFCHMux);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dFwdFCHMux);
    pstAAA->stInfo.dRevFCHMux = CVT_INT_CP(pstAAA->stInfo.dRevFCHMux);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dRevFCHMux);
    pstAAA->stInfo.dFwdTrafType = CVT_INT_CP(pstAAA->stInfo.dFwdTrafType);
	//fprintf(stderr,"5 %d ",pstAAA->stInfo.dFwdTrafType);
    pstAAA->stInfo.dRevTrafType = CVT_INT_CP(pstAAA->stInfo.dRevTrafType);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dRevTrafType);
    pstAAA->stInfo.dFCHSize = CVT_INT_CP(pstAAA->stInfo.dFCHSize);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dFCHSize);
    pstAAA->stInfo.dFwdFCHRC = CVT_INT_CP(pstAAA->stInfo.dFwdFCHRC);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dFwdFCHRC);
    pstAAA->stInfo.dRevFCHRC = CVT_INT_CP(pstAAA->stInfo.dRevFCHRC);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dRevFCHRC);
    pstAAA->stInfo.dIPTech = CVT_INT_CP(pstAAA->stInfo.dIPTech);
	//fprintf(stderr,"6 %d ",pstAAA->stInfo.dIPTech);
    pstAAA->stInfo.dDCCHSize = CVT_INT_CP(pstAAA->stInfo.dDCCHSize);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dDCCHSize);
    pstAAA->stInfo.dNASPort = CVT_INT_CP(pstAAA->stInfo.dNASPort);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dNASPort);
    pstAAA->stInfo.dNASPortType = CVT_INT_CP(pstAAA->stInfo.dNASPortType);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dNASPortType);
    pstAAA->stInfo.dReleaseInd = CVT_INT_CP(pstAAA->stInfo.dReleaseInd);
	//fprintf(stderr,"7 %d ", pstAAA->stInfo.dReleaseInd);
    pstAAA->stInfo.dAcctInOct = CVT_INT_CP(pstAAA->stInfo.dAcctInOct);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dAcctInOct);
    pstAAA->stInfo.dAcctOutOct = CVT_INT_CP(pstAAA->stInfo.dAcctOutOct);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dAcctOutOct);
    pstAAA->stInfo.dAcctInPkt = CVT_INT_CP(pstAAA->stInfo.dAcctInPkt);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dAcctInPkt);
    pstAAA->stInfo.dAcctOutPkt = CVT_INT_CP(pstAAA->stInfo.dAcctOutPkt);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dAcctOutPkt);
    pstAAA->stInfo.uiEventTime = CVT_INT_CP(pstAAA->stInfo.uiEventTime);
	//fprintf(stderr,"8 %d ",pstAAA->stInfo.uiEventTime);

    pstAAA->stInfo.uiActTime = CVT_INT_CP(pstAAA->stInfo.uiActTime);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiActTime);
    pstAAA->stInfo.uiAcctSessTime = CVT_INT_CP(pstAAA->stInfo.uiAcctSessTime);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiAcctSessTime);
    pstAAA->stInfo.uiAcctDelayTime = CVT_INT_CP(pstAAA->stInfo.uiAcctDelayTime);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiAcctDelayTime);
    
    pstAAA->stInfo.dTermSDBOctCnt = CVT_INT_CP(pstAAA->stInfo.dTermSDBOctCnt);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dTermSDBOctCnt);
    pstAAA->stInfo.dOrgSDBOctCnt = CVT_INT_CP(pstAAA->stInfo.dOrgSDBOctCnt);
	//fprintf(stderr,"9 %d ",pstAAA->stInfo.dOrgSDBOctCnt);
    pstAAA->stInfo.dTermNumSDB = CVT_INT_CP(pstAAA->stInfo.dTermNumSDB);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dTermNumSDB);
    pstAAA->stInfo.dOrgNumSDB = CVT_INT_CP(pstAAA->stInfo.dOrgNumSDB);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dOrgNumSDB);
    
    pstAAA->stInfo.dRcvHDLCOct = CVT_INT_CP(pstAAA->stInfo.dRcvHDLCOct);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dRcvHDLCOct);
    pstAAA->stInfo.dIPQoS = CVT_INT_CP(pstAAA->stInfo.dIPQoS);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dIPQoS);
    pstAAA->stInfo.dAirQoS = CVT_INT_CP(pstAAA->stInfo.dAirQoS);
	//fprintf(stderr,"10 %d ",pstAAA->stInfo.dAirQoS);
    pstAAA->stInfo.dRPConnectID = CVT_INT_CP(pstAAA->stInfo.dRPConnectID);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dRPConnectID);
    pstAAA->stInfo.dBadPPPFrameCnt = CVT_INT_CP(pstAAA->stInfo.dBadPPPFrameCnt);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dBadPPPFrameCnt);
    pstAAA->stInfo.dAcctAuth = CVT_INT_CP(pstAAA->stInfo.dAcctAuth);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dAcctAuth);
    pstAAA->stInfo.dAcctTermCause = CVT_INT_CP(pstAAA->stInfo.dAcctTermCause);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dAcctTermCause);
    pstAAA->stInfo.dAlwaysOn = CVT_INT_CP(pstAAA->stInfo.dAlwaysOn);
	//fprintf(stderr,"11 %d ",pstAAA->stInfo.dAlwaysOn);
    pstAAA->stInfo.dUserID = CVT_INT_CP(pstAAA->stInfo.dUserID);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dUserID);
    pstAAA->stInfo.dInMIPSigCnt = CVT_INT_CP(pstAAA->stInfo.dInMIPSigCnt);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dInMIPSigCnt);
    pstAAA->stInfo.dOutMIPSigCnt = CVT_INT_CP(pstAAA->stInfo.dOutMIPSigCnt);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dOutMIPSigCnt);
    pstAAA->stInfo.dAcctInterim = CVT_INT_CP(pstAAA->stInfo.dAcctInterim);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dAcctInterim);
    
    tmpLongLong = 0;
    tmpLongLong = pstAAA->stInfo.llAcctSessID;
    //fprintf(stdout, "Before Cvt tmp[%lld] stInfo.sessid[%lld]\n", tmpLongLong, pstAAA->stInfo.llAcctSessID);
    CVT_INT64_CP(&tmpLongLong, pstAAA->stInfo.llAcctSessID);
    pstAAA->stInfo.llAcctSessID = tmpLongLong;
	//fprintf(stderr,"12 %llx ",pstAAA->stInfo.llAcctSessID);
    //fprintf(stdout, "After Cvt tmp[%lld] stInfo.sessid[%lld]\n", tmpLongLong, pstAAA->stInfo.llAcctSessID);
    //pstAAA->stInfo.llAcctSessID = CVT_INT64_CP(pstAAA->stInfo.llAcctSessID);
    
    tmpLongLong = 0;
    tmpLongLong = pstAAA->stInfo.llCorrelID;
    CVT_INT64_CP(&tmpLongLong, pstAAA->stInfo.llCorrelID);
    pstAAA->stInfo.llCorrelID = tmpLongLong;
	//fprintf(stderr,"%llx ",pstAAA->stInfo.llCorrelID);
    
    pstAAA->stInfo.uiRetryFlg = CVT_INT_CP(pstAAA->stInfo.uiRetryFlg);
	//fprintf(stderr,"%d ",pstAAA->stInfo.uiRetryFlg);
    pstAAA->stInfo.dReserved = CVT_INT_CP(pstAAA->stInfo.dReserved);
	//fprintf(stderr,"%d ",pstAAA->stInfo.dReserved);
    
    pstAAA->stInfo.uiC23BIT = CVT_INT_CP(pstAAA->stInfo.uiC23BIT);
	//fprintf(stderr,"%d\n",pstAAA->stInfo.uiC23BIT);
    
	// 080220, poopee, HBIT
    pstAAA->stInfo.uiHBIT = CVT_INT_CP(pstAAA->stInfo.uiHBIT);
	//fprintf(stderr,"%d\n",pstAAA->stInfo.uiHBIT);

    /* End of ACCInfo */
    
   	// 071009, poopee 
	if (pstAAA->dUDRCount >= 2)
	{
		fprintf(stderr,"### IMSI=%s, dUDRCount=%d\n",
			pstAAA->stInfo.szMIN, pstAAA->dUDRCount);
		fflush(stderr);
		pstAAA->dUDRCount = 1;
	}

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
        /*
		pstAAA->stUDRInfo[i].dAppID = CVT_INT_CP(pstAAA->stUDRInfo[i].dAppID);
        pstAAA->stUDRInfo[i].dContentCode = CVT_INT_CP(pstAAA->stUDRInfo[i].dContentCode);
        */
		pstAAA->stUDRInfo[i].dMethodType = CVT_INT_CP(pstAAA->stUDRInfo[i].dMethodType);
        pstAAA->stUDRInfo[i].dResultCode= CVT_INT_CP(pstAAA->stUDRInfo[i].dResultCode);
        pstAAA->stUDRInfo[i].dIPUpSize= CVT_INT_CP(pstAAA->stUDRInfo[i].dIPUpSize);
        pstAAA->stUDRInfo[i].dIPDownSize = CVT_INT_CP(pstAAA->stUDRInfo[i].dIPDownSize);
        pstAAA->stUDRInfo[i].dRetransInSize = CVT_INT_CP(pstAAA->stUDRInfo[i].dRetransInSize);
        pstAAA->stUDRInfo[i].dRetransOutSize = CVT_INT_CP(pstAAA->stUDRInfo[i].dRetransOutSize);
        pstAAA->stUDRInfo[i].dContentLen = CVT_INT_CP(pstAAA->stUDRInfo[i].dContentLen);
        pstAAA->stUDRInfo[i].dTranComplete = CVT_INT_CP(pstAAA->stUDRInfo[i].dTranComplete);
        pstAAA->stUDRInfo[i].dTranTermReason = CVT_INT_CP(pstAAA->stUDRInfo[i].dTranTermReason);
        pstAAA->stUDRInfo[i].dCPCode = CVT_INT_CP(pstAAA->stUDRInfo[i].dCPCode);
        pstAAA->stUDRInfo[i].dUseCount = CVT_INT_CP(pstAAA->stUDRInfo[i].dUseCount);
        pstAAA->stUDRInfo[i].dUseTime = CVT_INT_CP(pstAAA->stUDRInfo[i].dUseTime);
        pstAAA->stUDRInfo[i].dTotalSize = CVT_INT_CP(pstAAA->stUDRInfo[i].dTotalSize);
        pstAAA->stUDRInfo[i].dTotalTime = CVT_INT_CP(pstAAA->stUDRInfo[i].dTotalTime);
        pstAAA->stUDRInfo[i].dAudioInputIPSize = CVT_INT_CP(pstAAA->stUDRInfo[i].dAudioInputIPSize);
        pstAAA->stUDRInfo[i].dAudioOutputIPSize = CVT_INT_CP(pstAAA->stUDRInfo[i].dAudioOutputIPSize);
        pstAAA->stUDRInfo[i].dVideoInputIPSize = CVT_INT_CP(pstAAA->stUDRInfo[i].dVideoInputIPSize);
        pstAAA->stUDRInfo[i].dVideoOutputIPSize = CVT_INT_CP(pstAAA->stUDRInfo[i].dVideoOutputIPSize);
        pstAAA->stUDRInfo[i].dBillcomCount = CVT_INT_CP(pstAAA->stUDRInfo[i].dBillcomCount);
        pstAAA->stUDRInfo[i].dGWCount = CVT_INT_CP(pstAAA->stUDRInfo[i].dGWCount);
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

    st_MsgQ         qmsg;
    st_AAAREQ       stAAA;
    st_DumpInfo     stUdrDumpInfo;
    int         iRet = 0;
    char        tmpStart[256];
    char        tmpStop[256];
    struct stat statbuf;
    time_t      start_time;

    memset( &qmsg, 0x00, sizeof(st_MsgQ) );
    memset(&stUdrDumpInfo, 0x00, sizeof(st_DumpInfo));

    PASS_EXCEPTION(stat(filename,&statbuf) < 0, STAT_ERROR);
    
    if (pSrchInfo->iSrchType == SRCHTYPE_TIME)
    {
        PASS_EXCEPTION(statbuf.st_mtime < pSrchInfo->tStartTime,
            FILE_END_ERR);

        PASS_EXCEPTION((start_time = get_ctime(filename)) > pSrchInfo->tEndTime,
            FILE_START_ERR);
    }

    len = strlen(filename);
    strcpy(tmp_filename,filename);

    PASS_EXCEPTION((fd=open(tmp_filename,O_RDONLY)) < 0, FILE_OPEN_ERR);
    
    PASS_EXCEPTION((len=read(fd,(void *)&stUdrDumpInfo,sizeof(st_DumpInfo))) <= 0,
        FILE_READ_ERR);

	fprintf(stderr, "Rcv DumpInfo len[%d] size[%d] udr_cnt[%d], udr_id[%d], udr_sequence[%d], fileName[%s], udr_crt_time[%d]\n", 
		len,
		sizeof(st_DumpInfo),
#if 0	// 071009, poopee
		stUdrDumpInfo.udr_cnt,
		stUdrDumpInfo.udr_id,
		stUdrDumpInfo.udr_sequence,
		stUdrDumpInfo.fileName,
		stUdrDumpInfo.udr_crt_time);
#else
		CVT_INT_CP(stUdrDumpInfo.udr_cnt),
		CVT_INT_CP(stUdrDumpInfo.udr_id),
		CVT_INT_CP(stUdrDumpInfo.udr_sequence),
		stUdrDumpInfo.fileName,
		CVT_INT_CP(stUdrDumpInfo.udr_crt_time));
#endif
    
	/*
    PASS_EXCEPTION(pSrchInfo->iSrchType == SRCHTYPE_TIME && 
        stUdrDumpInfo.udr_crt_time > pSrchInfo->tEndTime, START_AFTER_END_ERR);
    */
    /*
    pSRCHINFO->tot_file++;
    fprintf(stderr,"     : FILE              : %s\n",filename);
    
    */

    while(1)
    {
        len = read(fd, (void *)&stAAA, sizeof(st_AAAREQ));
		
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
        ConvertAllMsg(&stAAA);
        
        //pSRCHINFO->tot_msg++;
        tot_msg++;
        
        if( pSrchInfo->iSrchType == SRCHTYPE_TIME )
        {
            if( (stAAA.stInfo.uiTimeStamp < pSrchInfo->tStartTime)
                || (stAAA.stInfo.uiTimeStamp > pSrchInfo->tEndTime))
            {
                continue;
            }
        }
        /*
        if( pSRCHINFO->ucUdrSeqF == 1 )
        {
            if( stAAA.stInfo.uiUDRSeq != pSRCHINFO->uiUdrSeq )
            {
                continue;
            }
        }
        */
        
        switch (pSrchInfo->iSrchKeyType)
        {
            case SRCHKEY_IMSI:
				if (stAAA.stInfo.szMIN[0] == 0)
                {
					fprintf(stderr,"ERROR: MSID not exist in log header\n");
                }
				else if (!strcmp( pSrchInfo->szImsiNum, (char *)stAAA.stInfo.szMIN) )
				{
                    tot_found++;
                    if ((ret = print_aaa_msg(&stAAA, OutFd, pSrchInfo)) == 0)
                    {
                        tot_written++;
                    }
				}
                break;
            
            case SRCHKEY_SEQ:
				if (stAAA.stInfo.ucUDRSeqF == 0)
                {
					fprintf(stderr,"ERROR: UDRSequence not exist in log header\n");
                }
				else if ( pSrchInfo->uiUdrSeq == stAAA.stInfo.uiUDRSeq)
				{
                    tot_found++;
                    if ((ret = print_aaa_msg(&stAAA, OutFd, pSrchInfo)) == 0)
                    {
                        tot_written++;
                    }
				}
                break;

            case SRCHKEY_ALL:
            default:

            tot_found++;
            if ((ret = print_aaa_msg(&stAAA, OutFd, pSrchInfo)) == 0)
            {
                tot_written++;
            }

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
    
    PASS_CATCH(START_AFTER_END_ERR)
    fprintf(stderr,"ERROR: dump.udr_crt_time(%d) is Bigger than pSrchInfo->tEndTime(%d)!!\n",
        stUdrDumpInfo.udr_crt_time, pSrchInfo->tEndTime);
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


    sprintf(buffer,"TIME-STAMP< UDR-SEQUENCE< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"CALLING-STATION-ID< MDN< ESN< FRAMED-IP-ADDRESS< USER-NAME< ACCT-SESSION-ID< CORRELATION-ID<");
    len = strlen(buffer);

    sprintf(&buffer[len],"SESSION-CONT< BEGINNING-SESSION< HA-IP-ADDR< NAS-IP-ADDRESS< PCF-IP-ADDR< BSID< USER-ID< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"F-FCH-MUX< R-FCH-MUX< SERVICE-OPTION< FTYPE< RTYPE< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"FCH-FRAME-SIZE< FORWARD-FCH-RC< REVERSE-FCH-RC< IP-TECHNOLOGY< COMPULSORY-TUNNEL-INDICATOR< RELEASE-INDICATOR< DCCH-FRAME-SIZE< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"ALWAYS-ON< ACCT-OUTPUT-OCTETS< ACCT-INPUT-OCTETS< BAD-FRAME-COUNT<EVENT-TIMESTAMP< ACTIVE-TIME< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"NUM-ACTIVE< SDB-INPUT-OCTETS< SDB-OUTPUT-OCTETS< NUMSDB-INPUT< NUMSDB-OUTPUT< NUM-BYTES-RECEIVED-TOTAL< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"MIP-SIGNALING-INBOUND-COUNT< MIP-SIGNALING-OUTBOUND-COUNT< IP-QOS< AIR-PRIORITY< ACCT-INPUT-PACKETS< ACCT-OUTPUT-PACKETS< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"R-P-CONNECTION-ID< ACCT-AUTHENTIC< ACCT-SESSION-TIME< ACCT-TERMINATE-CAUSE< ACCT-STATUS-TYPE< ");
    len = strlen(buffer);

    sprintf (&buffer[len], "NAS-PORT-TYPE< NAS-PORT< NAS-PORT-ID< SERVICE-TYPE< ACCT-DELAY-TIME< C23BIT< HBIT< SUBNET< ");
    len = strlen(buffer);
    


    sprintf(&buffer[len],"DATA-SERVICE-TYPE< TRANSACTION-ID< REQUEST-TIME< RESPONSE-TIME< SESSION-TIME< SERVER-IP-ADDRESS< SERVER-PORT< TERMINAL-PORT< ");
    len = strlen(buffer);
	sprintf(&buffer[len],"URL< DOWNLOAD-TYPE< APPLICATION-ID< CONTENT-ID< METHOD-TYPE< RESULT-CODE< IP-LAYER-UPLOAD-SIZE< IP-LAYER-DOWMLOAD-SIZE< TCP-LAYER-RETRANS-INPUT-SIZE< TCP-LAYER-RETRANS-OUTPUT-SIZE< USE-COUNT< USE-TIME< TOTAL-SIZE< TOTAL-TIME< AUDIO-UPLOAD-SIZE< AUDIO-DOWNLOAD-SIZE< VIDEO-UPLOAD-SIZE< VIDEO-DOWNLOAD-SIZE< CALLEE-MIN< CALLER-MIN< TRANSACTION-CONTENT-LENGTH< TRANSACTION-COMPLETENESS< UDR-GENERATION-REASON< USER-AGENT< DOWNLOAD-INFO< SEND-OPT\n");
    len = strlen(buffer);
	
	/*	
	sprintf(&buffer[len],"URL< DOWNLOAD-TYPE< APPLICATION-ID< CONTENT-ID< METHOD-TYPE< RESULT-CODE< IP-LAYER-UPLOAD-SIZE< IP-LAYER-DOWMLOAD-SIZE< TCP-LAYER-RETRANS-INPUT-SIZE< TCP-LAYER-RETRANS-OUTPUT-SIZE< CP-CODE< PHONE-NUMBER< USE-COUNT< USE-TIME< TOTAL-SIZE< TOTAL-TIME< BILLCOM-HEADER-COUNT< WICGS-HEADER< HANDSET-MODEL< AUDIO-UPLOAD-SIZE< AUDIO-DOWNLOAD-SIZE< VIDEO-UPLOAD-SIZE< VIDEO-DOWNLOAD-SIZE< TRANSACTION-CONTENT-LENGTH< TRANSACTION-COMPLETENESS< UDR-GENERATION-REASON< USER-AGENT< DOWNLOAD-INFO\n");
    len = strlen(buffer);
	*/
	/*
		sprintf(&buffer[len],"URL< DOWNLOAD-TYPE< APPLICATION-ID< CONTENT-CODE< METHOD-TYPE< RESULT-CODE< IP-LAYER-UPLOAD-SIZE< IP-LAYER-DOWMLOAD-SIZE< TCP-LAYER-RETRANS-INPUT-SIZE< TCP-LAYER-RETRANS-OUTPUT-SIZE< TRANSACTION-CONTENT-LENGTH< TRANSACTION-COMPLETENESS< TRANSACTION-TERM-REASON< USER-AGENT\n");
    len = strlen(buffer);
	*/


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

time_t get_ctime(char *filename)
{
    int         len, i, tmp_val;
    char        *ptr, tmp_str[5];
    struct tm   bdtime;
    char        *tmp_filename = NULL;

    /* YYYYmmdd_HHMMSS */
    ptr = strrchr(filename,'/');
    ptr += 1;
    
    tmp_filename = ptr + 8;
    //tmp_filename = ptr;
    
    memcpy(tmp_str,tmp_filename,4);      // year
    tmp_str[4] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 1970 || tmp_val > 9999)
    {
        fprintf(stderr,"ERROR: invalid year value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_year = tmp_val - 1900;

    memcpy(tmp_str,tmp_filename+4,2);    // month
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 1 || tmp_val > 12)
    {
        fprintf(stderr,"ERROR: invalid month value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_mon = tmp_val - 1;

    memcpy(tmp_str,tmp_filename+6,2);    // day
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 1 || tmp_val > 31)
    {
        fprintf(stderr,"ERROR: invalid day value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_mday = tmp_val;

    memcpy(tmp_str,tmp_filename+8,2);    // hour
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 0 || tmp_val > 23)
    {
       fprintf(stderr,"ERROR: invalid hour value(%d)!!!\n",tmp_val);
       return 0;
    }
    bdtime.tm_hour = tmp_val;

    memcpy(tmp_str,tmp_filename+10,2);   // min
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 0 || tmp_val > 59)
    {
        fprintf(stderr,"ERROR: invalid min value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_min = tmp_val;

    memcpy(tmp_str,tmp_filename+12,2);   // sec
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 0 || tmp_val > 59)
    {
        fprintf(stderr,"ERROR: invalid sec value(%d)!!!\n",tmp_val);
        return 0;
    }

    bdtime.tm_sec = tmp_val;

    return (mktime(&bdtime));

}


int srch_file(st_SrchInfo_t *pSrchInfo, FILE *Outfd)
{
    DIR         *dirp;
    struct dirent   *direntp;
    char        filename[MAX_FILENAME_LEN+1];
    int         iRet = 0;
    int         iFileCnt = 0;
    struct tm   tmStart, tmEnd, tmInc;
    char        keepDir[15], endDir[15];
    char        searchDir[128], keepDate[15];
    char        startDate[15], endDate[15];
    char        sysSide;
    int         i = 0;
    int         j = 0;
    time_t      incrDay = 0;
	char		szTmpCmdName[256];
	char		szTmpFileName[256];
	int			iFoundCnt = 0;
	char		szBufFileName[MAX_FILE_COUNT][256];
	char		*tmpFilePtr = NULL;
	
	memset( szBufFileName, 0x00, sizeof(szBufFileName));

    localtime_r(&pSrchInfo->tStartTime, &tmStart);
    localtime_r(&pSrchInfo->tEndTime, &tmEnd);

    sprintf(endDir, "20%02d/%02d/%02d", tmEnd.tm_year-100, tmEnd.tm_mon+1, tmEnd.tm_mday);   
    sprintf(startDate, "20%02d%02d%02d%02d%02d%02d", tmStart.tm_year-100, tmStart.tm_mon+1, tmStart.tm_mday,
            tmStart.tm_hour, tmStart.tm_min, tmStart.tm_sec);   
    sprintf(endDate, "20%02d%02d%02d%02d%02d%02d", tmEnd.tm_year-100, tmEnd.tm_mon+1, tmEnd.tm_mday,
            tmEnd.tm_hour, tmEnd.tm_min, tmEnd.tm_sec);

    for( i=0; i<2; i++ )
    {

        if( i == 0 )
        {
            sysSide = 'A';
        }
        else
        {
            sysSide = 'B';
        }

        sprintf(keepDir, "20%02d/%02d/%02d", tmStart.tm_year-100, tmStart.tm_mon+1, tmStart.tm_mday);

        incrDay = pSrchInfo->tStartTime - (pSrchInfo->tStartTime % 86400);
        
        while( strcmp(endDir, keepDir) >= 0)
        {	
            sprintf(searchDir, "%s/BSD%c/%s", "/BSDM/UDR", sysSide, keepDir);
            fprintf(stdout, "Search Dir [%s]\n", searchDir);
			dirp = NULL;
            dirp = opendir(searchDir);
            while( dirp && ((direntp = readdir(dirp)) != 0))
            {
				/*
				fprintf(stdout, "Search Dir [%s] ReadFile[%s]\n", searchDir, direntp->d_name);
				*/
                if( !strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
                    continue;
                
                if( !(strlen(direntp->d_name) == UDR_FILE_LEN ||
					strlen(direntp->d_name) == UDR_GZFILE_LEN) )
                    continue;
				
                if( strncmp(startDate, &direntp->d_name[8], 14 ) > 0)
                    continue;
                
                if( strncmp(endDate, &direntp->d_name[8], 14 ) < 0)
                    continue;
                
				snprintf(szBufFileName[iFoundCnt++], 256, "%s/%s", searchDir, direntp->d_name);
            }

            if( dirp )
            {
                closedir(dirp);
            }
            
            incrDay = pSrchInfo->tStartTime + 86400;
            localtime_r(&incrDay, &tmInc);
			sprintf(keepDir, "20%02d/%02d/%02d", tmInc.tm_year-100, tmInc.tm_mon+1, tmInc.tm_mday);
        }
    }

	/* Sort By ID */
	for( i=0; i<iFoundCnt; i++ )
	{
		for( j=i; j<iFoundCnt; j++ )
		{
			if( strcmp(szBufFileName[i], szBufFileName[j]) > 0 )
			{
				snprintf(filename, 256, "%s", szBufFileName[i]);
				snprintf(szBufFileName[i], 256, "%s", szBufFileName[j]);
				snprintf(szBufFileName[j], 256, "%s", filename);
			}

		}
	}

	for( i=0; i<iFoundCnt; i++ )
	{
		memset(szTmpCmdName, 0x00, 256);
		memset( filename, 0x00, sizeof(filename));
		tmpFilePtr = strrchr(szBufFileName[i], '/');
		tmpFilePtr++;
		
		if( strlen(tmpFilePtr) == UDR_GZFILE_LEN )
		{
			snprintf(szTmpCmdName, 256, "gunzip %s", szBufFileName[i]);
			snprintf(filename, 256, "%s", szBufFileName[i]);
			filename[strlen(filename)-3] = '\0';
			system(szTmpCmdName);
			srch_msg(filename, Outfd, pSrchInfo);
			snprintf(szTmpCmdName, 256, "gzip %s", filename);
			system(szTmpCmdName);
		}
		else
		{
			sprintf(filename, "%s", szBufFileName[i]);
			srch_msg(filename, Outfd, pSrchInfo);
		}
	}
     
    return iRet;
}




