
#include "mmc_hld.h"
#include "mmcr.h"
////#include "SmApiBlocking_c.h"
#include "comm_timer.h"
#include "comm_trace.h"
#include "ipaf_stat.h"
#include "hash_pdsn.h"
#include "hasho.h"
#include "sm_subs_info.h"
#include "common_ana.h"
#include "comm_session.h"

////extern SM_INFO      gSM[MAX_SM_NUM];
extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern int      trcLogId, trcErrLogId;
extern stTimer      *mpTimer;
extern LEG_DATA_SUM	*gpstCallInfo[DEF_STAT_SET_CNT];
extern LEG_CALL_DATA *gpstCallDataPerSec;
extern int mmcdQid;
extern int	msgqTable[MSGQ_MAX_SIZE];
USHORT CVT_USHORT( USHORT value );

// TRACE_INFO.conf 구조체 
extern st_SESSInfo			*gpTrcList[DEF_SET_CNT];
// PDSN.conf 구조체  
extern PDSN_LIST        	*gpPdsnList[DEF_SET_CNT];
// PDSN HASH
extern stHASHOINFO         *gpPdsnHash[DEF_SET_CNT];
// RULESET_LIST.conf 구조체 
extern ST_PBTABLE_LIST  	*gpRsetList[DEF_SET_CNT];
// RULESET_USED.conf 구조체 
extern RULESET_USED_FLAG	*gpRSetUsedList[DEF_SET_CNT];
// CALL_OVER_CTRL.conf 구조체 
extern CPS_OVLD_CTRL		*gpCpsOvldCtrl[DEF_SET_CNT];
// TIMEOUT.conf 구조체 
extern MPTimer            	*gpMPTimer[DEF_SET_CNT];

extern st_NOTI	*gpIdx;

////SMB_HANDLE		bapi;
char    resBuf[4096*2], resHead[4096], resTmp[1024];
extern char    iv_home[64];
extern int      trcFlag, trcLogFlag, trcLogId, trcErrLogId;

extern _mem_check          *gpShmem;
ST_PBTABLE_LIST     stPBTableList;
RuleSetList			g_stRuleSetList[MAX_RULE_SET_LIST];
SM_BUF_CLR			gSmBufClr[MAX_RLEG_CNT];
//st_SESSInfo			stCallTrcList;

int dLoadPBTable(void);
//int dLoad_PBTable_mmcr(IxpcQMsgType *rxIxpcMsg);
int dReadRsetListFile(void);
int dReadRSetUsedFile(void);
int MMCResSndCont (IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag, int seqNo);

extern int dSendNOTIFY(unsigned short uhMsgID, SUBS_INFO *psi);
extern int dWriteFLTIDXFile(void);

int	readRuleSet(void)
{
	FILE *fp = NULL;
	int No = 0, Pbit = 0, Hbit = 0, PkgNo = 0, RedNo = 0, SmsOnOff = 0;
	char buf[128] = {0,};
	RuleSetList *pstRule = &g_stRuleSetList[0];

	if( (fp = fopen(PBTABLE_PATH, "r")) != NULL )
	{
		while( (fgets(buf, sizeof(buf), fp)) != NULL )
		{
			if (buf[0] == '#' )
				continue;
			else
			{
				sscanf( buf, "%d %d %d %d %d %d", &No, &Pbit, &Hbit, &PkgNo, &RedNo, &SmsOnOff );
				pstRule[PkgNo].pkgNo = PkgNo;
				pstRule[PkgNo].pBit = Pbit;
				pstRule[PkgNo].hBit = Hbit;
			}
		}
	}
	else
		return -1;

	fclose(fp);
	return 0;
}

int mmcr_mmc_dis_sess (IxpcQMsgType *rxIxpcMsg)
{
	int				ret;
	int				idx = 0;
	char 			txBuf[MMCMSGSIZE*2];
	MMLReqMsgType   *rxReqMsg;

	time_t			appendTime = 0;
	char			fileName[FILESIZE];
	FILE			*fp=NULL;
	int				complete=0;
	char 			p3dbCmd[256] = {0,};
	int				RuleId=-1, Pbit=-1, Hbit=-1;
	char			day[11] = {0,}, hour[9] = {0,};
	char			mappings_ip[32] = {0,}, subs_id[16] = {0,}, last_update[20] = {0,};
	int				name_index = -1, ip = -1;
	unsigned int    frameIP;
	struct in_addr 	client ;
	char			tmpBuf[256]={0,};

// RULESET_LIST.conf 파일을 읽어서 PageId로 Pbit, Hbit를 구해야 한다. 
	ret = readRuleSet();
	if( ret < 0 )
	{
		return -1;
	}

	memset(txBuf, 0, sizeof(txBuf));

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	if( (!strncasecmp(rxReqMsg->head.para[0].paraName, "IMSI",4) && strlen(rxReqMsg->head.para[0].paraVal) == 15)
	  || (!strncasecmp(rxReqMsg->head.para[1].paraName, "IMSI",4) && strlen(rxReqMsg->head.para[1].paraVal) == 15))
	{
		// , IMSI 와 IMSI가 들어 옴 
		if ((strlen(rxReqMsg->head.para[0].paraVal) == 15) && !strncasecmp(rxReqMsg->head.para[0].paraVal, "4500", 4)) {
			snprintf (subs_id, sizeof(subs_id), rxReqMsg->head.para[0].paraVal); 
		} else if ((strlen(rxReqMsg->head.para[1].paraVal) == 15) && !strncasecmp(rxReqMsg->head.para[1].paraVal, "4500", 4)) { 
			snprintf (subs_id, sizeof(subs_id), rxReqMsg->head.para[1].paraVal); 
		} else {
			sprintf(txBuf+idx, "\n    RESULT = SUCCESS   ");  idx = strlen(txBuf);
			sprintf(txBuf+idx, "\n    -----------------------------------------------------------"); idx = strlen(txBuf);
			sprintf(txBuf+idx, "\n    일치하는 데이터 없음.\n");

			MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 

			return 1;
		}

		appendTime = time(0);

		sprintf(fileName, "%sdis_sess_%s_%ld", SESS_TMP_FILE, subs_id, appendTime);

		// pcube_subs에서 검색 조건을 선택한다. 0 0 1 IMSI
		sprintf(p3dbCmd, "%spcube_subs -d PCube_SM_Repository -u pcube -o pcube -f %s s "
				"0 0 1 %s ",BIN_PATH, fileName, subs_id);

        logPrint(trcErrLogId,FL, "[dis_sess] cmd:%s start\n", p3dbCmd);
		ret = system(p3dbCmd);
		logPrint(trcErrLogId,FL, "[dis_sess] cmd:%s end\n", p3dbCmd); 

		if( ret >= 0 )
		{
			if( (fp = fopen(fileName, "r")) != NULL )
			{
				memset(tmpBuf, 0x00, sizeof(tmpBuf));
				while( fgets(tmpBuf, sizeof(tmpBuf), fp) != NULL )
				{
					if(strstr(tmpBuf, "Command terminated successfully"))
					{
						complete = 1;
						break;
					}
					else
					{
						if(tmpBuf[0] != 0x00)
						{
							sscanf(tmpBuf,"%d\t%s %s",&name_index,day,hour);
							sprintf(last_update,"%s %s", day,hour);
						}
						else
						{
							name_index = -1;
						}
					}
				}
			}
		}
		else
		{
            logPrint(trcErrLogId,FL, "[dis_sess] system() fail. cmd:%s\n", p3dbCmd);
            name_index = -1;
		}

		if( fp != NULL )
		{
			fclose(fp);
			remove(fileName);
		}
		else
			name_index = -1;

		if ( name_index == -1 ) {	// subcriber 없음. 
			sprintf(txBuf+idx, "\n    RESULT = SUCCESS   ");  idx = strlen(txBuf);
			sprintf(txBuf+idx, "\n    -----------------------------------------------------------"); idx = strlen(txBuf);
			sprintf(txBuf+idx, "\n    %s와 일치하는 데이터 없음.\n", subs_id);

			MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 
			return 1;
		}

		// mappings_ip 테이블에 ip 가 존재하는지 검색한다. 
		appendTime = time(0);

		sprintf(fileName, "%sdis_sess_ip_%s_%ld", SESS_TMP_FILE, subs_id, appendTime);

		// pcube_subs에서 검색 조건을 선택한다. 0 0 1 NAME_INDEX -->  mappings_ip 를 얻어온다. 
		sprintf(p3dbCmd, "%spcube_subs -d PCube_SM_Repository -u pcube -o pcube -f %s s "
				"0 0 1 %d ",BIN_PATH, fileName, name_index);

        logPrint(trcErrLogId,FL, "[dis_sess] cmd:%s start\n", p3dbCmd);
		system(p3dbCmd);
		logPrint(trcErrLogId,FL, "[dis_sess] cmd:%s end\n", p3dbCmd);

		if( ret >= 0 )
		{
			if( (fp = fopen(fileName, "r")) != NULL )
			{
				memset(tmpBuf, 0x00, sizeof(tmpBuf));
				while( fgets(tmpBuf, sizeof(tmpBuf), fp) != NULL )
				{
					if(strstr(tmpBuf, "Command terminated successfully"))
					{
						complete = 1;
						break;
					}
					else
					{
						if(tmpBuf[0] != 0x00)
						{
							sscanf(tmpBuf,"%d",&ip);
							printf("ip:%d\n",ip);
							if( ip != -1 )
							{
								frameIP = ip;
								client.s_addr = frameIP;
								strcpy(mappings_ip,inet_ntoa(client));           
							}
							else
								sprintf(mappings_ip, "LOG_OUT");
						}
						else
						{
							sprintf(mappings_ip, "LOG_OUT");
						}
					}
				}
				if( ip == -1 )
					sprintf(mappings_ip, "LOG_OUT");
			}
		}
		else
		{
			logPrint(trcErrLogId,FL, "[dis_sess] system() fail. cmd:%s\n", p3dbCmd);
			fp = NULL;
			sprintf(mappings_ip, "LOG_OUT");
		}

		if( fp != NULL )
			fclose(fp);

		// tunables 테이블에 pkgId 가 존재하는지 검색한다. 
		appendTime = time(0);

		sprintf(fileName, "%sdis_sess_pkg_%s_%ld", SESS_TMP_FILE, subs_id, appendTime);

		// pcube_subs에서 검색 조건을 선택한다. 1 0 1 NAME_INDEX -->  pkgId 를 얻어온다. 
		sprintf(p3dbCmd, "%spcube_subs -d PCube_SM_Repository -u pcube -o pcube -f %s s "
				"1 0 1 %d ",BIN_PATH, fileName, name_index);

		logPrint(trcErrLogId,FL, "[dis_sess] cmd:%s start\n", p3dbCmd);
		ret = system(p3dbCmd);
		logPrint(trcErrLogId,FL, "[dis_sess] cmd:%s end\n", p3dbCmd);

		if( ret >= 0 )
		{
			if( (fp = fopen(fileName, "r")) != NULL )
			{
				memset(tmpBuf, 0x00, sizeof(tmpBuf));
				while( fgets(tmpBuf, sizeof(tmpBuf), fp) != NULL )
				{
					if(strstr(tmpBuf, "Command terminated successfully"))
					{
						complete = 1;
						break;
					}
					else
					{
						if(tmpBuf[0] != 0x00)
						{
							sscanf(tmpBuf,"%d",&RuleId);
							if( g_stRuleSetList[RuleId].pkgNo == RuleId )
							{
								Pbit = g_stRuleSetList[RuleId].pBit;
								Hbit = g_stRuleSetList[RuleId].hBit;
							}
							else
							{
								Pbit = -1; Hbit = -1;
							}
						}
						else
						{
							Pbit = -1; Hbit = -1;
						}
					}
				}
			}
		}
		else
		{
			logPrint(trcErrLogId,FL, "[dis_sess] system() fail. cmd:%s\n", p3dbCmd);
			fp = NULL;
		}

		if( fp != NULL )
		{
			fclose(fp);
			remove(fileName);
		}

		sprintf(txBuf+idx, "\n    RESULT = SUCCESS   ");  idx = strlen(txBuf);
		sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);
		sprintf(txBuf+idx, "\n    %15s %19s %10s %12s", "IMSI", "IP_ADDR", "RULESET_ID", "LAST_UPDATE"); idx = strlen (txBuf);
		sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);
		sprintf(txBuf+idx, "\n    %15s %19s     %02d_%02d %20s ", subs_id, mappings_ip, Pbit, Hbit, last_update); idx = strlen (txBuf);
		sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------\n"); idx = strlen(txBuf);
	} // paraName : IMSI
	

	MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 

	return 1;
}

