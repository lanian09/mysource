#include "mmcd_proto.h"
#include <ctype.h>

extern int		condQid, ixpcQid, nmsibQid, cmdLogId, mmcLogId;
extern time_t	currentTime, MML_NUM_CMD;
extern char		trcBuf[4096], trcTmp[1024];
extern char		inputErrBuf[1024];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern int		trcFlag, trcLogFlag;
extern SFM_sfdb *sfdb;
extern MMLCmdContext		*mmlCmdTbl;
extern MMLHelpContext		*mmlHelpTbl;
extern MmcdJobTblContext	*mmcdJobTbl;
extern MmcdUserTblContext	*mmcdUserTbl;
extern MmcdCliTblContext	*mmcdCliTbl;
MMcdUserIPTblContext    *mmcdIPTbl;     // 2009.07.17 by sjs

//------------------------------------------------------------------------------
// client ���� ��������, �ش� client�� ���� �������� mmc�� job table���� ã��
//	application���� cancel �޽����� ������ �����Ѵ�.
// - client table�� �����Ѵ�.
// - user table�� lastLogoutTime���� update�Ѵ�.
//------------------------------------------------------------------------------
int mmcd_exeClientDisconn (
		int cliSockFd	// ������ ������ client�� ���ӵǾ����� socket fd
		)
{
	int		cliIndex,userIndex,jobNo;

	for (jobNo=0; jobNo<MML_NUM_TP_JOB_TBL; jobNo++) {
		if (mmcdJobTbl[jobNo].tpInd && mmcdJobTbl[jobNo].cliSockFd == cliSockFd) {
			// client ���� ��������, �ش� client�� ���� �������� mmc�� job table���� ã��
			//	application���� cancel �޽����� ������ job table�� �����Ѵ�.
			mmcd_sendCancMsg2App (jobNo);
			memset ((void*)&mmcdJobTbl[jobNo], 0, sizeof(MmcdJobTblContext));
		}
	}

	// search client table
	if ((cliIndex = mmcd_getCliIndex (cliSockFd)) < 0) {
		sprintf(trcBuf,"[mmcd_exeClientDisconn] already deleted client(fd=%d)\n", cliSockFd);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf(trcBuf,"[mmcd_exeClientDisconn] disconnected user=%s; fd=%d\n",
			mmcdCliTbl[cliIndex].userName, cliSockFd);
	trclib_writeLogErr (FL,trcBuf);

	userIndex = mmcdCliTbl[cliIndex].userIndex;

	mmcdIPTbl[mmcdCliTbl[cliIndex].useIPListIndex].useFlag = 0;
	// update user table
	(mmcdUserTbl[userIndex].loginCnt)--;
	mmcdUserTbl[userIndex].lastLogoutTime = currentTime;

	// lastLogoutTime�� passwd file�� �ݿ��ϱ� ����
	mmcd_savePasswdFile();

	// delete client table
	memset ((void*)&mmcdCliTbl[cliIndex], 0, sizeof(MmcdCliTblContext));

	return 1;

} //----- End of mmcd_exeClientDisconn -----//



//------------------------------------------------------------------------------
// cmdName�� ��ġ�ϴ� ��ɾ mmlCmdTbl���� ã�� �� index�� return�Ѵ�.
//------------------------------------------------------------------------------
int mmcd_getCmdIndex (
		char *cmdName	// command name
		)
{
	MMLCmdContext	*ptr;

	if ((ptr = (MMLCmdContext*) bsearch (
					cmdName,
					mmlCmdTbl,
					MML_NUM_CMD,
					sizeof(MMLCmdContext),
					mmcd_bsrchCmp)) == NULL) {
		sprintf(inputErrBuf,"NOT_REGISTERED_COMMAND");
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[mmcd_getCmdIndex] not registered command : %s\n", cmdName);
			trclib_writeLog (FL,trcBuf);
		}
		return -1;
	}
	return (ptr - mmlCmdTbl);

} //----- End of mmcd_getCmdIndex -----//



//------------------------------------------------------------------------------
// cliSockFd�� ��ġ�ϴ� ���� mmcdCliTbl���� ã�� �� index�� return�Ѵ�.
//------------------------------------------------------------------------------
int mmcd_getCliIndex (
		int cliSockFd
		)
{
	int		i;

	for (i=0; i<MML_NUM_TP_CLIENT_TBL; i++) {
		if (cliSockFd == mmcdCliTbl[i].cliSockFd)
			return i;
	}
	sprintf(inputErrBuf,"INTERNAL_ERROR(not found client table):LOG-IN FIRST");
	sprintf(trcBuf,"[mmcd_getCliIndex] not found cliSockFd(%d) in mmcdCliTbl\n", cliSockFd);
	trclib_writeLogErr (FL,trcBuf);
	return -1;
} //----- End of mmcd_getCliIndex -----//



//------------------------------------------------------------------------------
// userName�� ��ġ�ϴ� ���� mmcdUserTbl���� ã�� �� index�� return�Ѵ�.
//------------------------------------------------------------------------------
int mmcd_getUserIndex (
		char *userName
		)
{
	int		i;

	for (i=0; i<MML_NUM_TP_USER_TBL; i++) {

#if 0 /* jhnoh : 030430 DEBUG */
		fprintf(stderr, "\n userName : %s", userName);
		fprintf(stderr, "\n UserTbl  : %s", mmcdUserTbl[i].userName);
#endif
		if (!strcmp (userName, mmcdUserTbl[i].userName))
			return i;
	}
	sprintf(trcBuf,"[mmcd_getUserIndex] not found userName(%s) in mmcdUserTbl\n", userName);
	trclib_writeLogErr (FL,trcBuf);
	return -1;
} //----- End of mmcd_getUserIndex -----//

