#include "stmd_proto.h"

extern  int     ixpcQid;
extern  char    mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcFlag, trcLogFlag;
extern  OnDemandList  onDEMAND[MAX_ONDEMAND_NUM];
extern  int     sysCnt;
extern  STM_CommStatisticInfo   StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];

extern  short   printTIME [STMD_PERIOD_TYPE_NUM];
extern  short   maskITEM  [STMD_MASK_ITEM_NUM][STMD_PERIOD_TYPE_NUM];

extern  CronList    cronJOB[MAX_CRONJOB_NUM];
extern  char    strITEM[STMD_MASK_ITEM_NUM][14];
extern  MYSQL           sql, *conn;
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
char    ipafBuf[4096*4] ,ipafErrBuf[4096], ipafHead[1024];
char    resBuf[4096], resHead[4096], resTmp[4096];
extern SCE_t    g_stSCE[2];
extern int		SCE_CNT;

extern char	rsFname[32];
extern RuleSetList	g_stSCERule[MAX_SCE_NUM];
extern int	g_ruleIdBuf[MAX_RULE_NUM];
extern int	g_ruleItemCnt;

extern RuleEntryList	g_stSCEEntry[MAX_SCE_NUM];
//extern PDSN_LIST		g_stPdsn[2];
extern PDSN_LIST		g_stPdsn;
extern SVC_ALM			g_stSvcAlm; // logon/logout/traffic min/rate 값을 저장. 
extern SVC_VAL			g_stSvcVal; // 이전/현재 logon/logout/traffic 값을 저장. 


int dWrite_PdsnIpConf(void)
{
	FILE        *fp;    
	char        szFName[256];
	char        *env;
	int			i = 0;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf, "[dWrite_PdsnIpConf] getenv error! \n" );
        trclib_writeLogErr(FL, trcBuf);
		return -1;
	}
	sprintf (szFName, "%s/%s", env, PDSN_IP_CTRL_FILE);

	if ((fp = fopen(szFName, "w"))==NULL){
		sprintf(trcBuf, "[dWrite_PdsnIpConf] %s write failed\n", szFName);
        trclib_writeLogErr(FL, trcBuf);
		return -1;
	}   

	fprintf(fp, "@START\n");
	for( i = 0; i < g_stPdsn.pdsnCnt ; i++ )
	{
		fprintf(fp, "   %-16s %-64s\n", g_stPdsn.stItem[i].ip, g_stPdsn.stItem[i].desc);
	}
	fprintf(fp, "@END");

	fclose(fp);
	return 0;       
} 

void dLog_PdsnIpConf(void)
{
	//  dAppLog(level, "@ Overload Call Control: %s", (gCpsOvldCtrl.over_flag)? "Yes": "No");
	int i = 0;

	for( i = 0; i < g_stPdsn.pdsnCnt; i++ )
	{
		sprintf(trcBuf,"[%d] %-16s %-64s\n", i, g_stPdsn.stItem[i].ip, g_stPdsn.stItem[i].desc);
		trclib_writeLogErr(FL, trcBuf);
	}
}


int dLoad_PdsnIpConf(void)
{
	FILE        *fa;
	char        szBuffer[1024], szFName[256];
	int         cnt = 0;
	char		IP[16], DESC[64];
	char        *env;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf, "[dLoad_PdsnIpConf] getenv error! \n");
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}
	sprintf (szFName, "%s/%s", env, PDSN_IP_CTRL_FILE);

	if ((fa=fopen(szFName, "r")) == NULL) {
		sprintf(trcBuf, "[dLoad_PdsnIpConf] %s FILE NOT FOUND \n", szFName );
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	fseek(fa, 0, SEEK_SET);
	cnt = 0;
	while (fgets( szBuffer, 1024, fa ) != NULL)
	{                                                                                   
		memset(IP, 0x00, sizeof(IP));
		memset(DESC, 0x00, sizeof(DESC));
		memset(szBuffer, 0x00, sizeof(szBuffer));

		if(szBuffer[0] == '@')
			continue;

		if( sscanf( &szBuffer[0], "%s %s", IP, DESC) == 2 ) {
			sprintf(g_stPdsn.stItem[cnt].ip, "%s", IP);
			sprintf(g_stPdsn.stItem[cnt].desc, "%s", DESC);
		}
		else {
			sprintf(trcBuf, "[dLoad_SvcAlmConf] %s FILE FORMAT ERROR! \n", szFName );
			trclib_writeLogErr(FL, trcBuf);
			fclose(fa);
			return -1;
		}
		cnt++;
	}

	fclose(fa);
	dLog_SvcAlmConf();

	return 0;
}

int dWrite_SvcAlmConf(void)
{
	FILE        *fp;    
	char        szFName[256];
	char        *env;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf, "[dWrite_SvcAlmConf] getenv error! \n" );
        trclib_writeLogErr(FL, trcBuf);
		return -1;
	}
	sprintf (szFName, "%s/%s", env, SVC_ALM_CTRL_FILE);

	if ((fp = fopen(szFName, "w"))==NULL){
		sprintf(trcBuf, "[dWrite_SvcAlmConf] %s write failed\n", szFName);
        trclib_writeLogErr(FL, trcBuf);
		return -1;
	}   

	fprintf(fp, "@START\n");

	fprintf(fp, "@ %-10s %-10s\n", "LOGON", "RATE");
	fprintf(fp, "   %-10d %-10d\n", g_stSvcAlm.logon_min, g_stSvcAlm.logon_rate);

	fprintf(fp, "@ %-10s %-10s\n", "LOGOUT", "RATE");
	fprintf(fp, "   %-10d %-10d\n", g_stSvcAlm.logout_min, g_stSvcAlm.logout_rate);

	fprintf(fp, "@ %-10s %-10s\n", "THROUGHPUT", "RATE");
	fprintf(fp, "   %-10d %-10d\n", g_stSvcAlm.traffic_min, g_stSvcAlm.traffic_rate);

	fprintf(fp, "@END");
	fclose(fp);
	return 0;       
} 

int dInit_SvcAlmConf(void)
{
	FILE        *fa;
	char        szBuffer[1024], szFName[256];
	int         dMin=0, dRate=0, cnt = 0;
	char        *env;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf, "[dInit_SvcAlmConf] getenv error! \n" );
        trclib_writeLogErr(FL, trcBuf);
		return -1;
	}
	sprintf (szFName, "%s/%s", env, SVC_ALM_CTRL_FILE);

	if ((fa=fopen(szFName, "r")) == NULL) {
		sprintf(trcBuf, "[dInit_SvcAlmConf] %s FILE NOT FOUND! \n", szFName );
        trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	fseek(fa, 0, SEEK_SET);
	cnt = 0;
	while (fgets( szBuffer, 1024, fa ) != NULL)
	{                                                                                   
		dMin = 0; dRate = 0;

		if(szBuffer[0] == '@')
			continue;

		switch( cnt )
		{
			case 0 :
				if( sscanf( &szBuffer[0], "%d %d", &dMin, &dRate) == 2 ) {
					g_stSvcAlm.logon_min  = dMin;
					g_stSvcAlm.logon_rate = dRate;
					g_stSvcVal.logon_A[0] = dMin; // Previous Logon 값 초기화, SCE A 
					g_stSvcVal.logon_B[0] = dMin; // Previous Logon 값 초기화, SCE B 
//					g_stSvcVal.logonSum[0] = dMin;
				}
				else {
					sprintf(trcBuf, "[dInit_SvcAlmConf] %s FILE FORMAT ERROR! \n", szFName );
					trclib_writeLogErr(FL, trcBuf);
					fclose(fa);
					return -1;
				}
				cnt++;
				break;
			case 1 :
				if( sscanf( &szBuffer[0], "%d %d", &dMin, &dRate) == 2 ) {
					g_stSvcAlm.logout_min  = dMin;
					g_stSvcAlm.logout_rate = dRate;
					g_stSvcVal.logout_A[0] = dMin; // Previous Logout 값 초기화, SCE A 
					g_stSvcVal.logout_B[0] = dMin; // Previous Logout 값 초기화, SCE B 
//					g_stSvcVal.logoutSum[0] = dMin;
				}
				else {
					sprintf(trcBuf, "[dInit_SvcAlmConf] %s FILE FORMAT ERROR! \n", szFName );
					trclib_writeLogErr(FL, trcBuf);
					fclose(fa);
					return -1;
				}
				cnt++;
				break;
			case 2 :
				if( sscanf( &szBuffer[0], "%d %d", &dMin, &dRate) == 2 ) {
					g_stSvcAlm.traffic_min  = dMin;
					g_stSvcAlm.traffic_rate = dRate;
					g_stSvcVal.traffic_A[0] = dMin; // Previous traffic 값 초기화, SCE A 
					g_stSvcVal.traffic_B[0] = dMin; // Previous traffic 값 초기화, SCE B 
//					g_stSvcVal.trafficSum[0] = dMin;
				}
				else {
					sprintf(trcBuf, "[dInit_SvcAlmConf] %s FILE FORMAT ERROR! \n", szFName );
					trclib_writeLogErr(FL, trcBuf);
					fclose(fa);
					return -1;
				}
				cnt++;
				break;
		}
		memset(szBuffer, 0x00, sizeof(szBuffer));
	}

	fclose(fa);
	dLog_SvcAlmConf();

	return 0;
}

int dLoad_SvcAlmConf(void)
{
	FILE        *fa;
	char        szBuffer[1024], szFName[256];
	int         dMin=0, dRate=0, cnt = 0;
	char        *env;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf, "[dLoad_SvcAlmConf] getenv error! \n");
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}
	sprintf (szFName, "%s/%s", env, SVC_ALM_CTRL_FILE);

	if ((fa=fopen(szFName, "r")) == NULL) {
		sprintf(trcBuf, "[dLoad_SvcAlmConf] %s FILE NOT FOUND \n", szFName );
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	fseek(fa, 0, SEEK_SET);
	cnt = 0;
	while (fgets( szBuffer, 1024, fa ) != NULL)
	{                                                                                   
		dMin = 0; dRate = 0;
		memset(szBuffer, 0x00, sizeof(szBuffer));

		if(szBuffer[0] == '@')
			continue;

		switch( cnt )
		{
			case 0 :
				if( sscanf( &szBuffer[0], "%d %d", &dMin, &dRate) == 2 ) {
					g_stSvcAlm.logon_min  = dMin;
					g_stSvcAlm.logon_rate = dRate;
				}
				else {
					sprintf(trcBuf, "[dLoad_SvcAlmConf] %s FILE FORMAT ERROR! \n", szFName );
					trclib_writeLogErr(FL, trcBuf);
					fclose(fa);
					return -1;
				}
				cnt++;
				break;
			case 1 :
				if( sscanf( &szBuffer[0], "%d %d", &dMin, &dRate) == 2 ) {
					g_stSvcAlm.logout_min  = dMin;
					g_stSvcAlm.logout_rate = dRate;
				}
				else {
					sprintf(trcBuf, "[dLoad_SvcAlmConf] %s FILE FORMAT ERROR! \n", szFName );
					trclib_writeLogErr(FL, trcBuf);
					fclose(fa);
					return -1;
				}
				cnt++;
				break;
			case 2 :
				if( sscanf( &szBuffer[0], "%d %d", &dMin, &dRate) == 2 ) {
					g_stSvcAlm.traffic_min  = dMin;
					g_stSvcAlm.traffic_rate = dRate;
				}
				else {
					sprintf(trcBuf, "[dLoad_SvcAlmConf] %s FILE FORMAT ERROR! \n", szFName );
					trclib_writeLogErr(FL, trcBuf);
					fclose(fa);
					return -1;
				}
				cnt++;
				break;
		}
	}

	fclose(fa);
	dLog_SvcAlmConf();

	return 0;
}

void dLog_SvcAlmConf(void)
{
	//  dAppLog(level, "@ Overload Call Control: %s", (gCpsOvldCtrl.over_flag)? "Yes": "No");
	sprintf(trcBuf, "@ LOGON MIN : %d\n", g_stSvcAlm.logon_min);
	trclib_writeLogErr(FL, trcBuf);

	sprintf(trcBuf, "@ LOGON RATE: %d\n", g_stSvcAlm.logon_rate);
	trclib_writeLogErr(FL, trcBuf);

	sprintf(trcBuf, "@ LOGOUT MIN : %d\n", g_stSvcAlm.logout_min);
	trclib_writeLogErr(FL, trcBuf);

	sprintf(trcBuf, "@ LOGOUT RATE: %d\n", g_stSvcAlm.logout_rate);
	trclib_writeLogErr(FL, trcBuf);

	sprintf(trcBuf, "@ THROUGHPUT MIN : %d\n", g_stSvcAlm.traffic_min);
	trclib_writeLogErr(FL, trcBuf);

	sprintf(trcBuf, "@ THROUGHPUT RATE: %d\n", g_stSvcAlm.traffic_rate);
	trclib_writeLogErr(FL, trcBuf);
}

int SendAppNoty(int msgQid)
{
	int         qid = msgQid;
	int         txLen = 0;

	GeneralQMsgType     txGenQMsg;
	//2010. 04. 13 by BOMB JJINRI txIxpcMsg-> *txIxpcMsg
	IxpcQMsgType        *txIxpcMsg;

	//2010. 04. 13 by BOMB JJINRI ADD SOURCE
	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));
	//2010. 04. 13 by BOMB JJINRI ADD SOURCE

	txGenQMsg.mtype = MTYPE_PDSN_CONFIG;

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "SCMA");
	strcpy (txIxpcMsg->head.dstAppName, "MMCR");
	txIxpcMsg->head.segFlag = 0;
	txIxpcMsg->head.seqNo = 1;


	//2010. 04. 13 by BOMB JJINRI ADD SOURCE
	txLen = sizeof(txIxpcMsg->head);
	//2010. 04. 13 by BOMB JJINRI ADD SOURCE
	
	/*
	if (memcpy ((void*)txGenQMsg.body, &txLen, txLen) == NULL) {
		sprintf(trcBuf, "memcpy err = %s\n", strerror(errno));
		trclib_writeLogErr(FL,trcBuf);
		return -1;
	}
	*/
	if (msgsnd (qid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf, "[MMCReqBypassSnd] msgsnd error=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr(FL,trcBuf);
		return -1;
	}

	memset(txIxpcMsg->head.dstSysName, 0x00, sizeof(txIxpcMsg->head.dstSysName));
	strcpy (txIxpcMsg->head.dstSysName, "SCMB");
	txIxpcMsg->head.segFlag = 0;
	txIxpcMsg->head.seqNo = 1;

	if (msgsnd (qid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf, "[MMCReqBypassSnd] msgsnd error=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr(FL,trcBuf);
		return -1;
	}

	return 1;
}

/* ADD-PDSN-INFO */ 
int stmd_mmc_add_pdsn_info(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	char	addIP[16] = {0,}, alias[64] = {0,};
	char	query[1024];
	int i = 0, j = 0, seqNo = 0, cnt = 0, dotCnt = 0, ret = 0;
	int	notNull = 0;
	int	addIndex = 0;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "IP")) 
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
				sprintf(addIP, "%s", rxReqMsg->head.para[cnt].paraVal);
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "ALIAS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
				sprintf(alias, "%s", rxReqMsg->head.para[cnt].paraVal);
				notNull = 1; // Alias 있음. 
            }
			else
				notNull = 0; // Alias 없음. 
        }
    }

	// 체크 PDSN 최대 등록 개수 제한 - MAX_PDSN_NUM = 32
	if( g_stPdsn.pdsnCnt == 32 )
	{
		sprintf(resHead,"\n\n    MAX PDSN REGISTER NUMBER LIMIT OVER.!!!\n");
        sprintf(trcBuf,">>> add-pdsn-info fail; err=%s\n", resHead);
        trclib_writeLogErr (FL,trcBuf);
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = %s\n", resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
	}
	else
	{
		// Check aleady exist PDSN IP 
		for( i = 0; i < g_stPdsn.pdsnCnt; i++ )
		{
			if( !strcmp( addIP, g_stPdsn.stItem[i].ip ) )
			{
				sprintf(resHead,"\n\n    Aleady Exist PDSN IP[%s].!!!\n", addIP);
				sprintf(trcBuf,">>> add-pdsn-info fail; err=%s\n", resHead);
				trclib_writeLogErr (FL,trcBuf);
				sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = %s\n", resHead);
				stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
				return -1;
			}
		}

		addIndex = g_stPdsn.pdsnCnt; // 새로 추가 될 PDSN 의 배열 인덱스 값. 

		// IP 적법성 체크 
		for( j = 0; j < strlen(addIP); j++ )
		{
			if( addIP[j] == '.' )
				dotCnt++;
		}

		if( dotCnt != 3 )
		{
			sprintf(resHead,"\n\n    Illegal IP ADDRESS !!! [%s]\n", addIP);
			sprintf(trcBuf,">>> add-pdsn-info fail; err=%s\n", resHead);
			trclib_writeLogErr (FL,trcBuf);
			sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = %s\n", resHead);
			stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
			return -1;
		}
		else
		{
			g_stPdsn.pdsnCnt++;
			sprintf(g_stPdsn.stItem[addIndex].ip, "%s", addIP);
			if( notNull == 1 )
				sprintf(g_stPdsn.stItem[addIndex].desc,"%s", alias);
		}
	}

	dWrite_PdsnIpConf(); // Config 파일의 내용을 현재 mmc 파라메터 값으로 변경한다. 

	// DB INSERT PDSN IP
	if( notNull == 1 )
	{
		sprintf(query,"INSERT INTO ip_code_tbl VALUES (1,'%s','%s')", 
				g_stPdsn.stItem[addIndex].ip, g_stPdsn.stItem[addIndex].desc);
	}
	else
	{
		sprintf(query,"INSERT INTO ip_code_tbl VALUES (1,'%s','-')", g_stPdsn.stItem[addIndex].ip);
	}

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        strcpy (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = DB INSERT FAIL\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
		g_stPdsn.pdsnCnt--;
		memset(g_stPdsn.stItem[addIndex].ip, 0x00, sizeof(g_stPdsn.stItem[addIndex].ip));
		memset(g_stPdsn.stItem[addIndex].desc, 0x00, sizeof(g_stPdsn.stItem[addIndex].desc));
		dWrite_PdsnIpConf(); // DB Insert 실패 때문에 Config 파일의 내용을 추가 이전 상태로 원복한다. 
        return -1;
    }


	// RCOPY 로 SCMA, SCMB로 PDSN.conf 파일을 복사한다. 
	system("/usr/bin/rcp /DSC/DATA/PDSN.conf SCMA:/DSC/NEW/DATA/PDSN.conf");
	system("/usr/bin/rcp /DSC/DATA/PDSN.conf SCMB:/DSC/NEW/DATA/PDSN.conf");

	usleep(10000);

	// Noty Sync PDSN.conf 파일  To RLEG 
	ret = SendAppNoty(ixpcQid);
	if( ret < 0 )
	{
        sprintf(resHead,"MESSAGE QUEUE SEND NOTYFICATION FAIL TO RLEG !!!\n");
		sprintf(trcBuf,">>> add-pdsn-info fail; err=%s\n", resHead);
		trclib_writeLogErr (FL,trcBuf);
		sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = %s\n", resHead);
		stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
		g_stPdsn.pdsnCnt--;
		memset(g_stPdsn.stItem[addIndex].ip, 0x00, sizeof(g_stPdsn.stItem[addIndex].ip));
		memset(g_stPdsn.stItem[addIndex].desc, 0x00, sizeof(g_stPdsn.stItem[addIndex].desc));
		dWrite_PdsnIpConf(); // RLEG와의 SYNC 실패 때문에 Config 파일의 내용을 추가 이전 상태로 원복한다. 
        return -1;
	}


	sprintf(resHead,"\n\n    ==============================================\n");
	sprintf(resBuf,"%s", resHead);
	sprintf(resHead,"    NEW IP         ALIAS\n");
	strcat(resBuf, resHead);
	sprintf(resHead,"    ==============================================\n");
	strcat(resBuf, resHead);
	sprintf(resHead,"    %-16s %-64s\n", g_stPdsn.stItem[addIndex].ip, g_stPdsn.stItem[addIndex].desc);
	strcat(resBuf, resHead);
	sprintf(resHead,"    ==============================================\n");
	strcat(resBuf, resHead);

	stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

	return 0;
}

/* DEL-PDSN-INFO */ 
int stmd_mmc_del_pdsn_info(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	char	query[1024];
	char	delIP[16] = {0,}, delAlias[64] = {0,};
	int i = 0, seqNo = 0, cnt = 0, ret = 0;
	int	delIndex = 0;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "IP")) 
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
				sprintf(delIP, "%s", rxReqMsg->head.para[cnt].paraVal);
            }
        }
    }

	// Find Del PDSN IP 
	for( i = 0; i < g_stPdsn.pdsnCnt; i++ )
	{
		if( !strcmp( delIP, g_stPdsn.stItem[i].ip ) )
		{
			delIndex = i;
			break;
		}
	}

	if( i == g_stPdsn.pdsnCnt )
	{
		sprintf(resHead,"\n\n    NOT EXIST IP ADDRESS !!! [%s]\n", delIP);
		sprintf(trcBuf,">>> del-pdsn-info fail; err=%s\n", resHead);
		trclib_writeLogErr (FL,trcBuf);
		sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = %s\n", resHead);
		stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
		return -1;
	}

	delIndex = i; // 삭제 될 PDSN 의 배열 인덱스 값. 
	sprintf(delAlias, "%s", g_stPdsn.stItem[delIndex].desc);

	// DB DELETE PDSN IP
	sprintf(query,"DELETE FROM ip_code_tbl WHERE type = 1 and code = '%s'", g_stPdsn.stItem[delIndex].ip);

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
		trclib_writeLogErr (FL,trcBuf);
		sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = DB DELETE FAIL\n");
		stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
		return -1;
	}

	// PDSN 배열 정리 
	for( i = delIndex; i < g_stPdsn.pdsnCnt-1; i++ )
	{
		memset(g_stPdsn.stItem[i].ip, 0x00, sizeof(g_stPdsn.stItem[i].ip));		
		memset(g_stPdsn.stItem[i].desc, 0x00, sizeof(g_stPdsn.stItem[i].desc));		
		sprintf(g_stPdsn.stItem[i].ip, "%s", g_stPdsn.stItem[i+1].ip);
		sprintf(g_stPdsn.stItem[i].desc, "%s", g_stPdsn.stItem[i+1].desc);
	}
	memset(g_stPdsn.stItem[i].ip, 0x00, sizeof(g_stPdsn.stItem[i].ip));		
	memset(g_stPdsn.stItem[i].desc, 0x00, sizeof(g_stPdsn.stItem[i].desc));		

	g_stPdsn.pdsnCnt--;

	dWrite_PdsnIpConf(); // Config 파일의 내용을 현재 mmc 파라메터 값으로 변경한다. 

	// RCOPY 로 SCMA, SCMB로 PDSN.conf 파일을 복사한다. 
	system("/usr/bin/rcp /DSC/DATA/PDSN.conf SCMA:/DSC/NEW/DATA/PDSN.conf");
	system("/usr/bin/rcp /DSC/DATA/PDSN.conf SCMB:/DSC/NEW/DATA/PDSN.conf");

	usleep(10000);

	// Noty Sync PDSN.conf 파일  To RLEG 
	ret = SendAppNoty(ixpcQid);
	if( ret < 0 )
	{
        sprintf(resHead,"MESSAGE QUEUE SEND NOTYFICATION FAIL TO RLEG !!!\n");
		sprintf(trcBuf,">>> add-pdsn-info fail; err=%s\n", resHead);
		trclib_writeLogErr (FL,trcBuf);
		sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = %s\n", resHead);
		stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
		g_stPdsn.pdsnCnt++;
		sprintf(g_stPdsn.stItem[i].ip, "%s", delIP);
		sprintf(g_stPdsn.stItem[i].desc, "%s", delAlias);
        return -1;
	}

	sprintf(resHead,"\n\n    ==============================================\n");
	sprintf(resBuf,"%s", resHead);
	sprintf(resHead,"    DELETE PDSN IP ADDRESS SUCCESS!!!\n");
	strcat(resBuf, resHead);
	sprintf(resHead,"    DELETE IP is [%s]\n",delIP);
	strcat(resBuf, resHead);
	sprintf(resHead,"    ==============================================\n");
	strcat(resBuf, resHead);

	stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

	return 0;
}

/* CHG-PDSN-INFO */ 
int stmd_mmc_chg_pdsn_info(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	char	query[1024];
	char	oldIP[16] = {0,};
	char	newIP[16] = {0,};
	int i = 0, seqNo = 0, cnt = 0, ret = 0;
	int	chgIndex = 0;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "OLD_IP")) 
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
				sprintf(oldIP, "%s", rxReqMsg->head.para[cnt].paraVal);
            }
        }
		else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "NEW_IP")) 
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
				sprintf(newIP, "%s", rxReqMsg->head.para[cnt].paraVal);
            }
        }
    }

	// Find Chg PDSN IP 
	for( i = 0; i < g_stPdsn.pdsnCnt; i++ )
	{
		if( !strcmp( oldIP, g_stPdsn.stItem[i].ip ) )
		{
			chgIndex = i;
			break;
		}
	}

	if( i == MAX_PDSN_NUM )
	{
		sprintf(resHead,"\n\n    NOT EXIST IP ADDRESS !!! [%s]\n", oldIP);
		sprintf(trcBuf,">>> chg-pdsn-info fail; err=%s\n", resHead);
		trclib_writeLogErr (FL,trcBuf);
		sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = %s\n", resHead);
		stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
		return -1;
	}

	chgIndex = i; // 변경될 PDSN 의 배열 인덱스 값. 

	// DB UPDATE PDSN IP
	sprintf(query,"UPDATE ip_code_tbl set code = '%s' WHERE type = 1 and code = '%s'", newIP, oldIP);

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
		trclib_writeLogErr (FL,trcBuf);
		sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = DB UPDATE FAIL\n");
		stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
		return -1;
	}

	// PDSN 배열 정리 
	memset(g_stPdsn.stItem[chgIndex].ip, 0x00, sizeof(g_stPdsn.stItem[chgIndex].ip));		
	sprintf(g_stPdsn.stItem[chgIndex].ip, "%s", newIP);

	dWrite_PdsnIpConf(); // Config 파일의 내용을 현재 mmc 파라메터 값으로 변경한다. 

	// RCOPY 로 SCMA, SCMB로 PDSN.conf 파일을 복사한다. 
	system("/usr/bin/rcp /DSC/DATA/PDSN.conf SCMA:/DSC/NEW/DATA/PDSN.conf");
	system("/usr/bin/rcp /DSC/DATA/PDSN.conf SCMB:/DSC/NEW/DATA/PDSN.conf");

	usleep(10000);

	// Noty Sync PDSN.conf 파일  To RLEG 
	ret = SendAppNoty(ixpcQid);
	if( ret < 0 )
	{
        sprintf(resHead,"MESSAGE QUEUE SEND NOTYFICATION FAIL TO RLEG !!!\n");
		sprintf(trcBuf,">>> chg-pdsn-info fail; err=%s\n", resHead);
		trclib_writeLogErr (FL,trcBuf);
		sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = %s\n", resHead);
		stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
		sprintf(g_stPdsn.stItem[i].ip, "%s", oldIP);
		dWrite_PdsnIpConf(); // Config 파일의 내용을 현재 mmc 파라메터 값으로 변경한다. 
        return -1;
	}

	sprintf(resHead,"\n\n    ==============================================\n");
	sprintf(resBuf,"%s", resHead);
	sprintf(resHead,"    CHANGE PDSN IP ADDRESS SUCCESS!!!\n");
	strcat(resBuf, resHead);
	sprintf(resHead,"    OLD IP [%s] --> NEW IP [%s] \n",oldIP, newIP);
	strcat(resBuf, resHead);
	sprintf(resHead,"    ==============================================\n");
	strcat(resBuf, resHead);

	stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

	return 0;
}



/* DIS-PDSN-INFO */
int stmd_mmc_dis_pdsn_info(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	int seqNo = 0, i = 0;

	sprintf(resHead,"\n\n    ==============================================\n");
	sprintf(resBuf,"%s", resHead);
	sprintf(resHead,"    IP              ALIAS\n");
	strcat(resBuf, resHead);
	sprintf(resHead,"    ==============================================\n");
	strcat(resBuf, resHead);
	for( i = 0; i < g_stPdsn.pdsnCnt; i++ )
	{
		sprintf(resHead,"    %-16s %-64s\n", g_stPdsn.stItem[i].ip, g_stPdsn.stItem[i].desc);
		strcat(resBuf, resHead);
	}
	if( g_stPdsn.pdsnCnt == 0 )
	{
		sprintf(resHead,"    NO PDSN IP LIST.\n");
		strcat(resBuf, resHead);
	}

	sprintf(resHead,"    ==============================================\n");
	strcat(resBuf, resHead);
	stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

	return 0;
}

/* DIS-SVC-ALM */ 
int stmd_mmc_dis_svc_alm(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	int seqNo = 0;

	sprintf(resHead,"\n\n    -- CURRENT SERVICE ALARM SETTING --\n");
	sprintf (resBuf,"%s", resHead);
	sprintf(resHead,"    --------------------------------------");
	strcat(resBuf, resHead);
	sprintf(resHead,"\n    LOGON:      MIN = %d\t    RATE = %d(%%)\n", 
									g_stSvcAlm.logon_min, g_stSvcAlm.logon_rate);
	strcat (resBuf, resHead);
	sprintf(resHead,"\n    LOGOUT:     MIN = %d\t    RATE = %d(%%)\n",
									g_stSvcAlm.logout_min, g_stSvcAlm.logout_rate);
	strcat (resBuf, resHead);
	sprintf(resHead,"\n    THROUGHPUT: MIN = %d\t    RATE = %d(%%)\n", 
									g_stSvcAlm.traffic_min, g_stSvcAlm.traffic_rate);
	strcat (resBuf, resHead);
	sprintf(resHead,"    --------------------------------------\n");
	strcat(resBuf, resHead);
	stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

	return 0;
}


/* SET-SVC-ALM */ 
int stmd_mmc_set_svc_alm(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	char	szSvcName[32] = {0,};
	int i = 0, seqNo = 0, cnt = 0;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SVC"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                sprintf(szSvcName, "%s", rxReqMsg->head.para[cnt].paraVal );
            } 
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "MIN")) // add by jjinri 2009.04.22
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
				if( !strcasecmp(szSvcName, "LOGON") )
				{
					g_stSvcAlm.logon_min = atoi(rxReqMsg->head.para[cnt].paraVal);
				}
				else if( !strcasecmp(szSvcName, "LOGOUT") )
				{
					g_stSvcAlm.logout_min = atoi(rxReqMsg->head.para[cnt].paraVal);
				}
				else if( !strcasecmp(szSvcName, "THROUGHPUT") )
				{
					g_stSvcAlm.traffic_min = atoi(rxReqMsg->head.para[cnt].paraVal);
				}
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "RATE"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
				if( !strcasecmp(szSvcName, "LOGON") )
				{
					g_stSvcAlm.logon_rate = atoi(rxReqMsg->head.para[cnt].paraVal);
				}
				else if( !strcasecmp(szSvcName, "LOGOUT") )
				{
					g_stSvcAlm.logout_rate = atoi(rxReqMsg->head.para[cnt].paraVal);
				}
				else if( !strcasecmp(szSvcName, "THROUGHPUT") )
				{
					g_stSvcAlm.traffic_rate = atoi(rxReqMsg->head.para[cnt].paraVal);
				}
            }
        }
    }

	dWrite_SvcAlmConf(); // Config 파일의 내용을 현재 mmc 파라메터 값으로 변경한다. 

	sprintf(resHead,"\n\n    SVC = %s  Change.\n", szSvcName);
	sprintf (resBuf,"%s", resHead);
	sprintf(resHead,"    --------------------------------------");
	strcat(resBuf, resHead);
	sprintf(resHead,"\n    LOGON:      MIN = %d\t    RATE = %d(%%)\n", 
									g_stSvcAlm.logon_min, g_stSvcAlm.logon_rate);
	strcat (resBuf, resHead);
	sprintf(resHead,"\n    LOGOUT:     MIN = %d\t    RATE = %d(%%)\n",
									g_stSvcAlm.logout_min, g_stSvcAlm.logout_rate);
	strcat (resBuf, resHead);
	sprintf(resHead,"\n    THROUGHPUT: MIN = %d\t    RATE = %d(%%)\n", 
									g_stSvcAlm.traffic_min, g_stSvcAlm.traffic_rate);
	strcat (resBuf, resHead);
	sprintf(resHead,"    --------------------------------------\n");
	strcat(resBuf, resHead);
	stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

	return 0;
}

int doDisFaultStatQuery(IxpcQMsgType *rxIxpcMsg, char *stime, int cnt, char* time_type, char *table_type)
{
    char            query[4096], localHead[1204];
    MYSQL_RES       *result;
    MYSQL_ROW       row;
    int             select_cnt = 0;
    int             rowcnt, seqNo=1;

    sprintf(query, "SELECT * from %s where (stat_date >= '%s' AND stat_date <= "
            "DATE_ADD('%s', INTERVAL %d %s)) ORDER BY stat_date, length(system_name), system_type, system_name",
        table_type , stime, stime, (cnt - 1), time_type); 
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        strcpy (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }
    result = mysql_store_result(conn);

    rowcnt = mysql_num_rows(result);
    sprintf(trcBuf, "Fault rowcnt = %d\n", rowcnt);
    trclib_writeLog(FL, trcBuf);

    strcpy(localHead, resHead);
    sprintf(resTmp, "\n    %-6s  %-8s  %-9s  %-5s  %-5s  %-8s  %-20s\n",
        "TYPE", "SYSTEM", "ITEM", "MINOR", "MAJOR", "CRITICAL", "DATE");
    strcat(localHead, resTmp);
    sprintf(resTmp, "    ========================================================================\n");
    strcat(localHead, resTmp);

    strcpy(resBuf, localHead);

    while ((row = mysql_fetch_row(result)) != NULL ) {
        /*if ((rowcnt % sysCnt) == 0) {
            if ( select_cnt != 0)
                strcpy(resBuf, localHead);
        }*/

        sprintf(resTmp, "    %-6s  %-8s  %-9s  %5s  %5s  %8s  %-s\n", 
            row[0], row[1], "CPU", row[2], row[3], row[4], row[18]);
        strcat(resBuf,resTmp);
        sprintf(resTmp, "    %-6s  %-8s  %-9s  %5s  %5s  %8s\n", 
            row[0], row[1], "MEM", row[5], row[6], row[7]);
        strcat(resBuf,resTmp);
        sprintf(resTmp, "    %-6s  %-8s  %-9s  %5s  %5s  %8s\n", 
            row[0], row[1], "DISK", row[8], row[9], row[10]);
        strcat(resBuf,resTmp);
        sprintf(resTmp, "    %-6s  %-8s  %-9s  %5s  %5s  %8s\n", 
            row[0], row[1], "LAN", row[11], row[12], row[13]);
        strcat(resBuf,resTmp);
        sprintf(resTmp, "    %-6s  %-8s  %-9s  %5s  %5s  %8s\n", 
            row[0], row[1], "PROC", row[14], row[15], row[16]);
        strcat(resBuf,resTmp);

        /*if ( rowcnt == 1) {
            stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 10, 1, seqNo++);
            commlib_microSleep(80000);
            resBuf[0] = 0;
        } else if ((rowcnt % sysCnt) == 1){
            stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 10, 1, seqNo++);
            commlib_microSleep(80000);
            resBuf[0] = 0;
        }*/

        if(strlen(resBuf) > 3500){
            stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 10, 1, seqNo++);
            commlib_microSleep(80000);
            memset(resBuf, 0, 4096);
        }

        rowcnt--;
        select_cnt++;
    }
    mysql_free_result(result);

    stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 10, 1, seqNo++);

    sprintf(trcBuf, "select_cnt = %d\n", select_cnt);
    trclib_writeLog(FL, trcBuf);
    if ( select_cnt == 0 ) {
        strcpy (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = NO EXIST FAULT STATISTICS HISTORY\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);
    }
	return 0;
}

int doDishwFltStatQuery(IxpcQMsgType *rxIxpcMsg, char *stime, int cnt, char* time_type, char *table_type)
{
    char            query[4096];
    MYSQL_RES       *result;
    MYSQL_ROW       row;
    int             select_cnt = 0;
    int             rowcnt, seqNo=1;

    sprintf(query, "SELECT * from %s where (stat_date >= '%s' AND stat_date <= "
            "DATE_ADD('%s', INTERVAL %d %s)) ORDER BY stat_date, system_name",
        table_type , stime, stime, (cnt - 1), time_type); 
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        strcpy (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }
    result = mysql_store_result(conn);

    rowcnt = mysql_num_rows(result);
    sprintf(trcBuf, "hw Flt rowcnt = %d\n", rowcnt);
    trclib_writeLog(FL, trcBuf);

    sprintf(resTmp, "\n    %-6s  %-8s  %-9s  %-5s  %-5s  %-8s  %-4s\n",
        "TYPE", "SYSTEM", "ITEM", "MINOR", "MAJOR", "CRITICAL", "DATE");
    strcat(resHead, resTmp);
    sprintf(resTmp, "    ========================================================================\n");
    strcat(resHead, resTmp);

    strcpy(resBuf, resHead);

    while ((row = mysql_fetch_row(result)) != NULL ) {
/*        if ((rowcnt % sysCnt) == 0) {
            if ( select_cnt != 0)
                strcpy(resBuf, resHead);
        }*/
        sprintf(resTmp, "    %-6s  %-8s  %-9s  %5s  %5s  %8s  %-4s\n", 
            row[0], row[1], "H/W", row[2], row[3], row[4], row[6]);
        strcat(resBuf,resTmp);

        /*if ( rowcnt == 1) {
            stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);
            commlib_microSleep(80000);
            resBuf[0] = 0;
        } else if ((rowcnt % sysCnt) == 1){
            stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 10, 1, seqNo++);
            commlib_microSleep(80000);
            resBuf[0] = 0;
        }*/

        if(strlen(resBuf) > 3500){
            stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 10, 1, seqNo++);
            commlib_microSleep(80000);
            memset(resBuf, 0, 4096);
        }

        rowcnt--;
        select_cnt++;
    }
    mysql_free_result(result);

    stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

    sprintf(trcBuf, "select_cnt = %d\n", select_cnt);
    trclib_writeLog(FL, trcBuf);
    if ( select_cnt == 0 ) {
        strcpy (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = NO EXIST DSC FAULT STATISTICS HISTORY\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);
    }

	return 0;
}