// DIS-SESS-CNT : 현재 로그인된 가입자 수만 보여준다. 
int mmcr_mmc_dis_sess_cnt (IxpcQMsgType *rxIxpcMsg)
{
	int				ret;
	int				idx = 0;
	char 			txBuf[MMCMSGSIZE*2];
	MMLReqMsgType   *rxReqMsg;

	char			p3dbCmd2[256]={0,};
	time_t			appendTime;
	char			fileName2[FILESIZE];
	FILE			*fp=NULL;
	int				totalCnt=0, complete=0;

	memset(txBuf, 0, sizeof(txBuf));

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

// RULESET_LIST.conf 파일을 읽어서 PageId로 Pbit, Hbit를 구해야 한다. 
	ret = readRuleSet();
	if( ret < 0 )
	{
		return -1;
	}

// total login subscribers cnt
	appendTime = time(0);

	sprintf(fileName2, "%sp3db_total_num_%ld", SESS_TMP_FILE, appendTime);

	sprintf(p3dbCmd2, "%spcube_subs -d PCube_SM_Repository -u pcube -o pcube -f %s s "
			"1 1 1 ",BIN_PATH, fileName2);

	logPrint(trcErrLogId,FL, "[dis_sess_cnt] cmd:%s start\n", p3dbCmd2);
	ret = system(p3dbCmd2);
	logPrint(trcErrLogId,FL, "[dis_sess_cnt] cmd:%s end\n", p3dbCmd2);

	if( ret >= 0 )
	{
		if( (fp = fopen(fileName2, "r")) != NULL )
		{

			while( fgets(resBuf, sizeof(resBuf), fp) != NULL )
			{
				if(strstr(resBuf, "Command terminated successfully"))
				{
					complete = 1;
					break;
				}
				else {
					sscanf(resBuf,"%d",&totalCnt);
				}
			}

		}
	}
	else
	{
		logPrint(trcErrLogId,FL, "[dis_sess_cnt] system() fail. cmd:%s\n", p3dbCmd2);
	}

	if( fp != NULL )
	{
		fclose(fp);
		remove(fileName2);
	}

	sprintf(txBuf+idx, "\n    RESULT = SUCCESS   ");  idx = strlen(txBuf);
	sprintf(txBuf+idx, "\n    -----------------------------------------------------------"); idx = strlen(txBuf);
	sprintf(txBuf+idx, "\n    Total Login Subscribers Count : [ %d ] ",totalCnt); idx = strlen(txBuf);
	sprintf(txBuf+idx, "\n    -----------------------------------------------------------\n"); idx = strlen(txBuf);

	if( complete == 0 )
	{
		sprintf(txBuf+idx, "\n    RESULT = FAIL   ");  idx = strlen(txBuf);
		idx += sprintf(txBuf+idx, "\n	p3db command fail.\n");
	}
	
	MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 

	return 1;
}

int mmcr_mmc_dis_sess_list (IxpcQMsgType *rxIxpcMsg)
{
	int				ret;
	int				idx = 0;
	char 			txBuf[MMCMSGSIZE*2];
	MMLReqMsgType   *rxReqMsg;

	int				RuleId=0, Pbit=-1, Hbit=-1;
	int				Page = 0, PageM = 0, PageN = 0;
	char			p3dbCmd[256]={0,}, p3dbCmd2[256]={0,}; 
	char			IMSI[16]={0,}, IP[20]={0,}, STR_IP[20]={0,}, PKGID[6]={0,};
	char			LAST_UPDATE[20] = {0,}, DAY[11], HOUR[11];
	time_t			appendTime;
	char			fileName[FILESIZE], fileName2[FILESIZE];
	FILE			*fp=NULL;
	int				totalCnt=0, complete=0;
//	int				lineCnt = 0, colCnt = 0, p = 0, i = 0,  find = 0;
	unsigned int    frameIP;
	struct in_addr client ;
	int				pageEnd = 0, alpha = 0;
	int				seqNo = 1;


	memset(txBuf, 0, sizeof(txBuf));

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	// RULESET_LIST.conf 파일을 읽어서 PageId로 Pbit, Hbit를 구해야 한다. 
	ret = readRuleSet();
	if( ret < 0 )
	{
		return -1;
	}

	if( !strncasecmp(rxReqMsg->head.para[0].paraName,"LIST_INDEX",4) || !strncasecmp(rxReqMsg->head.para[1].paraName,"LIST_INDEX",4)) 
	{
		if( !strncasecmp(rxReqMsg->head.para[0].paraName,"LIST_INDEX",4) )
		{
			if( rxReqMsg->head.para[0].paraVal != NULL )
				Page = atoi(rxReqMsg->head.para[0].paraVal); 
			else
				Page = 1;
		}
		else if( !strncasecmp(rxReqMsg->head.para[1].paraName,"LIST_INDEX",4) )
		{
			if( rxReqMsg->head.para[1].paraVal != NULL )
				Page = atoi(rxReqMsg->head.para[1].paraVal); 
			else
				Page = 1;
		}
	}
	else
	{
		Page = 1;
	}

// total login subscribers cnt
	appendTime = time(0);

	sprintf(fileName2, "%sp3db_total_num_%ld", SESS_TMP_FILE, appendTime);

	sprintf(p3dbCmd2, "%spcube_subs -d PCube_SM_Repository -u pcube -o pcube -f %s s "
			"1 1 1 ", BIN_PATH, fileName2);

	logPrint(trcErrLogId,FL, "[dis_sess_list] cmd:%s start\n", p3dbCmd2);
	ret = system(p3dbCmd2);
	logPrint(trcErrLogId,FL, "[dis_sess_list] cmd:%s end\n", p3dbCmd2);

	if( ret >= 0 )
	{
		if( (fp = fopen(fileName2, "r")) != NULL )
		{

			while( fgets(resBuf, sizeof(resBuf), fp) != NULL )
			{
				if(strstr(resBuf, "Command terminated successfully"))
				{
					complete = 1;
					break;
				}
				else
				{
					sscanf(resBuf,"%d",&totalCnt);
				}
			}

		}
	}
	else
	{
		logPrint(trcErrLogId,FL, "[dis_sess_list] system() fail. cmd:%s\n", p3dbCmd2);
		totalCnt = 0;
	}

	if( fp != NULL )
	{
		fclose(fp);
		remove(fileName2);
	}
	complete=0;

	if( totalCnt != 0 )
	{
		pageEnd = totalCnt / PAGE_CNT;
		alpha = totalCnt % PAGE_CNT;
		if(alpha != 0 )
			pageEnd += 1;

		if( Page >= pageEnd )
			Page = pageEnd;
		else if( Page <= 1 )
			Page = 1;

		PageM = PAGE_CNT*(Page-1) + 1; // rows M to N query 의 M 에 해당 
		PageN = PAGE_CNT*Page; // rows M to N query 의 N 에 해당 

		sprintf(fileName, "%sp3db_result_%ld", SESS_TMP_FILE, appendTime);

		sprintf(p3dbCmd, "%spcube_subs -d PCube_SM_Repository -u pcube -o pcube -f %s s "
				"%d %d 1 ",BIN_PATH, fileName, PageM, PageN);

		logPrint(trcErrLogId,FL, "[dis_sess_list] cmd:%s start: %ld\n", p3dbCmd, time(0));
		ret = system(p3dbCmd);
		logPrint(trcErrLogId,FL, "[dis_sess_list] cmd:%s end: %ld\n", p3dbCmd, time(0));

		if( ret >= 0 )
		{
			if( (fp = fopen(fileName, "r")) != NULL )
			{
				sprintf(txBuf+idx, "\n    RESULT = SUCCESS   ");  idx = strlen(txBuf);             
				sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);
				sprintf(txBuf+idx, "\n    LIST_INDEX [%d] - [%d] : CURRENT INDEX [%d]",1,pageEnd,Page);  idx = strlen(txBuf);             
				sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);
				sprintf(txBuf+idx, "\n    %15s %19s %12s %20s", "IMSI", "IP_ADDR", "RULESET_ID", "LAST_UPDATE"); idx = strlen (txBuf);
				sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);

				logPrint(trcErrLogId,FL, "[dis_sess_list] parse start: %ld\n", time(0));

				while( fgets(resBuf, sizeof(resBuf), fp) != NULL )
				{
					if(strstr(resBuf, "Command terminated successfully"))
					{
						complete = 1;
						break;
					}
					else
					{
						sscanf(resBuf,"%s\t%s\t%s %s\t%s",IMSI, STR_IP, DAY, HOUR, PKGID);

						// IMSI field
						IMSI[strlen(IMSI)] = 0;

						// IP field
						STR_IP[strlen(STR_IP)] = 0;
						frameIP = atoi(STR_IP);
						client.s_addr = frameIP;
						memset(IP, 0x00, sizeof(IP));
						strcpy(IP,inet_ntoa(client));           

						// Last Update field
						memset(LAST_UPDATE, 0x00, sizeof(LAST_UPDATE));
						DAY[strlen(DAY)] = 0; HOUR[strlen(HOUR)] = 0; 
						sprintf(LAST_UPDATE,"%s %s",DAY, HOUR);

						// PKG ID field
						PKGID[strlen(PKGID)] = 0;
						RuleId = atoi(PKGID);
						if( g_stRuleSetList[RuleId].pkgNo == RuleId )
						{
							Pbit = g_stRuleSetList[RuleId].pBit;
							Hbit = g_stRuleSetList[RuleId].hBit;
						}
						else
						{
							Pbit = -1;
							Hbit = -1;
						}

						sprintf(txBuf+idx, "\n    %15s %19s       %02d_%02d   %20s ", IMSI, IP, Pbit, Hbit, LAST_UPDATE); 
						idx = strlen (txBuf);

						if( strlen(txBuf)+idx > 4096 )
						{
							////								MMCResSnd(rxIxpcMsg, txBuf, 0, 1); 
							logPrint(trcErrLogId,FL, "MMCResSndCont : %d\n[<!--- %s ---->] \n",seqNo, txBuf);
							MMCResSndCont(rxIxpcMsg, txBuf, 0, 1, seqNo++); 
							memset(txBuf, 0x00, sizeof(txBuf));
						}
					}
				}

				logPrint(trcErrLogId,FL, "[dis_sess_list] parse start: %ld\n", time(0));
						
				if( complete != 1)
				{
					memset(txBuf,0x00,sizeof(txBuf));
					sprintf(txBuf, "\n   RESULT = PARSING FAIL   ");   
				}
				else
				{
					sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);
					sprintf(txBuf+idx, "\n    TOTAL = %d ", totalCnt);  idx = strlen(txBuf);             
					sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------\n"); idx = strlen(txBuf);
				}
			}
		}
		else
		{
			logPrint(trcErrLogId,FL, "[dis_sess_list] system() fail. cmd:%s\n", p3dbCmd);
			fp = NULL;
		}
	}
	else
	{
		sprintf(txBuf+idx, "\n    RESULT = SUCCESS   ");  idx = strlen(txBuf);             
		sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);
		sprintf(txBuf+idx, "\n    LIST_INDEX [0] - [0] : CURRENT INDEX [0]");  idx = strlen(txBuf);             
		sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);
		sprintf(txBuf+idx, "\n    %15s %19s %12s %12s", "IMSI", "IP_ADDR", "RULESET_ID", "LAST_UPDATE"); idx = strlen (txBuf);
		sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);

		sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);
		sprintf(txBuf+idx, "\n    TOTAL = %d ", totalCnt);  idx = strlen(txBuf);             
		sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------\n"); idx = strlen(txBuf);
	}

////	MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 
	logPrint(trcErrLogId,FL, "MMCResSndCont : %d\n[<!--- %s ---->] \n",seqNo, txBuf);
	MMCResSndCont(rxIxpcMsg, txBuf, 0, 0, seqNo++); 

	if( fp != NULL )
	{
		fclose(fp);
		remove(fileName);
	}

	return 1;
}

int cdelay_mmc_stop_delay_check (IxpcQMsgType *rxIxpcMsg)
{
    printf ("cdelay_mmc_start_delay_check \n");
	return 1;
}

int mmcr_mmc_dis_rule_info (IxpcQMsgType *rxIxpcMsg)
{
	printf ("mmcr_mmc_dis_rule_info \n");
	//dLoad_PBTable_mmcr (rxIxpcMsg);
	return 1;
}

int dLog_PDSN_IP(PDSN_LIST *pPDSN, int dNewIdx)
{
	int i;
	struct in_addr in;

	logPrint(trcLogId, FL, "##### NEW_PDSN_INDEX[%d] PDSN IP LIST [COUNT:%u] #####\n", 
							dNewIdx, pPDSN->uiCount);
	for( i = 0; i < pPDSN->uiCount; i++ ) {
		in.s_addr = htonl(pPDSN->uiAddr[i]);
		logPrint(trcLogId, FL, "IDX[%2d]: PDSN=[%s]\n", i, inet_ntoa(in));
	}
	return 0;
}

int dReadPdsnFile(void)
{
	FILE				*fa;
	char				szBuffer[1024], szFName[256];
	char				szIP[16], szValue[32];
	char				*env;
	int					idx=0, dOldIdx, dNewIdx;
	st_Pdsn_HashKey		stPdsnHashKey;
	st_Pdsn_HashKey		*pPdsnHashKey = &stPdsnHashKey;
	st_Pdsn_HashData	stPdsnHashData;
	st_Pdsn_HashData	*pPdsnHashData = &stPdsnHashData;
	stHASHOINFO			*pSetPDSNHASH;
	PDSN_LIST			*pSetPDSNLIST;

	if((env = getenv(IV_HOME)) == NULL){
		logPrint(trcErrLogId, FL, "[dReadPdsnFile] getenv error! \n" );
		return -1;
	}
	sprintf (szFName, "%s/%s", env, PSDN_FILE);

	if((fa=fopen(szFName, "r")) == NULL) { 
		logPrint(trcErrLogId, FL, "[dReadPdsnFile] %s FILE NOT FOUND\n", szFName);
		return -2;
	}       

	dOldIdx = gpIdx->dPdsnIdx;
	switch( dOldIdx )
	{
		case -1:
		case 0 :
			dNewIdx = 1;
			break;
		case 1 :
			dNewIdx = 0;
			break;
		default :
			logPrint(trcErrLogId, FL, "[dReadPdsnFile] FAILED IN dPdsnIdx=%d default\n", dOldIdx);
			dNewIdx = 0;
			break;
	}

	pSetPDSNLIST = gpPdsnList[dNewIdx];
	memset(pSetPDSNLIST, 0x00, DEF_PDSN_LIST_SIZE);
	pSetPDSNHASH = gpPdsnHash[dNewIdx];
	hasho_reset(pSetPDSNHASH);

	while ( fgets( szBuffer, 1024, fa ) != NULL)
	{
		if( szBuffer[0] == '@' )
			continue;

		if( sscanf( &szBuffer[0], "%s %s", szIP, szValue ) == 2 ) {
			if( idx < 32 )
			{
				pSetPDSNLIST->uiAddr[idx] = ntohl(inet_addr(szIP));
				pPdsnHashKey->uiIP = pSetPDSNLIST->uiAddr[idx];
				pPdsnHashData->uiIdx = idx;
				if( hasho_add(pSetPDSNHASH, (U8 *)pPdsnHashKey, (U8 *)pPdsnHashData) == NULL ) {
					logPrint(trcErrLogId, FL, "PDSN HASH FILTER hasho_add NULL IDX=%d IP=%s:%u\n", idx, szIP, pSetPDSNLIST->uiAddr[idx]);
				}
			}
			else
			{
				logPrint(trcErrLogId, FL, "[dReadPdsnFile] %s PDSN COUNT OVER\n", szFName);
				fclose(fa);
				break;
			}
			idx++;
		}
		else {
			logPrint(trcErrLogId, FL, "[dReadPdsnFile] %s FILE FORMAT ERROR\n", szFName);
			fclose(fa);
			return -1;
		}       
	}
	pSetPDSNLIST->uiCount = idx;
	gpIdx->dPdsnCnt = idx;
	gpIdx->dPdsnIdx = dNewIdx;

	fclose(fa);
	dLog_PDSN_IP( pSetPDSNLIST, dNewIdx );

	return dNewIdx;

}

