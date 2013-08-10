#include "mmcr.h"
#include "comm_trace.h"
#include "sm_subs_info.h"

extern char     iv_home[64];
extern st_SESSInfo		*gpTrcList[DEF_SET_CNT];
extern st_NOTI			*gpIdx;

extern int g_Flag;
extern int trcErrLogId;

extern int dSendNOTIFY(unsigned short uhMsgID, SUBS_INFO *psi);
extern int dWriteFLTIDXFile(void);
extern int dWrite_TrcConf(st_SESSInfo *pCurTrc);
extern int dReadTrcFile(void);


void conf_file_sync(char *filename)
{
    char    *myname;
    char    hostname[20];

    myname = getenv(MY_SYS_NAME);
    
//   if(!strcasecmp(myname, "DSCA")){
//      strcpy(hostname, "DSCB");
//    }else if(!strcasecmp(myname, "DSCB")){
//        strcpy(hostname, "DSCA");
//    }

	if(!strcasecmp(myname, "SCMA")){
		strcpy(hostname, "SCMB");
	}else if(!strcasecmp(myname, "SCMB")){
		strcpy(hostname, "SCMA");
	}

// yhshin
// A/B side로 다 내려줘서 rsync 하는 경우가 없다. 	
//    sprintf(cmd, "%s %s/%s %s::DSC/DATA", RSYNC, getenv(IV_HOME), filename, hostname);
//   system(cmd);
}

char * strtoupper2(char *s1)
{
    int     i, len;
    static char     buff[1024];

    len = strlen(s1);

    memset(buff, 0x00, sizeof(buff));
    for(i = 0; (i < (sizeof(buff)-1)) && (i < len); i++){
        buff[i] = toupper(s1[i]);
    }

    buff[i] = 0x00;

    return buff;

}

int deleteTime ()
{
	time_t	cur_time;
	struct tm*	cur_tMS;

	cur_time = time(0);

    cur_tMS = (struct tm*)localtime((time_t*)&cur_time);

    if ( cur_tMS->tm_sec/5 < 1 )
	{
		g_Flag = DEF_OFF_FLAG;
		return 1;	
	}
	else
	{
		g_Flag = DEF_ON_FLAG;
		return -1;
	}
}
#if 0
int deleteTime ()
{
	time_t	cur_time;
	struct tm*	cur_tMS;

	cur_time = time( (time_t *)0);

    cur_tMS = (struct tm*)localtime((time_t*)&cur_time);

    if ( (cur_tMS->tm_min == 0)  && (cur_tMS->tm_sec/20 < 1)){
		return 1;	
	}
	else
		return -1;
}
#endif