int doDisFaultStatHis( IxpcQMsgType *rxIxpcMsg, int year, int mon, int day, int hour, int min, int cnt)
{
    MMLReqMsgType   *rxReqMsg;
    char            stime[32];

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "5MINUTELY")) {
        sprintf(stime, "%04d-%02d-%02d %02d:%02d", year, mon, day, hour, min);
        doDisFaultStatQuery(rxIxpcMsg, stime, cnt*5, STMD_ADD_MIN, STM_STATISTIC_5MINUTE_FAULT_TBL_NAME);
        //doDishwFltStatQuery(rxIxpcMsg, stime, cnt*5, STMD_ADD_MIN, STM_STATISTIC_5MINUTE_HW_FLT_TBL_NAME);
    } else if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "HOURLY")) {
        sprintf(stime, "%04d-%02d-%02d %02d", year, mon, day, hour);
        doDisFaultStatQuery(rxIxpcMsg, stime, cnt, STMD_ADD_HOUR, STM_STATISTIC_HOUR_FAULT_TBL_NAME);
        //doDishwFltStatQuery(rxIxpcMsg, stime, cnt, STMD_ADD_HOUR, STM_STATISTIC_HOUR_HW_FLT_TBL_NAME);
    } else if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "DAILY")) {
        sprintf(stime, "%04d-%02d-%02d ", year, mon, day);
        doDisFaultStatQuery(rxIxpcMsg, stime, cnt, STMD_ADD_DAY, STM_STATISTIC_DAY_FAULT_TBL_NAME);
        //doDishwFltStatQuery(rxIxpcMsg, stime, cnt, STMD_ADD_DAY, STM_STATISTIC_DAY_HW_FLT_TBL_NAME);
    } else if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "WEEKLY")) {
        sprintf(stime, "%04d-%02d-%02d ", year, mon, day);
        doDisFaultStatQuery(rxIxpcMsg, stime, cnt*7, STMD_ADD_DAY, STM_STATISTIC_WEEK_FAULT_TBL_NAME);
        //doDishwFltStatQuery(rxIxpcMsg, stime, cnt*7, STMD_ADD_DAY, STM_STATISTIC_WEEK_HW_FLT_TBL_NAME);
    } else if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "MONTHLY")) {
        sprintf(stime, "%04d-%02d-01", year, mon);
        doDisFaultStatQuery(rxIxpcMsg, stime, cnt, STMD_ADD_MONTH, STM_STATISTIC_MONTH_FAULT_TBL_NAME);
        //doDishwFltStatQuery(rxIxpcMsg, stime, cnt, STMD_ADD_MONTH, STM_STATISTIC_MONTH_HW_FLT_TBL_NAME);
    }

	return 0;
}

int doDisLoadStatQuery(IxpcQMsgType *rxIxpcMsg, char *stime, int cnt, char* time_type, char *table_type)
{
    char            query[4096];
    MYSQL_RES       *result;
    MYSQL_ROW       row;
    int             select_cnt = 0, seqNo=1;
    char            avr_mem[10], max_mem[10], avr_cpu[10], max_cpu[10];

    sprintf(query, "SELECT * from %s where (stat_date >= '%s' AND stat_date <= "
            "DATE_ADD('%s', INTERVAL %d %s)) ORDER BY stat_date, length(system_name), system_name",
        table_type , stime, stime, (cnt - 1), time_type); 
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        strcpy (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }
    result = mysql_store_result(conn);

    sprintf(resTmp, "\n    SYSTEM    AVR_MEM(%%)  MAX_MEM(%%)  AVR_CPU(%%)  MAX_CPU(%%)  DATE\n");
    strcat(resHead, resTmp);
    sprintf(resTmp, "    =============================================================================\n");
    strcat(resHead, resTmp);

    strcpy(resBuf, resHead);

    while ((row = mysql_fetch_row(result)) != NULL ) {
        strcpy(avr_mem, row[2]);
        sprintf(avr_mem, "%d.%d", atoi(avr_mem)/10, atoi(avr_mem)%10); 
        strcpy(max_mem, row[3]);
        sprintf(max_mem, "%d.%d", atoi(max_mem)/10, atoi(max_mem)%10); 
        strcpy(avr_cpu, row[4]);
        sprintf(avr_cpu, "%d.%d", atoi(avr_cpu)/10, atoi(avr_cpu)%10); 
        strcpy(max_cpu, row[5]);
        sprintf(max_cpu, "%d.%d", atoi(max_cpu)/10, atoi(max_cpu)%10); 

        sprintf(resTmp, "    %-8s  %10s  %10s  %10s  %10s  %s\n", 
            row[1], avr_mem, max_mem, avr_cpu, max_cpu, row[13]);
        strcat(resBuf, resTmp);
        select_cnt++;
    }
    mysql_free_result(result);

    sprintf(trcBuf, "select_cnt = %d\n", select_cnt);
    trclib_writeLog(FL, trcBuf);
    if ( select_cnt == 0 ) {
        strcpy (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = NO EXIST LOAD STATISTICS HISTORY\n");
    } 
    stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

	return 0;
}

int doDisLoadStatHis( IxpcQMsgType *rxIxpcMsg, int year, int mon, int day, int hour, int min, int cnt)
{
    MMLReqMsgType   *rxReqMsg;
    char            stime[32];

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "5MINUTELY")) {
        sprintf(stime, "%04d-%02d-%02d %02d:%02d", year, mon, day, hour, min);
        doDisLoadStatQuery(rxIxpcMsg, stime, cnt*5, STMD_ADD_MIN, STM_STATISTIC_5MINUTE_LOAD_TBL_NAME);
    } else if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "HOURLY")) {
        sprintf(stime, "%04d-%02d-%02d %02d", year, mon, day, hour);
        doDisLoadStatQuery(rxIxpcMsg, stime, cnt, STMD_ADD_HOUR, STM_STATISTIC_HOUR_LOAD_TBL_NAME);
    } else if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "DAILY")) {
        sprintf(stime, "%04d-%02d-%02d ", year, mon, day);
        doDisLoadStatQuery(rxIxpcMsg, stime, cnt, STMD_ADD_DAY, STM_STATISTIC_DAY_LOAD_TBL_NAME);
    } else if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "WEEKLY")) {
        sprintf(stime, "%04d-%02d-%02d ", year, mon, day);
        doDisLoadStatQuery(rxIxpcMsg, stime, cnt*7, STMD_ADD_DAY, STM_STATISTIC_WEEK_LOAD_TBL_NAME);
    } else if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "MONTHLY")) {
        sprintf(stime, "%04d-%02d-01", year, mon);
        doDisLoadStatQuery(rxIxpcMsg, stime, cnt, STMD_ADD_MONTH, STM_STATISTIC_MONTH_LOAD_TBL_NAME);
    }

	return 0;
}

int doDisDbStatQuery(IxpcQMsgType *rxIxpcMsg, char *stime, char* etime, int type, char *table_type)
{
    int rnt;
    char tmpMsg[1024];
    memset(tmpMsg,0,sizeof(tmpMsg));
    
    rnt = makeMsgDb(MMCJOB, 0, 0, table_type, tmpMsg, stime, etime, NULL, rxIxpcMsg);

    if(rnt == -1)
    {
        strcpy (tmpMsg,"      DSC  RESULT = FAIL\n      FAIL REASON = DB SELECT FAIL\n");
        stmd_txMMLResult (rxIxpcMsg, tmpMsg, -1, 0, 0, 0, 0);
        
        return -1;
    }
    else if(rnt == -2)
    {
        strcpy (tmpMsg,"      DSC  RESULT = FAIL\n      FAIL REASON = NO EXIST DB STATISTICS HISTORY\n");
        stmd_txMMLResult (rxIxpcMsg, tmpMsg, -1, 0, 0, 0, 0);
        return -1;
    }
    
    return 1;   
}

int doDisDbStatHis(IxpcQMsgType *rxIxpcMsg, int year, int mon, int day, int hour, int min, int cnt)
{
    MMLReqMsgType   *rxReqMsg;
    char            stime[32];
    char            etime[32];
    
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "5MINUTELY")) 
    {
        sprintf(stime, "%04d-%02d-%02d %02d:%02d", year, mon, day, hour, min);
        sprintf(etime, "%s", get_select_endtime(STMD_MIN, year, mon, day, hour, min,cnt));
//printf("%s <-> %s\n",stime, etime);
        //doDisDbStatQuery(rxIxpcMsg, stime, etime, STMD_MIN,STM_STATISTIC_5MINUTE_DB_TBL_NAME);
    } 
    else if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "HOURLY")) 
    {
        //sprintf(stime, "%04d-%02d-%02d %02d", year, mon, day, hour);
        sprintf(stime, "%04d-%02d-%02d %02d:00", year, mon, day, hour);
        sprintf(etime, "%s", get_select_endtime(STMD_HOUR, year, mon, day, hour, min,cnt));
//printf("%s <-> %s\n",stime, etime);
        //doDisDbStatQuery(rxIxpcMsg, stime, etime, STMD_HOUR, STM_STATISTIC_HOUR_DB_TBL_NAME);
    } 
    else if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "DAILY")) 
    {
        //sprintf(stime, "%04d-%02d-%02d ", year, mon, day);
        sprintf(stime, "%04d-%02d-%02d 00:00", year, mon, day);
        sprintf(etime, "%s", get_select_endtime(STMD_DAY, year, mon, day, hour, min,cnt));
//printf("%s <-> %s\n",stime, etime);
        //doDisDbStatQuery(rxIxpcMsg, stime, etime, STMD_DAY, STM_STATISTIC_DAY_DB_TBL_NAME);
    } 
    else if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "WEEKLY")) 
    {
        //sprintf(stime, "%04d-%02d-%02d ", year, mon, day);
        sprintf(stime, "%04d-%02d-%02d 00:00", year, mon, day);
        sprintf(etime, "%s", get_select_endtime(STMD_WEEK, year, mon, day, hour, min,cnt));
//printf("%s <-> %s\n",stime, etime);
        //doDisDbStatQuery(rxIxpcMsg, stime, etime, STMD_WEEK, STM_STATISTIC_DAY_DB_TBL_NAME);
    } 
    else if (!strcasecmp(rxReqMsg->head.para[1].paraVal, "MONTHLY")) 
    {
        //sprintf(stime, "%04d-%02d-01", year, mon);
        sprintf(stime, "%04d-%02d-01", year, mon);
        sprintf(etime, "%s", get_select_endtime(STMD_MONTH, year, mon, day, hour, min,cnt));
//printf("%s <-> %s\n",stime, etime);
        //doDisDbStatQuery(rxIxpcMsg, stime, etime, STMD_MONTH, STM_STATISTIC_MONTH_DB_TBL_NAME);
    }
    
    return 1;
}


int stmd_mmc_dis_stat_his(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             i;
    int             stat_count, seqNo=1;
    int             syear, smon, sday, shour, smin;
    char            stime[32];

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    for ( i = 0 ; i < strlen(rxReqMsg->head.para[0].paraVal) ; i++) // 대문자 출력
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);

    for ( i = 0 ; i < strlen(rxReqMsg->head.para[1].paraVal) ; i++)
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);

    if (!strcasecmp(rxReqMsg->head.para[3].paraVal,""))
        stat_count = 1; // 기본값으로 1로 구성한다.
    else
        stat_count = atoi(rxReqMsg->head.para[3].paraVal);

    sprintf(resHead,"    STAT_ITEM    = %s\n    START_TIME   = %s\n    STAT_COUNT   = %d\n",
        rxReqMsg->head.para[0].paraVal, rxReqMsg->head.para[2].paraVal, stat_count);
    sprintf (resBuf,"%s", resHead);

    if (stmd_checkParaTimeValue(rxReqMsg->head.para[2].paraVal,&syear,&smon,&sday,&shour,&smin) < 0 ) {
        strcat (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = INVALID START TIME VALUE\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return 1;
    }
    sprintf(stime, "%04d-%02d-%02d %02d:%02d", syear, smon, sday, shour, smin);
    // start_time이 현재 시간 보다 크면 error 처리
    if (strcasecmp(stime, get_select_time(STMD_MIN)) >= 0) {
        strcat (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = INVALID TIME RANGE\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }
    if (!strcasecmp(rxReqMsg->head.para[0].paraVal, "LOAD"))
        doDisLoadStatHis(rxIxpcMsg, syear, smon, sday, shour, smin, stat_count);
    else if (!strcasecmp(rxReqMsg->head.para[0].paraVal, "FAULT"))
        doDisFaultStatHis(rxIxpcMsg, syear, smon, sday, shour, smin, stat_count);
    else if(!strcasecmp(rxReqMsg->head.para[0].paraVal, "DB"))
    {
         doDisDbStatHis(rxIxpcMsg, syear, smon, sday, shour, smin, stat_count);
    }
   
    return 1;
}

// CANCEL_EXE_CMD 명령을 받았을 때  해당 리스트만 없앤다
int stmd_mmc_canc_exe_cmd(IxpcQMsgType *rxIxpcMsg)
{
    int     i, seqNo=1;
    MMLReqMsgType   *rxReqMsg;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    sprintf(trcBuf, " cmdId = %s\n", rxReqMsg->head.para[0].paraVal);
    trclib_writeLog(FL, trcBuf);

    for ( i=0; i<MAX_ONDEMAND_NUM; i++){
        if ( onDEMAND[i].statisticsType != NOT_REGISTERED &&
            onDEMAND[i].cmdId == atoi(rxReqMsg->head.para[0].paraVal) ) {
            onDEMAND[i].statisticsType = NOT_REGISTERED;
            sprintf(trcBuf, "stat-cmd-canc : %d\n", atoi(rxReqMsg->head.para[0].paraVal));
            trclib_writeLog(FL, trcBuf);
            break;
        }
    }
    strcpy(resBuf, "\n    DSC  RESULT = SUCCESS\n");
    stmd_txMMLResult(rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

	return 0;
}

/* STAT-LOAD :: changed by sukhee, 03.03.24 */
int stmd_mmc_stat_load(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     list;
    int     i, cnt, diffTime, seqNo=1;
    short   mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                sprintf(onDEMAND[list].svcName ,"%s", rxReqMsg->head.para[cnt].paraVal );
            } else {
                //printf("jean========stmd_mmc_stat_load no sys name\n");
                sprintf(onDEMAND[list].svcName ,"%s", "ALL" );
            }
        }
// disable        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STM"))
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) // add by jjinri 2009.04.22
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal);
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;
    onDEMAND[list].statisticsType = STMD_LOAD;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;
    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));

        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        sprintf (resBuf,"%s", resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    COUNT       = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), \
			onDEMAND[list].period, onDEMAND[list].count); 
        /*sprintf(resHead,"    STIME    = %s\n    PRD      = %d\n    CNT      = %d\n",
            onDEMAND[list].measureTime, onDEMAND[list].period, onDEMAND[list].count);*/
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, \
						((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);
    }
    else 
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandLoad(list);
    }

	return 0;
}

////////////
//jean load srch
#if 0
int stmd_mmc_srch_stat_load(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    int   tot_row[10]={0,};
    char  key_time[33];

    char        itemRes[10][10];
    int         i,j,k;
//    char *title[]={"CPU","MEM","DISK","QUEUE","SESS"};
    char title[4][16]={"CPU","MEM","DISK","QUEUE"};
    int row_index;
    char SysName[5][8];
    int realSysCnt =0;
    int realItemCnt =0;

    int         select_cnt = 0, snd_cnt,tcnt=0;
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for(i=0; i<sysCnt; i++ ){
        sprintf(SysName[i], "%s",StatisticSystemInfo[i].sysName);
        realSysCnt++;
    }
    sprintf(SysName[realSysCnt], "%s", "TAPA");realSysCnt++;
    sprintf(SysName[realSysCnt],"%s", "TAPB");realSysCnt++;
    sprintf(SysName[realSysCnt],"%s", "SCEA");realSysCnt++;
    sprintf(SysName[realSysCnt],"%s", "SCEB");realSysCnt++;

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        sprintf(table_name, "%s", STM_STATISTIC_5MINUTE_LOAD_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        sprintf(table_name, "%s", STM_STATISTIC_HOUR_LOAD_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        sprintf(table_name, "%s", STM_STATISTIC_DAY_LOAD_TBL_NAME);
    }
    else
    {
        sprintf(table_name, "%s", STM_STATISTIC_MONTH_LOAD_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    sprintf(cmdName,"%s", "srch-stat-load");

    sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
        rxReqMsg->head.para[1].paraVal,stime,etime);
    sprintf(resTmp, "    ==================================================\n");
    strcat(ipafHead, resTmp);
    sprintf(resTmp, "    STATTIME        SYSTEM    ITEM    AVG(%%)    MAX(%%)\n");
    strcat(ipafHead, resTmp);
    sprintf(resTmp, "    ==================================================\n");
    strcat(ipafHead, resTmp);

    strcat(ipafBuf, ipafHead);

    snd_cnt=1;
    for(i=0; i<realSysCnt; i++){ 
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
                continue;
        }

        if(!strcasecmp(SysName[i], "DSCM"))
            realItemCnt=4;
        else if(!strcasecmp(SysName[i], "TAPA") || !strcasecmp(SysName[i], "TAPB"))
            realItemCnt=2;
		else if(!strcasecmp(SysName[i], "SCEA") || !strcasecmp(SysName[i], "SCEB"))
            realItemCnt=3;
        else
            realItemCnt=4;

///////////////////////////

        sprintf(query, "SELECT "
                " avr_cpu0, max_cpu0," 
                " avr_memory, max_memory," 
                " avr_disk, max_disk," 
                " avr_msgQ, max_msgQ," 
//                " avr_sess, max_sess, stat_date" 
				" stat_date "
                " from %s "
                " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' ORDER BY stat_date ",
            table_name, SysName[i], stime,etime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(strlen(ipafBuf) > 2500){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                snd_cnt++;
                memset (ipafBuf, 0x00, sizeof(ipafBuf));

                strcat(ipafBuf, ipafHead);
            }

            if(strlen(row[8]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[8]+5,14);
            }

            for(k=0;k<8;k=k+2){
                tot_row[k]+=atoi(row[k]);
                if( tot_row[k+1] < atoi(row[k+1]) ){
                    tot_row[k+1] = atoi(row[k+1]);
                }
            }


            for(j=0;j<8;j++){
                sprintf(itemRes[j], "%s", row[j]);
                sprintf(itemRes[j], "%d.%d", atoi(itemRes[j])/8, atoi(itemRes[j])%8); 
            }

            row_index = 0;
            for(j=0;j<realItemCnt;j++){
                if (j==0){
                    sprintf(resTmp, "%3s %-14s  %-9s %-5s %8s %9s\n", 
                        "", key_time,SysName[i], title[j], itemRes[row_index],itemRes[row_index+1]);
                    strcat(ipafBuf, resTmp);
                } else {
                    sprintf(resTmp, "%3s %-14s  %-9s %-5s %8s %9s\n", 
                        "","", "", title[j], itemRes[row_index],itemRes[row_index+1]);
                    strcat(ipafBuf, resTmp);
                }

                row_index += 2;
            }
            sprintf(resTmp, "    --------------------------------------------------\n");
            strcat(ipafBuf, resTmp);


            select_cnt++;
            tcnt++;

        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(strlen(ipafBuf) > 2500){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                snd_cnt++;

                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }

            for(k=0;k<8;k=k+2){
                tot_row[k] = tot_row[k]/select_cnt;
            }

            for(j=0;j<8;j++){
                sprintf(itemRes[j], "%d.%d", tot_row[j]/8, tot_row[j]%8); 
            }

            row_index = 0;
            for(j=0;j<realItemCnt;j++){
                if (j==0){
                    sprintf(resTmp, "%3s %-14s  %-9s %-5s %8s %9s\n", 
                        "", "SUM",SysName[i], title[j], itemRes[row_index],itemRes[row_index+1]);
                    strcat(ipafBuf, resTmp);
                } else {
                    sprintf(resTmp, "%3s %-14s  %-9s %-5s %8s %9s\n", 
                        "","", "", title[j], itemRes[row_index],itemRes[row_index+1]);
                    strcat(ipafBuf, resTmp);
                }

                row_index += 2;
            }
            sprintf(resTmp, "    ==================================================\n");
            strcat(ipafBuf, resTmp);

        }

///////////////////////////
    }

    if (tcnt == 0) {
        //sprintf(resTmp, "      %-8s  %10s  %10s  %10s  %10s\n",
        //  "", "0.0", "0.0", "0.0", "0.0");
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
        sprintf(resTmp, "    ==================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;
}
#endif

int stmd_mmc_stat_rule_ent(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_RULE_ENT;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandRuleEnt(list);
    }

	return 0;
}

int stmd_mmc_stat_rule_set(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_RULE_SET;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandRuleSet(list);
    }

	return 0;
}

int stmd_mmc_stat_sms(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                sprintf(onDEMAND[list].svcName , "%s", rxReqMsg->head.para[cnt].paraVal );
            } else {
                sprintf(onDEMAND[list].svcName , "%s", "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_SMS;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        sprintf (resBuf, "%s", resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandSms(list);
    }

	return 0;
}



int stmd_mmc_stat_logon(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                sprintf(onDEMAND[list].svcName , "%s",rxReqMsg->head.para[cnt].paraVal );
            } else {
                sprintf(onDEMAND[list].svcName ,"%s", "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_LOGON;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        sprintf (resBuf,"%s", resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandLogon(list);
    }

	return 0;
}

int stmd_mmc_stat_leg(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                sprintf(onDEMAND[list].svcName ,"%s", rxReqMsg->head.para[cnt].paraVal );
            } else {
                sprintf(onDEMAND[list].svcName ,"%s", "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_LEG;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        sprintf (resBuf, "%s",resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandLeg(list);
    }

	return 0;
}

/** 2010.08.23 stat-flow */
int stmd_mmc_stat_flow(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                sprintf(onDEMAND[list].svcName ,"%s", rxReqMsg->head.para[cnt].paraVal );
            } else {
                sprintf(onDEMAND[list].svcName ,"%s", "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_FLOW;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        sprintf (resBuf, "%s",resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandFlow(list);
    }

	return 0;
}

int stmd_mmc_stat_link(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list = 0;
    int             i = 0, cnt =0, diffTime = 0, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                sprintf(onDEMAND[list].svcName , "%s",rxReqMsg->head.para[cnt].paraVal );
            } else {
                sprintf(onDEMAND[list].svcName ,"%s", "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_LINK;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        sprintf (resBuf, "%s", resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandLink(list);
    }

	return 0;
}

#ifdef DELAY
int stmd_mmc_stat_delay2(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }

    // Default setting
    diffTime = 0;
    mprd = 0;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                sprintf(onDEMAND[list].svcName , "%s", rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) 
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
#if 0
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
////                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
				// 파라메터 입력값에 관계없이 mprd = 1로 고정 . 
               	mprd = 1;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
////                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
				// 파라메터 입력값에 관계없이 mprd = 1로 고정 . 
               	measurecount = 1;
        }
#endif
    }

/////	onDEMAND[list].period = (diffTime / STMD_5MIN_OFFSET);

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_DEL;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600) )
    {
		sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_delay());
        sprintf(resHead,"    MEASURETIME = %s ",onDEMAND[list].measureTime);
        strcpy (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
//////        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ( (mprd*3600) + STMD_5MIN_OFFSET + diffTime +30), 0, seqNo++);
        doOnDemandDelay2(list);
    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 1;
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_delay(0));
        doOnDemandDelay2(list);
    }

	return 0;
}
#else
int stmd_mmc_stat_delay(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                sprintf(onDEMAND[list].svcName , "%s", rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_DEL;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandDelay(list);
    }

	return 0;
}
#endif

#if 0
int stmd_mmc_srch_stat_delay(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;

	char        query[4096] = {0,}, query_head[4096] = {0,}, query_pdsn[1024] = {0,};
	char        cmdName[64] = {0,};
	MYSQL_RES   *result, *res_pd;
	MYSQL_ROW   row, row_pd;

	char  	table_name[30] = {0,};
	char  	stime[33] = {0,}; char etime[33] = {0,};
	int   	i_tmp = 0;
	char  	*env, fname[256] = {0,}, tmp[64] = {0,};
	int   	row_cnt=0;
	char  	key_time[33] = {0,}, last_time[33] = {0,};

	char    itemRes[10][10];
	int     i,j,k, ridx, toResult;
	char title[5][16]={"STATTIME", "SYSTEM", "MIN_USEC", "MAX_USEC","AVG_USEC"};
	int 	row_index;
	char 	SysName[2][8]={"SCEA","SCEB"};
	char 	SysIp[2][16];
	int 	realSysCnt =1;
	int 	realItemCnt =1;
	int		index, select_cnt = 0, snd_cnt, tcnt=0;
	double 	delTotal[2][11] = {{0.0,0.0,0.0},{0.0,0.0,0.0}};
	double	min = 0.0, max = 0.0, avg = 0.0;
	double 	sum = 0.0;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	memset (ipafBuf, 0x00, sizeof(ipafBuf));
	memset (ipafHead, 0x00, sizeof(ipafHead));

    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        sprintf(table_name,"%s", STM_STATISTIC_5MINUTE_DELAY_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        sprintf(table_name, "%s", STM_STATISTIC_HOUR_DELAY_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        sprintf(table_name, "%s", STM_STATISTIC_DAY_DELAY_TBL_NAME);
    }
    else
    {
        sprintf(table_name, "%s", STM_STATISTIC_MONTH_DELAY_TBL_NAME);
    }

	for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
		rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
	}
	for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
		rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
	}

	if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
		memset(stime, 0, 33);
		memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(stime, "-"); strcat(stime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(stime, "-"); strcat(stime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(stime, " "); strcat(stime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(stime, ":"); strcat(stime, resTmp);
					}
				}
			}
		}
	}
	if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
		memset(etime, 0, 33);
		memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(etime, "-"); strcat(etime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(etime, "-"); strcat(etime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(etime, " "); strcat(etime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(etime, ":"); strcat(etime, resTmp);
					}
				}
			}
		}
	}
	sprintf(cmdName, "%s", "srch-stat-delay");

	sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
			rxReqMsg->head.para[1].paraVal,stime,etime);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%3s %15s %8s %-12s %-12s %-12s","", title[0],title[1],title[2],title[3],title[4] );
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);

	strcat(ipafBuf, ipafHead);

	snd_cnt=1;
	for(i=0; i<realSysCnt; i++)
	{ 
		if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
			if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
				continue;
		}

		sprintf(query, "SELECT stat_date, system_name, "
				" IFNULL(ROUND(MIN(min_usec),6), 0.0), IFNULL(ROUND(MAX(max_usec),6), 0.0), "
				" IFNULL(ROUND(AVG(avg_usec),6), 0.0) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by stat_date, system_name order by stat_date, system_name ",table_name
				, SysName[i], stime, etime );

		if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
			sprintf(trcBuf, "query = %s\n", query);
			trclib_writeLog(FL, trcBuf);
		}
		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		select_cnt=0; min = 0.0; max = 0.0; avg = 0.0; sum = 0.0;
		memset(last_time,0x00,sizeof(last_time));
		row_cnt = 0;                                                                                  
		result = mysql_store_result(conn);

		while( (row = mysql_fetch_row(result)) != NULL)
		{
			if(strlen(row[0]) == 19){
				memset(key_time,0,sizeof(key_time));
				strncpy(key_time,row[0]+5,14);
			}
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

			row_index = 2;
			if ( row_cnt == 0 )
			{
				sprintf(resBuf, "%3s %15s %8s %-12s %-12s %-12s\n",
						"", key_time, SysName[i], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
			}
			else
			{
				sprintf(resBuf, "%3s %15s %8s %-12s %-12s %-12s\n",
						"", "", SysName[i], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
			}
			if( min > strtod(row[row_index],0) ) 
				min = strtod(row[row_index],0);

			if( max < strtod(row[row_index+1],0) )
				max = strtod(row[row_index+1],0);

			sum += strtod(row[row_index+2],0);

			sprintf(last_time, "%s",key_time);

			select_cnt++;
			tcnt++;

			if(strlen(ipafBuf) > 2500)
			{
				stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
				snd_cnt++;
				memset (ipafBuf, 0x00, sizeof(ipafBuf));
			}
			row_cnt++;
		}
		mysql_free_result(result);

		sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

		if (select_cnt > 0)
		{
			row_index = 0;
			avg = sum / select_cnt;
			sprintf(resTmp, "%3s %15s %8s %-12.6f %-12.6f %-12.6f\n",
					"", "TOTAL", SysName[i], min, max, avg);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);
		}
			///////////////////////////
	}

	if (tcnt == 0) 
	{
		sprintf(resTmp, "    NO DATA\n");
		strcat(ipafBuf, resTmp);
		sprintf(resTmp, "    ====================================================================================================\n");
		strcat(ipafBuf,resTmp);
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
	} 
	else
	{
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
	}

	return 1;
}

int stmd_mmc_srch_stat_logon(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;

	char        query[4096] = {0,}, query_head[4096] = {0,}, query_pdsn[1024] ={0,};
	char        cmdName[64] = {0,};
	MYSQL_RES   *result, *res_pd;
	MYSQL_ROW   row, row_pd;

	char  	table_name[30] = {0,};
	char  	stime[33] = {0,}; char etime[33] = {0,};
	int   	i_tmp = 0;
	char  	*env, fname[256] = {0,}, tmp[64] = {0,};
	int   	row_cnt=0;
	char  	key_time[33] = {0,}, last_time[33] = {0,};

	char    itemRes[10][10];
	int     i,j,k, ridx, toResult;
	char title[5][16]={"STATTIME", "SYSTEM", "LOG_REQ", "LOG_SUCC","LOG_FAIL"};
	char title1[6][16]={"", "", "HBIT_0","HBIT_1","HBIT_2", "HBIT_3"};
	char title2[6][16]={"", "", "SM_INT_ERR","OP_ERR","OP_TIMEOUT", "ETC_FAIL"};
	int 	row_index;
	char 	SysName[2][8]={"SCMA","SCMB"};
	char 	SysIp[2][16];
	int 	realSysCnt =2;
	int 	realItemCnt =0;
	int		index = 0, select_cnt = 0, snd_cnt, tcnt=0;
	unsigned int logTotal[2][11] = {{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0}};

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	memset (ipafBuf, 0x00, sizeof(ipafBuf));
	memset (ipafHead, 0x00, sizeof(ipafHead));

    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        sprintf(table_name, "%s", STM_STATISTIC_5MINUTE_LOGON_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        sprintf(table_name, "%s", STM_STATISTIC_HOUR_LOGON_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        sprintf(table_name, "%s", STM_STATISTIC_DAY_LOGON_TBL_NAME);
    }
    else
    {
        sprintf(table_name, "%s", STM_STATISTIC_MONTH_LOGON_TBL_NAME);
    }

	for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
		rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
	}
	for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
		rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
	}

	if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
		memset(stime, 0, 33);
		memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(stime, "-"); strcat(stime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(stime, "-"); strcat(stime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(stime, " "); strcat(stime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(stime, ":"); strcat(stime, resTmp);
					}
				}
			}
		}
	}
	if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
		memset(etime, 0, 33);
		memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(etime, "-"); strcat(etime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(etime, "-"); strcat(etime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(etime, " "); strcat(etime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(etime, ":"); strcat(etime, resTmp);
					}
				}
			}
		}
	}
	sprintf(cmdName, "srch-stat-logon");

	sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
			rxReqMsg->head.para[1].paraVal,stime,etime);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-12s %-12s %-12s","","", title[0],title[1],title[2],title[3],title[4] );
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-12s %-12s %-12s %-12s", "","", "",title1[2],title1[3],title1[4],title1[5]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-12s %-12s %-12s %-12s", "","", "",title2[2],title2[3],title2[4],title2[5]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);

	strcat(ipafBuf, ipafHead);

	snd_cnt=1;
	for(i=0; i<realSysCnt; i++)
	{ 
		if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
			if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
				continue;
		}

		sprintf(query, "SELECT stat_date, system_name, "
				" IFNULL(SUM(log_req), 0), IFNULL(SUM(log_succ), 0), IFNULL(SUM(log_fail), 0), "
				" IFNULL(SUM(HBIT_0), 0), IFNULL(SUM(HBIT_1), 0), IFNULL(SUM(HBIT_2), 0), "
				" IFNULL(SUM(HBIT_3), 0) "
				" IFNULL(SUM(sm_int_err), 0), IFNULL(SUM(op_err), 0), IFNULL(SUM(op_timeout), 0) "
				" IFNULL(SUM(etc_fail), 0) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by stat_date, system_name order by stat_date, system_name ",table_name
				, SysName[i], stime, etime );

		if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
			sprintf(trcBuf, "query = %s\n", query);
			trclib_writeLog(FL, trcBuf);
		}
		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
		row_cnt = 0;                                                                                  
		result = mysql_store_result(conn);

		while( (row = mysql_fetch_row(result)) != NULL)
		{
			if(strlen(row[0]) == 19){
				memset(key_time,0,sizeof(key_time));
				strncpy(key_time,row[0]+5,14);
			}
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

			row_index = 2;
			if ( row_cnt == 0 )
			{
				sprintf(resBuf, "%2s %15s %6s %-12s %-12s %-12s\n",
						"", key_time, SysName[i], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+3], row[row_index+4], row[row_index+5], row[row_index+6]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+7], row[row_index+8], row[row_index+9]);
				strcat(ipafBuf, resBuf);
			}
			else
			{
				sprintf(resBuf, "%2s %15s %6s %-12s %-12s %-12s\n",
						"", "", SysName[i], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+3], row[row_index+4], row[row_index+5], row[row_index+6]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+7], row[row_index+8], row[row_index+9]);
				strcat(ipafBuf, resBuf);
			}
			logTotal[row_cnt][0] += atoi(row[row_index]); 
			logTotal[row_cnt][1] += atoi(row[row_index+1]); 
			logTotal[row_cnt][2] += atoi(row[row_index+2]);
			logTotal[row_cnt][3] += atoi(row[row_index+3]); 
			logTotal[row_cnt][4] += atoi(row[row_index+4]); 
			logTotal[row_cnt][5] += atoi(row[row_index+5]);
			logTotal[row_cnt][6] += atoi(row[row_index+6]);
			logTotal[row_cnt][7] += atoi(row[row_index+7]);
			logTotal[row_cnt][8] += atoi(row[row_index+8]);
			logTotal[row_cnt][9] += atoi(row[row_index+9]);
			logTotal[row_cnt][10] += atoi(row[row_index+10]);

			strcpy(last_time, key_time);

			select_cnt++;
			tcnt++;

			if(strlen(ipafBuf) > 2500)
			{
				stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
				snd_cnt++;
				memset (ipafBuf, 0x00, sizeof(ipafBuf));
			}
			row_cnt++;
		}
		mysql_free_result(result);

		sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

		if (select_cnt > 0)
		{
			row_index = 0;
			sprintf(resTmp, "%2s %15s %6s %-12d %-12d %-12d\n",
					"", "SUM",SysName[i], logTotal[i][0], logTotal[i][1], logTotal[i][2]);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "%2s %15s %6s %-12d %-12d %-12d %-12d\n",
					"", "SUM","", logTotal[i][3], logTotal[i][4], logTotal[i][5],logTotal[i][6]);
			sprintf(resTmp, "%2s %15s %6s %-12d %-12d %-12d %-12d\n",
					"", "SUM","", logTotal[i][7], logTotal[i][8], logTotal[i][9],logTotal[i][10]);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < 11; j++)
			{
				logTotal[i][j] = 0;
			}
		}
			///////////////////////////
	}

	if (tcnt == 0) 
	{
		sprintf(resTmp, "    NO DATA\n");
		strcat(ipafBuf, resTmp);
		sprintf(resTmp, "    ====================================================================================================\n");
		strcat(ipafBuf,resTmp);
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
	} 
	else
	{
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
	}

	return 1;
}