void Log_PBTable(ST_PBTABLE_LIST *pRsetList)
{
	int i, j, no=1;

	logPrint(trcLogId, FL, "#### PBTable LIST [COUNT:%u] ####\n", pRsetList->dCount); 

	for( i=0; i<MAX_PBIT_CNT; i++) {
		for( j=0; j<MAX_HBIT_CNT ; j++) {
			if (pRsetList->stPBTable[i][j].ucUsedFlag) {
				logPrint(trcLogId, FL, "\t%10d %10d %10d %10d %10d\n", 
						no++, i, j,
						pRsetList->stPBTable[i][j].sPkgNo,
						pRsetList->stPBTable[i][j].ucSMSFlag ); 
			}
		}
	}
}

int dReadRsetListFile(void)
{
    FILE			*fa;
    char			szBuffer[1024];
	int				dOldIdx, dNewIdx;		
    USHORT			usNo, usPBit, usHBit;
	ST_PBTABLE_LIST	*pSetRsetList;
	ST_PKG_INFO		stPKGInfo;

    if( (fa=fopen(PBTABLE_PATH, "r")) == NULL) {
		logPrint(trcErrLogId, FL, "dLoad_PBTable_mmcr: %s FILE NOT FOUND\n", PBTABLE_PATH );
        return -1;
    }

	dOldIdx = gpIdx->dRsetListIdx;
	switch( dOldIdx )
	{
		case -1:
		case 0 :
			dNewIdx = 1;
			break;
		case 1 :
			dNewIdx = 0;
			break;
		default :
			logPrint(trcErrLogId, FL, "[dReadRsetListFile] RsetListIdx=%d default\n", dOldIdx);
			dNewIdx = 0;
			break;
	}

	pSetRsetList = gpRsetList[dNewIdx];
    memset(pSetRsetList, 0, DEF_ST_PBTABLE_SIZE);
	memset(&stPKGInfo, 0, DEF_ST_PKG_INFO_SIZE);

    while( fgets( szBuffer, 1024, fa ) != NULL ) {
        if(szBuffer[0] == '#')
            continue;
        if( sscanf( &szBuffer[0], "%hd %hd %hd %hd %hd %c"
                                , &usNo
                                , &usPBit
                                , &usHBit
                                , &stPKGInfo.sPkgNo
                                , &stPKGInfo.sRePkgNo
                                , &stPKGInfo.ucSMSFlag) == 6 )
        {
            if( (usPBit<=MAX_PBIT_CNT) && (usHBit<=MAX_HBIT_CNT) ) {
                pSetRsetList->dCount++;
                stPKGInfo.ucUsedFlag = 1;
                pSetRsetList->stPBTable[usPBit][usHBit] = stPKGInfo;
            } else {
				logPrint(trcErrLogId, FL, "[dReadRsetListFile] VALUE OUT OF RANGE EXCEPTION PBIT[%d] HBIT[%d]\n", usPBit, usHBit );         
                continue;
            }
        } else {
			logPrint(trcErrLogId, FL, "[dReadRsetListFile] %s FILE FORMAT ERROR\n", PBTABLE_PATH );
            fclose(fa);
            return -1;
        }
    }
    fclose(fa);
    Log_PBTable(pSetRsetList);

	gpIdx->dRsetListIdx = dNewIdx;
	return 1;
}

int mmcr_mmc_sync_rule_file( IxpcQMsgType *rxIxpcMsg)
{
	char    mmlBuf[MMCMSGSIZE*2];

	if (dReadRsetListFile() < 0) {
		logPrint(trcErrLogId, FL, "[mmc_sync_rule_file] SYSTEM=%s  APP=%s  Failed Rule Set Sync.\n", mySysName, myAppName );
		sprintf( mmlBuf, "    SYSTEM = %s\n    APP = %s\n    Failed Rule Set Sync.\n", mySysName, myAppName );
	} else {
		logPrint(trcErrLogId, FL, "[mmc_sync_rule_file] SYSTEM=%s  APP=%s  Success Rule Set Sync.\n", mySysName, myAppName );
		sprintf( mmlBuf, "    SYSTEM = %s\n    APP = %s\n    Success Rule Set Sync.\n", mySysName, myAppName );
	}

	dWriteFLTIDXFile();
	dSendNOTIFY(NOTI_PB_TYPE, NULL);
	comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);

	return 1;
}

int mmcr_mmc_sce_log_out( IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	SUBS_INFO 		si;
	int 			flag=0;
	char    		mmlBuf[BUFSIZ];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* Parameter setting && make login feild */
	logPrint(trcLogId, FL, "[mmc_sce_log_out] paraName1:%s paraVal1:%s, paraName2:%s paraVal2:%s\n"
			, rxReqMsg->head.para[0].paraName, rxReqMsg->head.para[0].paraVal
			, rxReqMsg->head.para[1].paraName, rxReqMsg->head.para[1].paraVal);

	if (!strcasecmp(rxReqMsg->head.para[0].paraName, "IMSI")) {
		strcpy(si.szMIN, rxReqMsg->head.para[0].paraVal);
		logPrint(trcLogId, FL, "IMSI:%s\n ", rxReqMsg->head.para[0].paraVal);
		flag+=1;
	}
	if (!strcasecmp(rxReqMsg->head.para[1].paraName, "IP")) {
		strcpy(si.szFramedIP, rxReqMsg->head.para[1].paraVal);
		logPrint(trcLogId, FL, "IP:%s\n "	, si.szFramedIP);
		flag+=2;
	}

	if (flag == 3) {
		dWriteFLTIDXFile();
		dSendNOTIFY(NOTI_TIME_TYPE, &si);
		MMCResSnd(rxIxpcMsg, mmlBuf, 0, 0); 
	}
	else {
		logPrint(trcErrLogId, FL, "MMC][SESS-LOG-OUT] failed logout, invalid aguments\n");
		/* mml response */
		if (flag==1) 
			sprintf( mmlBuf, "    SYSTEM = %s\n    APP = %s\n    FAILED LOGOUT IMSI:%s parameter is wrong\n", mySysName, myAppName, si.szMIN);
		else 
			sprintf( mmlBuf, "    SYSTEM = %s\n    APP = %s\n    FAILED LOGOUT IMSI parameter is wrong\n", mySysName, myAppName);
	}
	comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);

	return 1;
}

int	mmcr_mmc_dis_tmr_info (IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char    		mmlBuf[BUFSIZ];
	MPTimer			*pMPTimer = gpMPTimer[gpIdx->dTimeIdx];

	int	idx=0;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	/* operation */

	/* mml response */
	sprintf( mmlBuf+idx, "\n    SYSTEM = %s\n    APP = %s\n    SUCCESSED    DIS RULESET USE" , mySysName, myAppName); 	idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------"); 						idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    %-10s  %-10d ", "RAD SESS TIMEOUT: ", pMPTimer->sess_timeout ); 						idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    %-10s  %-10d ", "SM  SESS TIMEOUT: ", pMPTimer->sm_sess_timeout ); 						idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    %-10s  %-10d ", "SMS TIMEOUT: ", pMPTimer->sms_timeout ); 								idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------"); 						idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    RESULT = SUCCESS   ");  																idx = strlen(mmlBuf);

	MMCResSnd(rxIxpcMsg, mmlBuf, 0, 0);

	return 0;
}

int	mmcr_mmc_dis_cps_info (IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char    		mmlBuf[BUFSIZ];
	LEG_DATA_SUM	*pCall = NULL;

	int	idx=0, i, cIdx = 0;
	time_t			now, tCallTime;
	struct tm		tmCall;

	now = time(0);
	tCallTime = (now/CALL_UNIT)*CALL_UNIT;
	localtime_r((time_t*)&tCallTime, &tmCall);
	cIdx = tmCall.tm_sec % DEF_STAT_SET_CNT;

	pCall = gpstCallInfo[cIdx];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	/* operation */

	/* mml response */
	sprintf( mmlBuf+idx, "\n    SYSTEM = %s\n    APP = %s\n    SUCCESSED    DIS CPS INFO" , mySysName, myAppName); 	
	idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------"); 						
	idx = strlen(mmlBuf);
	for( i = 0; i < MAX_RLEG_CNT; i++ )
	{
		sprintf( mmlBuf+idx, "\n    %-10s %d = %-10s %-10d %-10s %-10d", "SM CH ID: ", i, 
						"LOG ON", pCall->cps[i].uiLogOnSumCps/5, 
			  			"LOG OUT", pCall->cps[i].uiLogOutSumCps/5 ); 							
		idx = strlen(mmlBuf);
	}
	sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------"); 						
	idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    RESULT = SUCCESS   ");  																
	idx = strlen(mmlBuf);

	MMCResSnd(rxIxpcMsg, mmlBuf, 0, 0); 

	return 0;
}

void dLogTimeOut(MPTimer *pMPTimer)
{
	logPrint (trcLogId,FL, "sess timeout: %u\n", pMPTimer->sess_timeout);
	logPrint (trcLogId,FL, "sm_sess timeout: %u\n", pMPTimer->sm_sess_timeout);
	logPrint (trcLogId,FL, "sms  timeout: %u\n", pMPTimer->sms_timeout);
}


int dReadTimerFile(void)
{
	FILE        	*fa;    
	char        	szBuffer[1024], szFName[256];
	unsigned int	sess_to=0, sm_sess = 0, sms_to=0;
	int				dOldIdx, dNewIdx;
	MPTimer			*pSetMPTimer;

	sprintf (szFName, "%s", DEF_TIMEOUT_FILE);

	if ((fa=fopen(szFName, "r")) == NULL) { 
		logPrint (trcErrLogId,FL, "[dReadTimerFile] %s FILE NOT FOUND\n", szFName);
		return -1;
	}       

	dOldIdx = gpIdx->dTimeIdx;
	switch( dOldIdx )
	{
		case -1:
		case 0 :
			dNewIdx = 1;
			break;
		case 1 :
			dNewIdx = 0;
			break;
		default :
			logPrint(trcErrLogId,FL, "[dReadTimerFile] TimeIdx=%d default\n", dOldIdx);
			dNewIdx = 0;
			break;
	}

	pSetMPTimer = gpMPTimer[dNewIdx];
	memset(pSetMPTimer, 0x00, sizeof(MPTimer));

	while (fgets( szBuffer, 1024, fa ) != NULL)
	{
		if((szBuffer[0] == '@')||(szBuffer[0] == '#')) 
			continue;

		if( sscanf( &szBuffer[0], "%d %d %d", &sess_to, &sm_sess, &sms_to) == 3 ) {
			pSetMPTimer->sess_timeout = sess_to;
			pSetMPTimer->sm_sess_timeout = sm_sess;
			pSetMPTimer->sms_timeout = sms_to;
		}
		else {
			logPrint (trcErrLogId,FL, "[dReadTimerFile] %s FILE FORMAT ERROR\n", szFName);
			fclose(fa);
			return -1;
		}       
	}

	fclose(fa);
	dLogTimeOut(pSetMPTimer);

	gpIdx->dTimeIdx = dNewIdx;
	return dNewIdx;
}

int dWriteTimeOutFile(MPTimer *pMPTimer)
{
	FILE        *fp;    
	char        szFName[256];

	sprintf (szFName, "%s", DEF_TIMEOUT_FILE);

	if ((fp = fopen(szFName, "w"))==NULL){
		logPrint(trcErrLogId, FL, "[dWriteTimeOut] %s write failed\n", szFName);
		return -2;
	}   
 
	fprintf(fp, "@START\n");
	fprintf(fp, "@ %-12s %-12s %-12s\n", "SESSION(sec)", "SM_SESS(sec)", "SMS(sec)");

	fprintf(fp, "   %-10u %-10u %-10u\n", pMPTimer->sess_timeout, pMPTimer->sm_sess_timeout, pMPTimer->sms_timeout);

	fprintf(fp, "@END");
	fclose(fp);
	return 0;
}