//------------------------------------------------------------------------------
// ��ɾ ó���� Application���� ���� ��ɾ� ó�� �䱸 �޽����� �����Ѵ�.
// - command name�� �� �Ķ������ value�� string type���� ������.
// - �Ķ���͵��� command file�� ��ϵ� ������� ���ʷ� �ִµ�, �Էµ��� ���� optional
//	�Ķ���ʹ� �ش� �ڸ��� NULL�� ����.
// - �Ķ���͵��� inputCmdInfo()�� �̹� ������� ���ĵǾ� �ִ�. -> mmcd_arrangeInputPara()
// - �Ķ������ �̸��̳� id�� ������ �ʿ䰡 ����. application���� ���ǵ� �������
//	������ �ǰ� NULL�̸� �Էµ��� �ʴ� ������ �Ǵ��ϸ� �ȴ�.
//------------------------------------------------------------------------------
int mmcd_makeReqMsg (
		MMLInputCmdInfo *inputCmdInfo,	// �Էµ� ��ɾ� ������ ����ִ�.
		GeneralQMsgType	*txGenQMsg		// ���⿡ �޽����� �����Ѵ�.
		)
{
//  +------------------------+   ----------------------------+
//  | mtype                  |                               |
//  +------------------------+   --------------+             |
//  | +--------------------+ |                 |        GeneralQMsgType
//  | | ixpc header        | |                 |             |
//  | +--------------------+ |            IxpcQMsgType       |
//  | | +----------------+ | |   ----+         |             |
//  | | | mml req header | | |       |         |             |
//  | | |                | | |  MMLReqMsgType  |             |
//  | | |                | | |       |         |             |
//  | | |                | | |       |         |             |
//  | | +----------------+ | |   ----+         |             |
//  | +--------------------+ |   --------------+             |
//  +------------------------+   ----------------------------+ 

	int		i,cmd, count=0;
	IxpcQMsgType	*txIxpcMsg;
	MMLReqMsgType	*txReqMsg;

	cmd = inputCmdInfo->cmdIndex;

	txGenQMsg->mtype = MTYPE_MMC_REQUEST;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg->body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	txReqMsg = (MMLReqMsgType*)txIxpcMsg->body;


	// command name�� �Ķ���͵��� value�鸸 �ִ´�.
	//
	strcpy (txReqMsg->head.cmdName, inputCmdInfo->cmdName);

//printf("********jean %s\n",mmlCmdTbl[cmd].dstAppName); 
	if (  strncmp(mmlCmdTbl[cmd].dstAppName, "MCDM", strlen(mmlCmdTbl[cmd].dstSysName) ) ) {
		for (i=0; i<mmlCmdTbl[inputCmdInfo->cmdIndex].paraCnt; i++) {
			if( !(strlen(inputCmdInfo->paraInfo[i].paraVal))) continue;
        	strcpy (txReqMsg->head.para[count].paraName, inputCmdInfo->paraInfo[i].paraName);
        	strcpy (txReqMsg->head.para[count].paraVal, inputCmdInfo->paraInfo[i].paraVal);
#if 0
printf("\n\n============jean1 mmcd_makeReqMsg\n%d=[%s] [%s]\n", count, txReqMsg->head.para[count].paraName, txReqMsg->head.para[count].paraVal);
printf("==========jean1 mmcd_makeReqMsg end \n\n");
#endif
			count++;
    	}
		txReqMsg->head.paraCnt = count;
	} else {
		memset(txReqMsg->head.para, 0x00, MML_MAX_PARA_CNT*sizeof(CommPara));
		memcpy(txReqMsg->head.para, inputCmdInfo->paraInfo, MML_MAX_PARA_CNT*sizeof(CommPara));
        txReqMsg->head.paraCnt = mmlCmdTbl[inputCmdInfo->cmdIndex].paraCnt;

#if 0
printf("\n\n==========jean2 mmcd_makeReqMsg\n");
for(i=0; i<txReqMsg->head.paraCnt; i++ )
{
    printf("%d=[%s][%s]\n", i, txReqMsg->head.para[i].paraName, txReqMsg->head.para[i].paraVal);
}
printf("==========jean2 mmcd_makeReqMsg end \n\n");
#endif

	}
	
	// ixpc header�� ä�� ixpc�� �ش� application���� ������ �� �ֵ����Ѵ�.
	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, mmlCmdTbl[cmd].dstSysName);
	strcpy (txIxpcMsg->head.dstAppName, mmlCmdTbl[cmd].dstAppName);
	txIxpcMsg->head.bodyLen = sizeof(txReqMsg->head) - sizeof(txReqMsg->head.para)
							+ txReqMsg->head.paraCnt*sizeof(CommPara)+30;
//printf("\n===jean [mmcd_makeReqMsg] src=%s-%s, dst=%s-%s\n\n", 
//  txIxpcMsg->head.srcSysName, txIxpcMsg->head.srcAppName, txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName);

	return 1;

} //----- End of mmcd_makeReqMsg -----//



