/**A.1* FILE INCLUSION ********************************************************/
#include "mmcr.h"
#include "utillib.h"
#include "init_shm.h"
#include "ipaf_stat.h"
#include "comm_trace.h"
#include "nifo.h"
#include "sm_subs_info.h"
#include "common_ana.h"

/**B.1* DEFINITION OF NEW CONSTANTS *******************************************/

/**C.1* DECLARATION OF VARIABLES **********************************************/
int		msgqTable[MSGQ_MAX_SIZE];	
char	iv_home[64], l_sysconf[256];
char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
char    trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
int 	g_Flag;		// trace expire check flag
char	szLogBuf[1024];

stLogLevel   *logLevel;
stTimer      *mpTimer;

stMEMSINFO			*pstMEMSINFO;
// TRACE_INFO.conf 구조체 
st_SESSInfo			*gpTrcList[DEF_SET_CNT];
// PDSN.conf 구조체  
PDSN_LIST			*gpPdsnList[DEF_SET_CNT];
// PDSN HASH
stHASHOINFO			*gpPdsnHash[DEF_SET_CNT];
// RULESET_LIST.conf 구조체 
ST_PBTABLE_LIST		*gpRsetList[DEF_SET_CNT];
// RULESET_USED.conf 구조체 
RULESET_USED_FLAG	*gpRSetUsedList[DEF_SET_CNT];
// CALL_OVER_CTRL.conf 구조체 
CPS_OVLD_CTRL		*gpCpsOvldCtrl[DEF_SET_CNT];
// TIMEOUT.conf 구조체 
MPTimer            	*gpMPTimer[DEF_SET_CNT];

LEG_DATA_SUM		*gpstCallInfo[DEF_STAT_SET_CNT];
// CPS & TPS per 5sec save 구조체 
LEG_CALL_DATA       *gpstCallDataPerSec;
st_NOTI				gstIdx;
st_NOTI				*gpIdx = &gstIdx;

/* FOR LOG */
extern int	trcFlag, trcLogFlag;
extern 		T_keepalive	*keepalive;
extern int	trcLogId, trcErrLogId;

/**D.1*  DEFINITION OF FUNCTIONS  *********************************************/
extern void handleChildProcess(int);
extern int 	dReadTrcFile(void);
extern int 	dReadPdsnFile(void);
extern int 	dReadRsetListFile(void);
extern int 	dReadRSetUsedFile(void);
extern int 	dReadCpsOvldFile(void);
extern int 	dReadTimerFile(void);

int dWriteFLTIDXFile(void);
int dReadFLTINFO(void);
int dSendNOTIFY(unsigned short uhMsgID, SUBS_INFO *psi);

/*******************************************************************************

********************************************************************************/
int main(int argc, char* argv[])
{
	int 	dRet, rxQmsgCnt; 
	GeneralQMsgType	rxGenQMsg;
	pid_t	mypid;
	time_t	t_now, t_prev;

	mypid = getpid();
	Init_logdebug( mypid, "MMCR", "/DSC/APPLOG" );

#if 0
   	if((check_Index = check_my_run_status("MMCR")) <0 )
 		exit(0);
#endif

	if (InitSys() < 0 )	exit(1);
		
	// clear previous messages
	while (msgrcv(msgqTable[1], &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT) > 0);

	t_prev = t_now = time(0);

	handleChildProcess(0);
	g_Flag = DEF_ON_FLAG;

	if( (dRet = dReadFLTINFO()) < 0 )
	{
		logPrint(trcErrLogId, FL, "dReadFLTINFO() FAIL dRet=%d\n", dRet);
		exit(1);
	}

	dSendNOTIFY(NOTI_ALL, NULL);

	while(1)
	{
		rxQmsgCnt = HandleRxMsg();

		if (rxQmsgCnt < 0) return -1;
		if (rxQmsgCnt == 0) commlib_microSleep(1000);
		
		t_now = time(0);

        keepalivelib_increase ();
		
		if( t_now > t_prev + 10 )
		{
			callTraceDelete(t_now);
			t_prev = t_now;
		}
	}
	return 1;
}

int mmcr_mmcHdlrVector_qsortCmp (const void *a, const void *b)
{
	return (strcasecmp (((pMmcFuncTable)a)->cmdName, ((pMmcFuncTable)b)->cmdName));
} //----- End of mmcr_mmcHdlrVector_qsortCmp -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcr_mmcHdlrVector_bsrchCmp (const void *a, const void *b)
{
	return (strcasecmp(((pMmcFuncTable)a)->cmdName, ((pMmcFuncTable)b)->cmdName) );
} //----- End of mmcr_mmcHdlrVector_bsrchCmp -----//