int dSetMPTimer(char *tmr_type, time_t tmr_val)
{
	int				dOldIdx, dNewIdx;
	MPTimer			*pSetMPTimer;

	dOldIdx = gpIdx->dTimeIdx;
	switch( dOldIdx )
	{
		case -1:
		case 0 :
			dNewIdx = 1;
			break;
		case 1 :
			dNewIdx = 0;
			break;
		default :
			logPrint(trcErrLogId,FL, "[dSetMPTimer] TimeIdx=%d default\n", dOldIdx);
			dNewIdx = 0;
			break;
	}

	pSetMPTimer = gpMPTimer[dNewIdx];
	memset(pSetMPTimer, 0x00, sizeof(MPTimer));
	dOldIdx = (dNewIdx == 0) ? 1 : 0;

	if (!strcasecmp(tmr_type, "RAD_SESS")) {
		pSetMPTimer->sess_timeout = tmr_val;
		// 나머지 timer는 계속 유지 해야함. 
		pSetMPTimer->sm_sess_timeout = gpMPTimer[dOldIdx]->sm_sess_timeout;
		pSetMPTimer->sms_timeout = gpMPTimer[dOldIdx]->sms_timeout;
		logPrint(trcErrLogId,FL, "[dSetMPTimer] success sess timeout:%d\n", tmr_val);
	}
	else if (!strcasecmp(tmr_type, "SM_SESS")) {
		pSetMPTimer->sm_sess_timeout = tmr_val;
		// 나머지 timer는 계속 유지 해야함. 
		pSetMPTimer->sess_timeout = gpMPTimer[dOldIdx]->sess_timeout;
		pSetMPTimer->sms_timeout = gpMPTimer[dOldIdx]->sms_timeout;
		logPrint(trcErrLogId,FL, "[dSetMPTimer] success sm_sess timeout:%d\n", tmr_val);
	}
	else if (!strcasecmp(tmr_type, "SMS")) {
		pSetMPTimer->sms_timeout = tmr_val;
		// 나머지 timer는 계속 유지 해야함. 
		pSetMPTimer->sess_timeout = gpMPTimer[dOldIdx]->sess_timeout;
		pSetMPTimer->sm_sess_timeout = gpMPTimer[dOldIdx]->sm_sess_timeout;
		logPrint(trcErrLogId,FL, "[dSetMPTimer] success sms_timeout timeout:%d\n", tmr_val);
	}

	dLogTimeOut(pSetMPTimer);

	gpIdx->dTimeIdx = dNewIdx;
	return dNewIdx;
}


int mmcr_mmc_set_tmr_info (IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char    		mmlBuf[BUFSIZ];
	char			tmr_type[10];	
	time_t			tmr_val=0;
	int				idx=0, flag=0, i, dIdx = 0;
	USHORT			usParaCnt=0;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	usParaCnt = CVT_USHORT(rxReqMsg->head.paraCnt);
	/* Parameter setting && make login feild */
	if ((!strcasecmp(rxReqMsg->head.para[0].paraName, ""))
		&& (!strcasecmp(rxReqMsg->head.para[0].paraVal, "")))
	{
		i = 1;
	}
	else i = 0;

	if (!strcasecmp(rxReqMsg->head.para[i].paraName, "TMR_TYPE")) {
		sprintf(tmr_type, "%s",  rxReqMsg->head.para[i].paraVal);
		flag++;
	}
	if (!strcasecmp(rxReqMsg->head.para[i+1].paraName, "TMR_VAL")) {
		tmr_val = atoi(rxReqMsg->head.para[i+1].paraVal);
		flag++;
	}
	if (flag == 2) {
		sprintf( mmlBuf+idx, "\n    SYSTEM = %s\n    APP = %s\n    SUCCESSED    SET-TMR-INFO" , mySysName, myAppName); 		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------"); 						idx = strlen(mmlBuf);

		dIdx = dSetMPTimer(tmr_type, tmr_val);
		/* operation */
		if (!strcasecmp(tmr_type, "RAD_SESS")) {
			sprintf( mmlBuf+idx, "\n    %s  %-10d ", "RAD SESS TIMEOUT: ", gpMPTimer[dIdx]->sess_timeout); 							idx = strlen(mmlBuf);
		}
		else if (!strcasecmp(tmr_type, "SM_SESS")) {
			sprintf( mmlBuf+idx, "\n    %s  %-10d ", "SM  SESS TIMEOUT: ", gpMPTimer[dIdx]->sm_sess_timeout); 								idx = strlen(mmlBuf);
		}
		else if (!strcasecmp(tmr_type, "SMS")) {
			sprintf( mmlBuf+idx, "\n    %s  %-10d ", "SMS TIMEOUT: ", gpMPTimer[dIdx]->sms_timeout); 								idx = strlen(mmlBuf);
		}
		else {
			goto FAIL_CASE;
		}
		/* mml response */
		sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------"); 						idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    RESULT = SUCCESS   ");  																idx = strlen(mmlBuf);

		/* WRITE - RULESET_USED.conf */
		if (dWriteTimeOutFile(gpMPTimer[dIdx]) < 0) {
			logPrint(trcErrLogId,FL, "[mmc_set_tmr_info] set-tmr-info file write failed\n");
		}

		dWriteFLTIDXFile();
		dSendNOTIFY(NOTI_TIME_TYPE, NULL);
		MMCResSnd(rxIxpcMsg, mmlBuf, 0, 0); 
	}
	else {
FAIL_CASE:
		logPrint(trcErrLogId,FL, "[mmc_set_tmr_info] set tmr info failed\n");
		/* mml response */
		sprintf( mmlBuf+idx, "\n    SYSTEM = %s\n    APP = %s\n    FAILED	SET TMR INFO" , mySysName, myAppName); 			idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    RESULT = FAILED   ");  																	idx = strlen(mmlBuf);

		MMCResSnd(rxIxpcMsg, mmlBuf, 0, 0); 
	}

	return 0;
}

int dWrite_TrcConf(st_SESSInfo  *pTrc) 
{
    FILE        *fp;
    char        szFName[256];
    int         i = 0;

    sprintf(szFName, "%s/%s",iv_home, TRACE_INFO_FILE );

    if ((fp = fopen(szFName, "w"))==NULL){
		logPrint(trcErrLogId,FL, "[dWrite_TrcConf] fopen failed:%s\n",szFName);
        return -1;
    }

    fprintf(fp, "@START\n");
    for( i = 0; i < pTrc->dTCount ; i++ )
	{
		fprintf(fp," %d\t%s\t%ld\t%d\n", 
				pTrc->stTrc[i].dType,
				pTrc->stTrc[i].szImsi,
				pTrc->stTrc[i].tRegTime,
				pTrc->stTrc[i].dDura);
	}
    fprintf(fp, "@END");

    fclose(fp);
    return 0;
}

void dLog_TrcConf(st_SESSInfo  *pstCallTrcList)
{
    int i = 0;

    for( i = 0; i < pstCallTrcList->dTCount; i++ )
    {
		logPrint(trcErrLogId,FL,"%-12d %-18s %-16d %-4d\n", 
				pstCallTrcList->stTrc[i].dType,
				pstCallTrcList->stTrc[i].szImsi,
				pstCallTrcList->stTrc[i].tRegTime,
				pstCallTrcList->stTrc[i].dDura);
    }
}

int dReadTrcFile(void)
{
    FILE        *fa;
    char        szBuffer[1024], szFName[256];
    int         cnt = 0;
	int			dOldIdx, dNewIdx;
    char        TYPE[2], IMSI[17], REGTIME[14], DURATION[5];
	st_SESSInfo *pSetTrc;

    sprintf(szFName, "%s/%s",iv_home, TRACE_INFO_FILE );

    if ((fa=fopen(szFName, "r")) == NULL) {
		logPrint(trcErrLogId,FL,"[dReadTrcFile()] fopen fail.[%s]\n",szFName );
        return -1;
    }

	dOldIdx = gpIdx->dTrcIdx;
	switch( dOldIdx )
	{
		case -1:
		case 0 :
			dNewIdx = 1;				
			break;
		case 1 :
			dNewIdx = 0;
			break;
		default :
			logPrint(trcErrLogId, FL, "[dReadTrcFile] dTrcIdx=%d default\n", dOldIdx);
			dNewIdx = 0;
			break;
	}

    cnt = 0;
	pSetTrc = gpTrcList[dNewIdx];
	memset(pSetTrc, 0, sizeof(st_SESSInfo));
    while (fgets( szBuffer, 1024, fa ) != NULL)
    {
        memset(TYPE, 0x00, sizeof(TYPE));
        memset(IMSI, 0x00, sizeof(IMSI));
        memset(REGTIME, 0x00, sizeof(REGTIME));
        memset(DURATION, 0x00, sizeof(DURATION));

        if(szBuffer[0] == '#' || szBuffer[0] == '@')
            continue;

        if( sscanf( &szBuffer[0], "%s %s %s %s", TYPE, IMSI, REGTIME, DURATION) == 4 ) {
			pSetTrc->stTrc[cnt].dType = atoi(TYPE);
			pSetTrc->stTrc[cnt].dDura = atoi(DURATION);
            sprintf(pSetTrc->stTrc[cnt].szImsi, "%s", IMSI);
			pSetTrc->stTrc[cnt].tRegTime = atoi(REGTIME);

			logPrint(trcErrLogId,FL,"%-12d %-18s %-16d %-4d\n", 
				pSetTrc->stTrc[cnt].dType,
				pSetTrc->stTrc[cnt].szImsi,
				pSetTrc->stTrc[cnt].tRegTime,
				pSetTrc->stTrc[cnt].dDura);
       		cnt++;
        }
        else {
            logPrint(trcErrLogId,FL, "[dReadTrcFile] %s FILE FORMAT ERROR! \n", szFName );
			pSetTrc->dTCount = cnt;
			gpIdx->dTrcIdx = dNewIdx;
            fclose(fa);
            return dNewIdx;
        }
    }
	pSetTrc->dTCount = cnt;

    fclose(fa);
	gpIdx->dTrcIdx = dNewIdx;

    return dNewIdx;
}

void getTypeStr(int type, char *trcType)
{
	if( type == TYPE_IMSI )
		sprintf(trcType, "IMSI");
	else
		sprintf(trcType, "IP");
}

void convTimeStr(time_t time_val, char *strTime)
{
	struct tm	*pLocalTime;

	if( (pLocalTime = (struct tm*)localtime((time_t*)&time_val)) == NULL )
	{
		sprintf(strTime, " ");
	}
	else
	{
		strftime(strTime, 32, "%Y-%m-%d %H:%M", pLocalTime);
	}
}

// dura는 분 단위 
void getExpTime(time_t time_val, int dura, char *strTime)
{
	time_t tExpTime = time_val + (dura * 60 );
	struct tm *pLocalTime;

	if( (pLocalTime = (struct tm*)localtime((time_t*)&tExpTime)) == NULL )
	{
		sprintf(strTime, " ");
	}
	else
	{
		strftime(strTime, 32, "%Y-%m-%d %H:%M", pLocalTime);
	}
}


/* REG-CALL-TRC */
int mmcr_mmc_reg_call_trc(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	st_SESSInfo		*pCurTrc;

	int		dRet = 0, idx = 0;
	char 	txBuf[MMCMSGSIZE];

    char    regVal[16] = {0,}, duration[5] = {0,};
	int		dDura = 5, dType = 0;
    int 	i = 0, j = 0, cnt = 0;
    int 	addIndex = 0;
	const int _IMSI_SIZE = 15;
	char 	*dot = NULL;
	int 	dotCnt = 0;
	char	title[5][32] = {"TRACE_TYPE", "TRACE_VAL", "REGISTER_TIME", "TIME_OUT", "EXPIRED_TIME"};
	char   	regTime[20];
	char	expTime[20];
	char	trcType[10];

	memset(txBuf, 0x00, sizeof(txBuf));
	memset(trcType, 0x00, sizeof(trcType));
	memset(regTime, 0x00, sizeof(regTime));
	memset(expTime, 0x00, sizeof(expTime));

    // Parameter setting
	if(	rxReqMsg->head.paraCnt > 4 )
		rxReqMsg->head.paraCnt = 4;

    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
		if( rxReqMsg->head.para[cnt].paraName == NULL )
			continue;

        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);

        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "IP_IMSI"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                sprintf(regVal, "%s", rxReqMsg->head.para[cnt].paraVal);
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "DURATION"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                sprintf(duration, "%s", rxReqMsg->head.para[cnt].paraVal);
				dDura = atoi(duration);
            }
			else
				dDura = 5;
			break;
        }
    }

	dot = strchr(regVal, '.');

	if( (strlen(regVal) == _IMSI_SIZE) && dot == NULL )
	{
		dType = TYPE_IMSI;
	}
	else // IP 인지 검사 
	{
		for(j = 0; j < strlen(regVal); j++ ) // . 개수를 검사 
		{
			if( regVal[j] == '.' )
				dotCnt++;
		}
		// IP 최소 길이 7 에서 최대 길이 15 범위내이고, . 3개면 IP 
		if( strlen(regVal) >= 7 && strlen(regVal) <= 15 && dotCnt == 3 )
		{
			dType = 2; // TYPE_IP 인 경우 
		}
		else
		{
			dType = -1; // 
			sprintf(txBuf+idx,"\n    DSC  RESULT = SUCCESS\n    FAIL REASON = "); idx = strlen(txBuf);
			sprintf(txBuf+idx,"    INVALID IP VALUE[%s]\n", regVal); idx = strlen(txBuf);

			MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 
			return -1;
		}
	}

	if( (dRet = dReadTrcFile()) < 0 )
	{
		logPrint(trcErrLogId,FL, "[mmc_reg_call_trc] dReadTrcFile Fail.\n");
        sprintf(txBuf+idx,"\n    DSC  RESULT = SUCCESS\n    FAIL REASON = "); idx = strlen(txBuf);
        sprintf(txBuf+idx,"    TRACE FILE READ FAIL.\n"); idx = strlen(txBuf);

		MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 
        return -2;
	}

	if( dRet >= 0 && dRet <= 1)
		pCurTrc = gpTrcList[dRet];
	else
	{
		logPrint(trcErrLogId,FL, "[mmc_reg_call_trc] dReadTrcFile Fail.\n");
        sprintf(txBuf+idx,"\n    DSC  RESULT = SUCCESS\n    FAIL REASON = "); idx = strlen(txBuf);
        sprintf(txBuf+idx,"    TRACE FILE READ FAIL.\n"); idx = strlen(txBuf);

		MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 
		return -3;
	}

    if( pCurTrc->dTCount == MAX_TRACE_NUM )
    {
		logPrint(trcErrLogId,FL, "[mmc_reg_call_trc] max trace register number limit over.\n");

        sprintf(txBuf+idx,"\n    DSC  RESULT = SUCCESS\n    FAIL REASON = "); idx = strlen(txBuf);
        sprintf(txBuf+idx,"    MAX TRACE REGISTER NUMBER LIMIT OVER.\n"); idx = strlen(txBuf);

		MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 
        return -3;
    }
    else
    {
        // Check aleady exist Trace List 
        for( i = 0; i < pCurTrc->dTCount; i++ )
        {
            if( !strcmp( regVal, pCurTrc->stTrc[i].szImsi ) )
            {
				logPrint(trcErrLogId,FL, "[mmc_reg_call_trc] aleady exist trace list.\n");

                sprintf(txBuf+idx,"\n    DSC  RESULT = SUCCESS\n    FAIL REASON = "); 	idx = strlen(txBuf);
                sprintf(txBuf+idx,"    Aleady Exist Trace List [%s]\n", regVal); 	idx = strlen(txBuf);
				MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 
                return -4;
            }
        }

        addIndex = pCurTrc->dTCount;  // 배열 마지막 에 항상 추가된다. 

		pCurTrc->dTCount++;
		sprintf(pCurTrc->stTrc[addIndex].szImsi, "%s", regVal);
		pCurTrc->stTrc[addIndex].tRegTime = time(0);
		convTimeStr(pCurTrc->stTrc[addIndex].tRegTime,regTime);
		pCurTrc->stTrc[addIndex].dType = dType;
		getTypeStr(pCurTrc->stTrc[addIndex].dType, trcType);
		pCurTrc->stTrc[addIndex].dDura = dDura;
		getExpTime(pCurTrc->stTrc[addIndex].tRegTime, pCurTrc->stTrc[addIndex].dDura,expTime);
    }

    if( dWrite_TrcConf(pCurTrc) < 0 ) {
		logPrint(trcErrLogId, FL, "[mmc_reg_call_trc] reg-call-trc file write failed\n");
		sprintf(txBuf+idx,"\n    DSC  RESULT = SUCCESS\n    FAIL REASON = "); 	idx = strlen(txBuf);
		sprintf(txBuf+idx,"    Write Trace File Fail.[%s]\n", regVal); 	idx = strlen(txBuf);
		MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 
		return -5;
	}


	dWriteFLTIDXFile();
	dSendNOTIFY(NOTI_TRACE_TYPE, NULL);

    sprintf(txBuf+idx,"\n    ====================== NEW REGISTER TRACE INFO ======================="); idx = strlen(txBuf);
    sprintf(txBuf+idx,"\n    %-12s %-18s %-16s %-8s %-16s",title[0],title[1],title[2],title[3],title[4]); idx = strlen(txBuf);
    sprintf(txBuf+idx,"\n    ======================================================================"); idx = strlen(txBuf);

	sprintf(txBuf+idx,"\n    %-12s %-18s %-16s %-8d %-16s", trcType,
			pCurTrc->stTrc[addIndex].szImsi, regTime,
			pCurTrc->stTrc[addIndex].dDura, expTime);

	logPrint(trcErrLogId,FL, "\n    %-12s %-18s %-16s %-8d %-16s\n", trcType,
			pCurTrc->stTrc[addIndex].szImsi, regTime,
			pCurTrc->stTrc[addIndex].dDura, expTime);

	idx = strlen(txBuf);

    sprintf(txBuf+idx,"\n    ======================================================================\n"); idx = strlen(txBuf);

	MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 

    return 0;
}