//------------------------------------------------------------------------------
// mmcdJobTbl���� jobNo�� �Ҵ��Ͽ� ���� ������ �����Ѵ�.
// - canc-exe-cmd�� ������ ���� application ������ �Բ� �����Ѵ�.
//------------------------------------------------------------------------------
int mmcd_saveReqData2JobTbl (
		MMLInputCmdInfo *inputCmdInfo,	// �Էµ� ��ɾ� ������ ����ִ�.
		GeneralQMsgType	*txGenQMsg		// application���� ���� �޽����� �����Ǿ� �ִ�.
		)
{
	int		cmd,jobNo;
	IxpcQMsgType	*txIxpcMsg;
	MMLReqMsgType	*txReqMsg;

	cmd = inputCmdInfo->cmdIndex;

	// jobNo�� �Ҵ��Ѵ�.
	// - sequencial search�� ���ʿ� ��� �ִ� ���� ã�´�. (why? ������!!!)
	//
	for (jobNo=0; jobNo < MML_NUM_TP_JOB_TBL && mmcdJobTbl[jobNo].tpInd; jobNo++) ;
	if (jobNo >= MML_NUM_TP_JOB_TBL) {
		sprintf(inputErrBuf,"INTERNAL_ERROR(no more job)");
		sprintf(trcBuf,"[mmcd_saveReqMsg2JobTbl] no more seqeunce id\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	//printf("[mmcd_saveReqData2JobTbl] jobNo=%d, curr=%d, inputString=%s\n",
	//		jobNo, currentTime, inputCmdInfo->inputString);

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg->body;
	txReqMsg = (MMLReqMsgType*)txIxpcMsg->body;

	txReqMsg->head.mmcdJobNo = jobNo; // mmcd_makeReqMsg()���� �������� �̹� ��� setting �ߴ�.

	// save to mmcdJobTbl
	//
	mmcdJobTbl[jobNo].tpInd = 1; // ����� ǥ��
	mmcdJobTbl[jobNo].cliSockFd = inputCmdInfo->cliSockFd; // ����� ���� client
	mmcdJobTbl[jobNo].cliReqId  = inputCmdInfo->cliReqId;  // client���� �Ҵ��� key��
	mmcdJobTbl[jobNo].batchFlag = inputCmdInfo->batchFlag;  // client���� �Ҵ��� key��
	mmcdJobTbl[jobNo].cmdIndex  = inputCmdInfo->cmdIndex;  // cmdTbl������ index
	mmcdJobTbl[jobNo].nmsibFlag = inputCmdInfo->nmsibFlag; // nmsib���� ���� ������� ǥ��
	if(!strcasecmp(inputCmdInfo->cmdName, "srch-udr"))
		mmcdJobTbl[jobNo].deadlineTime = currentTime + (60*30);	// it may takes a little time over 10 minutes
	else
		mmcdJobTbl[jobNo].deadlineTime = currentTime + MML_DEFAULT_RES_TIMER; // ���� ��� ���� �ð�
	mmcdJobTbl[jobNo].clientType = inputCmdInfo->clientType;
	strcpy (mmcdJobTbl[jobNo].inputString, inputCmdInfo->inputString); // request ����
	strcpy (mmcdJobTbl[jobNo].cmdName, inputCmdInfo->cmdName);
	strcpy (mmcdJobTbl[jobNo].userName, inputCmdInfo->userName);
	strcpy (mmcdJobTbl[jobNo].dstSysName, txIxpcMsg->head.dstSysName);
	strcpy (mmcdJobTbl[jobNo].dstAppName, txIxpcMsg->head.dstAppName);
/*
	for (jobNo=0; jobNo<MML_NUM_TP_JOB_TBL && mmcdJobTbl[jobNo].tpInd; jobNo++) {
		printf(" job=%d,cliFd=%d,cmdidx=%d,deadline=%d,input=[%s],dst=%s-%s\n",
				jobNo, mmcdJobTbl[jobNo].cliSockFd, mmcdJobTbl[jobNo].cmdIndex,
				mmcdJobTbl[jobNo].deadlineTime, mmcdJobTbl[jobNo].inputString,
				mmcdJobTbl[jobNo].dstSysName, mmcdJobTbl[jobNo].dstAppName);
	}
*/
	return 1;

} //---- End of mmcd_saveReqData2JobTbl -----//


//------------------------------------------------------------------------------
// 1. history ������� ��ȸ�� �ֱ� ��ɾ client���� buffer�� 20������ �׾� �д�.
// 2. dis-cmd-his ������� ��ȸ�� log file�� ����Ѵ�.
// 3. dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
//------------------------------------------------------------------------------
int mmcd_logCmdInput (
		MMLInputCmdInfo *inputCmdInfo // �Էµ� ��ɾ� ����
		)
{
	int		cliIndex,hisIndex;
	char	buf[1024];

	// client�� ��ɾ� �Է� history�� �����ϴ� history buffer ����Ѵ�.
	// - history ��ɾ�� ��ȸ�� ���ȴ�.
	//
	cliIndex = inputCmdInfo->cliIndex;
	hisIndex = mmcdCliTbl[cliIndex].hisIndex;
	strcpy (mmcdCliTbl[cliIndex].history[hisIndex],
			inputCmdInfo->inputString);
	mmcdCliTbl[cliIndex].hisIndex = NEXT(mmcdCliTbl[cliIndex].hisIndex, MMCD_NUM_HISTORY_BUFF);

	// ��ɾ� ����� �̷��� ��ȸ�� �� �ֵ��� MMCD_CMDHIS_FILE �� MMCD_MMCLOG_FILE��
	//	����Ѵ�.
	// - dis-cmd-his ��ɽ� MMCD_CMDHIS_FILE�� �˻��ϰ�,
	// - dis-msg-his mmc ��ɽ� MMCD_MMCLOG_FILE�� �˻��Ѵ�.
	//   ["time_stamp"] ["user_name"]
	//     INPUT : "input_data"
	//   ACCEPTED
	//
	sprintf(buf,"    %s %s %s\n    %s\n    ACCEPTED\n\n",
			sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
			inputCmdInfo->inputString);
	logPrint (cmdLogId,FL,"%s",buf);
	logPrint (mmcLogId,FL,"%s\n",buf);
	

	return 1;

} //----- End of mmcd_logCmdInput -----//



//------------------------------------------------------------------------------
// �ֱ������� ȣ��Ǿ� mmcdJobTbl�� Ȯ���Ͽ� timeout�� ���� ã�� ó���Ѵ�.
// - client�� timeout �޽����� ������ job table���� �����Ѵ�.
// - currentTime�� update�Ѵ�.
//------------------------------------------------------------------------------
int mmcd_scanJobTbl (void)
{
	int		jobNo;

	keepalivelib_increase ();

	currentTime = time(0); // update currentTime

	for (jobNo=0; jobNo<MML_NUM_TP_JOB_TBL; jobNo++) {
		if (mmcdJobTbl[jobNo].tpInd && mmcdJobTbl[jobNo].deadlineTime < currentTime) {
			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[mmcd_scanJobTbl] TIMEOUT %s (%s-%s)\n",
						mmcdJobTbl[jobNo].cmdName,
						mmcdJobTbl[jobNo].dstSysName, mmcdJobTbl[jobNo].dstAppName);
				trclib_writeLog (FL,trcBuf);
			}
			mmcd_sendTimeOut2Client (jobNo);
			memset ((void*)&mmcdJobTbl[jobNo], 0, sizeof(MmcdJobTblContext));
		}
	}

	return 1;

} //----- End of scanJobTbl -----//



