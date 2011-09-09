#include "cond_proto.h"

extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern char	trcBuf[4096], trcTmp[1024];
extern int	ixpcQid, nmsifQid;
extern int	trcFlag, trcLogFlag;
extern int stsLogId;

extern InhMsgTbl *inhMsg;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int cond_txConsoleMsg2GUI (GeneralQMsgType *rxGenQMsg)
{
	int		txLen, i;
	SockLibMsgType	txSockMsg;
	IxpcQMsgType	*rxIxpcMsg;
	char	*tmp, *tmp2, *end, code1[4], code2[4];
	int logId;

	memset(&txSockMsg, 0, sizeof(SockLibMsgType));

	rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg->body;

	if(rxGenQMsg->mtype == MTYPE_TRC_CONSOLE)
	{
		txSockMsg.head.mapType = 1;
		logId = stsLogId;
		logPrint(logId,FL,"GUI SEND %s",rxIxpcMsg->body);
		sprintf(trcBuf,"[MTYPE_TRC_CONSOLE] srcApp: %s\n", rxIxpcMsg->head.srcAppName);
		trclib_writeLogErr (FL,trcBuf);
	}
	else if(rxGenQMsg->mtype == MTYPE_UAWAP_TRANSLOG)
		txSockMsg.head.mapType = 2;
	else if(rxGenQMsg->mtype == MTYPE_NO_TRANSMITTED_ACT){	
		txSockMsg.head.mapType = 3;
	}	
	else if(rxGenQMsg->mtype == MTYPE_NO_TRANSMITTED_DACT){	

		txSockMsg.head.mapType = 4;
	}
	else if(rxGenQMsg->mtype == MTYPE_QUEUE_CLEAR_REPORT){
		txSockMsg.head.mapType = 5;
		//fprintf(stderr, "test qcleat result\n");	
	}	
	else
		txSockMsg.head.mapType = 0;

	strcpy (txSockMsg.body, rxIxpcMsg->body);
#if 0
	sprintf(trcBuf,"[cond_txConsoleMsg2GUI] srcApp: %s\n", rxIxpcMsg->head.srcAppName);
	trclib_writeLogErr (FL,trcBuf);
#endif	
	if (txSockMsg.head.mapType < 3 ){
 		tmp=rxIxpcMsg->body;

		end = tmp+strlen(tmp);
		for(;;tmp++)
		if(*tmp != '\n' ) break;

		for(tmp2=tmp; (tmp2 != 0) && (tmp2 <= end); tmp2++){
			if(*tmp2 != 'S' && *tmp2 != 'A' ) continue;

			for(i=1; i<5; i++){
				if( *(tmp2+i) < '0' || *(tmp2+i) > '9') break;
			}

			if (i<5) continue;

			strncpy(code1, tmp2, 1);
			if( !strncasecmp(code1, "S", 1) )
				strncpy(code1, "STS", 3);
			else if( !strncasecmp(code1, "A", 1) )
				strncpy(code1, "ALM", 3);

			strncpy(code2, tmp2+1, 4);
			break;
		}

		for(i=0; i<MAX_INH_MSG_CNT; i++){
			if(!strncasecmp(inhMsg->msgType[i], code1,3) && !strncasecmp(inhMsg->msgNum[i], code2, 4) 
							&& inhMsg->msgFlag[i] == MSG_INHIB ){
				return 0;
			}
		}
	}
// 	socklib에서 head.bodyLen를 network byte order로 바꾼다.
	txSockMsg.head.mapType = htonl(txSockMsg.head.mapType); // jjinri 2009.05.05
	txSockMsg.head.bodyLen = htonl(rxIxpcMsg->head.bodyLen);
	txLen = sizeof(txSockMsg.head) + rxIxpcMsg->head.bodyLen;
	socklib_broadcast2Clients ((char*)&txSockMsg, txLen);
	return 1;

} //----- End of cond_txConsoleMsg2GUI -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int cond_txConsoleMsg2nmsif (GeneralQMsgType *rxGenQMsg)
{
    int     txLen;
    IxpcQMsgType    *rxIxpcMsg;
	
#if 0
	if (rxGenQMsg->mtype == MTYPE_MAP_NOTIFICATION)
        rxGenQMsg->mtype = MTYPE_STATUS_REPORT;
#endif
    rxGenQMsg->mtype = MTYPE_CONSOLE_REPORT;

    rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg->body;

    strcpy (rxIxpcMsg->head.srcAppName, myAppName);
    strcpy (rxIxpcMsg->head.dstAppName, "NMSIF");

    txLen = sizeof(rxIxpcMsg->head) + rxIxpcMsg->head.bodyLen;

    if (msgsnd(nmsifQid, (void*)rxGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf,"[cond_txConsoleMsg2nmsif] msgsnd fail; err=%d(%s); src(%s-%s)\n%s",
                errno, strerror(errno),
                rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName, rxIxpcMsg->body);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    } else {
        if (trcFlag || trcLogFlag) {
            sprintf(trcBuf,"[cond_txConsoleMsg2nmsif] src(%s-%s)\n%s",
                    rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName, rxIxpcMsg->body);
            trclib_writeLog (FL,trcBuf);
        }
    }

    return 1;

} //----- End of cond_txConsoleMsg2nmsif -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int cond_txMMLResult (
			IxpcQMsgType *rxIxpcMsg,
			char *buff,
			char resCode,
			char contFlag,
			unsigned short extendTime,
			char segFlag,
			char seqNo
			)
{
	int				txLen;
	GeneralQMsgType	txGenQMsg;
	IxpcQMsgType	*txIxpcMsg;
	MMLResMsgType	*txMmlResMsg;
	MMLReqMsgType	*rxMmlReqMsg;

	txGenQMsg.mtype = MTYPE_MMC_RESPONSE;
	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	txMmlResMsg = (MMLResMsgType*)txIxpcMsg->body;
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	// ixpc routing header
	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
	strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);
	txIxpcMsg->head.segFlag = segFlag;
	txIxpcMsg->head.seqNo   = seqNo;

	// mml result header
	txMmlResMsg->head.mmcdJobNo  = rxMmlReqMsg->head.mmcdJobNo;
	txMmlResMsg->head.extendTime = extendTime;
	txMmlResMsg->head.resCode    = resCode;
	txMmlResMsg->head.contFlag   = contFlag;
	strcpy (txMmlResMsg->head.cmdName, rxMmlReqMsg->head.cmdName);

	// result message
	strcpy (txMmlResMsg->body, buff);
	txIxpcMsg->head.bodyLen = sizeof(txMmlResMsg->head) + strlen(buff);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if (msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[cond_txMMLResult] msgsnd fail to MMCD; err=%d(%s)\n%s",
				errno, strerror(errno), buff);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[cond_txMMLResult] send to MMCD\n%s", buff);
			trclib_writeLog (FL,trcBuf);
		}
	}
	return 1;

} //----- End of cond_txMMLResult -----//