void PrintIDX(st_NOTI *pIdx)
{
	logPrint(trcLogId, FL, "TRC IDX = %d\n", pIdx->dTrcIdx);
	logPrint(trcLogId, FL, "PDSN IDX = %d\n", pIdx->dPdsnIdx);
	logPrint(trcLogId, FL, "PDSN CNT = %d\n", pIdx->dPdsnCnt);
	logPrint(trcLogId, FL, "RLIST IDX = %d\n", pIdx->dRsetListIdx);
	logPrint(trcLogId, FL, "RUSED IDX = %d\n", pIdx->dRsetUsedIdx);
	logPrint(trcLogId, FL, "CPS IDX = %d\n", pIdx->dCpsIdx);
	logPrint(trcLogId, FL, "TIMER IDX = %d\n", pIdx->dTimeIdx);
}


int dSendNOTIFY(unsigned short uhMsgID, SUBS_INFO *psi)
{
	int             dRet;
	unsigned char   *pNode;
	NOTIFY_SIG      *pNOTIFY;

	PrintIDX(gpIdx);

	if( (pNode = nifo_node_alloc(pstMEMSINFO)) == NULL)
	{
		logPrint(trcErrLogId, FL, "ERROR IN nifo_node_alloc()\n");
		return -1;
	}

	if( (pNOTIFY = (NOTIFY_SIG*)nifo_tlv_alloc(pstMEMSINFO, pNode, NOTIFY_SIG_DEF_NUM, sizeof(NOTIFY_SIG), DEF_MEMSET_ON)) == NULL)
	{
		logPrint(trcErrLogId, FL, "ERROR IN nifo_tlv_alloc()\n");
		nifo_node_delete(pstMEMSINFO, pNode);
		return -2;
	}
	else
	{
		pNOTIFY->dFltType = (unsigned int)uhMsgID;
		memcpy(&pNOTIFY->stNoti, gpIdx, sizeof(st_NOTI));
	}

	/* msgsnd RANA */
	if((dRet = nifo_msg_write(pstMEMSINFO, msgqTable[3], pNode)) < 0) 
	{
		logPrint(trcErrLogId, FL, "ERROR IN nifo_msg_write()\n");
		nifo_node_delete(pstMEMSINFO, pNode);
		return -3;
	}
	else
		logPrint(trcLogId, FL, "noti type = %d nifo_msg_write to RANA\n", uhMsgID);

	if( uhMsgID == NOTI_ALL )
	{
		oldSendAppNoty(7, MTYPE_TIMER_CONFIG, SID_CHG_INFO, MID_CHG_TIME); // RLEG0 7
		oldSendAppNoty(8, MTYPE_TIMER_CONFIG, SID_CHG_INFO, MID_CHG_TIME); // RLEG1 
		oldSendAppNoty(9, MTYPE_TIMER_CONFIG, SID_CHG_INFO, MID_CHG_TIME); // RLEG2 
		oldSendAppNoty(10, MTYPE_TIMER_CONFIG, SID_CHG_INFO, MID_CHG_TIME); // RLEG3 
		oldSendAppNoty(11, MTYPE_TIMER_CONFIG, SID_CHG_INFO, MID_CHG_TIME); // RLEG4 
		oldSendAppNoty(6, MTYPE_TIMER_CONFIG, SID_CHG_INFO, MID_CHG_TIME); // SMPP 6

		if( (pNode = nifo_node_alloc(pstMEMSINFO)) == NULL)
		{
			logPrint(trcErrLogId, FL, "ERROR IN nifo_node_alloc()\n");
			return -4;
		}

		if( (pNOTIFY = (NOTIFY_SIG*)nifo_tlv_alloc(pstMEMSINFO, pNode, NOTIFY_SIG_DEF_NUM, sizeof(NOTIFY_SIG), DEF_MEMSET_ON)) == NULL)
		{
			logPrint(trcErrLogId, FL, "ERROR IN nifo_tlv_alloc()\n");
			nifo_node_delete(pstMEMSINFO, pNode);
			return -5;
		}
		else
		{
			pNOTIFY->dFltType = (unsigned int)uhMsgID;
			memcpy(&pNOTIFY->stNoti, gpIdx, sizeof(st_NOTI));
		}

		/* msgsnd PANA */
		if((dRet = nifo_msg_write(pstMEMSINFO, msgqTable[2], pNode)) < 0) 
		{
			logPrint(trcErrLogId, FL, "ERROR IN nifo_msg_write dRet=%d\n", dRet);
			nifo_node_delete(pstMEMSINFO, pNode);
			return -6;
		}
		logPrint(trcLogId, FL, "noti type = %d nifo_msg_write to PANA\n", uhMsgID);
	}
	else if( uhMsgID == NOTI_TRACE_TYPE )
	{
		// TODO 
		oldSendAppNoty(5, MTYPE_TRC_CONFIG, SID_CHG_INFO, MID_TRC); // RDRANA 
		oldSendAppNoty(6, MTYPE_TRC_CONFIG, SID_CHG_INFO, MID_TRC); // SMPP

	}
	else if( uhMsgID == NOTI_PDSN_TYPE )
	{
		if( (pNode = nifo_node_alloc(pstMEMSINFO)) == NULL)
		{
			logPrint(trcErrLogId, FL, "ERROR IN nifo_node_alloc()\n");
			return -4;
		}

		if( (pNOTIFY = (NOTIFY_SIG*)nifo_tlv_alloc(pstMEMSINFO, pNode, NOTIFY_SIG_DEF_NUM, sizeof(NOTIFY_SIG), DEF_MEMSET_ON)) == NULL)
		{
			logPrint(trcErrLogId, FL, "ERROR IN nifo_tlv_alloc()\n");
			nifo_node_delete(pstMEMSINFO, pNode);
			return -5;
		}
		else
		{
			pNOTIFY->dFltType = (unsigned int)uhMsgID;
			memcpy(&pNOTIFY->stNoti, gpIdx, sizeof(st_NOTI));
		}

		/* msgsnd PANA */
		if((dRet = nifo_msg_write(pstMEMSINFO, msgqTable[2], pNode)) < 0) 
		{
			logPrint(trcErrLogId, FL, "ERROR IN nifo_msg_write dRet=%d\n", dRet);
			nifo_node_delete(pstMEMSINFO, pNode);
			return -6;
		}
		logPrint(trcLogId, FL, "noti type = %d nifo_msg_write to PANA\n", uhMsgID);
	}
	else if( uhMsgID == NOTI_TIME_TYPE )
	{
		// TODO 
		oldSendAppNoty(7, MTYPE_TIMER_CONFIG, SID_CHG_INFO, MID_CHG_TIME); // RLEG0 7
		oldSendAppNoty(8, MTYPE_TIMER_CONFIG, SID_CHG_INFO, MID_CHG_TIME); // RLEG1 
		oldSendAppNoty(9, MTYPE_TIMER_CONFIG, SID_CHG_INFO, MID_CHG_TIME); // RLEG2 
		oldSendAppNoty(10, MTYPE_TIMER_CONFIG, SID_CHG_INFO, MID_CHG_TIME); // RLEG3 
		oldSendAppNoty(11, MTYPE_TIMER_CONFIG, SID_CHG_INFO, MID_CHG_TIME); // RLEG4 
		oldSendAppNoty(6, MTYPE_TIMER_CONFIG, SID_CHG_INFO, MID_CHG_TIME); // SMPP 6
	}

	return 0;
}