//------------------------------------------------------------------------------
// client�κ��� ������ ����� �������� accept�Ǿ����� �˸��� �޽����� ������.
// - history ��ȸ�� ���� log�� �����.
// - log-in/log-out�� ������ ��� ��ɾ�� Accept �޽����� ���� ������, ���� ó��
//	��� �޽����� ���߿� ���޵Ǵ� �����̹Ƿ� Accept �޽����� continue�� ������.
//------------------------------------------------------------------------------
int mmcd_sendInputAccepted2Client (
		MMLInputCmdInfo *inputCmdInfo // client�κ��� ������ ����
		)
{
	int		txLen;
	SockLibMsgType	txSockMsg;
	MMLClientResMsgType	*txCliResMsg;

	//
	// ["time_stamp"] ["user_name"]
	//   INPUT : "input_data"
	// ACCEPTED
	//

	txCliResMsg = (MMLClientResMsgType*)txSockMsg.body;
	memset ((void*)txCliResMsg, 0, sizeof(txCliResMsg->head));

	/*blank sprintf(txCliResMsg->body, "\n\n<%d> [%s] [%s] [%s]\n  INPUT : %s\nACCEPTED\n\n",
			htonl(inputCmdInfo->cliReqId),  sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
			inputCmdInfo->inputString);*/
	sprintf(txCliResMsg->body, "\n\n    %s %s %s\n    INPUT : %s\n    ACCEPTED\n\n",
			sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
			inputCmdInfo->inputString);

	txCliResMsg->head.cliReqId = htonl(inputCmdInfo->cliReqId);
	txCliResMsg->head.batchFlag = htonl(inputCmdInfo->batchFlag);
	txCliResMsg->head.resCode  = 0; // success
	txCliResMsg->head.contFlag = 1; // Accept �޽����� ������ continue�� ������ �Ѵ�.
	
	txSockMsg.head.bodyLen = strlen(txCliResMsg->body) + sizeof(txCliResMsg->head);
	txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;

	// NMS���� ���� ��ɾ ���� NMSIB���� syntax check ����� MML port�� ���� ��ɾ�
    //  ó�� ����� ���� port�� ������ �ϹǷ� �̸� ������ �� �ֵ���, �޽����� type��
    //  contFlag�� ������.
    if (inputCmdInfo->nmsibFlag) {
        txCliResMsg->head.contFlag = MTYPE_MMC_SYNTAX_RESPONSE;
    }

	//yhshin
//	if (socklib_sndMsg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
	if (socklib_sndMsg_hdr_chg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
		printf("[mmcd_sendInputAccepted2Client] socklib_sndMsg fail Error: %s \n", strerror(errno));
		return -1;
	}
	//printf("[mmcd_sendInputAccepted2Client] ACCEPTED to client(%d)\n", inputCmdInfo->cliSockFd);

	// - history ������� ��ȸ�� �ֱ� ��ɾ client���� buffer�� 20������ �׾� �д�.
	// - dis-cmd-his ������� ��ȸ�� log file�� ����Ѵ�.
	// - dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
	mmcd_logCmdInput (inputCmdInfo);

	return 1;

} //----- End of mmcd_sendInputAccepted2Client -----//


int mmcd_sendInputConfirm2Client (
		MMLInputCmdInfo *inputCmdInfo // client�κ��� ������ ����
		)
{
	int		txLen;
	SockLibMsgType	txSockMsg;
	MMLClientResMsgType	*txCliResMsg;

	//
	// ["time_stamp"] ["user_name"]
	//   INPUT : "input_data"
	// ACCEPTED
	//

	txCliResMsg = (MMLClientResMsgType*)txSockMsg.body;
	memset ((void*)txCliResMsg, 0, sizeof(txCliResMsg->head));

	sprintf(txCliResMsg->body, "%s", inputCmdInfo->inputString);

	txCliResMsg->head.cliReqId = htonl(inputCmdInfo->cliReqId);
	txCliResMsg->head.batchFlag = htonl(inputCmdInfo->batchFlag);
	//txCliResMsg->head.confirm = 1; // need confirm 
	txCliResMsg->head.confirm = htonl(1); // need confirm : by sjjeon
	txCliResMsg->head.resCode  = 0; // success
	txCliResMsg->head.contFlag = 1; // 
	
	txSockMsg.head.bodyLen = strlen(txCliResMsg->body) + sizeof(txCliResMsg->head);
	txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;

	//yhshin
//	if (socklib_sndMsg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
	if (socklib_sndMsg_hdr_chg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
		printf("[mmcd_sendInputConfirm2Client] socklib_sndMsg fail Error: %s\n", strerror(errno));
		return -1;
	}

	// confirm�� ��� ACCEPT�� 2�� �����ϱ� ����
//	mmcd_logCmdInput (inputCmdInfo);

	return 1;

} //----- End of mmcd_sendInputAccepted2Client -----//



