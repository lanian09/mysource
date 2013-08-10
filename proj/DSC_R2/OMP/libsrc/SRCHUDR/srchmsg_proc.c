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
// 07.22 jjinri #include "srchmsg.h"
extern int              errno;

// 07.22 jjinri int proc_srchmsg(SRCH_INFO *pSRCHINFO, char *szResBuf);
// 07.22 jjinri int srch_file(char *dirpath, SRCH_INFO *pSRCHINFO, char *szResBuf);
int is_headlog(char *filename);
time_t get_ctime(char *filename);
// 07.22 jjinri int srch_msg(char *filename, SRCH_INFO *pSRCHINFO, char *szResBuf);
// 07.22 jjinri int write_title(SRCH_INFO *pSRCHINFO);
// 07.22 jjinri int write_msg(st_AAAREQ *pAAAReq, SRCH_INFO *pSRCHINFO);

extern char *str_time(time_t t);

#if 0 // 07.22 jjinri
void ConvertAllMsg(st_AAAREQ *pstAAA)
{

    int i = 0;
    long long tmpLongLong = 0;
    long long tmpLongLong_result = 0;
    
    pstAAA->dUDRCount = CVT_INT_CP(pstAAA->dUDRCount);
    pstAAA->dReserved = CVT_INT_CP(pstAAA->dReserved);
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

/*
    tmpLongLong = pstAAA->stInfo.llAcctSessID;
    CVT_INT64_CP(&tmpLongLong, pstAAA->stInfo.llAcctSessID);
    pstAAA->stInfo.llAcctSessID = tmpLongLong;
*/
   
    tmpLongLong = 0;
    tmpLongLong = pstAAA->stInfo.llCorrelID;
    CVT_INT64_CP(&tmpLongLong, pstAAA->stInfo.llCorrelID);
    pstAAA->stInfo.llCorrelID = tmpLongLong;
    
    pstAAA->stInfo.uiRetryFlg = CVT_INT_CP(pstAAA->stInfo.uiRetryFlg);
    pstAAA->stInfo.dReserved = CVT_INT_CP(pstAAA->stInfo.dReserved);
    pstAAA->stInfo.uiC23BIT = CVT_INT_CP(pstAAA->stInfo.uiC23BIT);
    /* End of ACCInfo */
    
    /* Start of UDRInfo */
    for(i = 0; (i < MAX_UDR_COUNT) && (i < pstAAA->dUDRCount); i++)
    {
        //pstAAA->stUDRInfo[i].llAcctSessID = CVT_INT_CP(pstAAA->stUDRInfo[i].llAcctSessID);
        tmpLongLong = 0;
/*
        tmpLongLong = pstAAA->stUDRInfo[i].llAcctSessID;
        CVT_INT64_CP(&tmpLongLong, pstAAA->stUDRInfo[i].llAcctSessID);
        pstAAA->stUDRInfo[i].llAcctSessID = tmpLongLong;
*/       
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

int proc_srchmsg(SRCH_INFO *pSRCHINFO, char *szResBuf)
{
    char        path[MAX_FILENAME_LEN+1], targetDir[MAX_FILENAME_LEN+1];
    char        e_month, e_day;
    time_t      tmp_timet;
    struct tm   *bdtime;
    int         fd;
    struct stat statbuf;
    int         iRet = 0;
	int			dOffset = 0;
	
	dOffset = strlen(szResBuf);
    sprintf(path,"%s",UDRLOG_PATH);

    PASS_EXCEPTION((pSRCHINFO->fp=fopen(pSRCHINFO->outfile,"w")) == NULL,
        FOPEN_FAIL);

    PASS_EXCEPTION((pSRCHINFO->err_fp=fopen(pSRCHINFO->errfile,"w")) == NULL,
        ERR_FOPEN_FAIL);

    if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
    {
        PASS_EXCEPTION(write_title(pSRCHINFO) != 0, TITLE_WRITE_FAIL);
    }

    if (pSRCHINFO->srchtype == SRCHTYPE_FILE)
    {
        PASS_EXCEPTION(srch_msg(pSRCHINFO->filename, pSRCHINFO, szResBuf) != 0,
            SEARCH_FAIL);
    }
    else
    {
        bdtime = localtime(&pSRCHINFO->endtime);
        e_month = bdtime->tm_mon + 1;
        e_day = bdtime->tm_mday;

        tmp_timet = pSRCHINFO->starttime;
        bdtime = localtime(&tmp_timet);
            
        sprintf(targetDir,"%s",path);
        
        PASS_EXCEPTION(srch_file(targetDir, pSRCHINFO, szResBuf)!= 0, SEARCH_FILE_FAIL);

        tmp_timet += 60*60*24;      /* 24½ð£ °æ */
        bdtime = localtime(&tmp_timet);
            
    }

	
	dOffset = strlen(szResBuf);
    sprintf(&szResBuf[dOffset],"    FINAL RESULT = FILE:%d MESSAGE:%d FOUND:%d WRITTEN:%d\n"
							   "    ==========================================================================\n",
        pSRCHINFO->tot_file,pSRCHINFO->tot_msg,pSRCHINFO->tot_found,pSRCHINFO->tot_written);

    if (pSRCHINFO->tot_written > 65535)
        fprintf (stderr, "[SPLIT RESULT] split -l 65535 [RESULT_FILE]");
    
    PASS_CATCH(FOPEN_FAIL)
    fprintf(stderr,"ERROR: file(%s) open error(%s)!!!\n",
        pSRCHINFO->outfile,strerror(errno));
    iRet = -1;
    
    PASS_CATCH(ERR_FOPEN_FAIL)
    fprintf(stderr,"ERROR: file(%s) open error(%s)!!!\n",
        pSRCHINFO->errfile,strerror(errno));
    iRet = -1;
    
    PASS_CATCH(TITLE_WRITE_FAIL)
    fprintf(stderr,"ERROR: file(%s) Write error(%s)!!!\n",
        pSRCHINFO->outfile,strerror(errno));
    iRet = -1;
    
    PASS_CATCH(SEARCH_FAIL)
    fprintf(stderr,"ERROR: SEARCH Fail!!!\n", strerror(errno));
    iRet = -1;
    
    PASS_CATCH(SEARCH_FILE_FAIL)
    fprintf(stderr,"ERROR: SEARCH File Fail!!!\n", strerror(errno));
    iRet = -1;

    PASS_CATCH_END;
    
    fclose(pSRCHINFO->fp);
    fclose(pSRCHINFO->err_fp);

    if (stat(pSRCHINFO->errfile,&statbuf) != -1)
    {
        if (statbuf.st_size == 0) 
        {
            unlink(pSRCHINFO->errfile);
        }
    }

    return iRet;

}


/*------------------------------------------------------------------------------
* FUNCTIONS   :

* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
#define DAY_BY_SECONDS		86400
int srch_file(char *rootpath, SRCH_INFO *pSRCHINFO, char *szResBuf)
{
    DIR         *dirp;
    struct dirent   *direntp;
    char        filename[MAX_FILENAME_LEN+1], cmd[256];
    int         iRet = 0, i;
    int         iFileCnt = 0, is_gziped;
	time_t		incrDay;
	struct tm	tmstart, tmend, tminc;
	char		keepDir[15], endDir[15];
	char		searchDir[128], sysSide, keepDate[15];
	char		startDate[15], endDate[15], *dot;
	UDRFilename	searchFile;

	localtime_r(&pSRCHINFO->starttime, &tmstart);
	localtime_r(&pSRCHINFO->endtime, &tmend);

	sprintf(endDir, "20%02d/%02d/%02d", tmend.tm_year-100, tmend.tm_mon+1, tmend.tm_mday);
	sprintf(startDate, "20%02d%02d%02d%02d%02d%02d", tmstart.tm_year-100, tmstart.tm_mon+1,
			tmstart.tm_mday, tmstart.tm_hour, tmstart.tm_min, tmstart.tm_sec);
	sprintf(endDate, "20%02d%02d%02d%02d%02d%02d", tmend.tm_year-100, tmend.tm_mon+1,
			tmend.tm_mday, tmend.tm_hour, tmend.tm_min, tmend.tm_sec);


	for(i = 0; i < 2; i++){
		if(i == 0)
			sysSide = 'A';
		else
			sysSide = 'B';

		sprintf(keepDir, "20%02d/%02d/%02d", tmstart.tm_year-100, tmstart.tm_mon+1, tmstart.tm_mday);
		// get 00:00:00 everyday
		incrDay = pSRCHINFO->starttime - (pSRCHINFO->starttime%DAY_BY_SECONDS);

		while(strcmp(endDir, keepDir) >= 0){
			sprintf(searchDir, "%s/BSD%c/%s", rootpath, sysSide, keepDir);

			dirp = NULL;
			dirp = opendir(searchDir);
			while(dirp && (direntp = readdir(dirp)) != 0){
				if (!strcmp(direntp->d_name,".") || !strcmp(direntp->d_name,"..")) 
					continue;
/* do not check a length of file name
				if(strlen(direntp->d_name) != sizeof(UDRFilename))
					continue;
*/

				if(filename_check(direntp->d_name) < 0)
					continue;

				is_gziped = is_file_gziped(direntp->d_name);
				if(is_gziped < 0)
					continue;

				memcpy(&searchFile, direntp->d_name, sizeof(UDRFilename));
				if(memcmp(startDate, searchFile.createtime.time14s, 14) > 0)
					continue;
				if(memcmp(endDate, searchFile.createtime.time14s, 14) < 0)
					continue;
					//break;

				pSRCHINFO->tot_file++;
				memset(filename, 0x00, sizeof(filename));
				sprintf(filename, "%s/%s", searchDir, direntp->d_name);

				// unzip and truncate ".gz" extension
				if(is_gziped == 1){
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "/usr/bin/gunzip %s", filename);
					system(cmd);
					dot = strrchr(filename, '.');
					*dot = 0x00;
				}
        		srch_msg(filename, pSRCHINFO, szResBuf);

				// if first ziped, rezip.
				if(is_gziped == 1){
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "/usr/bin/gzip %s", filename);
					system(cmd);
				}
			}
			if(dirp)
				closedir(dirp);

			incrDay = pSRCHINFO->starttime + DAY_BY_SECONDS;
			localtime_r(&incrDay, &tminc);
			sprintf(keepDir, "20%02d/%02d/%02d", tminc.tm_year-100, tminc.tm_mon+1, tminc.tm_mday);
		}
	}

    return iRet;
}
#endif

