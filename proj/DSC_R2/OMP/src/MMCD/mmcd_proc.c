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
// client 접속 끊어지면, 해당 client에 의해 실행중인 mmc를 job table에서 찾아
//	application으로 cancel 메시지를 보내고 삭제한다.
// - client table을 삭제한다.
// - user table에 lastLogoutTime등을 update한다.
//------------------------------------------------------------------------------
int mmcd_exeClientDisconn (
		int cliSockFd	// 접속이 끊어진 client가 접속되었었던 socket fd
		)
{
	int		cliIndex,userIndex,jobNo;

	for (jobNo=0; jobNo<MML_NUM_TP_JOB_TBL; jobNo++) {
		if (mmcdJobTbl[jobNo].tpInd && mmcdJobTbl[jobNo].cliSockFd == cliSockFd) {
			// client 접속 끊어지면, 해당 client에 의해 실행중인 mmc를 job table에서 찾아
			//	application으로 cancel 메시지를 보내고 job table을 해제한다.
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

	// lastLogoutTime를 passwd file에 반영하기 위해
	mmcd_savePasswdFile();

	// delete client table
	memset ((void*)&mmcdCliTbl[cliIndex], 0, sizeof(MmcdCliTblContext));

	return 1;

} //----- End of mmcd_exeClientDisconn -----//



//------------------------------------------------------------------------------
// cmdName과 일치하는 명령어를 mmlCmdTbl에서 찾아 그 index를 return한다.
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
// cliSockFd와 일치하는 놈을 mmcdCliTbl에서 찾아 그 index를 return한다.
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
// userName과 일치하는 놈을 mmcdUserTbl에서 찾아 그 index를 return한다.
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
// 명령어를 처리할 Application으로 보낼 명령어 처리 요구 메시지를 구성한다.
// - command name과 각 파라미터의 value를 string type으로 보낸다.
// - 파라미터들은 command file에 등록된 순서대로 차례로 넣는데, 입력되지 않은 optional
//	파라미터는 해당 자리에 NULL이 들어간다.
// - 파라미터들은 inputCmdInfo()에 이미 순서대로 정렬되어 있다. -> mmcd_arrangeInputPara()
// - 파라미터의 이름이나 id는 전달할 필요가 없다. application에서 정의된 순서대로
//	꺼내면 되고 NULL이면 입력되지 않는 것으로 판단하면 된다.
//------------------------------------------------------------------------------
int mmcd_makeReqMsg (
		MMLInputCmdInfo *inputCmdInfo,	// 입력된 명령어 정보가 들어있다.
		GeneralQMsgType	*txGenQMsg		// 여기에 메시지를 구성한다.
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


	// command name과 파라미터들의 value들만 넣는다.
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
	
	// ixpc header를 채워 ixpc가 해당 application으로 전달할 수 있도록한다.
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
// mmcdJobTbl에서 jobNo를 할당하여 각종 정보를 저장한다.
// - canc-exe-cmd를 보내기 위해 application 정보도 함께 저장한다.
//------------------------------------------------------------------------------
int mmcd_saveReqData2JobTbl (
		MMLInputCmdInfo *inputCmdInfo,	// 입력된 명령어 정보가 들어있다.
		GeneralQMsgType	*txGenQMsg		// application으로 보낼 메시지가 구성되어 있다.
		)
{
	int		cmd,jobNo;
	IxpcQMsgType	*txIxpcMsg;
	MMLReqMsgType	*txReqMsg;

	cmd = inputCmdInfo->cmdIndex;

	// jobNo를 할당한다.
	// - sequencial search로 앞쪽에 비어 있는 것을 찾는다. (why? 묻지마!!!)
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

	txReqMsg->head.mmcdJobNo = jobNo; // mmcd_makeReqMsg()에서 나머지는 이미 모두 setting 했다.

	// save to mmcdJobTbl
	//
	mmcdJobTbl[jobNo].tpInd = 1; // 사용중 표시
	mmcdJobTbl[jobNo].cliSockFd = inputCmdInfo->cliSockFd; // 명령을 보낸 client
	mmcdJobTbl[jobNo].cliReqId  = inputCmdInfo->cliReqId;  // client에서 할당한 key값
	mmcdJobTbl[jobNo].batchFlag = inputCmdInfo->batchFlag;  // client에서 할당한 key값
	mmcdJobTbl[jobNo].cmdIndex  = inputCmdInfo->cmdIndex;  // cmdTbl에서의 index
	mmcdJobTbl[jobNo].nmsibFlag = inputCmdInfo->nmsibFlag; // nmsib에서 보낸 명령인지 표시
	if(!strcasecmp(inputCmdInfo->cmdName, "srch-udr"))
		mmcdJobTbl[jobNo].deadlineTime = currentTime + (60*30);	// it may takes a little time over 10 minutes
	else
		mmcdJobTbl[jobNo].deadlineTime = currentTime + MML_DEFAULT_RES_TIMER; // 응답 대기 만료 시간
	mmcdJobTbl[jobNo].clientType = inputCmdInfo->clientType;
	strcpy (mmcdJobTbl[jobNo].inputString, inputCmdInfo->inputString); // request 원본
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
// 1. history 명령으로 조회할 최근 명령어를 client별로 buffer에 20개까지 쌓아 둔다.
// 2. dis-cmd-his 명령으로 조회할 log file에 기록한다.
// 3. dis-msg-his mmc 명령으로 조회할 log file에 기록한다.
//------------------------------------------------------------------------------
int mmcd_logCmdInput (
		MMLInputCmdInfo *inputCmdInfo // 입력된 명령어 정보
		)
{
	int		cliIndex,hisIndex;
	char	buf[1024];

	// client별 명령어 입력 history를 저장하는 history buffer 기록한다.
	// - history 명령어로 조회시 사용된다.
	//
	cliIndex = inputCmdInfo->cliIndex;
	hisIndex = mmcdCliTbl[cliIndex].hisIndex;
	strcpy (mmcdCliTbl[cliIndex].history[hisIndex],
			inputCmdInfo->inputString);
	mmcdCliTbl[cliIndex].hisIndex = NEXT(mmcdCliTbl[cliIndex].hisIndex, MMCD_NUM_HISTORY_BUFF);

	// 명령어 입출력 이력을 조회할 수 있도록 MMCD_CMDHIS_FILE 과 MMCD_MMCLOG_FILE에
	//	기록한다.
	// - dis-cmd-his 명령시 MMCD_CMDHIS_FILE를 검색하고,
	// - dis-msg-his mmc 명령시 MMCD_MMCLOG_FILE를 검색한다.
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
// 주기적으로 호출되어 mmcdJobTbl을 확인하여 timeout된 놈을 찾아 처리한다.
// - client로 timeout 메시지를 보내고 job table에서 삭제한다.
// - currentTime을 update한다.
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
// client로부터 수신한 명령이 정상적을 accept되었음을 알리는 메시지를 보낸다.
// - history 조회를 위해 log를 남긴다.
// - log-in/log-out을 제외한 모든 명령어는 Accept 메시지를 먼저 보내고, 실제 처리
//	결과 메시지가 나중에 전달되는 절차이므로 Accept 메시지는 continue를 보낸다.
//------------------------------------------------------------------------------
int mmcd_sendInputAccepted2Client (
		MMLInputCmdInfo *inputCmdInfo // client로부터 수신한 정보
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
	txCliResMsg->head.contFlag = 1; // Accept 메시지는 무조건 continue로 보내야 한다.
	
	txSockMsg.head.bodyLen = strlen(txCliResMsg->body) + sizeof(txCliResMsg->head);
	txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;

	// NMS에서 보낸 명령어에 대해 NMSIB에서 syntax check 결과는 MML port로 실제 명령어
    //  처리 결과는 상태 port로 보내야 하므로 이를 구분할 수 있도록, 메시지의 type을
    //  contFlag에 보낸다.
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

	// - history 명령으로 조회할 최근 명령어를 client별로 buffer에 20개까지 쌓아 둔다.
	// - dis-cmd-his 명령으로 조회할 log file에 기록한다.
	// - dis-msg-his mmc 명령으로 조회할 log file에 기록한다.
	mmcd_logCmdInput (inputCmdInfo);

	return 1;

} //----- End of mmcd_sendInputAccepted2Client -----//


int mmcd_sendInputConfirm2Client (
		MMLInputCmdInfo *inputCmdInfo // client로부터 수신한 정보
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

	// confirm인 경우 ACCEPT가 2번 찍히니까 삭제
//	mmcd_logCmdInput (inputCmdInfo);

	return 1;

} //----- End of mmcd_sendInputAccepted2Client -----//



//------------------------------------------------------------------------------
// client로부터 수신한 string이 syntax error 등 여러가지 원인으로 처리될 수 없는
//	경우 호출되어 client로 error message를 보낸다.
// - command file에 등록된 help 메시지를 붙여서 보낸다.
// - inputErrBuf에 처리 불가 원인 메시지
//------------------------------------------------------------------------------
int mmcd_sendInputError2Client (
		MMLInputCmdInfo *inputCmdInfo // client로부터 수신한 내용
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


	// NMS에서 보낸 명령어에 대해 NMSIB에서 syntax check 결과는 MML port로 실제 명령어
	//  처리 결과는 상태 port로 보내야 하므로 이를 구분할 수 있도록, 메시지의 type을
	//  contFlag에 보낸다.
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
		// printer로 출력하기 위해 cond로 보낸다.
		mmcd_send2COND (txCliResMsg->body);
		return 1;
	}

#if HELP_MSG_OK		/* HELP 메시지 출력 안함 */
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

	/* HELP 메시지 출력 안함 */
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
// client로부터 수신한 string이 syntax error 등 여러가지 원인으로 처리될 수 없는
//	경우 호출되어 client로 error message를 보낸다.
// - command file에 등록된 help 메시지를 붙여서 보낸다.
// - inputErrBuf에 처리 불가 원인 메시지
//------------------------------------------------------------------------------
int mmcd_sendInputCancel2Client (
		MMLInputCmdInfo *inputCmdInfo // client로부터 수신한 내용
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
// application으로부터 응답 메시지를 받지 못해 timeout 처리된 경우 호출되어 해당
//	jobs을 입력한 client로 timeout 메시지를 보낸다.
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

	// dis-msg-his mmc 명령으로 조회할 log file에 기록한다.
	logPrint (mmcLogId,FL, "%s\n", txCliResMsg->body);



	// 모든 status,alarm,MMC결과 메시지를 NMS로 전달하기 위해 nmsib로 전달한다.
	// - client가 nmsib이면, 이미 socket으로 보냈으므로 보내지 않는다.

	return 1;

} //----- End of mmcd_sendTimeOut2Client -----//



//------------------------------------------------------------------------------
// application으로부터 응답 메시지를 수신한 경우 호출되어 해당 jobs을 입력한 client로
//	결과 메시지를 보낸다.
//------------------------------------------------------------------------------
int mmcd_sendResult2Client (
		int jobNo,                 // mmcdJobTbl에서의 index
		GeneralQMsgType	*rxGenQMsg, // application으로부터 수신한 메시지
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


	// NMSIB에서는 COND와 MMCD로부터 NMS로 전송할 메시지를 수신했을때
	// - 메시지의 유형을 구분해야 하는데, msgQ를 통해 수신한 경우에는 mtype을 보고
	//   구분할 수 있고, socket으로 수신한 경우에는 mtype이 없으므로 mtype을 현재
	//   사용하지 않는 field인 contFlag에 mtype을 넣어주고 이 값으로 구분할 수 있다.
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


	// dis-msg-his, dis-cmd-his 등 로그 조회 명령어 결과 메시지는 로그에 기록하지 않는다.
	// NMS로도 보내지 않는다.
	//
	if (strcasecmp (mmcdJobTbl[jobNo].cmdName, "dis-msg-his") &&
		strcasecmp (mmcdJobTbl[jobNo].cmdName, "dis-cmd-his"))
	{
		// dis-msg-his mmc 명령으로 조회할 log file에 기록한다.
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
// canc-exe-cmd 명령에 의해 application으로 취소 메시지를 보낸다.
//------------------------------------------------------------------------------
int mmcd_sendCancMsg2App (
		int jobNo   // mmcdJobTbl에서의 index
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

	// 첫번째 파라미터 jobNo를, 두번째 파마미터에 해당 command_name을 넣는다.
	//
#if 0
	sprintf (txReqMsg->head.para[0], "%d", jobNo);
	sprintf (txReqMsg->head.para[1], "%s", mmcdJobTbl[jobNo].cmdName);
#endif
	sprintf (txReqMsg->head.para[0].paraVal, "%d", jobNo);

#if 0 /* jhnoh : 030425 테스트 */
	fprintf(stderr, "\n\n -- jhnoh -- JOBNO 는 다음과 같습니다. [%s]\n\n", txReqMsg->head.para[0].paraVal);
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
            char    *msg,    // GUI 등 client로 보낸 메시지
            long    mtype,   // NMSIB에서 메시지를 구분할 수 있는 값
            char    segFlag, // segmentation 여부를 표시하는 flag
            char    seqNo,   // sequence number
            int     msgId    // NMSIB를 위해 명령어마다 부여되는 temporary ID
            )
{
    int             txLen;
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;

    // 통계관련 명령어 결과메시지에는 STMD에서 mtype에 단기, 중장기를 구분해서 보낸다.
    // 그 이외 일반 명령어 결과메시지는 해당 프로세스들에서 MMC_RES로 보낸다.
    //
    txGenQMsg.mtype = mtype;
    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);
    strcpy (txIxpcMsg->head.dstSysName, "OMP");
    strcpy (txIxpcMsg->head.dstAppName, "NMSIB");

    // segmemted된 놈인 경우 NMSIB에서 다시 이어붙인 놈들을 구분할 수 있도록
    //  MMCD에서 각 명령어마다 값을 하나씩 부여하고 그 값을 채워보낸다.
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
// printer로 출력하기 위해 cond로 보낸다.
// - cond는 로그를 남기기 않고 바로 printer로 보낸다.
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
// sfdb->sys에서 system name과 일치하는 index를 찾는다.
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