//------------------------------------------------------------------------------
// client�κ��� ������ string�� syntax error �� �������� �������� ó���� �� ����
//	��� ȣ��Ǿ� client�� error message�� ������.
// - command file�� ��ϵ� help �޽����� �ٿ��� ������.
// - inputErrBuf�� ó�� �Ұ� ���� �޽���
//------------------------------------------------------------------------------
int mmcd_sendInputError2Client (
		MMLInputCmdInfo *inputCmdInfo // client�κ��� ������ ����
		)
{
	int		txLen;
	SockLibMsgType	txSockMsg;
	MMLClientResMsgType	*txCliResMsg;
	#define MAXHELP 3000
	#define MAXTEMP 10000
    char allHelpMessage[MAXTEMP], sendHelpMessage[MAXHELP];
    char *pallHelpMessageMove;

    memset(allHelpMessage, 0, sizeof (allHelpMessage));
    memset(sendHelpMessage, 0, sizeof (sendHelpMessage));
    pallHelpMessageMove = allHelpMessage;

	//
	// ["time_stamp"] ["user_name"]
	//   INPUT : "input_data"
	//   ERROR : "error_message"
	// REJECTED
	//
	// "HELP................
	// .....................
	// ....................."
	//

	txCliResMsg = (MMLClientResMsgType*)txSockMsg.body;
	memset ((void*)txCliResMsg, 0, sizeof(txCliResMsg->head));

	/*blank sprintf(txCliResMsg->body, "\n\n<%d> [%s] [%s] [%s]\n  INPUT : %s\n  ERROR : %s\nREJECTED\n\n",
			htonl(inputCmdInfo->cliReqId), sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
			inputCmdInfo->inputString,
			inputErrBuf);*/
	sprintf(txCliResMsg->body, "\n\n    %s %s %s\n    INPUT : %s\n    ERROR : %s\n    REJECTED\n\n",
			sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
			inputCmdInfo->inputString,
			inputErrBuf);


	// NMS���� ���� ��ɾ ���� NMSIB���� syntax check ����� MML port�� ���� ��ɾ�
	//  ó�� ����� ���� port�� ������ �ϹǷ� �̸� ������ �� �ֵ���, �޽����� type��
	//  contFlag�� ������.
	if (inputCmdInfo->nmsibFlag) {
		txCliResMsg->head.contFlag = MTYPE_MMC_SYNTAX_RESPONSE;
		txCliResMsg->head.cliReqId = htonl(inputCmdInfo->cliReqId);
		txCliResMsg->head.batchFlag = htonl(inputCmdInfo->batchFlag);
		txCliResMsg->head.resCode = -1; // fail
		txSockMsg.head.bodyLen = strlen(txCliResMsg->body) + sizeof(txCliResMsg->head);
		txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;

//		if (socklib_sndMsg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
		if (socklib_sndMsg_hdr_chg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
			sprintf(trcBuf,"[mmcd_sendInputError2Client] socklib_sndMsg fail; INPUT ERROR to client\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		} else {
			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[mmcd_sendInputError2Client] send INPUT ERROR to client\n");
				trclib_writeLog (FL,trcBuf);
			}
		}
		// printer�� ����ϱ� ���� cond�� ������.
		mmcd_send2COND (txCliResMsg->body);
		return 1;
	}

#if HELP_MSG_OK		/* HELP �޽��� ��� ���� */
	if (inputCmdInfo->cmdIndex >= 0) {
        strcat (allHelpMessage, mmlHelpTbl[inputCmdInfo->cmdIndex].cmdHelp);
	}

    count = strlen(allHelpMessage) / MAXHELP;

    for (i=0; i <= count;  i++) {
        memset(sendHelpMessage, 0, sizeof(sendHelpMessage));

        strncpy(sendHelpMessage, pallHelpMessageMove , MAXHELP-1);
        strcat(txCliResMsg->body, sendHelpMessage);

        if (i == count)  {
            txCliResMsg->head.contFlag = 0;
        } 
        else  {
            txCliResMsg->head.contFlag = 1;
        }

		txCliResMsg->head.cliReqId = htonl(inputCmdInfo->cliReqId);
		txCliResMsg->head.batchFlag = htonl(inputCmdInfo->batchFlag);
		txCliResMsg->head.resCode = -1; // fail
		txSockMsg.head.bodyLen = strlen(txCliResMsg->body) + sizeof(txCliResMsg->head);
		txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;

//    	if (socklib_sndMsg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
    	if (socklib_sndMsg_hdr_chg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
	    	sprintf(trcBuf,"[mmcd_sendInputError2Client] socklib_sndMsg fail; INPUT ERROR to client\n");
	    	trclib_writeLogErr (FL,trcBuf);
	     	return -1;
	    } else {
	    	if (trcFlag || trcLogFlag) {
	    		sprintf(trcBuf,"[mmcd_sendInputError2Client] send INPUT ERROR to client\n");
	    		trclib_writeLog (FL,trcBuf);
	    	}
        }
		pallHelpMessageMove = pallHelpMessageMove + MAXHELP-1;
        memset(txCliResMsg->body, 0, sizeof(txCliResMsg->body));
	}

	/* HELP �޽��� ��� ���� */
	strcat (txCliResMsg->body, mmlHelpTbl[inputCmdInfo->cmdIndex].cmdHelp);

#else
    txCliResMsg->head.contFlag = 0;
/*
printf("[%d] cliReqId:%d, confirm:%d,  batchFlag:%d, errCode:%d, resCode:%d, contFlag:%d, segFlag:%d, seqNo:%d\n",
		__FUNCTION__, txCliResMsg->head.cliReqId, txCliResMsg->head.confirm,  
		txCliResMsg->head.batchFlag, txCliResMsg->head.errCode, txCliResMsg->head.resCode, 
		txCliResMsg->head.contFlag, txCliResMsg->head.segFlag, txCliResMsg->head.seqNo);
*/

	txCliResMsg->head.cliReqId = htonl(inputCmdInfo->cliReqId);
	txCliResMsg->head.confirm = htonl(inputCmdInfo->confirm); // add by sjjeon
	txCliResMsg->head.batchFlag = htonl(inputCmdInfo->batchFlag);
	txCliResMsg->head.resCode = -1; // fail
	txSockMsg.head.bodyLen = strlen(txCliResMsg->body) + sizeof(txCliResMsg->head);
	txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;

//	if (socklib_sndMsg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
	if (socklib_sndMsg_hdr_chg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
		sprintf(trcBuf,"[mmcd_sendInputError2Client] socklib_sndMsg fail; INPUT ERROR to client\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[mmcd_sendInputError2Client] send INPUT ERROR to client\n");
			trclib_writeLog (FL,trcBuf);
		}
	}
#endif /* HELP_MSG_OK */

	return 1;

} //----- End of mmcd_sendInputError2Client -----//


