#include "samd.h"

extern int		samdQID, ixpcQID, trcFlag, trcLogFlag;
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern char		trcBuf[4096], trcTmp[1024];
extern SFM_L3PD		*l3pd;
extern SFM_L2Dev    *g_pstL2Dev;
extern SFM_SCE      *g_pstSCEInfo;
extern int			trcLogId, trcErrLogId;

extern SFM_SysCommMsgType       *loc_sadb;
extern SAMD_ProcessInfo         ProcessInfo[SYSCONF_MAX_APPL_NUM];

#ifdef DATA_SCP
#include "feaHand.h"
#define SAMD_TX_CLITBL_NUM	8
#define SAMD_TX_SRVTBL_NUM	4
int	shmKey_dscp_cliTbl, shmKey_dscp_srvTbl;
int	cliTbl_lastTxIndex=0;
int	srvTbl_lastTxIndex=0;
#endif

extern void doBkupPkg (IxpcQMsgType *rxIxpcMsg); // jjinri
extern void doDisSceMode (IxpcQMsgType *rxIxpcMsg); // sjjeon

int HandleRxMsg()
{
	int	i, rxCnt=0;
	GeneralQMsgType		rxGenQMsg;
	IxpcQMsgType		*rxIxpcMsg;
	MMLReqMsgType		*rxReqMsg;
	IFB_KillPrcNotiMsgType	*killsysMsg;
	while (1)
	{
		if (msgrcv (samdQID, (char*)&rxGenQMsg, sizeof(GeneralQMsgType), 0, IPC_NOWAIT) < 0) {
			/*
			if (errno != ENOMSG) {
				sprintf(trcBuf,"[HandleRxMsg] msgrcv fail; err=%d(%s)\n", errno, strerror(errno));
				trclib_writeLogErr (FL,trcBuf);
				return -1;
			}
			*/
			if (errno == EINVAL || errno == EFAULT) {
				sprintf(trcBuf,"[HandleRxMsg] msgrcv fail; err=%d(%s)\n", errno, strerror(errno));
				trclib_writeLogErr (FL,trcBuf);
				return -1;
			}
			return rxCnt;
		}
		rxCnt++;

		switch (rxGenQMsg.mtype) {
			case MTYPE_SETPRINT:
				trclib_exeSetPrintMsg ((TrcLibSetPrintMsgType*)&rxGenQMsg);
				break;
			case MTYPE_STATUS_REPORT : /* killsys로 인해 메세지가 왔을 때 **/
				killsysMsg = (IFB_KillPrcNotiMsgType*)rxGenQMsg.body;
				for (i=0; i<loc_sadb->processCount; i++) {
					if ( !strcasecmp(loc_sadb->loc_process_sts[i].processName,killsysMsg->processName)) {
						if(killsysMsg->type) { //START-PRC
							ProcessInfo[i].mask = SFM_STATUS_ALIVE; //UNMASKED
							logPrint (trcErrLogId,FL,"startprc startup process  %s is alive\n", killsysMsg->processName);
						} else { //KILL-PRC
							ProcessInfo[i].mask = SFM_ALM_MASKED;
							logPrint (trcErrLogId,FL,"killprc down process %s is dead\n", killsysMsg->processName);
						}
						break;
					}
				}
				break;
			case MTYPE_MMC_REQUEST :
				rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg.body;
				rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
				if (trcFlag || trcLogFlag) {
					sprintf(trcBuf, " cmdName = %s\n", rxReqMsg->head.cmdName);
					trclib_writeLog(FL,trcBuf);
				}
/* DEBUG : BY JUNE, 2011-05-12 */
#if 1
				sprintf(trcBuf, " >>>> cmd=%s src=%s dst=%s para[0]=%s para[1]=%s\n"
						, rxReqMsg->head.cmdName
						, rxIxpcMsg->head.srcAppName
						, rxIxpcMsg->head.dstAppName
						, rxReqMsg->head.para[0].paraVal
						, rxReqMsg->head.para[1].paraVal);
				trclib_writeLogErr (FL,trcBuf);
#endif
/* END DEBUG */
				if (!strcasecmp(rxReqMsg->head.cmdName,"dis-prc-sts"))
					doDisPrcSts(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName,"kill-prc") ||
				         !strcasecmp(rxReqMsg->head.cmdName,"down-sw-unit"))
					doKillPrc(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName,"start-prc") ||
				         !strcasecmp(rxReqMsg->head.cmdName,"online-sw-unit"))
					doRunPrc(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName,"dis-load-sts"))
					doDisLoadSts(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName,"dis-lan-sts"))
					doDisLanSts(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName,"dis-tap-sts"))
					doDisPdSts(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName,"dis-ntp-sts"))
					doDisNTPSts(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName,"dis-scm-sts")) 	// yhshin
					doDisDUPSts(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName,"auto-sce-mode")) 	// yhshin
					doSetAutoMode(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName,"switch-scm")) 	// jjinri
					doSetDUPSts(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName,"set-rule-sce")) 	// yhshin
					doSetRuleSce(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName, "bkup-pkg")) 		// jjinri
					doBkupPkg(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName, "dis-sys-ver")) 	// sjjeon
					doDisSysVer(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName, "dis-flow-cnt")) { 	// jjinri
					sprintf(trcBuf, "[HandleRxMsg] >>>> cmdName = %s\n", rxReqMsg->head.cmdName);
					trclib_writeLogErr(FL,trcBuf);
					doDisFlowCnt(rxIxpcMsg);
				}
				else if (!strcasecmp(rxReqMsg->head.cmdName, "clr-scm-fault")) // 20100927 by dcham
					                    doClrScmFaultSts(rxIxpcMsg);
				
				else {
					sprintf(trcBuf, "[HandleRxMsg] unknown cmdName = %s\n", rxReqMsg->head.cmdName);
					trclib_writeLogErr(FL,trcBuf);
				}
				break;
				sprintf(trcBuf, " >>>> cmdName = %s cmd terminated, when=%ld\n", rxReqMsg->head.cmdName, time(0));
				trclib_writeLogErr (FL,trcBuf);
			case MTYPE_STATISTICS_REQUEST:
				HandleStatistics(&rxGenQMsg);
				break;
			default:
				rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg.body;
				rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
				sprintf(trcBuf, "[HandleRxMsg] unknown mtype = %ld; src=%s-%s, cmdName :%s\n", rxGenQMsg.mtype, 
						rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName,  rxReqMsg->head.cmdName);
				trclib_writeLogErr(FL,trcBuf);
				break;
		} /*-- end of swich() --*/

	} /*-- end of while(1) --*/

	return rxCnt;
}