int stmd_mmc_srch_stat_sms(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;

	char        query[4096], query_head[4096], query_pdsn[1024];
	char        cmdName[64];
	MYSQL_RES   *result, *res_pd;
	MYSQL_ROW   row, row_pd;

	char  	table_name[30];
	char  	stime[33]; char etime[33];
	int   	i_tmp;
	char  	*env, fname[256], tmp[64];
	int   	row_cnt=0;
	char  	key_time[33], last_time[33] = {0,};

	char    itemRes[10][10];
	int     i,j,k, ridx, toResult;
	char title[6][16]={"STATTIME", "SYSTEM", "ITEM", "REQ", "SUCC","FAIL"};
	char title1[7][16]={"", "", "", "SMPP_ERR","SVR_ERR","SMSC_ERR","ETC_ERR"};
	int 	row_index;
	char 	SysName[2][8]={"SCMA","SCMB"};
	char 	SysIp[2][16];
	int 	realSysCnt =2;
	int 	realItemCnt =0;
	int		index, select_cnt = 0, snd_cnt, tcnt=0;
	unsigned int	smsTotal[2][7] = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}};

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	memset (ipafBuf, 0x00, sizeof(ipafBuf));
	memset (ipafHead, 0x00, sizeof(ipafHead));

    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_SMS_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_SMS_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_SMS_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_SMS_TBL_NAME);
    }

	for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
		rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
	}
	for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
		rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
	}

	if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
		memset(stime, 0, 33);
		memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(stime, "-"); strcat(stime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(stime, "-"); strcat(stime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(stime, " "); strcat(stime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(stime, ":"); strcat(stime, resTmp);
					}
				}
			}
		}
	}
	if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
		memset(etime, 0, 33);
		memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(etime, "-"); strcat(etime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(etime, "-"); strcat(etime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(etime, " "); strcat(etime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(etime, ":"); strcat(etime, resTmp);
					}
				}
			}
		}
	}
	strcpy(cmdName, "srch-stat-sms");

	sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
			rxReqMsg->head.para[1].paraVal,stime,etime);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-18s %-12s %-12s %-12s\n","",title[0],title[1],title[2],title[3],title[4]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-18s %-12s %-12s %-12s %-12s\n","","","","",title1[3],title1[4],title1[5],title1[6]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);

	strcat(ipafBuf, ipafHead);

	snd_cnt=1;
	for(i=0; i<realSysCnt; i++)
	{ 
		if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
			if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
				continue;
		}

		// realItemCnt : pdsn
		sprintf(query, "select system_name, sum(cnt) list "
				" from ( "
				" select system_name, smsc_ip, count(*) cnt "
				" from %s where system_name = '%s' and stat_date > '%s' AND stat_date <= '%s' "
				" group by system_name, smsc_ip ) a",
				table_name, SysName[i], stime,etime);

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
		result = mysql_store_result(conn);
		row = mysql_fetch_row(result);
		if( row == NULL )
		{
			realItemCnt = 0;
			mysql_free_result(result);
			continue;
		}
		else
		{
			if(row[1] == NULL)
			{
				mysql_free_result(result);
				continue;
			}
			else
			{
				realItemCnt = atoi(row[1]);
			}
		}
		sprintf(query, "SELECT stat_date, system_name, smsc_ip, "
				" IFNULL(SUM(req), 0), IFNULL(SUM(succ), 0), IFNULL(SUM(fail), 0), "
				" IFNULL(SUM(smpp_err), 0), "
				" IFNULL(SUM(svr_err), 0), IFNULL(SUM(smsc_err), 0), "
				" IFNULL(SUM(etc_err), 0) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by stat_date, system_name, smsc_ip order by stat_date, system_name, smsc_ip ",
				table_name, SysName[i], stime, etime );

		if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
			sprintf(trcBuf, "query = %s\n", query);
			trclib_writeLog(FL, trcBuf);
		}
		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
		row_cnt = 0;                                                                                  
		result = mysql_store_result(conn);

		while( (row = mysql_fetch_row(result)) != NULL)
		{
			if(strlen(row[0]) == 19){
				memset(key_time,0,sizeof(key_time));
				strncpy(key_time,row[0]+5,14);
			}
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

			row_index = 2;
			if ( row_cnt == 0 )
			{
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s\n",
						"", key_time, SysName[i], row[1], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5],row[row_index+6]);
				strcat(ipafBuf, resBuf);
			}
			else
			{
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s\n",
						"", "", "", row[1], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5],row[row_index+6]);
				strcat(ipafBuf, resBuf);
			}
			smsTotal[row_cnt][0] += atoi(row[row_index]); 
			smsTotal[row_cnt][1] += atoi(row[row_index+1]); 
			smsTotal[row_cnt][2] += atoi(row[row_index+2]);
			smsTotal[row_cnt][3] += atoi(row[row_index+3]); 
			smsTotal[row_cnt][4] += atoi(row[row_index+4]); 
			smsTotal[row_cnt][5] += atoi(row[row_index+5]);
			smsTotal[row_cnt][6] += atoi(row[row_index+6]);

			strcpy(last_time, key_time);

			select_cnt++;
			tcnt++;

			if(strlen(ipafBuf) > 2500)
			{
				stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
				snd_cnt++;
				memset (ipafBuf, 0x00, sizeof(ipafBuf));
			}
			row_cnt++;
		}
		mysql_free_result(result);

		sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

		if (select_cnt > 0)
		{
			row_index = 0;
			sprintf(resTmp, "%2s %15s %6s %-18s %-12d %-12d %-12d\n",
					"", "SUM",SysName[i], "", 
					smsTotal[i][0], smsTotal[i][1], smsTotal[i][2]);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "%2s %15s %6s %-18s %-12d %-12d %-12d %-12d\n",
					"", "SUM","", "",
					smsTotal[i][3],smsTotal[i][4], smsTotal[i][5], smsTotal[i][6]);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < 7; j++)
			{
				smsTotal[i][j] = 0;
			}
		}
			///////////////////////////
	}

	if (tcnt == 0) 
	{
		sprintf(resTmp, "    NO DATA\n");
		strcat(ipafBuf, resTmp);
		sprintf(resTmp, "    ====================================================================================================\n");
		strcat(ipafBuf,resTmp);
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
	} 
	else
	{
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
	}

	return 1;
}


int stmd_mmc_srch_stat_leg(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;

	char        query[4096], query_head[4096], query_pdsn[1024];
	char        cmdName[64];
	MYSQL_RES   *result, *res_pd;
	MYSQL_ROW   row, row_pd;

	char  	table_name[30];
	char  	stime[33]; char etime[33];
	int   	i_tmp;
	char  	*env, fname[256], tmp[64];
	int   	row_cnt=0;
	char  	key_time[33], last_time[33] = {0,};

	char    itemRes[10][10];
	int     i,j,k, ridx, toResult;
	char title[7][16]={"STATTIME", "SYSTEM", "PDSN_IP", "RX_CNT", "START","INTERIM","STOP"};
	char title1[6][16]={"", "", "", "START_LOGON","INTERIM_LOGON","LOG_OUT"};
	int 	row_index;
	char 	SysName[2][8]={"SCMA","SCMB"};
	char 	SysIp[2][16];
	int 	realSysCnt =2;
	int 	realItemCnt =0;
	int		index, select_cnt = 0, snd_cnt, tcnt=0;
	unsigned int	legTotal[2][7] = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}};

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	memset (ipafBuf, 0x00, sizeof(ipafBuf));
	memset (ipafHead, 0x00, sizeof(ipafHead));

    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_LEG_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_LEG_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_LEG_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_LEG_TBL_NAME);
    }

	for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
		rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
	}
	for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
		rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
	}

	if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
		memset(stime, 0, 33);
		memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(stime, "-"); strcat(stime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(stime, "-"); strcat(stime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(stime, " "); strcat(stime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(stime, ":"); strcat(stime, resTmp);
					}
				}
			}
		}
	}
	if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
		memset(etime, 0, 33);
		memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(etime, "-"); strcat(etime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(etime, "-"); strcat(etime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(etime, " "); strcat(etime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(etime, ":"); strcat(etime, resTmp);
					}
				}
			}
		}
	}
	strcpy(cmdName, "srch-stat-account");

	sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
			rxReqMsg->head.para[1].paraVal,stime,etime);
	sprintf(resTmp, "    ================================================================================================\n");
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-18s %-12s %-12s %-12s %-12s\n","",title[0],title[1],title[2],title[3],title[4],title[5],title[6]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-18s %-12s %-12s %-12s\n","","","","",title1[3],title1[4],title1[5]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "    ================================================================================================\n");
	strcat(ipafHead, resTmp);

	strcat(ipafBuf, ipafHead);

	snd_cnt=1;
	for(i=0; i<realSysCnt; i++)
	{ 
		if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
			if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
				continue;
		}

		sprintf(query, "SELECT  count(*) from ip_code_tbl where type = 1; ");

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
		result = mysql_store_result(conn);
		row = mysql_fetch_row(result);
		if( row == NULL )
		{
			realItemCnt = 0;                                                               
			mysql_free_result(result);
		}
		else
		{
			realItemCnt = atoi(row[0]);
			mysql_free_result(result);
		}

		sprintf(query, "SELECT stat_date, system_name, pdsn_ip, "
				" IFNULL(SUM(rx_cnt), 0), IFNULL(SUM(start), 0), IFNULL(SUM(interim), 0), "
				" IFNULL(SUM(stop), 0), "
				" IFNULL(SUM(start_logon_cnt), 0), IFNULL(SUM(int_logon_cnt), 0), "
				" IFNULL(SUM(logout_cnt), 0) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by stat_date, system_name, pdsn_ip order by stat_date, system_name, pdsn_ip ",table_name
				, SysName[i], stime, etime );

		if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
			sprintf(trcBuf, "query = %s\n", query);
			trclib_writeLog(FL, trcBuf);
		}
		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
		row_cnt = 0;                                                                                  
		result = mysql_store_result(conn);

		while( (row = mysql_fetch_row(result)) != NULL)
		{
			if(strlen(row[0]) == 19){
				memset(key_time,0,sizeof(key_time));
				strncpy(key_time,row[0]+5,14);
			}
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

			row_index = 3;
			if ( row_cnt == 0 )
			{
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", key_time, row[1], row[2],row[row_index], row[row_index+1], row[row_index+2],row[row_index+3]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s\n",
						"", "", "", "", row[row_index+4], row[row_index+5], row[row_index+6]);
				strcat(ipafBuf, resBuf);
			}
			else
			{
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", row[1], row[2], row[row_index], row[row_index+1], row[row_index+2],row[row_index+3]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s\n",
						"", "", "", "", row[row_index+4], row[row_index+5], row[row_index+6]);
				strcat(ipafBuf, resBuf);
			}
			legTotal[row_cnt][0] += atoi(row[row_index]); 
			legTotal[row_cnt][1] += atoi(row[row_index+1]); 
			legTotal[row_cnt][2] += atoi(row[row_index+2]);
			legTotal[row_cnt][3] += atoi(row[row_index+3]); 
			legTotal[row_cnt][4] += atoi(row[row_index+4]); 
			legTotal[row_cnt][5] += atoi(row[row_index+5]);
			legTotal[row_cnt][6] += atoi(row[row_index+6]);

			strcpy(last_time, key_time);

			select_cnt++;
			tcnt++;

			if(strlen(ipafBuf) > 2500)
			{
				stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
				snd_cnt++;
				memset (ipafBuf, 0x00, sizeof(ipafBuf));
			}
			row_cnt++;
		}
		mysql_free_result(result);

		sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

		if (select_cnt > 0)
		{
			row_index = 0;
			sprintf(resTmp, "%2s %15s %6s %-18s %-12d %-12d %-12d %-12d\n",
					"", "SUM",SysName[i], "", 
					legTotal[i][0], legTotal[i][1], legTotal[i][2], legTotal[i][3]);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "%2s %15s %6s %-18s %-12d %-12d %-12d\n",
					"", "","", "",
					legTotal[i][4], legTotal[i][5], legTotal[i][6]);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < 7; j++)
			{
				legTotal[i][j] = 0;
			}
		}
			///////////////////////////
	}

	if (tcnt == 0) 
	{
		for( i = 0; i < 2; i++ )
		{
			for(j=0;j<realItemCnt;j++)
			{
				if( j == 0 )
				{
					sprintf(resTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s\n",
							"", SysName[i], g_stPdsn[i].stItem[j].ip, "0","0","0","0");
					strcat(ipafBuf, resTmp);
					sprintf(resTmp, "%3s %6s %-18s %-12s %-12s %-12s\n",
							"", "", "", "0","0","0");
					strcat(ipafBuf, resTmp);
				}
				else
				{
					sprintf(resTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s\n",
							"", "", "", "0", "0","0","0");
					strcat(ipafBuf, resTmp);                                  
					sprintf(resTmp, "%3s %6s %-18s %-12s %-12s %-12s\n",
							"", "", "", "0","0","0");
					strcat(ipafBuf, resTmp);
				}
			}
		}

		sprintf(resTmp, "    ====================================================================================================\n");
		strcat(ipafBuf,resTmp);
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
	} 
	else
	{
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
	}

	return 1;
}


int stmd_mmc_srch_stat_link(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;

	char        query[4096], query_head[4096];
	char        cmdName[64];
	MYSQL_RES   *result;
	MYSQL_ROW   row;

	char  	table_name[30];
	char  	stime[33]; char etime[33];
	int   	i_tmp;
	char  	*env, fname[256], tmp[64];
	int   	row_cnt=0;
	char  	key_time[33], last_time[33] = {0,};

	char    itemRes[10][10];
	int     i,j,k, ridx, toResult;
	char 	title[6][16] = {"STATTIME", "SYSTEM", "ITEM", "UpStream(Mbps)", "DnStream(Mbps)", "Total(Mbps)"};
	char 	title1[6][16] = {"", "", "", "UpBytes(MBytes)", "DnBytes(MBytes)", "Total(MBytes)"};
	int 	row_index;
	char 	SysName[SCE_CNT][8];
	char 	SysIp[SCE_CNT][16];
	int 	realSysCnt =0;
	int 	realItemCnt =0;
	int		index, select_cnt = 0, snd_cnt, tcnt=0;
	char	linkName[2][10] = {"Link 1", "Link 2"};
	long double linkTotal[2][6] = {{0.0L,0.0L,0.0L,0.0L,0.0L,0.0L},{0.0L,0.0L,0.0L,0.0L,0.0L,0.0L}};

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	memset (ipafBuf, 0x00, sizeof(ipafBuf));
	memset (ipafHead, 0x00, sizeof(ipafHead));

	realItemCnt = 2;

	for(i=0; i<SCE_CNT; i++ )
	{
		strcpy(SysName[i], g_stSCE[i].sce_name);
		strcpy(SysIp[i], g_stSCE[i].sce_ip);
		realSysCnt++;
	}

	for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
		rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
	}
	for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
		rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
	}

	if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
	{
		strcpy(table_name,STM_STATISTIC_5MINUTE_LUR_TBL_NAME);
		sprintf(query_head, "SELECT stat_date, link_id, "
				" round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
				" round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
				" round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
				" round(sum(upstream_volume)/1024,2), "
				" round(sum(downstream_volume)/1024,2), "
				" round(sum(upstream_volume+downstream_volume)/1024,2) "
				" from %s ", table_name);
	}
	else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
	{
		strcpy(table_name,STM_STATISTIC_HOUR_LUR_TBL_NAME);
		sprintf(query_head, "SELECT stat_date, link_id, "
				" round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
				" round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
				" round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
				" round(sum(upstream_volume)/1024,2), "
				" round(sum(downstream_volume)/1024,2), "
				" round(sum(upstream_volume+downstream_volume)/1024,2) "
				" from %s ", table_name);
	}
	else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
	{
		strcpy(table_name,STM_STATISTIC_DAY_LUR_TBL_NAME);
		sprintf(query_head, "SELECT stat_date, link_id, "
				" round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
				" round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
				" round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
				" round(sum(upstream_volume)/1024,2), "
				" round(sum(downstream_volume)/1024,2), "
				" round(sum(upstream_volume+downstream_volume)/1024,2) "
				" from %s ", table_name);
	}
	else
	{
		strcpy(table_name,STM_STATISTIC_MONTH_LUR_TBL_NAME);
		sprintf(query_head, "SELECT stat_date, link_id, "
				" round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
				" round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
				" round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
				" round(sum(upstream_volume)/1024,2), "
				" round(sum(downstream_volume)/1024,2), "
				" round(sum(upstream_volume+downstream_volume)/1024,2) "
				" from %s ", table_name);
	}

	if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
		memset(stime, 0, 33);
		memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(stime, "-"); strcat(stime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(stime, "-"); strcat(stime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(stime, " "); strcat(stime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(stime, ":"); strcat(stime, resTmp);
					}
				}
			}
		}
	}
	if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
		memset(etime, 0, 33);
		memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(etime, "-"); strcat(etime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(etime, "-"); strcat(etime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(etime, " "); strcat(etime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(etime, ":"); strcat(etime, resTmp);
					}
				}
			}
		}
	}
	strcpy(cmdName, "srch-stat-link");

	sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
			rxReqMsg->head.para[1].paraVal,stime,etime);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%3s %15s %6s %-10s %15s %15s %15s\n","",title[0],title[1],title[2],title[3],title[4],title[5]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%3s %15s %6s %-10s %15s %15s %15s\n","","","","",title1[3],title1[4],title1[5]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);

	strcat(ipafBuf, ipafHead);

	snd_cnt=1;
	for(i=0; i<realSysCnt; i++)
	{ 
		if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
			if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
				continue;
		}

		sprintf(query, "%s "
				" where record_source = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" Group By stat_date, link_id ORDER BY stat_date, link_id ", 
				query_head, SysIp[i], stime, etime );

		if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
			sprintf(trcBuf, "query = %s\n", query);
			trclib_writeLog(FL, trcBuf);
		}
		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
		row_cnt = 0;                                                                                  
		result = mysql_store_result(conn);

		while( (row = mysql_fetch_row(result)) != NULL)
		{
			if(strlen(row[0]) == 19){
				memset(key_time,0,sizeof(key_time));
				strncpy(key_time,row[0]+5,14);
			}
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

			index = atoi(row[1]); // 배열 index 
			row_index = 2;
			if ( row_cnt == 0 )
			{
				sprintf(resBuf, "%3s %15s %6s %-10s %15s %15s %15s\n",
						"", key_time, SysName[i], linkName[index], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%3s %15s %6s %-10s %15s %15s %15s\n",
						"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
				strcat(ipafBuf, resBuf);
			}
			else
			{
				sprintf(resBuf, "%3s %15s %6s %-10s %15s %15s %15s\n",
						"", "", "", linkName[index], row[row_index], row[row_index+1], row[row_index+2]);

				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%3s %15s %6s %-10s %15s %15s %15s\n",
						"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
				strcat(ipafBuf, resBuf);
			}
			linkTotal[index][0] += strtold(row[row_index],0); 
			linkTotal[index][1] += strtold(row[row_index+1],0); 
			linkTotal[index][2] += strtold(row[row_index+2],0);
			linkTotal[index][3] += strtold(row[row_index+3],0); 
			linkTotal[index][4] += strtold(row[row_index+4],0); 
			linkTotal[index][5] += strtold(row[row_index+5],0);

			strcpy(last_time, key_time);

			select_cnt++;
			tcnt++;

			if(strlen(ipafBuf) > 2500)
			{
				stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
				snd_cnt++;
				memset (ipafBuf, 0x00, sizeof(ipafBuf));
			}
			row_cnt++;
		}
		mysql_free_result(result);

		sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

		if (select_cnt > 0)
		{
			row_index = 0;
			for(j=0;j<realItemCnt;j++)
			{
				sprintf(resTmp, "%3s %15s %6s %-10s %15.2Lf %15.2Lf %15.2Lf\n",
						"", "SUM(Thru)",SysName[i], linkName[j], 
						linkTotal[j][0], linkTotal[j][1], linkTotal[j][2]);

				strcat(ipafBuf, resTmp);
				sprintf(resTmp, "%3s %15s %6s %-10s %15.2Lf %15.2Lf %15.2Lf\n",
						"", "SUM(Bytes)",SysName[i], linkName[j],
						linkTotal[j][3], linkTotal[j][4], linkTotal[j][5]);
				strcat(ipafBuf, resTmp);
			}
			sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < 6; j++)
			{
				linkTotal[i][j] = 0.0L;
			}
		}
			///////////////////////////
	}

	if (tcnt == 0) 
	{
		sprintf(resTmp, "    NO DATA\n");
		strcat(ipafBuf, resTmp);
		sprintf(resTmp, "    ====================================================================================================\n");
		strcat(ipafBuf,resTmp);
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
	} 
	else
	{
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
	}

	return 1;
}




int stmd_mmc_srch_stat_rule_ent(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096], query_head[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;

    char  	table_name[30];
    char  	stime[33]; char etime[33];
    int   	i_tmp;
    char  	*env, fname[256], tmp[64];
    int   	row_cnt=0;
    char  	key_time[33], last_time[33] = {0,};

    char    itemRes[10][10];
    int     i,j,k, ridx, toResult;
//	char 	*title[] = {"STATTIME", "SYSTEM", "ITEM", "Session", "Block", "Redirect"};
	char 	title[5][16] = {"STATTIME", "SYSTEM", "ITEM", "Session", "Block" };
	char 	title1[6][16] = {"", "", "", "UpStream(Mbps)", "DnStream(Mbps)", "Total(Mbps)"};
	char 	title2[6][16] = {"", "", "", "UpByte(MBytes)", "DnByte(MBytes)", "Total(MBytes)"};
//	char 	*title3[] = {"", "", "", "Active User", "Total Active User", "Usage(%)"};
    int 	row_index;
    char 	SysName[SCE_CNT][8];
    char 	SysIp[SCE_CNT][16];
    int 	realSysCnt =0;
    int 	realItemCnt =0;
	char	tmpRule[5][512];
	int		ruleSet[5], index;
    int     select_cnt = 0, snd_cnt,tcnt=0;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_RULEENT_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, rule_ent_id, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0), IFNULL(SUM(session),0)+IFNULL(SUM(block_cnt),0), "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_RULEENT_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, rule_ent_id, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0),IFNULL(SUM(session),0)+IFNULL(SUM(block_cnt),0), "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_RULEENT_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, rule_ent_id, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0), IFNULL(SUM(session),0)+IFNULL(SUM(block_cnt),0), "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_RULEENT_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, rule_ent_id, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0), IFNULL(SUM(session),0)+IFNULL(SUM(block_cnt),0), "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-rule-ent");

    sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
        rxReqMsg->head.para[1].paraVal,stime,etime);
    sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafBuf, resTmp);
	sprintf(resTmp, "%3s %6s %-28s %-15s %-15s \n","",title[0],title[1],title[2],title[3],title[4]);
	strcat(ipafBuf, resTmp);
	sprintf(resTmp, "%3s %6s %-28s %-15s %-15s %-15s\n","","","",title1[2],title1[3],title1[4]);
	strcat(ipafBuf, resTmp);
	sprintf(resTmp, "%3s %6s %-28s %-15s %-15s %-15s\n","","","",title2[2],title2[3],title2[4]);
	strcat(ipafBuf, resTmp);
    sprintf(resTmp, "    ====================================================================================================\n");
    strcat(ipafHead, resTmp);

    strcat(ipafBuf, ipafHead);


	for(i=0; i<SCE_CNT; i++ )
	{
		strcpy(SysName[i], g_stSCE[i].sce_name);
		strcpy(SysIp[i], g_stSCE[i].sce_ip);
		realSysCnt++;
	}

    snd_cnt=1;
    for(i=0; i<realSysCnt; i++)
	{ 
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
                continue;
        }

		realItemCnt = g_stSCEEntry[i].ruleEntryCnt;

        sprintf(query, "%s "
                " where record_source = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" Group By stat_date, rule_ent_id ORDER BY stat_date, rule_ent_id ", 
				query_head, SysIp[i], stime, etime );

        if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
        row_cnt = 0;                                                                                  
        result = mysql_store_result(conn);

        while( (row = mysql_fetch_row(result)) != NULL)
        {
			toResult = 0;
            if(strlen(row[0]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[0]+5,14);
            }
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

            index = atoi(row[1]); // 배열 index 
			
			row_index = 2;
			if ( row_cnt == 0 )
			{
				sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
						"", key_time, SysName[i], g_stSCEEntry[i].stEntry[index].eName, row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
						"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
						"", "", "", "", row[row_index+6], row[row_index+7], row[row_index+8]);
				strcat(ipafBuf, resBuf);
			}
			else
			{
				sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
						"", "", "", g_stSCEEntry[i].stEntry[index].eName, row[row_index], row[row_index+1], row[row_index+2]);

				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
						"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
						"", "", "", "", row[row_index+6], row[row_index+7], row[row_index+8]);
				strcat(ipafBuf, resBuf);
			}
			g_stSCEEntry[i].stEntry[index].unblk += atoi(row[row_index]); 
			g_stSCEEntry[i].stEntry[index].blk += atoi(row[row_index+1]); 
			g_stSCEEntry[i].stEntry[index].tot += atoi(row[row_index+2]);
			g_stSCEEntry[i].stEntry[index].uThru += strtold(row[row_index+3],0); 
			g_stSCEEntry[i].stEntry[index].dThru += strtold(row[row_index+4],0); 
			g_stSCEEntry[i].stEntry[index].tThru += strtold(row[row_index+5],0);
			g_stSCEEntry[i].stEntry[index].uByte += strtold(row[row_index+6],0); 
			g_stSCEEntry[i].stEntry[index].dByte += strtold(row[row_index+7],0); 
			g_stSCEEntry[i].stEntry[index].tByte += strtold(row[row_index+8],0);

			strcpy(last_time, key_time);

			select_cnt++;
			tcnt++;

            if(strlen(ipafBuf) > 2500)
			{
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                snd_cnt++;
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            row_cnt++;
        }

        mysql_free_result(result);

    	sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

        if (select_cnt > 0)
		{
            if(strlen(ipafBuf) > 2500)
			{
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                snd_cnt++;
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
			row_index = 0;
			for(j=0;j<realItemCnt;j++)
			{
				if( g_stSCEEntry[i].stEntry[j].eName != NULL )
				{
					sprintf(resTmp, "%3s %17s %6s %-28s %15d %15d %15d\n",
							"", "SUM(Cnt)",SysName[i], g_stSCEEntry[i].stEntry[j].eName, g_stSCEEntry[i].stEntry[j].unblk, \
							g_stSCEEntry[i].stEntry[j].blk, g_stSCEEntry[i].stEntry[j].tot);
					strcat(ipafBuf, resTmp);
					sprintf(resTmp, "%3s %17s %6s %-28s %15.2Lf %15.2Lf %15.2Lf\n",
							"", "SUM(Thru)",SysName[i], g_stSCEEntry[i].stEntry[j].eName, g_stSCEEntry[i].stEntry[j].uThru, \
							g_stSCEEntry[i].stEntry[j].dThru, g_stSCEEntry[i].stEntry[j].tThru);
					strcat(ipafBuf, resTmp);
					sprintf(resTmp, "%3s %17s %6s %-28s %15.2Lf %15.2Lf %15.2Lf\n",
							"", "SUM(Bytes)",SysName[i], g_stSCEEntry[i].stEntry[j].eName, g_stSCEEntry[i].stEntry[j].uByte, \
							g_stSCEEntry[i].stEntry[j].dByte, g_stSCEEntry[i].stEntry[j].tByte);
					strcat(ipafBuf, resTmp);
				}
			}
    		sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < MAX_ENTRY_NUM; j++)
			{
				g_stSCEEntry[i].stEntry[j].unblk = 0;
				g_stSCEEntry[i].stEntry[j].blk = 0;
				g_stSCEEntry[i].stEntry[j].tot = 0;
				g_stSCEEntry[i].stEntry[j].uThru = 0;
				g_stSCEEntry[i].stEntry[j].dThru = 0;
				g_stSCEEntry[i].stEntry[j].tThru = 0;
				g_stSCEEntry[i].stEntry[j].uByte = 0;
				g_stSCEEntry[i].stEntry[j].dByte = 0;
				g_stSCEEntry[i].stEntry[j].tByte = 0;
			}
		}
///////////////////////////
    }

    if (tcnt == 0) 
	{
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
    	sprintf(resTmp, "    ====================================================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } 
	else
	{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;
}


int stmd_mmc_srch_stat_rule_set(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096], query_head[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;

    char  	table_name[30];
    char  	stime[33]; char etime[33];
    int   	i_tmp;
    char  	*env, fname[256], tmp[64];
    int   	row_cnt=0;
    char  	key_time[33], last_time[33] = {0,};

    char    itemRes[10][10];
    int     i,j,k, ridx, toResult;
	//char 	*title[] = {"STATTIME", "SYSTEM", "ITEM", "Session", "Block", "Redirect"};
	char 	title[5][16] = {"STATTIME", "SYSTEM", "ITEM", "Session", "Block" };
	char 	title1[6][16] = {"", "", "", "UpStream(Mbps)", "DnStream(Mbps)", "Total(Mbps)"};
	char 	title2[6][16] = {"", "", "", "UpByte(MBytes)", "DnByte(MBytes)", "Total(MBytes)"};
    int 	row_index;
    char 	SysName[SCE_CNT][8];
    char 	SysIp[SCE_CNT][16];
    int 	realSysCnt =0;
    int 	realItemCnt =0;
	char	tmpRule[5][512];
	int		ruleSet[5], index;
    int     select_cnt = 0, snd_cnt,tcnt=0;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

	realItemCnt = g_ruleItemCnt;

	for(i=0; i<SCE_CNT; i++ )
	{
		strcpy(SysName[i], g_stSCE[i].sce_name);
		strcpy(SysIp[i], g_stSCE[i].sce_ip);
		realSysCnt++;
	}

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_RULESET_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, pkg_id, rule_set_id, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0), IFNULL(SUM(session+block_cnt),0), "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_RULESET_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, pkg_id, rule_set_id, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0), IFNULL(SUM(session+block_cnt),0), "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_RULESET_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, pkg_id, rule_set_id, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0), IFNULL(SUM(session_block_cnt),0), "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_RULESET_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, pkg_id, rule_set_id, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0), IFNULL(SUM(session+block_cnt),0), "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-rule-set");

    sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
        rxReqMsg->head.para[1].paraVal,stime,etime);
    sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafBuf, resTmp);
	sprintf(resTmp, "%3s %6s %-28s %-15s %-15s \n","",title[0],title[1],title[2],title[3],title[4]);
	strcat(ipafBuf, resTmp);
	sprintf(resTmp, "%3s %6s %-28s %-15s %-15s %-15s\n","","","",title1[2],title1[3],title1[4]);
	strcat(ipafBuf, resTmp);
	sprintf(resTmp, "%3s %6s %-28s %-15s %-15s %-15s\n","","","",title2[2],title2[3],title2[4]);
	strcat(ipafBuf, resTmp);
    sprintf(resTmp, "    ====================================================================================================\n");
    strcat(ipafHead, resTmp);

    strcat(ipafBuf, ipafHead);

    snd_cnt=1;
    for(i=0; i<realSysCnt; i++)
	{ 
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
                continue;
        }

        sprintf(query, "%s "
                " where record_source = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" Group By stat_date, pkg_id, rule_set_id ORDER BY stat_date, pkg_id, rule_set_id ", 
				query_head, SysIp[i], stime, etime );

        if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
        row_cnt = 0;                                                                                  
        result = mysql_store_result(conn);

        while( (row = mysql_fetch_row(result)) != NULL)
        {
			toResult = 0;
            if(strlen(row[0]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[0]+5,14);
            }
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

            index = atoi(row[1]); // 배열 index 
			for( ridx = 0; ridx < realItemCnt; ridx++ )
			{
				if( g_ruleIdBuf[ridx] == index && g_stSCERule[i].stRule[index].real == 1 )
				{
					toResult = 1;
					break;
				}
			}
			if( toResult == 1 )
			{
				row_index = 3;
				if ( row_cnt == 0 )
				{
					sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
							"", key_time, SysName[i], g_stSCERule[i].stRule[index].rName, row[row_index], row[row_index+1], row[row_index+2]);
					strcat(ipafBuf, resBuf);
					sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
							"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
					strcat(ipafBuf, resBuf);
					sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
							"", "", "", "", row[row_index+6], row[row_index+7], row[row_index+8]);
					strcat(ipafBuf, resBuf);
				}
				else
				{
					sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
							"", "", "", g_stSCERule[i].stRule[index].rName, row[row_index], row[row_index+1], row[row_index+2]);

					strcat(ipafBuf, resBuf);
					sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
							"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
					strcat(ipafBuf, resBuf);
					sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
							"", "", "", "", row[row_index+6], row[row_index+7], row[row_index+8]);
					strcat(ipafBuf, resBuf);
				}
				g_stSCERule[i].stRule[index].unblk += atoi(row[row_index]); 
				g_stSCERule[i].stRule[index].blk += atoi(row[row_index+1]); 
				g_stSCERule[i].stRule[index].tot += atoi(row[row_index+2]);
				g_stSCERule[i].stRule[index].uThru += strtold(row[row_index+3],0); 
				g_stSCERule[i].stRule[index].dThru += strtold(row[row_index+4],0); 
				g_stSCERule[i].stRule[index].tThru += strtold(row[row_index+5],0);
				g_stSCERule[i].stRule[index].uByte += strtold(row[row_index+6],0); 
				g_stSCERule[i].stRule[index].dByte += strtold(row[row_index+7],0); 
				g_stSCERule[i].stRule[index].tByte += strtold(row[row_index+8],0);

				strcpy(last_time, key_time);

				select_cnt++;
				tcnt++;
			}

            if(strlen(ipafBuf) > 2500)
			{
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                snd_cnt++;
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            row_cnt++;
        }

        mysql_free_result(result);

    	sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

        if (select_cnt > 0)
		{
			row_index = 0;
			for(j=0;j<realItemCnt;j++)
			{
				index = g_ruleIdBuf[j];
				if( g_ruleIdBuf[j] == index && g_stSCERule[i].stRule[index].real == 1 )
				{
					sprintf(resTmp, "%3s %17s %6s %-28s %15d %15d %15d\n",
							"", "SUM(Cnt)",SysName[i], g_stSCERule[i].stRule[index].rName, g_stSCERule[i].stRule[index].unblk, \
							g_stSCERule[i].stRule[index].blk, g_stSCERule[i].stRule[index].tot);
					strcat(ipafBuf, resTmp);
					sprintf(resTmp, "%3s %17s %6s %-28s %15.2Lf %15.2Lf %15.2Lf\n",
							"", "SUM(Thru)",SysName[i], g_stSCERule[i].stRule[index].rName, g_stSCERule[i].stRule[index].uThru, \
							g_stSCERule[i].stRule[index].dThru, g_stSCERule[i].stRule[index].tThru);
					strcat(ipafBuf, resTmp);
					sprintf(resTmp, "%3s %17s %6s %-28s %15.2Lf %15.2Lf %15.2Lf\n",
							"", "SUM(Bytes)",SysName[i], g_stSCERule[i].stRule[index].rName, g_stSCERule[i].stRule[index].uByte, \
							g_stSCERule[i].stRule[index].dByte, g_stSCERule[i].stRule[index].tByte);
					strcat(ipafBuf, resTmp);
				}
			}
    		sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < MAX_RULE_NUM; j++)
			{
				g_stSCERule[i].stRule[j].unblk = 0;
				g_stSCERule[i].stRule[j].blk = 0;
				g_stSCERule[i].stRule[j].tot = 0;
				g_stSCERule[i].stRule[j].uThru = 0;
				g_stSCERule[i].stRule[j].dThru = 0;
				g_stSCERule[i].stRule[j].tThru = 0;
				g_stSCERule[i].stRule[j].uByte = 0;
				g_stSCERule[i].stRule[j].dByte = 0;
				g_stSCERule[i].stRule[j].tByte = 0;
			}
		}
///////////////////////////
    }

    if (tcnt == 0) 
	{
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
    	sprintf(resTmp, "    ====================================================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } 
	else
	{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;
}
#endif // 0619 jjinri

#if 0
int stmd_mmc_stat_rule_set(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;
=======
                snd_cnt++;

                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }

            for(k=0;k<8;k=k+2){
                tot_row[k] = tot_row[k]/select_cnt;
            }

            for(j=0;j<8;j++){
                sprintf(itemRes[j], "%d.%d", tot_row[j]/8, tot_row[j]%8); 
            }

            row_index = 0;
            for(j=0;j<realItemCnt;j++){
                if (j==0){
                    sprintf(resTmp, "%3s %-14s  %-9s %-5s %8s %9s\n", 
                        "", "SUM",SysName[i], title[j], itemRes[row_index],itemRes[row_index+1]);
                    strcat(ipafBuf, resTmp);
                } else {
                    sprintf(resTmp, "%3s %-14s  %-9s %-5s %8s %9s\n", 
                        "","", "", title[j], itemRes[row_index],itemRes[row_index+1]);
                    strcat(ipafBuf, resTmp);
                }

                row_index += 2;
            }
            sprintf(resTmp, "    ==================================================\n");
            strcat(ipafBuf, resTmp);

        }

///////////////////////////
    }

    if (tcnt == 0) {
        //sprintf(resTmp, "      %-8s  %10s  %10s  %10s  %10s\n",
        //  "", "0.0", "0.0", "0.0", "0.0");
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
        sprintf(resTmp, "    ==================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;
}
#endif

#if 0 // 0616
int stmd_mmc_stat_sms(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_SMS;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandSms(list);
    }
}



int stmd_mmc_stat_logon(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_LOGON;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandLogon(list);
    }
}

int stmd_mmc_stat_leg(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_LEG;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandLeg(list);
    }
}



int stmd_mmc_stat_link(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_LINK;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandLink(list);
    }
}

int stmd_mmc_stat_delay(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_DEL;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandDelay(list);
    }
}




int stmd_mmc_srch_stat_delay(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;

	char        query[4096], query_head[4096], query_pdsn[1024];
	char        cmdName[64];
	MYSQL_RES   *result, *res_pd;
	MYSQL_ROW   row, row_pd;

	char  	table_name[30];
	char  	stime[33]; char etime[33];
	int   	i_tmp;
	char  	*env, fname[256], tmp[64];
	int   	row_cnt=0;
	char  	key_time[33], last_time[33] = {0,};

	char    itemRes[10][10];
	int     i,j,k, ridx, toResult;
	char *title[]={"STATTIME", "SYSTEM", "MIN_USEC", "MAX_USEC","AVG_USEC"};
	int 	row_index;
	char 	SysName[2][8]={"SCEA","SCEB"};
	char 	SysIp[2][16];
	int 	realSysCnt =1;
	int 	realItemCnt =1;
	int		index, select_cnt = 0, snd_cnt, tcnt=0;
	double 	delTotal[2][11] = {{0.0,0.0,0.0},{0.0,0.0,0.0}};
	double	min = 0.0, max = 0.0, avg = 0.0;
	double 	sum = 0.0;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	memset (ipafBuf, 0x00, sizeof(ipafBuf));
	memset (ipafHead, 0x00, sizeof(ipafHead));

    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_DELAY_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_DELAY_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_DELAY_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_DELAY_TBL_NAME);
    }

	for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
		rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
	}
	for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
		rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
	}

	if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
		memset(stime, 0, 33);
		memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(stime, "-"); strcat(stime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(stime, "-"); strcat(stime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(stime, " "); strcat(stime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(stime, ":"); strcat(stime, resTmp);
					}
				}
			}
		}
	}
	if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
		memset(etime, 0, 33);
		memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(etime, "-"); strcat(etime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(etime, "-"); strcat(etime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(etime, " "); strcat(etime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(etime, ":"); strcat(etime, resTmp);
					}
				}
			}
		}
	}
	strcpy(cmdName, "srch-stat-delay");

	sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
			rxReqMsg->head.para[1].paraVal,stime,etime);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%3s %15s %8s %-12s %-12s %-12s","", title[0],title[1],title[2],title[3],title[4] );
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);

	strcat(ipafBuf, ipafHead);

	snd_cnt=1;
	for(i=0; i<realSysCnt; i++)
	{ 
		if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
			if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
				continue;
		}

		sprintf(query, "SELECT stat_date, system_name, "
				" IFNULL(ROUND(MIN(min_usec),6), 0.0), IFNULL(ROUND(MAX(max_usec),6), 0.0), "
				" IFNULL(ROUND(AVG(avg_usec),6), 0.0) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by stat_date, system_name order by stat_date, system_name ",table_name
				, SysName[i], stime, etime );

		if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
			sprintf(trcBuf, "query = %s\n", query);
			trclib_writeLog(FL, trcBuf);
		}
		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		select_cnt=0; min = 0.0; max = 0.0; avg = 0.0; sum = 0.0;
		memset(last_time,0x00,sizeof(last_time));
		row_cnt = 0;                                                                                  
		result = mysql_store_result(conn);

		while( (row = mysql_fetch_row(result)) != NULL)
		{
			if(strlen(row[0]) == 19){
				memset(key_time,0,sizeof(key_time));
				strncpy(key_time,row[0]+5,14);
			}
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

			row_index = 2;
			if ( row_cnt == 0 )
			{
				sprintf(resBuf, "%3s %15s %8s %-12s %-12s %-12s\n",
						"", key_time, SysName[i], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
			}
			else
			{
				sprintf(resBuf, "%3s %15s %8s %-12s %-12s %-12s\n",
						"", "", SysName[i], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
			}
			if( min > strtod(row[row_index],0) ) 
				min = strtod(row[row_index],0);

			if( max < strtod(row[row_index+1],0) )
				max = strtod(row[row_index+1],0);

			sum += strtod(row[row_index+2],0);

			strcpy(last_time, key_time);

			select_cnt++;
			tcnt++;

			if(strlen(ipafBuf) > 2500)
			{
				stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
				snd_cnt++;
				memset (ipafBuf, 0x00, sizeof(ipafBuf));
			}
			row_cnt++;
		}
		mysql_free_result(result);

		sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

		if (select_cnt > 0)
		{
			row_index = 0;
			avg = sum / select_cnt;
			sprintf(resTmp, "%3s %15s %8s %-12.6f %-12.6f %-12.6f\n",
					"", "TOTAL", SysName[i], min, max, avg);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);
		}
			///////////////////////////
	}

	if (tcnt == 0) 
	{
		sprintf(resTmp, "    NO DATA\n");
		strcat(ipafBuf, resTmp);
		sprintf(resTmp, "    ====================================================================================================\n");
		strcat(ipafBuf,resTmp);
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
	} 
	else
	{
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
	}

	return 1;
}