//------------------------------------------------------------------------------
// client�κ��� ������ string�� syntax error �� �������� �������� ó���� �� ����
//	��� ȣ��Ǿ� client�� error message�� ������.
// - command file�� ��ϵ� help �޽����� �ٿ��� ������.
// - inputErrBuf�� ó�� �Ұ� ���� �޽���
//------------------------------------------------------------------------------
int mmcd_sendInputCancel2Client (
		MMLInputCmdInfo *inputCmdInfo // client�κ��� ������ ����
		)
{
	int		txLen;
	SockLibMsgType	txSockMsg;
	MMLClientResMsgType	*txCliResMsg;

	//
	// ["time_stamp"] ["user_name"]
	//   INPUT : "input_data"
	//   ERROR : "error_message"
	// REJECTED
	//
	// "HELP................
	// .....................
	// ....................."
	//

	txCliResMsg = (MMLClientResMsgType*)txSockMsg.body;
	memset ((void*)txCliResMsg, 0, sizeof(txCliResMsg->head));

	/*blank sprintf(txCliResMsg->body, "\n\n<%d> [%s] [%s] [%s]\n  INPUT : %s\n  ERROR : %s\nCANCLED\n\n",
			htonl(inputCmdInfo->cliReqId), sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
			inputCmdInfo->inputString,
			"USER CANCELED COMMAND");*/
	sprintf(txCliResMsg->body, "\n\n    %s %s %s\n    INPUT : %s\n    ERROR : %s\n    CANCLED\n\n",
			sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
			inputCmdInfo->inputString,
			"USER CANCELED COMMAND");

	txCliResMsg->head.contFlag = 0;
	txCliResMsg->head.cliReqId = htonl(inputCmdInfo->cliReqId);
	txCliResMsg->head.batchFlag = htonl(inputCmdInfo->batchFlag);
	txCliResMsg->head.resCode = -1; // fail
	txSockMsg.head.bodyLen = strlen(txCliResMsg->body) + sizeof(txCliResMsg->head);
	txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;

//	if (socklib_sndMsg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
	if (socklib_sndMsg_hdr_chg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
    		sprintf(trcBuf,"[mmcd_sendInputError2Client] socklib_sndMsg fail; INPUT ERROR to client\n");
    		trclib_writeLogErr (FL,trcBuf);
     		return -1;
    	} else {
    		if (trcFlag || trcLogFlag) {
    			sprintf(trcBuf,"[mmcd_sendInputError2Client] send INPUT ERROR to client\n");
    			trclib_writeLog (FL,trcBuf);
    		}
	}
	memset(txCliResMsg->body, 0, sizeof(txCliResMsg->body));

	return 1;

} //----- End of mmcd_sendInputError2Client -----//



//------------------------------------------------------------------------------
// application���κ��� ���� �޽����� ���� ���� timeout ó���� ��� ȣ��Ǿ� �ش�
//	jobs�� �Է��� client�� timeout �޽����� ������.
//------------------------------------------------------------------------------
int mmcd_sendTimeOut2Client (
		int jobNo 
		)
{
	int		txLen,cmd;
	SockLibMsgType	txSockMsg;
	MMLClientResMsgType	*txCliResMsg;

	//
	// ["time_stamp"] ["user_name"]
	// "slogan"
	//   RESULT = FAIL
	//   REASON = NO_RESPONSE_FROM_APPLICATION
	//            [input_string]
	// COMPLETED
	//

	txCliResMsg = (MMLClientResMsgType*)txSockMsg.body;
	memset ((void*)txCliResMsg, 0, sizeof(txCliResMsg->head));

	cmd = mmcdJobTbl[jobNo].cmdIndex;
	
	/*blank sprintf(txCliResMsg->body, "\n\n<%d> [%s] [%s] [%s]\n%s\n  RESULT = FAIL\n  REASON = NO RESPONSE_FROM_APPLICATION\n  [INPUT : %s]\nCOMPLETED\n\n",
			htonl(mmcdJobTbl[jobNo].cliReqId),
			sysLabel,
			commlib_printTStamp(),
			mmcdJobTbl[jobNo].userName,
			mmlHelpTbl[cmd].cmdSlogan,
			mmcdJobTbl[jobNo].inputString);*/

	sprintf(txCliResMsg->body, "\n\n    %s %s %s\n    %s\n    RESULT = FAIL\n    REASON = NO RESPONSE_FROM_APPLICATION\n    [INPUT : %s]\n    COMPLETED\n\n",
			sysLabel,
			commlib_printTStamp(),
			mmcdJobTbl[jobNo].userName,
			mmlHelpTbl[cmd].cmdSlogan,
			mmcdJobTbl[jobNo].inputString);

	txCliResMsg->head.cliReqId = htonl(mmcdJobTbl[jobNo].cliReqId);
	txCliResMsg->head.batchFlag = htonl(mmcdJobTbl[jobNo].batchFlag);
	txCliResMsg->head.resCode = -1; // fail
	if (mmcdJobTbl[jobNo].nmsibFlag) {
		txCliResMsg->head.contFlag = MTYPE_MMC_RESPONSE;
	}
	else 
		txCliResMsg->head.contFlag = 0;  // last
	txCliResMsg->head.segFlag  = 0;  // not segmented
	txCliResMsg->head.seqNo    = 1;  // first message	
	
	txSockMsg.head.bodyLen = strlen(txCliResMsg->body) + sizeof(txCliResMsg->head);
	txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;

//	if (socklib_sndMsg(mmcdJobTbl[jobNo].cliSockFd, (char*)&txSockMsg, txLen) < 0) {
	if (socklib_sndMsg_hdr_chg(mmcdJobTbl[jobNo].cliSockFd, (char*)&txSockMsg, txLen) < 0) {
		sprintf(trcBuf,"[mmcd_sendTimeOut2Client] socklib_sndMsg fail; TIMEOUT to client (%s)(%s)\n",
				mmcdJobTbl[jobNo].inputString, mmcdJobTbl[jobNo].userName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[mmcd_sendTimeOut2Client] TIMEOUT to client (%s)(%s)\n",
					mmcdJobTbl[jobNo].inputString, mmcdJobTbl[jobNo].userName);
			trclib_writeLog (FL,trcBuf);
		}
	}

	// dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
	logPrint (mmcLogId,FL, "%s\n", txCliResMsg->body);



	// ��� status,alarm,MMC��� �޽����� NMS�� �����ϱ� ���� nmsib�� �����Ѵ�.
	// - client�� nmsib�̸�, �̹� socket���� �������Ƿ� ������ �ʴ´�.

	return 1;

} //----- End of mmcd_sendTimeOut2Client -----//