#define EXPIERED 1
#define REWRITE_ON	1
int callTraceDelete (time_t now)
{
	int i, idx = 0;
	time_t deadline = now;
//	time_t expired_time;
	int	delIndex[MAX_TRACE_NUM] = {0,};
	int	reWrite = 0;
	int totalCnt = 0;
	char szFName[32];
	FILE	*fp = NULL;
	int	dCurIdx, dNewIdx;

	st_SESSInfo *pCurTrc, *pNewTrc;

	memset(szFName, 0x00, sizeof(szFName));

	dCurIdx = gpIdx->dTrcIdx;
	pCurTrc = gpTrcList[dCurIdx];

	dNewIdx = (dCurIdx == 0) ? 1 : 0;
	pNewTrc = gpTrcList[dNewIdx];

	totalCnt = pCurTrc->dTCount;

	if( totalCnt == 0 )
		return 0;

	for( i = 0; i < totalCnt; i++ )
	{
		if( deadline - pCurTrc->stTrc[i].tRegTime > pCurTrc->stTrc[i].dDura * 60 )
		{
			logPrint(trcErrLogId,FL, "[%d.. time %ld > %ld expired] LIST[%s] REG[%ld] DURATION[%d]\n",
				totalCnt, deadline, 
				(pCurTrc->stTrc[i].tRegTime + pCurTrc->stTrc[i].dDura * 60), 
				pCurTrc->stTrc[i].szImsi, pCurTrc->stTrc[i].tRegTime, pCurTrc->stTrc[i].dDura);
			reWrite = REWRITE_ON;
			delIndex[i] = EXPIERED;
		}
		else
		{
			logPrint(trcErrLogId,FL, "[%d time %ld < %ld] LIST[%s] REG[%ld] DURATION[%d]\n", 
					totalCnt, deadline, 
					(pCurTrc->stTrc[i].tRegTime + pCurTrc->stTrc[i].dDura * 60), 
					pCurTrc->stTrc[i].szImsi, pCurTrc->stTrc[i].tRegTime, pCurTrc->stTrc[i].dDura);
			delIndex[i] = 0;
		}
	}

	if( reWrite == REWRITE_ON ) // Expire된 list가 존재한다. 
	{
		sprintf(szFName, "%s/%s",iv_home, TRACE_INFO_FILE );

		if ((fp = fopen(szFName, "w"))==NULL){
			logPrint(trcErrLogId,FL, "[dWrite_TrcConf] fopen failed:%s\n",szFName);
			return -1;                          
		}                                           

		fprintf(fp, "@START\n");
		for( i = 0; i < totalCnt; i++ ) // expire 되지 않은 list만 파일에 쓴다. 
		{                                   
			if( delIndex[i] != EXPIERED )
			{
				fprintf(fp," %d\t%s\t%ld\t%d\n",            
						pCurTrc->stTrc[i].dType, pCurTrc->stTrc[i].szImsi,                                            			pCurTrc->stTrc[i].tRegTime, pCurTrc->stTrc[i].dDura);
			
				logPrint(trcErrLogId,FL, "[ReWrite] LIST[%s] REG[%ld] DURATION[%d]\n", 
					pCurTrc->stTrc[i].szImsi, pCurTrc->stTrc[i].tRegTime,
					pCurTrc->stTrc[i].dDura);

				pNewTrc->stTrc[idx++].dType = pCurTrc->stTrc[i].dType;	
				pNewTrc->stTrc[idx].tRegTime = pCurTrc->stTrc[i].tRegTime;	
				pNewTrc->stTrc[idx].dDura = pCurTrc->stTrc[i].dDura;	
				memcpy(pNewTrc->stTrc[idx].szImsi, pCurTrc->stTrc[i].szImsi, MAX_IMSI_LEN);
				pNewTrc->dTCount++;
			}
		}  
		fprintf(fp, "@END\n");
		fclose(fp);

		gpIdx->dTrcIdx = dNewIdx;
		dWriteFLTIDXFile();
		dSendNOTIFY(NOTI_TRACE_TYPE, NULL);
		memset(pCurTrc, 0x00, sizeof(st_SESSInfo));
	}

	return 1;

} /* End of callTraceDelete */
#if 0
int callTraceDelete (time_t now)
{
	int i , j ;
	time_t expired_time;
	int	delIndex;
	int	reWrite = 0;
	int delCnt = 0;
	int totalCnt = stCallTrcList.dTCount;

	if( totalCnt == 0 )
		return 0;

	for( i = 0; i < totalCnt; i++ )
	{
		expired_time = stCallTrcList.stTrc[i].tRegTime + (60 * stCallTrcList.stTrc[i].dDura);
		if( expired_time < now )
		{
			reWrite = 1;
			delIndex = i;
			delCnt++;

			for( j = delIndex; j < totalCnt-1; j++ )
			{
				memset(stCallTrcList.stTrc[j].szImsi, 0x00, sizeof(stCallTrcList.stTrc[j].szImsi));
				sprintf(stCallTrcList.stTrc[j].szImsi, "%s", stCallTrcList.stTrc[j+1].szImsi);
				stCallTrcList.stTrc[j].tRegTime = stCallTrcList.stTrc[j+1].tRegTime;
				stCallTrcList.stTrc[j].dType = stCallTrcList.stTrc[j+1].dType;
				stCallTrcList.stTrc[j].dDura = stCallTrcList.stTrc[j+1].dDura;
			}

			memset(stCallTrcList.stTrc[j].szImsi, 0x00, sizeof(stCallTrcList.stTrc[j].szImsi));
			stCallTrcList.stTrc[j].tRegTime = 0;                                               
			stCallTrcList.stTrc[j].dType = 0;
			stCallTrcList.stTrc[j].dDura = 0;

			stCallTrcList.dTCount--;
		}
	}

	if( reWrite == 1 )
	{
		dWrite_TrcConf();
		usleep(10000);

		// Notify Sync Trace config file reload
		oldSendAppNoty(2, DEF_SYS, SID_CHG_INFO, MID_TRC);
		oldSendAppNoty(3, DEF_SVC, SID_CHG_INFO, MID_TRC);
		oldSendAppNoty(4, DEF_SVC, SID_CHG_INFO, MID_TRC);
	}

	return 1;

} /* End of callTraceDelete */
#endif