void report_sadb2FIMD ()
{
   	GeneralQMsgType		txGenQMsg;
	IxpcQMsgType		*txIxpcMsg;
	int	txLen;
	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "FIMD");

	txIxpcMsg->head.msgId   = MSGID_SYS_COMM_STATUS_REPORT;
	txIxpcMsg->head.bodyLen = sizeof(SFM_SysCommMsgType);

	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

   	memcpy ((void*)txIxpcMsg->body, loc_sadb, sizeof(SFM_SysCommMsgType));
	//yhshin big edian
	SFM_SysCommMsgType_H2N ((SFM_SysCommMsgType*)&txIxpcMsg->body);

   	if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
   		sprintf(trcBuf,"[report_sadb2FIMD] msgsnd(report_sadb2FIMD) fail; err=%d(%s)\n", errno, strerror(errno));
   		trclib_writeLogErr (FL,trcBuf);
	}

} /** End of report_sadb2FIMD **/

/* by helca */
int report_l3pd2FIMD (void)
{
   	GeneralQMsgType		txGenQMsg;
	IxpcQMsgType		*txIxpcMsg;
	int					txLen;
	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "FIMD");

	txIxpcMsg->head.msgId   = MSGID_SYS_L3PD_STATUS_REPORT;
	txIxpcMsg->head.bodyLen = sizeof(SFM_L3PD);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;
	
	memcpy ((void*)txIxpcMsg->body, l3pd, sizeof(SFM_L3PD));
	SFM_L3PD_H2N(((SFM_L3PD*)&txIxpcMsg->body));

   	if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
   		sprintf(trcBuf,"[report_sadb2FIMD] msgsnd(report_sadb2FIMD) fail; err=%d(%s)\n", errno, strerror(errno));
   		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	else
	{
	}
	return 1;

} /** End of report_l3pd2FIMD **/


#if 1	/* by june */
int report_SCE2FIMD (void)
{
	GeneralQMsgType		txGenQMsg;
	IxpcQMsgType		*txIxpcMsg;
	int					txLen;
	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "FIMD");

	txIxpcMsg->head.msgId   = MSGID_SYS_SCE_STATUS_REPORT;
	txIxpcMsg->head.bodyLen = sizeof(SFM_SCE);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	memcpy ((void*)txIxpcMsg->body, g_pstSCEInfo, sizeof(SFM_SCE));
	SFM_SCE_N2H ((SFM_SCE *)&txIxpcMsg->body);

	if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[report_SCE2FIMD] msgsnd(report_SCEFIMD) fail; err=%d(%s)\n"
						, errno
						, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	else {
	}
	return 1;

} /** End of report_SCE2FIMD **/
#endif

