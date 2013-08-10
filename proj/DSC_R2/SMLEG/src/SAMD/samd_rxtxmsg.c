#include "samd.h"

extern int		samdQID, ixpcQID, trcFlag, trcLogFlag, alarm_flag, prev_alarm_flag;
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN], systemModel[5];
extern SFM_SysCommMsgType	*loc_sadb;
extern SAMD_ProcessInfo		ProcessInfo[SYSCONF_MAX_APPL_NUM];

extern void	doBkupPkg (IxpcQMsgType *rxIxpcMsg);
extern void	doDisSessLoad(IxpcQMsgType *rxIxpcMsg);

int HandleRxMsg(void)
{
	int						i, rxCnt;
	GeneralQMsgType			rxGenQMsg;
	IxpcQMsgType			*rxIxpcMsg;
	MMLReqMsgType			*rxReqMsg;
	IFB_KillPrcNotiMsgType	*killsysMsg;
	char					logbuf[256];

	rxCnt	= 0;
	rxReqMsg	= NULL;
#ifdef _USE_MMC_THREAD_
	while(1)
	{
		if(msgrcv(samdQID, (char*)&rxGenQMsg, sizeof(GeneralQMsgType), 0, MSG_NOERROR /*IPC_NOWAIT*/) < 0)
#else
		if(msgrcv(samdQID, (char*)&rxGenQMsg, sizeof(GeneralQMsgType), 0, /*MSG_NOERROR*/IPC_NOWAIT) < 0)
#endif
		{
			if(errno != ENOMSG)
			{
				sprintf(logbuf,"[HandleRxMsg] msgrcv fail; err=%d(%s)\n", errno, strerror(errno));
				trclib_writeLogErr (FL,logbuf);
				return -1;
			}
			return rxCnt;
		}
		rxCnt++;

		switch(rxGenQMsg.mtype)
		{
			case MTYPE_SETPRINT:
				trclib_exeSetPrintMsg((TrcLibSetPrintMsgType*)&rxGenQMsg);
				break;
			case MTYPE_STATUS_REPORT:	/*	killsys로 인해 메세지가 왔을 때	*/
				killsysMsg = (IFB_KillPrcNotiMsgType*)rxGenQMsg.body;
				for(i = 0; i < loc_sadb->processCount; i++)
				{
					if(!strcasecmp(loc_sadb->loc_process_sts[i].processName,killsysMsg->processName))
					{
						if( killsysMsg->type == START_PRC ){
							ProcessInfo[i].mask = SFM_STATUS_ALIVE;//UNMASKED
							sprintf(logbuf, "[%s] process=%s started\n", 
									__FUNCTION__, killsysMsg->processName);
						} else {
							ProcessInfo[i].mask = SFM_ALM_MASKED;
							sprintf(logbuf, "[%s] process=%s terminated\n", 
									__FUNCTION__, killsysMsg->processName);
						}
						trclib_writeLogErr(FL, logbuf);
						if(trcFlag || trcLogFlag)
						{
							sprintf(logbuf, "[HandleRxMsg] killsys[%s] = %d\n", killsysMsg->processName, i);
							trclib_writeLog(FL, logbuf);
						}
					}
				}
				break;
			case MTYPE_MMC_REQUEST:
				rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg.body;
				rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
				if(trcFlag || trcLogFlag)
				{
					sprintf(logbuf, " cmdName = %s\n", rxReqMsg->head.cmdName);
					trclib_writeLog(FL, logbuf);
				}
/* DEBUG : BY JUNE, 2011-05-12 */
				sprintf(logbuf, " >>>> cmd=%s src=%s dst=%s para[0]=%s para[1]=%s \n"
						, rxReqMsg->head.cmdName
						, rxIxpcMsg->head.srcAppName
						, rxIxpcMsg->head.dstAppName
						, rxReqMsg->head.para[0].paraVal
						, rxReqMsg->head.para[1].paraVal);
				trclib_writeLogErr (FL,logbuf);
/* END DEBUG */
				if (!strcasecmp(rxReqMsg->head.cmdName, "dis-prc-sts"))
					doDisPrcSts(rxIxpcMsg);
				else if(!strcasecmp(rxReqMsg->head.cmdName, "kill-prc") || !strcasecmp(rxReqMsg->head.cmdName, "down-sw-unit"))
					doKillPrc(rxIxpcMsg);
				else if(!strcasecmp(rxReqMsg->head.cmdName, "start-prc") || !strcasecmp(rxReqMsg->head.cmdName, "online-sw-unit"))
					doRunPrc(rxIxpcMsg);
				else if(!strcasecmp(rxReqMsg->head.cmdName, "dis-load-sts"))
					doDisLoadSts(rxIxpcMsg);
				else if(!strcasecmp(rxReqMsg->head.cmdName, "dis-lan-sts"))
					doDisLanSts(rxIxpcMsg);
				else if(!strcasecmp(rxReqMsg->head.cmdName, "dis-ntp-sts"))
					doDisNTPSts(rxIxpcMsg);
				else if(!strcasecmp(rxReqMsg->head.cmdName, "dis-sys-sts"))
				{
					// sjjeon
					doDisSysSts(rxIxpcMsg);
				//	if( !strcasecmp(systemModel, "G3"))
				//		doDisSysSts(rxIxpcMsg);
				//	else if ( !strcasecmp(systemModel, "G5"))
				//		doDisSysSts2(rxIxpcMsg);
				}
				else if (!strcasecmp(rxReqMsg->head.cmdName, "bkup-pkg"))
					doBkupPkg(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName, "dis-sess-load"))
					doDisSessLoad(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName, "dis-sys-ver"))    // sjjeon
					doDisSysVer(rxIxpcMsg);
				else if (!strcasecmp(rxReqMsg->head.cmdName, "clr-alm-sysmsg")) {    // by june, 20100315
					trclib_writeLogErr(FL,logbuf);
					doClrAlm_SysMsg(rxIxpcMsg);
				}
				else
				{
					sprintf(logbuf, "[HandleRxMsg] unknown cmdName = %s\n", rxReqMsg->head.cmdName);
					trclib_writeLogErr(FL,logbuf);
				}
				break;
			case MTYPE_STATISTICS_REQUEST:
				sprintf(logbuf, "[HandleRxMsg] STATISTICS REQUEST START = %ld\n", time(0));
				trclib_writeLogErr(FL, logbuf);
				HandleStatistics(&rxGenQMsg);
				sprintf(logbuf, "[HandleRxMsg] STATISTICS REQUEST END = %ld\n", time(0));
				trclib_writeLogErr(FL, logbuf);
				break;
			case MTYPE_TRANINIT_REQUEST:
				alarm_flag		= 0;
				prev_alarm_flag	= 0;
				break;
			default:
				rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg.body;
				sprintf(logbuf, "[HandleRxMsg] unknown mtype = %ld; src=%s-%s\n", rxGenQMsg.mtype, rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
				trclib_writeLogErr(FL, logbuf);
				break;
		} /*-- end of swich() --*/
#ifdef _USE_MMC_THREAD_
	} /*-- end of while(1) --*/
