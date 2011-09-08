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
// ��ɾ ó���� ��� �޽����� ������ ��� ȣ��ȴ�.
// 1. ������ �޽����� mcdmJobTbl.resBuf�� �ִ´�.
//    ������ ���� �ý��� ���� resBuf�� ���� �ִ�.
// 2. ������ ���� �޽����� �޾�����(contFlag==0) resFlag�� setting�Ѵ�.
// 3. request�� ���� �� ���κ��� ������ ��� ���� ���°� �ƴϸ� ��� ������ ��ٷ���
//	�ϰ� ��� �޾����� mmcd�� ��� �޽����� ������ mcdmJobTbl�� �����Ѵ�.
//------------------------------------------------------------------------------
int mcdm_rxDistribMmcRes (GeneralQMsgType *rxGenQMsg)
{
	int		i, jobNo, sysIdx, msgLen, bodyLen;
	McdmResBufContext	*lastNode, *ptr;
	IxpcQMsgType		*rxIxpcMsg;
	MMLResMsgType		*mmlResMsg;

	rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg->body;
	mmlResMsg = (MMLResMsgType*)rxIxpcMsg->body;

	// req�� �����Ҷ� mcdm���� �Ҵ��� jobNo�� mmcdJobNo �ڸ��� �־� ���¾���.
	//
	jobNo = mmlResMsg->head.mmcdJobNo;
	mmlResMsg->head.extendTime = ntohs(mmlResMsg->head.extendTime);

	// jobNo�� �߸��ǰų�, �̹� timeoutó���� �� �ڴʰ� ������ ��� ����Ѵ�.
	//
	if (!mcdmJobTbl[jobNo].tpInd) {
		sprintf(trcBuf,"[mcdm_rxDistribMmcRes] not used jobNo=%d; cmd=%s, src=%s-%s\n%s\n",
				jobNo, mmlResMsg->head.cmdName, rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName,mmlResMsg->body);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	// ������ ���� �ý��ۺ��� resBuf�� ���� �ִµ�,
	//	���° resBuf�� ������ ������ �����ϱ� ���� ��� �Դ��� Ȯ���Ѵ�.
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
	// ������ �޽����� resBuf�� �״´�.
	//

	// resBuf�� linked list�� �����Ǿ� �����Ƿ� last node�� append�� pointer�� ã�´�.
	//
	lastNode = ptr = mcdmJobTbl[jobNo].resBuf[sysIdx];
	if (ptr != NULL) {
		do { // resBuf�� ������ �̵�
			lastNode = ptr;
			ptr = ptr->next;
		} while (ptr != NULL);
	}

	// �����ϱ� ���� memory�Ҵ�
	//
	if ((ptr = (McdmResBufContext*) malloc (sizeof(McdmResBufContext))) == NULL) {
		sprintf(trcBuf,"[mcdm_rxDistribMmcRes] malloc fail; jobNo=%d, cmd=%s, src=%s-%s\n",
				jobNo, mmlResMsg->head.cmdName, rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	// ������ ���� ����
	//
	//memcpy ((void*)&ptr->rxIxpcMsg, rxIxpcMsg, sizeof(IxpcQMsgType));
	bodyLen = rxIxpcMsg->head.bodyLen;
	msgLen  = sizeof(rxIxpcMsg->head) + bodyLen;
	memcpy ((void*)&ptr->rxIxpcMsg, rxIxpcMsg, msgLen);
	ptr->rxIxpcMsg.body[bodyLen] = 0;

	// linked list ����
	//
	ptr->next = NULL;
	ptr->prev = lastNode;
	if (lastNode != NULL) {
		lastNode->next = ptr;
	} else { // ù��° ��尡 �߰��� ����̸�
		mcdmJobTbl[jobNo].resBuf[sysIdx] = ptr;
	}


	// ������ ���� �޽����� �޾�����(contFlag==0) resFlag�� setting�Ѵ�.
	//
	if (!mmlResMsg->head.contFlag) {
		mcdmJobTbl[jobNo].resFlag[sysIdx] = 1;
	} else {
		mcdmJobTbl[jobNo].deadlineTime +=  mmlResMsg->head.extendTime;
	}

	// request�� ���� �� ���κ��� ������ ��� ���� ���°� �ƴϸ� ��� ������ ��ٷ���
	//	�ϰ� ��� �޾����� mmcd�� ��� �޽����� ������ mcdmJobTbl�� �����Ѵ�.
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

	// mmcd�� ��� �޽����� ������.
	//
	mcdm_sendDistribMmcRes2MMCD (jobNo);

	// mcdmJobTbl�� �����Ѵ�.
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
// �� ����� ���� ������ ���� �޽����� mmcd�� ������.
// - resBuf���� �� ����κ��� ������ ���� �޽����� ���(MP)���� �׾� �ξ���.
// - �� ��庰�� resBuf���� ���� mmcd�� �����µ�, ������ ���(MP)���� ���� ������ �޽�����
//   ��쿡�� end�� ������.
// - resBuf�� ��� �ִ� ���� ������ �ѹ��� ���� ���� ����̴�.
// - resBuf�� NULL�� �ƴѵ� resFlag�� setting���� �ʴ� ���� continue�� ������ ���� ��
//   end�� ������ ������ ���� ���� ����̴�.
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

		if (ptr == NULL) // resBuf�� ��� �ִ� ���� ������ �ѹ��� ���� ���� ����̴�.
		{
			sprintf (txMmlResMsg->body, "      SYSTEM = %s\n      RESULT = FAIL\n      REASON = NO_RESPONSE\n",
					mcdmJobTbl[jobNo].dstSysName[sysIdx]);
			txMmlResMsg->head.resCode = -1; // fail
			if (sysIdx == (mcdmJobTbl[jobNo].reqSysCnt - 1)) { // ������ ����� ��쿡�� end�� ������.
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
		// ������ �ѹ� �̻� �޾� resBuf�� �׿� �ִ� �޽����� �ִ� ����̴�.
		// �ϳ��� ������ mmcd�� ������.
		// ������ ���(MP)�� ���� ������ �޽����� ��츸 end�� ������.
		// - ������ ��尡 �ƴϸ� ������ contiue�� ������.
		// - resBuf�� ����ִ� ������ �޽����� �ƴϸ� continue;
		// - ������ ���(MP)���� ���������� ������(end) ���� �޽����� ���� ��� end�� ������.
		// - resBuf�� NULL�� �ƴѵ� resFlag�� setting���� ���� ���� continue��
		//   ������ ���� �� end�� ������ ������ ���� ���� ����̴�.
		//   -> ���ŵ� �޽����� ��� continue�� ���� ����, while loop �ڿ���
		//      "NO_RESPONSE"�� �ѹ� �� ������.
		//
		while (1)
		{
			// ���� ��ɾ ó���� �𿡼� ���� �޽����� ���δ�.
			rxMmlResMsg = (MMLResMsgType*)ptr->rxIxpcMsg.body;
			strcpy (txMmlResMsg->body, rxMmlResMsg->body);

			txMmlResMsg->head.resCode = rxMmlResMsg->head.resCode;

			if ((sysIdx == (mcdmJobTbl[jobNo].reqSysCnt - 1)) && // ������ ���(MP)�̰�
				(ptr->next == NULL) &&              // resBuf�� ����ִ� ������ �޽����̰�
				(mcdmJobTbl[jobNo].resFlag[sysIdx]))// ���������� end�� ���� ���
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

			// resBuf���� ������ �޽������� �������� break;
			//
			if (ptr->next == NULL) {
				break;
			} else {
				ptr = ptr->next;
			}
		} //-- end of while(1) --//


		// ���������� end�� ���� ���� ��� "NO_RESPONSE"�� �ѹ� �� ������.
		//
		if (!mcdmJobTbl[jobNo].resFlag[sysIdx]) {
			sprintf (txMmlResMsg->body, "      SYSTEM = %s\n      RESULT = FAIL\n      REASON = NO_RESPONSE\n",
			mcdmJobTbl[jobNo].dstSysName[sysIdx]);
			txMmlResMsg->head.resCode = -1; // fail
			if (sysIdx == (mcdmJobTbl[jobNo].reqSysCnt - 1)) { // ������ ����� ��쿡�� end�� ������.
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
 * act-backup ��ɾ�� ����� ���� mcdm_sendDistribMmcRes2MMCD �Լ��� ������ �Ǿ� �������
 * ���� mcdm_sendDistribMmcRes2MMCD �Լ����� �޼����� CONTINUE �� MCDM�� MMCD ���� �޼����� 
 * ������ �ʰ� ���۸� �ϰ� �ֺz ���� MMCD �� response time �� �ٵǾ� NO_RESPONE �޼����� �ߴ� �������� 
 * mcdm_sendDistribMmcRes2MMCD �Լ����� contFlag�� extendTime �� ���ڷ� �޿��� �Ͽ�
 * MMCD �� �޼����� ��� ��ٸ� �� �ֵ��� �Ͽ���
 * 2005.8.18 ������(ABC Solutione)
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


// ��ɾ� �������� ���� ��� ���� �޽����� �ٷ� MMCD�� ����

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