#if 1	/* by june */
int report_L2SW2FIMD (void)
{
	GeneralQMsgType		txGenQMsg;
	IxpcQMsgType		*txIxpcMsg;
	int					txLen;
	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "FIMD");

	txIxpcMsg->head.msgId   = MSGID_SYS_L2_STATUS_REPORT;
	txIxpcMsg->head.bodyLen = sizeof(SFM_L2Dev);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	memcpy ((void*)txIxpcMsg->body, g_pstL2Dev, sizeof(SFM_L2Dev));
	SFM_L2DEV_H2N (((SFM_L2Dev *)&txIxpcMsg->body));

	if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[report_L2SW2FIMD] msgsnd fail; err=%d(%s)\n"
						, errno
						, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
//		printf("%s",trcBuf);
		return -1;
	}
	else {
/*		printf("report_SCE2FIMD::%s->%s::%s->%s\n"
				, mySysName
				, myAppName
				, txIxpcMsg->head.dstSysName
				, txIxpcMsg->head.dstAppName);
*/
	}
	return 1;

} /** End of report_L2SW2FIMD **/
#endif


/* 자신의 MMC명령어가 아니면 해당 시스템으로 전송하는 함수 */
void MMCReqBypassSnd (IxpcQMsgType *rxIxpcMsg)
{
	GeneralQMsgType txGenQMsg;
	int             txLen;

	txGenQMsg.mtype = MTYPE_MMC_REQUEST;

	txLen = sizeof(rxIxpcMsg->head) + rxIxpcMsg->head.bodyLen;
	if (memcpy ((void*)txGenQMsg.body, rxIxpcMsg, txLen) == NULL) {
		sprintf(trcBuf, "memcpy err = %s\n", strerror(errno));
		trclib_writeLogErr(FL,trcBuf);
		return;
	} 

	if (msgsnd (ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf, "[MMCReqBypassSnd] msgsnd error=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr(FL,trcBuf);
	}
}



void MMCResSnd (IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag)
{
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;
    MMLResMsgType   *txResMsg;
    MMLReqMsgType   *rxReqMsg;
    int             txLen;

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    txResMsg = (MMLResMsgType*)txIxpcMsg->body;

    txGenQMsg.mtype = MTYPE_MMC_RESPONSE;

    strcpy (txIxpcMsg->head.srcSysName, rxIxpcMsg->head.dstSysName);
    strcpy (txIxpcMsg->head.srcAppName, rxIxpcMsg->head.dstAppName);
    strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
    strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);

	txIxpcMsg->head.segFlag = 0;
	txIxpcMsg->head.seqNo = 1;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    txResMsg->head.mmcdJobNo = rxReqMsg->head.mmcdJobNo;
    txResMsg->head.resCode = resCode;
    txResMsg->head.contFlag = contFlag;
    strcpy(txResMsg->head.cmdName, rxReqMsg->head.cmdName);

    strcpy(txResMsg->body, resBuf);

    txIxpcMsg->head.bodyLen = sizeof(txResMsg->head) + strlen(txResMsg->body);
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf, "msgsnd error = %s, cmd = %s\n", strerror(errno), txResMsg->head.cmdName);
        trclib_writeLogErr(FL,trcBuf);
    }
}



void sendWatchdogMsg2COND (int procIdx)
{
	int				txLen;
	char			tmpBuf[256];
	GeneralQMsgType	txGenQMsg;
	IxpcQMsgType	*txIxpcMsg;

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "COND");
	txIxpcMsg->head.msgId   = MSGID_WATCHDOG_STATUS_REPORT; // NMSIB에서만 임시로 사용하는 값이다. COND에서는 사용하지 않는
	txIxpcMsg->head.segFlag = 0;
	txIxpcMsg->head.seqNo   = 1;

	sprintf(txIxpcMsg->body, "    %s-%s %s\n    S%04d WATCH-DOG FUNCTION STATUS REPORT\n",
			sysLabel, mySysName, commlib_printTStamp(), STSCODE_SFM_WATCHDOG_REPORT);

	sprintf(tmpBuf, "      SYSTEM = %s\n", mySysName);
	strcat (txIxpcMsg->body, tmpBuf);
	sprintf(tmpBuf, "      PROC   = %s\n", ProcessInfo[procIdx].procName);
	strcat (txIxpcMsg->body, tmpBuf);
	sprintf(tmpBuf, "      INFORM = WATCH-DOG_FUNCTION_EVENT_DETECTED\n");
	strcat (txIxpcMsg->body, tmpBuf);
	strcat (txIxpcMsg->body, "      COMPLETED\n\n\n");

	txIxpcMsg->head.bodyLen = strlen(txIxpcMsg->body);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[sendWatchdogMsg2COND] msgsnd fail to COND; err=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[sendWatchdogMsg2COND] send to COND\n");
			trclib_writeLog (FL,trcBuf);
		}
	}

	return;
}