/* CANC-CALL-TRC */
int mmcr_mmc_canc_call_trc(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	st_SESSInfo		*pCurTrc;

	int		idx = 0;
	char 	txBuf[MMCMSGSIZE] = {0,};
	char	title[5][32] = {"TRACE_TYPE", "TRACE_VAL", "REGISTER_TIME", "TIME_OUT", "EXPIRED_TIME"};

    char    cancVal[16] = {0,};
    int 	dRet = 0, i = 0, cnt = 0;
    int delIndex = 0;

	char regTime[20];
	char expTime[20];
	char trcType[10];

	memset(txBuf, 0x00, sizeof(txBuf));
	memset(trcType, 0x00, sizeof(trcType));
	memset(regTime, 0x00, sizeof(regTime));
	memset(expTime, 0x00, sizeof(expTime));

    // Parameter setting
	if(	rxReqMsg->head.paraCnt > 4 )
		rxReqMsg->head.paraCnt = 4;

    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
		if( rxReqMsg->head.para[cnt].paraName == NULL )
			continue;

        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);

        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "IP_IMSI"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                sprintf(cancVal, "%s", rxReqMsg->head.para[cnt].paraVal);
            }
        }
    }

	if( (dRet = dReadTrcFile()) < 0 )
	{
		logPrint(trcErrLogId,FL, "[mmc_canc_call_trc] dReadTrcFile Fail.\n");
        sprintf(txBuf+idx,"\n    DSC  RESULT = SUCCESS\n    FAIL REASON = "); idx = strlen(txBuf);
        sprintf(txBuf+idx,"    TRACE FILE READ FAIL.\n"); idx = strlen(txBuf);

		MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 
        return -1;
	}

	pCurTrc = gpTrcList[dRet];

    // Find Del CANCEL LIST 
    for( i = 0; i < pCurTrc->dTCount; i++ )
    {
        if( !strcmp( cancVal, pCurTrc->stTrc[i].szImsi ) )
        {
            delIndex = i;
            break;
        }
    }

	if( pCurTrc->dTCount == 0 )
	{
        sprintf(txBuf+idx,"\n    DSC  RESULT = FAIL\n    FAIL REASON = "); idx = strlen(txBuf);
        sprintf(txBuf+idx,"    NOT REGISTERED TRACE LIST\n"); idx = strlen(txBuf);

		MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 
		return -2;
	}
	else if( i == pCurTrc->dTCount )
	{
		logPrint(trcErrLogId,FL, "[mmc_reg_call_trc] not exist trace list [%s]\n",cancVal);

        sprintf(txBuf+idx,"\n    DSC  RESULT = FAIL\n    FAIL REASON = "); idx = strlen(txBuf);
        sprintf(txBuf+idx,"\n    NOT EXIST TRACE LIST[%s]", cancVal); idx = strlen(txBuf);

		MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 
		return -3;
	}
    delIndex = i; 
	getTypeStr(pCurTrc->stTrc[delIndex].dType, trcType),
	convTimeStr(pCurTrc->stTrc[delIndex].tRegTime,regTime);
	getExpTime(pCurTrc->stTrc[delIndex].tRegTime, pCurTrc->stTrc[delIndex].dDura,expTime);

    sprintf(txBuf+idx,"\n    =================== CANCEL REGISTER TRACE INFO ======================="); idx = strlen(txBuf);
    sprintf(txBuf+idx,"\n    %-12s %-18s %-16s %-8s %-16s",title[0],title[1],title[2],title[3],title[4]); idx = strlen(txBuf);
    sprintf(txBuf+idx,"\n    ======================================================================"); idx = strlen(txBuf);

	sprintf(txBuf+idx,"\n    %-12s %-18s %-16s %-8d %-16s", 
			trcType,
			pCurTrc->stTrc[delIndex].szImsi,
			regTime,
			pCurTrc->stTrc[delIndex].dDura,
			expTime);

	idx = strlen(txBuf);

    sprintf(txBuf+idx,"\n    ======================================================================\n"); idx = strlen(txBuf);


    for( i = delIndex; i < pCurTrc->dTCount-1; i++ )
    {
        memset(pCurTrc->stTrc[i].szImsi, 0x00, sizeof(pCurTrc->stTrc[i].szImsi));
        sprintf(pCurTrc->stTrc[i].szImsi, "%s", pCurTrc->stTrc[i+1].szImsi);
		pCurTrc->stTrc[i].tRegTime = pCurTrc->stTrc[i+1].tRegTime;
		pCurTrc->stTrc[i].dType = pCurTrc->stTrc[i+1].dType;
		pCurTrc->stTrc[i].dDura = pCurTrc->stTrc[i+1].dDura;
    }

	// 제일 마지막에 남아 있는 놈은 지워준다. 
    memset(pCurTrc->stTrc[i].szImsi, 0x00, sizeof(pCurTrc->stTrc[i].szImsi));
	pCurTrc->stTrc[i].tRegTime = 0;
	pCurTrc->stTrc[i].dType = 0;
	pCurTrc->stTrc[i].dDura = 0;

    pCurTrc->dTCount--;

    if( dWrite_TrcConf(pCurTrc) < 0 ) {
		logPrint(trcErrLogId, FL, "[mmc_canc_call_trc] canc-call-trc file write failed\n");
		sprintf(txBuf+idx,"\n    DSC  RESULT = SUCCESS\n    FAIL REASON = "); 	idx = strlen(txBuf);
		sprintf(txBuf+idx,"    Write Trace File Fail.[%s]\n", cancVal); 	idx = strlen(txBuf);
		MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 
		return -4;
	}

	dWriteFLTIDXFile();
	dSendNOTIFY(NOTI_TRACE_TYPE, NULL);

	MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 

    return 0;
}

/* DIS-CALL-TRC */
int mmcr_mmc_dis_call_trc(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	st_SESSInfo		*pCurTrc = gpTrcList[gpIdx->dTrcIdx];

	int		idx = 0;
	char 	txBuf[MMCMSGSIZE];
    int 	i = 0;
	char	title[6][32] = {"NO", "TRACE_TYPE", "TRACE_VAL", "REGISTER_TIME", "TIME_OUT", "EXPIRED_TIME"};
	char	regTime[20];
	char	expTime[20];
	char	trcType[10];

	memset(txBuf, 0x00, sizeof(txBuf));
	memset(trcType, 0x00, sizeof(trcType));
	memset(regTime, 0x00, sizeof(regTime));
	memset(expTime, 0x00, sizeof(expTime));

    sprintf(txBuf+idx,"\n    ======================================================================"); idx = strlen(txBuf);
    sprintf(txBuf+idx,"\n    %-4s %-12s %-18s %-16s %-8s %-16s",title[0],title[1],title[2],title[3],title[4],title[5]); idx = strlen(txBuf);
    sprintf(txBuf+idx,"\n    ======================================================================"); idx = strlen(txBuf);

    for( i = 0; i < pCurTrc->dTCount; i++ )
    {
		getTypeStr(pCurTrc->stTrc[i].dType, trcType),
		convTimeStr(pCurTrc->stTrc[i].tRegTime,regTime );
		getExpTime(pCurTrc->stTrc[i].tRegTime, pCurTrc->stTrc[i].dDura,expTime);

        sprintf(txBuf+idx,"\n    %-4d %-12s %-18s %-16s   %-4d   %-16s", 
				i,
				trcType,
				pCurTrc->stTrc[i].szImsi,
				regTime, 
				pCurTrc->stTrc[i].dDura,
				expTime);
		idx = strlen(txBuf);
    }

    if( pCurTrc->dTCount == 0 )
        sprintf(txBuf+idx,"\n    NO TRACE LIST."); idx = strlen(txBuf);

    sprintf(txBuf+idx,"\n    ======================================================================\n"); idx = strlen(txBuf);

	MMCResSnd(rxIxpcMsg, txBuf, 0, 0); 

    return 0;
}

int dGetIndex(char *chName)
{
	int i, len;

	if(chName[0] == '\0')
		return -1;

	len = strlen(chName);

	for(i = len-1; i >= 0; --i)
	{
		if(chName[i] >= '0' && chName[i] <= '9')
			break;
	}

	if(i < 0)
		return -1;
	return atoi(&chName[i]);
}

int SendToRLEG(int chID, SM_BUF_CLR *pSmBufClr, int sid)
{
	int             dRet;
	int             dSize;
	GeneralQMsgType txGenQMsg;
	pst_MsgQ        pstMsgQ;
	pst_MsgQSub     pstMsgQSub;

	txGenQMsg.mtype = MTYPE_SM_BUF_CLR;

	pstMsgQ = (st_MsgQ *)&txGenQMsg.body;
	pstMsgQ->llMType = 0;
	pstMsgQSub = (st_MsgQSub *)&pstMsgQ->llMType;

	pstMsgQSub->usType  = 0;
	pstMsgQSub->usSvcID = sid;

	pstMsgQ->usBodyLen = sizeof(SM_BUF_CLR);

	memcpy( &pstMsgQ->szBody[0], pSmBufClr, sizeof(SM_BUF_CLR) );

	/* send subs info msg */
	dSize = sizeof(st_MsgQ) - MAX_MSGBODY_SIZE + pstMsgQ->usBodyLen;
	if( (dRet = msgsnd( msgqTable[3] + chID, &txGenQMsg, dSize, 0)) < 0 )
	{
		logPrint(trcLogId, FL, "[SendToRLEG]Qid = %d, chID=%d, sid=%d] ERROR SEND : %d[%s]\n",
							msgqTable[4]+chID, chID, sid, errno, strerror(errno));
		return -1;
	}

	logPrint(trcLogId, FL, "[SendToRLEG] Qid=%d chID=%s sid=%d\n",
						msgqTable[4]+chID, chID, sid);
	return 0;
}

unsigned int calc_smbuf_clear_pediod (SM_BUF_CLR *p, int curHour)
{
	int dSumHour=0;
	unsigned int uiSumMiliSec=0;

	if (p->dHour > curHour) {
		dSumHour = (p->dPeriod * DAY2HOUR) + (p->dHour - curHour);
	}
	else if (p->dHour == curHour) {
		dSumHour = (p->dPeriod * DAY2HOUR);
	}
	else {
		dSumHour = (p->dPeriod * DAY2HOUR) - (curHour - p->dHour);
	}
	uiSumMiliSec = dSumHour * HOUR2MILISEC;
	return uiSumMiliSec;
}


