#include "ixpc_proto.h"

extern IXPC_MsgQRoutTable	msgQRoutTbl[SYSCONF_MAX_APPL_NUM];
extern IXPC_SockRoutTable	sockRoutTbl[SYSCONF_MAX_ASSO_SYS_NUM];
extern int					ixpcQid, ixpcPortNum, trcLogId, trcErrLogId, trcLogFlag;
extern char					mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern time_t				currentTime, lastPingTestTime;
extern char					trcBuf[4096], trcTmp[1024];
extern IxpcConSts			*ixpcCON;

/** watch dog 07.17  */
extern SFM_SysCommMsgType   *loc_sadb;
extern char sysLabel[COMM_MAX_NAME_LEN];

//char    ver[8] = "R2.2.3";
char    ver[8] = "R1.0.0";

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
int ixpc_initial (void)
{
	char	*env,tmp[64],fname[256];
	int		key,i,id;

	if ((env = getenv(MY_SYS_NAME)) == NULL) {
		fprintf(stderr,"[ixpc_init] not found %s environment name\n", MY_SYS_NAME);
		return -1;
	}
	strcpy (mySysName, env);
	strcpy (myAppName, "IXPC");
	currentTime = lastPingTestTime = time(0);
	commlib_setupSignals(NULL);

	memset ((void*)msgQRoutTbl, 0, sizeof(msgQRoutTbl));
	memset ((void*)sockRoutTbl, 0, sizeof(sockRoutTbl));

	//
	if (keepalivelib_init (myAppName) < 0)
		return -1;

	//socklib_initial();

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[ixpc_init] not found %s environment name\n", IV_HOME);
		return -1;
	}
	sprintf(fname,"%s/%s", env, SYSCONF_FILE);

	if(set_proc_version(OMP_VER_INX_IXPC, ver) < 0){
		fprintf(stderr, "[ixpc_initial] setting process version failed\n");
		return -1;
	}

	/* 시스템 내부 application들의 이름과 queue key를 읽어 msgQRoutTbl에 setting한다.
	*/
	if (ixpc_initMsgQRoutTbl(fname) < 0)
		return -1;

	/* tcp로 접속할 remote 시스템들의 이름과 ip address를 읽어 sockRoutTbl에 setting한다.
	*/
	if (ixpc_initSockRoutTbl(fname) < 0)
		return -1;

	/* config file에서 자신의 message queue key를 읽어, attach queue
	*/
	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", myAppName, 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((ixpcQid = msgget(key,IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[ixpc_init] msgget fail; key=0x%x,err=%d(%s)\n",key,errno,strerror(errno));
		return -1;
	}

	/* config file에서 자신의 bind port number를 읽어 binding한다.
	*/
	if (conflib_getNthTokenInFileSection (fname, "[SOCKET_PORT]", "IXPC", 1, tmp) < 0)
		return -1;
	ixpcPortNum = strtol(tmp,0,0);
	if (socklib_initTcpBind(ixpcPortNum) < 0)
		return -1;

	/* IXPC connection 상태 정보를 위한 shared memory 초기화 */
	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_IXPC_CON", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	if ((id = (int)shmget (key, sizeof(IxpcConSts), IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"IXPC con shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno) );
		return -1;
	}
	if ((ixpcCON = (IxpcConSts*) shmat (id,0,0)) == (IxpcConSts*)-1) {
		fprintf(stderr,"IXPC con shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno) );
		return -1;
	}
	for ( i=0; i<SYSCONF_MAX_ASSO_SYS_NUM;i++ ){
		if ( sockRoutTbl[i].sysName[0] == NULL ) {
			ixpcCON->ixpcCon[i].name[0] = NULL;
			continue;
		}
		strcpy ( ixpcCON->ixpcCon[i].name, sockRoutTbl[i].sysName );
		ixpcCON->ixpcCon[i].connSts = SFM_LAN_DISCONNECTED;
	}



	/* sockRoutTbl에 setting된 주소로 remote 시스템의 ixpc에 connect한다.
	*/
	ixpc_checkConnections ();

	/* watch-dog ￠￢¨­¨oAAo￠￢| ¡¾￠￢¨u¨￢CO ￠O¡× ¡ic￠?eCO SYSTEM_LABEL AI￠￢¡× ¡¾￠￢CN￠￥U */
	if (conflib_getNthTokenInFileSection(fname, "GENERAL", "SYSTEM_LABEL", 1, sysLabel) < 0)
		return -1;

	if(conflib_getNthTokenInFileSection(fname, "SHARED_MEMORY_KEY", "SHM_LOC_SADB", 1, tmp) < 0)
		return -1;

	key = strtol(tmp, 0, 0);

	if( (id = (int)shmget(key, sizeof(SFM_SysCommMsgType), 0666 | IPC_CREAT)) < 0)
	{
		if(errno != ENOENT)
		{
			fprintf(stderr, "[%s:%s:%d] shmget fail; key=0x%x, err=%d(%s)\n", __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
			return -1;
		}
	}

	if( (loc_sadb = (SFM_SysCommMsgType*)shmat(id, 0, 0)) == (SFM_SysCommMsgType*)-1)
	{   
		fprintf(stderr, "[%s:%s:%d] shmat fail; key=0x%x, err=%d(%s)\n", __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
		return -1;
	}


	/* log file들을 open한다.
	*/
	if (ixpc_initLog () < 0)
		return -1;

	logPrint (trcLogId,FL,"%s startup...\n", myAppName);
	logPrint (trcErrLogId,FL,"%s startup...\n", myAppName);
	return 1;

} /** End of ixpc_initial **/



/*------------------------------------------------------------------------------
* config file에서 등록된 시스템 내부 application들의 정보를 읽어 이름과 queue key를
*	msgQRoutTbl에 setting한다.
* - [APPLICATIONS] section에 최대 SYSCONF_MAX_APPL_NUM개까지 등록될 수 있다.
------------------------------------------------------------------------------*/
int ixpc_initMsgQRoutTbl (char *fname)
{
	char	getBuf[256],token[8][64];
	int		appCnt=0,lNum;
	FILE	*fp;

	memset ((void*)msgQRoutTbl, 0, sizeof(msgQRoutTbl));

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[ixpc_initMsgQRoutTbl] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* [APPLICATIONS] section으로 이동
	*/
	if ((lNum = conflib_seekSection(fp,"[APPLICATIONS]")) < 0) {
		fprintf(stderr,"[ixpc_initMsgQRoutTbl] not found section[APPLICATIONS] in file[%s]\n", fname);
		return -1;
	}

	/* 등록된 application들의 이름과 Qkey를 저장한다.
	*/
	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(appCnt < SYSCONF_MAX_APPL_NUM) ) {
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		/* config 파일에 "name = Qkey 실행파일위치" format으로 들어있다.
		*/
		if (sscanf(getBuf,"%s%s%s%s",token[0],token[1],token[2],token[3]) < 4) {
			fprintf(stderr,"[ixpc_initMsgQRoutTbl] syntax error; file=%s, lNum=%d\n", fname, lNum);
			return -1;
		}
		strcpy (msgQRoutTbl[appCnt].appName, token[0]);
		msgQRoutTbl[appCnt].msgQkey = strtol(token[2],0,0);
		msgQRoutTbl[appCnt++].pres = 1;
	}

	fclose(fp);
	return 1;

} /** End of ixpc_initMsgQRoutTbl **/



/*------------------------------------------------------------------------------
* config file에서 tcp로 접속할 remote 시스템들의 ip address를 읽어 sockRoutTbl에
*	setting한다.
* - [ASSOCIATE_SYSTEMS] section에 최대 SYSCONF_MAX_ASSO_SYS_NUM개까지 등록될 수 있다.
* - config file에 ip address를 지정하지 않는 경우 반드시 "NULL"로 기록되어야 한다.
------------------------------------------------------------------------------*/
int ixpc_initSockRoutTbl (char *fname)
{
	char	getBuf[256],token[8][64];
	int		sysCnt=0,lNum;
	FILE	*fp;

	memset ((void*)sockRoutTbl, 0, sizeof(sockRoutTbl));

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[ixpc_initSockRoutTbl] fopen fail[%s]\n", fname);
		return -1;
	}

	/* [ASSOCIATE_SYSTEMS] section으로 이동
	*/
	if ((lNum = conflib_seekSection(fp,"ASSOCIATE_SYSTEMS")) < 0) {
		fprintf(stderr,"[ixpc_initSockRoutTbl] not found section[ASSOCIATE_SYSTEMS] in file[%s]\n", fname);
		return -1;
	}

	/* 등록된 system들의 이름과 ip address를 저장한다.
	*/
	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(sysCnt < SYSCONF_MAX_ASSO_SYS_NUM) ) {
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;
		if (!strncmp(mySysName,getBuf,strlen(mySysName))) /* 자신의 정보는 등록하지 않는다.  */
			continue;

		/* config 파일에 "name = type group primary_addr seconday_addr" format으로 들어있다.
		*/
		if (sscanf(getBuf,"%s%s%s%s%s%s",token[0],token[1],token[2],token[3],token[4],token[5]) < 6) {
			fprintf(stderr,"[ixpc_initSockRoutTbl] syntax error; file=%s, lNum=%d\n", fname, lNum);
			return -1;
		}
		strcpy (sockRoutTbl[sysCnt].sysName, token[0]);
		strcpy (sockRoutTbl[sysCnt].sysType, token[2]);
		strcpy (sockRoutTbl[sysCnt].sysGroup, token[3]);
		strcpy (sockRoutTbl[sysCnt].ipAddrPri, token[4]);
		if (strcasecmp(token[5],"NULL"))
			strcpy (sockRoutTbl[sysCnt].ipAddrSec, token[5]);
		sockRoutTbl[sysCnt].disconnTime = currentTime;
		sockRoutTbl[sysCnt].lastTxTime = currentTime;
		sockRoutTbl[sysCnt].lastRxTime = currentTime;
		sockRoutTbl[sysCnt++].pres = 1;
	}
	fclose(fp);

	return 1;

} /** End of ixpc_initSockRoutTbl **/



/*------------------------------------------------------------------------------
* error 메시지를 남길 error log file과 trace 메시지를 남길 log file을 open한다.
------------------------------------------------------------------------------*/
int ixpc_initLog (void)
{
	char	*env,fname[256];

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[ixpc_initLog] not found %s environment name\n", IV_HOME);
		return -1;
	}
	sprintf(fname,"%s/%s.%s", env, IXPC_LOG_FILE, mySysName);

	if ((trcLogId = loglib_openLog (fname,
			LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
			LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
		fprintf(stderr,"[ixpc_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	sprintf(fname,"%s/%s.%s", env, IXPC_ERRLOG_FILE, mySysName);
	if ((trcErrLogId = loglib_openLog (fname,
			LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
			LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
		fprintf(stderr,"[ixpc_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	return 1;

} /** End of ixpc_initLog **/