void sendApplySCERst2COND (int stsNum)
{
	int				txLen;
	char			tmpBuf[256];
	GeneralQMsgType	txGenQMsg;
	IxpcQMsgType	*txIxpcMsg;

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "COND");
	txIxpcMsg->head.msgId   = MSGID_SYS_SCE_STATUS_REPORT; // NMSIB에서만 임시로 사용하는 값이다. COND에서는 사용하지 않는
	txIxpcMsg->head.segFlag = 0;
	txIxpcMsg->head.seqNo   = 1;

	//APPLY_RULESET_PROGRESS_ALARM : 8000
	//APPLY_RULESET_SCEA_COMPLETE_ALARM: 8001
	//APPLY_RULESET_SCEA_COMPLETE_ALARM: 8002
	sprintf(txIxpcMsg->body, "    %s-%s %s\n    S%04d RULE SET FUNCTION STATUS REPORT\n",
			sysLabel, mySysName, commlib_printTStamp(), stsNum);
	
//	sprintf(tmpBuf, "      SYSTEM = %s\n", mySysName);
	if(stsNum==APPLY_RULESET_SCEA_COMPLETE_ALARM)
		sprintf(tmpBuf, "      SYSTEM = SCE-A\n");
	else if(stsNum==APPLY_RULESET_SCEB_COMPLETE_ALARM)
		sprintf(tmpBuf, "      SYSTEM = SCE-B\n");
	else
		sprintf(tmpBuf, "      SYSTEM = %s\n", mySysName);


	strcat (txIxpcMsg->body, tmpBuf);
	sprintf(tmpBuf, "      INFORM = APPLY_RULESET_FUNCTION_EVENT_DETECTED\n");
	strcat (txIxpcMsg->body, tmpBuf);
	strcat (txIxpcMsg->body, "      COMPLETED\n\n\n");

	txIxpcMsg->head.bodyLen = strlen(txIxpcMsg->body);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[sendWatchdogMsg2COND] msgsnd fail to COND; err=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[sendWatchdogMsg2COND] send to COND\n");
			trclib_writeLog (FL,trcBuf);
		}
	}

	return;
}

/*
	* ADD: BY JUNE, 2010-08-23

typedef struct {
#define MAX_FLOW_IDX    3
	    char    szSysName[8];       // DSCA, DSCB, DSCM, PD_A, PD_B
		    time_t  tGetTime;           // Getting time of SCE_FLOW 
			    unsigned int    uiFlowNum;  // FLOW1, FLOW2, FLOW3
} SCE_FLOW_INFO;
#endif
 */

int report_SceFlow_Samd2Stmd (int flow_num, int sce_idx)
{
	GeneralQMsgType		txGenQMsg;
	IxpcQMsgType		*txIxpcMsg;
	int					txLen;
	SCE_FLOW_INFO 		stSceFlow;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "STMD");

	txIxpcMsg->head.msgId   = MSGID_SYS_SCEFLOW_STATUS_REPORT;
	txIxpcMsg->head.bodyLen = sizeof(SCE_FLOW_INFO);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	// Make SCE FLOW DATA 
	memset (&stSceFlow, 0x00, sizeof(SCE_FLOW_INFO));
	if (sce_idx)
		sprintf(stSceFlow.szSysName, "SCEB");
	else
		sprintf(stSceFlow.szSysName, "SCEA");

	stSceFlow.tGetTime  = time((time_t *)0);
	stSceFlow.uiFlowNum = flow_num;

	memcpy ((void*)txIxpcMsg->body, &stSceFlow, sizeof(SCE_FLOW_INFO));
	SCE_FLOW_INFO_N2H((SCE_FLOW_INFO *)&txIxpcMsg->body);

	if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[report_SceFlow_Samd2Stmd] msgsnd(SCE_FLOW_INFO) fail; err=%d(%s)\n"
						, errno
						, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	else {
/*		printf("report_SceFlow_Samd2Stmd: %s->%s::%s->%s\n"
				, mySysName
				, myAppName
				, txIxpcMsg->head.dstSysName
				, txIxpcMsg->head.dstAppName);
*/
	}
	return 1;

} /** End of report_SceFlow_Samd2Stmd**/