int stmd_mmc_srch_stat_logon(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;

	char        query[4096], query_head[4096], query_pdsn[1024];
	char        cmdName[64];
	MYSQL_RES   *result, *res_pd;
	MYSQL_ROW   row, row_pd;

	char  	table_name[30];
	char  	stime[33]; char etime[33];
	int   	i_tmp;
	char  	*env, fname[256], tmp[64];
	int   	row_cnt=0;
	char  	key_time[33], last_time[33] = {0,};

	char    itemRes[10][10];
	int     i,j,k, ridx, toResult;
	char *title[]={"STATTIME", "SYSTEM", "LOG_REQ", "LOG_SUCC","LOG_FAIL"};
	char *title1[]={"", "", "HBIT_0","HBIT_1","HBIT_2", "HBIT_3"};
	char *title2[]={"", "", "SM_INT_ERR","OP_ERR","OP_TIMEOUT", "ETC_FAIL"};
	int 	row_index;
	char 	SysName[2][8]={"SCMA","SCMB"};
	char 	SysIp[2][16];
	int 	realSysCnt =2;
	int 	realItemCnt =0;
	int		index, select_cnt = 0, snd_cnt, tcnt=0;
	unsigned int logTotal[2][11] = {{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0}};

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	memset (ipafBuf, 0x00, sizeof(ipafBuf));
	memset (ipafHead, 0x00, sizeof(ipafHead));

    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_LOGON_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_LOGON_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_LOGON_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_LOGON_TBL_NAME);
    }

	for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
		rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
	}
	for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
		rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
	}

	if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
		memset(stime, 0, 33);
		memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(stime, "-"); strcat(stime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(stime, "-"); strcat(stime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(stime, " "); strcat(stime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(stime, ":"); strcat(stime, resTmp);
					}
				}
			}
		}
	}
	if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
		memset(etime, 0, 33);
		memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(etime, "-"); strcat(etime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(etime, "-"); strcat(etime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(etime, " "); strcat(etime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(etime, ":"); strcat(etime, resTmp);
					}
				}
			}
		}
	}
	strcpy(cmdName, "srch-stat-logon");

	sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
			rxReqMsg->head.para[1].paraVal,stime,etime);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-12s %-12s %-12s","","", title[0],title[1],title[2],title[3],title[4] );
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-12s %-12s %-12s %-12s", "","", "",title1[2],title1[3],title1[4],title1[5]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-12s %-12s %-12s %-12s", "","", "",title2[2],title2[3],title2[4],title2[5]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);

	strcat(ipafBuf, ipafHead);

	snd_cnt=1;
	for(i=0; i<realSysCnt; i++)
	{ 
		if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
			if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
				continue;
		}

		sprintf(query, "SELECT stat_date, system_name, "
				" IFNULL(SUM(log_req), 0), IFNULL(SUM(log_succ), 0), IFNULL(SUM(log_fail), 0), "
				" IFNULL(SUM(HBIT_0), 0), IFNULL(SUM(HBIT_1), 0), IFNULL(SUM(HBIT_2), 0), "
				" IFNULL(SUM(HBIT_3), 0) "
				" IFNULL(SUM(sm_int_err), 0), IFNULL(SUM(op_err), 0), IFNULL(SUM(op_timeout), 0) "
				" IFNULL(SUM(etc_fail), 0) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by stat_date, system_name order by stat_date, system_name ",table_name
				, SysName[i], stime, etime );

		if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
			sprintf(trcBuf, "query = %s\n", query);
			trclib_writeLog(FL, trcBuf);
		}
		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
		row_cnt = 0;                                                                                  
		result = mysql_store_result(conn);

		while( (row = mysql_fetch_row(result)) != NULL)
		{
			if(strlen(row[0]) == 19){
				memset(key_time,0,sizeof(key_time));
				strncpy(key_time,row[0]+5,14);
			}
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

			row_index = 2;
			if ( row_cnt == 0 )
			{
				sprintf(resBuf, "%2s %15s %6s %-12s %-12s %-12s\n",
						"", key_time, SysName[i], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+3], row[row_index+4], row[row_index+5], row[row_index+6]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+7], row[row_index+8], row[row_index+9]);
				strcat(ipafBuf, resBuf);
			}
			else
			{
				sprintf(resBuf, "%2s %15s %6s %-12s %-12s %-12s\n",
						"", "", SysName[i], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+3], row[row_index+4], row[row_index+5], row[row_index+6]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+7], row[row_index+8], row[row_index+9]);
				strcat(ipafBuf, resBuf);
			}
			logTotal[row_cnt][0] += atoi(row[row_index]); 
			logTotal[row_cnt][1] += atoi(row[row_index+1]); 
			logTotal[row_cnt][2] += atoi(row[row_index+2]);
			logTotal[row_cnt][3] += atoi(row[row_index+3]); 
			logTotal[row_cnt][4] += atoi(row[row_index+4]); 
			logTotal[row_cnt][5] += atoi(row[row_index+5]);
			logTotal[row_cnt][6] += atoi(row[row_index+6]);
			logTotal[row_cnt][7] += atoi(row[row_index+7]);
			logTotal[row_cnt][8] += atoi(row[row_index+8]);
			logTotal[row_cnt][9] += atoi(row[row_index+9]);
			logTotal[row_cnt][10] += atoi(row[row_index+10]);

			strcpy(last_time, key_time);

			select_cnt++;
			tcnt++;

			if(strlen(ipafBuf) > 2500)
			{
				stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
				snd_cnt++;
				memset (ipafBuf, 0x00, sizeof(ipafBuf));
			}
			row_cnt++;
		}
		mysql_free_result(result);

		sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

		if (select_cnt > 0)
		{
			row_index = 0;
			sprintf(resTmp, "%2s %15s %6s %-12d %-12d %-12d\n",
					"", "SUM",SysName[i], logTotal[i][0], logTotal[i][1], logTotal[i][2]);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "%2s %15s %6s %-12d %-12d %-12d %-12d\n",
					"", "SUM","", logTotal[i][3], logTotal[i][4], logTotal[i][5],logTotal[i][6]);
			sprintf(resTmp, "%2s %15s %6s %-12d %-12d %-12d %-12d\n",
					"", "SUM","", logTotal[i][7], logTotal[i][8], logTotal[i][9],logTotal[i][10]);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < 11; j++)
			{
				logTotal[i][j] = 0;
			}
		}
			///////////////////////////
	}

	if (tcnt == 0) 
	{
		sprintf(resTmp, "    NO DATA\n");
		strcat(ipafBuf, resTmp);
		sprintf(resTmp, "    ====================================================================================================\n");
		strcat(ipafBuf,resTmp);
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
	} 
	else
	{
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
	}

	return 1;
}

int stmd_mmc_srch_stat_sms(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;

	char        query[4096], query_head[4096], query_pdsn[1024];
	char        cmdName[64];
	MYSQL_RES   *result, *res_pd;
	MYSQL_ROW   row, row_pd;

	char  	table_name[30];
	char  	stime[33]; char etime[33];
	int   	i_tmp;
	char  	*env, fname[256], tmp[64];
	int   	row_cnt=0;
	char  	key_time[33], last_time[33] = {0,};

	char    itemRes[10][10];
	int     i,j,k, ridx, toResult;
	char *title[]={"STATTIME", "SYSTEM", "ITEM" "REQ", "SUCC","FAIL"};
	char *title1[]={"", "", "", "SMPP_ERR","SVR_ERR","SMSC_ERR","ETC_ERR"};
	int 	row_index;
	char 	SysName[2][8]={"SCMA","SCMB"};
	char 	SysIp[2][16];
	int 	realSysCnt =2;
	int 	realItemCnt =0;
	int		index, select_cnt = 0, snd_cnt, tcnt=0;
	unsigned int	smsTotal[2][7] = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}};

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	memset (ipafBuf, 0x00, sizeof(ipafBuf));
	memset (ipafHead, 0x00, sizeof(ipafHead));

    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_SMS_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_SMS_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_SMS_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_SMS_TBL_NAME);
    }

	for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
		rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
	}
	for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
		rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
	}

	if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
		memset(stime, 0, 33);
		memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(stime, "-"); strcat(stime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(stime, "-"); strcat(stime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(stime, " "); strcat(stime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(stime, ":"); strcat(stime, resTmp);
					}
				}
			}
		}
	}
	if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
		memset(etime, 0, 33);
		memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(etime, "-"); strcat(etime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(etime, "-"); strcat(etime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(etime, " "); strcat(etime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(etime, ":"); strcat(etime, resTmp);
					}
				}
			}
		}
	}
	strcpy(cmdName, "srch-stat-sms");

	sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
			rxReqMsg->head.para[1].paraVal,stime,etime);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-18s %-12s %-12s %-12s\n","",title[0],title[1],title[2],title[3],title[4]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-18s %-12s %-12s %-12s %-12s\n","","","","",title1[3],title1[4],title1[5],title1[6]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);

	strcat(ipafBuf, ipafHead);

	snd_cnt=1;
	for(i=0; i<realSysCnt; i++)
	{ 
		if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
			if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
				continue;
		}

		// realItemCnt : pdsn
		sprintf(query, "select system_name, sum(cnt) list "
				" from ( "
				" select system_name, smsc_ip, count(*) cnt "
				" from %s where system_name = '%s' and stat_date > '%s' AND stat_date <= '%s' "
				" group by system_name, smsc_ip ) a",
				table_name, SysName[i], stime,etime);

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
		result = mysql_store_result(conn);
		row = mysql_fetch_row(result);
		if( row == NULL )
		{
			realItemCnt = 0;
			mysql_free_result(result);
			continue;
		}
		else
		{
			if(row[1] == NULL)
			{
				mysql_free_result(result);
				continue;
			}
			else
			{
				realItemCnt = atoi(row[1]);
			}
		}
		sprintf(query, "SELECT stat_date, system_name, smsc_ip, "
				" IFNULL(SUM(req), 0), IFNULL(SUM(succ), 0), IFNULL(SUM(fail), 0), "
				" IFNULL(SUM(smpp_err), 0), "
				" IFNULL(SUM(svr_err), 0), IFNULL(SUM(smsc_err), 0), "
				" IFNULL(SUM(etc_err), 0) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by stat_date, system_name, smsc_ip order by stat_date, system_name, smsc_ip ",
				table_name, SysName[i], stime, etime );

		if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
			sprintf(trcBuf, "query = %s\n", query);
			trclib_writeLog(FL, trcBuf);
		}
		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
		row_cnt = 0;                                                                                  
		result = mysql_store_result(conn);

		while( (row = mysql_fetch_row(result)) != NULL)
		{
			if(strlen(row[0]) == 19){
				memset(key_time,0,sizeof(key_time));
				strncpy(key_time,row[0]+5,14);
			}
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

			row_index = 2;
			if ( row_cnt == 0 )
			{
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s\n",
						"", key_time, SysName[i], row[1], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5],row[row_index+6]);
				strcat(ipafBuf, resBuf);
			}
			else
			{
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s\n",
						"", "", "", row[1], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5],row[row_index+6]);
				strcat(ipafBuf, resBuf);
			}
			smsTotal[row_cnt][0] += atoi(row[row_index]); 
			smsTotal[row_cnt][1] += atoi(row[row_index+1]); 
			smsTotal[row_cnt][2] += atoi(row[row_index+2]);
			smsTotal[row_cnt][3] += atoi(row[row_index+3]); 
			smsTotal[row_cnt][4] += atoi(row[row_index+4]); 
			smsTotal[row_cnt][5] += atoi(row[row_index+5]);
			smsTotal[row_cnt][6] += atoi(row[row_index+6]);

			strcpy(last_time, key_time);

			select_cnt++;
			tcnt++;

			if(strlen(ipafBuf) > 2500)
			{
				stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
				snd_cnt++;
				memset (ipafBuf, 0x00, sizeof(ipafBuf));
			}
			row_cnt++;
		}
		mysql_free_result(result);

		sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

		if (select_cnt > 0)
		{
			row_index = 0;
			sprintf(resTmp, "%2s %15s %6s %-18s %-12d %-12d %-12d\n",
					"", "SUM",SysName[i], "", 
					smsTotal[i][0], smsTotal[i][1], smsTotal[i][2]);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "%2s %15s %6s %-18s %-12d %-12d %-12d %-12d\n",
					"", "SUM","", "",
					smsTotal[i][3],smsTotal[i][4], smsTotal[i][5], smsTotal[i][6]);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < 7; j++)
			{
				smsTotal[i][j] = 0;
			}
		}
			///////////////////////////
	}

	if (tcnt == 0) 
	{
		sprintf(resTmp, "    NO DATA\n");
		strcat(ipafBuf, resTmp);
		sprintf(resTmp, "    ====================================================================================================\n");
		strcat(ipafBuf,resTmp);
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
	} 
	else
	{
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
	}

	return 1;
}



int stmd_mmc_srch_stat_leg(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;

	char        query[4096], query_head[4096], query_pdsn[1024];
	char        cmdName[64];
	MYSQL_RES   *result, *res_pd;
	MYSQL_ROW   row, row_pd;

	char  	table_name[30];
	char  	stime[33]; char etime[33];
	int   	i_tmp;
	char  	*env, fname[256], tmp[64];
	int   	row_cnt=0;
	char  	key_time[33], last_time[33] = {0,};

	char    itemRes[10][10];
	int     i,j,k, ridx, toResult;
	char *title[]={"STATTIME", "SYSTEM", "PDSN_IP", "RX_CNT", "START","INTERIM","STOP"};
	char *title1[]={"", "", "", "START_LOGON","INTERIM_LOGON","LOG_OUT"};
	int 	row_index;
	char 	SysName[2][8]={"SCMA","SCMB"};
	char 	SysIp[2][16];
	int 	realSysCnt =2;
	int 	realItemCnt =0;
	int		index, select_cnt = 0, snd_cnt, tcnt=0;
	unsigned int	legTotal[2][7] = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}};

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	memset (ipafBuf, 0x00, sizeof(ipafBuf));
	memset (ipafHead, 0x00, sizeof(ipafHead));

    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_LEG_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_LEG_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_LEG_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_LEG_TBL_NAME);
    }

	for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
		rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
	}
	for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
		rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
	}

	if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
		memset(stime, 0, 33);
		memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(stime, "-"); strcat(stime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(stime, "-"); strcat(stime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(stime, " "); strcat(stime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(stime, ":"); strcat(stime, resTmp);
					}
				}
			}
		}
	}
	if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
		memset(etime, 0, 33);
		memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(etime, "-"); strcat(etime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(etime, "-"); strcat(etime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(etime, " "); strcat(etime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(etime, ":"); strcat(etime, resTmp);
					}
				}
			}
		}
	}
	strcpy(cmdName, "srch-stat-account");

	sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
			rxReqMsg->head.para[1].paraVal,stime,etime);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-18s %-12s %-12s %-12s %-12s\n","",title[0],title[1],title[2],title[3],title[4],title[5],title[6]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%2s %15s %6s %-18s %-12s %-12s %-12s\n","","","","",title1[3],title1[4],title1[5]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);

	strcat(ipafBuf, ipafHead);

	snd_cnt=1;
	for(i=0; i<realSysCnt; i++)
	{ 
		if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
			if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
				continue;
		}

		// realItemCnt : pdsn
		sprintf(query, "select system_name, sum(cnt) list "
				" from ( "
				" select system_name, pdsn_ip, count(*) cnt "
				" from %s where system_name = '%s' and stat_date > '%s' AND stat_date <= '%s' "
				" group by system_name, pdsn_ip ) a",
				table_name, SysName[i], stime,etime);

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
		result = mysql_store_result(conn);
		row = mysql_fetch_row(result);
		if( row == NULL )
		{
			realItemCnt = 0;
			mysql_free_result(result);
		}
		else
		{
			if( row[1] != NULL )
				realItemCnt = atoi(row[1]);
			else
			{
				realItemCnt = 1;
			}
			mysql_free_result(result);
		}
		sprintf(query, "SELECT stat_date, system_name, pdsn_ip, "
				" IFNULL(SUM(rx_cnt), 0), IFNULL(SUM(start), 0), IFNULL(SUM(interim), 0), "
				" IFNULL(SUM(stop), 0), "
				" IFNULL(SUM(start_logon_cnt), 0), IFNULL(SUM(int_logon_cnt), 0), "
				" IFNULL(SUM(logout_cnt), 0) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by stat_date, system_name, pdsn_ip order by stat_date, system_name, pdsn_ip ",table_name
				, SysName[i], stime, etime );

		if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
			sprintf(trcBuf, "query = %s\n", query);
			trclib_writeLog(FL, trcBuf);
		}
		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
		row_cnt = 0;                                                                                  
		result = mysql_store_result(conn);

		while( (row = mysql_fetch_row(result)) != NULL)
		{
			if(strlen(row[0]) == 19){
				memset(key_time,0,sizeof(key_time));
				strncpy(key_time,row[0]+5,14);
			}
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

			row_index = 3;
			if ( row_cnt == 0 )
			{
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", key_time, row[1], row[2],row[row_index], row[row_index+1], row[row_index+2],row[row_index+3]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s\n",
						"", "", "", "", row[row_index+4], row[row_index+5], row[row_index+6]);
				strcat(ipafBuf, resBuf);
			}
			else
			{
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", row[1], row[2], row[row_index], row[row_index+1], row[row_index+2],row[row_index+3]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%2s %15s %6s %-18s %-12s %-12s %-12s\n",
						"", "", "", "", row[row_index+4], row[row_index+5], row[row_index+6]);
				strcat(ipafBuf, resBuf);
			}
			legTotal[row_cnt][0] += atoi(row[row_index]); 
			legTotal[row_cnt][1] += atoi(row[row_index+1]); 
			legTotal[row_cnt][2] += atoi(row[row_index+2]);
			legTotal[row_cnt][3] += atoi(row[row_index+3]); 
			legTotal[row_cnt][4] += atoi(row[row_index+4]); 
			legTotal[row_cnt][5] += atoi(row[row_index+5]);
			legTotal[row_cnt][6] += atoi(row[row_index+6]);

			strcpy(last_time, key_time);

			select_cnt++;
			tcnt++;

			if(strlen(ipafBuf) > 2500)
			{
				stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
				snd_cnt++;
				memset (ipafBuf, 0x00, sizeof(ipafBuf));
			}
			row_cnt++;
		}
		mysql_free_result(result);

		sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

		if (select_cnt > 0)
		{
			row_index = 0;
			sprintf(resTmp, "%2s %15s %6s %-18s %-12d %-12d %-12d %-12d\n",
					"", "SUM",SysName[i], "", 
					legTotal[i][0], legTotal[i][1], legTotal[i][2], legTotal[i][3]);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "%2s %15s %6s %-18s %-12d %-12d %-12d\n",
					"", "","", "",
					legTotal[i][4], legTotal[i][5], legTotal[i][6]);
			strcat(ipafBuf, resTmp);
			sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < 7; j++)
			{
				legTotal[i][j] = 0;
			}
		}
			///////////////////////////
	}

	if (tcnt == 0) 
	{
		sprintf(resTmp, "    NO DATA\n");
		strcat(ipafBuf, resTmp);
		sprintf(resTmp, "    ====================================================================================================\n");
		strcat(ipafBuf,resTmp);
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
	} 
	else
	{
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
	}

	return 1;
}


int stmd_mmc_srch_stat_link(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;

	char        query[4096], query_head[4096];
	char        cmdName[64];
	MYSQL_RES   *result;
	MYSQL_ROW   row;

	char  	table_name[30];
	char  	stime[33]; char etime[33];
	int   	i_tmp;
	char  	*env, fname[256], tmp[64];
	int   	row_cnt=0;
	char  	key_time[33], last_time[33] = {0,};

	char    itemRes[10][10];
	int     i,j,k, ridx, toResult;
	char 	*title[] = {"STATTIME", "SYSTEM", "ITEM", "UpStream(Mbps)", "DnStream(Mbps)", "Total(Mbps)"};
	char 	*title1[] = {"", "", "", "UpBytes(MBytes)", "DnBytes(MBytes)", "Total(MBytes)"};
	int 	row_index;
	char 	SysName[SCE_CNT][8];
	char 	SysIp[SCE_CNT][16];
	int 	realSysCnt =0;
	int 	realItemCnt =0;
	int		index, select_cnt = 0, snd_cnt, tcnt=0;
	char	linkName[2][10] = {"Link 0", "Link 1"};
	long double linkTotal[2][6] = {{0.0L,0.0L,0.0L,0.0L,0.0L,0.0L},{0.0L,0.0L,0.0L,0.0L,0.0L,0.0L}};

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	memset (ipafBuf, 0x00, sizeof(ipafBuf));
	memset (ipafHead, 0x00, sizeof(ipafHead));

	realItemCnt = 2;

	for(i=0; i<SCE_CNT; i++ )
	{
		strcpy(SysName[i], g_stSCE[i].sce_name);
		strcpy(SysIp[i], g_stSCE[i].sce_ip);
		realSysCnt++;
	}

	for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
		rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
	}
	for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
		rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
	}

	if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
	{
		strcpy(table_name,STM_STATISTIC_5MINUTE_LUR_TBL_NAME);
		sprintf(query_head, "SELECT stat_date, link_id, "
				" round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
				" round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
				" round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
				" round(sum(upstream_volume)/1024,2), "
				" round(sum(downstream_volume)/1024,2), "
				" round(sum(upstream_volume+downstream_volume)/1024,2) "
				" from %s ", table_name);
	}
	else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
	{
		strcpy(table_name,STM_STATISTIC_HOUR_LUR_TBL_NAME);
		sprintf(query_head, "SELECT stat_date, link_id, "
				" round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
				" round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
				" round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
				" round(sum(upstream_volume)/1024,2), "
				" round(sum(downstream_volume)/1024,2), "
				" round(sum(upstream_volume+downstream_volume)/1024,2) "
				" from %s ", table_name);
	}
	else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
	{
		strcpy(table_name,STM_STATISTIC_DAY_LUR_TBL_NAME);
		sprintf(query_head, "SELECT stat_date, link_id, "
				" round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
				" round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
				" round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
				" round(sum(upstream_volume)/1024,2), "
				" round(sum(downstream_volume)/1024,2), "
				" round(sum(upstream_volume+downstream_volume)/1024,2) "
				" from %s ", table_name);
	}
	else
	{
		strcpy(table_name,STM_STATISTIC_MONTH_LUR_TBL_NAME);
		sprintf(query_head, "SELECT stat_date, link_id, "
				" round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
				" round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
				" round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
				" round(sum(upstream_volume)/1024,2), "
				" round(sum(downstream_volume)/1024,2), "
				" round(sum(upstream_volume+downstream_volume)/1024,2) "
				" from %s ", table_name);
	}

	if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
		memset(stime, 0, 33);
		memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(stime, "-"); strcat(stime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(stime, "-"); strcat(stime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(stime, " "); strcat(stime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(stime, ":"); strcat(stime, resTmp);
					}
				}
			}
		}
	}
	if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
		memset(etime, 0, 33);
		memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
		memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
		if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
			strcat(etime, "-"); strcat(etime, resTmp);
			memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
			if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
				strcat(etime, "-"); strcat(etime, resTmp);
				memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
				if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
					strcat(etime, " "); strcat(etime, resTmp);
					memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
					if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
						i_tmp = atoi(resTmp);
						i_tmp = i_tmp - (i_tmp%5);
						sprintf(resTmp, "%02d", i_tmp); 
						strcat(etime, ":"); strcat(etime, resTmp);
					}
				}
			}
		}
	}
	strcpy(cmdName, "srch-stat-link");

	sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
			rxReqMsg->head.para[1].paraVal,stime,etime);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%3s %15s %6s %-10s %15s %15s %15s\n","",title[0],title[1],title[2],title[3],title[4],title[5]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "%3s %15s %6s %-10s %15s %15s %15s\n","","","","",title1[3],title1[4],title1[5]);
	strcat(ipafHead, resTmp);
	sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafHead, resTmp);

	strcat(ipafBuf, ipafHead);

	snd_cnt=1;
	for(i=0; i<realSysCnt; i++)
	{ 
		if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
			if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
				continue;
		}

		sprintf(query, "%s "
				" where record_source = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" Group By stat_date, link_id ORDER BY stat_date, link_id ", 
				query_head, SysIp[i], stime, etime );

		if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
			sprintf(trcBuf, "query = %s\n", query);
			trclib_writeLog(FL, trcBuf);
		}
		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
		row_cnt = 0;                                                                                  
		result = mysql_store_result(conn);

		while( (row = mysql_fetch_row(result)) != NULL)
		{
			if(strlen(row[0]) == 19){
				memset(key_time,0,sizeof(key_time));
				strncpy(key_time,row[0]+5,14);
			}
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

			index = atoi(row[1]); // 배열 index 
			row_index = 2;
			if ( row_cnt == 0 )
			{
				sprintf(resBuf, "%3s %15s %6s %-10s %15s %15s %15s\n",
						"", key_time, SysName[i], linkName[index], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%3s %15s %6s %-10s %15s %15s %15s\n",
						"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
				strcat(ipafBuf, resBuf);
			}
			else
			{
				sprintf(resBuf, "%3s %15s %6s %-10s %15s %15s %15s\n",
						"", "", "", linkName[index], row[row_index], row[row_index+1], row[row_index+2]);

				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%3s %15s %6s %-10s %15s %15s %15s\n",
						"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
				strcat(ipafBuf, resBuf);
			}
			linkTotal[index][0] += strtold(row[row_index],0); 
			linkTotal[index][1] += strtold(row[row_index+1],0); 
			linkTotal[index][2] += strtold(row[row_index+2],0);
			linkTotal[index][3] += strtold(row[row_index+3],0); 
			linkTotal[index][4] += strtold(row[row_index+4],0); 
			linkTotal[index][5] += strtold(row[row_index+5],0);

			strcpy(last_time, key_time);

			select_cnt++;
			tcnt++;

			if(strlen(ipafBuf) > 2500)
			{
				stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
				snd_cnt++;
				memset (ipafBuf, 0x00, sizeof(ipafBuf));
			}
			row_cnt++;
		}
		mysql_free_result(result);

		sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

		if (select_cnt > 0)
		{
			row_index = 0;
			for(j=0;j<realItemCnt;j++)
			{
				sprintf(resTmp, "%3s %15s %6s %-10s %15.2Lf %15.2Lf %15.2Lf\n",
						"", "SUM(Thru)",SysName[i], linkName[j], 
						linkTotal[j][0], linkTotal[j][1], linkTotal[j][2]);

				strcat(ipafBuf, resTmp);
				sprintf(resTmp, "%3s %15s %6s %-10s %15.2Lf %15.2Lf %15.2Lf\n",
						"", "SUM(Bytes)",SysName[i], linkName[j],
						linkTotal[j][3], linkTotal[j][4], linkTotal[j][5]);
				strcat(ipafBuf, resTmp);
			}
			sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < 6; j++)
			{
				linkTotal[i][j] = 0.0L;
			}
		}
			///////////////////////////
	}

	if (tcnt == 0) 
	{
		sprintf(resTmp, "    NO DATA\n");
		strcat(ipafBuf, resTmp);
		sprintf(resTmp, "    ====================================================================================================\n");
		strcat(ipafBuf,resTmp);
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
	} 
	else
	{
		stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
	}

	return 1;
}


int stmd_mmc_stat_rule_ent(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_RULE_ENT;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandRuleEnt(list);
    }
}


int stmd_mmc_srch_stat_rule_ent(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096], query_head[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;

    char  	table_name[30];
    char  	stime[33]; char etime[33];
    int   	i_tmp;
    char  	*env, fname[256], tmp[64];
    int   	row_cnt=0;
    char  	key_time[33], last_time[33] = {0,};

    char    itemRes[10][10];
    int     i,j,k, ridx, toResult;
//	char 	*title[] = {"STATTIME", "SYSTEM", "ITEM", "Session", "Block", "Redirect"};
	char 	*title[] = {"STATTIME", "SYSTEM", "ITEM", "Session", "Block", "Total"};
	char 	*title1[] = {"", "", "", "UpStream(Mbps)", "DnStream(Mbps)", "Total(Mbps)"};
	char 	*title2[] = {"", "", "", "UpByte(MBytes)", "DnByte(MBytes)", "Total(MBytes)"};
//	char 	*title3[] = {"", "", "", "Active User", "Total Active User", "Usage(%)"};
    int 	row_index;
    char 	SysName[SCE_CNT][8];
    char 	SysIp[SCE_CNT][16];
    int 	realSysCnt =0;
    int 	realItemCnt =0;
	char	tmpRule[5][512];
	int		ruleSet[5], index;
    int     select_cnt = 0, snd_cnt,tcnt=0;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_RULEENT_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, rule_ent_id, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0), IFNULL(SUM(session),0)+IFNULL(SUM(block_cnt),0), "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_RULEENT_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, rule_ent_id, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0),IFNULL(SUM(session),0)+IFNULL(SUM(block_cnt),0), "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_RULEENT_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, rule_ent_id, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0), IFNULL(SUM(session),0)+IFNULL(SUM(block_cnt),0), "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_RULEENT_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, rule_ent_id, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0), IFNULL(SUM(session),0)+IFNULL(SUM(block_cnt),0), "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-rule-ent");

    sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
        rxReqMsg->head.para[1].paraVal,stime,etime);
    sprintf(resTmp, "    ====================================================================================================\n");
	strcat(ipafBuf, resTmp);
	sprintf(resTmp, "%3s %6s %-28s %-15s %-15s %-15s\n","",title[0],title[1],title[2],title[3],title[4]);
	strcat(ipafBuf, resTmp);
	sprintf(resTmp, "%3s %6s %-28s %-15s %-15s %-15s\n","","","",title1[2],title1[3],title1[4]);
	strcat(ipafBuf, resTmp);
	sprintf(resTmp, "%3s %6s %-28s %-15s %-15s %-15s\n","","","",title2[2],title2[3],title2[4]);
	strcat(ipafBuf, resTmp);
    sprintf(resTmp, "    ====================================================================================================\n");
    strcat(ipafHead, resTmp);

    strcat(ipafBuf, ipafHead);


	for(i=0; i<SCE_CNT; i++ )
	{
		strcpy(SysName[i], g_stSCE[i].sce_name);
		strcpy(SysIp[i], g_stSCE[i].sce_ip);
		realSysCnt++;
	}

    snd_cnt=1;
    for(i=0; i<realSysCnt; i++)
	{ 
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
                continue;
        }

		realItemCnt = g_stSCEEntry[i].ruleEntryCnt;

        sprintf(query, "%s "
                " where record_source = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" Group By stat_date, rule_ent_id ORDER BY stat_date, rule_ent_id ", 
				query_head, SysIp[i], stime, etime );

        if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
        row_cnt = 0;                                                                                  
        result = mysql_store_result(conn);

        while( (row = mysql_fetch_row(result)) != NULL)
        {
			toResult = 0;
            if(strlen(row[0]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[0]+5,14);
            }
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

            index = atoi(row[1]); // 배열 index 
			
			row_index = 2;
			if ( row_cnt == 0 )
			{
				sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
						"", key_time, SysName[i], g_stSCEEntry[i].stEntry[index].eName, row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
						"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
						"", "", "", "", row[row_index+6], row[row_index+7], row[row_index+8]);
				strcat(ipafBuf, resBuf);
			}
			else
			{
				sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
						"", "", "", g_stSCEEntry[i].stEntry[index].eName, row[row_index], row[row_index+1], row[row_index+2]);

				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
						"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
				strcat(ipafBuf, resBuf);
				sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
						"", "", "", "", row[row_index+6], row[row_index+7], row[row_index+8]);
				strcat(ipafBuf, resBuf);
			}
			g_stSCEEntry[i].stEntry[index].unblk += atoi(row[row_index]); 
			g_stSCEEntry[i].stEntry[index].blk += atoi(row[row_index+1]); 
			g_stSCEEntry[i].stEntry[index].tot += atoi(row[row_index+2]);
			g_stSCEEntry[i].stEntry[index].uThru += strtold(row[row_index+3],0); 
			g_stSCEEntry[i].stEntry[index].dThru += strtold(row[row_index+4],0); 
			g_stSCEEntry[i].stEntry[index].tThru += strtold(row[row_index+5],0);
			g_stSCEEntry[i].stEntry[index].uByte += strtold(row[row_index+6],0); 
			g_stSCEEntry[i].stEntry[index].dByte += strtold(row[row_index+7],0); 
			g_stSCEEntry[i].stEntry[index].tByte += strtold(row[row_index+8],0);

			strcpy(last_time, key_time);

			select_cnt++;
			tcnt++;

            if(strlen(ipafBuf) > 2500)
			{
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                snd_cnt++;
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            row_cnt++;
        }

        mysql_free_result(result);

    	sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

        if (select_cnt > 0)
		{
            if(strlen(ipafBuf) > 2500)
			{
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                snd_cnt++;
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
			row_index = 0;
			for(j=0;j<realItemCnt;j++)
			{
				if( g_stSCEEntry[i].stEntry[j].eName != NULL )
				{
					sprintf(resTmp, "%3s %17s %6s %-28s %15d %15d %15d\n",
							"", "SUM(Cnt)",SysName[i], g_stSCEEntry[i].stEntry[j].eName, g_stSCEEntry[i].stEntry[j].unblk, \
							g_stSCEEntry[i].stEntry[j].blk, g_stSCEEntry[i].stEntry[j].tot);
					strcat(ipafBuf, resTmp);
					sprintf(resTmp, "%3s %17s %6s %-28s %15.2Lf %15.2Lf %15.2Lf\n",
							"", "SUM(Thru)",SysName[i], g_stSCEEntry[i].stEntry[j].eName, g_stSCEEntry[i].stEntry[j].uThru, \
							g_stSCEEntry[i].stEntry[j].dThru, g_stSCEEntry[i].stEntry[j].tThru);
					strcat(ipafBuf, resTmp);
					sprintf(resTmp, "%3s %17s %6s %-28s %15.2Lf %15.2Lf %15.2Lf\n",
							"", "SUM(Bytes)",SysName[i], g_stSCEEntry[i].stEntry[j].eName, g_stSCEEntry[i].stEntry[j].uByte, \
							g_stSCEEntry[i].stEntry[j].dByte, g_stSCEEntry[i].stEntry[j].tByte);
					strcat(ipafBuf, resTmp);
				}
			}
    		sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < MAX_ENTRY_NUM; j++)
			{
				g_stSCEEntry[i].stEntry[j].unblk = 0;
				g_stSCEEntry[i].stEntry[j].blk = 0;
				g_stSCEEntry[i].stEntry[j].tot = 0;
				g_stSCEEntry[i].stEntry[j].uThru = 0;
				g_stSCEEntry[i].stEntry[j].dThru = 0;
				g_stSCEEntry[i].stEntry[j].tThru = 0;
				g_stSCEEntry[i].stEntry[j].uByte = 0;
				g_stSCEEntry[i].stEntry[j].dByte = 0;
				g_stSCEEntry[i].stEntry[j].tByte = 0;
			}
		}