//------------------------------------------------------------------------------
// application���κ��� ���� �޽����� ������ ��� ȣ��Ǿ� �ش� jobs�� �Է��� client��
//	��� �޽����� ������.
//------------------------------------------------------------------------------
int mmcd_sendResult2Client (
		int jobNo,                 // mmcdJobTbl������ index
		GeneralQMsgType	*rxGenQMsg, // application���κ��� ������ �޽���
		int clientType)
{
	int		txLen,cmd;
	IxpcQMsgType		*rxIxpcMsg;
	MMLResMsgType		*rxResMsg;
	SockLibMsgType		txSockMsg;
	MMLClientResMsgType	*txCliResMsg;

	rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg->body;
	rxResMsg = (MMLResMsgType*)rxIxpcMsg->body;

	//
	// ["time_stamp"] ["user_name"]
	// "slogan"
	// "result_msg"
	// COMPLETED or CONTINUED
	//

	txCliResMsg = (MMLClientResMsgType*)txSockMsg.body;
	memset ((void*)txCliResMsg, 0, sizeof(txCliResMsg->head));

	cmd = mmcdJobTbl[jobNo].cmdIndex;
	/*blank sprintf(txCliResMsg->body, "\n\n<%d> [%s] [%s] [%s]\n%s\n%s",
			htonl(mmcdJobTbl[jobNo].cliReqId), sysLabel, commlib_printTStamp(), mmcdJobTbl[jobNo].userName,
			mmlHelpTbl[cmd].cmdSlogan,
			rxResMsg->body);
	*/
	/* jean sprintf(txCliResMsg->body, "\n\n    %s %s %s\n    %s\n%s\n",
			sysLabel, commlib_printTStamp(), mmcdJobTbl[jobNo].userName,
			mmlHelpTbl[cmd].cmdSlogan,
			rxResMsg->body);
	*/


	sprintf(txCliResMsg->body, "\n\n    %s %s %s\n    %s\n    ACTIVE SYSTEM IS %s\n%s",
			sysLabel, commlib_printTStamp(), mmcdJobTbl[jobNo].userName,
			mmlHelpTbl[cmd].cmdSlogan,
			sfdb->active_sys_name, 
			rxResMsg->body);
	if (rxResMsg->head.contFlag) {
		strcat (txCliResMsg->body,"    CONTINUED\n\n"); 
	} else {
		strcat (txCliResMsg->body,"    COMPLETED\n\n"); 
	}


	// NMSIB������ COND�� MMCD�κ��� NMS�� ������ �޽����� ����������
	// - �޽����� ������ �����ؾ� �ϴµ�, msgQ�� ���� ������ ��쿡�� mtype�� ����
	//   ������ �� �ְ�, socket���� ������ ��쿡�� mtype�� �����Ƿ� mtype�� ����
	//   ������� �ʴ� field�� contFlag�� mtype�� �־��ְ� �� ������ ������ �� �ִ�.
	//
	if (mmcdJobTbl[jobNo].nmsibFlag) {
		txCliResMsg->head.contFlag = (char)rxGenQMsg->mtype; // MMC_RES, STAT_SHORT_TERM, STAT_LONG_TERM
	} else {
		txCliResMsg->head.contFlag = rxResMsg->head.contFlag; // last(0) or not_last(1)
	}
	txCliResMsg->head.cliReqId = htonl(mmcdJobTbl[jobNo].cliReqId);

	txCliResMsg->head.batchFlag = htonl(mmcdJobTbl[jobNo].batchFlag);
	txCliResMsg->head.resCode  = rxResMsg->head.resCode;  // success(0) or fail(-1)
	txCliResMsg->head.segFlag  = rxIxpcMsg->head.segFlag; // not_segmented(0) segmented(1)
	txCliResMsg->head.seqNo    = rxIxpcMsg->head.seqNo;   // not_defined(0), first(1), second(2),...
	txSockMsg.head.bodyLen = strlen(txCliResMsg->body) + sizeof(txCliResMsg->head);
	txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;

//	if (socklib_sndMsg(mmcdJobTbl[jobNo].cliSockFd, (char*)&txSockMsg, txLen) < 0) {
	if (socklib_sndMsg_hdr_chg(mmcdJobTbl[jobNo].cliSockFd, (char*)&txSockMsg, txLen) < 0) {
		sprintf(trcBuf,"[mmcd_sendResult2Client] send fail; result to client (%s)(%s)\n",
				mmcdJobTbl[jobNo].inputString, mmcdJobTbl[jobNo].userName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[mmcd_sendResult2Client] send result to client (%s)(%s)\n",
					mmcdJobTbl[jobNo].inputString, mmcdJobTbl[jobNo].userName);
			trclib_writeLog (FL,trcBuf);
		}
	}


	// dis-msg-his, dis-cmd-his �� �α� ��ȸ ��ɾ� ��� �޽����� �α׿� ������� �ʴ´�.
	// NMS�ε� ������ �ʴ´�.
	//
	if (strcasecmp (mmcdJobTbl[jobNo].cmdName, "dis-msg-his") &&
		strcasecmp (mmcdJobTbl[jobNo].cmdName, "dis-cmd-his"))
	{
		// dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
		logPrint (mmcLogId,FL, "%s\n", txCliResMsg->body);
	}

	if (!mmcdJobTbl[jobNo].nmsibFlag)
    {
        if (strcasecmp (mmcdJobTbl[jobNo].cmdName, "dis-msg-his") &&
            strcasecmp (mmcdJobTbl[jobNo].cmdName, "dis-cmd-his"))
        {
            mmcd_send2Nmsib(txCliResMsg->body,
                            rxGenQMsg->mtype,
                            rxIxpcMsg->head.segFlag,
                            rxIxpcMsg->head.seqNo,
                            rxIxpcMsg->head.msgId );
        }
    }

 	mmcd_send2COND (txCliResMsg->body);

	return 1;

} //----- End of mmcd_sendResult2Client -----//