// BY JUNE, 2011-03-16
int mmcr_mmc_set_smbuf_clr(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char mmlBuf[BUFSIZ];
	int i, dRet = 0, dActTime=0, dPeriod=0, dChID = -1, buflen = 0;
	int dFail = 0;
	unsigned int uiClrPeriod;
	time_t      cur_time;
	struct tm*  pcur_tm;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* Parameter setting && make login feild */
	logPrint(trcLogId, FL, "[mmc_set_smbuf_clr] paraName1:%s paraVal1:%s, paraName2:%s paraVal2:%s\n"
			, rxReqMsg->head.para[0].paraName, rxReqMsg->head.para[0].paraVal
			, rxReqMsg->head.para[1].paraName, rxReqMsg->head.para[1].paraVal);

	// CHECK CH ID PARAMETER
	if (!strcasecmp(rxReqMsg->head.para[0].paraName, "CH_ID")) {
		if(!strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL")) 
			dChID = MAX_RLEG_CNT;	
		else
			dChID = dGetIndex(rxReqMsg->head.para[0].paraVal);

		if (dChID < 0) {
			sprintf (mmlBuf, "\n    RESULT = FAIL \n    FAIL  REASON = INVALID PARAMETER (PARAM VAL=%s)\n", rxReqMsg->head.para[0].paraVal);
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
			return 1;
		}
	}
	else {
		sprintf (mmlBuf, "\n    RESULT = FAIL \n    FAIL  REASON = UNKNOWN PARAMETER (PARAM NAME=%s)\n", rxReqMsg->head.para[0].paraName);
		comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		return 1;
	}

	// CHAECK PERIOD PARAMETOR
	if (!strcasecmp(rxReqMsg->head.para[0].paraName, "PERIOD")) {
		dPeriod = atoi(rxReqMsg->head.para[0].paraVal);
		if ((dPeriod< 1) && (dPeriod > 30)) {
			sprintf (mmlBuf, "\n    RESULT = FAIL \n    FAIL  REASON = INVALID PARAMETER (PARAM VAL=%s)\n", rxReqMsg->head.para[0].paraVal);
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
			return 1;
		}
	}
	else {
		sprintf (mmlBuf, "\n    RESULT = FAIL \n    FAIL  REASON = UNKNOWN PARAMETER (PARAM NAME=%s)\n", rxReqMsg->head.para[0].paraName);
		comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		return 1;
	}

	// CHAECK TIME PARAMETOR
	if (!strcasecmp(rxReqMsg->head.para[1].paraName, "HOUR")) {
		dActTime = atoi(rxReqMsg->head.para[1].paraVal);
		if ((dActTime < 1) && (dActTime > 24)) {
			sprintf (mmlBuf, "\n    RESULT = FAIL \n    FAIL  REASON = INVALID PARAMETER (HOUR=%s)\n", rxReqMsg->head.para[1].paraVal);
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
			return 1;
		}
	}
	else {
		sprintf (mmlBuf, "\n    RESULT = FAIL \n    FAIL  REASON = UNKNOWN PARAMETER (HOUR=%s)\n",
				rxReqMsg->head.para[1].paraVal);
		comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		return 1;
	}

	// SET SMBUFFER CLEAR (PERIOD & TIME)
	if( dChID == MAX_RLEG_CNT )
	{
		for( i = 0; i < MAX_RLEG_CNT; i++ )
		{
			gSmBufClr[i].dUsed		= CMD_USE_ON;
			gSmBufClr[i].dPeriod 	= dPeriod;
			gSmBufClr[i].dHour		= dActTime;

			cur_time  = time ((time_t *)0);
			pcur_tm = (struct tm*)localtime((time_t*)&cur_time);

			uiClrPeriod = calc_smbuf_clear_pediod (&gSmBufClr[i], pcur_tm->tm_hour);
			if (uiClrPeriod>0) {
				gSmBufClr[i].uiClrPeriod = uiClrPeriod;
				if( (dRet = SendToRLEG(i, &gSmBufClr[i], SID_SET_SM_BUF_CLR)) < 0 )
				{
					logPrint(trcLogId, FL, "[mmc_set_smbuf_clr] SendToRLEG chID=%d FAIL. dRet=%d\n", i, dRet);
					if( buflen == 0 )
						buflen += sprintf(mmlBuf+buflen, "\n    RESULT = FAIL \n    FAIL  REASON = SendToRLEG(chID=%d ", i);
					else
						buflen += sprintf(mmlBuf+buflen, "chID=%d ", i);
					dFail++;
				}
			}
			else {
				logPrint(trcLogId, FL, "[mmc_set_smbuf_clr] SET SMBUF CLEAR FAILED, PERIOD=%s HOUR=%d CALC:%u\n" , gSmBufClr[i].dPeriod, gSmBufClr[i].dHour, uiClrPeriod);
				sprintf(mmlBuf, "\n    RESULT = FAIL \n    FAIL  REASON = INVALID CLEAR PERIOD(PERIOD=%d HOURE=%d)\n" , gSmBufClr[i].dPeriod, gSmBufClr[i].dHour);
				comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
				return 1;
			}
		}
		if( dFail == 0 )
		{
			logPrint(trcLogId, FL, "[mmc_set_smbuf_clr] SET SMBUF CLEAR SUCCESSED, PERIOD=%d HOUR=%d CALC:%u\n" , gSmBufClr[0].dPeriod, gSmBufClr[0].dHour, uiClrPeriod);
			sprintf(mmlBuf, "    SYSTEM = %s\n    APP = %s\n    SUCCESSED     SET SMBUF CLEAR, PERIOD=%d HOUR=%d" , mySysName, myAppName, gSmBufClr[0].dPeriod, gSmBufClr[0].dHour);
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		}
		else
		{
			buflen += sprintf(mmlBuf+buflen, ")\n");
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		}
	}
	else
	{
		gSmBufClr[dChID].dUsed		= CMD_USE_ON;
		gSmBufClr[dChID].dPeriod 	= dPeriod;
		gSmBufClr[dChID].dHour		= dActTime;

		cur_time  = time ((time_t *)0);
		pcur_tm = (struct tm*)localtime((time_t*)&cur_time);

		uiClrPeriod = calc_smbuf_clear_pediod (&gSmBufClr[dChID], pcur_tm->tm_hour);
		if (uiClrPeriod>0) {
			gSmBufClr[dChID].uiClrPeriod = uiClrPeriod;
			if( (dRet = SendToRLEG(dChID, &gSmBufClr[dChID], SID_SET_SM_BUF_CLR)) < 0 )
			{
				logPrint(trcLogId, FL, "[mmc_set_smbuf_clr] SendToRLEG chID=%d FAIL. dRet=%d\n", dChID, dRet);
				buflen += sprintf(mmlBuf+buflen, "\n    RESULT = FAIL \n    FAIL  REASON = SendToRLEG(chID=%d)\n", dChID);
				dFail++;
			}
		}
		else {
			logPrint(trcLogId, FL, "[mmc_set_smbuf_clr] SET SMBUF CLEAR FAILED, PERIOD=%s HOUR=%d CALC:%u\n" , gSmBufClr[dChID].dPeriod, gSmBufClr[dChID].dHour, uiClrPeriod);
			sprintf(mmlBuf, "\n    RESULT = FAIL \n    FAIL  REASON = INVALID CLEAR PERIOD(PERIOD=%d HOURE=%d)\n" , gSmBufClr[dChID].dPeriod, gSmBufClr[dChID].dHour);
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
			return 1;
		}
		if( dFail == 0 )
		{
			logPrint(trcLogId, FL, "[mmc_set_smbuf_clr] SET SMBUF CLEAR SUCCESSED, PERIOD=%d HOUR=%d CALC:%u\n" , gSmBufClr[dChID].dPeriod, gSmBufClr[dChID].dHour, uiClrPeriod);
			sprintf(mmlBuf, "    SYSTEM = %s\n    APP = %s\n    SUCCESSED     SET SMBUF CLEAR, PERIOD=%d HOUR=%d" , mySysName, myAppName, gSmBufClr[dChID].dPeriod, gSmBufClr[dChID].dHour);
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		}
		else
		{
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		}
	}
	

	return 0;
}

// BY JUNE, 2011-03-16
int mmcr_mmc_del_smbuf_clr (IxpcQMsgType *rxIxpcMsg)
{
	int i, dChID = -1, dRet = 0, buflen = 0; 
	int dFail = 0;
	MMLReqMsgType   *rxReqMsg;
	char mmlBuf[BUFSIZ];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* Parameter setting && make login feild */
	logPrint(trcLogId, FL, "[mmc_del_smbuf_clr] paraName1:%s paraVal1:%s\n"
			, rxReqMsg->head.para[0].paraName, rxReqMsg->head.para[0].paraVal);

	// CHECK CH ID PARAMETER
	if (!strcasecmp(rxReqMsg->head.para[0].paraName, "CH_ID")) {
		if(!strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL")) 
			dChID = MAX_RLEG_CNT;	
		else
			dChID = dGetIndex(rxReqMsg->head.para[0].paraVal);

		if (dChID < 0) {
			sprintf (mmlBuf, "\n    RESULT = FAIL \n    FAIL  REASON = INVALID PARAMETER (PARAM VAL=%s)\n", rxReqMsg->head.para[0].paraVal);
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
			return 1;
		}
	}
	else {
		sprintf (mmlBuf, "\n    RESULT = FAIL \n    FAIL  REASON = UNKNOWN PARAMETER (PARAM NAME=%s)\n", rxReqMsg->head.para[0].paraName);
		comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		return 1;
	}


	if( dChID == MAX_RLEG_CNT )
	{
		for(i = 0; i < MAX_RLEG_CNT; i++)
		{
			// SET SMBUFFER CLEAR (PERIOD & TIME)
			gSmBufClr[i].dUsed		= CMD_USE_OFF;
			gSmBufClr[i].dPeriod 	= 0;
			gSmBufClr[i].dHour		= 0;
			gSmBufClr[i].uiClrPeriod = 0;

			if( (dRet = SendToRLEG(i, &gSmBufClr[i], SID_DEL_SM_BUF_CLR)) < 0 )
			{
				logPrint(trcErrLogId, FL, "[mmc_del_smbuf_clr] SendToRLEG chID=%d FAIL. dRet=%d\n", i, dRet);
				if( buflen == 0 )
					buflen += sprintf(mmlBuf+buflen, "\n    RESULT = FAIL \n    FAIL  REASON = SendToRLEG(chID=%d ", i);
				else
					buflen += sprintf(mmlBuf+buflen, "chID=%d ", i);

				dFail++;
			}
		}
		if( dFail == 0 )
		{
			logPrint(trcLogId, FL, "[mmc_del_smbuf_clr] DEL SMBUF CLEAR SUCCESSED\n");
			sprintf(mmlBuf, "    SYSTEM = %s\n    APP = %s\n    SUCCESSED     DEL SMBUF CLEAR.", mySysName, myAppName);
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		}
		else
		{
			buflen += sprintf(mmlBuf+buflen, ")\n");
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		}
	}
	else
	{
		// SET SMBUFFER CLEAR (PERIOD & TIME)
		gSmBufClr[dChID].dUsed		= CMD_USE_OFF;
		gSmBufClr[dChID].dPeriod 	= 0;
		gSmBufClr[dChID].dHour		= 0;

		if( (dRet = SendToRLEG(dChID, &gSmBufClr[dChID], SID_DEL_SM_BUF_CLR)) < 0 )
		{
			logPrint(trcErrLogId, FL, "[mmc_del_smbuf_clr] SendToRLEG chID=%d FAIL. dRet=%d\n", dChID, dRet);
			buflen += sprintf(mmlBuf+buflen, "\n    RESULT = FAIL \n    FAIL  REASON = SendToRLEG(chID=%d)\n", dChID);
			dFail++;
		}
		if( dFail == 0 )
		{
			logPrint(trcLogId, FL, "[mmc_del_smbuf_clr] DEL SMBUF CLEAR SUCCESSED\n");
			sprintf(mmlBuf, "    SYSTEM = %s\n    APP = %s\n    SUCCESSED     DEL SMBUF CLEAR.", mySysName, myAppName);
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		}
		else
		{
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		}

	}

	return 0;
}

// BY JUNE, 2011-03-16
int mmcr_mmc_dis_smbuf_clr (IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char mmlBuf[BUFSIZ];
	int i;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* Parameter setting && make login feild */
	logPrint(trcLogId, FL, "[mmc_dis_smbuf_clr] paraName1:%s paraVal1:%s\n"
			, rxReqMsg->head.para[0].paraName, rxReqMsg->head.para[0].paraVal);

	for( i = 0; i < MAX_RLEG_CNT; i++ )
	{
		logPrint(trcLogId, FL, "[mmc_dis_smbuf_clr] DIS SMBUF CLEAR SUCCESSED, USED=%d PERIOD=%d HOUR=%d\n"
							, gSmBufClr[i].dUsed, gSmBufClr[i].dPeriod, gSmBufClr[i].dHour);
		if (gSmBufClr[i].dUsed == CMD_USE_ON) {
			sprintf(mmlBuf, "    SYSTEM = %s\n    CH_ID = %d\n    SUCCESSED     DIS SMBUF CLEAR, COMMAND IS ENABLE(PERIOD=%d HOUR=%d)" , mySysName, i, gSmBufClr[i].dPeriod, gSmBufClr[i].dHour);
		}
		else {
			sprintf(mmlBuf, "    SYSTEM = %s\n    CH_ID = %d\n    SUCCESSED     DIS SMBUF CLEAR, COMMAND IS DISABLE" , mySysName, i);
		}
	}
	comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);

	return 0;
}

void dLogCpsOvld(CPS_OVLD_CTRL *pCpsOvld)
{
	logPrint(trcLogId, FL, "[dLogCpsOvld] CPS_OVER = %d CPS_RATE = %d\n",
					pCpsOvld->over_cps, pCpsOvld->over_rate);
}

int dReadCpsOvldFile(void)
{
	FILE        *fa;
	char        szBuffer[1024], szFName[256];
	int         dCps=0, dRate=0, dOldIdx, dNewIdx;
	char        *env;
	CPS_OVLD_CTRL *pSetCpsOvld;

	if ((env = getenv(IV_HOME)) == NULL){
		logPrint(trcErrLogId, FL, "[dReadCpsOvldFile] getenv error!\n" );
		return -1;
	}
	sprintf (szFName, "%s/%s", env, CALL_OVER_CTRL_FILE);

	if ((fa=fopen(szFName, "r")) == NULL) {
		logPrint(trcErrLogId, FL, "[dReadCpsOvldFile] %s FILE NOT FOUND\n", szFName);
		return -2;
	}

	dOldIdx = gpIdx->dCpsIdx;
	switch( dOldIdx )
	{
		case -1:
		case 0 :
			dNewIdx = 1;		
			break;
		case 1 :
			dNewIdx = 0;
			break;
		default :
			logPrint(trcErrLogId, FL, "[dReadCpsOvldFile] default dCpsIdx=%d\n", dOldIdx);
	}

	pSetCpsOvld = gpCpsOvldCtrl[dNewIdx];
	memset(pSetCpsOvld, 0, sizeof(CPS_OVLD_CTRL));
	while (fgets( szBuffer, 1024, fa ) != NULL)
	{
		if(szBuffer[0] == '@')
			continue;

		if( sscanf( &szBuffer[0], "%d %d", &dCps, &dRate) == 2 ) {
			pSetCpsOvld->over_flag = 0;
			pSetCpsOvld->over_cps  = ntohl(dCps);
			pSetCpsOvld->over_rate = ntohl(dRate);
		}
		else {
			logPrint(trcErrLogId, FL, "[dReadCpsOvldFile] %s FILE FORMAT ERROR\n", szFName);
			fclose(fa);
			return -1;
		}
	}
	fclose(fa);
	dLogCpsOvld(pSetCpsOvld);

	gpIdx->dCpsIdx = dNewIdx;
	return dNewIdx;
}


int dWrite_CallOverCtrl(CPS_OVLD_CTRL *pCurCpsOvld)
{
	FILE        *fp;    
	char        szFName[256];
	char		*env;

	if ((env = getenv(IV_HOME)) == NULL){
		logPrint(trcErrLogId, FL, "[dWrite_CallOverCtrl] getenv error! \n" );
		return -1;
	}
	sprintf (szFName, "%s/%s", env, CALL_OVER_CTRL_FILE);

	if ((fp = fopen(szFName, "w"))==NULL){
		logPrint(trcErrLogId, FL, "[dWrite_CallOverCtrl] %s write failed\n", szFName);
		return -2;
	}   
 
	fprintf(fp, "@START\n");
	fprintf(fp, "@ %-10s %-10s\n", "CPS", "RATE");

	fprintf(fp, "   %-10d %-10d\n", pCurCpsOvld->over_cps, pCurCpsOvld->over_rate);

	fprintf(fp, "@END");
	fclose(fp);
	return 0;
}

int dSetOVLDCtrl(int over_cps, int over_rate)
{
	int             dOldIdx, dNewIdx;
	CPS_OVLD_CTRL	*pSetCpsOvld;

	dOldIdx = gpIdx->dCpsIdx;
	switch( dOldIdx )
	{
		case -1:
		case 0 :
			dNewIdx = 1;
			break;
		case 1 :
			dNewIdx = 0;
			break;
		default :
			logPrint(trcErrLogId,FL, "[dSetMPTimer] TimeIdx=%d default\n", dOldIdx);
			dNewIdx = 0;
			break;
	}

	pSetCpsOvld = gpCpsOvldCtrl[dNewIdx];
	memset(pSetCpsOvld, 0x00, sizeof(CPS_OVLD_CTRL));

	pSetCpsOvld->over_cps = over_cps;
	pSetCpsOvld->over_rate = over_rate;

	dLogCpsOvld(pSetCpsOvld);	
	gpIdx->dCpsIdx = dNewIdx;
	return dNewIdx;
}

int mmcr_mmc_set_ovld_ctrl(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char    		mmlBuf[BUFSIZ];
	int flag=0;
	int	dRet = 0, dIdx = 0, over_cps = 0, over_rate = 0;
	
	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	logPrint(trcLogId, FL, "[mmc_set_ovld_ctrl] paraName1:%s paraVal1:%s, paraName2:%s paraVal2:%s\n"
			, rxReqMsg->head.para[0].paraName, rxReqMsg->head.para[0].paraVal
			, rxReqMsg->head.para[1].paraName, rxReqMsg->head.para[1].paraVal);

	if (!strcasecmp(rxReqMsg->head.para[0].paraName, "CPS")) {
		over_cps = atoi(rxReqMsg->head.para[0].paraVal);
		logPrint(trcLogId, FL, " >> CPS:%d\n", over_cps);
		++flag;
	}
	if (!strcasecmp(rxReqMsg->head.para[1].paraName, "RATE")) {
		over_rate = atoi(rxReqMsg->head.para[1].paraVal);
		logPrint(trcLogId, FL, " >> RATE:%d\n", over_rate);
		++flag;
	}

	if (flag == 2) {

		dIdx = dSetOVLDCtrl(over_cps, over_rate);

		if( (dRet = dWrite_CallOverCtrl(gpCpsOvldCtrl[dIdx])) < 0 )
		{
			logPrint(trcErrLogId, FL, "[mmc_set_ovld_ctrl] dWrite_CallOverCtrl write fail\n");
			/* mml response */
			sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED\tCPS OVLD FILE WRITE FAIL\n"
					,mySysName, myAppName);
		}
		else
		{
			/* mml response */
			logPrint(trcLogId,FL, "[mmc_set_ovld_ctrl] SET OVERLOAD CONTROL\tSUCCESSED\tCPS:%d RATE:%d\n ", gpCpsOvldCtrl[dIdx]->over_cps, gpCpsOvldCtrl[dIdx]->over_rate);
			sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tSUCCESSED\tSET OVLD CTRL\tCPS:%d\tRATE:%d\n"
				,mySysName, myAppName, gpCpsOvldCtrl[dIdx]->over_cps, gpCpsOvldCtrl[dIdx]->over_rate);

			dWriteFLTIDXFile();
			dSendNOTIFY(NOTI_CPS_TYPE, NULL);
		}
	}
	else {
		logPrint(trcErrLogId, FL, "[mmc_set_ovld_ctrl] SET OVERLOAD CONTROL FAILED  CPS:%d RATE:%d\n", 
						gpCpsOvldCtrl[dIdx]->over_cps, gpCpsOvldCtrl[dIdx]->over_rate);
		/* mml response */
		sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED\tSET OVLD CTRL\tCPS:%d  RATE:%d\n"
				,mySysName, myAppName, gpCpsOvldCtrl[dIdx]->over_cps, gpCpsOvldCtrl[dIdx]->over_rate);

	}
	comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
	return 0;
}