///////////////////////////
    }

    if (tcnt == 0) 
	{
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
    	sprintf(resTmp, "    ====================================================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } 
	else
	{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;
}

#endif

#if 0
////////////
int stmd_mmc_stat_rule_thru(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_RULE_THRU;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandRuleThru(list);
    }
}


int stmd_mmc_srch_stat_rule_thru(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096], query_head[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;

    char  	table_name[30];
    char  	stime[33]; char etime[33];
    int   	i_tmp;
    char  	*env, fname[256], tmp[64];
    int   	row_cnt=0;
    char  	key_time[33], last_time[33] = {0,};

    char    itemRes[10][10];
    int     i,j,k, ridx, toResult;
    char 	*title[] = {"STATTIME", "SYSTEM", "ITEM", "UpStream(Mbps)", "DnStream(Mbps)", "Total(Mbps)"};
    char 	*title1[] = {"", "", "", "UpBytes(MBytes)", "DnBytes(MBytes)", "Total(MBytes)"};
    int 	row_index;
    char 	SysName[SCE_CNT][8];
    char 	SysIp[SCE_CNT][16];
    int 	realSysCnt =0;
    int 	realItemCnt =0;
	char	tmpRule[5][512];
	int		ruleSet[5], index;
    int     select_cnt = 0, snd_cnt,tcnt=0;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

	realItemCnt = g_ruleItemCnt;

	for(i=0; i<SCE_CNT; i++ )
	{
		strcpy(SysName[i], g_stSCE[i].sce_name);
		strcpy(SysIp[i], g_stSCE[i].sce_ip);
		realSysCnt++;
	}

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_PUR_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, pkg_usg_cnt_id, "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+300),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_PUR_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, pkg_usg_cnt_id, "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+3600),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_PUR_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, pkg_usg_cnt_id, "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_PUR_TBL_NAME);
        sprintf(query_head, "SELECT stat_date, pkg_usg_cnt_id, "
                " round(sum(upstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
                " round(sum(downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
                " round(sum(upstream_volume+downstream_volume)/1024/(unix_timestamp(max(stat_date))-unix_timestamp(min(stat_date))+86400*30),2), "
                " round(sum(upstream_volume)/1024,2), "
                " round(sum(downstream_volume)/1024,2), "
                " round(sum(upstream_volume+downstream_volume)/1024,2) "
                " from %s ", table_name);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-rule-thru");

    sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = (>)%s  -  (<=)%s\n",
        rxReqMsg->head.para[1].paraVal,stime,etime);
    sprintf(resTmp, "    ====================================================================================================\n");
    strcat(ipafHead, resTmp);
	sprintf(resTmp, "%3s %17s %6s %-28s %15s %15s %15s\n","",title[0],title[1],title[2],title[3],title[4],title[5]);
    strcat(ipafHead, resTmp);
	sprintf(resTmp, "%3s %17s %6s %-28s %15s %15s %15s\n","","","","",title[3],title[4],title[5]);
    strcat(ipafHead, resTmp);
    sprintf(resTmp, "    ====================================================================================================\n");
    strcat(ipafHead, resTmp);

    strcat(ipafBuf, ipafHead);

    snd_cnt=1;
    for(i=0; i<realSysCnt; i++)
	{ 
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
                continue;
        }

        sprintf(query, "%s "
                " where record_source = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" Group By stat_date, pkg_usg_cnt_id ORDER BY stat_date, pkg_usg_cnt_id ", 
				query_head, SysIp[i], stime, etime );

        if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
        row_cnt = 0;                                                                                  
        result = mysql_store_result(conn);

        while( (row = mysql_fetch_row(result)) != NULL)
        {
			toResult = 0;
            if(strlen(row[0]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[0]+5,14);
            }
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
				sprintf(resTmp, "    --------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

            index = atoi(row[1]); // 배열 index 
			for( ridx = 0; ridx < realItemCnt; ridx++ )
			{
				if( g_ruleIdBuf[ridx] == index && g_stSCERule[i].stRule[index].real == 1 )
				{
					toResult = 1;
					break;
				}
			}
			if( toResult == 1 )
			{
				row_index = 2;
				if ( row_cnt == 0 )
				{
					sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
							"", key_time, SysName[i], g_stSCERule[i].stRule[index].rName, row[row_index], row[row_index+1], row[row_index+2]);
					strcat(ipafBuf, resBuf);
					sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
							"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
					strcat(ipafBuf, resBuf);
				}
				else
				{
					sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
							"", "", "", g_stSCERule[i].stRule[index].rName, row[row_index], row[row_index+1], row[row_index+2]);

					strcat(ipafBuf, resBuf);
					sprintf(resBuf, "%3s %17s %6s %-28s %15s %15s %15s\n",
							"", "", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
					strcat(ipafBuf, resBuf);
				}
				g_stSCERule[i].stRule[index].uThru += strtold(row[row_index],0); 
				g_stSCERule[i].stRule[index].dThru += strtold(row[row_index+1],0); 
				g_stSCERule[i].stRule[index].tThru += strtold(row[row_index+2],0);
				g_stSCERule[i].stRule[index].uByte += strtold(row[row_index+3],0); 
				g_stSCERule[i].stRule[index].dByte += strtold(row[row_index+4],0); 
				g_stSCERule[i].stRule[index].tByte += strtold(row[row_index+5],0);

				strcpy(last_time, key_time);

				select_cnt++;
				tcnt++;
			}

            if(strlen(ipafBuf) > 2500)
			{
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                snd_cnt++;
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            row_cnt++;
        }

        mysql_free_result(result);

    	sprintf(resTmp, "    ----------------------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

        if (select_cnt > 0)
		{
			row_index = 0;
			for(j=0;j<realItemCnt;j++)
			{
				index = g_ruleIdBuf[j];
				if( g_ruleIdBuf[j] == index && g_stSCERule[i].stRule[index].real == 1 )
				{
					sprintf(resTmp, "%3s %17s %6s %-28s %15.2Lf %15.2Lf %15.2Lf\n",
							"", "SUM(Thru)",SysName[i], g_stSCERule[i].stRule[index].rName, g_stSCERule[i].stRule[index].uThru, \
							g_stSCERule[i].stRule[index].dThru, g_stSCERule[i].stRule[index].tThru);
					strcat(ipafBuf, resTmp);
					sprintf(resTmp, "%3s %17s %6s %-28s %15.2Lf %15.2Lf %15.2Lf\n",
							"", "SUM(Bytes)",SysName[i], g_stSCERule[i].stRule[index].rName, g_stSCERule[i].stRule[index].uByte, \
							g_stSCERule[i].stRule[index].dByte, g_stSCERule[i].stRule[index].tByte);
					strcat(ipafBuf, resTmp);
				}
			}
    		sprintf(resTmp, "    ====================================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < MAX_RULE_NUM; j++)
			{
				g_stSCERule[i].stRule[j].uThru = 0;
				g_stSCERule[i].stRule[j].dThru = 0;
				g_stSCERule[i].stRule[j].tThru = 0;
				g_stSCERule[i].stRule[j].uByte = 0;
				g_stSCERule[i].stRule[j].dByte = 0;
				g_stSCERule[i].stRule[j].tByte = 0;
			}
		}
///////////////////////////
    }

    if (tcnt == 0) 
	{
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
    	sprintf(resTmp, "    ====================================================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } 
	else
	{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;
}

////////////
int stmd_mmc_stat_rule_thru(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_RULE_THRU;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandRuleThru(list);
    }
}

int stmd_mmc_srch_stat_rule_usrcnt(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_cnt=0;
    int   tot_row[5][3]={0,}; // sum(unblock_cnt), sum(block_cnt), sum(unblock_cnt+block_cnt)
    char  key_time[33], last_time[33] = {0,};

    char        itemRes[10][10];
    int         i,j,k, ridx, toResult;
    char *title[] = {"STATTIME", "SYSTEM", "ITEM", "Active User", "Total User"};
    int row_index;
    char SysName[SCE_CNT][8];
    char SysIp[SCE_CNT][16];
    int realSysCnt =0;
    int realItemCnt =0;
	char	tmpRule[5][512];
	int		ruleSet[5], index;
    int     select_cnt = 0, snd_cnt,tcnt=0;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

	realItemCnt = g_ruleItemCnt;

	for(i=0; i<SCE_CNT; i++ )
	{
		strcpy(SysName[i], g_stSCE[i].sce_name);
		strcpy(SysIp[i], g_stSCE[i].sce_ip);
		realSysCnt++;
	}

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_PUR_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_PUR_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_PUR_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_PUR_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-rule-usrcnt");

    sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
        rxReqMsg->head.para[1].paraVal,stime,etime);
    sprintf(resTmp, "    ===============================================================================\n");
    strcat(ipafHead, resTmp);
	sprintf(resTmp, "%3s %17s %6s %-32s %9s %9s\n","",title[0],title[1],title[2],title[3],title[4]);
    strcat(ipafHead, resTmp);
    sprintf(resTmp, "    ===============================================================================\n");
    strcat(ipafHead, resTmp);

    strcat(ipafBuf, ipafHead);

    snd_cnt=1;
    for(i=0; i<realSysCnt; i++)
	{ 
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
                continue;
        }

        sprintf(query, "SELECT stat_date, pkg_usg_cnt_id, "
                " sum(active_subscribers), sum(total_active_subscribers) " 
                " from %s "
                " where record_source = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" Group By stat_date, pkg_usg_cnt_id ORDER BY stat_date, pkg_usg_cnt_id ", 
				table_name, SysIp[i], stime, etime );

        if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
        row_cnt = 0;                                                                                  
        result = mysql_store_result(conn);

        while( (row = mysql_fetch_row(result)) != NULL)
        {
			toResult = 0;
            if(strlen(row[0]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[0]+5,14);
            }
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
    			sprintf(resTmp, "    -------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

            index = atoi(row[1]); // 배열 index 
			for( ridx = 0; ridx < realItemCnt; ridx++ )
			{
				//if( g_ruleIdBuf[ridx] == index )
				if( g_ruleIdBuf[ridx] == index && g_stSCERule[i].stRule[index].real == 1 )
				{
					toResult = 1;
					break;
				}
			}
			if( toResult == 1 )
			{
				row_index = 2;
				if ( row_cnt == 0 )
				{
					sprintf(resBuf, "%3s %17s %6s %-32s %9s %9s\n",
							"", key_time, SysName[i], g_stSCERule[i].stRule[index].rName, row[row_index], row[row_index+1]);
					strcat(ipafBuf, resBuf);
				}
				else
				{
					sprintf(resBuf, "%3s %17s %6s %-32s %9s %9s\n",
							"", "", "", g_stSCERule[i].stRule[index].rName, row[row_index], row[row_index+1]);

					strcat(ipafBuf, resBuf);
				}
				g_stSCERule[i].stRule[index].actSub += atoi(row[row_index]); 
				g_stSCERule[i].stRule[index].totSub += atoi(row[row_index+1]); 

				strcpy(last_time, key_time);

				select_cnt++;
				tcnt++;
			}

            if(strlen(ipafBuf) > 2500)
			{
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                snd_cnt++;
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            row_cnt++;
        }

        mysql_free_result(result);

		sprintf(resTmp, "    -------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

        if (select_cnt > 0)
		{
			row_index = 0;
			for(j=0;j<realItemCnt;j++)
			{
				index = g_ruleIdBuf[j];
				if( g_ruleIdBuf[j] == index && g_stSCERule[i].stRule[index].real == 1 )
				{
					sprintf(resTmp, "%3s %17s %6s %-32s %9d %9d\n",
							"", "SUM",SysName[i], g_stSCERule[i].stRule[index].rName, g_stSCERule[i].stRule[index].actSub, \
							g_stSCERule[i].stRule[index].totSub);
					strcat(ipafBuf, resTmp);
				}
			}
    		sprintf(resTmp, "    ===============================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < MAX_RULE_NUM; j++)
			{
				g_stSCERule[i].stRule[j].unblk = 0;
				g_stSCERule[i].stRule[j].blk = 0;
				g_stSCERule[i].stRule[j].tot = 0;
			}
		}
///////////////////////////
    }

    if (tcnt == 0) 
	{
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
   		sprintf(resTmp, "    ===============================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } 
	else
	{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;
}

////////////
int stmd_mmc_stat_rule_usrcnt(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_RULE_USER;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandRuleUser(list);
    }
}


int stmd_mmc_srch_stat_rule_count(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_cnt=0;
    int   tot_row[5][3]={0,}; // sum(unblock_cnt), sum(block_cnt), sum(unblock_cnt+block_cnt)
    char  key_time[33], last_time[33] = {0,};

    char        itemRes[10][10];
    int         i,j,k, ridx, toResult;
    char *title[] = {"STATTIME", "SYSTEM", "ITEM", "UnBlock", "Block", "Total"};
//	char *item[]={"21(PDA P bit=1)/H bit=1","22(PDA P bit=1)/H bit=2","23(PDA P bit=1)/H bit=3","24(PDA P bit=1)/H bit=4","25(PDA P bit=1)/H bit=31"};
    int row_index;
    char SysName[SCE_CNT][8];
    char SysIp[SCE_CNT][16];
    int realSysCnt =0;
    int realItemCnt =0;
	char	tmpRule[5][512];
	int		ruleSet[5], index;
    int         select_cnt = 0, snd_cnt,tcnt=0;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

	realItemCnt = g_ruleItemCnt;

	for(i=0; i<SCE_CNT; i++ )
	{
		strcpy(SysName[i], g_stSCE[i].sce_name);
		strcpy(SysIp[i], g_stSCE[i].sce_ip);
		realSysCnt++;
	}

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_RULE_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_RULE_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_RULE_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_RULE_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-rule-count");

    sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
        rxReqMsg->head.para[1].paraVal,stime,etime);
    sprintf(resTmp, "    =======================================================================================\n");
    strcat(ipafHead, resTmp);
	sprintf(resTmp, "%3s %17s %6s %-32s %9s %9s %9s\n","",title[0],title[1],title[2],title[3],title[4],title[5]);
    strcat(ipafHead, resTmp);
    sprintf(resTmp, "    =======================================================================================\n");
    strcat(ipafHead, resTmp);

    strcat(ipafBuf, ipafHead);

    snd_cnt=1;
    for(i=0; i<realSysCnt; i++)
	{ 
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
                continue;
        }

        sprintf(query, "SELECT stat_date, rule_set_id, "
                " sum(unblock_cnt), sum(block_cnt), sum(unblock_cnt+block_cnt) " 
                " from %s "
                " where record_source = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" Group By stat_date, rule_set_id ORDER BY stat_date, rule_set_id ", table_name, SysIp[i], stime, etime );

        if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        select_cnt=0;
		memset(last_time,0x00,sizeof(last_time));
        row_cnt = 0;                                                                                  
        result = mysql_store_result(conn);

        while( (row = mysql_fetch_row(result)) != NULL)
        {
			toResult = 0;
            if(strlen(row[0]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[0]+5,14);
            }
			if(strcmp(key_time, last_time) && last_time[0] != 0 ) // 시간이 변경 
			{
    			sprintf(resTmp, "    ---------------------------------------------------------------------------------------\n");
				strcat(ipafBuf, resTmp);
				row_cnt = 0;
			}

            index = atoi(row[1]); // 배열 index 
			for( ridx = 0; ridx < realItemCnt; ridx++ )
			{
				//if( g_ruleIdBuf[ridx] == index )
				if( g_ruleIdBuf[ridx] == index && g_stSCERule[i].stRule[index].real == 1 )
				{
					toResult = 1;
					break;
				}
			}
			if( toResult == 1 )
			{
				row_index = 2;
				if ( row_cnt == 0 )
				{
					sprintf(resBuf, "%3s %17s %6s %-32s %9s %9s %9s\n",
							"", key_time, SysName[i], g_stSCERule[i].stRule[index].rName, row[row_index], row[row_index+1], row[row_index+2]);
					strcat(ipafBuf, resBuf);
				}
				else
				{
					sprintf(resBuf, "%3s %17s %6s %-32s %9s %9s %9s\n",
							"", "", "", g_stSCERule[i].stRule[index].rName, row[row_index], row[row_index+1], row[row_index+2]);

					strcat(ipafBuf, resBuf);
				}
				g_stSCERule[i].stRule[index].unblk += atoi(row[row_index]); 
				g_stSCERule[i].stRule[index].blk += atoi(row[row_index+1]); 
				g_stSCERule[i].stRule[index].tot += atoi(row[row_index+2]);

				strcpy(last_time, key_time);

				select_cnt++;
				tcnt++;
			}

            if(strlen(ipafBuf) > 2500)
			{
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                snd_cnt++;
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            row_cnt++;
        }

        mysql_free_result(result);

		sprintf(resTmp, "    ---------------------------------------------------------------------------------------\n");
		strcat(ipafBuf, resTmp);

        if (select_cnt > 0)
		{
			row_index = 0;
			for(j=0;j<realItemCnt;j++)
			{
				index = g_ruleIdBuf[j];
				if( g_ruleIdBuf[j] == index && g_stSCERule[i].stRule[index].real == 1 )
				{
					sprintf(resTmp, "%3s %17s %6s %-32s %9d %9d %9d\n",
							"", "SUM",SysName[i], g_stSCERule[i].stRule[index].rName, g_stSCERule[i].stRule[index].unblk, \
							g_stSCERule[i].stRule[index].blk, g_stSCERule[i].stRule[index].tot);
					strcat(ipafBuf, resTmp);
				}
			}
    		sprintf(resTmp, "    =======================================================================================\n");
			strcat(ipafBuf, resTmp);

			for(j = 0; j < MAX_RULE_NUM; j++)
			{
				g_stSCERule[i].stRule[j].unblk = 0;
				g_stSCERule[i].stRule[j].blk = 0;
				g_stSCERule[i].stRule[j].tot = 0;
			}
		}
///////////////////////////
    }

    if (tcnt == 0) 
	{
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
    	sprintf(resTmp, "    =======================================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } 
	else
	{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;
}

////////////
int stmd_mmc_stat_rule_count(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_RULE_CNT;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandRuleCount(list);
    }
}
#endif


int stmd_mmc_stat_fault(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, diffTime, seqNo=1;
    short           mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    DSC  RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                //printf("jean========stmd_mmc_stat_fault no sys name\n");
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME")) //jjinri 수정 "STM" -> "STIME"
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal); // jjinri note: para-now 초 단위
	    	}
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;   
    onDEMAND[list].statisticsType = STMD_FAULT;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    //strcpy(onDEMAND[list].svcName, "ALL");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));
        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    CNT         = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);

    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandFault(list);
    }

	return 0;
}

//jean fault
#if 0
int stmd_mmc_srch_stat_fault(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j;

    char  table_name1[30], table_name2[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    int   tot_row[12]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt,tcnt=0;
    char *title1[]={"CPU","MEM","HW","PROC"};
	/*
    char *title1[]={"HW","PROC","NET_NMS","CONN_NTP","CONN_NMS"};
    char *title2[]={"HW","PROC","DUP_HB","DUP_OOS", "RATE_WAP1", "RATE_WAP2","RATE_HTTP","RATE_UAWAP","RATE_AAA","RATE_VODS", "RATE_ANAAA",
                    "RATE_VT","RATE_RADIUS","NET_UAWAP","NET_AAA","CONN_UAWAP","CONN_NTP"};
	*/
    int row_index;
    char SysName[5][8];
    int realSysCnt =0;
    int realItemCnt =0;

    for(i=0; i<sysCnt; i++ ){
        strcpy(SysName[i], StatisticSystemInfo[i].sysName);
        realSysCnt++;
    }
    strcpy(SysName[realSysCnt], "TAPA");realSysCnt++;
    strcpy(SysName[realSysCnt], "TAPB");realSysCnt++;
    strcpy(SysName[realSysCnt], "SCEA");realSysCnt++;
    strcpy(SysName[realSysCnt], "SCEB");realSysCnt++;

    
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name1,STM_STATISTIC_5MINUTE_FAULT_TBL_NAME);
        strcpy(table_name2,STM_STATISTIC_5MINUTE_BSD_FLT_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name1,STM_STATISTIC_HOUR_FAULT_TBL_NAME);
        strcpy(table_name2,STM_STATISTIC_HOUR_BSD_FLT_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name1,STM_STATISTIC_DAY_FAULT_TBL_NAME);
        strcpy(table_name2,STM_STATISTIC_DAY_BSD_FLT_TBL_NAME);
    }
    else
    {
        strcpy(table_name1,STM_STATISTIC_MONTH_FAULT_TBL_NAME);
        strcpy(table_name2,STM_STATISTIC_MONTH_BSD_FLT_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-fault");

    sprintf(ipafHead, "    PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
        rxReqMsg->head.para[1].paraVal,stime,etime);
        
    sprintf(resTmp, "    ===============================================================\n");
    strcat(ipafHead, resTmp);
    sprintf(resTmp, "    STATTIME        SYSTEM  ITEM           MINOR    MAJOR  CRITICAL\n");
    strcat(ipafHead, resTmp);
    sprintf(resTmp, "    ===============================================================\n");
    strcat(ipafHead, resTmp);

    strcat(ipafBuf, ipafHead);

    snd_cnt=1;

    for(i=0; i<realSysCnt; i++)
	{ 
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) 
		{
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, SysName[i]))
                continue;
        }

        if(!strcasecmp(SysName[i], "DSCM"))
            realItemCnt=4;
        else if(!strcasecmp(SysName[i], "TAPA") || !strcasecmp(SysName[i], "TAPB"))
            realItemCnt=3;
		else if(!strcasecmp(SysName[i], "SCEA") || !strcasecmp(SysName[i], "SCEB"))
            realItemCnt=3;
		else
			realItemCnt=4;

		sprintf(query, "SELECT"
				" cpu_min_cnt,cpu_maj_cnt,cpu_cri_cnt,"
				" mem_min_cnt,mem_maj_cnt,mem_cri_cnt,"
				" etc_hw_min_cnt,etc_hw_maj_cnt,etc_hw_cri_cnt,"
				" proc_min_cnt, proc_maj_cnt, proc_cri_cnt,"
				" stat_date "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s'  ORDER BY stat_date ",
				table_name1, SysName[i], stime, etime);

		if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
			sprintf(trcBuf, "query = %s\n", query);
			trclib_writeLog(FL, trcBuf);
		}
		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		result = mysql_store_result(conn);

		select_cnt=0;
		memset(tot_row,0,sizeof(tot_row));
		while((row = mysql_fetch_row(result)) != NULL)  
		{
			if(strlen(ipafBuf) > 2500)
			{
				stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
				snd_cnt++;

				memset (ipafBuf, 0x00, sizeof(ipafBuf));

				strcat(ipafBuf, ipafHead);
			}

			if(strlen(row[12]) == 19)
			{
				memset(key_time,0,sizeof(key_time));
				strncpy(key_time,row[12]+5,14);
			}

			row_index = 0;
			for(j=0;j<realItemCnt;j++)
			{
				if (j==0)
				{
					sprintf(resTmp, "%3s %-14s  %-6s  %-12s %7s %8s %9s\n", 
							"", key_time,SysName[i], title1[j], row[row_index], row[row_index+1], row[row_index+2]);
					strcat(ipafBuf, resTmp);
				} 
				else 
				{
					sprintf(resTmp, "%3s %-14s  %-6s  %-12s %7s %8s %9s\n", 
							"", "","", title1[j], row[row_index], row[row_index+1], row[row_index+2]);
					strcat(ipafBuf, resTmp);
				}
				row_index += 3;
			}
			sprintf(resTmp, "    ---------------------------------------------------------------\n");
			strcat(ipafBuf, resTmp);

			tot_row[0]+=atoi(row[0]);   tot_row[1]+=atoi(row[1]);   tot_row[2]+=atoi(row[2]);   
			tot_row[3]+=atoi(row[3]); 	tot_row[4]+=atoi(row[4]);   tot_row[5]+=atoi(row[5]);   
			tot_row[6]+=atoi(row[6]);   tot_row[7]+=atoi(row[7]); 	tot_row[8]+=atoi(row[8]);   
			tot_row[9]+=atoi(row[9]);   tot_row[10]+=atoi(row[10]); tot_row[11]+=atoi(row[11]);

			select_cnt++;
			tcnt++;

		}
		mysql_free_result(result);

		if (select_cnt > 0)
		{
			if(strlen(ipafBuf) > 2500)
			{
				stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
				snd_cnt++;

				memset (ipafBuf, 0x00, sizeof(ipafBuf));

				strcat(ipafBuf, ipafHead);
			}

			row_index = 0;
			for(j=0;j<realItemCnt;j++)
			{
				if (j==0)
				{
					sprintf(resTmp, "%3s %-14s  %-6s  %-12s %7d %8d %9d\n", 
							"", "SUM",SysName[i], title1[j], tot_row[row_index], tot_row[row_index+1], tot_row[row_index+2]);
					strcat(ipafBuf, resTmp);
				} 
				else 
				{
					sprintf(resTmp, "%3s %-14s  %-6s  %-12s %7d %8d %9d\n", 
							"", "","", title1[j], tot_row[row_index], tot_row[row_index+1], tot_row[row_index+2]);
					strcat(ipafBuf, resTmp);
				}
				row_index += 3;
			}

			sprintf(resTmp, "    ===============================================================\n");
			strcat(ipafBuf, resTmp);
		}
	}

    if (tcnt == 0) 
	{
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
        sprintf(resTmp, "    ===============================================================\n");
        strcat(ipafBuf, resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } 
	else
	{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

}

int stmd_mmc_srch_stat_rad(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[26]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt,tcnt=0;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_RADIUS_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_RADIUS_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_RADIUS_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_RADIUS_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-rad");

    sprintf(resTmp, "    ==================================================================================================\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    STATTIME       TYPE       TOT_AUTH_REQ          TOT_AUTH_RES   TOT_AUTH_TIMEOUT      TOT_AUTH_ACPT\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                              TOT_AUTH_RJT  NO_INTERIM_AUTH_ACPT  TOT_CMPL_AUTH_TXN     ONLY_AUTH_ACPT\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                             ONLY_AUTH_RJT      DEC_ERR_AUHT_REQ   DEC_ERR_AUTH_RES       DUP_AUTH_REQ\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                          ETC_ERR_AUTH_REQ      ETC_ERR_AUTH_RES\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                              TOT_ACCT_REQ          TOT_ACCT_RES    TOT_START\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                               TOT_INTERIM              TOT_STOP       SESS_CONTINUE  DEC_ERR_ACCT_REQ\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                          DEC_ERR_ACCT_RES          DUP_ACCT_REQ    ETC_ERR_ACCT_REQ  ETC_ERR_ACCT_RES\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    =================================================================================================\n");
    strcat(ipafHead,resTmp);

    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }

        sprintf(query, "SELECT PDSN_ADDR, TOT_AUTH_REQ, TOT_AUTH_RES, TOT_AUTH_TIMEOUT, TOT_AUTH_ACPT, TOT_AUTH_RJT, "
            "NO_INTERIM_AUTH_ACPT, TOT_CMPL_AUTH_TXN, ONLY_AUTH_ACPT,  ONLY_AUTH_RJT, DEC_ERR_AUHT_REQ,  "
            "DEC_ERR_AUTH_RES, DUP_AUTH_REQ, ETC_ERR_AUTH_REQ,  ETC_ERR_AUTH_RES, TOT_ACCT_REQ,  "
            "TOT_ACCT_RES, TOT_START,  TOT_INTERIM, TOT_STOP,  "
            "SESS_CONTINUE, "
            "DEC_ERR_ACCT_REQ, DEC_ERR_ACCT_RES, DUP_ACCT_REQ,  ETC_ERR_ACCT_REQ, ETC_ERR_ACCT_RES, stat_date "
            " from %s "
            " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s'  ORDER BY PDSN_ADDR, stat_date ",
            table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
//printf("====jean\n%s\n",query);
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        
	while((row = mysql_fetch_row(result)) != NULL)  {
            
	    keepalivelib_increase();
 
	    if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PDSN_ADDR = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName, row[0], rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);
        
            if(strlen(row[26]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[26]+5,14);
            }

            sprintf(resTmp, "%3s %14s %-6s %16s %21s %18s %18s\n"
                            "%3s %14s %-6s %16s %21s %18s %18s\n"
                            "%3s %14s %-6s %16s %21s %18s %18s\n"
                            "%3s %14s %-6s %16s %21s\n"
                            "%3s %14s %-6s %16s %21s %18s\n"
                            "%3s %14s %-6s %16s %21s %18s %18s\n"
                            "%3s %14s %-6s %16s %21s %18s %18s\n"
                ,"",key_time,"AUTH",row[1],row[2],row[3],row[4]
                ,"","","",row[5],row[6],row[7],row[8]
                ,"","","",row[9],row[10],row[11],row[12]
                ,"","","",row[13],row[14]
                ,"","","ACCT",row[15],row[16],row[17]
                ,"","","",row[18],row[19],row[20],row[21]
                ,"","","",row[22],row[23],row[24],row[25]
            );

            strcat(ipafBuf,resTmp);

            sprintf(resTmp, "    -------------------------------------------------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);

            select_cnt++;
            snd_cnt++;

            for(k=0;k<25;k++){
                tot_row[k]+=atoll(row[k+1]);
            }

        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PDSN_ADDR = ALL PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName, rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            sprintf(resTmp, "%3s %-14s %-6s %16lld %21lld %18lld %18lld\n"
                            "%3s %-14s %-6s %16lld %21lld %18lld %18lld\n"
                            "%3s %-14s %-6s %16lld %21lld %18lld %18lld\n"
                            "%3s %-14s %-6s %16lld %21lld\n"
                            "%3s %-14s %-6s %16lld %21lld %18lld \n"
                            "%3s %-14s %-6s %16lld %21lld %18lld %18lld\n"
                            "%3s %-14s %-6s %16lld %21lld %18lld %18lld\n"
               	,"","SUM","AUTH",tot_row[0],tot_row[1],tot_row[2],tot_row[3]
                ,"","","",tot_row[4],tot_row[5],tot_row[6],tot_row[7]
                ,"","","",tot_row[8],tot_row[9],tot_row[10],tot_row[11]
                ,"","","",tot_row[12],tot_row[13]
                ,"","","ACCT",tot_row[14],tot_row[15],tot_row[16]
                ,"","","",tot_row[17],tot_row[18],tot_row[19],tot_row[20]
                ,"","","",tot_row[21],tot_row[22],tot_row[23],tot_row[24]
            );
            strcat(ipafBuf,resTmp);

            sprintf(resTmp, "    ==================================================================================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }
    }

    if (snd_cnt == 1) {
        //sprintf(resTmp, "      %-8s  %10s  %10s  %10s  %10s\n",
        //  "", "0.0", "0.0", "0.0", "0.0");
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    ==================================================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

}
#endif // by jjinri 2009.04.18

#if 0
int stmd_mmc_srch_stat_ttxn(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[20]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt;
    char *title[]={"TOT","OUT_SVC_TYPE","NO_CALL","RETX","ETC_ERR"};
    int row_index;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_SVC_TTR_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_SVC_TTR_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_SVC_TTR_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_SVC_TTR_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-svc");

    sprintf(resTmp, "    ===========================================================\n");
    strcat(ipafHead, resTmp);
    sprintf(resTmp, "    STATTIME       STATTYPE             UP_PKTS       DOWN_PKTS\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                       UP_BYTES      DOWN_BYTES\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    ===========================================================\n");
    strcat(ipafHead,resTmp);

    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }

///////////////////////////
        sprintf(query, "SELECT "
                        "TOT_UP_PKTS, TOT_DOWN_PKTS, TOT_UP_BYTES, TOT_DOWN_BYTES,  "
                        "OUT_SVC_TYPE_UP_PKTS, OUT_SVC_TYPE_DOWN_PKTS, OUT_SVC_TYPE_UP_BYTES,  OUT_SVC_TYPE_DOWN_BYTES,  "
                        "NO_CALL_UP_PKTS, NO_CALL_DOWN_PKTS, NO_CALL_UP_BYTES, NO_CALL_DOWN_BYTES,  "
                        "RETX_UP_PKTS, RETX_DOWN_PKTS, RETX_UP_BYTES,  RETX_DOWN_BYTES,  "
                        "ETC_ERR_UP_PKTS, ETC_ERR_DOWN_PKTS, ETC_ERR_UP_BYTES, ETC_ERR_DOWN_BYTES, stat_date "
            " from %s "
            " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' ORDER BY stat_date ",
            table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
//printf("====jean\n%s\n",query);
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt > 1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName, rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);
        

            if(strlen(row[20]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[20]+5,14);
            }
            row_index=0;
            for(j=0;j<5;j++){
                if(j==0)
                    strcpy(tmp,key_time);
                else
                    strcpy(tmp,"");
                
                sprintf(resTmp, "%3s %14s %-12s %15s %15s\n","",tmp,title[j],row[row_index],row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
                sprintf(resTmp, "%3s %14s %12s %15s %15s\n","","","",row[row_index],row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
            }
            sprintf(resTmp, "    -----------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);

            select_cnt++;
            snd_cnt++;

            for(k=0;k<20;k++){
                tot_row[k]+=atoll(row[k]);
            }

        }
        mysql_free_result(result);

        if (select_cnt > 0){

            if(snd_cnt > 1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName, rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            row_index=0;
            for(j=0;j<5;j++){
                if(j==0)
                    strcpy(tmp,"SUM");
                else
                    strcpy(tmp,"");
                
                sprintf(resTmp, "%3s %-14s %-12s %15lld %15lld\n","",tmp,title[j],tot_row[row_index],tot_row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
                sprintf(resTmp, "%3s %14s %12s %15lld %15lld\n","","","",tot_row[row_index],tot_row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
            }
            sprintf(resTmp, "    ===========================================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

///////////////////////////
    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
        sprintf(resTmp, "    ===========================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

}
#endif // by jjinri 2009.04.18
    
#if 0
int stmd_mmc_srch_stat_txn(IxpcQMsgType *rxIxpcMsg)
{
    int i;
    MMLReqMsgType   *rxReqMsg;
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"WAP1") )
    {
        stmd_mmc_srch_stat_wap1(rxIxpcMsg);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"WAP2") )
    {
        stmd_mmc_srch_stat_wap2(rxIxpcMsg);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HTTP") )
    {
        stmd_mmc_srch_stat_http(rxIxpcMsg);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"JAVA") )
    {
        stmd_mmc_srch_stat_java(rxIxpcMsg);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"VODS") )
    {
        stmd_mmc_srch_stat_vods(rxIxpcMsg);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"WIPI") )
    {
        stmd_mmc_srch_stat_wipi(rxIxpcMsg);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"VT") )
    {
        stmd_mmc_srch_stat_vt(rxIxpcMsg);
    }
}

int stmd_mmc_srch_stat_wap1(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[80]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt,tcnt=0;
    char *title[]={ "TOT_UP_PKTS", "TOT_REQ_BYTES", "TOT_DOWN_PKTS", "TOT_RES_BYTES", 
                    "TOT_UP_RETX_PKTS", "TOT_UP_RETX_BYTES", "TOT_DOWN_RETX_PKTS", "TOT_DOWN_RETX_BYTES", 
                    "TOT_DROP_REQ_MSGS", "TOT_DROP_REQ_BYTES", "TOT_DROP_RES_MSGS", "TOT_DROP_RES_BYTES", 
                    "FAIL_DROP_REQ_MSGS", "FAIL_DROP_REQ_BYTES", "FAIL_DROP_RES_MSGS", "FAIL_DROP_RES_BYTES", 
                    "NORM_TXN_CNT", "COMPLETED_TXN_CNT", "NEWRES_MATCHED_TXN", "TIMEOUT_ONLY_REQ_TXN", 
                    "CALL_STOP_ONLY_REQ_TXN", "TIMEOUT_ONLY_RES_TXN", "CALL_STOP_ONLY_RES_TXN", "TIMEOUT_ONLY_ACK_TXN", 
                    "CALL_STOP_ONLY_ACK_TXN", "TIMEOUT_ONLY_REQ_ACK_TXN", "CALL_STOP_ONLY_REQ_ACK_TXN", "TIMEOUT_ONLY_REQ_RES_TXN", 
                    "CALL_STOP_ONLY_REQ_RES_TXN", "TIMEOUT_ONLY_RES_ACK_TXN", "CALL_STOP_ONLY_RES_ACK_TXN", "GET_METHOD_CNT", 
                    "POST_METHOD_CNT", "CONN_METHOD_CNT", "ETC_METHOD_CNT"            
                  };
    int row_index;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[2].paraVal); i++){
        rxReqMsg->head.para[2].paraVal[i] = toupper(rxReqMsg->head.para[2].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_SVC_WAP1_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_SVC_WAP1_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_SVC_WAP1_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_SVC_WAP1_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[4].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[4].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-svc");
        
    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }
        sprintf(query, "SELECT TRAN_TYPE, "
                        "TOT_UP_PKTS, TOT_REQ_BYTES, TOT_DOWN_PKTS, TOT_RES_BYTES, TOT_UP_RETX_PKTS, "
                        "TOT_UP_RETX_BYTES, TOT_DOWN_RETX_PKTS, TOT_DOWN_RETX_BYTES, TOT_DROP_REQ_MSGS, "
                        "TOT_DROP_REQ_BYTES, TOT_DROP_RES_MSGS, TOT_DROP_RES_BYTES, FAIL_DROP_REQ_MSGS, "
                        "FAIL_DROP_REQ_BYTES, FAIL_DROP_RES_MSGS, FAIL_DROP_RES_BYTES, NORM_TXN_CNT, "
                        "COMPLETED_TXN_CNT, NEWRES_MATCHED_TXN, TIMEOUT_ONLY_REQ_TXN, CALL_STOP_ONLY_REQ_TXN, "
                        "TIMEOUT_ONLY_RES_TXN, CALL_STOP_ONLY_RES_TXN, TIMEOUT_ONLY_ACK_TXN, CALL_STOP_ONLY_ACK_TXN, "
                        "TIMEOUT_ONLY_REQ_ACK_TXN, CALL_STOP_ONLY_REQ_ACK_TXN, TIMEOUT_ONLY_REQ_RES_TXN, CALL_STOP_ONLY_REQ_RES_TXN, "
                        "TIMEOUT_ONLY_RES_ACK_TXN, CALL_STOP_ONLY_RES_ACK_TXN, GET_METHOD_CNT, POST_METHOD_CNT, "
                        "CONN_METHOD_CNT, ETC_METHOD_CNT, "             
                        "stat_date "
                        " from %s "
                        " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' ORDER BY stat_date ",
                    table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

            sprintf(resTmp, "    =========================================================\n");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %-26s %15s\n",   "","STATTIME","MSG_TYPE", row[0]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    =========================================================\n");
            strcat(ipafBuf,resTmp);

            /* stat_date */
            if(strlen(row[36]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[36]+5,14);
            }

            for ( row_index=0 ; row_index<SVC_WAP1_ROW_CNT ;row_index++ )
            {
                if(row_index==0)
                    strcpy(tmp,key_time);
                else
                    strcpy(tmp,"");
                sprintf(resTmp, "%3s %-14s %-26s %15s\n",
                    "", tmp, title[row_index], row[row_index+1]);
                strcat(ipafBuf,resTmp);

                if (strlen (ipafBuf) > 3000) {
                    strcat (ipafBuf, "    ---------------------------------------------------------\n");
                    stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                    memset (ipafBuf, 0x00, sizeof(ipafBuf));
                    sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                        StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

                    sprintf(resTmp, "    =========================================================\n");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "%3s %-14s %-26s %15s\n",   "","STATTIME","MSG_TYPE", row[0]);
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "    =========================================================\n");
                    strcat(ipafBuf,resTmp);
                }
            }
            sprintf(resTmp, "    ---------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);


            select_cnt++;
            snd_cnt++;

            for(k=0;k<37;k++){ 
                tot_row[k]+=atoll(row[k+1]);
            }
        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-26s %15s\n", "","MSG_TYPE", "SUM");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);

            for ( row_index=0 ; row_index<SVC_WAP1_ROW_CNT ;row_index++ )
            {
                sprintf(resTmp, "%3s %-26s %15lld\n",
                    "", title[row_index], tot_row[row_index]);
                strcat(ipafBuf,resTmp);

                if (strlen (ipafBuf) > 4000) {
                    strcat (ipafBuf, "    ---------------------------------------------------------\n");
                    stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                    memset (ipafBuf, 0x00, sizeof(ipafBuf));
                    sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                        StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

                    sprintf(resTmp, "    ==========================================\n");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "%3s %-26s %15s\n", "","MSG_TYPE", "SUM");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "    ==========================================\n");
                    strcat(ipafBuf,resTmp);
                }
            }
            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    ==========================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} /* End of stmd_mmc_srch_stat_wap1 */

int stmd_mmc_srch_stat_wap2(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[80]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt,tcnt=0;
    char *title[]={ "TOT_UP_PKTS", "TOT_REQ_PKTS", "TOT_REQ_BYTES", "TOT_DOWN_PKTS", "TOT_RES_PKTS", "TOT_RES_BYTES", 
                    "TOT_UP_RETX_PKTS", "TOT_UP_RETX_BYTES", "TOT_DOWN_RETX_PKTS", "TOT_DOWN_RETX_BYTES", 
                    "TOT_DROP_REQ_MSGS", "TOT_DROP_REQ_BYTES", "TOT_DROP_RES_MSGS", "TOT_DROP_RES_BYTES", 
                    "NO_TCP_DROP_REQ_MSGS", "NO_TCP_DROP_REQ_BYTES", "NO_TCP_DROP_RES_MSGS", "NO_TCP_DROP_RES_BYTES", 
                    "NO_TXN_DROP_REQ_MSGS", "NO_TXN_DROP_REQ_BYTES", "NO_TXN_DROP_RES_MSGS", "NO_TXN_DROP_RES_BYTES", 
                    "FAIL_DROP_REQ_MSGS", "FAIL_DROP_REQ_BYTES", "FAIL_DROP_RES_MSGS", "FAIL_DROP_RES_BYTES", 
                    "NORM_TXN_CNT", "COMPLETED_TXN_CNT", "NOT_EQUAL_TXN_CNT", "NOT_EQUAL_TXN_BYTES", "NEWRES_MATCHED_TXN", 
                    "TIMEOUT_MATCHED_TXN", "TCP_STOP_MATCHED_TXN", "CALL_STOP_MACHED_TXN", "NEWREQ_ONLY_REQ_TXN", 
                    "TIMEOUT_ONLY_REQ_TXN", "TCP_STOP_ONLY_REQ_TXN", "CALL_STOP_ONLY_REQ_TXN", "NEWRES_ONLY_RES_TXN", 
                    "TIMEOUT_ONLY_RES_TXN", "TCP_STOP_ONLY_RES_TXN", "CALL_STOP_ONLY_RES_TXN", "GET_METHOD_CNT", 
                    "POST_METHOD_CNT", "CONN_METHOD_CNT", "ETC_METHOD_CNT"                     
                  };
    int row_index;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[2].paraVal); i++){
        rxReqMsg->head.para[2].paraVal[i] = toupper(rxReqMsg->head.para[2].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_SVC_WAP2_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_SVC_WAP2_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_SVC_WAP2_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_SVC_WAP2_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[4].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[4].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-svc");
        
    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }
        sprintf(query, "SELECT TRAN_TYPE, "
                        "TOT_UP_PKTS, TOT_REQ_PKTS, TOT_REQ_BYTES, TOT_DOWN_PKTS, TOT_RES_PKTS, TOT_RES_BYTES, "
                        "TOT_UP_RETX_PKTS, TOT_UP_RETX_BYTES, TOT_DOWN_RETX_PKTS, TOT_DOWN_RETX_BYTES, "
                        "TOT_DROP_REQ_MSGS, TOT_DROP_REQ_BYTES, TOT_DROP_RES_MSGS, TOT_DROP_RES_BYTES, "
                        "NO_TCP_DROP_REQ_MSGS, NO_TCP_DROP_REQ_BYTES, NO_TCP_DROP_RES_MSGS, NO_TCP_DROP_RES_BYTES, "
                        "NO_TXN_DROP_REQ_MSGS, NO_TXN_DROP_REQ_BYTES, NO_TXN_DROP_RES_MSGS, NO_TXN_DROP_RES_BYTES, "
                        "FAIL_DROP_REQ_MSGS, FAIL_DROP_REQ_BYTES, FAIL_DROP_RES_MSGS, FAIL_DROP_RES_BYTES, "
                        "NORM_TXN_CNT, COMPLETED_TXN_CNT, NOT_EQUAL_TXN_CNT, NOT_EQUAL_TXN_BYTES, NEWRES_MATCHED_TXN, "
                        "TIMEOUT_MATCHED_TXN, TCP_STOP_MATCHED_TXN, CALL_STOP_MACHED_TXN, NEWREQ_ONLY_REQ_TXN, "
                        "TIMEOUT_ONLY_REQ_TXN, TCP_STOP_ONLY_REQ_TXN, CALL_STOP_ONLY_REQ_TXN, NEWRES_ONLY_RES_TXN, "
                        "TIMEOUT_ONLY_RES_TXN, TCP_STOP_ONLY_RES_TXN, CALL_STOP_ONLY_RES_TXN, GET_METHOD_CNT, "
                        "POST_METHOD_CNT, CONN_METHOD_CNT, ETC_METHOD_CNT, "                      
                        "stat_date "
                        " from %s "
                        " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' ORDER BY stat_date ",
                    table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

            sprintf(resTmp, "    =========================================================\n");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %-26s %15s\n",   "","STATTIME","MSG_TYPE", row[0]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    =========================================================\n");
            strcat(ipafBuf,resTmp);

            /* stat_date */
            if(strlen(row[47]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[47]+5,14);
            }

            for ( row_index=0 ; row_index<SVC_WAP2_ROW_CNT ;row_index++ )
            {
                if(row_index==0)
                    strcpy(tmp,key_time);
                else
                    strcpy(tmp,"");
                sprintf(resTmp, "%3s %-14s %-26s %15s\n",
                    "", tmp, title[row_index], row[row_index+1]);
                strcat(ipafBuf,resTmp);

                if (strlen (ipafBuf) > 3000) {
                    strcat (ipafBuf, "    ---------------------------------------------------------\n");
                    stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                    memset (ipafBuf, 0x00, sizeof(ipafBuf));
                    sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                        StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

                    sprintf(resTmp, "    =========================================================\n");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "%3s %-14s %-26s %15s\n",   "","STATTIME","MSG_TYPE", row[0]);
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "    =========================================================\n");
                    strcat(ipafBuf,resTmp);
                }
            }
            sprintf(resTmp, "    ---------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);


            select_cnt++;
            snd_cnt++;

            for(k=0;k<48;k++){ 
                tot_row[k]+=atoll(row[k+1]);
            }
        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-26s %15s\n", "","MSG_TYPE", "SUM");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);

            for ( row_index=0 ; row_index<SVC_WAP2_ROW_CNT ;row_index++ )
            {
                sprintf(resTmp, "%3s %-26s %15lld\n",
                    "", title[row_index], tot_row[row_index]);
                strcat(ipafBuf,resTmp);

                if (strlen (ipafBuf) > 4000) {
                    strcat (ipafBuf, "    ---------------------------------------------------------\n");
                    stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                    memset (ipafBuf, 0x00, sizeof(ipafBuf));
                    sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                        StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

                    sprintf(resTmp, "    ==========================================\n");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "%3s %-26s %15s\n", "","MSG_TYPE", "SUM");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "    ==========================================\n");
                    strcat(ipafBuf,resTmp);
                }
            }
            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    ==========================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} /* End of stmd_mmc_srch_stat_wap2 */

