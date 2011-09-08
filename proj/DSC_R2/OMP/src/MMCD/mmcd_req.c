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
// client�κ��� ������ input sting�� �м��Ͽ� syntax�� Ȯ���ϰ� ó���� ���μ�����
//	�����Ѵ�.
// - inputCmdInfo�� ���� ������ setting�� �� application���� ���� txReqMsg�� �����ϰ�
//	mmcdJoTbl�� ���� ������ ������ �� client�� ACCEPT �޽����� ������.
// 1. ����� ���� client�� ����� socket fd�� ���� input string�� �����Ѵ�.
// 2. client socket fd�� Ȯ���Ͽ� �޽����� ���� client ������ mmcdCliTbl���� ã�´�.
// 3. input string�� token���� �߶󳻾� inputCmdInfo�� cmdName�� parameter���� �����Ѵ�.
// 4. ������ cmdName�� ��ġ�ϴ� ��ɾ mmlCmdTbl���� ã�´�.
// 5. mmlCmdTbl�� �˻��Ͽ� �� �Ķ���͵��� �̸��� ä���.  �Ķ���� ���� �ʰ�, �Ķ����
//	�̸� Ȯ��, �Ķ���� �ߺ�Ȯ�� �� error check
// 6. �Ķ���� list�� mmlCmdTbl�� Ȯ���Ͽ� ��ϵ� ������� �����Ѵ�.
// 7. �ʼ� �Ķ���� �Է¿��� Ȯ��
// 8. �� �Ķ���� ������ validation�� check�Ѵ�.
// 9. command privilege�� client�� ����� ���Ͽ� ������ �ִ��� Ȯ���Ѵ�.
// 10. Application���� ���� ��ɾ� ó�� �䱸 �޽����� �����Ѵ�.
// 11. mmcdJobTbl���� jobNo�� �Ҵ��ϰ� ������ �����Ѵ�.
// 12. ixpc�� ���� application���� �����Ѵ�.
// 13. client�� ����� ���� �ԷµǾ����� �˸��� accept �޽����� ������.
//		- history ��ȸ�� ���� ��� log�� �Բ� ���´�.
//-----------------------------------------------------------------------------
int mmcd_exeMmcReqMsg (
    SockLibMsgType	*rxSockMsg,	// client�κ��� ������ ������
    int		cliFd				// client�� ���ӵ� socket fd
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
        int     cliReqId; // client���� �Ҵ��� key�� -> client���� �Ҵ��ؼ� ������, MMCD��
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
        // ����� ���� client�� ����� socket fd�� ���� input string�� �����Ѵ�.
        // client���� �Ҵ��ؼ� ���� key���� request_id�� �Բ� �����Ѵ�.
        //
        inputCmdInfo.cliSockFd = cliFd;
        inputCmdInfo.cliReqId  = (rxCliReqMsg->head.cliReqId);
        inputCmdInfo.batchFlag  = (rxCliReqMsg->head.batchFlag);
//		inputCmdInfo.cliReqId  = ntohl(rxCliReqMsg->head.cliReqId);
//		inputCmdInfo.batchFlag  = ntohl(rxCliReqMsg->head.batchFlag);
        inputCmdInfo.confirm = rxCliReqMsg->head.confirm;
        inputCmdInfo.clientType = rxCliReqMsg->head.clientType;
        
        strcpy (inputCmdInfo.inputString, inputString);
                
		// client�κ��� ������ input string�� token���� �߶󳻾� inputCmdInfo��
		//	cmdName�� parameter���� �����Ѵ�.
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
        
        // �޽����� ���� client�� ã�´�.
        // - client socket fd�� ���� ���� mmcdCliTbl���� ã�� �� index�� return�Ѵ�.
        if ((inputCmdInfo.cliIndex = mmcd_getCliIndex (inputCmdInfo.cliSockFd)) < 0) 
        {
            if (!strcasecmp (inputCmdInfo.cmdName, "LOG-IN")) 
            {
                // client�� socket ���� �� ���� ���� login������ ���ľ� client table��
                //	������ ����ǹǷ� ...
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
	
        // client�� userName�� �����Ѵ�.
        strcpy (inputCmdInfo.userName, mmcdCliTbl[inputCmdInfo.cliIndex].userName);
        inputCmdInfo.nmsibFlag = mmcdCliTbl[inputCmdInfo.cliIndex].nmsibFlag;
        //printf("userName=(%s)cliIndex=%d\n  ", inputCmdInfo.userName, inputCmdInfo.cliIndex);
	
        if (trcFlag || trcLogFlag) 
        {
            sprintf(trcBuf,"[mmcd_exeMmcReqMsg] input=(%s)(%s)\n",
                    inputCmdInfo.inputString, inputCmdInfo.userName);
            trclib_writeLog (FL,trcBuf);
        }
        
        // '?'�� �̿��� command_help�� ��û�� ����̸� help����� �����Ѵ�.
        //
        if (strstr(inputCmdInfo.inputString, "?") != NULL) 
        {
            isHelp = 1;
            /* �Ķ���� �� '?'�� ������ ������ �̸��� �� �����Ƿ� ���� ó���� (2005.9.1) */
            for (j=0; j < inputCmdInfo.paraCnt; j++) 
            {
                if (strstr(inputCmdInfo.paraInfo[j].paraVal, "?") != NULL) 
                {
                    /* �Ķ���Ͱ� '?'�� �ִ� ���� ������ */
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
        // ������ cmdName�� ��ġ�ϴ� ��ɾ mmlCmdTbl���� ã�´�.
        // - mmlCmdTbl�� index�� return�ȴ�.
        //
        if ((inputCmdInfo.cmdIndex = mmcd_getCmdIndex (inputCmdInfo.cmdName)) < 0) 
        {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        
        // client�� �Ķ���͵��� ���� �������Ƿ�, mmlCmdTbl�� �˻��Ͽ� �� �Ķ���͵���
        //	�̸��� ä���.
        // - �Ķ���� ���� �ʰ�, �Ķ���� �̸� Ȯ��, �Ķ���� �ߺ�Ȯ�� �� error check
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
        
        
        
        // �ʼ� �Ķ���� �� (OPTIONAL ����)  �Ķ���� �Է¿��� Ȯ��
        if (mmcd_checkMandatoryPara (&inputCmdInfo) < 0) 
        {
//			mmcd_sendInputError2Client (&inputCmdInfo);
            strcat(inputCmdInfo.inputString, "?");
            strcat(inputCmdInfo.cmdName, "?");
            mmcd_builtin_cmd_help(&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_checkMandatoryPara... OK\n");
        
        
        // �� �Ķ���� ������ validation�� check�Ѵ�.
        //
        if (mmcd_checkValidParaVal (&inputCmdInfo) < 0) 
        {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_checkValidParaVal... OK\n");
        
        // command privilege�� client�� ����� ���Ͽ� ������ �ִ��� Ȯ���Ѵ�.
        //
        if (mmcd_checkUserPrivilege (&inputCmdInfo) < 0) {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_checkUserPrivilege... OK\n");
        
        
        // mmcd�� ���� ó���ؾ� �ϴ� built-in ��ɿ��θ� Ȯ���Ѵ�.
        // - built-in ����� ��� ���� ó�� �� return�ȴ�.
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
        
        // Application���� ���� ��ɾ� ó�� �䱸 �޽����� �����Ѵ�.
        //
        mmcd_makeReqMsg (&inputCmdInfo, &txGenQMsg);
        //printf("mmcd_makeReqMsg\n");
        
        // mmcdJobTbl���� jobNo�� �Ҵ��ϰ� ������ �����Ѵ�.
        //
        if (mmcd_saveReqData2JobTbl (&inputCmdInfo, &txGenQMsg) < 0) 
        {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_saveReqData2JobTbl\n");
        
        
        // ixpc�� ���� application���� �����Ѵ�.
        //
        txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
        txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;
        
        if (msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0)
        {
            sprintf(inputErrBuf,"INTERNAL_ERROR(msgsnd fail)");
            mmcd_sendInputError2Client (&inputCmdInfo);
            
            // �Ҵ��ߴ� job table�� �����Ѵ�.
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
        
        // client�� �Ķ���͵��� ���� �������Ƿ�, mmlCmdTbl�� �˻��Ͽ� �� �Ķ���͵���
        //	�̸��� ä���.
        // - �Ķ���� ���� �ʰ�, �Ķ���� �̸� Ȯ��, �Ķ���� �ߺ�Ȯ�� �� error check
        //
        
        if (mmcd_fillInputParaName (&inputCmdInfo) < 0) {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        
        mmcd_arrangeInputPara (&inputCmdInfo);
        
        // �ʼ� �Ķ���� �� (OPTIONAL ����)  �Ķ���� �Է¿��� Ȯ��
        if (mmcd_checkMandatoryPara (&inputCmdInfo) < 0) {
//			mmcd_sendInputError2Client (&inputCmdInfo);
            strcat(inputCmdInfo.inputString, "?");
            strcat(inputCmdInfo.cmdName, "?");
            mmcd_builtin_cmd_help(&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_checkMandatoryPara... OK\n");
        
        
        // �� �Ķ���� ������ validation�� check�Ѵ�.
        //
        if (mmcd_checkValidParaVal (&inputCmdInfo) < 0) {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_checkValidParaVal... OK\n");
        
        // command privilege�� client�� ����� ���Ͽ� ������ �ִ��� Ȯ���Ѵ�.
        //
        if (mmcd_checkUserPrivilege (&inputCmdInfo) < 0) {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_checkUserPrivilege... OK\n");
        
        
        // mmcd�� ���� ó���ؾ� �ϴ� built-in ��ɿ��θ� Ȯ���Ѵ�.
        // - built-in ����� ��� ���� ó�� �� return�ȴ�.
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
        
        // Application���� ���� ��ɾ� ó�� �䱸 �޽����� �����Ѵ�.
        //
        mmcd_makeReqMsg (&inputCmdInfo, &txGenQMsg);
        //printf("mmcd_makeReqMsg\n");
        
        // mmcdJobTbl���� jobNo�� �Ҵ��ϰ� ������ �����Ѵ�.
		//
        if (mmcd_saveReqData2JobTbl (&inputCmdInfo, &txGenQMsg) < 0) {
            mmcd_sendInputError2Client (&inputCmdInfo);
            return -1;
        }
        //printf("mmcd_saveReqData2JobTbl\n");
        
        
        // ixpc�� ���� application���� �����Ѵ�.
        //
        txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
        txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;
        
        if (msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0)
        {
            sprintf(inputErrBuf,"INTERNAL_ERROR(msgsnd fail)");
            mmcd_sendInputError2Client (&inputCmdInfo);
            
            // �Ҵ��ߴ� job table�� �����Ѵ�.
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
    
    // client�� ����� ���� �ԷµǾ����� �˸��� accept �޽����� ������.
    // - history ��ȸ�� ���� ��� log�� �Բ� ���´�.
    //
    mmcd_sendInputAccepted2Client (&inputCmdInfo);
    
    return 1;
    
} //----- End of mmcd_exeMmcReqMsg -----//



//------------------------------------------------------------------------------
// client�κ��� ������ command ������ token���� �߶� inputCmdInfo�� �ִ´�.
// - command_name�� �Ķ���� �κз� ������, �� �Ķ���͵��� token���� �߶󳻾�
//	�����Ѵ�.
// - �Ķ������ ������ �Բ� �����Ѵ�.
// - �Է����� �ʴ� �Ķ���Ϳ��� ������� �����Ƿ� NULL�� ����.
// - ���߿� ���� �� �����Ƿ� ������ command ������ �״�� �����صд�.
//------------------------------------------------------------------------------
int mmcd_tokeniseInputString (
		MMLInputCmdInfo *inputCmdInfo // ������ ���� string�� ����ְ�, ���⿡ token���� ����ȴ�.
		)
{
	char	*ptr,*next,*token,remain[256];
	int		paraCnt, k;

	// ���ۺκ��� white-space�� �����.
	for (ptr=inputCmdInfo->inputString; isspace(*ptr); ptr++);
	strcpy (inputCmdInfo->inputString, ptr); // ���ۺκ��� white-space�� ���� ������ �ٽ� �����Ѵ�.

	// command name�� �߶󳽴�.
	// - name�� parameter�� �ϳ��̻��� space�� ���еȴ�.
	strcpy (remain, inputCmdInfo->inputString);
	ptr = remain;

	token = (char*)strtok_r(ptr," \t",&next);
	if (token == NULL) {
		sprintf(trcBuf,"[mmcd_tokeniseInputString] strtok_r fail");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	if (next == NULL) {
		// parameter�� ���� ���
		strcpy (inputCmdInfo->cmdName, remain);
		return 1;
	}
	strcpy (inputCmdInfo->cmdName, token);

	//
	// �Ķ���͸� �ϳ��� �߶󳽴�.
	// - �Ķ���͵��� ","�� ���еȴ�.
	//
	paraCnt = 0;
	while(1)
	{
		// �Ķ���͵� ������ white-space�� �����.
		for (ptr = next; isspace(*ptr); ptr++) ;

		// optional parameter�� �Էµ��� ���� ���. ex) "aaa-bbb xxx,,,2"
		if (*ptr == ',') {
			paraCnt++;
			next = ++ptr;
			continue;
		}

		if ((token = (char*)strtok_r(ptr,",",&next)) == NULL)
			break;
		strcpy (inputCmdInfo->paraInfo[paraCnt].paraVal, token);

		// �Ķ���� ���� white-space�� �����.

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
			break; // ������ �Ķ����
		}
	}
	inputCmdInfo->paraCnt = paraCnt;

	return 1;

} //----- End of mmcd_tokeniseInputString -----//



//------------------------------------------------------------------------------
// mmcd_tokenise()���� �Ķ���� �κ��� delimeter�� ���� token��� �߶� ��������
//	mmlCmdTbl�� �˻��Ͽ� �� �Ķ���͵��� �̸��� ä���.
// - �Ķ���� ���� �ʰ�, �Ķ���� �̸� Ȯ��, �Ķ���� �ߺ�Ȯ�� �� error check
// - �Ϻ� �Ķ���͸� �̸��� �����Ͽ� �Է��� ��� error ó���Ѵ�.
//------------------------------------------------------------------------------
int mmcd_fillInputParaName (
		MMLInputCmdInfo *inputCmdInfo // token���� �߶󳻾��� parameter���� ����ְ�, ���⿡ �̸��� ä������.
		)
{
	char	*name, *value, tmp[COMM_MAX_VALUE_LEN]={0,};
	int		i, cmd, paraIndex;
	int		paraPattern=0;

	cmd = inputCmdInfo->cmdIndex;

	// �Ķ���͸� �ʹ� ���� �Է��� ���
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
		// �Էµ� �Ķ���͸� ã�´�.
		if (strcasecmp(inputCmdInfo->paraInfo[paraIndex].paraVal, ""))
		{
			// �Ķ���͸� "aaa=123" �������� �Ķ���� �̸��� �����Ͽ� �Է��� ���
			//	�ش� ��ɾ ��ϵ� �̸����� Ȯ���Ѵ�.
			if ((strstr(inputCmdInfo->paraInfo[paraIndex].paraVal, "=") != NULL) && (inputCmdInfo->paraInfo[paraIndex].paraVal[0] != '('))
			{
				// �Ϻ� �Ķ���͸� �̸��� �������� ���ϵ��� �����Ѵ�.
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

				// paraName�� value�� �и��Ѵ�.
				strcpy (tmp, inputCmdInfo->paraInfo[paraIndex].paraVal);
				name = (char*)strtok_r(tmp, "=", &value);

				// "=" �յ� ��, name�� ���ʰ� value�� ���� white-space�� ���ش�.
				for (i=0; i<strlen(name) && !isspace(name[i]); i++); name[i]=0;
				for (i=0; isspace(*value); value++);

				// ��ϵ� �Ķ�������� Ȯ���Ѵ�.
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
			} else { // �Ķ���� �̸����� value�� �Է��� ���
				// �Ϻ� �Ķ���͸� �̸��� �������� ���ϵ��� �����Ѵ�.
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

			// �Ķ���͸� �ߺ��ؼ� �����ߴ��� Ȯ���Ѵ�.
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

			// Ȯ���� �������� �Ķ���� �̸��� ä���.
			strcpy (inputCmdInfo->paraInfo[paraIndex].paraName, name);
			strcpy (inputCmdInfo->paraInfo[paraIndex].paraVal, value);
//fprintf(stderr, "name = %s, val = %s\n", inputCmdInfo->paraInfo[paraIndex].paraName, inputCmdInfo->paraInfo[paraIndex].paraVal);
		}
	}

	// client�κ��� ������ �Է� string ������ ����� inputCmdInfo->inputString��
	//	"aaa=xxx" ���·� ��ȯ�Ѵ�.
	// -> client�� accept �޽��� ��� �� ��ɾ� history�� ����� ���·� ��ȯ
	sprintf(inputCmdInfo->inputString, "%s", inputCmdInfo->cmdName);
	for (i=0; i < strlen(inputCmdInfo->inputString); i++)
		inputCmdInfo->inputString[i] = toupper(inputCmdInfo->inputString[i]);
	
	// ��°�� pass �ٲܶ� ���� ***ǥ�ø� �Ѱ̴ϱ�?
#if 0	
	for (i=0; i < inputCmdInfo->paraCnt; i++)
	{
		if (strcasecmp(inputCmdInfo->paraInfo[i].paraVal, "")) {
			// PASSWD �Ķ���ʹ� ***�� ����ϱ� ���� inputString�� ������ �ٲ۴�.
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

	// �ǳ��� �ִ� ","�� �����.
	if (strcmp (tmp, "")) {
		inputCmdInfo->inputString[strlen(inputCmdInfo->inputString)-1] = 0;
	}

	return 1;

} //----- End of mmcd_fillInputParaName -----//



//------------------------------------------------------------------------------
// mmlCmdTbl�� �˻��Ͽ� �Էµ� �Ķ���͵��� ������ mmlCmdTbl�� ��ϵ� �������
//	�����Ѵ�.
//------------------------------------------------------------------------------
int mmcd_arrangeInputPara (
		MMLInputCmdInfo *inputCmdInfo // �Էµ� �Ķ���͵��� ����ְ�, ������ ���⿡ �ٽ� ��ϵȴ�.
		)
{
	MMLInputCmdInfo	tmp;
	int		i,k,cmd,cnt=0;

	memset ((void*)&tmp, 0, sizeof(MMLInputCmdInfo));
	cmd = inputCmdInfo->cmdIndex;

	// mmlCmdTbl�� ��ϵ� �Ķ���͵��� �ϳ��� ���� inputCmdInfo�� ������ tmp��
	//	�����ϴ� ������� �����Ѵ�.
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

	// ���ĵ� �����͸� inputCmdInfo�� �����.
	memcpy ((void*)inputCmdInfo->paraInfo, tmp.paraInfo, sizeof(tmp.paraInfo));

	return 1;

} //----- End of mmcd_arrangeInputPara -----//



//------------------------------------------------------------------------------
// �ʼ� �Ķ���� �Է¿��θ� Ȯ���Ѵ�.
//------------------------------------------------------------------------------
int mmcd_checkMandatoryPara (
		MMLInputCmdInfo *inputCmdInfo // �Էµ� �Ķ���͵��� ���ĵǾ� ����ִ�.
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
// �� �Ķ���� ������ validation�� check�Ѵ�.
//------------------------------------------------------------------------------
int mmcd_checkValidParaVal (
		MMLInputCmdInfo *inputCmdInfo // �Էµ� �Ķ���͵��� ���ĵǾ� ����ִ�.
		)
{
	int		cmd,paraIndex;
	char	reason[2048]; //050401.r11.cjs 1024->2048

	cmd = inputCmdInfo->cmdIndex;

  //limsh
  //for (paraIndex=0; paraIndex < inputCmdInfo->paraCnt; paraIndex++) {

	for (paraIndex=0; paraIndex < mmlCmdTbl[cmd].paraCnt; paraIndex++) {
		if (strcasecmp(inputCmdInfo->paraInfo[paraIndex].paraName, "")) {
			// �Ķ���� type�� �°� �ԷµǾ����� Ȯ���Ѵ�.
			// - �Է°����� ��������, range�� �������� Ȯ��
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
// �Էµ� �Ķ������ ���� ��ϵ� type�� �´���, range�� �������� Ȯ���Ѵ�.
//------------------------------------------------------------------------------
int mmcd_checkValidValueOnType (
		int cmd,             // mmlCmdTbl���� �ش� command�� index
		int paraIndex,       // mmlCmdTbl���� �ش� parameter�� index
		char *inputValString,// �Էµ� �Ķ���� ��(string)
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

		case MML_PTYPE_STRING: // string type�� ��� min,max�� �ڸ����� �ǹ��Ѵ�.
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
		case MML_PTYPE_FIXSTR: // fix string type�� ��� fixval �� �ڸ����� ���Ѵ�
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
		case MML_PTYPE_FIXDEC: // fix string type�� ��� fixval �� �ڸ����� ���Ѵ�
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

		case MML_PTYPE_DECSTR: // string type�� ��� min,max�� �ڸ����� �ǹ��Ѵ�.
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
			// enumList�� ��ϵ� string���� Ȯ���Ѵ�.
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
// mmcdCliTbl�� mmlCmdTbl �˻��Ͽ� ����� ��û�� client�� ��ɾ� ��� ������ �ִ���
//	Ȯ���Ѵ�.
//------------------------------------------------------------------------------
int mmcd_checkUserPrivilege (
		MMLInputCmdInfo *inputCmdInfo	// �Էµ� ��ɾ� ������ ����ִ�.
		)
{
	// mmlCmdTbl�� ��ϵ� ��ɾ� ���Ѱ� mmcdCliTbl�� ����� ����� ����� ���Ͽ�
	//	��� ���θ� �Ǵ��Ѵ�.
	// - privilege���� �������� ������ ����
	//
	// chg-pwd ��� �ڽ��� pwd�� ������ �� �ֵ��� �Ѵ�.
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