int mmcr_mmc_dis_ovld_ctrl(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char    		mmlBuf[BUFSIZ];
	CPS_OVLD_CTRL	*pCurCpsOvld = gpCpsOvldCtrl[gpIdx->dCpsIdx];
	
	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	/* mml response */
	logPrint(trcLogId, FL, "[mmc_dis_ovld_ctrl] DIS OVERLOAD CONTROL  SUCCESSED  CPS:%d RATE:%d\n ", 
					pCurCpsOvld->over_cps, pCurCpsOvld->over_rate);
	sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tSUCCESSED\tDIS OVLD CTRL\tCPS:%d  RATE:%d\n"
				,mySysName, myAppName, pCurCpsOvld->over_cps, pCurCpsOvld->over_rate);

	comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
	return 0;
}

void dLog_RuleSet_Used(RULESET_USED_FLAG *pCurRuleSet)
{
	int i,j;

	logPrint(trcLogId, FL, "##### RULESET USED LIST  #####\n");
	logPrint(trcLogId, FL, "%-10s %-10s %-10s\n", "PBIT", "HBIT", "USED");

	for( i = 0; i < MAX_PBIT_CNT; i++ ) {
		for( j = 0; j < MAX_HBIT_CNT; j++ ) {
			if (pCurRuleSet->stRule[i][j].uiOperF == 1) {
				logPrint(trcLogId, FL, "\t%-10d %-10d %-10u\n", i, j, pCurRuleSet->stRule[i][j].uiUsedF);
			}
		}
	}
}

int dReadRSetUsedFile(void)
{
	FILE        *fa;    
	char        szBuffer[1024], szFName[256];
	UINT        pbit=0, hbit=0;
	char        used[8];
	char        *env;
	int			dOldIdx, dNewIdx;
	RULESET_USED_FLAG	*pSetRuleUsed;

	if ((env = getenv(IV_HOME)) == NULL){
		logPrint(trcErrLogId, FL, "[dReadRuleUsed] getenv error! \n" );
		return -1; 
	}
	sprintf (szFName, "%s/%s", env, RULESET_USED_FILE);

	if ((fa=fopen(szFName, "r")) == NULL) {
		logPrint(trcErrLogId, FL, "[dReadRuleUsed] %s FILE NOT FOUND\n", szFName);
		return -2; 
	}       
	
	dOldIdx = gpIdx->dRsetUsedIdx;
	switch( dOldIdx )
	{
		case -1:
		case 0 :
			dNewIdx = 1;
			break;
		case 1 :
			dNewIdx = 0;
			break;
		default :
			logPrint(trcErrLogId,FL, "[dReadRuleUsed] RuleIdx=%d default\n", dOldIdx);
			dNewIdx = 0;
			break;
	}

	pSetRuleUsed = gpRSetUsedList[dNewIdx];
	memset(pSetRuleUsed, 0x00, sizeof(RULESET_USED_FLAG));

	while (fgets( szBuffer, 1024, fa ) != NULL)
	{
		if(szBuffer[0] == '@')
			continue;

		if( sscanf( &szBuffer[0], "%u %u %s", &pbit, &hbit, used) == 3 ) {
			pSetRuleUsed->stRule[pbit][hbit].uiPBit = ntohl(pbit);
			pSetRuleUsed->stRule[pbit][hbit].uiHBit = ntohl(hbit);
			if(!strcasecmp(used, "OFF")) {
				pSetRuleUsed->stRule[pbit][hbit].uiUsedF = 0;
			}
			else {
				pSetRuleUsed->stRule[pbit][hbit].uiUsedF = 1;
			}
			pSetRuleUsed->stRule[pbit][hbit].uiOperF = 1;
			pSetRuleUsed->uiCount++;
		}
		else {
			logPrint(trcLogId, FL, "[dReadRSetUsed] %s FILE FORMAT ERROR\n", szFName);
			fclose(fa);
			return -3;
		}
	}
	fclose(fa);
	dLog_RuleSet_Used(pSetRuleUsed);
	gpIdx->dRsetUsedIdx = dNewIdx;

	return dNewIdx;
}

void dSetRuleUsed(RULESET_USED_FLAG *pSetRuleUsed, int Pbit, int Hbit, int Oper, char *Used)
{
	pSetRuleUsed->stRule[Pbit][Hbit].uiPBit = Pbit;
	pSetRuleUsed->stRule[Pbit][Hbit].uiHBit = Hbit;
	pSetRuleUsed->stRule[Pbit][Hbit].uiOperF = Oper;
	if ((!strcasecmp(Used, "ON")) || (!strcasecmp(Used, "on"))) 
		pSetRuleUsed->stRule[Pbit][Hbit].uiUsedF = 1;
	else 
		pSetRuleUsed->stRule[Pbit][Hbit].uiUsedF = 0;
}


int dWrite_RuleSet_Used(RULESET_USED_FLAG *pCurRuleUsed)
{
	FILE        *fp;    
	char        szFName[256];
	char		*env;
	int 		i,j;

	if ((env = getenv(IV_HOME)) == NULL){
		logPrint(trcErrLogId, FL, "[dWrite_RuleSet_Used] getenv error! \n" );
		return -1;
	}
	sprintf (szFName, "%s/%s", env, RULESET_USED_FILE);

	if ((fp = fopen(szFName, "w"))==NULL){
		logPrint(trcErrLogId, FL, "[dWrite_RuleSet_Used] %s write failed\n", szFName);
		return -2;
	}   
 
	fprintf(fp, "@START\n");
	fprintf(fp, "@ %-10s %-10s %-10s\n", "PBIT", "HBIT", "USED");
#if 1
	for(i=0; i< MAX_PBIT_CNT; i++){
		for(j=0; j< MAX_HBIT_CNT; j++){
			if (pCurRuleUsed->stRule[i][j].uiOperF) {
				if (pCurRuleUsed->stRule[i][j].uiUsedF) {
					fprintf(fp, "   %-10u %-10u %-10s\n", 
							pCurRuleUsed->stRule[i][j].uiPBit, pCurRuleUsed->stRule[i][j].uiHBit, "ON");
				}
				else {
					fprintf(fp, "   %-10u %-10u %-10s\n", 
							pCurRuleUsed->stRule[i][j].uiPBit, pCurRuleUsed->stRule[i][j].uiHBit, "OFF");
				}
			}
		}
	}
#endif
	fprintf(fp, "@END");
	fclose(fp);

	dLog_RuleSet_Used(pCurRuleUsed);
	return 0;
}

int	mmcr_mmc_dis_rule_use(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char    		mmlBuf[MMCMSGSIZE*2];	// BECAUSE- P/HBIT RANGE each 100 number(35*100)
	int i, j;
	int	idx=0, cnt=0;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	RULESET_USED_FLAG	*pCurRSetUsed = gpRSetUsedList[gpIdx->dRsetUsedIdx];

	/* mml response */
	sprintf( mmlBuf+idx, "\n\tSYSTEM = %s\n\tAPP = %s\n\tSUCCESSED\tDIS RULESET USE", 
						mySysName, myAppName); 
	idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------"); 
	idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n\t%-10s %-10s %-10s", "PBIT", "HBIT", "USED"); idx = strlen(mmlBuf);

	for(i=0; i<MAX_PBIT_CNT; i++) {
		for(j=0; j<MAX_HBIT_CNT; j++) {
			if (pCurRSetUsed->stRule[i][j].uiOperF == 1) {
				if (pCurRSetUsed->stRule[i][j].uiUsedF == 1) {
					sprintf( mmlBuf+idx, "\n      %-10d %-10d %-10s", 
							pCurRSetUsed->stRule[i][j].uiPBit, 
							pCurRSetUsed->stRule[i][j].uiHBit, "ON"); 
					idx = strlen(mmlBuf);
				}
				else {
					sprintf( mmlBuf+idx, "\n      %-10d %-10d %-10s", 
							pCurRSetUsed->stRule[i][j].uiPBit, 
							pCurRSetUsed->stRule[i][j].uiHBit, "OFF"); 
					idx = strlen(mmlBuf);
				}
				cnt++;
			}
		}
	}
	sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------"); 
	idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    COUNT : %d", cnt); 
	idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------"); 
	idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    RESULT = SUCCESS   ");  
	idx = strlen(mmlBuf);

	comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);

	return 0;
}