int stmd_mmc_srch_stat_http(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[80]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt,tcnt=0;
    char *title[]={ "TOT_UP_PKTS", "TOT_REQ_PKTS", "TOT_REQ_BYTES", "TOT_DOWN_PKTS", "TOT_RES_PKTS", "TOT_RES_BYTES", 
                    "TOT_UP_RETX_PKTS", "TOT_UP_RETX_BYTES", "TOT_DOWN_RETX_PKTS", "TOT_DOWN_RETX_BYTES", 
                    "TOT_DROP_REQ_MSGS", "TOT_DROP_REQ_BYTES", "TOT_DROP_RES_MSGS", "TOT_DROP_RES_BYTES", 
                    "NO_TCP_DROP_REQ_MSGS", "NO_TCP_DROP_REQ_BYTES", "NO_TCP_DROP_RES_MSGS", "NO_TCP_DROP_RES_BYTES", 
                    "NO_TXN_DROP_REQ_MSGS", "NO_TXN_DROP_REQ_BYTES", "NO_TXN_DROP_RES_MSGS", "NO_TXN_DROP_RES_BYTES", 
                    "FAIL_DROP_REQ_MSGS", "FAIL_DROP_REQ_BYTES", "FAIL_DROP_RES_MSGS", "FAIL_DROP_RES_BYTES", 
                    "NORM_TXN_CNT", "COMPLETED_TXN_CNT", "NOT_EQUAL_TXN_CNT", "NOT_EQUAL_TXN_BYTES", "NEWRES_MATCHED_TXN",
                    "TIMEOUT_MATCHED_TXN", "TCP_STOP_MATCHED_TXN", "CALL_STOP_MACHED_TXN", "NEWREQ_ONLY_REQ_TXN", 
                    "TIMEOUT_ONLY_REQ_TXN", "TCP_STOP_ONLY_REQ_TXN", "CALL_STOP_ONLY_REQ_TXN", "NEWRES_ONLY_RES_TXN", 
                    "TIMEOUT_ONLY_RES_TXN", "TCP_STOP_ONLY_RES_TXN", "CALL_STOP_ONLY_RES_TXN", "GET_METHOD_CNT", 
                    "POST_METHOD_CNT", "CONN_METHOD_CNT", "ETC_METHOD_CNT"                              
                };
    int row_index;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[2].paraVal); i++){
        rxReqMsg->head.para[2].paraVal[i] = toupper(rxReqMsg->head.para[2].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_SVC_HTTP_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_SVC_HTTP_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_SVC_HTTP_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_SVC_HTTP_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[4].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[4].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-svc");
        
    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }
        sprintf(query, "SELECT TRAN_TYPE, "
                        "TOT_UP_PKTS, TOT_REQ_PKTS, TOT_REQ_BYTES, TOT_DOWN_PKTS, TOT_RES_PKTS, TOT_RES_BYTES, "
                        "TOT_UP_RETX_PKTS, TOT_UP_RETX_BYTES, TOT_DOWN_RETX_PKTS, TOT_DOWN_RETX_BYTES, "
                        "TOT_DROP_REQ_MSGS, TOT_DROP_REQ_BYTES, TOT_DROP_RES_MSGS, TOT_DROP_RES_BYTES, "
                        "NO_TCP_DROP_REQ_MSGS, NO_TCP_DROP_REQ_BYTES, NO_TCP_DROP_RES_MSGS, NO_TCP_DROP_RES_BYTES, "
                        "NO_TXN_DROP_REQ_MSGS, NO_TXN_DROP_REQ_BYTES, NO_TXN_DROP_RES_MSGS, NO_TXN_DROP_RES_BYTES, "
                        "FAIL_DROP_REQ_MSGS, FAIL_DROP_REQ_BYTES, FAIL_DROP_RES_MSGS, FAIL_DROP_RES_BYTES, "
                        "NORM_TXN_CNT, COMPLETED_TXN_CNT, NOT_EQUAL_TXN_CNT, NOT_EQUAL_TXN_BYTES, NEWRES_MATCHED_TXN, "
                        "TIMEOUT_MATCHED_TXN, TCP_STOP_MATCHED_TXN, CALL_STOP_MACHED_TXN, NEWREQ_ONLY_REQ_TXN, "
                        "TIMEOUT_ONLY_REQ_TXN, TCP_STOP_ONLY_REQ_TXN, CALL_STOP_ONLY_REQ_TXN, NEWRES_ONLY_RES_TXN, "
                        "TIMEOUT_ONLY_RES_TXN, TCP_STOP_ONLY_RES_TXN, CALL_STOP_ONLY_RES_TXN, GET_METHOD_CNT, "
                        "POST_METHOD_CNT, CONN_METHOD_CNT, ETC_METHOD_CNT, "                      
                        "stat_date "
                        " from %s "
                        " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' ORDER BY stat_date ",
                    table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

            sprintf(resTmp, "    =========================================================\n");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %-26s %15s\n",   "","STATTIME","MSG_TYPE", row[0]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    =========================================================\n");
            strcat(ipafBuf,resTmp);

            /* stat_date */
            if(strlen(row[47]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[47]+5,14);
            }

            for ( row_index=0 ; row_index<SVC_HTTP_ROW_CNT ;row_index++ )
            {
                if(row_index==0)
                    strcpy(tmp,key_time);
                else
                    strcpy(tmp,"");
                sprintf(resTmp, "%3s %-14s %-26s %15s\n",
                    "", tmp, title[row_index], row[row_index+1]);
                strcat(ipafBuf,resTmp);

                if (strlen (ipafBuf) > 3000) {
                    strcat (ipafBuf, "    ---------------------------------------------------------\n");
                    stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                    memset (ipafBuf, 0x00, sizeof(ipafBuf));
                    sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                        StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

                    sprintf(resTmp, "    =========================================================\n");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "%3s %-14s %-26s %15s\n",   "","STATTIME","MSG_TYPE", row[0]);
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "    =========================================================\n");
                    strcat(ipafBuf,resTmp);
                }
            }
            sprintf(resTmp, "    ---------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);


            select_cnt++;
            snd_cnt++;

            for(k=0;k<48;k++){ 
                tot_row[k]+=atoll(row[k+1]);
            }
        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-26s %15s\n", "","MSG_TYPE", "SUM");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);

            for ( row_index=0 ; row_index<SVC_HTTP_ROW_CNT ;row_index++ )
            {
                sprintf(resTmp, "%3s %-26s %15lld\n",
                    "", title[row_index], tot_row[row_index]);
                strcat(ipafBuf,resTmp);

                if (strlen (ipafBuf) > 4000) {
                    strcat (ipafBuf, "    ---------------------------------------------------------\n");
                    stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                    memset (ipafBuf, 0x00, sizeof(ipafBuf));
                    sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                        StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

                    sprintf(resTmp, "    ==========================================\n");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "%3s %-26s %15s\n", "","MSG_TYPE", "SUM");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "    ==========================================\n");
                    strcat(ipafBuf,resTmp);
                }
            }
            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    ==========================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} /* End of stmd_mmc_srch_stat_http */

int stmd_mmc_srch_stat_java(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[80]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt,tcnt=0;
    char *title[]={ "TOT_REQ_PKTS", "TOT_REQ_BYTES", "TOT_RES_PKTS", "TOT_RES_BYTES", 
                    "TOT_UP_RETX_PKTS", "TOT_UP_RETX_BYTES", "TOT_DOWN_RETX_PKTS", "TOT_DOWN_RETX_BYTES", 
                    "FAIL_DROP_REQ_MSGS", "FAIL_DROP_REQ_BYTES", "FAIL_DROP_RES_MSGS", "FAIL_DROP_RES_BYTES"                               
                };
    int row_index;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[2].paraVal); i++){
        rxReqMsg->head.para[2].paraVal[i] = toupper(rxReqMsg->head.para[2].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_SVC_JAVA_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_SVC_JAVA_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_SVC_JAVA_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_SVC_JAVA_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[4].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[4].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-svc");
        
    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }
        sprintf(query, "SELECT TRAN_TYPE, "
                        "TOT_REQ_PKTS, TOT_REQ_BYTES, TOT_RES_PKTS, TOT_RES_BYTES, "
                        "TOT_UP_RETX_PKTS, TOT_UP_RETX_BYTES, TOT_DOWN_RETX_PKTS, TOT_DOWN_RETX_BYTES, "
                        "FAIL_DROP_REQ_MSGS, FAIL_DROP_REQ_BYTES, FAIL_DROP_RES_MSGS, FAIL_DROP_RES_BYTES, "                      
                        "stat_date "
                        " from %s "
                        " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' ORDER BY stat_date ",
                    table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

            sprintf(resTmp, "    =========================================================\n");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %-26s %15s\n",   "","STATTIME","MSG_TYPE", row[0]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    =========================================================\n");
            strcat(ipafBuf,resTmp);

            /* stat_date */
            if(strlen(row[13]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[13]+5,14);
            }

            for ( row_index=0 ; row_index<SVC_JAVA_ROW_CNT ;row_index++ )
            {
                if(row_index==0)
                    strcpy(tmp,key_time);
                else
                    strcpy(tmp,"");
                sprintf(resTmp, "%3s %-14s %-26s %15s\n",
                    "", tmp, title[row_index], row[row_index+1]);
                strcat(ipafBuf,resTmp);

                if (strlen (ipafBuf) > 3000) {
                    strcat (ipafBuf, "    ---------------------------------------------------------\n");
                    stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                    memset (ipafBuf, 0x00, sizeof(ipafBuf));
                    sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                        StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

                    sprintf(resTmp, "    =========================================================\n");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "%3s %-14s %-26s %15s\n",   "","STATTIME","MSG_TYPE", row[0]);
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "    =========================================================\n");
                    strcat(ipafBuf,resTmp);
                }
            }
            sprintf(resTmp, "    ---------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);


            select_cnt++;
            snd_cnt++;

            for(k=0;k<14;k++){ 
                tot_row[k]+=atoll(row[k+1]);
            }
        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-26s %15s\n", "","MSG_TYPE", "SUM");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);

            for ( row_index=0 ; row_index<SVC_JAVA_ROW_CNT ;row_index++ )
            {
                sprintf(resTmp, "%3s %-26s %15lld\n",
                    "", title[row_index], tot_row[row_index]);
                strcat(ipafBuf,resTmp);

                if (strlen (ipafBuf) > 4000) {
                    strcat (ipafBuf, "    ---------------------------------------------------------\n");
                    stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                    memset (ipafBuf, 0x00, sizeof(ipafBuf));
                    sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                        StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

                    sprintf(resTmp, "    ==========================================\n");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "%3s %-26s %15s\n", "","MSG_TYPE", "SUM");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "    ==========================================\n");
                    strcat(ipafBuf,resTmp);
                }
            }
            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    ==========================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} /* End of stmd_mmc_srch_stat_java */

int stmd_mmc_srch_stat_vods(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[80]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt,tcnt=0;
    char *title[]={ "TOT_UP_PKTS", "TOT_REQ_PKTS", "TOT_REQ_BYTES", "TOT_DOWN_PKTS", "TOT_RES_PKTS", "TOT_RES_BYTES", 
                    "TOT_UP_RETX_PKTS", "TOT_UP_RETX_BYTES", "TOT_DOWN_RETX_PKTS", "TOT_DOWN_RETX_BYTES", 
                    "TOT_DROP_REQ_MSGS", "TOT_DROP_REQ_BYTES", "TOT_DROP_RES_MSGS", "TOT_DROP_RES_BYTES", 
                    "NO_TCP_DROP_REQ_MSGS", "NO_TCP_DROP_REQ_BYTES", "NO_TCP_DROP_RES_MSGS", "NO_TCP_DROP_RES_BYTES", 
                    "NO_TXN_DROP_REQ_MSGS", "NO_TXN_DROP_REQ_BYTES", "NO_TXN_DROP_RES_MSGS", "NO_TXN_DROP_RES_BYTES", 
                    "FAIL_DROP_REQ_MSGS", "FAIL_DROP_REQ_BYTES", "FAIL_DROP_RES_MSGS", "FAIL_DROP_RES_BYTES", 
                    "NORM_TXN_CNT", "NEWRES_MATCHED_TXN", "TIMEOUT_MATCHED_TXN", "TCP_STOP_MATCHED_TXN", 
                    "CALL_STOP_MACHED_TXN", "NEWREQ_ONLY_REQ_TXN", "TIMEOUT_ONLY_REQ_TXN", "TCP_STOP_ONLY_REQ_TXN", 
                    "CALL_STOP_ONLY_REQ_TXN", "NEWRES_ONLY_RES_TXN", "TIMEOUT_ONLY_RES_TXN", "TCP_STOP_ONLY_RES_TXN", 
                    "CALL_STOP_ONLY_RES_TXN", "DESC_METHOD_CNT", "SETUP_METHOD_CNT", "PLAY_METHOD_CNT", "TEARDOWN_METHOD_CNT", 
                    "PAUSE_METHOD_CNT", "OPTION_METHOD_CNT", "ANNOUNCE_METHOD_CNT", "ETC_METHOD_CNT", "UDP_UP_PKTS", 
                    "UDP_UP_BYTES", "UDP_DOWN_PKTS", "UDP_DOWN_BYTES"                                        
                };
    int row_index;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[2].paraVal); i++){
        rxReqMsg->head.para[2].paraVal[i] = toupper(rxReqMsg->head.para[2].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_SVC_VODS_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_SVC_VODS_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_SVC_VODS_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_SVC_VODS_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[4].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[4].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-svc");
        
    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }
        sprintf(query, "SELECT TRAN_TYPE, "
                        "TOT_UP_PKTS, TOT_REQ_PKTS, TOT_REQ_BYTES, TOT_DOWN_PKTS, TOT_RES_PKTS, TOT_RES_BYTES, "
                        "TOT_UP_RETX_PKTS, TOT_UP_RETX_BYTES, TOT_DOWN_RETX_PKTS, TOT_DOWN_RETX_BYTES, "
                        "TOT_DROP_REQ_MSGS, TOT_DROP_REQ_BYTES, TOT_DROP_RES_MSGS, TOT_DROP_RES_BYTES, "
                        "NO_TCP_DROP_REQ_MSGS, NO_TCP_DROP_REQ_BYTES, NO_TCP_DROP_RES_MSGS, NO_TCP_DROP_RES_BYTES, "
                        "NO_TXN_DROP_REQ_MSGS, NO_TXN_DROP_REQ_BYTES, NO_TXN_DROP_RES_MSGS, NO_TXN_DROP_RES_BYTES, "
                        "FAIL_DROP_REQ_MSGS, FAIL_DROP_REQ_BYTES, FAIL_DROP_RES_MSGS, FAIL_DROP_RES_BYTES, "
                        "NORM_TXN_CNT, NEWRES_MATCHED_TXN, TIMEOUT_MATCHED_TXN, TCP_STOP_MATCHED_TXN, "
                        "CALL_STOP_MACHED_TXN, NEWREQ_ONLY_REQ_TXN, TIMEOUT_ONLY_REQ_TXN, TCP_STOP_ONLY_REQ_TXN, "
                        "CALL_STOP_ONLY_REQ_TXN, NEWRES_ONLY_RES_TXN, TIMEOUT_ONLY_RES_TXN, TCP_STOP_ONLY_RES_TXN, "
                        "CALL_STOP_ONLY_RES_TXN, DESC_METHOD_CNT, SETUP_METHOD_CNT, PLAY_METHOD_CNT, TEARDOWN_METHOD_CNT, "
                        "PAUSE_METHOD_CNT, OPTION_METHOD_CNT, ANNOUNCE_METHOD_CNT, ETC_METHOD_CNT, UDP_UP_PKTS, "
                        "UDP_UP_BYTES, UDP_DOWN_PKTS, UDP_DOWN_BYTES, "                             
                        "stat_date "
                        " from %s "
                        " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' ORDER BY stat_date ",
                    table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

            sprintf(resTmp, "    =========================================================\n");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %-26s %15s\n",   "","STATTIME","MSG_TYPE", row[0]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    =========================================================\n");
            strcat(ipafBuf,resTmp);

            /* stat_date */
            if(strlen(row[52]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[52]+5,14);
            }

            for ( row_index=0 ; row_index<SVC_VODS_ROW_CNT ;row_index++ )
            {
                if(row_index==0)
                    strcpy(tmp,key_time);
                else
                    strcpy(tmp,"");
                sprintf(resTmp, "%3s %-14s %-26s %15s\n",
                    "", tmp, title[row_index], row[row_index+1]);
                strcat(ipafBuf,resTmp);

                if (strlen (ipafBuf) > 3000) {
                    strcat (ipafBuf, "    ---------------------------------------------------------\n");
                    stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                    memset (ipafBuf, 0x00, sizeof(ipafBuf));
                    sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                        StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

                    sprintf(resTmp, "    =========================================================\n");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "%3s %-14s %-26s %15s\n",   "","STATTIME","MSG_TYPE", row[0]);
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "    =========================================================\n");
                    strcat(ipafBuf,resTmp);
                }
            }
            sprintf(resTmp, "    ---------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);


            select_cnt++;
            snd_cnt++;

            for(k=0;k<53;k++){ 
                tot_row[k]+=atoll(row[k+1]);
            }
        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-26s %15s\n", "","MSG_TYPE", "SUM");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);

            for ( row_index=0 ; row_index<SVC_VODS_ROW_CNT ;row_index++ )
            {
                sprintf(resTmp, "%3s %-26s %15lld\n",
                    "", title[row_index], tot_row[row_index]);
                strcat(ipafBuf,resTmp);

                if (strlen (ipafBuf) > 4000) {
                    strcat (ipafBuf, "    ---------------------------------------------------------\n");
                    stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                    memset (ipafBuf, 0x00, sizeof(ipafBuf));
                    sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                        StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

                    sprintf(resTmp, "    ==========================================\n");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "%3s %-26s %15s\n", "","MSG_TYPE", "SUM");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "    ==========================================\n");
                    strcat(ipafBuf,resTmp);
                }
            }
            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    ==========================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} /* End of stmd_mmc_srch_stat_vods */

int stmd_mmc_srch_stat_wipi(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[80]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt,tcnt=0;
    char *title[]={ "TOT_REQ_PKTS", "TOT_REQ_BYTES", "TOT_RES_PKTS", "TOT_RES_BYTES", 
                    "TOT_UP_RETX_PKTS", "TOT_UP_RETX_BYTES", "TOT_DOWN_RETX_PKTS", "TOT_DOWN_RETX_BYTES", 
                    "FAIL_DROP_REQ_MSGS", "FAIL_DROP_REQ_BYTES", "FAIL_DROP_RES_MSGS", "FAIL_DROP_RES_BYTES", 
                    "UP_HEAD_CNT", "DOWN_HEAD_CNT", "NORM_UDR_CNT", "ABNORM_UDR_CNT"                                             
                };
    int row_index;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[2].paraVal); i++){
        rxReqMsg->head.para[2].paraVal[i] = toupper(rxReqMsg->head.para[2].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_SVC_WIPI_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_SVC_WIPI_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_SVC_WIPI_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_SVC_WIPI_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[4].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[4].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-svc");
        
    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }
        sprintf(query, "SELECT TRAN_TYPE, "
                        "TOT_REQ_PKTS, TOT_REQ_BYTES, TOT_RES_PKTS, TOT_RES_BYTES, "
                        "TOT_UP_RETX_PKTS, TOT_UP_RETX_BYTES, TOT_DOWN_RETX_PKTS, TOT_DOWN_RETX_BYTES, "
                        "FAIL_DROP_REQ_MSGS, FAIL_DROP_REQ_BYTES, FAIL_DROP_RES_MSGS, FAIL_DROP_RES_BYTES, "
                        "UP_HEAD_CNT, DOWN_HEAD_CNT, NORM_UDR_CNT, ABNORM_UDR_CNT, "                                  
                        "stat_date "
                        " from %s "
                        " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' ORDER BY stat_date ",
                    table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

            sprintf(resTmp, "    =========================================================\n");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %-26s %15s\n",   "","STATTIME","MSG_TYPE", row[0]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    =========================================================\n");
            strcat(ipafBuf,resTmp);

            /* stat_date */
            if(strlen(row[17]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[17]+5,14);
            }

            for ( row_index=0 ; row_index<SVC_WIPI_ROW_CNT ;row_index++ )
            {
                if(row_index==0)
                    strcpy(tmp,key_time);
                else
                    strcpy(tmp,"");
                sprintf(resTmp, "%3s %-14s %-26s %15s\n",
                    "", tmp, title[row_index], row[row_index+1]);
                strcat(ipafBuf,resTmp);

                if (strlen (ipafBuf) > 3000) {
                    strcat (ipafBuf, "    ---------------------------------------------------------\n");
                    stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                    memset (ipafBuf, 0x00, sizeof(ipafBuf));
                    sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                        StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

                    sprintf(resTmp, "    =========================================================\n");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "%3s %-14s %-26s %15s\n",   "","STATTIME","MSG_TYPE", row[0]);
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "    =========================================================\n");
                    strcat(ipafBuf,resTmp);
                }
            }
            sprintf(resTmp, "    ---------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);


            select_cnt++;
            snd_cnt++;

            for(k=0;k<18;k++){ 
                tot_row[k]+=atoll(row[k+1]);
            }
        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-26s %15s\n", "","MSG_TYPE", "SUM");
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);

            for ( row_index=0 ; row_index<SVC_WIPI_ROW_CNT ;row_index++ )
            {
                sprintf(resTmp, "%3s %-26s %15lld\n",
                    "", title[row_index], tot_row[row_index]);
                strcat(ipafBuf,resTmp);

                if (strlen (ipafBuf) > 4000) {
                    strcat (ipafBuf, "    ---------------------------------------------------------\n");
                    stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                    memset (ipafBuf, 0x00, sizeof(ipafBuf));
                    sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                        StatisticSystemInfo[i].sysName,rxReqMsg->head.para[2].paraVal,stime,etime);

                    sprintf(resTmp, "    ==========================================\n");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "%3s %-26s %15s\n", "","MSG_TYPE", "SUM");
                    strcat(ipafBuf,resTmp);
                    sprintf(resTmp, "    ==========================================\n");
                    strcat(ipafBuf,resTmp);
                }
            }
            sprintf(resTmp, "    ==========================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    ==========================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} /* End of stmd_mmc_srch_stat_wipi */

#endif // by jjinri 2009.04.18

int stmd_mmc_add_stat_schd(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             list;
    int             i, cnt, dup, seqNo=1;
    short           mprd;
    int             syear, smon, sday, shour, smin;
    char            stime[32];
    time_t          now;
    struct tm       *pLocalTime;
    char            tmp[12];
    int             i_tmp;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (CRONJOB)) < 0) {
        sprintf (resBuf,"\n    DSC      RESULT = FAIL\n    FAIL REASON = CRONJOB STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    }


    for(i = 0 ; i < STMD_MASK_ITEM_NUM ; i++) 
	{
        if ( !strcasecmp( strITEM[i], rxReqMsg->head.para[0].paraVal) ) 
        {
            for ( dup = 0; dup < list; dup++)
            {
                if (cronJOB[dup].statisticsType == i)
                {
                    sprintf (resBuf,"\n    DSC  RESULT = FAIL\n    FAIL REASON = ALREADY REGISTERED ITEM \n");
                    stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
                    return -1;
                }
            }
            cronJOB[list].statisticsType = i;
            break;
        }
    }

    // default 값
    strcpy(cronJOB[list].sysName, "ALL");
    mprd = 5; 
    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "MPRD"))
        {
            mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
            mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            break;
        } 
    }
    cronJOB[list].period = mprd;

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 32);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(tmp, 0, 12); memcpy(tmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(tmp) > 0 && atoi(tmp) < 13){
            strcat(stime, "-"); strcat(stime, tmp);
            memset(tmp, 0, 12); memcpy(tmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(tmp) > 0 && atoi(tmp) < 32){
                strcat(stime, "-"); strcat(stime, tmp);
                memset(tmp, 0, 12); memcpy(tmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(tmp) >= 0 && atoi(tmp) < 24){
                    strcat(stime, " "); strcat(stime, tmp);
                    memset(tmp, 0, 12); memcpy(tmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(tmp) > 0 && atoi(tmp) < 60){
                        i_tmp = atoi(tmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(tmp, "%d", i_tmp);  
                        strcat(stime, ":"); strcat(stime, tmp);
                    }
                }
            }
        }
    }

    if(strlen(stime) != 16){ //위에서 stime을 정확히 구성했다면 16자리가 된다
    //만약 16자리가 아니면 현재 시간을 구성한다
        now = time(0);
        now = ((now+(cronJOB[list].period*60))/STAT_OFFSET_UNIT) *STAT_OFFSET_UNIT;
        if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
            cronJOB[list].statisticsType = NOT_REGISTERED;
            sprintf (resBuf,"\n    DSC  RESULT = FAIL\n    FAIL REASON = TIME FUNCTION ERR\n");
            stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
            return -1;
        } else {
            strftime (stime, 32, "%Y-%m-%d %H:%M", pLocalTime);
        }
    }

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);

		/*
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            for (i = 0; i < strlen(rxReqMsg->head.para[cnt].paraVal); i++)
                rxReqMsg->head.para[cnt].paraVal[i] = toupper(rxReqMsg->head.para[cnt].paraVal[i]);

            if( strcasecmp(rxReqMsg->head.para[cnt].paraVal, SYSCONF_SYSTYPE_BSD) && 
                    strcasecmp(rxReqMsg->head.para[cnt].paraVal, SYSCONF_SYSTYPE_OMP) && 
                    strcasecmp(rxReqMsg->head.para[cnt].paraVal, "ALL") ){
                cronJOB[list].statisticsType = NOT_REGISTERED;
                sprintf (resBuf,"\n    DSC  RESULT = FAIL\n    FAIL REASON = BAD PARAMETER (INVALID SYSNAME)\n");
                stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);

                return -1;
            }

            if( strcasecmp(rxReqMsg->head.para[0].paraVal, strITEM[0]) &&
                    strcasecmp(rxReqMsg->head.para[0].paraVal, strITEM[1]) &&
                    strcasecmp(rxReqMsg->head.para[0].paraVal, strITEM[2]) &&
                    strcasecmp(rxReqMsg->head.para[0].paraVal, strITEM[3]) &&
                    strcasecmp(rxReqMsg->head.para[0].paraVal, strITEM[4]) )
            {
                cronJOB[list].statisticsType = NOT_REGISTERED;
                sprintf (resBuf,"\n    DSC  RESULT = FAIL\n    FAIL REASON = BAD PARAMETER (INVALID SYSNAME)\n");
                stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);

                return -1;
            } 
            else
                strcpy(cronJOB[list].sysName, rxReqMsg->head.para[cnt].paraVal);
        }
        else*/
		if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME"))
        {
            if (stmd_checkParaTimeValue(rxReqMsg->head.para[cnt].paraVal,&syear,&smon,&sday,&shour,&smin) < 0) {
                cronJOB[list].statisticsType = NOT_REGISTERED;
                sprintf (resBuf,"\n    DSC  RESULT = FAIL\n    FAIL REASON = INVALID START TIME VALUE\n");
                stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
                return -1;
            }

            sprintf(stime, "%04d-%02d-%02d %02d:%02d", syear, smon, sday, shour, smin);
            // 현재 시간보다 작으면 error 처리
            if (strcasecmp(stime, get_select_time(STMD_MIN)) <= 0) {
                cronJOB[list].statisticsType = NOT_REGISTERED;
                sprintf (resBuf,"\n    DSC  RESULT = FAIL\n    FAIL REASON = INVALID TIME RANGE\n");
                stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
                return -1;
            }
        }
    }

    strcpy(cronJOB[list].measureTime, stime);
    if ( writeCronJobInFile() < 0) {
        cronJOB[list].statisticsType = NOT_REGISTERED;
        sprintf (resBuf,"\n    DSC  RESULT = FAIL\n    FAIL REASON = WRITE FAIL\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return -1;
    } else 
        sprintf(resBuf, "\n    DSC  RESULT = SUCCESS & JOBNO[%d] ADDED\n", list);

    stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

    return 1;
}

int stmd_mmc_del_stat_schd(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int             jobNo, seqNo=1;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    jobNo = atoi(rxReqMsg->head.para[0].paraVal);

    if (cronJOB[jobNo].statisticsType == NOT_REGISTERED) {
        sprintf(resBuf, "\n    DSC  RESULT = FAIL\n    FAIL REASON = NO SUCH JOB\n");
    } else {
        cronJOB[jobNo].statisticsType = NOT_REGISTERED;
        cronJOB[jobNo].period         = 0;
        cronJOB[jobNo].sysName[0]     = 0;
        cronJOB[jobNo].measureTime[0] = 0;
        if ( writeCronJobInFile() < 0) {
            sprintf(resBuf, "\n    DSC  RESULT = FAIL\n    FAIL REASON = WRITE CRON FILE FAIL\n");
        } else 
            //sprintf(resBuf, "\n    BSD  RESULT = SUCCESS & JOBNO[%d] DELETED\n", jobNo);
            sprintf(resBuf, "\n    DSC  RESULT = SUCCESS & JOBNO[%d] DELETED\n", jobNo);
    }
    stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

    return 1;
}

int stmd_mmc_dis_stat_schd(IxpcQMsgType *rxIxpcMsg)
{
    int     i, flag = 0, seqNo=1;

    memset(resBuf, 0, 4096);

    sprintf(resBuf, "    %-5s  %-7s  %-10s  %4s  %s\n    ---------------------------------------\n",
            "JOBNO", "SYSTEM", "ITEM", "MPRD", "MTIME");

    for ( i=0 ; i < MAX_CRONJOB_NUM; i++) {
        if ( cronJOB[i].statisticsType != NOT_REGISTERED) {
            flag = 1;
            sprintf(resTmp, "    %-5d  %-7s  %-10s  %4d  %s\n",
                i, cronJOB[i].sysName,
                strITEM[cronJOB[i].statisticsType],
                cronJOB[i].period,
                cronJOB[i].measureTime);
            strcat(resBuf, resTmp);
        }
    }

    if (flag == 0 ) {
        //sprintf(resBuf, "\n    BSD  RESULT = FAIL\n    FAIL REASON = NOT EXIST SCHEDULE STATISTICS\n");
        sprintf(resBuf, "\n    DSC  RESULT = FAIL\n    FAIL REASON = NOT EXIST SCHEDULE STATISTICS\n");
    }

    stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

    return 1;
}


int stmd_mmc_dis_stat_info(IxpcQMsgType *rxIxpcMsg)
{
//    int     i, flag = 0, seqNo=1;
    int     seqNo=1;

    sprintf (resBuf, "\n");
    strcat (resBuf, "    STAT-FAULT    [STM], [MPRD], [MTIM]");
    strcat (resBuf, "\n    STAT-LOAD    [STM], [MPRD], [MTIM]");
    strcat (resBuf, "\n    STAT-SCIB    [IP], [STM], [MPRD], [MTIM]");
    strcat (resBuf, "\n    STAT-RCIF    [IP], [STM], [MPRD], [MTIM]");
    strcat (resBuf, "\n    STAT-SCPIF   [SCPID], [STM], [MPRD], [MTIM]");
    strcat (resBuf, "\n    STAT-SCPIF    [STM], [MPRD], [MTIM]");
    strcat (resBuf, "\n    STAT-DB      [STM], [MPRD], [MTIM]");
    strcat (resBuf, "\n    DIS-STAT-HIS  ITEM, TYPE, [STM], [COUNT]");
    strcat (resBuf, "\n    DIS-STAT-SCHD ");
    strcat (resBuf, "\n    DEL-STAT-SCHD JOB");
    strcat (resBuf, "\n    DIS-STAT-INFO");
    strcat (resBuf, "\n    STAT-CMD-CANC JOB\n");

    stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

    return 1;
}