int dReadFLTIDXFile(void)
{
	FILE *fa;
	char szBuf[1024];
	char szType[64];
	int i = 0, dIdx = 0;

	fa = fopen(DEF_NOTI_INDEX_FILE, "r");
	if(fa == NULL)
	{
		logPrint(trcErrLogId, FL, "%s FILE OPEN FAIL\n", DEF_NOTI_INDEX_FILE);
		return -1;
	}

	while(fgets(szBuf,1024,fa) != NULL)
	{
		if(szBuf[0] != '#')
		{
			logPrint(trcErrLogId, FL, "%s FILE %d row format error\n", DEF_NOTI_INDEX_FILE, i);
			fclose(fa);
			return -1;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{

			if(sscanf(&szBuf[2],"%s %d",szType, &dIdx) == 2)
			{
				if(strcmp(szType,"TRACE") == 0)
				{
					gpIdx->dTrcIdx = dIdx;
					logPrint(trcLogId, FL, "TRACE READ IDX=%d\n", dIdx);
				}
				else if( strcmp(szType, "PDSN") == 0 )
				{
					gpIdx->dPdsnIdx = dIdx;
					logPrint(trcLogId, FL, "PDSN READ IDX=%d\n", dIdx);
				}
				else if( strcmp(szType, "RULESET_LIST") == 0 )
				{
					gpIdx->dRsetListIdx = dIdx;
					logPrint(trcLogId, FL, "RULESET_LIST READ IDX=%d\n", dIdx);
				}
				else if( strcmp(szType, "RULESET_USED") == 0 )
				{
					gpIdx->dRsetUsedIdx = dIdx;
					logPrint(trcLogId, FL, "RULESET_USED READ IDX=%d\n", dIdx);
				}
				else if( strcmp(szType, "CPS") == 0 )
				{
					gpIdx->dCpsIdx = dIdx;
					logPrint(trcLogId, FL, "CPS_OVLD_CTRL READ IDX=%d\n", dIdx);
				}
				else if( strcmp(szType, "TIMEOUT") == 0 )
				{
					gpIdx->dTimeIdx = dIdx;
					logPrint(trcLogId, FL, "TIMEOUT READ IDX=%d\n", dIdx);
				}
			}
		}
		dIdx = 0; i++;
	}

	fclose(fa);

	return i;
} 

int dWriteFLTIDXFile(void)
{
	char	g_confpath[1024]; /* G*/
	char	mesg_path[80];
	int		dRet;

	FILE		*fptr;
	DIR			*dirp;
	char	szBuf[256];

	sprintf(g_confpath, "%s", DATA_PATH);
	sprintf(mesg_path, "%s", DATA_PATH"NOTI_INIT.conf");
	logPrint(trcLogId, FL, "mesg_path[%s]\n", mesg_path);

	dirp = opendir( g_confpath );
	if( dirp == (DIR*)NULL )
		mkdir( g_confpath, 0777 );
	else
		closedir( dirp );

	if ((fptr = fopen(mesg_path, "w+")) == NULL)
	{
		logPrint(trcErrLogId, FL, "%s FILE OPEN FAIL\n", mesg_path);
		return -1;
	}

	sprintf(&szBuf[0], "## ACTIVE FILTER INDEX\n"
						"#@ TRACE %d\n"
						"#@ PDSN %d\n"
						"#@ PDSN_CNT %d\n"
						"#@ RULESET_LIST %d\n"
						"#@ RULESET_USED %d\n"
						"#@ CPS %d\n"
						"#@ TIMEOUT %d\n"
						"##\n#E \n", 
						gpIdx->dTrcIdx, gpIdx->dPdsnIdx, gpIdx->dPdsnCnt,
						gpIdx->dRsetListIdx, gpIdx->dRsetUsedIdx,
						gpIdx->dCpsIdx, gpIdx->dTimeIdx);

	dRet = fwrite(szBuf, strlen(szBuf), 1, fptr);

	fclose(fptr);

	return dRet;
}

int dReadFLTINFO(void)
{
	int dRet;

	// Active 필터 인덱스 구성. 
	dRet = dReadFLTIDXFile();
	if( dRet < 0 )
	{
		logPrint(trcLogId,FL,"dReadFLTIDXFile() FAIL dRet=%d >> Index Init Zero\n", dRet);
		gpIdx->dTrcIdx = 0;
		gpIdx->dPdsnIdx = 0; gpIdx->dPdsnCnt = 0;
		gpIdx->dRsetListIdx = 0;
		gpIdx->dRsetUsedIdx = 0;
		gpIdx->dCpsIdx = 0;
		gpIdx->dTimeIdx = 0;
		gpIdx->szMIN[0] = '\0'; gpIdx->szIP[0] = '\0';

	}

	// 파일읽어서 STANDBY INDEX HASH 구성. 성공하면, Active Standby Index 절체 됨.  
	dRet = dReadTrcFile();
	if( dRet < 0 )
		logPrint (trcErrLogId,FL,"ERROR dReadTrcFile() FAIL dRet=%d\n", dRet);

	dRet = dReadPdsnFile();
	if( dRet < 0 )
		logPrint (trcErrLogId,FL,"ERROR dReadPdsnFile() FAIL dRet=%d\n", dRet);

	dRet = dReadRsetListFile();
	if( dRet < 0 )
		logPrint (trcErrLogId,FL,"ERROR dReadRsetListFile() FAIL dRet=%d\n", dRet);

	dRet = dReadRSetUsedFile();
	if( dRet < 0 )
		logPrint (trcErrLogId,FL,"ERROR dReadRSetUsedFile() FAIL dRet=%d\n", dRet);

	dRet = dReadCpsOvldFile();
	if( dRet < 0 )
		logPrint (trcErrLogId,FL,"ERROR dReadCpsOvldFile() FAIL dRet=%d\n", dRet);

	dRet = dReadTimerFile();
	if( dRet < 0 )
		logPrint (trcErrLogId,FL,"ERROR dReadTimerFile() FAIL dRet=%d\n", dRet);

	dRet = dWriteFLTIDXFile();
	if( dRet < 0 )
	{
		logPrint (trcErrLogId,FL,"ERROR dWriteFLTIDXFile() FAIL dRet=%d\n", dRet);
		return dRet;
	}

	return 0;
}