/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/

int is_headlog(char *filename)
{
    int         len, i, length;
    char        *ptr = 0;

    ptr = strstr(filename,".gz");
    if (ptr != NULL && !strcmp(ptr,".gz"))
        length = 18;
    else
        length = 15;

    len = strlen(filename);
    if (len != length)      /* YYYYmmdd_HHMMSS */
    {
        fprintf(stderr,"ERROR: invalid headlog filename(%s) length(%d)!!!\n",filename,len);
        return -1;
    }

    if (len > 15) len = 15;

    for (i=0; i<len; i++)
    {
        if (!isdigit(filename[i]) && filename[i] != '_')
        {
            fprintf(stderr,"ERROR: invalid headlog filename(%s)!!!\n",filename);
            return -1;
        }
    }

    return 0;
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
    char        *tmp_filename = NULL;

    /* YYYYmmdd_HHMMSS */
    ptr = strrchr(filename,'/');
    ptr += 1;
    
    tmp_filename = ptr + 8;
    
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


#if 0 // 07.22 jjinri
/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/

int srch_msg(char *filename, SRCH_INFO *pSRCHINFO, char *szResBuf)
{
    int         fd, len, ret;
    int         tot_msg=0, tot_found=0, tot_written=0;
    char        tmp_filename[MAX_FILENAME_LEN+1], command[128], *ptr;
	char		statBuf[256];

    st_MsgQ         qmsg;
    st_AAAREQ       stAAA;
    st_DumpInfo     stUdrDumpInfo;
    int         iRet = 0;
    CHAR        tmpStart[256];
    char        tmpStop[256];
    struct stat statbuf;
    time_t      start_time;
	int			dOffset = 0;

    memset( &qmsg, 0x00, sizeof(st_MsgQ) );
    memset(&stUdrDumpInfo, 0x00, sizeof(st_DumpInfo));
    PASS_EXCEPTION(stat(filename,&statbuf) < 0, STAT_ERROR);

    if (pSRCHINFO->srchtype == SRCHTYPE_TIME)
    {
        PASS_EXCEPTION(statbuf.st_mtime < pSRCHINFO->starttime,
            FILE_END_ERR);

        PASS_EXCEPTION((start_time = get_ctime(filename)) > pSRCHINFO->endtime,
            FILE_START_ERR);
    }

    len = strlen(filename);
    strcpy(tmp_filename,filename);

    PASS_EXCEPTION((fd=open(tmp_filename,O_RDONLY)) < 0, FILE_OPEN_ERR);
    
    PASS_EXCEPTION((len=read(fd,(void *)&stUdrDumpInfo,sizeof(st_DumpInfo))) <= 0,
        FILE_READ_ERR);

//    PASS_EXCEPTION(pSRCHINFO->srchtype == SRCHTYPE_TIME && 
//        stUdrDumpInfo.udr_crt_time > pSRCHINFO->endtime, START_AFTER_END_ERR);

    
    while(1)
    {
        len = read(fd, (void *)&stAAA, sizeof(st_AAAREQ));
		
		//fprintf(stderr, "ReadLen[%d], st_AAAREQSize[%d]\n", len, sizeof(st_AAAREQ));
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
        
        pSRCHINFO->tot_msg++;
        tot_msg++;
        
        if( pSRCHINFO->srchtype == SRCHTYPE_TIME )
        {
            if( (stAAA.stInfo.uiTimeStamp < pSRCHINFO->starttime )
                || (stAAA.stInfo.uiTimeStamp > pSRCHINFO->endtime ))
            {
                continue;
            }
        }
        if( pSRCHINFO->ucUdrSeqF == 1 )
        {
            if( stAAA.stInfo.uiUDRSeq != pSRCHINFO->uiUdrSeq )
            {
                continue;
            }
        }

        switch (pSRCHINFO->numtype)
        {
            case NUMTYPE_IMSI:
				if (stAAA.stInfo.szMIN[0] == 0) 
				{
					fprintf(stdout,"ERROR: MSID not exist in log header\n");
					break;
				}
				if (!strcmp( pSRCHINFO->imsi, (char *)stAAA.stInfo.szMIN) )
				{
					pSRCHINFO->tot_found++;
					tot_found++;
					if ((ret=write_msg(&stAAA, pSRCHINFO)) == 0)
					{
						pSRCHINFO->tot_written++;
						tot_written++;
					}
					else if (ret == -1)
					{
						fprintf(pSRCHINFO->err_fp,"FILENAME: %s",filename);
						//print_hex_log(pSRCHINFO->err_fp, qmsg.szBody, qmsg.usBodyLen);
					}
				}
            break;
            
            case NUMTYPE_UDRSEQ:
				if (stAAA.stInfo.ucUDRSeqF == 0) 
					fprintf(stdout,"ERROR: UDRSequence not exist in log header\n");
				if ( pSRCHINFO->uiUdrSeq == stAAA.stInfo.uiUDRSeq)
				{
					pSRCHINFO->tot_found++;
					tot_found++;
					if ((ret=write_msg(&stAAA, pSRCHINFO)) == 0)
					{
						pSRCHINFO->tot_written++;
						tot_written++;
					}
					else if (ret == -1)
					{
						fprintf(pSRCHINFO->err_fp,"FILENAME: %s",filename);
						//print_hex_log(pSRCHINFO->err_fp, qmsg.szBody, qmsg.usBodyLen);
					}
				}
            break;

            default:
					pSRCHINFO->tot_found++;
					tot_found++;
					if ((ret=write_msg(&stAAA, pSRCHINFO)) == 0)
					{
						pSRCHINFO->tot_written++;
						tot_written++;
					}
					else if (ret == -1)
					{
						fprintf(pSRCHINFO->err_fp,"FILENAME: %s",filename);
						//print_hex_log(pSRCHINFO->err_fp, qmsg.szBody, qmsg.usBodyLen);
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

//	dOffset = strlen(szResBuf);
//  sprintf(&szResBuf[dOffset],"     %-15s   : MESSAGE=%d FOUND=%d WRITTEN=%d\n",
//            ptr,tot_msg,tot_found,tot_written);           
	sprintf(statBuf,"     %-15s   : MESSAGE=%d FOUND=%d WRITTEN=%d\n",
            ptr,tot_msg,tot_found,tot_written);           
	fputs(statBuf, pSRCHINFO->err_fp);
	fflush(pSRCHINFO->err_fp);
    
    PASS_CATCH(STAT_ERROR)
    fprintf(stderr,"ERROR: stat(%s) error(%s)!!!\n",filename,strerror(errno));
    iRet = -1;
        
    PASS_CATCH(FILE_END_ERR)
    iRet = -1;
        
    PASS_CATCH(FILE_START_ERR)
    strftime( tmpStart, 256, "%Y-%m-%d %T %a", localtime(&start_time));
    strftime( tmpStop, 256, "%Y-%m-%d %T %a", localtime(&pSRCHINFO->endtime));

    iRet = -1;
    
    PASS_CATCH(FILE_OPEN_ERR)
    fprintf(stderr,"ERROR: file(%s) open error(%s)!!!\n",tmp_filename,strerror(errno));
    iRet = -1;
    
    PASS_CATCH(FILE_READ_ERR)
    fprintf(stderr,"ERROR: file(%s) read error(%s)!!!\n",tmp_filename,strerror(errno));
    close(fd);
    iRet = -1;
    
    
    PASS_CATCH(START_AFTER_END_ERR);
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
int write_title(SRCH_INFO *pSRCHINFO)
{
    char        buffer[1024*20];
    int         len;
    int         iRet = 0;


    sprintf(buffer,"TIME-STAMP< UDR-SEQUENCE< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"MSID< MDN< ESN< SOURCE-IP-ADDRESS< NETWORK-ACCESS-INDENTIFIER< ACCT-SESSION-ID< CORRELATION-ID<");
    len = strlen(buffer);

    sprintf(&buffer[len],"SESSION-CONTINUE< BEGINNING-SESSION< HOME-AGENT< PDSN-ADDRESS< SERVING-PCF< BSID< USER-ZONE< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"FORWARD-FCH-MUX-OPTION< REVERSE-FCH-MUX-OPTION< SERVICE-OPTION< FORWARD-TRAFFIC-TYPE< REVERSE-TRAFFIC-TYPE< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"FCH-FRAME-SIZE< FORWARD-FCH-RC< REVERSE-FCH-RC< IP-TECHNOLOGY< COMPULSORY-TUNNEL-INDICATOR< RELEASE-INDICATOR< DCCH-FRAME-SIZE< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"ALWAYS-ON< DATA-OCTET-COUNT-TERMINATING< DATA-OCTET-COUNT-ORIGINATING< BAD-PPP-FRAME-COUNT< EVENT-TIME< ACTIVE-TIME< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"NUMBER-ACTIVE-TRANSITIONS< SDB-OCTET-COUNT-TERMINATING< SDB-OCTET-COUNT-ORIGINATING< NUMBER-OF-SDBS-TERMINATING< NUMBER-OF-SDBS-ORIGINATING< NUMBER-OF-HDLC-LAYER-OCTET-RECV< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"INBOUND-MIP-SIGNALING-OCTET-COUNT< OUTBOUND-MIP-SIGNALING-OCTET-COUNT< IP-QOS< AIRLINK-QOS< ACCT-INPUT-PACKETS< ACCT-OUTPUT-PACKETS< ");
    len = strlen(buffer);

    sprintf(&buffer[len],"R-P-CONNECTION-ID< ACCT-AUTHENTIC< ACCT-SESSION-TIME< ACCT-TERMINATE-CAUSE< ACCT-STATUS-TYPE< ");
    len = strlen(buffer);

    sprintf (&buffer[len], "NAS-PORT-TYPE< NAS-PORT< NAS-PORT-ID< SERVICE-TYPE< ACCT-DELAY-TIME< C23BIT< ");
    len = strlen(buffer);
    


    sprintf(&buffer[len],"DATA-SERVICE-TYPE< TRANSACTION-ID< REQUEST-TIME< RESPONSE-TIME< SESSION-TIME< SERVER-IP-ADDRESS< SERVER-PORT< TERMINAL-PORT< ");
    len = strlen(buffer);
/*
    sprintf(&buffer[len],"URL, C_TYPE, APPLICATION-ID, CONTENT-CODE, METHOD-TYPE, RESULT-CODE, IP-LAYER-UPLOAD-SIZE, IP-LAYER-DOWMLOAD-SIZE, TCP-LAYER-RETRANS-INPUT-SIZE, TCP-LAYER-RETRANS-OUTPUT-SIZE, TRANSACTION-CONTENT-LENGTH, TRANSACTION-COMPLETENESS, TRANSACTION-TERMINATION-REASON, USER-AGENT, \n");
    len = strlen(buffer);
*/
	sprintf(&buffer[len],"URL< DOWNLOAD-TYPE< APPLICATION-ID< CONTENT-CODE< METHOD-TYPE< RESULT-CODE< IP-LAYER-UPLOAD-SIZE< IP-LAYER-DOWMLOAD-SIZE< TCP-LAYER-RETRANS-INPUT-SIZE< TCP-LAYER-RETRANS-OUTPUT-SIZE< CP-CODE< PHONE-NUMBER< USE-COUNT< USE-TIME< TOTAL-SIZE< TOTAL-TIME< BILLCOM-HEADER-COUNT< GATEWAY-HEADER-COUNT< HANDSET-MODEL< TRANSACTION-CONTENT-LENGTH< TRANSACTION-COMPLETENESS< TRANSACTION-TERM-REASON< USER-AGENT< DOWNLOAD-INFO< SEND-OPT< CALLER-MIN< CALLEE-MIN\n");
	len = strlen(buffer);

    PASS_EXCEPTION(fprintf(pSRCHINFO->fp,"%s",buffer) < 0, FWRITE_FAIL);
    

    PASS_CATCH(FWRITE_FAIL)
    fprintf(stderr,"ERROR: file(%s) write error(%s)!!!\n",
            pSRCHINFO->outfile,strerror(errno));
    fclose(pSRCHINFO->fp);
    fclose(pSRCHINFO->err_fp);    
    
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
int write_msg(st_AAAREQ *pAAAReq, SRCH_INFO *pSRCHINFO)
{
    st_ACCInfo  nas_msg;
    st_UDRInfo  udr_msg; 

    int         ret, adr;
    char        IMSI[16];
	
	int			tmpCnt = 0;
	
	//CVT_INT_2(pAAAReq->dUDRCount, &tmpCnt);
	//tmpCnt = SWAB4(pAAAReq->dUDRCount);
    //fprintf(stderr,"[%s] UDR-Count[%d]\n",__func__, pAAAReq->dUDRCount);

    if (print_aaa_msg(pAAAReq, pSRCHINFO) < 0)
    {
        fprintf(stderr,"ERROR: AAA message print error");
    }
		
    return 0;
}
#endif


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

#if 0 // 07.22 jjinri 
int filename_check(char *filename)
{
        int                             i;
    UDRFilename     namebuf;

	memcpy(&namebuf, filename, sizeof(UDRFilename));
	if(namebuf.udrscore1 != '_'){
		return -1;
	}

	if(!isalnum(namebuf.side)){
		return -1;
	}

	if(namebuf.udrscore2 != '_'){
		return -1;
	}

	if(namebuf.udrscore3 != '_'){
		return -1;
	}

	for(i = 0; i < 14; i++){
		if(!isdigit(namebuf.createtime.time14s[i])){
			return -1;
		}
	}

	return 0;

}
#endif

int is_file_gziped(char *filename)
{
	char	*dot;

	dot = strrchr(filename, '.');

	if(!dot)
		return -1;

	dot++;

	if(!strcmp(dot, "gz"))
		return 1;
	else
		return 0;


}