//------------------------------------------------------------------------------
// canc-exe-cmd ��ɿ� ���� application���� ��� �޽����� ������.
//------------------------------------------------------------------------------
int mmcd_sendCancMsg2App (
		int jobNo   // mmcdJobTbl������ index
		)
{
	int				txLen;
	GeneralQMsgType	txGenQMsg;
	IxpcQMsgType	*txIxpcMsg;
	MMLReqMsgType	*txReqMsg;

	txGenQMsg.mtype = MTYPE_MMC_REQUEST;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	txReqMsg = (MMLReqMsgType*)txIxpcMsg->body;

	//
	txReqMsg->head.mmcdJobNo = jobNo;
	strcpy (txReqMsg->head.cmdName, "CANC-EXE-CMD");

	// ù��° �Ķ���� jobNo��, �ι�° �ĸ����Ϳ� �ش� command_name�� �ִ´�.
	//
#if 0
	sprintf (txReqMsg->head.para[0], "%d", jobNo);
	sprintf (txReqMsg->head.para[1], "%s", mmcdJobTbl[jobNo].cmdName);
#endif
	sprintf (txReqMsg->head.para[0].paraVal, "%d", jobNo);

#if 0 /* jhnoh : 030425 �׽�Ʈ */
	fprintf(stderr, "\n\n -- jhnoh -- JOBNO �� ������ �����ϴ�. [%s]\n\n", txReqMsg->head.para[0].paraVal);
#endif
    sprintf (txReqMsg->head.para[1].paraVal, "%s", mmcdJobTbl[jobNo].cmdName);

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, mmcdJobTbl[jobNo].dstSysName);
	strcpy (txIxpcMsg->head.dstAppName, mmcdJobTbl[jobNo].dstAppName);

	txIxpcMsg->head.bodyLen = sizeof(txReqMsg->head) - sizeof(txReqMsg->head.para)
								+ (2 * COMM_MAX_NAME_LEN);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if (msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) 
	{
//		sprintf(trcBuf,"[mmcd_sendCancMsg2App] msgsnd fail; err=%d(%s); CANCEL [job=%d(%s)(%s)] to (%s-%s)\n",
		sprintf(trcBuf,"[mmcd_sendCancMsg2App] msgsnd fail; err=%d(%s); CANCEL [job=%d] to (%s-%s)\n",
				errno, strerror(errno), jobNo,
//				(char*)txReqMsg->head.para[0], (char*)txReqMsg->head.para[1],
				txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	} 
	else 
	{
		if (trcFlag || trcLogFlag) 
		{
//			sprintf(trcBuf,"[mmcd_sendCancMsg2App] send CANCEL [job=%d(%s)] to (%s-%s)\n",
			sprintf(trcBuf,"[mmcd_sendCancMsg2App] send CANCEL [job=%d] to (%s-%s)\n",
					jobNo, //txReqMsg->head.para[1],
					txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName);
			trclib_writeLog (FL,trcBuf);
		}
	}

	return 1;

} //----- End of mmcd_sendCancMsg2App -----//

int mmcd_send2Nmsib (
            char    *msg,    // GUI �� client�� ���� �޽���
            long    mtype,   // NMSIB���� �޽����� ������ �� �ִ� ��
            char    segFlag, // segmentation ���θ� ǥ���ϴ� flag
            char    seqNo,   // sequence number
            int     msgId    // NMSIB�� ���� ��ɾ�� �ο��Ǵ� temporary ID
            )
{
    int             txLen;
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;

    // ������ ��ɾ� ����޽������� STMD���� mtype�� �ܱ�, ����⸦ �����ؼ� ������.
    // �� �̿� �Ϲ� ��ɾ� ����޽����� �ش� ���μ����鿡�� MMC_RES�� ������.
    //
    txGenQMsg.mtype = mtype;
    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);
    strcpy (txIxpcMsg->head.dstSysName, "OMP");
    strcpy (txIxpcMsg->head.dstAppName, "NMSIB");

    // segmemted�� ���� ��� NMSIB���� �ٽ� �̾���� ����� ������ �� �ֵ���
    //  MMCD���� �� ��ɾ�� ���� �ϳ��� �ο��ϰ� �� ���� ä��������.
    //
    txIxpcMsg->head.msgId   = msgId;
    txIxpcMsg->head.segFlag = segFlag; // not_segmented(0) segmented(1)
    txIxpcMsg->head.seqNo   = seqNo;   // not_defined(0), first(1), second(2),...

    strcpy (txIxpcMsg->body, msg);

    txIxpcMsg->head.bodyLen = strlen(msg) + 1;
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;
#ifdef NMSIB
    if (msgsnd(nmsibQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf,"[mmcd_send2Nmsib] msgsnd fail; err=%d(%s) qid=%d mtype=%d\n", errno, strerror(errno), nmsibQid, mtype);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    } else {
        if (trcFlag || trcLogFlag) {
            sprintf(trcBuf,"[mmcd_send2Nmsib] send to NMSIB\n");
            trclib_writeLog (FL,trcBuf);
        }
    }
#endif
    return 1;

} //----- End of mmcd_send2Nmsib -----//

//------------------------------------------------------------------------------
// printer�� ����ϱ� ���� cond�� ������.
// - cond�� �α׸� ����� �ʰ� �ٷ� printer�� ������.
//------------------------------------------------------------------------------
int mmcd_send2COND (char *msg)
{
    int             txLen;
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;

    //
    txGenQMsg.mtype = MTYPE_MMC_RESPONSE;
    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);
    strcpy (txIxpcMsg->head.dstSysName, "OMP");
    strcpy (txIxpcMsg->head.dstAppName, "COND");

    //
    txIxpcMsg->head.msgId   = 1;
    txIxpcMsg->head.segFlag = 0; // not_segmented(0) segmented(1)
    txIxpcMsg->head.seqNo   = 1;   // not_defined(0), first(1), second(2),...

    strcpy (txIxpcMsg->body, msg);

    txIxpcMsg->head.bodyLen = strlen(msg) + 1;
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if (msgsnd(condQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf,"[mmcd_send2COND] msgsnd fail; err=%d(%s)\n", errno, strerror(errno));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    } else {
        if (trcFlag || trcLogFlag) {
            sprintf(trcBuf,"[mmcd_send2COND] send to COND\n");
            trclib_writeLog (FL,trcBuf);
        }
    }

    return 1;

} //----- End of mmcd_send2COND -----//


//------------------------------------------------------------------------------
// sfdb->sys���� system name�� ��ġ�ϴ� index�� ã�´�.
//------------------------------------------------------------------------------
int mmcd_getSysIndexByName (char *sysName)
{
    int     i;

    for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) {
        if (!strcasecmp (sfdb->sys[i].commInfo.name, sysName))
            return i;
    }
    sprintf(trcBuf,"[mmcd_getSysIndexByName] not found sysName[%s]\n", sysName);
    trclib_writeLogErr (FL,trcBuf);
    return -1;

} //----- End of mmcd_getSysIndexByName -----//
