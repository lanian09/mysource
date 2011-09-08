#include "mmcr.h"
#include "sm_subs_info.h"

extern int     	msgqTable[MSGQ_MAX_SIZE];
extern int      trcFlag, trcLogFlag, trcLogId, trcErrLogId;
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern char		trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
extern st_NOTI	*gpIdx;

extern int mmcd_exeMMCMsg (IxpcQMsgType *rxIxpcMsg);
extern int dReadPdsnFile(void);
extern int dSendNOTIFY(unsigned short uhMsgID, SUBS_INFO *psi);
extern int dWriteFLTIDXFile(void);


//      cmdName, function, function2, confFile, cmdType, paraCnt, mode,frmMode, keyCnt,startIndex, 
MmcFuncTable mmcFuncTable[MMCR_MAX_MMC_HANDLER] = {
    {"set-log-level", doSetInfo, doSetLogLevel, LOG_LEVEL_FILE, 0, 3,0, COMM_FRM_MODE, 2, 0},
    {"dis-log-level", doDisInfo, doDisCommProcess, LOG_LEVEL_FILE, 0, 1,0, 0, 1, 0},
};

int     numMmcHdlr=61;

int HandleRxMsg (void)
{
	int		rxCnt=0, ret;
	GeneralQMsgType		rxGenQMsg;
	IxpcQMsgType		*rxIxpcMsg;
	MMLReqMsgType		*rxReqMsg;
	pMmcFuncTable	mmcHdlr;
	
	while (1)
	{
		if (msgrcv (msgqTable[1], (char*)&rxGenQMsg, sizeof(GeneralQMsgType), 0, IPC_NOWAIT) < 0) {
			if (errno != ENOMSG) {
				fprintf(stderr,"[HandleRxMsg] msgrcv fail; err=%d(%s)\n", errno, strerror(errno));
				trclib_writeLogErr (FL,trcBuf);
				return -1;
			}
			return rxCnt;
		}
		rxCnt++;

//printf("HandleRxMsg- start\n");
		switch (rxGenQMsg.mtype) 
		{
			case MTYPE_MMC_REQUEST :
				rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg.body;
				rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
#if 0 /* debugging, by june */
				core 발생 막았음.......sjjeon
					sprintf(trcBuf, " 1.cmdName = %s\n", rxReqMsg->head.cmdName);
					sprintf(trcBuf, " 2.paraCnt = %d\n", rxReqMsg->head.paraCnt);
					trclib_writeLogErr(FL,trcBuf);
#endif
				if (trcFlag || trcLogFlag) 
				{
					sprintf(trcBuf, " cmdName = %s\n", rxReqMsg->head.cmdName);
					fprintf(stderr, " cmdName = %s\n", rxReqMsg->head.cmdName);
					trclib_writeLogErr(FL,trcBuf);
				}
				if ((mmcHdlr = (pMmcFuncTable) bsearch (
					rxReqMsg->head.cmdName,
					mmcFuncTable,
					numMmcHdlr,
					sizeof(MmcFuncTable),
					mmcr_mmcHdlrVector_bsrchCmp)) == NULL) 
				{

					//yhshin 
					// 위에 함수는 너무 복잡하다. 
					// 정리한 함수로 사용하자. 
					// 아래 함수 call하면 mmc_hdl.c 에 정의된 함수 콜하게 된다. 
					ret = mmcd_exeMMCMsg (rxIxpcMsg);
					if (ret == 1) // 성공 
					{
						break;
					}

					sprintf(trcBuf,"[fimd_exeMMCMsg] received unknown mml_cmd(%s)\n", rxReqMsg->head.cmdName);
					trclib_writeLogErr (FL,trcBuf);
					return -1;
				}

				// 처리 function을 호출한다.
				//
				((mmcHdlr->mmcFunc)) (rxIxpcMsg, mmcHdlr);

				break;
			case MTYPE_PDSN_CONFIG:
				if( (ret = dReadPdsnFile()) < 0 )
					logPrint(trcErrLogId, FL, "[MTYPE_PDSN_CONFIG] dReadPdsnFile FAIL ret=%d\n", ret);
				else
				{
					dWriteFLTIDXFile();
					dSendNOTIFY(NOTI_PDSN_TYPE, NULL);
				}	
				break;
			default:
				rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg.body;
				sprintf(trcBuf, "[HandleRxMsg] unknown mtype = %ld; src=%s-%s\n", rxGenQMsg.mtype, rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
				trclib_writeLogErr(FL,trcBuf);
				break;
		} /*-- end of swich() --*/

	} /*-- end of while(1) --*/

	return rxCnt;
}

/* 자신의 MMC명령어가 아니면 해당 시스템으로 전송하는 함수 */
int MMCReqBypassSnd (IxpcQMsgType *rxIxpcMsg)
{
	GeneralQMsgType txGenQMsg;
	int             txLen;

	txGenQMsg.mtype = MTYPE_MMC_REQUEST;

	txLen = sizeof(rxIxpcMsg->head) + rxIxpcMsg->head.bodyLen;
	if (memcpy ((void*)txGenQMsg.body, rxIxpcMsg, txLen) == NULL) {
		sprintf(trcBuf, "memcpy err = %s\n", strerror(errno));
		trclib_writeLogErr(FL,trcBuf);
		return -1;
	} 

	if (msgsnd (msgqTable[0], (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf, "[MMCReqBypassSnd] msgsnd error=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr(FL,trcBuf);
		return -1;
	}
	return 1;
}

int MMCResSndCont (IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag, int seqNo)
{
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;
    MMLResMsgType   *txResMsg;
    MMLReqMsgType   *rxReqMsg;
    int             txLen;
    char    totalBuf[MMCMSGSIZE*2];

    sprintf( totalBuf, "    SYSTEM = %s   ", mySysName );

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    txResMsg = (MMLResMsgType*)txIxpcMsg->body;

    txGenQMsg.mtype = MTYPE_MMC_RESPONSE;

    strcpy (txIxpcMsg->head.srcSysName, rxIxpcMsg->head.dstSysName);
    strcpy (txIxpcMsg->head.srcAppName, rxIxpcMsg->head.dstAppName);
    strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
    strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);
	txIxpcMsg->head.segFlag = contFlag;
	txIxpcMsg->head.seqNo = seqNo;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    txResMsg->head.mmcdJobNo = rxReqMsg->head.mmcdJobNo;
    if(contFlag)
    {
		txResMsg->head.extendTime = htons(5);
	}
    txResMsg->head.resCode = resCode;
    txResMsg->head.contFlag = contFlag;
    strcpy(txResMsg->head.cmdName, rxReqMsg->head.cmdName);
    strcat( totalBuf,resBuf );
    strcpy(txResMsg->body, totalBuf);

    txIxpcMsg->head.bodyLen = sizeof(txResMsg->head) + strlen(txResMsg->body);
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if (msgsnd(msgqTable[0], (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf, "msgsnd error = %s, cmd = %s\n", strerror(errno), txResMsg->head.cmdName);
        trclib_writeLogErr(FL,trcBuf);
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
    char    totalBuf[MMCMSGSIZE*2];

    sprintf( totalBuf, "    SYSTEM = %s   ", mySysName );

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    txResMsg = (MMLResMsgType*)txIxpcMsg->body;

    txGenQMsg.mtype = MTYPE_MMC_RESPONSE;

    strcpy (txIxpcMsg->head.srcSysName, rxIxpcMsg->head.dstSysName);
    strcpy (txIxpcMsg->head.srcAppName, rxIxpcMsg->head.dstAppName);
    strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
    strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);
	txIxpcMsg->head.segFlag = contFlag;
	txIxpcMsg->head.seqNo = 1;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    txResMsg->head.mmcdJobNo = rxReqMsg->head.mmcdJobNo;
    if(contFlag)
    {
		txResMsg->head.extendTime = htons(5);
	}
    txResMsg->head.resCode = resCode;
    txResMsg->head.contFlag = contFlag;
    strcpy(txResMsg->head.cmdName, rxReqMsg->head.cmdName);
    strcat( totalBuf,resBuf );
    strcpy(txResMsg->body, totalBuf);

    txIxpcMsg->head.bodyLen = sizeof(txResMsg->head) + strlen(txResMsg->body);
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if (msgsnd(msgqTable[0], (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf, "msgsnd error = %s, cmd = %s\n", strerror(errno), txResMsg->head.cmdName);
        trclib_writeLogErr(FL,trcBuf);
		return -1;
    }
	return 1;
}

// cmdType에 따른 qid를 찾아 반환한다.
//ixpc[0], mmcr[1], wap1ana1[2],wap1ana2[3],wap1ana3[4],wap1ana4[5],wap1ana5[6]
//uawapana[7], udrgen[8],aaaif[9],sdmd[10]
//trcdr1[11],trcdr2[12],trcdr3[13],trcdr4[14],trcdr5[15],trcdr6[16],trcdr7[17],trcdr8[18],trcdr9[19],trcdr10[20]
//httpana1[21],httpana2[22],httpana3[23],httpana4[24],httpana5[25]
int SendAppNoty( GeneralQMsgType *aSndGenQMsg )
{
	int     i, txLen;
	int		qid,dRet;
    MpConfigCmd *rxConfig;
	static GeneralQMsgType sndGenQMsg;

	//qid를 찾기 위해 body분해 
	memcpy(&sndGenQMsg, aSndGenQMsg, sizeof(GeneralQMsgType) );
	rxConfig = (MpConfigCmd *)sndGenQMsg.body;
	//fprintf( stderr, "cnt : %d \n", rxConfig->cmdParaCnt );
	//fprintf( stderr, "cmdType : %d \n", rxConfig->cmdType );
	//fprintf( stderr, "mtype : %d \n", sndGenQMsg.mtype );

#if 0
{
	int i;
	for(i = 0; i < 5; i++){
		fprintf(stderr, "%d %s\n", i, rxConfig->cmdPara[i]);
	}
}
#endif
	if( rxConfig->cmdType == 0 ) //  MmcFuncTable 
		return 0;

	//length값 구하기
    txLen = sizeof( sndGenQMsg.mtype ) + (sizeof(MpConfigCmd));
	    
	if( 1<=rxConfig->cmdType && rxConfig->cmdType<=10 ){
		qid = msgqTable[10]; //sdmd
		//messgaeQ send
		if ((dRet = msgsnd(qid, (void*)&sndGenQMsg, txLen , IPC_NOWAIT)) < 0)
		{
			logPrint (trcErrLogId,FL,"[SendAppNoty] msgsnd fail[%d]; len=%d, err=%d(%s)\n", qid,txLen,errno, strerror(errno));
			return -1;
		} else {
			;
		}        
	}else if ( 11<=rxConfig->cmdType && rxConfig->cmdType<=20 ){ /* .... ... */
		qid = msgqTable[7]; //uawapana
		//messgaeQ send
		if ((dRet = msgsnd(qid, (void*)&sndGenQMsg, txLen , IPC_NOWAIT)) < 0)
		{
			logPrint (trcErrLogId,FL,"[SendAppNoty] msgsnd fail[%d]; len=%d, err=%d(%s)\n", qid,txLen,errno, strerror(errno));
			return -1;
		} else {
			;
		}
	}else if ( 21<=rxConfig->cmdType && rxConfig->cmdType<=30 ){
		qid = msgqTable[9]; //aaaifQid
		//messgaeQ send
		if ((dRet = msgsnd(qid, (void*)&sndGenQMsg, txLen , IPC_NOWAIT)) < 0)
		{
			logPrint (trcErrLogId,FL,"[SendAppNoty] msgsnd fail[%d]; len=%d, err=%d(%s)\n", qid,txLen,errno, strerror(errno));
			return -1;
		} else {
			;
		}
	}else if ( 31<=rxConfig->cmdType && rxConfig->cmdType<=40 ){
		qid = msgqTable[8]; //udrgen
		//messgaeQ send
		if ((dRet = msgsnd(qid, (void*)&sndGenQMsg, txLen , IPC_NOWAIT)) < 0)
		{
			logPrint (trcErrLogId,FL,"[SendAppNoty] msgsnd fail[%d]; len=%d, err=%d(%s)\n", qid,txLen,errno, strerror(errno));
			return -1;
		} else {
			;
		}
	}else if ( 41<=rxConfig->cmdType && rxConfig->cmdType<=50){/* .... ... */
		for(i=2;i<=6;i++){
			qid = msgqTable[i]; //wap1ana
			//messgaeQ send
			if ((dRet = msgsnd(qid, (void*)&sndGenQMsg, txLen , IPC_NOWAIT)) < 0)
			{
				logPrint (trcErrLogId,FL,"[SendAppNoty] msgsnd fail[%d]; len=%d, err=%d(%s)\n", qid,txLen,errno, strerror(errno));
				//return -1;
			} else {
				;
			}
		}
	}else if ( 51<=rxConfig->cmdType && rxConfig->cmdType<=60 ){
		for(i=21;i<=25;i++){
			qid = msgqTable[i]; //httpana
			//messgaeQ send
			if ((dRet = msgsnd(qid, (void*)&sndGenQMsg, txLen , IPC_NOWAIT)) < 0)
			{
				logPrint (trcErrLogId,FL,"[SendAppNoty] msgsnd fail[%d]; len=%d, err=%d(%s)\n", qid,txLen,errno, strerror(errno));
				//return -1;
			} else {
				;
			}
		}
	}else if ( 61<=rxConfig->cmdType && rxConfig->cmdType<=70 ){/* .... ... */
		qid = msgqTable[8]; //udrgen
		//messgaeQ send
		if ((dRet = msgsnd(qid, (void*)&sndGenQMsg, txLen , IPC_NOWAIT)) < 0)
		{
			logPrint (trcErrLogId,FL,"[SendAppNoty] msgsnd fail[%d]; len=%d, err=%d(%s)\n", qid,txLen,errno, strerror(errno));
			return -1;
		} else {
			;
		}
	}else if ( 71<=rxConfig->cmdType && rxConfig->cmdType<=80 ){
		for(i=11;i<=20;i++){
			qid = msgqTable[i]; //trcdr
			//messgaeQ send
			if ((dRet = msgsnd(qid, (void*)&sndGenQMsg, txLen , IPC_NOWAIT)) < 0)
			{
				logPrint (trcErrLogId,FL,"[SendAppNoty] msgsnd fail[%d]; len=%d, err=%d(%s)\n", qid,txLen,errno, strerror(errno));
				//return -1;
			} else {
				;
			}
		}
	}else if ( 81<=rxConfig->cmdType && rxConfig->cmdType<=90 ){
		qid = msgqTable[8]; //udrgen
		//messgaeQ send
		if ((dRet = msgsnd(qid, (void*)&sndGenQMsg, txLen , IPC_NOWAIT)) < 0)
		{
			logPrint (trcErrLogId,FL,"[SendAppNoty] msgsnd fail[%d]; len=%d, err=%d(%s)\n", qid,txLen,errno, strerror(errno));
			return -1;
		} else {
			;
		}
	}else{
		logPrint (trcErrLogId,FL,"[SendAppNoty] findSendQid fail; err=%d(%s)\n",errno, strerror(errno));
		return -1;
	}

	return 1;
} /* End of SendAppNoty */

int oldSendAppNoty(int appKind, USHORT sType, UCHAR sSvcID, UCHAR sMsgID)
{
    int         qid;
	int			txLen;
	NOTIFY_SIG	stNOTIFY;

	GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;

	//PANA, RANA 는 nifo_msg_write를 사용해서 보내도록 한다.
	if( 2 == appKind || 3 == appKind ){
		logPrint(trcErrLogId,FL,"DON'T USEFUL APPKIND(PANA,RANA) => replace, nifo_msg_write, appKind=%d",appKind);
		return 0;
	}

	memset(&txGenQMsg, 0x00, sizeof(GeneralQMsgType));

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;

	txGenQMsg.mtype = sType;

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);
    strcpy (txIxpcMsg->head.dstSysName, mySysName);
    txIxpcMsg->head.segFlag = 0;
    txIxpcMsg->head.seqNo = 1;


	memset(&stNOTIFY, 0x00, sizeof(NOTIFY_SIG));
	memcpy(&stNOTIFY.stNoti, gpIdx, sizeof(st_NOTI));

	if(sType == MTYPE_TRC_CONFIG)
		stNOTIFY.dFltType = NOTI_TRACE_TYPE;
	else if( sType == MTYPE_PDSN_CONFIG)
		stNOTIFY.dFltType = NOTI_PDSN_TYPE;
	else if( sType == MTYPE_TIMER_CONFIG )
		stNOTIFY.dFltType = NOTI_TIME_TYPE;

	switch( appKind )
	{
		case 2 :
        	qid = msgqTable[2]; // mmcr_init.c 
    		strcpy (txIxpcMsg->head.dstAppName, "PANA");
			break;				
		case 3 :
        	qid = msgqTable[3]; // mmcr_init.c 
    		strcpy (txIxpcMsg->head.dstAppName, "RANA");
			break;
		case 5 :
        	qid = msgqTable[5]; // mmcr_init.c 
    		strcpy (txIxpcMsg->head.dstAppName, "RDRANA");
			break;
		case 6 :
        	qid = msgqTable[6]; // mmcr_init.c 
    		strcpy (txIxpcMsg->head.dstAppName, "SMPP");
			break;
		case 7 :
        	qid = msgqTable[7]; // mmcr_init.c 
    		strcpy (txIxpcMsg->head.dstAppName, "RLEG0");
			break;
		case 8 :
        	qid = msgqTable[8]; // mmcr_init.c 
    		strcpy (txIxpcMsg->head.dstAppName, "RLEG1");
			break;
		case 9 :
        	qid = msgqTable[9]; // mmcr_init.c 
    		strcpy (txIxpcMsg->head.dstAppName, "RLEG2");
			break;
		case 10 :
        	qid = msgqTable[10]; // mmcr_init.c 
    		strcpy (txIxpcMsg->head.dstAppName, "RLEG3");
			break;
		case 11 :
        	qid = msgqTable[11]; // mmcr_init.c 
    		strcpy (txIxpcMsg->head.dstAppName, "RLEG4");
			break;
	}			

	txIxpcMsg->head.bodyLen = sizeof(NOTIFY_SIG);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;
	if (memcpy ((void*)txIxpcMsg->body, &stNOTIFY, sizeof(NOTIFY_SIG)) == NULL) {
		sprintf(trcBuf, "memcpy err = %s\n", strerror(errno));
		trclib_writeLogErr(FL,trcBuf);
		return -1;
	}

	if (msgsnd(qid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0)
	{
		sprintf(trcBuf, "[oldSendAppNoty] msgsnd error=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr(FL,trcBuf);
		return -1;
	}
	sprintf(trcBuf, "[oldSendAppNoty] sType=%d msgsnd snd ok (%s)\n", sType, txIxpcMsg->head.dstAppName );
	trclib_writeLogErr(FL,trcBuf);

    return 1;
}


// cmdType에 따른 qid를 찾아 반환한다.
//ixpc[0], mmcr[1], wap1ana1[2],wap1ana2[3],wap1ana3[4],wap1ana4[5],wap1ana5[6]
//uawapana[7], udrgen[8],aaaif[9],sdmd[10]
//trcdr1[11],trcdr2[12],trcdr3[13],trcdr4[14],trcdr5[15],trcdr6[16],trcdr7[17],trcdr8[18],trcdr9[19],trcdr10[20]
//httpana1[21],httpana2[22],httpana3[23],httpana4[24],httpana5[25]
int findMsgQID( int type )
{
	int qid;

	if( 1<=type && type<=10 ){
        qid = msgqTable[10]; //sdmd
    }else if ( 11<=type && type<=20 ){ /* .... ... */
        qid = msgqTable[7]; //uawapana
    }else if ( 21<=type && type<=30 ){
        qid = msgqTable[9]; //aaaifQid
    }else if ( 31<=type && type<=40 ){
        qid = msgqTable[8]; //udrgen
    }else if ( 41<=type && type<=50){/* .... ... */
        qid = msgqTable[2]; //wap1ana1
    }else if ( 51<=type && type<=60 ){
        qid = msgqTable[21]; //httpPara1
    }else if ( 61<=type && type<=70 ){/* .... ... */
        qid = msgqTable[8]; //udrgen
    }else if ( 71<=type && type<=80 ){
        qid = msgqTable[11]; //trcdr1
    }else if ( 81<=type && type<=90 ){
        qid = msgqTable[8]; //udrgen
    }else{
        qid = -1;
    }

	return qid;
}

// jjinri ALIAS등록시 NULL 인 경우 $ 문자로 대체 -> $ 로 현재 집어 넣고 있음 . 
char * convertNulltoDollar( char * item ) {
	if( item[0] == ' ' ) 
		item = "-";

	return item;
}