int mmcr_mmc_set_rule_use(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char    		mmlBuf[MMCMSGSIZE*2];	// BECAUSE- P/HBIT RANGE each 100 number(35*100)
	int				dIdx = 0, Pbit=0, Hbit=0;
	char			Used[4];
	int flag=0;
	RULESET_USED_FLAG	*pCurRuleUsed;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* Parameter setting && make login feild */
	logPrint(trcLogId, FL, "[mmc_set_rule_use] paraName1:%s paraVal1:%s, paraName2:%s paraVal2:%s, paraName3:%s paraVal3:%s\n"
			, rxReqMsg->head.para[0].paraName, rxReqMsg->head.para[0].paraVal
			, rxReqMsg->head.para[1].paraName, rxReqMsg->head.para[1].paraVal
			, rxReqMsg->head.para[2].paraName, rxReqMsg->head.para[2].paraVal);

	if (!strcasecmp(rxReqMsg->head.para[0].paraName, "PBIT")) {
		Pbit = atoi(rxReqMsg->head.para[0].paraVal);
		++flag;
	}
	if (!strcasecmp(rxReqMsg->head.para[1].paraName, "HBIT")) {
		Hbit = atoi(rxReqMsg->head.para[1].paraVal);
		++flag;
	}
	if (!strcasecmp(rxReqMsg->head.para[2].paraName, "USE")) {
		strncpy(Used, rxReqMsg->head.para[2].paraVal, 3);
		++flag;
	}

	if (flag == 3) {
		/* operation */
		if( (dIdx = dReadRSetUsedFile()) < 0 )
		{
			logPrint(trcErrLogId, FL, "[mmc_set_rule_use] SET RULE USE\tFAILED\tPBIT:%u HBIT:%u USE:%s\n", 
							Pbit, Hbit, Used);
			/* mml response */
			sprintf(mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED\tSET RULE USE\tPBIT:%u.HBIT:%u USE:%s\n"
				,mySysName, myAppName, Pbit, Hbit, Used );

			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
			return -1;
		}

		pCurRuleUsed = gpRSetUsedList[dIdx];
		dSetRuleUsed(pCurRuleUsed, Pbit, Hbit, 1, Used);

		/* mml response */
		if (pCurRuleUsed->stRule[Pbit][Hbit].uiUsedF == 1) {
			logPrint(trcLogId, FL, "[mmc_set_rule_use] RULE USE ON\tSUCCESSED\n"
							 		"\tPBIT:%u HBIT:%u USE:%s\n", Pbit, Hbit, "ON");
			sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tSUCCESSED"
								"\tRULE USED ON\tPBIT:%u.HBIT:%u USE:%s\n" , 
								mySysName, myAppName, Pbit, Hbit, "ON");
		}
		else {
			logPrint(trcLogId, FL, "[mmc_set_rule_use] RULE USE OFF\tSUCCESSED\n"
									"\tPBIT:%u HBIT:%u USE:%s", Pbit, Hbit, "OFF");
			sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tSUCCESSED"
								"\tRULE USED OFF  PBIT:%u.HBIT:%u USE:%s\n"
								,mySysName, myAppName, Pbit, Hbit, "OFF");
		}

		/* WRITE - RULESET_USED.conf */
		if (dWrite_RuleSet_Used(pCurRuleUsed) < 0) {
			logPrint(trcErrLogId, FL, "[mmc_set_rule_use] SET RULE USE WRITE FAILED \n "
								"PBIT:%u HBIT:%u USE:%s", Pbit, Hbit, Used);
			sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED FILE WRITE"
								,mySysName, myAppName);
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
			return -2;
		}
		dWriteFLTIDXFile();
		dSendNOTIFY(NOTI_RULE_TYPE, NULL);
		comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
	}
	else {
		logPrint(trcErrLogId, FL, "[mmc_set_rule_use] SET RULE USE\tFAILED\tPBIT:%u HBIT:%u USE:%s\n", 
								Pbit, Hbit, Used);
		/* mml response */
		sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED\tSET RULE USE\tPBIT:%u.HBIT:%u USE:%s\n"
				, mySysName, myAppName, Pbit, Hbit, Used );

		comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
	}

	return 0;
}

int mmcr_mmc_del_rule_use(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   	*rxReqMsg;
	char    			mmlBuf[MMCMSGSIZE*2]; // BECAUSE- P/HBIT RANGE each 100 number(35*100)
	int					flag = 0, dIdx = 0, Pbit=0, Hbit=0;
	char				Used[4];
	RULESET_USED_FLAG	*pCurRuleUsed;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* Parameter setting && make login feild */
	logPrint(trcLogId, FL, "[mmc_del_rule_use] paraName1:%s paraVal1:%s, paraName2:%s paraVal2:%s\n"
			, rxReqMsg->head.para[0].paraName, rxReqMsg->head.para[0].paraVal
			, rxReqMsg->head.para[1].paraName, rxReqMsg->head.para[1].paraVal);

	if (!strcasecmp(rxReqMsg->head.para[0].paraName, "PBIT")) {
		Pbit = atoi(rxReqMsg->head.para[0].paraVal);
		++flag;
	}
	if (!strcasecmp(rxReqMsg->head.para[1].paraName, "HBIT")) {
		Hbit = atoi(rxReqMsg->head.para[1].paraVal);
		++flag;
	}

	if (flag == 2) {
		if( (dIdx = dReadRSetUsedFile()) < 0 )
		{
			logPrint(trcErrLogId, FL, "[mmc_del_rule_use] SET RULE USE\tFILE READ FAILED\t"
										"PBIT:%u HBIT:%u USE:%s\n", Pbit, Hbit, Used);
			/* mml response */
			sprintf(mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED\tDEL RULE USE\t"
							"PBIT:%u.HBIT:%u USE:%s\n" ,mySysName, myAppName, Pbit, Hbit, Used );

			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
			return -1;
		}

		pCurRuleUsed = gpRSetUsedList[dIdx];
		//dSetRuleUsed(pCurRuleUsed, Pbit, Hbit, 0, NULL);
		dSetRuleUsed(pCurRuleUsed, Pbit, Hbit, 0, "OFF");

		/* WRITE - RULESET_USED.conf */
		if (dWrite_RuleSet_Used(pCurRuleUsed) < 0) {
			logPrint(trcErrLogId, FL, "[mmc_set_rule_use] SET RULE USE WRITE FAILED \n "
								"PBIT:%u HBIT:%u USE:%s", Pbit, Hbit, Used);
			sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED FILE WRITE"
								,mySysName, myAppName);
			comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
			return -2;
		}
		dWriteFLTIDXFile();
		dSendNOTIFY(NOTI_RULE_TYPE, NULL);
		sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tSUCCESSED\tRULE USE DELETE"
						"\tPBIT:%u.HBIT:%u \n" , mySysName, myAppName, Pbit, Hbit);
		comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		
		/* mml response */
		logPrint(trcLogId, FL, "[mmc_del_rule_use] RULE USE DELETE SUCCESSED\tPBIT:%u HBIT:%u\n", 
												Pbit, Hbit);
#if 0
		sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tSUCCESSED\tRULE USE DELETE"
						"\tPBIT:%u.HBIT:%u \n" , mySysName, myAppName, Pbit, Hbit);
#endif
	}
	else {
		logPrint(trcLogId, FL, "[mmc_del_rule_use] DEL RULE USE\tFAILED\tPBIT:%u HBIT:%u\n", Pbit, Hbit);
		/* mml response */
		sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED\tSET RULE USE\tPBIT:%u.HBIT:%u\n"
				, mySysName, myAppName, Pbit, Hbit);
		comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
	}

	return 0;
}

int mmcr_mmc_dis_rleg_sess (IxpcQMsgType *rxIxpcMsg)
{
	int             idx = 0; 
	char            mmlBuf[MMCMSGSIZE];
	MMLReqMsgType   *rxReqMsg;

	memset(mmlBuf, 0, sizeof(mmlBuf));
	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* mml response */
	sprintf( mmlBuf+idx, "\n    SYSTEM = %s\n    APP = %s\n    SUCCESSED    DIS SESS INFO" , mySysName, myAppName);	idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------");    				idx = strlen(mmlBuf); 
	sprintf( mmlBuf+idx, "\n    [RLEG] Current Session Count = %u", gpShmem->rad_sess);    						idx = strlen(mmlBuf); 
	sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------");     				idx = strlen(mmlBuf);
	sprintf( mmlBuf+idx, "\n    RESULT = SUCCESS   ");

	logPrint(trcLogId, FL,"\n%s\n",mmlBuf);
	comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);

	return 1;
}

int	mmcr_mmc_dis_call_info (IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char    		mmlBuf[BUFSIZ];
	LEG_DATA_SUM	*pCPS = NULL;
	LEG_CALL_DATA   *pstCallData = gpstCallDataPerSec;
	char    argSys[32], argType[32];

	int	idx=0, i, cIdx = 0;
	time_t			now, tCpsTime;
	struct tm		tmCps;

	now = time(0);
	tCpsTime = (now/CPS_UNIT)*CPS_UNIT;
	localtime_r((time_t*)&tCpsTime, &tmCps);
	cIdx = tmCps.tm_sec % DEF_STAT_SET_CNT;

	memset(mmlBuf, 0, sizeof(mmlBuf));

	pCPS = gpstCallInfo[cIdx];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	/* operation */

	if (!strcasecmp(rxReqMsg->head.para[0].paraName, "SYS")) {
		strcpy (argSys, rxReqMsg->head.para[0].paraVal);
		for (i=0; i<strlen(argSys); i++) argSys[i] = toupper(argSys[i]);

		strcpy (argType, rxReqMsg->head.para[1].paraVal);
		for (i=0; i<strlen(argType); i++) argType[i] = toupper(argType[i]);

	} else {
		strcpy (argSys, "");
		strcpy (argType, rxReqMsg->head.para[0].paraVal);
		for (i=0; i<strlen(argType); i++) argType[i] = toupper(argType[i]);
	}

	if (!strcasecmp(argType, "CPS")) {

		/* mml response */
		sprintf( mmlBuf+idx, "\n    SYSTEM = %s\n    APP = %s\n    SUCCESSED    DIS CALL INFO" , mySysName, myAppName);
		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------");
		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    [CPS] LOGON Count = %u, LOGOUT Count = %u", pstCallData->cps.uiLogOnSumCps/5, pstCallData->cps.uiLogOutSumCps/5 );
		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------");
		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    RESULT = SUCCESS   ");  																
		idx = strlen(mmlBuf);

		MMCResSnd(rxIxpcMsg, mmlBuf, 0, 0); 

	} else if(!strcasecmp(argType, "RAD-SESS")) {
		/* mml response */
		sprintf( mmlBuf+idx, "\n    SYSTEM = %s\n    APP = %s\n    SUCCESSED    DIS CALL INFO" , mySysName, myAppName);
		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------");
		idx = strlen(mmlBuf); 
		sprintf( mmlBuf+idx, "\n    [RAD] Session Count = %u", gpShmem->rad_sess);
		idx = strlen(mmlBuf); 
		sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------");
		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    RESULT = SUCCESS   ");

		logPrint(trcLogId, FL,"\n%s\n",mmlBuf);
		comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);

	} else if(!strcasecmp(argType, "TPS")) {
		/* mml response */
		sprintf( mmlBuf+idx, "\n    SYSTEM = %s\n    APP = %s\n    SUCCESSED    DIS CALL INFO" , mySysName, myAppName);
		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------");
		idx = strlen(mmlBuf); 
		sprintf( mmlBuf+idx, "\n    [TPS] Transaction Count = %u", pstCallData->tps/5);
		idx = strlen(mmlBuf); 
		sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------");
		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    RESULT = SUCCESS   ");

		logPrint(trcLogId, FL,"\n%s\n",mmlBuf);
		comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);

	} else if(!strcasecmp(argType, "ALL")) {
		sprintf( mmlBuf+idx, "\n    SYSTEM = %s\n    APP = %s\n    SUCCESSED    DIS CALL INFO" , "ALL", myAppName);
		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------");
		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    [CPS] LOGON Count = %u, LOGOUT Count = %u", pstCallData->cps.uiLogOnSumCps/5, pstCallData->cps.uiLogOutSumCps/5 );
		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------");
		idx = strlen(mmlBuf); 
		sprintf( mmlBuf+idx, "\n    [TPS] Transaction Count = %u", pstCallData->tps/5);
		idx = strlen(mmlBuf); 
		sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------");
		idx = strlen(mmlBuf); 
		sprintf( mmlBuf+idx, "\n    [RAD] Session Count     = %u", gpShmem->rad_sess);
		idx = strlen(mmlBuf); 
		sprintf( mmlBuf+idx, "\n    -------------------------------------------------------------");
		idx = strlen(mmlBuf);
		sprintf( mmlBuf+idx, "\n    RESULT = SUCCESS   ");

		logPrint(trcLogId, FL,"\n%s\n",mmlBuf);
		comm_txMMLResult(msgqTable[0], rxIxpcMsg, mmlBuf, 0, 0, 0, 0, 1);
		
	}

	return 0;
}