int stmd_mmc_dis_stat_nms(IxpcQMsgType *rxIxpcMsg)
{
#if 0
    MMLReqMsgType   *rxReqMsg;
    char            seqNo=1, resFlag=0;
    struct  tm      mktime_nms;
    struct  tm      *pLocalTime;
    time_t          time_nms;
    char            nms_time[32];
    char            nms_end_time[32];
    int             syear, smon, sday, shour, smin;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
#endif
#if 0

    if (!strcasecmp(rxReqMsg->head.para[0].paraVal, "")) 
    {
        /*strcpy(nms_time, (char *)get_nms_select_time());
        strcpy(nms_end_time, (char *)get_period_end_time(STMD_HOUR));*/
        strcpy(nms_time, (char *)(long)get_nms_select_time());
        strcpy(nms_end_time, (char *)(long)get_period_end_time(STMD_HOUR));
    } 
    else 
    {
        if (stmd_nms_checkParaTimeValue(rxReqMsg->head.para[0].paraVal, &syear, &smon, &sday, &shour) < 0) {
            strcpy (resBuf,"    BSD  RESULT = FAIL\n    FAIL REASON = INVALID_START_TIME_VALUE\n");
            stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
            return;
        }
        sprintf(nms_time, "%04d-%02d-%02d %02d:00", syear, smon, sday, shour);

        if (strcasecmp(nms_time, get_select_time(STMD_HOUR)) >= 0) {
            strcpy (resBuf,"    BSD  RESULT = FAIL\n    FAIL REASON = INVALID_TIME_RANGE\n");
            stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
            return;
        }
        mktime_nms.tm_year = syear - 1900;
        mktime_nms.tm_mon = smon - 1;
        mktime_nms.tm_mday = sday;
        mktime_nms.tm_hour = shour;
        mktime_nms.tm_min = 0;
        mktime_nms.tm_sec = 0;
        mktime_nms.tm_isdst = 0;
        time_nms = mktime(&mktime_nms);
        time_nms = time_nms + 3600;

        pLocalTime = (struct tm*)localtime((time_t*)&time_nms);
        strftime (nms_end_time, 32, "%Y-%m-%d %H:00", pLocalTime);
    }

    strcpy(resBuf, "\n    BSD  RESULT = SUCCESS\n");
    stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 10, 1, seqNo++);

    if ( stmd_nms_LoadSelect(STMD_HOUR, STM_STATISTIC_HOUR_LOAD_TBL_NAME, nms_time, nms_end_time) < 0 ) {
        sprintf(trcBuf, "stmd_nms_LoadSelect fail\n");
        trclib_writeLogErr(FL, trcBuf);
    }
    else
        resFlag = 1;

    if ( stmd_nms_FaultSelect(STMD_HOUR, STM_STATISTIC_HOUR_FAULT_TBL_NAME, STM_STATISTIC_HOUR_HW_FLT_TBL_NAME, nms_time, nms_end_time) < 0 ) {
        sprintf(trcBuf, "stmd_nms_FaultSelect fail\n");
        trclib_writeLogErr(FL, trcBuf);
    }
    else
        resFlag = 1;

    if (resFlag == 0)
    {
        strcpy (resBuf, "\n    BSD RESULT = FAIL \n    FAIL  REASON = NO EXIST STATISTICS HISTORY\n");
        stmd_txMsg2Nmsib(resBuf, (STSCODE_STM_PERIODIC_FAULT_HOUR - STSCODE_TO_MSGID_STATISTICS), 0, 1);
    }
#endif

    return 1;
}

int stmd_mmc_dis_stat_ptime(IxpcQMsgType *rxIxpcMsg)
{
    char    temp[60];

    sprintf ( temp,    "\n    HOURLY  PRINT TIME  = %2d min [every hour]\n",printTIME[STMD_HOUR]);
    strcat  ( resBuf, temp );
    sprintf ( temp,      "    DAILY   PRINT TIME  = %2d min [every day 00 hour]\n",printTIME[STMD_DAY]);
    strcat  ( resBuf, temp );
    sprintf ( temp,      "    WEEKLY  PRINT TIME  = %2d min [every monday 00 hour]\n",printTIME[STMD_WEEK]);
    strcat  ( resBuf, temp );
    sprintf ( temp,      "    MONTHLY PRINT TIME  = %2d min [every month  00 hour]\n",printTIME[STMD_MONTH]);
    strcat  ( resBuf, temp );
    
    stmd_txMMLResult (rxIxpcMsg, resBuf, 1, 0, 0, 0, 0);
    
    return 1;
}

int stmd_mmc_chg_stat_ptime(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     type, timer;
    char    temp[1024];
    
    memset(temp, 0, sizeof(temp));

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
        
    if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"HOURLY") )
    {
        type = STMD_HOUR;
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"DAILY") )
    {
        type = STMD_DAY;
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"WEEKLY") )
    {
        type = STMD_WEEK;
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"MONTHLY") )
    {
        type = STMD_MONTH;
    }
    
//    printf("Type : %s, %d\n",rxReqMsg->head.para[0].paraVal, type);
    
    timer = atoi(rxReqMsg->head.para[1].paraVal);   //TIMER
    printf("Timer : %d\n", timer);
    
    if( (timer % 5) != 0)
    {
        strcpy (resBuf, "\n    RESULT = FAIL \n    FAIL  REASON = TIMER VALUE IS NOT 5 MULTIPLE\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 0);
        
        return 1;
    }
    
    if(printTIME[type] == timer)
    {
        strcpy (resBuf, "\n    RESULT = FAIL \n    FAIL  REASON = VALUE_IS_NOT_DIFFERENT\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 0);
        return 1;
    }
    
    sprintf (resBuf, "\n    RESULT = SUCCESS \n");
    sprintf (temp,     "    TYPE   = %s \n", rxReqMsg->head.para[0].paraVal);
    strcat  ( resBuf, temp );
    sprintf (temp,     "    TIMER  = %d \n", timer);
    strcat  ( resBuf, temp );
    
    stmd_txMMLResult (rxIxpcMsg, resBuf, 1, 0, 0, 0, 0);
    
    /* Memory & File change */
    printTIME[type] = timer;
    writePrintTime();
    
    return 1;
}

int stmd_mmc_mask_stat_item(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     item, type;
    char    temp[1024];
    
    memset(temp, 0, sizeof(temp));

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"LOAD") )
    {
        item = STMD_LOAD;   //1
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"FAULT") )
    {
        item = STMD_FAULT;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"LINK") )
    {
        item = STMD_LINK;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"ACCOUNT") )
    {
        item = STMD_LEG;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"LOGON") )
    {
        item = STMD_LOGON;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"RULE_SET") )
    {
        item = STMD_RULE_SET;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"RULE_ENT") )
    {
        item = STMD_RULE_ENT;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"SMS") )
    {
        item = STMD_SMS;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"DELAY") )
    {
        item = STMD_DEL;  //0
    }
#if 0
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"IPAF") )
    {
        item = STMD_FAULT;  //2
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"AAA") )
    {
        item = STMD_AAA;    //3
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"MEAN") )
    {
        item = STMD_FAULT;  //4
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"UAWAP") )
    {
        item = STMD_FAULT;  //5
    }
#endif // by jjinri 2009.04.18
    else
    {
        strcpy (resBuf, "\n    RESULT = FAIL \n    FAIL  REASON = UNKNOWN ITEM\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 0);
        return 1;
    }
    
	if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOURLY") )
    {
        type = STMD_HOUR;
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAILY") )
    {
        type = STMD_DAY;
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"WEEKLY") )
    {
        type = STMD_WEEK;
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"MONTHLY") )
    {
        type = STMD_MONTH;
    }
    else
    {
        type = STMD_HOUR;
    }
    
    printf("stmd_mmc_mask_stat_item : item:%d = type:%d\n",item, type);
    
    if(maskITEM[item][type] == MASK)
    {
        strcpy (resBuf, "\n    RESULT = FAIL \n    FAIL  REASON = VALUE_IS_NOT_DIFFERENT\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 0);
        return 1;
    }
    
    strcpy  (resBuf, "\n    RESULT = SUCCESS \n");
    sprintf (temp,     "    ITEM   = %s \n", toUpper(rxReqMsg->head.para[0].paraVal));
    strcat  ( resBuf, temp );
    sprintf (temp,     "    TYPE   = %s    MASKED\n", toUpper(rxReqMsg->head.para[1].paraVal));
    strcat  ( resBuf, temp );
    
    stmd_txMMLResult (rxIxpcMsg, resBuf, 1, 0, 0, 0, 0);
    
    maskITEM[item][type] = MASK;
    writePrintMaskValue();
    
    return 1;
}

int stmd_mmc_umask_stat_item(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     item, type;
    char    temp[1024];
    
    memset(temp, 0, sizeof(temp));

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"LOAD") )
    {
        item = STMD_LOAD;   //1
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"FAULT") )
    {
        item = STMD_FAULT;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"LINK") )
    {
        item = STMD_LINK;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"ACCOUNT") )
    {
        item = STMD_LEG;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"LOGON") )
    {
        item = STMD_LOGON;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"RULE_SET") )
    {
        item = STMD_RULE_SET;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"RULE_ENT") )
    {
        item = STMD_RULE_ENT;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"SMS") )
    {
        item = STMD_SMS;  //0
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"DELAY") )
    {
        item = STMD_DEL;  //0
    }
    
#if 0
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"IPAF") )
    {
        item = STMD_FAULT;  //2
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"AAA") )
    {
        item = STMD_FAULT;  //3
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"MEAN") )
    {
        item = STMD_FAULT;  //4
    }
    else if( !strcasecmp(rxReqMsg->head.para[0].paraVal,"UAWAP") )
    {
        item = STMD_FAULT;  //5
    }
#endif // by jjinri 2009.04.18
    else
    {
        strcpy (resBuf, "\n    RESULT = FAIL \n    FAIL  REASON = UNKNOWN ITEM\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 0);
        return 1;
    }
    
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOURLY") )
    {
        type = STMD_HOUR;
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAILY") )
    {
        type = STMD_DAY;
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"WEEKLY") )
    {
        type = STMD_WEEK;
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"MONTHLY") )
    {
        type = STMD_MONTH;
    }
    else
    {
        type = STMD_HOUR;
    }
    
    printf("stmd_mmc_mask_stat_item : item:%d = type:%d\n",item, type);
    
    if(maskITEM[item][type] == UNMASK)
    {
        strcpy (resBuf, "\n    RESULT = FAIL \n    FAIL  REASON = VALUE_IS_NOT_DIFFERENT\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 0);
        return 1;
    }
    
    strcpy  (resBuf, "\n    RESULT = SUCCESS \n");
    sprintf (temp,     "    ITEM   = %s \n", toUpper(rxReqMsg->head.para[0].paraVal));
    strcat  ( resBuf, temp );
    sprintf (temp,     "    TYPE   = %s    UNMASKED\n", toUpper(rxReqMsg->head.para[1].paraVal));
    strcat  ( resBuf, temp );
    
    stmd_txMMLResult (rxIxpcMsg, resBuf, 1, 0, 0, 0, 0);
    
    maskITEM[item][type] = UNMASK;
    writePrintMaskValue();
    
    return 1;
}

#define MU(a)   a == MASK ? "MASKED" : "UNMASKED"
int stmd_mmc_dis_stat_mask(IxpcQMsgType *rxIxpcMsg)
{
    char    tmpMsg[512];
    int     i;
    
    memset(tmpMsg, 0, sizeof(tmpMsg));
    
    strcpy (resBuf,"    ==============================================");
    sprintf (tmpMsg, "\n    %10s %8s %8s %8s %8s\n",
            " ITEM "," HOURLY ","  DAILY ", " WEEKLY "," MONTHLY" );
    strcat  ( resBuf, tmpMsg );
    sprintf(tmpMsg,"    ==============================================\n");
    strcat  ( resBuf, tmpMsg );
    
    for ( i=0; i<STMD_MASK_ITEM_NUM; i++)
    {
        sprintf (tmpMsg, "    %10s %8s %8s %8s %8s\n",
                strITEM[i],        MU(maskITEM[i][1]),MU(maskITEM[i][2]),
                MU(maskITEM[i][3]),MU(maskITEM[i][4]) );
        strcat  ( resBuf, tmpMsg );
    }
    
    sprintf (tmpMsg,"    ==============================================\n");
    strcat  ( resBuf, tmpMsg );
    
    stmd_txMMLResult (rxIxpcMsg, resBuf, 1, 0, 0, 0, 0);
    
    return 1;
}
#if 0
int stmd_mmc_stat_tcpip(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     list;
    int     i, cnt, diffTime, seqNo=1;
    short   mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }
 
    //strcpy(onDEMAND[list].svcName, "BSDA");
    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);

        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                //printf("jean========stmd_mmc_stat_load no sys name\n");
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STM"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal);
fprintf(stderr, "stime: %s  diffTime: %d\n", rxReqMsg->head.para[cnt].paraVal, diffTime);            
	    }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;
    onDEMAND[list].statisticsType = STMD_TRAN;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;
    
    if ((diffTime >= 0) && (diffTime <= 3600))
    {
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));

        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    COUNT       = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        /*sprintf(resHead,"    STIME    = %s\n    PERIOD   = %d\n    CNT      = %d\n",
           onDEMAND[list].measureTime, onDEMAND[list].period, onDEMAND[list].count);*/
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);
    }
    else 
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s", 
                    get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandIpaf(list);
    }
    
    return 1;
}
#define TITLE_COUNT     9
int stmd_mmc_srch_stat_tcpip(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[40]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt;

/*  char *title[]={"TOT","IP","TCP","UDP","IP_ERR","UTCP_ERR","ETC","USER","DROP","ETC_ERR"}; */
    char *title[]={"TOT","FILTER_OUT","UDP","TCP","ETC","USER","IP_ERR","UTCP_ERR","ETC_ERR"};
    int row_index;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_IPAF_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_IPAF_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_IPAF_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_IPAF_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-tcpip");

    sprintf(resTmp, "    ====================================================================\n");
    strcat(ipafHead, resTmp);
    sprintf(resTmp, "    STATTIME       STATTYPE               UP_FRAMES          DOWN_FRAMES\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                           UP_BYTES           DOWN_BYTES\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    ====================================================================\n");
    strcat(ipafHead,resTmp);

    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }
///////////////////////////

        sprintf(query, "SELECT stat_date, "
/*          "tot_up_frames, tot_down_frames, tot_up_bytes, tot_down_bytes, " */ /*excluded 2006.08.18 */
            "ip_up_frames, ip_down_frames, ip_up_bytes, ip_down_bytes, "                         // total
            "filterout_up_frames, filterout_down_frames, filterout_up_bytes, filterout_down_bytes, "                     // filterout
            "udp_up_frames, udp_down_frames, udp_up_bytes, udp_down_bytes, "                     // udp
            "tcp_up_frames, tcp_down_frames, tcp_up_bytes, tcp_down_bytes, "                     // tcp
            "tcp_re_up_frames, tcp_re_down_frames, tcp_re_up_bytes, tcp_re_down_bytes, "         // etc
            "out_up_frames, out_down_frames, out_up_bytes, out_down_bytes, "                     // user
            "ip_err_up_frames, ip_err_down_frames, ip_err_up_bytes, ip_err_down_bytes, "         // ip_err
            "utcp_err_up_frames, utcp_err_down_frames, utcp_err_up_bytes, utcp_err_down_bytes, " // utcp_err
/*          "drop_up_frames, drop_down_frames, drop_up_bytes, drop_down_bytes, " */ /*excluded 2006.08.18 */
            "fail_up_frames, fail_down_frames, fail_up_bytes, fail_down_bytes "                  // etc_err
            " from %s "
            " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' ORDER BY stat_date ",
            table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName, rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            if(strlen(row[0]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[0]+5,14);
            }

            row_index=1;
            for(j=0;j<TITLE_COUNT;j++){
                if (j ==0){
                    strcpy(tmp,key_time);
                }else {
                    strcpy(tmp,"");
                }
                
                sprintf(resTmp, "%3s %-14s %-11s %20s %20s\n",
                    "",tmp,title[j],row[row_index],row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
                sprintf(resTmp, "%3s %-14s %11s %20s %20s\n","","","",row[row_index],row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
            }
            sprintf(resTmp, "    --------------------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);

            for(k=0;k<(TITLE_COUNT*4);k++){
                tot_row[k]+=atoll(row[k+1]);
            }

            select_cnt++;
            snd_cnt++;
        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName, rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            row_index=0;
            for(j=0;j<TITLE_COUNT;j++){
                if (j ==0){
                    strcpy(tmp,"SUM");
                }else {
                    strcpy(tmp,"");
                }
                sprintf(resTmp, "%3s %-14s %-11s %20lld %20lld\n",
                    "",tmp,title[j],tot_row[row_index],tot_row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
                sprintf(resTmp, "%3s %-14s %11s %20lld %20lld\n","","","",tot_row[row_index],tot_row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
            }
            sprintf(resTmp, "    ====================================================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

///////////////////////////
    }

    if (snd_cnt == 1) {
        //sprintf(resTmp, "      %-8s  %10s  %10s  %10s  %10s\n",
        //  "", "0.0", "0.0", "0.0", "0.0");
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
        sprintf(resTmp, "    ====================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;
}

int stmd_mmc_stat_if(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     list;
    int     i, cnt, diffTime, seqNo=1;
    short   mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }
 
    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraVal); i++)
            rxReqMsg->head.para[cnt].paraVal[i] = toupper(rxReqMsg->head.para[cnt].paraVal[i]);
        
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "IFTYPE"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                if(!strcasecmp("AAA",rxReqMsg->head.para[cnt].paraVal)){
                    onDEMAND[list].statisticsType = STMD_AAA;
                    onDEMAND[list].sysNO = 1;
                }
                else if(!strcasecmp("WAP_GW",rxReqMsg->head.para[cnt].paraVal)){
                    onDEMAND[list].statisticsType = STMD_UAWAP;
                }
                else if(!strcasecmp("AN_AAA",rxReqMsg->head.para[cnt].paraVal)){
                    onDEMAND[list].statisticsType = STMD_AN_AAA;
                    onDEMAND[list].sysNO = 2;
                }   
                else if(!strcasecmp("FAIL_UDR",rxReqMsg->head.para[cnt].paraVal)){
                    onDEMAND[list].statisticsType = STMD_FAIL_UDR;
                    onDEMAND[list].sysNO = 0;
                }
		else{
                    return;
                }
            }
            else{
                return;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "IP"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                    strcpy(onDEMAND[list].ipAddr,rxReqMsg->head.para[cnt].paraVal);
            }
            else{
                return;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STM"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal);
            }
        }
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));

        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    COUNT       = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
    }
    else 
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s", 
                    get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandAaa(list);
        doOnDemandANAaa(list); // by helca 2007.01.05
   	doOnDemandFailUdr(list); // by helca 2007.05.31 
    }
    return 1;
}
int stmd_mmc_srch_stat_if(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[40]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    memset(tot_row,0,sizeof(tot_row));
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
//printf("jean ********************** %s \n", rxReqMsg->head.para[1].paraVal);

    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"WAP_GW") ){
        stmd_mmc_srch_stat_if_uawap( rxIxpcMsg );
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"AAA") ){
        stmd_mmc_srch_stat_if_aaa( rxIxpcMsg );
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"AN_AAA") ){
        stmd_mmc_srch_stat_if_an_aaa( rxIxpcMsg );    
    }else{ // FAIL UDR
	stmd_mmc_srch_stat_fail_udr( rxIxpcMsg );
    }

    return 1;
}

int stmd_mmc_srch_stat_if_uawap(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[22]={0,};
    char  key_time[33];
    char  sysip[30]={0,};

    int         select_cnt = 0, snd_cnt;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[3].paraVal); i++){
        rxReqMsg->head.para[3].paraVal[i] = toupper(rxReqMsg->head.para[3].paraVal[i]);
    }


    if( !strcasecmp(rxReqMsg->head.para[3].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_UAWAP_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[3].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_UAWAP_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[3].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_UAWAP_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_UAWAP_TBL_NAME);
    }

    strcpy(sysip,rxReqMsg->head.para[2].paraVal);
    

    if(strlen(rxReqMsg->head.para[4].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[4].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[5].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[5].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-if");

    /* 2006.08.21 : uawap 통계 출력 시 TOT_EXT_WAP_LOG 필드 미출력 by sdlee
    */
    sprintf(resTmp, "    ============================================================================================\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    STATTIME            TOT_EXT_LOG TOT_EXT_DEC_ERR_LOG   NO_IPPOOL_WAP_LOG           TOT_WAPLOG\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                        TOT_SUCCESS            TOT_FAIL             NO_CALL              TX_FAIL\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                           ETC_FAIL            COMPLETE             TIMEOUT            CANCELLED\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                        TOT_UP_BYTE         TOT_DN_BYTE     NO_CALL_UP_BYTE      NO_CALL_DN_BYTE\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                   ETC_FAIL_UP_BYTE    ETC_FAIL_DN_BYTE      GET_METHOD_CNT      POST_METHOD_CNT\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                     ETC_METHOD_CNT\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    ============================================================================================\n");
    strcat(ipafHead,resTmp);


    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }

///////////////////////////
        if( !(strcasecmp(sysip,"0.0.0.0"))) {
            sprintf(query, "SELECT SUM(TOT_EXT_LOG), SUM(TOT_EXT_WAP_LOG), SUM(TOT_EXT_DEC_ERR_LOG), SUM(NO_IPPOOL_WAP_LOG), SUM(TOT_WAP_LOG) "
                ", SUM(TOT_SUCCESS), SUM(TOT_FAIL) ,SUM(NO_CALL) ,SUM(TX_FAIL) ,SUM(ETC_FAIL) "
                ", SUM(COMPLETE), SUM(TIMEOUT) ,SUM(CANCELLED) , SUM(TOT_UP_BYTES) ,SUM(TOT_DOWN_BYTES) "
                ", SUM(NO_CALL_UP_BYTES) , SUM(NO_CALL_DOWN_BYTES) ,SUM(ETC_FAIL_UP_BYTES) ,SUM(ETC_FAIL_DOWN_BYTES), SUM(GET_METHOD_CNT) "
                ", SUM(POST_METHOD_CNT),  SUM(ETC_METHOD_CNT) , stat_date"
                " from %s "
                " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' GROUP BY stat_date ORDER BY stat_date ",
                table_name, StatisticSystemInfo[i].sysName, stime,etime
            );
        } else {
            sprintf(query, "SELECT SUM(TOT_EXT_LOG), SUM(TOT_EXT_WAP_LOG), SUM(TOT_EXT_DEC_ERR_LOG), SUM(NO_IPPOOL_WAP_LOG), SUM(TOT_WAP_LOG) "
                ", SUM(TOT_SUCCESS), SUM(TOT_FAIL) ,SUM(NO_CALL) ,SUM(TX_FAIL) ,SUM(ETC_FAIL) "
                ", SUM(COMPLETE), SUM(TIMEOUT) ,SUM(CANCELLED) , SUM(TOT_UP_BYTES) ,SUM(TOT_DOWN_BYTES) "
                ", SUM(NO_CALL_UP_BYTES) , SUM(NO_CALL_DOWN_BYTES) ,SUM(ETC_FAIL_UP_BYTES) ,SUM(ETC_FAIL_DOWN_BYTES), SUM(GET_METHOD_CNT) "
                ", SUM(POST_METHOD_CNT),  SUM(ETC_METHOD_CNT) , stat_date"
                " from %s "
                " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' AND sys_ip = '%s' GROUP BY stat_date ORDER BY stat_date ",
                table_name, StatisticSystemInfo[i].sysName, stime,etime, sysip
            );
        }

//printf("===23=jean\n%s\n",query);
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s IFTYPE = WAP_GW SYSNO = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,sysip,rxReqMsg->head.para[3].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            if(strlen(row[22]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[22]+5,14);
            }

            /* 2006.08.21 : uawap 통계 출력 시 TOT_EXT_WAP_LOG(row[1]) 필드 미출력 by sdlee
            */
            sprintf(resTmp, "%3s %-14s %16s %19s %19s %20s\n",
                    "", key_time, row[0], row[2], row[3], row[4]);
            strcat (ipafBuf, resTmp);

            sprintf (resTmp, "%3s %-14s %16s %19s %19s %20s\n",
                    "", "", row[5], row[6], row[7], row[8]);
            strcat (ipafBuf, resTmp);

            sprintf (resTmp, "%3s %-14s %16s %19s %19s %20s\n",
                    "", "", row[9], row[10], row[11], row[12]);
            strcat (ipafBuf, resTmp);

            sprintf (resTmp, "%3s %-14s %16s %19s %19s %20s\n",
                    "", "", row[13], row[14], row[15], row[16]);
            strcat (ipafBuf, resTmp);

            sprintf (resTmp, "%3s %-14s %16s %19s %19s %20s\n",
                    "", "", row[17], row[18], row[19], row[20]);
            strcat (ipafBuf, resTmp);

            sprintf (resTmp, "%3s %-14s %16s\n", "", "", row[21]);
            strcat(ipafBuf, resTmp);

            sprintf(resTmp, "    --------------------------------------------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);

            select_cnt++;
            snd_cnt++;

            for(k=0;k<22;k++){
                tot_row[k]+=atoll(row[k]);
            }
        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s IFTYPE = WAP_GW SYSNO = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,sysip,rxReqMsg->head.para[3].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            /* 2006.08.21 : uawap 통계 출력 시 TOT_EXT_WAP_LOG(tot_row[1]) 필드 미출력 by sdlee
            */
            sprintf(resTmp, "%3s %-14s %16lld %19lld %19lld %20lld\n"
                                "%3s %-14s %16lld %19lld %19lld %20lld\n"
                                "%3s %-14s %16lld %19lld %19lld %20lld\n"
                                "%3s %-14s %16lld %19lld %19lld %20lld\n"
                                "%3s %-14s %16lld %19lld %19lld %20lld\n"
                                "%3s %-14s %16lld\n",
                    "", "SUM", tot_row[0], tot_row[2], tot_row[3], tot_row[4],
                    "", "", tot_row[5], tot_row[6], tot_row[7], tot_row[8],
                    "", "", tot_row[9], tot_row[10], tot_row[11], tot_row[12],
                    "", "", tot_row[13], tot_row[14], tot_row[15], tot_row[16],
                    "", "", tot_row[17], tot_row[18], tot_row[19], tot_row[20],
                    "", "", tot_row[21]);
            strcat(ipafBuf, resTmp);

            sprintf(resTmp, "    ============================================================================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

///////////////////////////
    }

    if (snd_cnt == 1) {
        //sprintf(resTmp, "      %-8s  %10s  %10s  %10s  %10s\n",
        //  "", "0.0", "0.0", "0.0", "0.0");
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
        sprintf(resTmp, "    ============================================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

}

int stmd_mmc_srch_stat_if_aaa(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[22]={0,};
    char  key_time[33];
    char  sysip[30]={0,};

    int         select_cnt = 0, snd_cnt;

    long long total,nsucc, nfail;
    double succ_rate;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[3].paraVal); i++){
        rxReqMsg->head.para[3].paraVal[i] = toupper(rxReqMsg->head.para[3].paraVal[i]);
    }


    if( !strcasecmp(rxReqMsg->head.para[3].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_AAA_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[3].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_AAA_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[3].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_AAA_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_AAA_TBL_NAME);
    }

    strcpy(sysip,rxReqMsg->head.para[2].paraVal);
    

    if(strlen(rxReqMsg->head.para[4].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[4].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[5].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[5].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-if");

    sprintf(resTmp, "    ===================================================================================================\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    STATTIME               REQUEST         SUCCESS              FAIL             START   START_TIMEOUT\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                     START_RETRANS         INTERIM   INTERIM_TIMEOUT   INTERIM_RETRANS            STOP\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                      STOP_TIMEOUT    STOP_RETRANS           TIMEOUT       NETWORK_ERR         ETC_ERR\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                      SUCC_RATE(%%)\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    ===================================================================================================\n");
    strcat(ipafHead,resTmp);

    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }

///////////////////////////

        if( !(strcasecmp(sysip,"0.0.0.0"))) {
                sprintf(query, "SELECT SUM(REQUEST), SUM(SUCCESS), SUM(FAIL), SUM(START) "
                " , SUM(START_TIMEOUT), SUM(START_RETRANS), SUM(INTERIM)  "
                " , SUM(INTERIM_TIMEOUT), SUM(INTERIM_RETRANS), SUM(STOP), SUM(STOP_TIMEOUT), SUM(STOP_RETRANS) "
                " , SUM(TIMEOUT), SUM(NETWORK_ERR) , SUM(ETC_ERR), stat_date "
                " from %s "
                " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' GROUP BY stat_date  ORDER BY stat_date ",
                table_name, StatisticSystemInfo[i].sysName, stime,etime
            );
        } else {
                sprintf(query, "SELECT SUM(REQUEST), SUM(SUCCESS), SUM(FAIL), SUM(START) "
                " , SUM(START_TIMEOUT), SUM(START_RETRANS), SUM(INTERIM)  "
                " , SUM(INTERIM_TIMEOUT), SUM(INTERIM_RETRANS), SUM(STOP), SUM(STOP_TIMEOUT), SUM(STOP_RETRANS) "
                " , SUM(TIMEOUT), SUM(NETWORK_ERR) , SUM(ETC_ERR), stat_date "
                " from %s "
                " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' AND sys_ip = '%s' AND sys_no = '%d' GROUP BY stat_date ORDER BY stat_date ",
                table_name, StatisticSystemInfo[i].sysName, stime,etime, sysip, TYPE_AAA
            );
        }

//printf("===23=jean\n%s\n",query);
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s IFTYPE = AAA SYSNO = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,sysip,rxReqMsg->head.para[3].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);
        
            if(strlen(row[15]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[15]+5,14);
            }

            nsucc = atoi(row[1]);
            total = atoi(row[0]);
            if(total == 0){
                succ_rate=0.0;
            }
            else{
                succ_rate = (nsucc/(double)total)*100;
            }

            sprintf(resTmp, "%3s %-14s %15s %15s   %15s   %15s %15s\n"
                ,"", key_time,row[0],row[1],row[2],row[3],row[4]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %15s %15s   %15s   %15s %15s\n"
                ,"", "",row[5],row[6],row[7],row[8],row[9]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %15s %15s   %15s   %15s %15s\n"
                ,"","",row[10],row[11],row[12],row[13],row[14]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %12.2f(%%)\n" ,"","",succ_rate);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    ---------------------------------------------------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);
            
            select_cnt++;
            snd_cnt++;

            for(k=0;k<14;k++){
                tot_row[k]+=atoll(row[k]);
            }
        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s IFTYPE = AAA SYSNO = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,sysip,rxReqMsg->head.para[3].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            nsucc = tot_row[1];
            total = tot_row[0];
            if(total == 0){
                succ_rate=0.0;
            }
            else{
                succ_rate = (nsucc/(double)total)*100;
            }

            sprintf(resTmp, "%3s %-14s %15lld %15lld   %15lld   %15lld %15lld\n"
                ,"", "SUM",tot_row[0],tot_row[1],tot_row[2],tot_row[3],tot_row[4]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %15lld %15lld   %15lld   %15lld %15lld\n"
                ,"", "",tot_row[5],tot_row[6],tot_row[7],tot_row[8],tot_row[9]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %15lld %15lld   %15lld   %15lld %15lld\n"
                ,"","",tot_row[10],tot_row[11],tot_row[12],tot_row[13],tot_row[14]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %12.2f(%%)\n" ,"","",succ_rate);
            strcat(ipafBuf,resTmp);

            sprintf(resTmp, "    ===================================================================================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

///////////////////////////
    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    ===================================================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} // end of stmd_mmc_srch_stat_if_aaa //

int stmd_mmc_srch_stat_if_an_aaa(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[22]={0,};
    char  key_time[33];
    char  sysip[30]={0,};

    int         select_cnt = 0, snd_cnt;

    long long total,nsucc, nfail;
    double succ_rate;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[3].paraVal); i++){
        rxReqMsg->head.para[3].paraVal[i] = toupper(rxReqMsg->head.para[3].paraVal[i]);
    }


    if( !strcasecmp(rxReqMsg->head.para[3].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_AAA_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[3].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_AAA_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[3].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_AAA_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_AAA_TBL_NAME);
    }

    strcpy(sysip,rxReqMsg->head.para[2].paraVal);
    

    if(strlen(rxReqMsg->head.para[4].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[4].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[5].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[5].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-if");

    sprintf(resTmp, "    ===================================================================================================\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    STATTIME               REQUEST         SUCCESS              FAIL             START   START_TIMEOUT\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                     START_RETRANS         INTERIM   INTERIM_TIMEOUT   INTERIM_RETRANS            STOP\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                      STOP_TIMEOUT    STOP_RETRANS           TIMEOUT       NETWORK_ERR         ETC_ERR\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                      SUCC_RATE(%%)\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    ===================================================================================================\n");
    strcat(ipafHead,resTmp);

    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }

///////////////////////////

        if( !(strcasecmp(sysip,"0.0.0.0"))) {
                sprintf(query, "SELECT SUM(REQUEST), SUM(SUCCESS), SUM(FAIL), SUM(START) "
                " , SUM(START_TIMEOUT), SUM(START_RETRANS), SUM(INTERIM)  "
                " , SUM(INTERIM_TIMEOUT), SUM(INTERIM_RETRANS), SUM(STOP), SUM(STOP_TIMEOUT), SUM(STOP_RETRANS) "
                " , SUM(TIMEOUT), SUM(NETWORK_ERR) , SUM(ETC_ERR), stat_date "
                " from %s "
                " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' GROUP BY stat_date  ORDER BY stat_date ",
                table_name, StatisticSystemInfo[i].sysName, stime,etime
            );
        } else {
                sprintf(query, "SELECT SUM(REQUEST), SUM(SUCCESS), SUM(FAIL), SUM(START) "
                " , SUM(START_TIMEOUT), SUM(START_RETRANS), SUM(INTERIM)  "
                " , SUM(INTERIM_TIMEOUT), SUM(INTERIM_RETRANS), SUM(STOP), SUM(STOP_TIMEOUT), SUM(STOP_RETRANS) "
                " , SUM(TIMEOUT), SUM(NETWORK_ERR) , SUM(ETC_ERR), stat_date "
                " from %s "
                " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' AND sys_ip = '%s' AND sys_no = '%d' GROUP BY stat_date ORDER BY stat_date ",
                table_name, StatisticSystemInfo[i].sysName, stime,etime, sysip, TYPE_AN_AAA
            );
        }

//printf("===23=jean\n%s\n",query);
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s IFTYPE = AN_AAA SYSNO = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,sysip,rxReqMsg->head.para[3].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);
        
            if(strlen(row[15]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[15]+5,14);
            }

            nsucc = atoi(row[1]);
            total = atoi(row[0]);
            if(total == 0){
                succ_rate=0.0;
            }
            else{
                succ_rate = (nsucc/(double)total)*100;
            }

            sprintf(resTmp, "%3s %-14s %15s %15s   %15s   %15s %15s\n"
                ,"", key_time,row[0],row[1],row[2],row[3],row[4]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %15s %15s   %15s   %15s %15s\n"
                ,"", "",row[5],row[6],row[7],row[8],row[9]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %15s %15s   %15s   %15s %15s\n"
                ,"","",row[10],row[11],row[12],row[13],row[14]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %12.2f(%%)\n" ,"","",succ_rate);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    ---------------------------------------------------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);
            
            select_cnt++;
            snd_cnt++;

            for(k=0;k<14;k++){
                tot_row[k]+=atoll(row[k]);
            }
        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s IFTYPE = AN_AAA SYSNO = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,sysip,rxReqMsg->head.para[3].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            nsucc = tot_row[1];
            total = tot_row[0];
            if(total == 0){
                succ_rate=0.0;
            }
            else{
                succ_rate = (nsucc/(double)total)*100;
            }

            sprintf(resTmp, "%3s %-14s %15lld %15lld   %15lld   %15lld %15lld\n"
                ,"", "SUM",tot_row[0],tot_row[1],tot_row[2],tot_row[3],tot_row[4]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %15lld %15lld   %15lld   %15lld %15lld\n"
                ,"", "",tot_row[5],tot_row[6],tot_row[7],tot_row[8],tot_row[9]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %15lld %15lld   %15lld   %15lld %15lld\n"
                ,"","",tot_row[10],tot_row[11],tot_row[12],tot_row[13],tot_row[14]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %12.2f(%%)\n" ,"","",succ_rate);
            strcat(ipafBuf,resTmp);

            sprintf(resTmp, "    ===================================================================================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

///////////////////////////
    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    ===================================================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} //end of stmd_mmc_srch_stat_if_an_aaa //

int stmd_mmc_stat_rad(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     list;
    int     i, cnt, diffTime, seqNo=1;
    short   mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }
 
    //strcpy(onDEMAND[list].svcName, "BSDA");
    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);

        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                //printf("jean========stmd_mmc_stat_rad no sys name\n");
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STM"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal);
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;
    onDEMAND[list].statisticsType = STMD_RADIUS;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;
    
    if ((diffTime >= 0) && (diffTime <= 3600))
    {
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));

        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    COUNT       = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        /*sprintf(resHead,"    STIME    = %s\n    PERIOD   = %d\n    CNT      = %d\n",
            onDEMAND[list].measureTime, onDEMAND[list].period, onDEMAND[list].count);*/
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);
    }
    else 
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s", 
                    get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandRadius(list);
    }
    
    return 1;
}

int stmd_mmc_stat_ttxn(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     list;
    int     i, cnt, diffTime, seqNo=1;
    short   mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }
 
    //strcpy(onDEMAND[list].svcName, "BSDA");
    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);

        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STM"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal);
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;
    onDEMAND[list].statisticsType = STMD_SVC_TTR;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;
    
    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));

        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    COUNT       = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
    }
    else 
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s", 
                    get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandSvcTtr(list);
    }
    
    return 1;
}

int stmd_mmc_stat_txn(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     list;
    int     i, cnt, diffTime, seqNo=1;
    short   mprd, measurecount;
    char    typeName[10];
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }
 
    //strcpy(onDEMAND[list].svcName, "BSDA");
    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                //printf("jean========stmd_mmc_stat_rad no sys name\n");
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "TYPE"))
        {
            if (!strcasecmp(rxReqMsg->head.para[cnt].paraVal,"WAP1")){
                onDEMAND[list].svc_type = 1;
            } 
            else if (!strcasecmp(rxReqMsg->head.para[cnt].paraVal,"WAP2")){
                onDEMAND[list].svc_type = 2;
            }
            else if (!strcasecmp(rxReqMsg->head.para[cnt].paraVal,"HTTP")){
                onDEMAND[list].svc_type = 3;
            }
            else if (!strcasecmp(rxReqMsg->head.para[cnt].paraVal,"JAVA")){
                onDEMAND[list].svc_type = 4;
            }
            else if (!strcasecmp(rxReqMsg->head.para[cnt].paraVal,"VODS")){
                onDEMAND[list].svc_type = 5;
            }
            else if (!strcasecmp(rxReqMsg->head.para[cnt].paraVal,"WIPI")){
                onDEMAND[list].svc_type = 6;
            }  
            else if (!strcasecmp(rxReqMsg->head.para[cnt].paraVal,"VT")){
                onDEMAND[list].svc_type = 7;
            } 
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STIME"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal);
	    }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;
    onDEMAND[list].statisticsType = STMD_SVC_TR;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;

    
    if ((diffTime >= 0) && (diffTime <= 3600))
    {
    	if(diffTime != 0)
        	sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
    	else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));

        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    COUNT       = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
    }
    else 
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s", 
                    get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandSvcTr(list);
    }
    
    return 1;
}
#endif

