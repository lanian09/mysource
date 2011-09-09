#include "mmcd_proto.h"

extern int		ixpcQid;
extern char		inputErrBuf[1024];
extern char		trcBuf[4096], trcTmp[1024];
extern MMLCmdContext		*mmlCmdTbl;
extern MmcdJobTblContext    *mmcdJobTbl;
extern MmcdCliTblContext	*mmcdCliTbl;
extern int		trcFlag, trcLogFlag;
extern int checkEqual(char *);


//------------------------------------------------------------------------------
// client로부터 수신한 input sting을 분석하여 syntax를 확인하고 처리할 프로세스로
//	전달한다.
// - inputCmdInfo에 각종 정보를 setting한 후 application으로 보낼 txReqMsg를 구성하고
//	mmcdJoTbl에 실행 정보를 저장한 후 client로 ACCEPT 메시지를 보낸다.
// 1. 명령을 보낸 client와 연결된 socket fd와 원본 input string을 저장한다.
// 2. client socket fd를 확인하여 메시지를 보낸 client 정보를 mmcdCliTbl에서 찾는다.
// 3. input string을 token으로 잘라내어 inputCmdInfo에 cmdName과 parameter들을 저장한다.
// 4. 수신한 cmdName과 일치하는 명령어를 mmlCmdTbl에서 찾는다.
// 5. mmlCmdTbl을 검색하여 각 파라미터들의 이름을 채운다.  파라미터 갯수 초과, 파라미터
//	이름 확인, 파라미터 중복확인 등 error check
// 6. 파라미터 list를 mmlCmdTbl을 확인하여 등록된 순서대로 정렬한다.
// 7. 필수 파라미터 입력여부 확인
// 8. 각 파라미터 값들의 validation을 check한다.
// 9. command privilege와 client의 등급을 비교하여 권한이 있는지 확인한다.
// 10. Application으로 보낼 명령어 처리 요구 메시지를 구성한다.
// 11. mmcdJobTbl에서 jobNo를 할당하고 정보를 저장한다.
// 12. ixpc를 통해 application으로 전송한다.
// 13. client로 명령이 정상 입력되었음을 알리는 accept 메시지를 보낸다.
//		- history 조회를 위한 몇가지 log가 함께 남는다.
//-----------------------------------------------------------------------------
int mmcd_exeMmcReqMsg (
    SockLibMsgType	*rxSockMsg,	// client로부터 수신한 데이터
    int		cliFd				// client와 접속된 socket fd
    )
{
    int				j,txLen,len, ret, isHelp;
    char			inputString[256];
//    char			resBuf[256];	
//    char			ConnIP[IPLEN];
//    unsigned int		connIP;
    MMLClientReqMsgType		*rxCliReqMsg;
    MMLInputCmdInfo		inputCmdInfo;
    GeneralQMsgType		txGenQMsg;
    IxpcQMsgType		*txIxpcMsg;
    MMLReqMsgType		*txReqMsg;
    
    

#if 0  // yhshin	
    typedef struct 
    {        
        int     cliReqId; // client에서 할당한 key값 -> client에서 할당해서 보내고, MMCD는
        int     confirm;  /* (-1)=X, (0)=NO, (1)=YES */
        int     batchFlag;
        int     clientType; // GUI(1), RMI(0), OMDMMC(2) 
    } MMLClientReqMsgHeadType;
#endif


    rxCliReqMsg = (MMLClientReqMsgType*)rxSockMsg->body;
    
    rxCliReqMsg->head.cliReqId = ntohl(rxCliReqMsg->head.cliReqId);
    rxCliReqMsg->head.confirm = ntohl(rxCliReqMsg->head.confirm);
    rxCliReqMsg->head.batchFlag = ntohl(rxCliReqMsg->head.batchFlag);
    rxCliReqMsg->head.clientType = ntohl(rxCliReqMsg->head.clientType);

#if 0 /*debug */
    fprintf(stderr, "\n\n rxCliReqMsg->head.cliReqId [%x]", rxCliReqMsg->head.cliReqId);
    fprintf(stderr, "\n rxCliReqMsg->head.confirm [%d]", rxCliReqMsg->head.confirm);
    fprintf(stderr, "\n rxCliReqMsg->batchFlag [%d]", rxCliReqMsg->head.batchFlag);
    fprintf(stderr, "\n rxCliReqMsg->clientType [%d]", rxCliReqMsg->head.clientType);
    fprintf(stderr, "\n rxCliReqMsg->body [%s]\n\n", rxCliReqMsg->body);
#endif

    len = rxSockMsg->head.bodyLen - sizeof(rxCliReqMsg->head);
    /*strncpy (inputString, rxCliReqMsg->body, len);
      inputString[len] = 0;*/
    
    strncpy (inputString, rxCliReqMsg->body, strlen(rxCliReqMsg->body));
    inputString[strlen(rxCliReqMsg->body)] = 0;
    
    memset ((void*)&inputCmdInfo, 0, sizeof(MMLInputCmdInfo));
    memset ((void*)&txGenQMsg, 0, sizeof(GeneralQMsgType));
    
    /* GUI, RMI */
    if ((rxCliReqMsg->head.clientType == 0) || (rxCliReqMsg->head.clientType == 1)) 
    {
        // 명령을 보낸 client와 연결된 socket fd와 원본 input string을 저장한다.
        // client에서 할당해서 보낸 key값이 request_id를 함께 저장한다.
        //
        inputCmdInfo.cliSockFd = cliFd;
        inputCmdInfo.cliReqId  = (rxCliReqMsg->head.cliReqId);
        inputCmdInfo.batchFlag  = (rxCliReqMsg->head.batchFlag);
//		inputCmdInfo.cliReqId  = ntohl(rxCliReqMsg->head.cliReqId);
//		inputCmdInfo.batchFlag  = ntohl(rxCliReqMsg->head.batchFlag);
        inputCmdInfo.confirm = rxCliReqMsg->head.confirm;
        inputCmdInfo.clientType = rxCliReqMsg->head.clientType;
        
        strcpy (inputCmdInfo.inputString, inputString);
                
		// client로부터 수신한 input string을 token으로 잘라내어 inputCmdInfo에
		//	cmdName과 parameter들을 저장한다.
		//
        if (mmcd_tokeniseInputString (&inputCmdInfo) < 0) 
        {
            return -1;
        }

#if 0
        printf("\n\n==========jean mmcd_tokeniseInputString \n");
        for(i=0; i<inputCmdInfo.paraCnt; i++ )
        {
            printf("%d=[%s][%s]\n", i, inputCmdInfo.paraInfo[i].paraName, 
                   inputCmdInfo.paraInfo[i].paraVal );
        }
        printf("==========jean mmcd_tokeniseInputString end \n\n");
#endif

//2009.07.16 by sjs
//        memset( ConnIP, 0, sizeof( ConnIP ) );
//        strcpy( ConnIP, GetClientIP( inputCmdInfo.cliSockFd ) );
//        connIP = GetClientIPhl( inputCmdInfo.cliSockFd );
        
        // 메시지를 보낸 client를 찾는다.
        // - client socket fd와 같은 놈을 mmcdCliTbl에서 찾아 그 index를 return한다.
        if ((inputCmdInfo.cliIndex = mmcd_getCliIndex (inputCmdInfo.cliSockFd)) < 0) 
        {
            if (!strcasecmp (inputCmdInfo.cmdName, "LOG-IN")) 
            {
                // client와 socket 접속 후 가장 먼저 login절차를 거쳐야 client table에
                //	정보가 저장되므로 ...
                mmcd_builtin_log_in (&inputCmdInfo);
                return 1;
            }
            if (trcFlag || trcLogFlag) 
            {
                sprintf(trcBuf,"[mmcd_exeMmcReqMsg] input=(%s)\n", inputCmdInfo.inputString);
                trclib_writeLogErr (FL,trcBuf);
            }
            inputCmdInfo.cmdIndex = -1;
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        
        if (trcFlag || trcLogFlag) 
        {
            sprintf(trcBuf,"[mmcd_exeMmcReqMsg] input=(%s)\n", inputCmdInfo.inputString);
            trclib_writeLogErr (FL,trcBuf);
        }
	
        // client의 userName을 저장한다.
        strcpy (inputCmdInfo.userName, mmcdCliTbl[inputCmdInfo.cliIndex].userName);
        inputCmdInfo.nmsibFlag = mmcdCliTbl[inputCmdInfo.cliIndex].nmsibFlag;
        //printf("userName=(%s)cliIndex=%d\n  ", inputCmdInfo.userName, inputCmdInfo.cliIndex);
	
        if (trcFlag || trcLogFlag) 
        {
            sprintf(trcBuf,"[mmcd_exeMmcReqMsg] input=(%s)(%s)\n",
                    inputCmdInfo.inputString, inputCmdInfo.userName);
            trclib_writeLog (FL,trcBuf);
        }
        
        // '?'를 이용한 command_help를 요청한 경우이면 help기능을 수행한다.
        //
        if (strstr(inputCmdInfo.inputString, "?") != NULL) 
        {
            isHelp = 1;
            /* 파라미터 에 '?'이 있으면 가입자 이름일 수 있으므로 정상 처리함 (2005.9.1) */
            for (j=0; j < inputCmdInfo.paraCnt; j++) 
            {
                if (strstr(inputCmdInfo.paraInfo[j].paraVal, "?") != NULL) 
                {
                    /* 파라미터가 '?'만 있는 경우는 제외함 */
                    if (strlen(inputCmdInfo.paraInfo[j].paraVal) > 1)
                        isHelp = 0;
                    break;
                }
            }
            
            if (isHelp == 1) 
            {
                mmcd_builtin_cmd_help(&inputCmdInfo);
                return 1;
            }
        }
        
        if (!strcasecmp (inputCmdInfo.cmdName, "alive")) // 2009.07.15 by sjs
        {
            mmcd_builtin_heart_beat( &inputCmdInfo );
            return 1;
        }
        // 수신한 cmdName과 일치하는 명령어를 mmlCmdTbl에서 찾는다.
        // - mmlCmdTbl의 index가 return된다.
        //
        if ((inputCmdInfo.cmdIndex = mmcd_getCmdIndex (inputCmdInfo.cmdName)) < 0) 
        {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        
        // client는 파라미터들의 값만 보냈으므로, mmlCmdTbl을 검색하여 각 파라미터들의
        //	이름을 채운다.
        // - 파라미터 갯수 초과, 파라미터 이름 확인, 파라미터 중복확인 등 error check
        //
        
        if (mmcd_fillInputParaName (&inputCmdInfo) < 0) 
        {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
#if 0
        printf("\n\n==========jean mmcd_fillInputParaName\n");
        for(i=0; i<inputCmdInfo.paraCnt; i++ )
        {
            printf("%d=[%s][%s]\n", i, inputCmdInfo.paraInfo[i].paraName, 
                   inputCmdInfo.paraInfo[i].paraVal );
        }
        printf("==========jean mmcd_fillInputParaName end \n\n");
#endif
        
        mmcd_arrangeInputPara (&inputCmdInfo);
#if 0
        printf("\n\n==========jean mmcd_arrangeInputPara\n");
        for(i=0; i<inputCmdInfo.paraCnt; i++ )
        {
            printf("%d=[%s][%s]\n", i, inputCmdInfo.paraInfo[i].paraName, 
                   inputCmdInfo.paraInfo[i].paraVal );
        }
        printf("==========jean mmcd_arrangeInputPara end \n\n");
#endif
        
        
        
        // 필수 파라미터 및 (OPTIONAL 선택)  파라미터 입력여부 확인
        if (mmcd_checkMandatoryPara (&inputCmdInfo) < 0) 
        {
//			mmcd_sendInputError2Client (&inputCmdInfo);
            strcat(inputCmdInfo.inputString, "?");
            strcat(inputCmdInfo.cmdName, "?");
            mmcd_builtin_cmd_help(&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_checkMandatoryPara... OK\n");
        
        
        // 각 파라미터 값들의 validation을 check한다.
        //
        if (mmcd_checkValidParaVal (&inputCmdInfo) < 0) 
        {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_checkValidParaVal... OK\n");
        
        // command privilege와 client의 등급을 비교하여 권한이 있는지 확인한다.
        //
        if (mmcd_checkUserPrivilege (&inputCmdInfo) < 0) {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_checkUserPrivilege... OK\n");
        
        
        // mmcd가 직접 처리해야 하는 built-in 명령여부를 확인한다.
        // - built-in 명령인 경우 직접 처리 후 return된다.
        //
        if (mmcd_isBuiltInCmd (&inputCmdInfo)) 
        {
            return 1;
        }
        
        if ((ret = mmcd_checkUserConfirm (&inputCmdInfo)) < 0) 
        {
            mmcd_sendInputConfirm2Client (&inputCmdInfo);
            return -1;
        }
        if (ret == 0) 
        {
            mmcd_sendInputCancel2Client (&inputCmdInfo);
            return -1;
        }
        
        // Application으로 보낼 명령어 처리 요구 메시지를 구성한다.
        //
        mmcd_makeReqMsg (&inputCmdInfo, &txGenQMsg);
        //printf("mmcd_makeReqMsg\n");
        
        // mmcdJobTbl에서 jobNo를 할당하고 정보를 저장한다.
        //
        if (mmcd_saveReqData2JobTbl (&inputCmdInfo, &txGenQMsg) < 0) 
        {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_saveReqData2JobTbl\n");
        
        
        // ixpc를 통해 application으로 전송한다.
        //
        txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
        txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;
        
        if (msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0)
        {
            sprintf(inputErrBuf,"INTERNAL_ERROR(msgsnd fail)");
            mmcd_sendInputError2Client (&inputCmdInfo);
            
            // 할당했던 job table을 해제한다.
            txReqMsg = (MMLReqMsgType*)txIxpcMsg->body;
            memset ((void*)&mmcdJobTbl[txReqMsg->head.mmcdJobNo], 0,
                    sizeof(MmcdJobTblContext));
            
            sprintf(trcBuf,"[mmcd_exeMmcReqMsg] msgsnd fail to %s-%s; err=%d(%s)\n",
                    txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName,
                    errno, strerror(errno));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }
#if 0
        fprintf(stderr,"[mmcd_exeMmcReqMsg] %s to (%s-%s); \n",
                inputCmdInfo.cmdName,
                txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName);
#endif
    } 
    else if (rxCliReqMsg->head.clientType == 2) 
    {
        inputCmdInfo.cliSockFd = cliFd;
        inputCmdInfo.cliReqId  = ntohl(rxCliReqMsg->head.cliReqId);
        inputCmdInfo.batchFlag  = ntohl(rxCliReqMsg->head.batchFlag);
        inputCmdInfo.confirm = rxCliReqMsg->head.confirm;
        inputCmdInfo.clientType = rxCliReqMsg->head.clientType;
        
        strcpy (inputCmdInfo.inputString, inputString);
        
        if (mmcd_tokeniseInputString (&inputCmdInfo) < 0) {
            return -1;
        }
        
        if ((inputCmdInfo.cliIndex = mmcd_getCliIndex (inputCmdInfo.cliSockFd)) < 0) {
            if (!strcasecmp (inputCmdInfo.cmdName, "LOG-IN")) {
                return 1;
            }
            if (trcFlag || trcLogFlag) {
                sprintf(trcBuf,"[mmcd_exeMmcReqMsg] input=(%s)\n", inputCmdInfo.inputString);
                trclib_writeLogErr (FL,trcBuf);
            }
            inputCmdInfo.cmdIndex = -1;
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        
        strcpy (inputCmdInfo.userName, mmcdCliTbl[inputCmdInfo.cliIndex].userName);
        inputCmdInfo.nmsibFlag = mmcdCliTbl[inputCmdInfo.cliIndex].nmsibFlag;
	
        if (trcFlag || trcLogFlag) {
            sprintf(trcBuf,"[mmcd_exeMmcReqMsg] input=(%s)(%s)\n",
                    inputCmdInfo.inputString, inputCmdInfo.userName);
            trclib_writeLog (FL,trcBuf);
        }
        
        if (strstr(inputCmdInfo.inputString, "?") != NULL) {
            mmcd_builtin_cmd_help(&inputCmdInfo);
            return 1;
        }
        
        if ((inputCmdInfo.cmdIndex = mmcd_getCmdIndex (inputCmdInfo.cmdName)) < 0) {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
//		printf("cmdIndex=%d, inputString=%s\n  ", inputCmdInfo.cmdIndex, inputCmdInfo.inputString);
        
        // client는 파라미터들의 값만 보냈으므로, mmlCmdTbl을 검색하여 각 파라미터들의
        //	이름을 채운다.
        // - 파라미터 갯수 초과, 파라미터 이름 확인, 파라미터 중복확인 등 error check
        //
        
        if (mmcd_fillInputParaName (&inputCmdInfo) < 0) {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        
        mmcd_arrangeInputPara (&inputCmdInfo);
        
        // 필수 파라미터 및 (OPTIONAL 선택)  파라미터 입력여부 확인
        if (mmcd_checkMandatoryPara (&inputCmdInfo) < 0) {
//			mmcd_sendInputError2Client (&inputCmdInfo);
            strcat(inputCmdInfo.inputString, "?");
            strcat(inputCmdInfo.cmdName, "?");
            mmcd_builtin_cmd_help(&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_checkMandatoryPara... OK\n");
        
        
        // 각 파라미터 값들의 validation을 check한다.
        //
        if (mmcd_checkValidParaVal (&inputCmdInfo) < 0) {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_checkValidParaVal... OK\n");
        
        // command privilege와 client의 등급을 비교하여 권한이 있는지 확인한다.
        //
        if (mmcd_checkUserPrivilege (&inputCmdInfo) < 0) {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_checkUserPrivilege... OK\n");
        
        
        // mmcd가 직접 처리해야 하는 built-in 명령여부를 확인한다.
        // - built-in 명령인 경우 직접 처리 후 return된다.
        //
        if (mmcd_isBuiltInCmd (&inputCmdInfo)) {
            return 1;
        }
        
        if ((ret = mmcd_checkUserConfirm (&inputCmdInfo)) < 0) {
            mmcd_sendInputConfirm2Client (&inputCmdInfo);
            return -1;
        }
        if (ret == 0) {
            mmcd_sendInputCancel2Client (&inputCmdInfo);
            return -1;
        }
        
        // Application으로 보낼 명령어 처리 요구 메시지를 구성한다.
        //
        mmcd_makeReqMsg (&inputCmdInfo, &txGenQMsg);
        //printf("mmcd_makeReqMsg\n");
        
        // mmcdJobTbl에서 jobNo를 할당하고 정보를 저장한다.
		//
        if (mmcd_saveReqData2JobTbl (&inputCmdInfo, &txGenQMsg) < 0) {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_saveReqData2JobTbl\n");
        
        
        // ixpc를 통해 application으로 전송한다.
        //
        txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
        txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;
        
        if (msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0)
        {
            sprintf(inputErrBuf,"INTERNAL_ERROR(msgsnd fail)");
            mmcd_sendInputError2Client (&inputCmdInfo);
            
            // 할당했던 job table을 해제한다.
            txReqMsg = (MMLReqMsgType*)txIxpcMsg->body;
            memset ((void*)&mmcdJobTbl[txReqMsg->head.mmcdJobNo], 0,
                    sizeof(MmcdJobTblContext));
            
            sprintf(trcBuf,"[mmcd_exeMmcReqMsg] msgsnd fail to %s-%s; err=%d(%s)\n",
                    txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName,
                    errno, strerror(errno));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }
#if 0
        fprintf(stderr,"[2.mmcd_exeMmcReqMsg] %s to (%s-%s); \n",
                inputCmdInfo.cmdName,
					txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName);
#endif
    }
    
    // client로 명령이 정상 입력되었음을 알리는 accept 메시지를 보낸다.
    // - history 조회를 위한 몇가지 log가 함께 남는다.
    //
    mmcd_sendInputAccepted2Client (&inputCmdInfo);
    
    return 1;
    
} //----- End of mmcd_exeMmcReqMsg -----//



//------------------------------------------------------------------------------
// client로부터 수신한 command 라인을 token으로 잘라내 inputCmdInfo에 넣는다.
// - command_name과 파라미터 부분로 나누고, 각 파라미터들을 token으로 잘라내어
//	저장한다.
// - 파라미터의 갯수도 함께 저장한다.
// - 입력하지 않는 파라미터에는 저장되지 않으므로 NULL이 들어간다.
// - 나중에 사용될 수 있으므로 수신한 command 라인을 그대로 복사해둔다.
//------------------------------------------------------------------------------
int mmcd_tokeniseInputString (
		MMLInputCmdInfo *inputCmdInfo // 수신한 원본 string이 들어있고, 여기에 token들이 저장된다.
		)
{
	char	*ptr,*next,*token,remain[256];
	int		paraCnt, k;

	// 시작부분의 white-space를 지운다.
	for (ptr=inputCmdInfo->inputString; isspace(*ptr); ptr++);
	strcpy (inputCmdInfo->inputString, ptr); // 시작부분의 white-space를 지운 원본을 다시 저장한다.

	// command name을 잘라낸다.
	// - name과 parameter는 하나이상의 space로 구분된다.
	strcpy (remain, inputCmdInfo->inputString);
	ptr = remain;

	token = (char*)strtok_r(ptr," \t",&next);
	if (token == NULL) {
		sprintf(trcBuf,"[mmcd_tokeniseInputString] strtok_r fail");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	if (next == NULL) {
		// parameter가 없는 경우
		strcpy (inputCmdInfo->cmdName, remain);
		return 1;
	}
	strcpy (inputCmdInfo->cmdName, token);

	//
	// 파라미터를 하나씩 잘라낸다.
	// - 파라미터들은 ","로 구분된다.
	//
	paraCnt = 0;
	while(1)
	{
		// 파라미터들 사이의 white-space를 지운다.
		for (ptr = next; isspace(*ptr); ptr++) ;

		// optional parameter가 입력되지 않은 경우. ex) "aaa-bbb xxx,,,2"
		if (*ptr == ',') {
			paraCnt++;
			next = ++ptr;
			continue;
		}

		if ((token = (char*)strtok_r(ptr,",",&next)) == NULL)
			break;
		strcpy (inputCmdInfo->paraInfo[paraCnt].paraVal, token);

		// 파라미터 뒤쪽 white-space를 지운다.

		ptr = inputCmdInfo->paraInfo[paraCnt].paraVal;
		ptr = ptr + strlen(inputCmdInfo->paraInfo[paraCnt].paraVal) - 1;
		for ( ; isspace(*ptr); ptr--)
			*ptr = 0;
		
		if (inputCmdInfo->paraInfo[paraCnt].paraVal[0] == '"') {
#if 0 /* jhnoh : 030813 */
            for (j=1; (((inputCmdInfo->paraInfo[paraCnt].paraVal[j] != '"') && (inputCmdInfo->paraInfo[paraCnt].paraVal[j] != 0))  && (j < 256)); j++) 
#else 
            for (k=1; k<256; k++)
#endif
				inputCmdInfo->paraInfo[paraCnt].paraVal[k-1] = inputCmdInfo->paraInfo[paraCnt].paraVal[k];
			inputCmdInfo->paraInfo[paraCnt].paraVal[k-1] = 0;
		}
		else
			checkEqual(inputCmdInfo->paraInfo[paraCnt].paraVal);

		paraCnt++;

		if (next == NULL) {
			break; // 마지막 파라미터
		}
	}
	inputCmdInfo->paraCnt = paraCnt;

	return 1;

} //----- End of mmcd_tokeniseInputString -----//



//------------------------------------------------------------------------------
// mmcd_tokenise()에서 파라미터 부분을 delimeter에 의해 token들로 잘라낸 데이터을
//	mmlCmdTbl을 검색하여 각 파라미터들의 이름을 채운다.
// - 파라미터 갯수 초과, 파라미터 이름 확인, 파라미터 중복확인 등 error check
// - 일부 파라미터만 이름을 지정하여 입력한 경우 error 처리한다.
//------------------------------------------------------------------------------
int mmcd_fillInputParaName (
		MMLInputCmdInfo *inputCmdInfo // token으로 잘라내어진 parameter들이 들어있고, 여기에 이름이 채워진다.
		)
{
	char	*name, *value, tmp[COMM_MAX_VALUE_LEN]={0,};
	int		i, cmd, paraIndex;
	int		paraPattern=0;

	cmd = inputCmdInfo->cmdIndex;

	// 파라미터를 너무 많이 입력한 경우
	//
	if (inputCmdInfo->paraCnt > mmlCmdTbl[cmd].paraCnt) {
		sprintf(inputErrBuf,"TOO_MANY_PARAMETERS");
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[mmcd_fillInputParaName] too many parameters; cmdName=%s, paraCnt=%d; inputCnt=%d\n",
					mmlCmdTbl[cmd].cmdName, mmlCmdTbl[cmd].paraCnt, inputCmdInfo->paraCnt);
			trclib_writeLog (FL,trcBuf);
		}
		return -1;
	}

	for (paraIndex=0; paraIndex < inputCmdInfo->paraCnt; paraIndex++)
	{
		// 입력된 파라미터만 찾는다.
		if (strcasecmp(inputCmdInfo->paraInfo[paraIndex].paraVal, ""))
		{
			// 파라미터를 "aaa=123" 형식으로 파라미터 이름을 지정하여 입력한 경우
			//	해당 명령어에 등록된 이름인지 확인한다.
			if ((strstr(inputCmdInfo->paraInfo[paraIndex].paraVal, "=") != NULL) && (inputCmdInfo->paraInfo[paraIndex].paraVal[0] != '('))
			{
				// 일부 파라미터만 이름을 지정하지 못하도록 제한한다.
				if (paraPattern != 0 && paraPattern != 1) {
					sprintf(inputErrBuf,"PARAMETER_PATTERN_MISMATCH");
					if (trcFlag || trcLogFlag) {
						sprintf(trcBuf,"[mmcd_fillInputParaName] parameter pattern mismatch; paraNo=%d\n", paraIndex);
						trclib_writeLog (FL,trcBuf);
					}
					return -1;
				} else {
					paraPattern = 1;
				}

				// paraName과 value을 분리한다.
				strcpy (tmp, inputCmdInfo->paraInfo[paraIndex].paraVal);
				name = (char*)strtok_r(tmp, "=", &value);

				// "=" 앞뒤 즉, name의 뒤쪽과 value의 앞쪽 white-space를 없앤다.
				for (i=0; i<strlen(name) && !isspace(name[i]); i++); name[i]=0;
				for (i=0; isspace(*value); value++);

				// 등록된 파라미터인지 확인한다.
				for (i=0; i < mmlCmdTbl[cmd].paraCnt; i++) {
					if (!strcasecmp(name, mmlCmdTbl[cmd].paraInfo[i].paraName)) {
						break;
					}
				}
				if (i >= mmlCmdTbl[cmd].paraCnt) {
					sprintf(inputErrBuf,"NOT_REGISTERED_PARAMETER");
					if (trcFlag || trcLogFlag) {
						sprintf(trcBuf,"[mmcd_fillInputParaName] not registered parameter; cmdName=%s, paraName=%s\n",
								mmlCmdTbl[cmd].cmdName, name);
						trclib_writeLog (FL,trcBuf);
					}
					return -1;
				}
			} else { // 파라미터 이름없이 value만 입력한 경우
				// 일부 파라미터만 이름을 지정하지 못하도록 제한한다.
				if (paraPattern != 0 && paraPattern != 2) {
					sprintf(inputErrBuf,"PARAMETER_PATTERN_MISMATCH");
					if (trcFlag || trcLogFlag) {
						sprintf(trcBuf,"[mmcd_fillInputParaName] parameter pattern mismatch; paraNo=%d\n", paraIndex);
						trclib_writeLog (FL,trcBuf);
					}
					return -1;
				} else {
					paraPattern = 2;
				}

				name  = mmlCmdTbl[cmd].paraInfo[paraIndex].paraName;
				value = inputCmdInfo->paraInfo[paraIndex].paraVal;
			}

			// 파라미터를 중복해서 지정했는지 확인한다.
			//
			for (i=0; i < paraIndex; i++) {
				if (!strcasecmp(name, inputCmdInfo->paraInfo[i].paraName)) {
					sprintf(inputErrBuf,"DUPLICATED_PARAMETER(para=%s)", name);
					if (trcFlag || trcLogFlag) {
						sprintf(trcBuf,"[mmcd_fillInputParaName] duplicated parameter; cmdName=%s, paraName=%s\n",
								mmlCmdTbl[cmd].cmdName, name);
						trclib_writeLog (FL,trcBuf);
					}
					return -1;
				}
			}

			// 확인이 끝났으면 파라미터 이름을 채운다.
			strcpy (inputCmdInfo->paraInfo[paraIndex].paraName, name);
			strcpy (inputCmdInfo->paraInfo[paraIndex].paraVal, value);
//fprintf(stderr, "name = %s, val = %s\n", inputCmdInfo->paraInfo[paraIndex].paraName, inputCmdInfo->paraInfo[paraIndex].paraVal);
		}
	}

	// client로부터 수신한 입력 string 원본이 저장된 inputCmdInfo->inputString을
	//	"aaa=xxx" 형태로 변환한다.
	// -> client로 accept 메시지 출력 및 명령어 history에 기록할 형태로 변환
	sprintf(inputCmdInfo->inputString, "%s", inputCmdInfo->cmdName);
	for (i=0; i < strlen(inputCmdInfo->inputString); i++)
		inputCmdInfo->inputString[i] = toupper(inputCmdInfo->inputString[i]);
	
	// 어째서 pass 바꿀때 마져 ***표시를 한겁니까?
#if 0	
	for (i=0; i < inputCmdInfo->paraCnt; i++)
	{
		if (strcasecmp(inputCmdInfo->paraInfo[i].paraVal, "")) {
			// PASSWD 파라미터는 ***로 출력하기 위해 inputString의 내용을 바꾼다.
			if (!strcasecmp(inputCmdInfo->paraInfo[i].paraName, "PASSWD") ||
			    !strcasecmp(inputCmdInfo->paraInfo[i].paraName, "OLD_PSWD") ||
			    !strcasecmp(inputCmdInfo->paraInfo[i].paraName, "NEW_PSWD") ){
				// strcpy (tmp2, "");
				// for (j=0; j<strlen(inputCmdInfo->paraInfo[i].paraVal); j++)
				sprintf (tmp2, "******");
				sprintf (tmp," %s=%s,",inputCmdInfo->paraInfo[i].paraName, tmp2);
			} else {
				sprintf(tmp," %s=%s,",inputCmdInfo->paraInfo[i].paraName,
						inputCmdInfo->paraInfo[i].paraVal);
			}
			strcat (inputCmdInfo->inputString, tmp);
		}
	}
#endif
	for (i=0; i < inputCmdInfo->paraCnt; i++)
	{
		if (strcasecmp(inputCmdInfo->paraInfo[i].paraVal, "")) {	
			sprintf(tmp," %s=%s,",inputCmdInfo->paraInfo[i].paraName,
					inputCmdInfo->paraInfo[i].paraVal);
			strcat (inputCmdInfo->inputString, tmp);
		}

	}	

	// 맨끝에 있는 ","를 지운다.
	if (strcmp (tmp, "")) {
		inputCmdInfo->inputString[strlen(inputCmdInfo->inputString)-1] = 0;
	}

	return 1;

} //----- End of mmcd_fillInputParaName -----//



//------------------------------------------------------------------------------
// mmlCmdTbl을 검색하여 입력된 파라미터들의 순서를 mmlCmdTbl에 등록된 순서대로
//	정렬한다.
//------------------------------------------------------------------------------
int mmcd_arrangeInputPara (
		MMLInputCmdInfo *inputCmdInfo // 입력된 파라미터들이 들어있고, 정렬후 여기에 다시 기록된다.
		)
{
	MMLInputCmdInfo	tmp;
	int		i,k,cmd,cnt=0;

	memset ((void*)&tmp, 0, sizeof(MMLInputCmdInfo));
	cmd = inputCmdInfo->cmdIndex;

	// mmlCmdTbl에 등록된 파라미터들을 하나씩 꺼내 inputCmdInfo에 있으면 tmp에
	//	저장하는 방법으로 정렬한다.
	for (i=0; i < mmlCmdTbl[cmd].paraCnt; i++) {
		for (k=0; k < inputCmdInfo->paraCnt; k++) {
			if (!strcasecmp(mmlCmdTbl[cmd].paraInfo[i].paraName,
							inputCmdInfo->paraInfo[k].paraName)) {
				strcpy (tmp.paraInfo[cnt].paraName, inputCmdInfo->paraInfo[k].paraName);
				strcpy (tmp.paraInfo[cnt].paraVal, inputCmdInfo->paraInfo[k].paraVal);
			}
		}
		cnt++;
	}

	// 정렬된 데이터를 inputCmdInfo에 덮어쓴다.
	memcpy ((void*)inputCmdInfo->paraInfo, tmp.paraInfo, sizeof(tmp.paraInfo));

	return 1;

} //----- End of mmcd_arrangeInputPara -----//



//------------------------------------------------------------------------------
// 필수 파라미터 입력여부를 확인한다.
//------------------------------------------------------------------------------
int mmcd_checkMandatoryPara (
		MMLInputCmdInfo *inputCmdInfo // 입력된 파라미터들이 정렬되어 들어있다.
		)
{
	int		i,cmd, option2_flag=0, option2_present=0;

	cmd = inputCmdInfo->cmdIndex;

	for (i=0; i < mmlCmdTbl[cmd].paraCnt; i++) {
		if ((mmlCmdTbl[cmd].paraInfo[i].mandFlag == 1) &&
			!strcasecmp(inputCmdInfo->paraInfo[i].paraName, "")) {
			sprintf(inputErrBuf,"OMITTED_MANDATORY_PARAMETER(para=%s)",
					mmlCmdTbl[cmd].paraInfo[i].paraName);
			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[mmcd_checkMandatoryPara] omitted mandatory parameter; cmdName=%s, paraName=%s\n",
						mmlCmdTbl[cmd].cmdName, mmlCmdTbl[cmd].paraInfo[i].paraName);
				trclib_writeLog (FL,trcBuf);
			}
			return -1;
		}
		if (mmlCmdTbl[cmd].paraInfo[i].mandFlag == 2) {
			option2_flag++;	
			if (strcasecmp(inputCmdInfo->paraInfo[i].paraName, "")) {
				option2_present++; 
			} 
		}
			
	}
	if ((option2_flag != 0) && (option2_present < 1)) {
		sprintf(inputErrBuf,"OMITTED_OPTIONARY_PARAMETER");
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[mmcd_checkMandatoryPara] omitted optionary parameter");
			trclib_writeLog (FL,trcBuf);
		}
		return -1;
	} 
	else if ((option2_flag != 0) && (option2_present > 1)) {
		sprintf(inputErrBuf,"TOO_MANY_OPTIONARY_PARAMETER");
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[mmcd_checkMandatoryPara] too many optionary parameter");
			trclib_writeLog (FL,trcBuf);
		}
		return -1;
	} 

	return 1;

} /** End of mmcd_checkMandatoryPara **/



//------------------------------------------------------------------------------
// 각 파라미터 값들의 validation을 check한다.
//------------------------------------------------------------------------------
int mmcd_checkValidParaVal (
		MMLInputCmdInfo *inputCmdInfo // 입력된 파라미터들이 정렬되어 들어있다.
		)
{
	int		cmd,paraIndex;
	char	reason[2048]; //050401.r11.cjs 1024->2048

	cmd = inputCmdInfo->cmdIndex;

  //limsh
  //for (paraIndex=0; paraIndex < inputCmdInfo->paraCnt; paraIndex++) {

	for (paraIndex=0; paraIndex < mmlCmdTbl[cmd].paraCnt; paraIndex++) {
		if (strcasecmp(inputCmdInfo->paraInfo[paraIndex].paraName, "")) {
			// 파라미터 type에 맞게 입력되었는지 확인한다.
			// - 입력가능한 문자인지, range에 들어오는지 확인
//printf("MMC_result: %s, paraCnt : %d\n", inputCmdInfo->paraInfo[paraIndex].paraVal, mmlCmdTbl[cmd].paraCnt);
			if (mmcd_checkValidValueOnType (cmd, paraIndex,
											inputCmdInfo->paraInfo[paraIndex].paraVal, reason) < 0) {
				sprintf(inputErrBuf,"INVALID_PARAMETER(%s)",reason );
				if (trcFlag || trcLogFlag) {
					sprintf(trcBuf,"[mmcd_checkValidParaVal] invalid parameter; cmdName=%s, paraName=%s\n",
							inputCmdInfo->cmdName, inputCmdInfo->paraInfo[paraIndex].paraVal);
					trclib_writeLog (FL,trcBuf);
				}
				return -1;
			}
		}
	}

	return 1;

} //----- End of mmcd_checkValidParaVal -----//



//------------------------------------------------------------------------------
// 입력된 파라미터의 값이 등록된 type에 맞는지, range에 들어오는지 확인한다.
//------------------------------------------------------------------------------
int mmcd_checkValidValueOnType (
		int cmd,             // mmlCmdTbl에서 해당 command의 index
		int paraIndex,       // mmlCmdTbl에서 해당 parameter의 index
		char *inputValString,// 입력된 파라미터 값(string)
		char *reason 		 //  error reason
		)
{
	int		i,value,len, j;

	switch (mmlCmdTbl[cmd].paraInfo[paraIndex].paraType)
	{
		case MML_PTYPE_DECIMAL:
			len = strlen(inputValString);
			for (i=0; i<len; i++) {
				if (!isdigit(inputValString[i])) {
					//printf("[mmcd_checkValidValueOnType] not digit character; %s\n", inputValString);
					sprintf (reason, "(PARA=%s TYPE=DECIMAL RANGE=%d~%d)", 
						mmlCmdTbl[cmd].paraInfo[paraIndex].paraName,
						mmlCmdTbl[cmd].paraInfo[paraIndex].minVal,
						mmlCmdTbl[cmd].paraInfo[paraIndex].maxVal );
					return -1;
				}
			}
			value = strtol(inputValString, 0, 0);
			if (value < mmlCmdTbl[cmd].paraInfo[paraIndex].minVal ||
				value > mmlCmdTbl[cmd].paraInfo[paraIndex].maxVal) {
				//printf("[mmcd_checkValidValueOnType] range over; value=(%s)\n", inputValString);
				sprintf (reason, "(PARA=%s TYPE=DECIMAL RANGE=%d~%d)", 
					mmlCmdTbl[cmd].paraInfo[paraIndex].paraName,
					mmlCmdTbl[cmd].paraInfo[paraIndex].minVal,
					mmlCmdTbl[cmd].paraInfo[paraIndex].maxVal );
				return -1;
			}
			break;

		case MML_PTYPE_HEXA:
			len = strlen(inputValString);
			for (i=0; i<len; i++) {
				if (!isxdigit(inputValString[i])) {
					//printf("[mmcd_checkValidValueOnType] not xdigit character; %s\n", inputValString);
					sprintf (reason, "(PARA=%s TYPE=HEXA RANGE=0x%x~0x%x)", 
						mmlCmdTbl[cmd].paraInfo[paraIndex].paraName,
						mmlCmdTbl[cmd].paraInfo[paraIndex].minVal,
						mmlCmdTbl[cmd].paraInfo[paraIndex].maxVal );
					return -1;
				}
			}
			value = strlen(inputValString);
			if (value < mmlCmdTbl[cmd].paraInfo[paraIndex].minVal ||
				value > mmlCmdTbl[cmd].paraInfo[paraIndex].maxVal) {
				//printf("[mmcd_checkValidValueOnType] range over; %s\n", inputValString);
				sprintf (reason, "(PARA=%s TYPE=HEXA HEXA_LENGTH=%d~%d)", 
					mmlCmdTbl[cmd].paraInfo[paraIndex].paraName,
					mmlCmdTbl[cmd].paraInfo[paraIndex].minVal,
					mmlCmdTbl[cmd].paraInfo[paraIndex].maxVal );
				return -1;
			}
			break;

		case MML_PTYPE_STRING: // string type인 경우 min,max는 자릿수를 의미한다.
			len = strlen(inputValString);
			if (len < mmlCmdTbl[cmd].paraInfo[paraIndex].minVal ||
				len > mmlCmdTbl[cmd].paraInfo[paraIndex].maxVal) {
				//printf("[mmcd_checkValidValueOnType] range over; %s\n", inputValString);
				sprintf (reason, "(PARA=%s TYPE=STRING LENGTH=%d~%d)", 
					mmlCmdTbl[cmd].paraInfo[paraIndex].paraName,
					mmlCmdTbl[cmd].paraInfo[paraIndex].minVal,
					mmlCmdTbl[cmd].paraInfo[paraIndex].maxVal );
				return -1;
			}
			break;
#if 1 /* jhnoh : 030623 */		
		case MML_PTYPE_FIXSTR: // fix string type인 경우 fixval 의 자릿수를 비교한다
			len = strlen(inputValString);
			for (i=0; i< 5; i++) {
				if (mmlCmdTbl[cmd].paraInfo[paraIndex].fixVal[i] == len)
					return 1; 
			}
			//printf("[mmcd_checkValidValueOnType] range over; %s\n", inputValString);
			sprintf (reason, "(PARA=%s TYPE=FIX_STRING LENGTH=", 
				mmlCmdTbl[cmd].paraInfo[paraIndex].paraName );
			for (i=0; i< 5; i++) {
				if ( mmlCmdTbl[cmd].paraInfo[paraIndex].fixVal[i] != 0 ){
					sprintf (&reason[strlen(reason)],"%d ", mmlCmdTbl[cmd].paraInfo[paraIndex].fixVal[i] );
				}
			}
			sprintf (&reason[strlen(reason)],")");
			return -1;
			break;
#endif
#if 1 /* jhnoh : 030815 */		
		case MML_PTYPE_FIXDEC: // fix string type인 경우 fixval 의 자릿수를 비교한다
			len = strlen(inputValString);
			for (i=0; i< 5; i++) {
				if (mmlCmdTbl[cmd].paraInfo[paraIndex].fixVal[i] == len) {
            		for (j=0; j<len; j++) {
                		if (!isdigit(inputValString[j])) {
                    		//printf("[mmcd_checkValidValueOnType] not xdigit character;
							sprintf (reason, "(PARA=%s TYPE=FIX_DECIMAL LENGTH=", 
								mmlCmdTbl[cmd].paraInfo[paraIndex].paraName );
							for (i=0; i< 5; i++) {
								if ( mmlCmdTbl[cmd].paraInfo[paraIndex].fixVal[i] != 0 ){
									sprintf (&reason[strlen(reason)],"%d ", mmlCmdTbl[cmd].paraInfo[paraIndex].fixVal[i] );
								}
							}
							sprintf (&reason[strlen(reason)],")");
                    		return -1;
               			}
            		}
					return 1; 
				}
			}
			//printf("[mmcd_checkValidValueOnType] range over; %s\n", inputValString);
			sprintf (reason, "(PARA=%s TYPE=FIX_DECIMAL LENGTH=", 
				mmlCmdTbl[cmd].paraInfo[paraIndex].paraName );
			for (i=0; i< 5; i++) {
				if ( mmlCmdTbl[cmd].paraInfo[paraIndex].fixVal[i] != 0 ){
					sprintf (&reason[strlen(reason)],"%d ", mmlCmdTbl[cmd].paraInfo[paraIndex].fixVal[i] );
				}
			}
			sprintf (&reason[strlen(reason)],")");
			return -1;
			break;

		case MML_PTYPE_DECSTR: // string type인 경우 min,max는 자릿수를 의미한다.
			len = strlen(inputValString);
			if (len < mmlCmdTbl[cmd].paraInfo[paraIndex].minVal ||
				len > mmlCmdTbl[cmd].paraInfo[paraIndex].maxVal) {
				//printf("[mmcd_checkValidValueOnType] range over; %s\n", inputValString);
				sprintf (reason, "(PARA=%s TYPE=DECIMAL_STRING RANGE=%d~%d)", 
					mmlCmdTbl[cmd].paraInfo[paraIndex].paraName,
					mmlCmdTbl[cmd].paraInfo[paraIndex].minVal,
					mmlCmdTbl[cmd].paraInfo[paraIndex].maxVal );
				return -1;
			}
            for (i=0; i<len; i++) {
                if (!isdigit(inputValString[i])) {
                    //printf("[mmcd_checkValidValueOnType] not xdigit character;
					sprintf (reason, "(PARA=%s TYPE=DECIMAL_STRING RANGE=%d~%d)", 
						mmlCmdTbl[cmd].paraInfo[paraIndex].paraName,
						mmlCmdTbl[cmd].paraInfo[paraIndex].minVal,
						mmlCmdTbl[cmd].paraInfo[paraIndex].maxVal );
                    return -1;
                }
            }
			break;
#endif
		case MML_PTYPE_ENUM:
			// enumList에 등록된 string인지 확인한다.
			for (i=0; i < MML_MAX_ENUM_ITEM; i++) {
#if 0 /*debug*/
{
	fprintf(stderr, "input validation %s, %s\n", inputValString, mmlCmdTbl[cmd].paraInfo[paraIndex].enumList[i].enumStr);
}
#endif
				if (!strcasecmp(inputValString, mmlCmdTbl[cmd].paraInfo[paraIndex].enumList[i].enumStr) ) {
					return 1;
				}

				if (!strcasecmp(inputValString, mmlCmdTbl[cmd].paraInfo[paraIndex].enumList[i].enumStr2) ) {
					strcpy(inputValString, mmlCmdTbl[cmd].paraInfo[paraIndex].enumList[i].enumStr);
					return 1;
				}
			}
			if (i >= MML_MAX_ENUM_ITEM) {
				//printf("[mmcd_checkValidValueOnType] not registered enum; %s\n", inputValString);
				sprintf (reason, "(PARA=%s TYPE=ENUM VALUE=", 
					mmlCmdTbl[cmd].paraInfo[paraIndex].paraName );
				for (i=0; i< MML_MAX_ENUM_ITEM; i++) {
					if (mmlCmdTbl[cmd].paraInfo[paraIndex].enumList[i].enumStr[0] != NULL) { 
						sprintf (&reason[strlen(reason)],"%s:%s ", 
						mmlCmdTbl[cmd].paraInfo[paraIndex].enumList[i].enumStr,
						mmlCmdTbl[cmd].paraInfo[paraIndex].enumList[i].enumStr2 );
					}
				}
				sprintf (&reason[strlen(reason)],")");
				return -1;
			}
			break;

	} //-- end of switch(paraType) --//

	return 1;

} //----- End of mmcd_checkValidValueOnType -----//



//------------------------------------------------------------------------------
// mmcdCliTbl과 mmlCmdTbl 검색하여 명령을 요청한 client가 명령어 사용 권한이 있는지
//	확인한다.
//------------------------------------------------------------------------------
int mmcd_checkUserPrivilege (
		MMLInputCmdInfo *inputCmdInfo	// 입력된 명령어 정보가 들어있다.
		)
{
	// mmlCmdTbl에 등록된 명령어 권한과 mmcdCliTbl에 저장된 사용자 등급을 비교하여
	//	허용 여부를 판단한다.
	// - privilege값이 작을수록 권한이 높다
	//
	// chg-pwd 경우 자신의 pwd는 변경할 수 있도록 한다.
	if (!strcasecmp(inputCmdInfo->cmdName, "chg-pwd")) return 1;	
	if (mmcdCliTbl[inputCmdInfo->cliIndex].privilege > mmlCmdTbl[inputCmdInfo->cmdIndex].privilege) {
		sprintf(inputErrBuf,"CAN'T_EXECUTE_COMMAND(PRIVILEGE_VIOLATION)");
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[mmcd_checkUserPrivilege] can't execute command (privilege violation)\n");
			trclib_writeLog (FL,trcBuf);
		}
		return -1;
	}
	return 1;
} //----- End of mmcd_checkUserPrivilege -----//


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
int mmcd_checkUserConfirm (
		MMLInputCmdInfo *inputCmdInfo
		)
{
	if (mmlCmdTbl[inputCmdInfo->cmdIndex].confirm == 'Y') {
		if (inputCmdInfo->confirm == 0) 
			return 0;
		else if (inputCmdInfo->confirm == -1)
		 	return -1;	
		else if (inputCmdInfo->confirm == 1)
		 	return 1;	
	}
	return 1;
} //----- End of mmcd_checkUserConfirm -----//


int checkEqual(char *input)
{
	int len, i, j;
//	char realinput[256];

	len = strlen(input) +1;

	for (i=0, j=0; i< len; i++) 
	{
		if (input[i] != '"') 
		{
			input[j] = input[i];
			j++;
		}
	}		
	return 0;
}