#endif
	return rxCnt;
}


int report_sadb2FIMD(void)
{
	int					i, j, txLen;
	GeneralQMsgType		txGenQMsg;
	IxpcQMsgType		*txIxpcMsg;
	char 				logbuf[256];
	//SFM_SysCommMsgType	*sysComMsg; sjjeon 쓸모없어 막는다.
#if 0
	printf("SAMD SFM_SysCommMsgType size : %d\n", sizeof(SFM_SysCommMsgType));
    printf("SAMD SFM_SysCommProcSts size : %d\n",sizeof(SFM_SysCommProcSts));
    printf("SAMD SFM_SysCommDiskSts size : %d\n",sizeof(SFM_SysCommDiskSts));
    printf("SAMD SFM_SysCommLanSts  size : %d\n",sizeof(SFM_SysCommLanSts));
    printf("SAMD SFM_SysCommQueSts  size : %d\n",sizeof(SFM_SysCommQueSts));
    printf("SAMD SFM_SysCommLinkSts size : %d\n",sizeof(SFM_SysCommLinkSts));
    printf("SAMD SFM_SysDuplicationSts size : %d\n",sizeof(SFM_SysDuplicationSts));
    printf("SAMD SFM_SysSuccessRate size : %d\n",sizeof(SFM_SysSuccessRate));
    printf("SAMD SFM_SysIFBond size : %d\n",sizeof(SFM_SysIFBond));
    printf("SAMD SFM_SysRsrcLoad size : %d\n",sizeof(SFM_SysIFBond));
    printf("SAMD SFM_SysSts size : %d\n",sizeof(SFM_SysSts));
    printf("SAMD SFM_Duia size : %d\n",sizeof(SFM_Duia));
#endif


/*
	fprintf(stderr,"TEST cpuCount:%d,diskCount:%d,lanCount:%d,processCount:%d,queCount:%d\n",
		loc_sadb->cpuCount, loc_sadb->diskCount,loc_sadb->lanCount, loc_sadb->processCount, loc_sadb->queCount);

	for(i = 0; i < loc_sadb->diskCount; i++)
	{
		fprintf(stderr,"TEST diskName:%s, usage:%d\n",
			loc_sadb->loc_disk_sts[i].diskName, loc_sadb->loc_disk_sts[i].disk_usage);
	}
*/
	txIxpcMsg	= (IxpcQMsgType*)txGenQMsg.body;
	memset( (void*)&txIxpcMsg->head, 0x00, sizeof(txIxpcMsg->head));

	txGenQMsg.mtype	= MTYPE_STATUS_REPORT;

	strcpy(txIxpcMsg->head.srcSysName, mySysName);
	strcpy(txIxpcMsg->head.srcAppName, myAppName);
	strcpy(txIxpcMsg->head.dstSysName, "DSCM");
	strcpy(txIxpcMsg->head.dstAppName, "FIMD");

	txIxpcMsg->head.msgId	= MSGID_SYS_COMM_STATUS_REPORT;
// not used : sjjeon
//	txIxpcMsg->head.bodyLen	= sizeof(SFM_SysCommMsgType) - (sizeof(SFM_SysCommSdmdPrcSts)*SFM_MAX_PROC_CNT);
	txIxpcMsg->head.bodyLen	= sizeof(SFM_SysCommMsgType) ;
	txLen					= sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

#if 0	
	// sjjeon log
	fprintf(stderr, "GMSG-mtype : %d, all-Len: %d, bodyLen : %d, ",
			txGenQMsg.mtype, txLen, txIxpcMsg->head.bodyLen
			);
	fprintf(stderr, 
	"Proc=%d, Disk=%d Lan=%d Que=%d Link=%d Dupli=%d, Rate=%d BOND=%d LOAD=%d SYSSTS=%d DUIA=%d",
			sizeof(SFM_SysCommProcSts),sizeof(SFM_SysCommDiskSts), sizeof(SFM_SysCommLanSts),
			sizeof(SFM_SysCommQueSts), sizeof(SFM_SysCommLinkSts), sizeof(SFM_SysDuplicationSts),
			sizeof(SFM_SysSuccessRate), sizeof(SFM_SysIFBond), sizeof(SFM_SysRsrcLoad),
			sizeof(SFM_SysSts), sizeof(SFM_Duia));
#endif		

#if 0
	for(i = 0; i < loc_sadb->queCount; i++)
	{
		fprintf(stderr, "TEST %d qID:%d,qKEY:%d,qNUM:%d,cBYTES:%d,qBYTES:%d\n",
			i, loc_sadb->loc_que_sts[i].qID,loc_sadb->loc_que_sts[i].qKEY,loc_sadb->loc_que_sts[i].qNUM,
			loc_sadb->loc_que_sts[i].cBYTES,loc_sadb->loc_que_sts[i].qBYTES);
	}
#endif

	for(i = 0; i < loc_sadb->lanCount; i++)
		loc_sadb->loc_lan_sts[i].target_IPaddress = htonl((unsigned int)loc_sadb->loc_lan_sts[i].target_IPaddress);

	for(i = 0; i < loc_sadb->rmtlanCount; i++)
		loc_sadb->rmt_lan_sts[i].target_IPaddress = htonl((unsigned int)loc_sadb->rmt_lan_sts[i].target_IPaddress);

	/* 2006.06.28 by helca */
	for(i = 0; i < loc_sadb->queCount; i++)
	{
		loc_sadb->loc_que_sts[i].qID	= htonl((int)loc_sadb->loc_que_sts[i].qID);
		loc_sadb->loc_que_sts[i].qKEY	= htonl((int)loc_sadb->loc_que_sts[i].qKEY);
		loc_sadb->loc_que_sts[i].qNUM	= htonl((unsigned int)loc_sadb->loc_que_sts[i].qNUM);
		loc_sadb->loc_que_sts[i].cBYTES	= htonl((unsigned int)loc_sadb->loc_que_sts[i].cBYTES);
		loc_sadb->loc_que_sts[i].qBYTES	= htonl((unsigned int)loc_sadb->loc_que_sts[i].qBYTES);
	}

	for(i = 0; i < 3; i++)
		loc_sadb->loc_system_dup.RmtDbSts[i].iStatus = htonl(loc_sadb->loc_system_dup.RmtDbSts[i].iStatus);

	/*	by helca 08.04	*/
	loc_sadb->loc_system_dup.heartbeatAlarm	= htonl(loc_sadb->loc_system_dup.heartbeatAlarm);
	loc_sadb->loc_system_dup.OosAlarm		= htonl(loc_sadb->loc_system_dup.OosAlarm);

/*
	for(i = 0; i < SFM_MAX_RSRC_LOAD_CNT; i++)
	{
		sprintf(trcBuf, "[%s] before rsrcIdx: %d - usage:%d \n", __FUNCTION__, i, loc_sadb->rsrc_load.rsrcload[i]);
		trclib_writeLogErr(FL, trcBuf);
	}
*/
	/*	by helca 08.07	*/
	for(i = 0; i < SFM_MAX_RSRC_LOAD_CNT; i++)
	{
		for(j = 0; j < MAX_MP_NUM1; j++)
			loc_sadb->rsrc_load.rsrcload[i][j] = htonl(loc_sadb->rsrc_load.rsrcload[i][j]);
	}

#if 0
	for(i = 0; i < SFM_REAL_SUCC_RATE_CNT; i++)
	{
		fprintf(stderr, "succ_rate[%d]_cnt: %d\n",i, loc_sadb->succ_rate[i].count);
		fprintf(stderr, "succ_rate[%d]_rate: %d\n",i, loc_sadb->succ_rate[i].rate);
	}
#endif

	/*	by helca 08.25	*/
	for(i = 0; i < SFM_REAL_SUCC_RATE_CNT; i++)
	{
		loc_sadb->succ_rate[i].count	= htonl(loc_sadb->succ_rate[i].count);
		loc_sadb->succ_rate[i].rate		= htonl(loc_sadb->succ_rate[i].rate);
	}

#if 0
	fprintf(stderr,"cpu %d mem %d  qID %d qKEY %d qNAME %s qNUM %d cBYTES %d qBYTES %d\n",
		loc_sadb->cpu_usage[0], loc_sadb->mem_usage, loc_sadb->loc_que_sts->qID, loc_sadb->loc_que_sts->qKEY,
		loc_sadb->loc_que_sts->qNAME, loc_sadb->loc_que_sts->qNUM, loc_sadb->loc_que_sts->cBYTES, loc_sadb->loc_que_sts->qBYTES);
	for(i=0;i<SFM_HW_MAX_LINK_NUM;i++){
		fprintf(stderr,"link-name: %s, status: %d\n",
				loc_sadb->sysSts.linkSts[i].StsName, loc_sadb->sysSts.linkSts[i].status);
	}
#endif

//	memcpy( (void*)txIxpcMsg->body, loc_sadb, sizeof(SFM_SysCommMsgType));
	memcpy( (void*)txIxpcMsg->body, loc_sadb, txIxpcMsg->head.bodyLen);
	//sysComMsg = (SFM_SysCommMsgType*)txIxpcMsg->body; sjjeon 막는다. 쓸모없다. 

	if(msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0)
	{
		sprintf(logbuf, "[%s] msgsnd(report_sadb2FIMD) fail; err=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
		trclib_writeLogErr(FL, logbuf);
#if 0
		printf("[%s] ixpcQID:%d, txLen:%d\n", __FUNCTION__, ixpcQID, txLen);
		//printf("%s", trcBuf);
#endif
		return -1;
	}
#if 0
	else
		printf("[%s]::%s->%s::%s->%s\n", __FUNCTION__, mySysName, myAppName, txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName);
#endif

	for(i = 0; i < SFM_MAX_RSRC_LOAD_CNT; i++)
	{
		/*	by helca 08.07	*/
		for(j = 0; j < MAX_MP_NUM1; j++)
			loc_sadb->rsrc_load.rsrcload[i][j] = ntohl(loc_sadb->rsrc_load.rsrcload[i][j]);
	}

	/*	2006.06.28 by helca	*/
	for(i = 0; i < loc_sadb->queCount; i++)
	{
		loc_sadb->loc_que_sts[i].qID	= ntohl((int)loc_sadb->loc_que_sts[i].qID);
		loc_sadb->loc_que_sts[i].qKEY	= ntohl((int)loc_sadb->loc_que_sts[i].qKEY);
		loc_sadb->loc_que_sts[i].qNUM	= ntohl((unsigned int)loc_sadb->loc_que_sts[i].qNUM);
		loc_sadb->loc_que_sts[i].cBYTES	= ntohl((unsigned int)loc_sadb->loc_que_sts[i].cBYTES);
		loc_sadb->loc_que_sts[i].qBYTES	= ntohl((unsigned int)loc_sadb->loc_que_sts[i].qBYTES);
	}

	/*	by helca 08.10	*/
	for(i = 0; i < 3; i++)
		loc_sadb->loc_system_dup.RmtDbSts[i].iStatus	= ntohl(loc_sadb->loc_system_dup.RmtDbSts[i].iStatus);

	loc_sadb->loc_system_dup.heartbeatAlarm	= ntohl(loc_sadb->loc_system_dup.heartbeatAlarm);
	loc_sadb->loc_system_dup.OosAlarm		= ntohl(loc_sadb->loc_system_dup.OosAlarm);

	for(i = 0; i < SFM_REAL_SUCC_RATE_CNT; i++)
	{
		loc_sadb->succ_rate[i].count	= ntohl(loc_sadb->succ_rate[i].count);
		loc_sadb->succ_rate[i].rate		= ntohl(loc_sadb->succ_rate[i].rate);
	}

/*
	for(i = 0; i < SFM_MAX_RSRC_LOAD_CNT; i++)
	{
		sprintf(trcBuf, "[%s] after rsrcIdx: %d - usage: %d\n", __FUNCTION__, i, loc_sadb->rsrc_load.rsrcload[i]);
		trclib_writeLogErr(FL, trcBuf);
	}
*/
	return 1;
}	/** End of report_sadb2FIMD **/

/* 자신의 MMC명령어가 아니면 해당 시스템으로 전송하는 함수 */
int MMCReqBypassSnd (IxpcQMsgType *rxIxpcMsg)
{
	GeneralQMsgType txGenQMsg;
	int             txLen;
	char			logbuf[256];

	txGenQMsg.mtype = MTYPE_MMC_REQUEST;

	txLen = sizeof(rxIxpcMsg->head) + rxIxpcMsg->head.bodyLen;
	if (memcpy ((void*)txGenQMsg.body, rxIxpcMsg, txLen) == NULL) {
		sprintf(logbuf, "memcpy err = %s\n", strerror(errno));
		trclib_writeLogErr(FL,logbuf);
		return -1;
	}

	if (msgsnd (ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(logbuf, "[MMCReqBypassSnd] msgsnd error=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr(FL,logbuf);
		return -1;
	}
	return 1;
}

/* dis-load-sts, dis-prc-sts 명령어가 전체 시스템일 경우 MCDM으로 전송하는 함수 */
int MMCReqMCDMBypassSnd (IxpcQMsgType *rxIxpcMsg)
{
	GeneralQMsgType txGenQMsg;
	int             txLen;
	char 			logbuf[256];

	txGenQMsg.mtype = MTYPE_MMC_REQUEST;

	strcpy(rxIxpcMsg->head.dstAppName, "MCDM");

	txLen = sizeof(rxIxpcMsg->head) + rxIxpcMsg->head.bodyLen;
	if (memcpy ((void*)txGenQMsg.body, rxIxpcMsg, txLen) == NULL) {
		sprintf(logbuf, "[MMCReqMCDMBypassSnd] memcpy err = %s\n", strerror(errno));
		trclib_writeLogErr(FL,logbuf);
		return -1;
	}

	if (msgsnd (ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(logbuf, "[MMCReqMCDMBypassSnd] msgsnd error=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr(FL,logbuf);
		return -1;
	}
	return 1;
}


int MMCResSnd (IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag)
{
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;
    MMLResMsgType   *txResMsg;
    MMLReqMsgType   *rxReqMsg;
    int             txLen;
	char 			logbuf[256];

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
        sprintf(logbuf, "msgsnd error = %s, cmd = %s\n", strerror(errno), txResMsg->head.cmdName);
        trclib_writeLogErr(FL,logbuf);
		return -1;
    }
	return 1;
}



int sendWatchdogMsg2COND (int procIdx)
{
	int				txLen;
	char			tmpBuf[256];
	GeneralQMsgType	txGenQMsg;
	IxpcQMsgType	*txIxpcMsg;
	char			logbuf[256];

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "COND");
	txIxpcMsg->head.msgId   = MSGID_WATCHDOG_STATUS_REPORT; // TTIB에서만 임시로 사용하는 값이다. COND에서는 사용하지 않는
	txIxpcMsg->head.segFlag = 0;
	txIxpcMsg->head.seqNo   = 1;

	sprintf(txIxpcMsg->body, "    %s %s\n    S%04d WATCH-DOG FUNCTION STATUS REPORT\n",
			sysLabel, commlib_printTStamp(), STSCODE_SFM_WATCHDOG_REPORT);

	sprintf(tmpBuf, "      SYSTEM = %s\n", mySysName);
	strcat (txIxpcMsg->body, tmpBuf);
	sprintf(tmpBuf, "      PROC   = %s\n", ProcessInfo[procIdx].procName);
	strcat (txIxpcMsg->body, tmpBuf);
	sprintf(tmpBuf, "      INFORM = WATCH-DOG_FUNCTION_EVENT_DETECTED\n");
	strcat (txIxpcMsg->body, tmpBuf);
	strcat (txIxpcMsg->body, "      COMPLETED\n\n\n");

	txIxpcMsg->head.bodyLen = strlen(txIxpcMsg->body) +1;
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(logbuf,"[sendWatchdogMsg2COND] msgsnd fail to COND; err=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr (FL,logbuf);
		return -1;
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(logbuf,"[sendWatchdogMsg2COND] send to COND\n");
			trclib_writeLog (FL,logbuf);
		}
	}

	return 1;
}

int sendQueClearMsg2COND (int procIdx, int qcnt)
{
	int             txLen;
	char            tmpBuf[256];
	GeneralQMsgType txGenQMsg;
	IxpcQMsgType    *txIxpcMsg;
	char			logbuf[256];

	txGenQMsg.mtype = MTYPE_QUEUE_CLEAR_REPORT;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "COND");
	txIxpcMsg->head.msgId   = 0; // COND에서는 사용하지 않는다.
	txIxpcMsg->head.segFlag = 0;
	txIxpcMsg->head.seqNo   = 1;

	sprintf(txIxpcMsg->body, "    %s %s\n    S%04d QUEUE CLEAR REPORT\n",
			sysLabel, commlib_printTStamp(), STSCODE_SFM_QUEUE_CLEAR);

	sprintf(tmpBuf, "      SYSTEM = %s\n", mySysName);
	strcat (txIxpcMsg->body, tmpBuf);
	sprintf(tmpBuf, "      PROC   = %s\n", ProcessInfo[procIdx].procName);
	strcat (txIxpcMsg->body, tmpBuf);
	sprintf(tmpBuf, "      INFORM = QUEUE CLEAR (QCNT: %d)\n", qcnt);
	strcat (txIxpcMsg->body, tmpBuf);
	strcat (txIxpcMsg->body, "      COMPLETED\n\n\n");

	txIxpcMsg->head.bodyLen = strlen(txIxpcMsg->body) +1;
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(logbuf,"[sendQueClearMsg2COND] msgsnd fail to COND; err=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr (FL,logbuf);
		return -1;
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(logbuf,"[sendQueClearMsg2COND] send to COND\n");
			trclib_writeLog (FL,logbuf);
		}
	}

	return 1;
} /* End of sendQueClearMsg2COND */