char* toUpper(char* str)
{
    int  i;

    for (i = 0; i < strlen(str); i++)
        str[i] = toupper(str[i]);
    return str;
}


#if 0
int stmd_mmc_stat_udr (IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     list;
    int     i, cnt, diffTime, seqNo=1;
    short   mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }
 
    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);

        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STM"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal);
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;
    onDEMAND[list].statisticsType = STMD_UDR;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;
    
    if ((diffTime >= 0) && (diffTime <= 3600))
    {
//        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(diffTime-(mprd*60)));
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));
            //sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(STAT_OFFSET_UNIT));

        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    COUNT       = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        /*sprintf(resHead,"    STIME    = %s\n    PERIOD   = %d\n    CNT      = %d\n",
            onDEMAND[list].measureTime, onDEMAND[list].period, onDEMAND[list].count);*/
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
        //stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) +diffTime +10), 0, seqNo++);
    }
    else 
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s", 
                    get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandUdr(list);
    }
    
    return 1;

} /* End of stmd_mmc_stat_udr () */

// SvcType UDR by hela 2007.05.08
int stmd_mmc_stat_type_udr (IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     list;
    int     i, cnt, diffTime, seqNo=1;
    short   mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }
 
    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);

        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STM"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal);
fprintf(stderr, "stime: %s, diffTime: %d\n", rxReqMsg->head.para[cnt].paraVal, diffTime);            
	    }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;
    onDEMAND[list].statisticsType = STMD_TYPE_UDR;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;
    
    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));

        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    COUNT       = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
    }
    else 
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s", 
                    get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandSvcTypeUdr(list);
    }
    
    return 1;

} /* End of stmd_mmc_stat_type_udr () */

int stmd_mmc_srch_stat_udr(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[30]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt,tcnt=0;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_UDR_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_UDR_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_UDR_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_UDR_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-udr");

    sprintf(resTmp, "    ===========================================================================================\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    STATTIME       SVC_OPT         TOTAL_UDR        START_UDR      INTERIM_UDR      INTERIM_URL\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                    STOP_UDR         STOP_URL        TOTAL_URL          PPS_UDR\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                     PPS_URL             GIFT              GET             POST\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                         ETC     RESULT_IINFO   RESULT_SUCCESS  RESULT_REDIRECT\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                              RESULT_CLI_ERR   RESULT_SVR_ERR RESULT_TERM_ABRT  RESULT_SVR_ABRT\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                 IP_UP_BYTES    IP_DOWN_BYTES   TCP_RE_UPBYTES TCP_RE_DWN_BYTES\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                               COMPLETED_TXN       NORMAL_TXN     ABNORMAL_TXN\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    ===========================================================================================\n");
    strcat(ipafHead,resTmp);

    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }
///////////////////////////

        sprintf(query, "SELECT SVC_TYPE, TOT_UDR, START_UDR, INTERIM_UDR, INTERIM_URL, "
            "STOP_UDR, STOP_URL, TOT_URL, PPS_UDR, PPS_URL, "
            "GIFT, GET, POST, ETC, "
            "RES_INFO, RES_SUCC, RES_REDIR, RES_CLI_ERR, RES_SVR_ERR, RES_TERM_ABORT, RES_SVR_ABORT, "
            "IP_UP_BYTES, IP_DOWN_BYTES, TCP_RE_UP_BYTES, TCP_RE_DOWN_BYTES, "
            "COMPLETED_TXN, NORMAL_TXN, ABNORMAL_TXN, stat_date  "
            " from %s "
            " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s'  ORDER BY stat_date ",
            table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {	
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {
            if(snd_cnt>1){

                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 10, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);
        
            if(strlen(row[28]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[28]+5,14);
            }

            sprintf(resTmp, "%3s %14s %-8s %16s %16s %16s %16s\n"
                            "%3s %14s %8s %16s %16s %16s %16s\n"
                            "%3s %14s %8s %16s %16s %16s %16s\n"
                            "%3s %14s %8s %16s %16s %16s %16s\n"
                            "%3s %14s %8s %16s %16s %16s %16s\n"
                            "%3s %14s %8s %16s %16s %16s %16s\n"
                            "%3s %14s %8s %16s %16s %16s\n"
                ,"",key_time,row[0],row[1],row[2],row[3],row[4]
                ,"","","",row[5],row[6],row[7],row[8]
                ,"","","",row[9],row[10],row[11],row[12]
                ,"","","",row[13],row[14],row[15],row[16]
                ,"","","",row[17],row[18],row[19],row[20]
                ,"","","",row[21],row[22],row[23],row[24]
                ,"","","",row[25],row[26],row[27]
            );

            strcat(ipafBuf,resTmp);

            sprintf(resTmp, "    -------------------------------------------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);

            select_cnt++;
            snd_cnt++;

            for(k=0;k<27;k++){
                tot_row[k]+=atoll(row[k+1]);
            }

        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 10, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            sprintf(resTmp, "%3s %-14s %-8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld\n"
                ,"","SUM","SVC_OPT",tot_row[0],tot_row[1],tot_row[2],tot_row[3]
                ,"","","",tot_row[4],tot_row[5],tot_row[6],tot_row[7]
                ,"","","",tot_row[8],tot_row[9],tot_row[10],tot_row[11]
                ,"","","",tot_row[12],tot_row[13],tot_row[14],tot_row[15]
                ,"","","",tot_row[16],tot_row[17],tot_row[18],tot_row[19]
                ,"","","",tot_row[20],tot_row[21],tot_row[22],tot_row[23]
                ,"","","",tot_row[24],tot_row[25],tot_row[26]
            );
            strcat(ipafBuf,resTmp);

            sprintf(resTmp, "    ===========================================================================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

    }

    if (snd_cnt == 1) {
        //sprintf(resTmp, "      %-8s  %10s  %10s  %10s  %10s\n",
        //  "", "0.0", "0.0", "0.0", "0.0");
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    ===========================================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} /* End of stmd_mmc_srch_stat_udr */

int stmd_mmc_srch_stat_type_udr(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[30]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt,tcnt=0;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_SVC_TYPE_UDR_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_SVC_TYPE_UDR_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_SVC_TYPE_UDR_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_SVC_TYPE_UDR_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-udr");

    sprintf(resTmp, "    ===========================================================================================\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    STATTIME       SVC_OPT          SVC_TYPE\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                   TOTAL_UDR      INTERIM_URL         STOP_URL          PPS_URL\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                        GIFT              GET             POST              ETC\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                 RESULT_INFO   RESULT_SUCCESS  RESULT_REDIRECT   RESULT_CLI_ERR\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                              RESULT_SVR_ERR RESULT_TERM_ABRT  RESULT_SVR_ABRT \n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                 IP_UP_BYTES    IP_DOWN_BYTES   TCP_RE_UPBYTES TCP_RE_DWN_BYTES\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                               COMPLETED_TXN       NORMAL_TXN     ABNORMAL_TXN \n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    ===========================================================================================\n");
    strcat(ipafHead,resTmp);

    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }

        sprintf(query, "SELECT SVC_OPT, SVC_TYPE, TOT_UDR, INTERIM_URL, "
                    "STOP_URL, PPS_URL, GIFT, GET, POST, ETC,"
                    "RES_INFO, RES_SUCC, RES_REDIR, RES_CLI_ERR, RES_SVR_ERR, RES_TERM_ABORT, RES_SVR_ABORT, "
                    "IP_UP_BYTES, IP_DOWN_BYTES, TCP_RE_UP_BYTES, TCP_RE_DOWN_BYTES, "
                    "COMPLETED_TXN, NORMAL_TXN, ABNORMAL_TXN, stat_date  "
                    " from %s "
                    " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s'  GROUP BY stat_date, SVC_OPT, SVC_TYPE ORDER BY stat_date, SVC_OPT, SVC_TYPE ",
                    table_name, StatisticSystemInfo[i].sysName, stime,etime
                    );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

	    keepalivelib_increase();

            if(snd_cnt>1){

                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 10, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);
        
            if(strlen(row[24]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[24]+5,14);
            }
             /*******************************************************************************************
             * STATTIME     SVC_OPT          SVC_TYPE
             *                              TOTAL_UDR      INTERIM_URL         STOP_URL          PPS_URL
             *                                   GIFT              GET             POST              ETC
             *                            RESULT_INFO   RESULT_SUCCESS  RESULT_REDIRECT   RESULT_CLI_ERR
             *                         RESULT_SVR_ERR RESULT_TERM_ABRT  RESULT_SVR_ABRT
             *                            IP_UP_BYTES    IP_DOWN_BYTES   TCP_RE_UPBYTES TCP_RE_DWN_BYTES
             *                          COMPLETED_TXN       NORMAL_TXN     ABNORMAL_TXN
             ********************************************************************************************/
            sprintf(resTmp, "%3s %14s %-8s %16s\n"
                            "%3s %14s %8s %16s %16s %16s %16s\n"
                            "%3s %14s %8s %16s %16s %16s %16s\n"
                            "%3s %14s %8s %16s %16s %16s %16s\n"
                            "%3s %14s %8s %16s %16s %16s \n"
                            "%3s %14s %8s %16s %16s %16s %16s\n"
                            "%3s %14s %8s %16s %16s %16s\n"
                        ,"",key_time,row[0],row[1]
                        ,"","","",row[2],row[3],row[4],row[5]
                        ,"","","",row[6],row[7],row[8],row[9]
                        ,"","","",row[10],row[11],row[12],row[13]
                        ,"","","",row[14],row[15],row[16]
                        ,"","","",row[17],row[18],row[19],row[20]
                        ,"","","",row[21],row[22],row[23]
                        );

            strcat(ipafBuf,resTmp);

            sprintf(resTmp, "    -------------------------------------------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);

            select_cnt++;
            snd_cnt++;

            for(k=0;k<22;k++){
                tot_row[k]+=atoll(row[k+2]);
            }
            
            if(strlen(ipafBuf) > 3000){
	             stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 1, snd_cnt);
	             ipafBuf[0] = 0;
	        }

        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 10, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);
             /*******************************************************************************************
             * STATTIME     SVC_OPT          SVC_TYPE
             *                              TOTAL_UDR      INTERIM_URL         STOP_URL          PPS_URL
             *                                   GIFT              GET             POST              ETC
             *                            RESULT_INFO   RESULT_SUCCESS  RESULT_REDIRECT   RESULT_CLI_ERR
             *                         RESULT_SVR_ERR RESULT_TERM_ABRT  RESULT_SVR_ABRT
             *                            IP_UP_BYTES    IP_DOWN_BYTES   TCP_RE_UPBYTES TCP_RE_DWN_BYTES
             *                          COMPLETED_TXN       NORMAL_TXN     ABNORMAL_TXN
             ********************************************************************************************/
            sprintf(resTmp, "%3s %-14s %-8s %16s\n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld \n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld\n"
                        ,"","SUM","SVC_OPT","SVC_TYPE"
                        ,"","","",tot_row[0],tot_row[1],tot_row[2],tot_row[3]
                        ,"","","",tot_row[4],tot_row[5],tot_row[6],tot_row[7]
                        ,"","","",tot_row[8],tot_row[9],tot_row[10],tot_row[11]
                        ,"","","",tot_row[12],tot_row[13],tot_row[14]
                        ,"","","",tot_row[15],tot_row[16],tot_row[17],tot_row[18]
                        ,"","","",tot_row[19],tot_row[20],tot_row[21]
                        );
            strcat(ipafBuf,resTmp);

            sprintf(resTmp, "    ===========================================================================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }
    }
    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    ===========================================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} /* End of stmd_mmc_srch_stat_type_udr */

int stmd_mmc_stat_cdr(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     list;
    int     i, cnt, diffTime, seqNo=1;
    short   mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }
 
    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);

        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STM"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal);
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;
    onDEMAND[list].statisticsType = STMD_CDR;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;
    
    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));

        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    COUNT       = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
    }
    else 
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s", 
                    get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandCdr(list);
    }
    
    return 1;
}

int stmd_mmc_srch_stat_cdr(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[20]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt;
    char *title[]={"TOT","NO_CALL","RETX","ETC_ERR"};
    int row_index;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_CDR_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_CDR_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_CDR_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_CDR_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
//fprintf(stderr, "CDR: startTime: %s\n", stime);
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
//fprintf(stderr, "CDR: endTime: %s\n", etime);
    strcpy(cmdName, "srch-stat-cdr");

    sprintf(resTmp, "    ===========================================================\n");
    strcat(ipafHead, resTmp);
    sprintf(resTmp, "    STATTIME       STATTYPE             UP_PKTS       DOWN_PKTS\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                       UP_BYTES      DOWN_BYTES\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    ===========================================================\n");
    strcat(ipafHead,resTmp);

    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }

///////////////////////////
        sprintf(query, "SELECT "
                        "TOT_UP_PKTS, TOT_DOWN_PKTS, TOT_UP_BYTES, TOT_DOWN_BYTES,  "
                        "NO_CALL_UP_PKTS, NO_CALL_DOWN_PKTS, NO_CALL_UP_BYTES, NO_CALL_DOWN_BYTES,  "
                        "RETX_UP_PKTS, RETX_DOWN_PKTS, RETX_UP_BYTES,  RETX_DOWN_BYTES,  "
                        "ETC_ERR_UP_PKTS, ETC_ERR_DOWN_PKTS, ETC_ERR_UP_BYTES, ETC_ERR_DOWN_BYTES, stat_date "
            " from %s "
            " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' ORDER BY stat_date ",
            table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt > 1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName, rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);
        

            if(strlen(row[16]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[16]+5,14);
            }
            row_index=0;
            for(j=0;j<CDR_TITLE_COUNT;j++){
                if(j==0)
                    strcpy(tmp,key_time);
                else
                    strcpy(tmp,"");
                
                sprintf(resTmp, "%3s %14s %-12s %15s %15s\n","",tmp,title[j],row[row_index],row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
                sprintf(resTmp, "%3s %14s %12s %15s %15s\n","","","",row[row_index],row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
            }
            sprintf(resTmp, "    -----------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);

            select_cnt++;
            snd_cnt++;

            for(k=0;k<16;k++){
                tot_row[k]+=atoll(row[k]);
            }

        }
        mysql_free_result(result);

        if (select_cnt > 0){

            if(snd_cnt > 1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName, rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            row_index=0;
            for(j=0;j<CDR_TITLE_COUNT;j++){
                if(j==0)
                    strcpy(tmp,"SUM");
                else
                    strcpy(tmp,"");
                
                sprintf(resTmp, "%3s %-14s %-12s %15lld %15lld\n","",tmp,title[j],tot_row[row_index],tot_row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
                sprintf(resTmp, "%3s %14s %12s %15lld %15lld\n","","","",tot_row[row_index],tot_row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
            }
            sprintf(resTmp, "    ===========================================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

///////////////////////////
    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
        sprintf(resTmp, "    ===========================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} /* End of stmd_mmc_srch_stat_cdr */

int stmd_mmc_srch_stat_vt(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[30]={0,};
    char  key_time[33];

    int   select_cnt = 0, snd_cnt,tcnt=0;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_VT_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_VT_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[2].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_VT_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_VT_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM
            if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
                strcat(stime, "-"); strcat(stime, resTmp);
        	memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD
        	if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
            	    strcat(stime, "-"); strcat(stime, resTmp);
            	    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH
            	    if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                	strcat(stime, " "); strcat(stime, resTmp);
                	memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM
                	if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                    	    i_tmp = atoi(resTmp);
                    	    i_tmp = i_tmp - (i_tmp%5);
                    	    sprintf(resTmp, "%02d", i_tmp);
                            strcat(stime, ":"); strcat(stime, resTmp);
                	}
            	    }
        	}
    	    }
    }

    if(strlen(rxReqMsg->head.para[4].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[4].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }

    strcpy(cmdName, "srch-stat-svc");
    
    sprintf(resTmp, "    =====================================================================================================\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    MSG_TYPE      VTANA\n");
    strcat(ipafHead,resTmp); 
    sprintf(resTmp, "    =====================================================================================================\n");
    strcat(ipafHead,resTmp); 
    sprintf(resTmp, "    STATTIME                        TOT_CALL_CNT               UDR_CNT_BY_BYE       UDR_CNT_BY_CALL_STOP\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                              UDR_CNT_BY_TIMEOUT   UDR_CNT_BY_CALL_ID_CHANGED             WITH_OTHER_NET\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                 BYTE_METHOD_CNT            INVITE_METHOD_CNT          NOTIFY_METHOD_CNT\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                              MESSAGE_METHOD_CNT           PUBLISH_METHOD_CNT        REGISTER_METHOD_CNT\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                            SUBSCRIBE_METHOD_CNT            UPDATE_METHOD_CNT             ETC_METHOD_CNT\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                  TOT_AUDIO_PKTS              TOT_AUDIO_BYTES             TOT_VIDEO_PKTS\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                 TOT_VIDEO_BYTES\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    =====================================================================================================\n");
    strcat(ipafHead,resTmp);
    

    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }

        sprintf(query, "SELECT ucSvcType, TOT_CALL_CNT, UDR_CNT_BY_BYE, UDR_CNT_BY_CALL_STOP, "
                       "UDR_CNT_BY_TIMEOUT, UDR_CNT_BY_CALL_ID_CHANGED, WITH_OTHER_NET, BYTE_METHOD_CNT, "
                       "INVITE_METHOD_CNT, NOTIFY_METHOD_CNT, MESSAGE_METHOD_CNT, PUBLISH_METHOD_CNT, "
                       "REGISTER_METHOD_CNT, SUBSCRIBE_METHOD_CNT, UPDATE_METHOD_CNT, ETC_METHOD_CNT, "
                       "TOT_AUDIO_PKTS, TOT_AUDIO_BYTES, TOT_VIDEO_PKTS, TOT_VIDEO_BYTES, stat_date "
                       "from %s "
                       "where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s'  ORDER BY stat_date, ucSvcType ",
                       table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }
        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {
            if(snd_cnt>1){

                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 10, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);
        
            if(strlen(row[20]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[20]+5,14);
            }

            sprintf(resTmp, "%3s %14s %8s %21s %28s %26s\n"
                            "%3s %14s %8s %21s %28s %26s\n"
                            "%3s %14s %8s %21s %28s %26s\n"
                            "%3s %14s %8s %21s %28s %26s\n"
                            "%3s %14s %8s %21s %28s %26s\n"
                            "%3s %14s %8s %21s %28s %26s\n"
                            "%3s %14s %8s %21s\n"
                            ,"",key_time,"",row[1],row[2],row[3]
                            ,"","","",row[4],row[5],row[6]
                            ,"","","",row[7],row[8],row[9]
                            ,"","","",row[10],row[11],row[12]
                            ,"","","",row[13],row[14],row[15]
                            ,"","","",row[16],row[17],row[18]
                            ,"","","",row[19]
            );

            strcat(ipafBuf,resTmp);

            sprintf(resTmp, "    -----------------------------------------------------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);

            select_cnt++;
            snd_cnt++;

            for(k=0;k<19;k++){
                tot_row[k]+=atoll(row[k+1]);
            }

        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 10, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            sprintf(resTmp, "%3s %-14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            "%3s %14s %8s %16lld %16lld %16lld %16lld\n"
                            ,"","SUM","",tot_row[0],tot_row[1],tot_row[2],tot_row[3]
                            ,"","","",tot_row[4],tot_row[5],tot_row[6],tot_row[7]
                            ,"","","",tot_row[8],tot_row[9],tot_row[10],tot_row[11]
                            ,"","","",tot_row[12],tot_row[13],tot_row[14],tot_row[15]
                            ,"","","",tot_row[16],tot_row[17],tot_row[18],tot_row[19]
            );
            
            sprintf(resTmp, "%3s %-14s %8s %21lld %28lld %26lld\n"
                            "%3s %14s %8s %21lld %28lld %26lld\n"
                            "%3s %14s %8s %21lld %28lld %26lld\n"
                            "%3s %14s %8s %21lld %28lld %26lld\n"
                            "%3s %14s %8s %21lld %28lld %26lld\n"
                            "%3s %14s %8s %21lld %28lld %26lld\n"
                            "%3s %14s %8s %21lld\n"
                            ,"","SUM","",tot_row[0],tot_row[1],tot_row[2]
                            ,"","","",tot_row[3],tot_row[4],tot_row[5]
                            ,"","","",tot_row[6],tot_row[7],tot_row[8]
                            ,"","","",tot_row[9],tot_row[10],tot_row[11]
                            ,"","","",tot_row[12],tot_row[13],tot_row[14]
                            ,"","","",tot_row[15],tot_row[16],tot_row[17]
                            ,"","","",tot_row[18]
            );
            strcat(ipafBuf,resTmp);

            sprintf(resTmp, "    =====================================================================================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    =====================================================================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} /* End of stmd_mmc_srch_stat_vt */

int stmd_mmc_stat_cdr2(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     list;
    int     i, cnt, diffTime, seqNo=1;
    short   mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }
 
    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);

        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STM"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal);
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.  
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;
    onDEMAND[list].statisticsType = STMD_CDR2;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;
    
    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));

        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    COUNT       = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count); 
        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
    }
    else 
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s", 
                    get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandCdr2(list);
    }
    
    return 1;
} /* End of stmd_mmc_stat_cdr2*/

int stmd_mmc_srch_stat_cdr2(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
   
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[20]={0,};
    char  key_time[33];

    int         select_cnt = 0, snd_cnt;
    char *title[]={"TOT","NO_CALL","RETX","ETC_ERR"};
    int row_index;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[1].paraVal); i++){
        rxReqMsg->head.para[1].paraVal[i] = toupper(rxReqMsg->head.para[1].paraVal[i]);
    }
    
    if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_CDR2_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_CDR2_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[1].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_CDR2_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_CDR2_TBL_NAME);
    }

    if(strlen(rxReqMsg->head.para[2].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[2].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[2].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[3].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[3].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+4, 2);//MM 
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+6, 2);//DD 
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+8, 2);//HH 
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[3].paraVal+10, 2);//MM 
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp); 
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-cdr2");

    sprintf(resTmp, "    ===========================================================\n");
    strcat(ipafHead, resTmp);
    sprintf(resTmp, "    STATTIME       STATTYPE             UP_PKTS       DOWN_PKTS\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                                       UP_BYTES      DOWN_BYTES\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    ===========================================================\n");
    strcat(ipafHead,resTmp);

    snd_cnt=1;
    for(i=0; i<sysCnt; i++){ 
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }

///////////////////////////
        sprintf(query, "SELECT "
                        "TOT_UP_PKTS, TOT_DOWN_PKTS, TOT_UP_BYTES, TOT_DOWN_BYTES,  "
                        "NO_CALL_UP_PKTS, NO_CALL_DOWN_PKTS, NO_CALL_UP_BYTES, NO_CALL_DOWN_BYTES,  "
                        "RETX_UP_PKTS, RETX_DOWN_PKTS, RETX_UP_BYTES,  RETX_DOWN_BYTES,  "
                        "ETC_ERR_UP_PKTS, ETC_ERR_DOWN_PKTS, ETC_ERR_UP_BYTES, ETC_ERR_DOWN_BYTES, stat_date "
            " from %s "
            " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' ORDER BY stat_date ",
            table_name, StatisticSystemInfo[i].sysName, stime,etime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt > 1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName, rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);
        

            if(strlen(row[16]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[16]+5,14);
            }
            row_index=0;
            for(j=0;j<CDR2_TITLE_COUNT;j++){
                if(j==0)
                    strcpy(tmp,key_time);
                else
                    strcpy(tmp,"");
                
                sprintf(resTmp, "%3s %14s %-12s %15s %15s\n","",tmp,title[j],row[row_index],row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
                sprintf(resTmp, "%3s %14s %12s %15s %15s\n","","","",row[row_index],row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
            }
            sprintf(resTmp, "    -----------------------------------------------------------\n");
            strcat(ipafBuf,resTmp);

            select_cnt++;
            snd_cnt++;

            for(k=0;k<16;k++){
                tot_row[k]+=atoll(row[k]);
            }

        }
        mysql_free_result(result);

        if (select_cnt > 0){

            if(snd_cnt > 1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName, rxReqMsg->head.para[1].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            row_index=0;
            for(j=0;j<CDR2_TITLE_COUNT;j++){
                if(j==0)
                    strcpy(tmp,"SUM");
                else
                    strcpy(tmp,"");
                
                sprintf(resTmp, "%3s %-14s %-12s %15lld %15lld\n","",tmp,title[j],tot_row[row_index],tot_row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
                sprintf(resTmp, "%3s %14s %12s %15lld %15lld\n","","","",tot_row[row_index],tot_row[row_index+1]);
                strcat(ipafBuf,resTmp);
                row_index +=2;
            }
            sprintf(resTmp, "    ===========================================================\n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
        sprintf(resTmp, "    ===========================================================\n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} /* End of stmd_mmc_srch_stat_cdr */

int stmd_mmc_stat_fail_udr(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    int     list;
    int     i, cnt, diffTime, seqNo=1;
    short   mprd, measurecount;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((list = checkFreeList (ONDEMANDJOB)) < 0) {
        sprintf (resBuf,"    RESULT = FAIL\n    FAIL REASON = ONDEMAND STAT TBL FULL ERR\n");
        stmd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo++);
        return;
    }

    // Default setting
    diffTime = 0;
    mprd = 5;
    measurecount = 1;

    // Parameter setting
    for (cnt=0; cnt<rxReqMsg->head.paraCnt; cnt++)
    {
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraName); i++)
            rxReqMsg->head.para[cnt].paraName[i] = toupper(rxReqMsg->head.para[cnt].paraName[i]);
        for (i=0; i<strlen(rxReqMsg->head.para[cnt].paraVal); i++)
            rxReqMsg->head.para[cnt].paraVal[i] = toupper(rxReqMsg->head.para[cnt].paraVal[i]);

        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "SYS"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                strcpy(onDEMAND[list].svcName , rxReqMsg->head.para[cnt].paraVal );
            } else {
                strcpy(onDEMAND[list].svcName , "ALL" );
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "IP"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                    strcpy(onDEMAND[list].ipAddr,rxReqMsg->head.para[cnt].paraVal);
            }
            else{
                //printf("jean you must input para IP\n");
                return;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "STM"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,"")){
                diffTime = get_diff_time(rxReqMsg->head.para[cnt].paraVal);
            }
        }
        if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "PRD"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
            {
                mprd = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
                // 5분의 배수가 아닐 경우는 값을 내림해서 수집주기를 만든다.
                mprd = ( mprd / STAT_UNIT) * STAT_UNIT;
            }
        }
        else if (!strcasecmp(rxReqMsg->head.para[cnt].paraName, "CNT"))
        {
            if (strcasecmp(rxReqMsg->head.para[cnt].paraVal,""))
                measurecount = strtol(rxReqMsg->head.para[cnt].paraVal, 0, 10);
        }
    }

    onDEMAND[list].period = mprd;
    //onDEMAND[list].statisticsType = STMD_AAA;
    onDEMAND[list].count = measurecount;
    onDEMAND[list].Txcount = 0;
    onDEMAND[list].cmdId = rxReqMsg->head.mmcdJobNo;
    //strcpy(onDEMAND[list].svcName, "BSDA");

    if ((diffTime >= 0) && (diffTime <= 3600))
    {
        if(diffTime != 0)
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(diffTime));
        else
            sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time2(mprd*60));

        sprintf(resHead,"    MEASURETIME = %s  -  ",get_ondemand_time2(STAT_OFFSET_UNIT));
        strcpy (resBuf, resHead);
        sprintf(resHead,"%s\n    PERIOD      = %d\n    COUNT       = %d\n",
            get_ondemand_time2(STAT_OFFSET_UNIT+(mprd*60*measurecount)), onDEMAND[list].period, onDEMAND[list].count);

        strcat (resBuf, resHead);
        stmd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, ((mprd*60) + STAT_OFFSET_UNIT + diffTime +30), 0, seqNo++);
    }
    else
    {
        onDEMAND[list].count = -1;
        onDEMAND[list].period = 5;
        sprintf((char *)onDEMAND[list].measureTime, "%s",
                    get_ondemand_time(0-onDEMAND[list].period*60));
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(0));
        doOnDemandFailUdr(list);
    }
    return 1;
} /* End of stmd_mmc_stat_fail_udr */

int stmd_mmc_srch_stat_fail_udr(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;

    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j,k;
    char  sysTypeName[16];

    char  table_name[30];
    char  stime[33]; char etime[33];
    int   i_tmp;
    char  *env, fname[256], tmp[64];
    int   row_count=0;
    long long   tot_row[22]={0,};
    char  key_time[33];
    char  sysip[30]={0,};

    int         select_cnt = 0, snd_cnt;

    long long total,nsucc, nfail;
    double succ_rate;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    memset(tot_row,0,sizeof(tot_row));
    memset (ipafBuf, 0x00, sizeof(ipafBuf));
    memset (ipafHead, 0x00, sizeof(ipafHead));

    for (i=0; i<strlen(rxReqMsg->head.para[0].paraVal); i++){
        rxReqMsg->head.para[0].paraVal[i] = toupper(rxReqMsg->head.para[0].paraVal[i]);
    }
    for (i=0; i<strlen(rxReqMsg->head.para[3].paraVal); i++){
        rxReqMsg->head.para[3].paraVal[i] = toupper(rxReqMsg->head.para[3].paraVal[i]);
    }


    if( !strcasecmp(rxReqMsg->head.para[3].paraVal,"5MIN") )
    {
        strcpy(table_name,STM_STATISTIC_5MINUTE_FAIL_UDR_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[3].paraVal,"HOUR") )
    {
        strcpy(table_name,STM_STATISTIC_HOUR_FAIL_UDR_TBL_NAME);
    }
    else if( !strcasecmp(rxReqMsg->head.para[3].paraVal,"DAY") )
    {
        strcpy(table_name,STM_STATISTIC_DAY_FAIL_UDR_TBL_NAME);
    }
    else
    {
        strcpy(table_name,STM_STATISTIC_MONTH_FAIL_UDR_TBL_NAME);
    }

    strcpy(sysip,rxReqMsg->head.para[2].paraVal);

    if(strlen(rxReqMsg->head.para[4].paraVal) == 12){
        memset(stime, 0, 33);
        memcpy(stime, rxReqMsg->head.para[4].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+4, 2);//MM
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(stime, "-"); strcat(stime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+6, 2);//DD
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(stime, "-"); strcat(stime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+8, 2);//HH
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(stime, " "); strcat(stime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[4].paraVal+10, 2);//MM
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp);
                        strcat(stime, ":"); strcat(stime, resTmp);
                    }
                }
            }
        }
    }
    if(strlen(rxReqMsg->head.para[5].paraVal) == 12){
        memset(etime, 0, 33);
        memcpy(etime, rxReqMsg->head.para[5].paraVal, 4); //YYYY
        memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+4, 2);//MM
        if(atoi(resTmp) > 0 && atoi(resTmp) < 13){
            strcat(etime, "-"); strcat(etime, resTmp);
            memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+6, 2);//DD
            if(atoi(resTmp) > 0 && atoi(resTmp) < 32){
                strcat(etime, "-"); strcat(etime, resTmp);
                memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+8, 2);//HH
                if(atoi(resTmp) >= 0 && atoi(resTmp) < 24){
                    strcat(etime, " "); strcat(etime, resTmp);
                    memset(resTmp, 0, 12);  memcpy(resTmp, rxReqMsg->head.para[5].paraVal+10, 2);//MM
                    if(atoi(resTmp) >=0 && atoi(resTmp) < 60){
                        i_tmp = atoi(resTmp);
                        i_tmp = i_tmp - (i_tmp%5);
                        sprintf(resTmp, "%02d", i_tmp);
                        strcat(etime, ":"); strcat(etime, resTmp);
                    }
                }
            }
        }
    }
    strcpy(cmdName, "srch-stat-if");

    sprintf(resTmp, "    ===================================================================================================\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    STATTIME               REQUEST         SUCCESS              FAIL             START   START_TIMEOUT\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                     START_RETRANS         INTERIM   INTERIM_TIMEOUT   INTERIM_RETRANS            STOP\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                      STOP_TIMEOUT    STOP_RETRANS           TIMEOUT       NETWORK_ERR         ETC_ERR\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "                      SUCC_RATE(%%)\n");
    strcat(ipafHead,resTmp);
    sprintf(resTmp, "    ===================================================================================================\n");
    strcat(ipafHead,resTmp);

    snd_cnt=1;
    for(i=0; i<sysCnt; i++){
        if(!strcasecmp(StatisticSystemInfo[i].sysType, "OMP"))
            continue;
        if ( strcasecmp(rxReqMsg->head.para[0].paraVal, "ALL") ) {
            if(strcasecmp(rxReqMsg->head.para[0].paraVal, StatisticSystemInfo[i].sysName))
                continue;
        }

///////////////////////////

        if( !(strcasecmp(sysip,"0.0.0.0"))) {
                sprintf(query, "SELECT SUM(REQUEST), SUM(SUCCESS), SUM(FAIL), SUM(START) "
                " , SUM(START_TIMEOUT), SUM(START_RETRANS), SUM(INTERIM)  "
                " , SUM(INTERIM_TIMEOUT), SUM(INTERIM_RETRANS), SUM(STOP), SUM(STOP_TIMEOUT), SUM(STOP_RETRANS) "
                " , SUM(TIMEOUT), SUM(NETWORK_ERR) , SUM(ETC_ERR), stat_date "
                " from %s "
                " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' GROUP BY stat_date  ORDER BY stat_date ",
                table_name, StatisticSystemInfo[i].sysName, stime,etime
            );
        } else {
                sprintf(query, "SELECT SUM(REQUEST), SUM(SUCCESS), SUM(FAIL), SUM(START) "
                " , SUM(START_TIMEOUT), SUM(START_RETRANS), SUM(INTERIM)  "
                " , SUM(INTERIM_TIMEOUT), SUM(INTERIM_RETRANS), SUM(STOP), SUM(STOP_TIMEOUT), SUM(STOP_RETRANS) "
                " , SUM(TIMEOUT), SUM(NETWORK_ERR) , SUM(ETC_ERR), stat_date "
                " from %s "
                " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' AND sys_ip = '%s' AND sys_no = '%s' GROUP BY stat_date ORDER BY stat_date ",
                table_name, StatisticSystemInfo[i].sysName, stime,etime, sysip, "0"
            );
        }
    
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);

        select_cnt=0;
        memset(tot_row,0,sizeof(tot_row));
        while((row = mysql_fetch_row(result)) != NULL)  {

            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s TYPE = FAIL_UDR SYSNO = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,sysip,rxReqMsg->head.para[3].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            if(strlen(row[15]) == 19){
                memset(key_time,0,sizeof(key_time));
                strncpy(key_time,row[15]+5,14);
            }

            nsucc = atoi(row[1]);
            total = atoi(row[0]);
            if(total == 0){
                succ_rate=0.0;
            }
            else{
                succ_rate = (nsucc/(double)total)*100;
            }

            sprintf(resTmp, "%3s %-14s %15s %15s   %15s   %15s %15s\n"
                ,"", key_time,row[0],row[1],row[2],row[3],row[4]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %15s %15s   %15s   %15s %15s\n"
                ,"", "",row[5],row[6],row[7],row[8],row[9]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %15s %15s   %15s   %15s %15s\n"
                ,"","",row[10],row[11],row[12],row[13],row[14]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %12.2f(%%)\n" ,"","",succ_rate);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "    ---------------------------------------------------------------------------------------------------\
n");
            strcat(ipafBuf,resTmp);

            select_cnt++;
            snd_cnt++;

            for(k=0;k<14;k++){
                tot_row[k]+=atoll(row[k]);
            }
        }
        mysql_free_result(result);

        if (select_cnt > 0){
            if(snd_cnt>1){
                stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 1, 300, 1, snd_cnt);
                memset (ipafBuf, 0x00, sizeof(ipafBuf));
            }
            sprintf(ipafBuf, "    SYSTYPE = %s TYPE = FAIL_UDR SYSNO = %s PERIOD = %s\n    MEASURETIME = %s  -  %s\n",
                StatisticSystemInfo[i].sysName,sysip,rxReqMsg->head.para[3].paraVal,stime,etime);
            strcat(ipafBuf, ipafHead);

            nsucc = tot_row[1];
            total = tot_row[0];
            if(total == 0){
                succ_rate=0.0;
            }
            else{
                succ_rate = (nsucc/(double)total)*100;
            }

            sprintf(resTmp, "%3s %-14s %15lld %15lld   %15lld   %15lld %15lld\n"
                ,"", "SUM",tot_row[0],tot_row[1],tot_row[2],tot_row[3],tot_row[4]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %15lld %15lld   %15lld   %15lld %15lld\n"
                ,"", "",tot_row[5],tot_row[6],tot_row[7],tot_row[8],tot_row[9]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %15lld %15lld   %15lld   %15lld %15lld\n"
                ,"","",tot_row[10],tot_row[11],tot_row[12],tot_row[13],tot_row[14]);
            strcat(ipafBuf,resTmp);
            sprintf(resTmp, "%3s %-14s %12.2f(%%)\n" ,"","",succ_rate);
            strcat(ipafBuf,resTmp);

            sprintf(resTmp, "    ===================================================================================================\
n");
            strcat(ipafBuf,resTmp);

            snd_cnt++;
        }

    }

    if (snd_cnt == 1) {
        sprintf(resTmp, "    NO DATA\n");
        strcat(ipafBuf, resTmp);
            sprintf(resTmp, "    ===================================================================================================\
n");
        strcat(ipafBuf,resTmp);
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, -1, 0, 0, 0, snd_cnt);
    } else{
        stmd_txMMLResult (rxIxpcMsg, ipafBuf, 0, 0, 0, 0, snd_cnt);
    }

    return 1;

} // end of stmd_mmc_srch_stat_fail_udr //
#endif // by jjinri 2009.04.18
