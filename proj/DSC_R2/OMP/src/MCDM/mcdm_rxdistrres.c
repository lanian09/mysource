#include "mcdm_proto.h"

extern int		ixpcQid;
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern char		trcBuf[4096], trcTmp[1024];
extern time_t	currentTime;
extern McdmJobTblContext		mcdmJobTbl[MCDM_NUM_TP_JOB_TBL];
extern int						lastAllocJobNo;
extern McdmDistribMmcTblContext	mcdmDistrMmcTbl[MCDM_MAX_DISTRIB_MMC];
extern int		numDistrMmc;
extern int		trcFlag, trcLogFlag;


//------------------------------------------------------------------------------
// 명령어를 처리한 결과 메시지를 수신한 경우 호출된다.
// 1. 수신한 메시지를 mcdmJobTbl.resBuf에 넣는다.
//    응답을 보낸 시스템 별로 resBuf가 따로 있다.
// 2. 마지막 응답 메시지를 받았으면(contFlag==0) resFlag를 setting한다.
// 3. request를 보낸 각 노드로부터 응답을 모두 받은 상태가 아니면 계속 응답을 기다려야
//	하고 모두 받았으면 mmcd로 결과 메시지를 보내고 mcdmJobTbl을 해지한다.
//------------------------------------------------------------------------------
int mcdm_rxDistribMmcRes (GeneralQMsgType *rxGenQMsg)
{
	int		i, jobNo, sysIdx, msgLen, bodyLen;
	McdmResBufContext	*lastNode, *ptr;
	IxpcQMsgType		*rxIxpcMsg;
	MMLResMsgType		*mmlResMsg;

	rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg->body;
	mmlResMsg = (MMLResMsgType*)rxIxpcMsg->body;

	// req를 전달할때 mcdm에서 할당한 jobNo를 mmcdJobNo 자리에 넣어 보냈었음.
	//
	jobNo = mmlResMsg->head.mmcdJobNo;
	mmlResMsg->head.extendTime = ntohs(mmlResMsg->head.extendTime);

	// jobNo가 잘못되거나, 이미 timeout처리된 후 뒤늦게 수신한 경우 폐기한다.
	//
	if (!mcdmJobTbl[jobNo].tpInd) {
		sprintf(trcBuf,"[mcdm_rxDistribMmcRes] not used jobNo=%d; cmd=%s, src=%s-%s\n%s\n",
				jobNo, mmlResMsg->head.cmdName, rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName,mmlResMsg->body);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	// 응답을 보낸 시스템별로 resBuf가 따로 있는데,
	//	몇번째 resBuf에 저장할 것인지 결정하기 위해 어디서 왔는지 확인한다.
	//
	for (i=0; i < mcdmJobTbl[jobNo].reqSysCnt; i++) {
		if (!strcasecmp (mcdmJobTbl[jobNo].dstSysName[i], rxIxpcMsg->head.srcSysName)) {
			sysIdx = i;
			break;
		}
	}
	if (i >= mcdmJobTbl[jobNo].reqSysCnt) {
		sprintf(trcBuf,"[mcdm_rxDistribMmcRes] unknown srcSysName; cmd=%s, src=%s-%s\n",
				mmlResMsg->head.cmdName, rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (trcFlag || trcLogFlag) {
		sprintf(trcBuf,"[mcdm_rxDistribMmcRes] recv response; jobNo=%d, cmd=%s, src=%s-%s, cont=%d, extendT=%d\n%s\n",
				jobNo, mmlResMsg->head.cmdName, rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName,
				mmlResMsg->head.contFlag, mmlResMsg->head.extendTime, mmlResMsg->body);
		trclib_writeLog (FL,trcBuf);
	}
	//
	// 수신한 메시지를 resBuf에 쌓는다.
	//

	// resBuf는 linked list로 구성되어 있으므로 last node와 append할 pointer를 찾는다.
	//
	lastNode = ptr = mcdmJobTbl[jobNo].resBuf[sysIdx];
	if (ptr != NULL) {
		do { // resBuf의 끝으로 이동
			lastNode = ptr;
			ptr = ptr->next;
		} while (ptr != NULL);
	}

	// 저장하기 위한 memory할당
	//
	if ((ptr = (McdmResBufContext*) malloc (sizeof(McdmResBufContext))) == NULL) {
		sprintf(trcBuf,"[mcdm_rxDistribMmcRes] malloc fail; jobNo=%d, cmd=%s, src=%s-%s\n",
				jobNo, mmlResMsg->head.cmdName, rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	// 수신한 내용 저장
	//
	//memcpy ((void*)&ptr->rxIxpcMsg, rxIxpcMsg, sizeof(IxpcQMsgType));
	bodyLen = rxIxpcMsg->head.bodyLen;
	msgLen  = sizeof(rxIxpcMsg->head) + bodyLen;
	memcpy ((void*)&ptr->rxIxpcMsg, rxIxpcMsg, msgLen);
	ptr->rxIxpcMsg.body[bodyLen] = 0;

	// linked list 구성
	//
	ptr->next = NULL;
	ptr->prev = lastNode;
	if (lastNode != NULL) {
		lastNode->next = ptr;
	} else { // 첫번째 노드가 추가된 경우이면
		mcdmJobTbl[jobNo].resBuf[sysIdx] = ptr;
	}


	// 마지막 응답 메시지를 받았으면(contFlag==0) resFlag를 setting한다.
	//
	if (!mmlResMsg->head.contFlag) {
		mcdmJobTbl[jobNo].resFlag[sysIdx] = 1;
	} else {
		mcdmJobTbl[jobNo].deadlineTime +=  mmlResMsg->head.extendTime;
	}

	// request를 보낸 각 노드로부터 응답을 모두 받은 상태가 아니면 계속 응답을 기다려야
	//	하고 모두 받았으면 mmcd로 결과 메시지를 보내고 mcdmJobTbl을 해지한다.
	//
	for (i=0; i < mcdmJobTbl[jobNo].reqSysCnt; i++) {
		if (!mcdmJobTbl[jobNo].resFlag[i])
			break;
	}
	if (i < mcdmJobTbl[jobNo].reqSysCnt){
		if(mmlResMsg->head.contFlag)
			mcdm_sendDistribMmcRes2MMCD2 (jobNo, mmlResMsg->head.contFlag, mmlResMsg->head.resCode, 
										  mmlResMsg->head.extendTime, mmlResMsg->body);
		return 0;
	}

	// mmcd로 결과 메시지를 보낸다.
	//
	mcdm_sendDistribMmcRes2MMCD (jobNo);

	// mcdmJobTbl을 해지한다.
	//
	mcdm_deallocJobTbl (jobNo);

	return 1;

} //----- End of mcdm_rxDistribMmcRes -----//


#define SEND_DISTRIB_RES_2_MMCD   														\
do {																					\
	txIxpcMsg->head.seqNo = seqNo++;													\
	txIxpcMsg->head.bodyLen = sizeof(txMmlResMsg->head) + strlen(txMmlResMsg->body) + 1;\
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;							\
    if (msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {					\
        sprintf(trcBuf,"[mcdm_sendDistribMmcRes2MMCD] msgsnd fail; err=%d(%s)\n%s",		\
                errno, strerror(errno), txMmlResMsg->body);								\
        trclib_writeLogErr (FL,trcBuf);													\
        return -1;																		\
    } else {																			\
        if (trcFlag || trcLogFlag) {													\
            sprintf(trcBuf,"[mcdm_sendDistribMmcRes2MMCD] send to MMCD\n%s",			\
					txMmlResMsg->body);													\
            trclib_writeLog (FL,trcBuf);												\
        }																				\
    }																					\
	commlib_microSleep(50000);															\
} while(0)

//------------------------------------------------------------------------------
// 각 노드들로 부터 수신한 응답 메시지를 mmcd로 보낸다.
// - resBuf에는 각 노드들로부터 수신한 응답 메시지를 노드(MP)별로 쌓아 두었다.
// - 각 노드별로 resBuf에서 꺼내 mmcd로 보내는데, 마지막 노드(MP)에서 받은 마지막 메시지인
//   경우에만 end로 보낸다.
// - resBuf가 비어 있는 놈은 응답을 한번도 받지 못한 경우이다.
// - resBuf가 NULL은 아닌데 resFlag가 setting되지 않는 놈은 continue로 응답을 받은 후
//   end로 마지막 응답을 받지 못한 경우이다.
//------------------------------------------------------------------------------
int mcdm_sendDistribMmcRes2MMCD (int jobNo)
{
//	int		i, sysIdx, txLen;
	int		sysIdx, txLen;
	char	seqNo=1;
	GeneralQMsgType		txGenQMsg;
	McdmResBufContext	*ptr;
	IxpcQMsgType		*txIxpcMsg;//, *rxIxpcMsg;
	MMLResMsgType		*txMmlResMsg, *rxMmlResMsg;

	txGenQMsg.mtype = MTYPE_MMC_RESPONSE;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	txMmlResMsg = (MMLResMsgType*)txIxpcMsg->body;

	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	// ixpc routing header
	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, mcdmJobTbl[jobNo].srcSysName);
	strcpy (txIxpcMsg->head.dstAppName, mcdmJobTbl[jobNo].srcAppName);
	txIxpcMsg->head.byteOrderFlag = BYTE_ORDER_TAG;

	// mml result header
	strcpy (txMmlResMsg->head.cmdName, mcdmJobTbl[jobNo].cmdName);
	txMmlResMsg->head.mmcdJobNo  = mcdmJobTbl[jobNo].mmcdJobNo;

	for (sysIdx=0; sysIdx < mcdmJobTbl[jobNo].reqSysCnt; sysIdx++)
	{
		ptr = mcdmJobTbl[jobNo].resBuf[sysIdx];

		if (ptr == NULL) // resBuf가 비어 있는 놈은 응답을 한번도 받지 못한 경우이다.
		{
			sprintf (txMmlResMsg->body, "      SYSTEM = %s\n      RESULT = FAIL\n      REASON = NO_RESPONSE\n",
					mcdmJobTbl[jobNo].dstSysName[sysIdx]);
			txMmlResMsg->head.resCode = -1; // fail
			if (sysIdx == (mcdmJobTbl[jobNo].reqSysCnt - 1)) { // 마지막 노드인 경우에만 end로 보낸다.
				txMmlResMsg->head.contFlag   = 0; // end
				txMmlResMsg->head.extendTime = 0;
				txIxpcMsg->head.segFlag = 0;
			} else {
				txMmlResMsg->head.contFlag   = 1; // continue;
				txMmlResMsg->head.extendTime = 3;
				txIxpcMsg->head.segFlag = 1;
			}
			SEND_DISTRIB_RES_2_MMCD;
			continue;
		}

		//
		// 응답을 한번 이상 받아 resBuf에 쌓여 있는 메시지가 있는 경우이다.
		// 하나씩 꺼내서 mmcd로 보낸다.
		// 마지막 노드(MP)가 보낸 마지막 메시지인 경우만 end로 보낸다.
		// - 마지막 노드가 아니면 무조건 contiue로 보낸다.
		// - resBuf에 들어있는 마지막 메시지가 아니면 continue;
		// - 마지막 노드(MP)에서 정상적으로 마지막(end) 응답 메시지를 받은 경우 end로 보낸다.
		// - resBuf가 NULL은 아닌데 resFlag가 setting되지 않은 놈은 continue로
		//   응답을 받은 후 end로 마지막 응답을 받지 못한 경우이다.
		//   -> 수신된 메시지를 모두 continue로 보낸 다음, while loop 뒤에서
		//      "NO_RESPONSE"로 한번 더 보낸다.
		//
		while (1)
		{
			// 실제 명령어를 처리한 놈에서 받은 메시지를 붙인다.
			rxMmlResMsg = (MMLResMsgType*)ptr->rxIxpcMsg.body;
			strcpy (txMmlResMsg->body, rxMmlResMsg->body);

			txMmlResMsg->head.resCode = rxMmlResMsg->head.resCode;

			if ((sysIdx == (mcdmJobTbl[jobNo].reqSysCnt - 1)) && // 마지막 노드(MP)이고
				(ptr->next == NULL) &&              // resBuf에 들어있는 마지막 메시지이고
				(mcdmJobTbl[jobNo].resFlag[sysIdx]))// 정상적으로 end를 받은 경우
			{
				txMmlResMsg->head.contFlag   = 0; // end
				txMmlResMsg->head.extendTime = 0;
				txIxpcMsg->head.segFlag = 0;
			} else {
				txMmlResMsg->head.contFlag   = 1; // continue
				txMmlResMsg->head.extendTime = 3;
				txIxpcMsg->head.segFlag = 1;
			}

			if(ptr->next == NULL)
				SEND_DISTRIB_RES_2_MMCD;

			// resBuf에는 마지막 메시지까지 보냈으면 break;
			//
			if (ptr->next == NULL) {
				break;
			} else {
				ptr = ptr->next;
			}
		} //-- end of while(1) --//


		// 정상적으로 end를 받지 못한 경우 "NO_RESPONSE"로 한번 더 보낸다.
		//
		if (!mcdmJobTbl[jobNo].resFlag[sysIdx]) {
			sprintf (txMmlResMsg->body, "      SYSTEM = %s\n      RESULT = FAIL\n      REASON = NO_RESPONSE\n",
			mcdmJobTbl[jobNo].dstSysName[sysIdx]);
			txMmlResMsg->head.resCode = -1; // fail
			if (sysIdx == (mcdmJobTbl[jobNo].reqSysCnt - 1)) { // 마지막 노드인 경우에만 end로 보낸다.
				txMmlResMsg->head.contFlag   = 0; // end
				txMmlResMsg->head.extendTime = 0;
				txIxpcMsg->head.segFlag = 0;
			} else {
				txMmlResMsg->head.contFlag   = 1; // continue;
				txMmlResMsg->head.extendTime = 3;
				txIxpcMsg->head.segFlag = 1;
			}
			SEND_DISTRIB_RES_2_MMCD;
		}

	} //-- end of for(reqSysCnt) --//

	return 1;

} //----- End of mcdm_sendDistribMmcRes2MMCD -----//

/*
 * act-backup 명령어에서 실행시 기존 mcdm_sendDistribMmcRes2MMCD 함수가 문제가 되어 만들었다
 * 기존 mcdm_sendDistribMmcRes2MMCD 함수에서 메세지가 CONTINUE 시 MCDM이 MMCD 에게 메세지를 
 * 보내지 않고 버퍼링 하고 있틑 동안 MMCD 는 response time 이 다되어 NO_RESPONE 메세지가 뜨는 문제때문 
 * mcdm_sendDistribMmcRes2MMCD 함수에서 contFlag와 extendTime 을 인자로 받오록 하여
 * MMCD 가 메세지를 계속 기다릴 수 있도록 하였다
 * 2005.8.18 강종걸(ABC Solutione)
*/
int mcdm_sendDistribMmcRes2MMCD2 (int jobNo, char cflag, char rescode, unsigned short etime, char *msg)
{
    int      txLen;
	char    seqNo=1;
	GeneralQMsgType     txGenQMsg;
//	McdmResBufContext   *ptr;
	IxpcQMsgType        *txIxpcMsg;
	MMLResMsgType       *txMmlResMsg;

	txGenQMsg.mtype = MTYPE_MMC_RESPONSE;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	txMmlResMsg = (MMLResMsgType*)txIxpcMsg->body;

	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	// ixpc routing header
	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, mcdmJobTbl[jobNo].srcSysName);
	strcpy (txIxpcMsg->head.dstAppName, mcdmJobTbl[jobNo].srcAppName);
	txIxpcMsg->head.byteOrderFlag = BYTE_ORDER_TAG;

	// mml result header
	strcpy (txMmlResMsg->head.cmdName, mcdmJobTbl[jobNo].cmdName);
	txMmlResMsg->head.mmcdJobNo  = mcdmJobTbl[jobNo].mmcdJobNo;

	strcpy (txMmlResMsg->body, msg);

	txMmlResMsg->head.contFlag   = cflag; // continue
	txMmlResMsg->head.extendTime = etime;
	txMmlResMsg->head.resCode    = rescode;
	txIxpcMsg->head.segFlag = 1;

	SEND_DISTRIB_RES_2_MMCD;

    return 1;

} //----- End of mcdm_sendDistribMmcRes2MMCD -----//


// 명령어 전송하지 못한 경우 실패 메시지를 바로 MMCD로 전송

int mcdm_send_local_fail_res (IxpcQMsgType *rxIxpcMsg, char *reason, char *reSysName, int contFlag)
//int mcdm_send_local_fail_res (IxpcQMsgType *rxIxpcMsg, char *reason)
{
	int		txLen;
	char	seqNo=1;
	GeneralQMsgType		txGenQMsg;
	IxpcQMsgType		*txIxpcMsg;
	//MMLResMsgType		*txMmlResMsg, *rxMmlResMsg;
	MMLResMsgType		*txMmlResMsg;
	MMLReqMsgType		*rxMmlReqMsg;

	txGenQMsg.mtype = MTYPE_MMC_RESPONSE;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	txMmlResMsg = (MMLResMsgType*)txIxpcMsg->body;
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	//rxMmlResMsg = (MMLResMsgType*)rxIxpcMsg->body;

	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	// ixpc routing header
	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
	strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);
	txIxpcMsg->head.byteOrderFlag = BYTE_ORDER_TAG;

	// mml result header
	strcpy (txMmlResMsg->head.cmdName, rxMmlReqMsg->head.cmdName );
	txMmlResMsg->head.mmcdJobNo  = rxMmlReqMsg->head.mmcdJobNo;

	sprintf (txMmlResMsg->body, "      SYSTEM = %s\n      RESULT = FAIL\n      REASON = %s\n", reSysName, reason );
			
	txMmlResMsg->head.resCode = -1; // fail
	txMmlResMsg->head.contFlag   = contFlag; // end
	//txMmlResMsg->head.contFlag   = 0; // end
	if (contFlag) 
		txMmlResMsg->head.extendTime = 30;
	else
		txMmlResMsg->head.extendTime = 0;
	txIxpcMsg->head.segFlag = 0;

	SEND_DISTRIB_RES_2_MMCD;

	return 1;

} //----- End of mcdm_send_local_fail_res  -----//
