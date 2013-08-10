#include "cond_proto.h"
#include <ctype.h>

extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern char	trcBuf[4096], trcTmp[1024];
extern int	trcFlag, trcLogFlag;

char	resBuf[4096], resHead[1024];

extern InhMsgTbl *inhMsg;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void *cond_mmc_srch_log_his (void *arg)
{
	int		i, noMsgFlag=1;
	int		syear, smon, sday, shour, smin;
	int		eyear, emon, eday, ehour, emin;
	int		year, mon, day, hour;
	char	argType[COMM_MAX_VALUE_LEN], argStime[COMM_MAX_VALUE_LEN], argEtime[COMM_MAX_VALUE_LEN];
	char	stime[32], etime[32], firstFileName[256], lastFileName[256];
	char	*env, tmpBuf[256], logBaseDir[256], logFileName[256];
	char	seqNo=1;
	IxpcQMsgType	rxIxpcMsg;
	MMLReqMsgType	*rxMmlReqMsg;

/*
	rxIxpcMsg   = (IxpcQMsgType*)arg;
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
*/
	memcpy ((void*)&rxIxpcMsg, arg, sizeof(IxpcQMsgType));
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg.body;

	if (!strcasecmp (rxMmlReqMsg->head.cmdName, "dis-msg-his")) {
		strcpy (argType,  rxMmlReqMsg->head.para[0].paraVal);
		strcpy (argStime, rxMmlReqMsg->head.para[1].paraVal);
		strcpy (argEtime, rxMmlReqMsg->head.para[2].paraVal);
	} else if (!strcasecmp (rxMmlReqMsg->head.cmdName, "dis-cmd-his")) {
		strcpy (argType,  "CMD");
		strcpy (argStime, rxMmlReqMsg->head.para[0].paraVal);
		strcpy (argEtime, rxMmlReqMsg->head.para[1].paraVal);
	} else {
		return NULL;
	}

	for (i=0; i<strlen(argType); i++) // 결과 출력시 대문자로 출력하기 위해
		argType[i] = toupper (argType[i]);

	if (!strcasecmp (rxMmlReqMsg->head.cmdName, "dis-msg-his")) {
		sprintf (resHead,"    MSG_TYPE    = %s\n    START_TIME  = %s\n    END_TIME    = %s\n",
		//blank sprintf (resHead,"  MSG_TYPE    = %s\n  START_TIME  = %s\n  END_TIME    = %s\n",
				argType, argStime, argEtime);
	} else if (!strcasecmp (rxMmlReqMsg->head.cmdName, "dis-cmd-his")) {
		sprintf (resHead,"    START_TIME  = %s\n    END_TIME    = %s\n",
		//blank sprintf (resHead,"  START_TIME  = %s\n  END_TIME    = %s\n",
				argStime, argEtime);
	}
	strcpy (resBuf, resHead);

	//
	// check start_time, end_time validation
	//
	if (cond_srch_log_his_checkParaTimeValue (argStime, &syear, &smon, &sday, &shour, &smin) < 0) {
		strcat (resBuf,"    RESULT      = FAIL\n    REASON      = INVALID_START_TIME_VALUE\n");
		//blank strcat (resBuf,"  RESULT      = FAIL\n  REASON      = INVALID_START_TIME_VALUE\n");
		cond_txMMLResult (&rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
		return NULL;
	}
	if (cond_srch_log_his_checkParaTimeValue (argEtime, &eyear, &emon, &eday, &ehour, &emin) < 0) {
		strcat (resBuf,"    RESULT      = FAIL\n    REASON      = INVALID_END_TIME_VALUE\n");
		//blank strcat (resBuf,"  RESULT      = FAIL\n  REASON      = INVALID_END_TIME_VALUE\n");
		cond_txMMLResult (&rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
		return NULL;
	}


	// start_time이 end_time보다 크면 error처리
	if (strcasecmp(argStime, argEtime) > 0) {
		strcat (resBuf,"    RESULT      = FAIL\n    REASON      = INVALID_TIME_RANGE\n");
		//blank strcat (resBuf,"  RESULT      = FAIL\n  REASON      = INVALID_TIME_RANGE\n");
		cond_txMMLResult (&rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
		return NULL;
	}

	if ((env = getenv(IV_HOME)) == NULL) {
		sprintf (tmpBuf,"    RESULT     = FAIL\n    REASON      = NOT_FOUND_%s_ENVIRONMENT_NAME\n", IV_HOME);
		//blank sprintf (tmpBuf,"  RESULT     = FAIL\n  REASON      = NOT_FOUND_%s_ENVIRONMENT_NAME\n", IV_HOME);
		strcat (resBuf, tmpBuf);
		cond_txMMLResult (&rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
		return NULL;
	}

	// 로그의 type에 따라 저장된 directory가 다르므로 그 위치를 결정한다.
	//
	if (!strcasecmp (argType, "ALM")) {
		sprintf (logBaseDir, "%s/%s", env, COND_ALMLOG_FILE);
		strcpy  (argType, "ALARM");
	} else if (!strcasecmp (argType, "STS")) {
		sprintf (logBaseDir, "%s/%s", env, COND_STSLOG_FILE);
		strcpy  (argType, "STATUS");
	} else if (!strcasecmp (argType, "MMC")) {
		sprintf (logBaseDir, "%s/%s", env, MMCD_MMCLOG_FILE);
		strcpy  (argType, "MMC");
	} else if (!strcasecmp (argType, "CMD")) {
		sprintf (logBaseDir, "%s/%s", env, MMCD_CMDHIS_FILE);
		strcpy  (argType, "COMMAND");
	}

	// 검색대상 파일을 비교하기 위해 해당 시간범위 내의 첫번째와 마지막 파일이름을 결정한다.
	//
	sprintf (firstFileName,"%s/%04d/%02d/%02d/%02d", logBaseDir, syear, smon, sday, shour);
	sprintf (lastFileName, "%s/%04d/%02d/%02d/%02d", logBaseDir, eyear, emon, eday, ehour);
	// 로그 검색시 시간범위 확인을 위한 string을 구성한다.
	//
	sprintf (stime, "%04d-%02d-%02d %02d:%02d", syear, smon, sday, shour, smin);
	sprintf (etime, "%04d-%02d-%02d %02d:%02d", eyear, emon, eday, ehour, emin);

	if (!strcasecmp (rxMmlReqMsg->head.cmdName, "dis_msg_his")) {
		sprintf (resHead,"    MSG_TYPE    = %s \n", argType);
		//blank sprintf (resHead,"  MSG_TYPE    = %s \n", argType);
	} else {
		strcpy (resHead, "");
	}
	sprintf (tmpBuf,"    START_TIME  = %04d-%02d-%02d %02d:%02d \n",
			syear, smon, sday, shour, smin);
	strcat (resHead, tmpBuf);
	sprintf (tmpBuf,"    END_TIME    = %04d-%02d-%02d %02d:%02d \n",
			eyear, emon, eday, ehour, emin);
	strcat (resHead, tmpBuf);

	strcpy (resBuf, resHead);


	//
	// 해당 파일들을 차례로 검색한다.
	// - 1시간 단위로 파일이 생성되는데, 해당 시간대에 로그내용이 없으면 파일도 없다.
	//
	for (year=syear; year<=eyear; year++) 
	{
		for (mon=1; mon<=12; mon++) 
		{
			for (day=1; day<=31; day++) 
			{
				for (hour=0; hour<=23; hour++) 
				{
					sprintf (logFileName, "%s/%04d/%02d/%02d/%02d", logBaseDir, year, mon, day, hour);

					if (strcasecmp (logFileName, firstFileName) < 0)
					{
						continue;
					}
					if (strcasecmp (logFileName, lastFileName) > 0)
					{
						goto search_complete;
					}

					cond_srch_log_his_searchLogFile (&rxIxpcMsg, logFileName, stime, etime, &noMsgFlag, &seqNo);
				}
			}
		}
	}
search_complete:

	// cond_srch_log_his_searchLogFile에서 일정크기로 나누어 보낼때에는 contFlag를 1로 setting해서
	//	보냈고, 마지막(END) 메시지는 여기서 보낸다.
	//
	if (noMsgFlag) {
		strcat (resBuf,"    RESULT      = FAIL\n    REASON      = NO_MESSAGE\n");
		//blank strcat (resBuf,"  RESULT      = FAIL\n  REASON      = NO_MESSAGE\n");
		cond_txMMLResult (&rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
	} else {
		strcat (resBuf,"    RESULT      = SUCCESS\n");
		//blank strcat (resBuf,"  RESULT      = SUCCESS\n");
		cond_txMMLResult (&rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);
	}

	return NULL;

} //----- End of cond_mmc_srch_log_his -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int cond_srch_log_his_checkParaTimeValue (char *time, int *year, int *mon, int *day, int *hour, int *min)
{
	int		i;
	char	tmp[32];

	for (i=0; i<strlen(time); i++) {
		if (!isdigit(time[i]))
			return -1;
	}

	strncpy (tmp, &time[0], 4); tmp[4] = 0; *year = atoi(tmp);
	strncpy (tmp, &time[4], 2); tmp[2] = 0; *mon  = atoi(tmp);
	strncpy (tmp, &time[6], 2); tmp[2] = 0; *day  = atoi(tmp);
	strncpy (tmp, &time[8], 2); tmp[2] = 0; *hour = atoi(tmp);
	strncpy (tmp, &time[10],2); tmp[2] = 0; *min  = atoi(tmp);

	if (*mon  < 1 || *mon  > 12) return -1;
	if (*day  < 1 || *day  > 31) return -1;
	if (*hour < 0 || *hour > 23) return -1;
	if (*min  < 0 || *min  > 59) return -1;

	return 1;

} //----- End of cond_srch_log_his_checkParaTimeValue -----//



//------------------------------------------------------------------------------
// resBuf에 검색한 메시지를 넣고 일정크기가 넘으면 mmcd로 결과 메시지를 보낸다.
//------------------------------------------------------------------------------
void cond_srch_log_his_searchLogFile (
			IxpcQMsgType *rxIxpcMsg,
			char *fname,
			char *stime,
			char *etime,
			int *noMsgFlag,
			char *seqNo
			)
{
	FILE	*fp;
	char	lineBuf[1024], tmpBuf[1024], oneMsg[4096], firstWord[1024];

	if ((fp = fopen(fname, "r")) == NULL) {
		if ((errno != ENOENT) && (errno != errno != ENOMSG)) {
			sprintf (trcBuf,"[cond_srch_log_his_searchLogFile] fopen fail[%s]; err=%d(%s)\n",
					fname, errno, strerror(errno));
			trclib_writeLogErr (FL,trcBuf);
		}
		return;
	}

	strcpy (oneMsg, "");
//printf("jean cond_srch_log_his_searchLogFile : %s\n", fname);
	while (fgets(lineBuf,sizeof(lineBuf),fp) != NULL)
	{
		//
		// 하나의 메시지는 "COMPLETE", "CONTINUE", "ACCEPT" 로 끝난다는 가정으로 구분한다.
		// 읽혀진 빈줄은 출력 메시지에 넣지 않는다.
		//

		if (lineBuf[0] == '\n')
			continue;

		 // - 로그를 검색한 내용임을 알아보기 쉽게 하기위해 앞에 "*"를 붙인다.
		 // - 한개의 메시지가 여러줄로 이루어져 있으므로 임시로 oneMsg에 저장한다.
		sprintf(tmpBuf, "    -%s", lineBuf);
		//blank sprintf(tmpBuf, "  -  %s", lineBuf);
		if(strlen(tmpBuf)+strlen(oneMsg) > 4000){
			cond_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, (*seqNo));

			strcpy (resBuf, resHead);
			strcpy (oneMsg, "");
			commlib_microSleep(300000);
			strcpy(oneMsg, "");
		}

		strcat (oneMsg, tmpBuf);

		if (sscanf (lineBuf, "%s", firstWord) < 1)
			continue;

		if ((strstr(firstWord,"COMPLETE") == NULL) &&
			(strstr(firstWord,"CONTINUE") == NULL) &&
			(strstr(firstWord,"ACCEPT") == NULL)) {
			continue;
		}

		// 한개의 메시지가 다 읽어진 경우

		// 각 메시지의 구분을 위해 빈줄을 하나씩 붙인다.
		strcat (oneMsg, "    -\n");
		if (cond_srch_log_his_isTimeRange (oneMsg, stime, etime) == 0) {
			// 시간범위에 들지 않는 놈이면 버린다.
			strcpy (oneMsg, "");
			continue;
		}
		*noMsgFlag = 0; // 메시지가 하나라도 검색되었음을 표시하여 result를 success로 보낼 수 있도록 한다.
// strlen(oneMsg) + strlen(resBuf)가 4096 크지 않을 때 전송하기 위해서
// strlen(oneMsg) minimum 이 1000 일 때 미리 전송하고 , 아래 strlen(resBuf)가 3000일때
// 전송하므로 위 두조건이 맞지 경우 maxium(1000+3000)이므로 4096을 넘지 않는다.
		if (strlen(oneMsg) > 1000 && strlen(resBuf) > 200)  { // 2000->1000(수정) byte 이상이면 mmcd로 결과메시지를 보낸다.
			strcat (resBuf,"    RESULT      = SUCCESS\n");
			cond_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, (*seqNo)++);

			strcpy (resBuf, resHead);
			commlib_microSleep(300000);

		}

		strcat (resBuf, oneMsg); // resBuf에 넣는다.

		strcpy (oneMsg, "");

		if (strlen(resBuf) > 3000) { // 3000 byte 이상이면 mmcd로 결과메시지를 보낸다.
			strcat (resBuf,"    RESULT      = SUCCESS\n");
			cond_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, (*seqNo)++);

			strcpy (resBuf, resHead);
			commlib_microSleep(300000);
		}

	}
	fclose(fp);

	return;

} //----- End of cond_srch_log_his_searchLogFile -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int cond_srch_log_his_isTimeRange (char *msg, char *stime, char *etime)
{
	char	*ptr, tmp[4096];

	strcpy (tmp, msg); // strtok_r을 호출하면 delimiter 자리에 NULL을 채워서 원본메시지가 회손된다.

	ptr = tmp;
	ptr = ptr + 16;
	// 년,월,일,시,분 까지 비교
	//
	if (strncmp(stime, ptr, strlen(stime)) > 0) // stime보다 작으면
		return 0;
	if (strncmp(etime, ptr, strlen(etime)) < 0) // etime보다 크면
		return 0;

	return 1;

} //----- End of cond_srch_log_his_isTimeRange -----//

#if 1 //jean 
void *cond_mmc_alw_msg (void *arg)
{
	int		i, noMsgFlag=1;
	char	msgType[INH_MSG_NUM_LEN], msgNum[INH_MSG_NUM_LEN];
	char	tmpBuf[256];
	char	seqNo=1;
	IxpcQMsgType	rxIxpcMsg;
	MMLReqMsgType	*rxMmlReqMsg;

	int delNum;
	
	memcpy ((void*)&rxIxpcMsg, arg, sizeof(IxpcQMsgType));
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg.body;

	strcpy (msgType,  rxMmlReqMsg->head.para[0].paraVal);
	strcpy (msgNum, rxMmlReqMsg->head.para[1].paraVal);

	for (i=0; i<strlen(msgType); i++) // 결과 출력시 대문자로 출력하기 위해
		msgType[i] = toupper (msgType[i]);

	sprintf (resHead,"    MSGTYPE = %s\n    MSGNUM = %s\n",	msgType, msgNum);
	strcpy (resBuf, resHead);

	for( i=0; i<MAX_INH_MSG_CNT ; i++){
		if( !strcasecmp(msgType,inhMsg->msgType[i]) && 
			!strcasecmp(msgNum,inhMsg->msgNum[i])){

			if( inhMsg->msgFlag[i] == MSG_INHIB ){
				delNum=i;
				noMsgFlag=1;
			} else{
				noMsgFlag=0;
			} 
			break;
		}
	}
	if ( i == MAX_INH_MSG_CNT ){
		sprintf(tmpBuf,"    RESULT = FAIL\n    REASON = NOT_FOUND_%s_MSG(%s)\n",msgType, msgNum);
		strcat (resBuf, tmpBuf);
		cond_txMMLResult (&rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
		return NULL;
	} else {
		if( noMsgFlag == 0 ){
			sprintf(tmpBuf,"    RESULT = FAIL\n    REASON = %s_MSG(%s) is Already Allowed\n",msgType, msgNum);
			strcat (resBuf, tmpBuf);
			cond_txMMLResult (&rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
			return NULL;
		}else{
			//bzero(inhMsg->msgType[delNum],sizeof(char)*INH_MSG_NUM_LEN);
			//bzero(inhMsg->msgNum[delNum],sizeof(char)*INH_MSG_NUM_LEN);
			inhMsg->msgFlag[delNum] = MSG_ALLOW;
		}
	}

    Write_InhMsg_Info(inhMsg);

	// cond_srch_log_his_searchLogFile에서 일정크기로 나누어 보낼때에는 contFlag를 1로 setting해서
	//	보냈고, 마지막(END) 메시지는 여기서 보낸다.
	//
	sprintf(tmpBuf,"    RESULT = SUCCESS\n    REASON = INHIB_MSG_%s(%s) --> ALLOW_MSG_%s(%s) CHANGE OK!!\n", msgType, msgNum, msgType, msgNum);
	strcat (resBuf,tmpBuf);
	cond_txMMLResult (&rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);

	return NULL;

} //----- End of cond_mmc_alw_msg -----//

void *cond_mmc_dis_inh_msg (void *arg)
{
	int     i;
	char    msgType[INH_MSG_NUM_LEN];
	char    tmpBuf[1024], tmpStr[16];
	char    seqNo=1;
	IxpcQMsgType    rxIxpcMsg;
	MMLReqMsgType   *rxMmlReqMsg;

	int paraFlag=0, count=0;

	memcpy ((void*)&rxIxpcMsg, arg, sizeof(IxpcQMsgType));
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg.body;


	if( strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "") ){
		paraFlag=1;

		strcpy (msgType,  rxMmlReqMsg->head.para[0].paraVal);
		for (i=0; i<strlen(msgType); i++) // 결과 출력시 대문자로 출력하기 위해
			msgType[i] = toupper (msgType[i]);
	} else{
		paraFlag=0;
		strcpy (msgType,  "ALL");
	}

	sprintf (resHead,"    MSGTYPE  = %s\n", msgType);
	strcat (resBuf, tmpBuf);
	sprintf(tmpBuf,"    RESULT = SUCCESS\n");
	strcat (resHead, tmpBuf);
	sprintf(tmpBuf,"    =====================================================================\n");
	strcat (resHead, tmpBuf);
	sprintf(tmpBuf,"    %-8s  %-8s  %-7s  %s\n", "INH_FLAG", "MSG_TYPE", "MSG_NUM", "INFORMATION");
	strcat (resHead, tmpBuf);
	sprintf(tmpBuf,"    =====================================================================\n");
	strcat (resHead, tmpBuf);

	strcpy (resBuf, resHead);

	if(paraFlag == 1){
		for( i=0; i<MAX_INH_MSG_CNT ; i++){
			if( inhMsg->msgType[i][0] != 0  &&  !strcasecmp(inhMsg->msgType[i], msgType) ){
				if(strlen(resBuf) > 4000){
					cond_txMMLResult (&rxIxpcMsg, resBuf, 0, 1, 5, 1, seqNo);
					seqNo = seqNo+1;
					strcpy (resBuf, resHead);
					commlib_microSleep(100000);
				}

				if (inhMsg->msgFlag[i] == MSG_INHIB)
					strcpy(tmpStr , "INHIBIT");
				else
					strcpy(tmpStr , "ALLOW");

				sprintf(tmpBuf,"    %-8s  %-8s  %-7s  %s\n", tmpStr, inhMsg->msgType[i], inhMsg->msgNum[i], inhMsg->msgInfo[i]);
				strcat (resBuf, tmpBuf);
				count++;
			}
		}
	}else{
		for( i=0; i<MAX_INH_MSG_CNT ; i++){
			if( inhMsg->msgType[i][0] != 0 ){
				if(strlen(resBuf) > 3500){

					cond_txMMLResult (&rxIxpcMsg, resBuf, 0, 1, 5, 1, seqNo);
					seqNo = seqNo+1;
					strcpy (resBuf, resHead);
					commlib_microSleep(100000);
				}

				if (inhMsg->msgFlag[i] == MSG_INHIB)
					strcpy(tmpStr , "INHIBIT");
				else
					strcpy(tmpStr , "ALLOW");

				sprintf(tmpBuf,"    %-8s  %-8s  %-7s  %s\n", tmpStr, inhMsg->msgType[i], inhMsg->msgNum[i], inhMsg->msgInfo[i]);
				strcat (resBuf, tmpBuf);
				count++;
			}
		}
	}

	if(count==0){
		sprintf(tmpBuf,"    NO DATA!!\n");
		strcat (resBuf, tmpBuf);
		sprintf(tmpBuf,"    =====================================================================\n");
		strcat (resBuf, tmpBuf);
	}else{
		sprintf(tmpBuf,"    ---------------------------------------------------------------------\n");
		strcat (resBuf, tmpBuf);
		sprintf(tmpBuf,"    TOTAL COUNT = %d\n", count);
		strcat (resBuf, tmpBuf);
		sprintf(tmpBuf,"    =====================================================================\n");
		strcat (resBuf, tmpBuf);
	}

	// cond_srch_log_his_searchLogFile에서 일정크기로 나누어 보낼때에는 contFlag를 1로 setting해서
	//      보냈고, 마지막(END) 메시지는 여기서 보낸다.
	//
	cond_txMMLResult (&rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);

	return NULL;

} //----- End of cond_mmc_dis_msg -----//

void *cond_mmc_inh_msg (void *arg)
{
        int		i, noMsgFlag=1;
        char    msgType[INH_MSG_NUM_LEN], msgNum[INH_MSG_NUM_LEN];
        char    tmpBuf[256];
        char    seqNo=1;
        IxpcQMsgType    rxIxpcMsg;
        MMLReqMsgType   *rxMmlReqMsg;

        int delNum;

        memcpy ((void*)&rxIxpcMsg, arg, sizeof(IxpcQMsgType));
        rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg.body;

        strcpy (msgType,  rxMmlReqMsg->head.para[0].paraVal);
        strcpy (msgNum, rxMmlReqMsg->head.para[1].paraVal);

        for (i=0; i<strlen(msgType); i++) // 결과 출력시 대문자로 출력하기 위해
                msgType[i] = toupper (msgType[i]);

        sprintf (resHead,"    MSGTYPE = %s\n    MSGNUM = %s\n", msgType, msgNum);
        strcpy (resBuf, resHead);
////////////////////////////
	for( i=0; i<MAX_INH_MSG_CNT ; i++){
		if( !strcasecmp(msgType,inhMsg->msgType[i]) && !strcasecmp(msgNum,inhMsg->msgNum[i])){
			if( inhMsg->msgFlag[i] == MSG_ALLOW ){
				delNum=i;
				noMsgFlag=1;
			} else{
				noMsgFlag=0;
			} 
			break;
		}
	}
	if ( i == MAX_INH_MSG_CNT ){
		sprintf(tmpBuf,"    RESULT = FAIL\n    REASON = NOT_FOUND_%s_MSG(%s)\n",msgType, msgNum);
		strcat (resBuf, tmpBuf);
		cond_txMMLResult (&rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
		return NULL;
	} else {
		if( noMsgFlag == 0 ){
			sprintf(tmpBuf,"    RESULT = FAIL\n    REASON = %s_MSG(%s) is Already Inhibited\n",msgType, msgNum);
			strcat (resBuf, tmpBuf);
			cond_txMMLResult (&rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
			return NULL;
		}else{
			inhMsg->msgFlag[delNum] = MSG_INHIB;
		}
	}
///////////////////////////

    Write_InhMsg_Info(inhMsg);

    // cond_srch_log_his_searchLogFile에서 일정크기로 나누어 보낼때에는 contFlag를 1로 setting해서
    //      보냈고, 마지막(END) 메시지는 여기서 보낸다.
    //
    sprintf(tmpBuf,"    RESULT = SUCCESS\n    REASON = ALLOW_MSG_%s(%s) --> INH_MSG_%s(%s) CHANGE OK!!\n", msgType, msgNum, msgType, msgNum);
    strcat (resBuf, tmpBuf);
    cond_txMMLResult (&rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);

	return NULL;
} //----- End of cond_mmc_inh_msg -----//
#endif 
