#include "ixpc_proto.h"
#include "ipaf_names.h"

extern IXPC_MsgQRoutTable	msgQRoutTbl[SYSCONF_MAX_APPL_NUM];
extern IXPC_SockRoutTable	sockRoutTbl[SYSCONF_MAX_ASSO_SYS_NUM];
extern int					ixpcQid, ixpcPortNum, trcLogId, trcErrLogId, trcLogFlag;
extern char					mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern time_t				currentTime, lastPingTestTime;
extern char					trcBuf[4096], trcTmp[1024];
extern IxpcConSts			*ixpcCON;
extern SFM_SysCommMsgType   *loc_sadb;

extern char sysLabel[COMM_MAX_NAME_LEN];
extern BindSockFdContext   bindSockFdTbl[SOCKLIB_MAX_BIND_CNT];
extern ServerSockFdContext serverSockFdTbl[SOCKLIB_MAX_SERVER_CNT];
extern ClientSockFdContext clientSockFdTbl[SOCKLIB_MAX_CLIENT_CNT];
char vERSION[8] = "R2.0.0"; //BEFORE : R1.0.1 -> R2.0.0 (2011-05-06)

void FinishProgram()
{
    GeneralQMsgType rxGenQMsg;

	sprintf (trcBuf,">>> FinishProgram Start\n"); trclib_writeLogErr (FL,trcBuf);

    // �wait 
	commlib_microSleep(1000000);

    //signal�� ���� �� �� ��� ��� �� �ѹ� �� msgq�� Ȯ���� �� ����
    if(msgrcv(ixpcQid, &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT) > 0) {
        ixpc_exeRxQMsg (&rxGenQMsg);
    }

	bind_socket_release();

    if( !g_dStopFlag ){
        sprintf (trcBuf,">>> DON'T WANT PROGRAM FINISHED, cause=%d\n",g_dStopFlag);
    }else{
        sprintf (trcBuf," >>> PROGRAM IS NORMALLY TERMINATED, cause=%d\n", g_dStopFlag);
    }
    trclib_writeLogErr (FL,trcBuf);

    exit(0);
}

void bind_socket_release()
{
	int i;
	// bind socket release
	for (i=0; i<SOCKLIB_MAX_BIND_CNT; i++) {
		if (bindSockFdTbl[i].fd != 0) {
			close(bindSockFdTbl[i].fd);
		}
	} 

	for (i=0; i<SOCKLIB_MAX_SERVER_CNT; i++){
		if (serverSockFdTbl[i].fd !=0) {
			close(serverSockFdTbl[i].fd);

		};

	}

	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++){
		if (clientSockFdTbl[i].fd !=0) {
			close(clientSockFdTbl[i].fd);

		};
	}

}

void UserControlledSignal(int sign)
{
	g_dStopFlag = 1;                                                                                                                                                               
	logPrint (trcLogId,FL, "[%s] RECEIVED TERMINATED SIGNAL, sig=%d, when=%ld\n"                                                                                               
			,__FUNCTION__, sign, time(0));                                                                                                                                         
 
}/*End of UserControlledSignal()*/
                                                                                                                                                                                  
                                                                                                                                                                                   
/*******************************************************************************                                                                                                   
                                                                                                                                                                                   
*******************************************************************************/                                                                                                   
void IgnoreSignal(int sign)                                                                                                                                                        
{                                                                                                                                                                                  
    if (sign != SIGALRM){                                                                                                                                                          
        logPrint (trcLogId,FL, "[%s] UNWANTED SIGNAL IS RECEIVED, sig=%d\n"                                                                                                        
                ,__FUNCTION__, sign);                                                                                                                                              
	}
    signal(sign, IgnoreSignal);                                                                                                                                                    
}                                                                                                                                                                                  
                                                                                                                                                                                   
/*******************************************************************************                                                                                                   
                                                                                                                                                                                   
*******************************************************************************/                                                                                                   
void SetUpSignal()                                                                                                                                                                 
{                                                                                                                                                                                  
                                                                                                                                                                                   
    /* WANTED SIGNALS   */                                                                                                                                                         
    signal(SIGTERM, UserControlledSignal);                                                                                                                                         
    signal(SIGINT,  UserControlledSignal);                                                                                                                                         
    signal(SIGQUIT, UserControlledSignal);                                                                                                                                         
                                                                                                                                                                                   
    /* UNWANTED SIGNALS */                                                                                                                                                         
    signal(SIGHUP,  IgnoreSignal);                                                                                                                                                 
    signal(SIGALRM, IgnoreSignal);                                                                                                                                                 
    signal(SIGPIPE, IgnoreSignal);                                                                                                                                                 
    signal(SIGPOLL, IgnoreSignal);                                                                                                                                                 
    signal(SIGPROF, IgnoreSignal);                                                                                                                                                 
    signal(SIGUSR1, IgnoreSignal);                                                                                                                                         
    signal(SIGUSR2, IgnoreSignal);                                                                                                                                                 
    signal(SIGVTALRM, IgnoreSignal);                                                                                                                                               
}        

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
	//commlib_setupSignals (NULL);
	SetUpSignal ();

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

	/* �ý��� ���� application���� �̸��� queue key�� �о� msgQRoutTbl�� setting�Ѵ�.
	*/
	if (ixpc_initMsgQRoutTbl(fname) < 0)
		return -1;

	/* tcp�� ������ remote �ý��۵��� �̸��� ip address�� �о� sockRoutTbl�� setting�Ѵ�.
	*/
	if (ixpc_initSockRoutTbl(fname) < 0)
		return -1;

	/* config file���� �ڽ��� message queue key�� �о�, attach queue
	*/
	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", myAppName, 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((ixpcQid = msgget(key,IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[ixpc_init] msgget fail; key=0x%x,err=%d(%s)\n",key,errno,strerror(errno));
		return -1;
	}
//printf("IXPC key : %d, qid : %d\n", key, ixpcQid);

	/* config file���� �ڽ��� bind port number�� �о� binding�Ѵ�.
	*/
	if (conflib_getNthTokenInFileSection (fname, "[SOCKET_PORT]", "IXPC", 1, tmp) < 0)
		return -1;
	ixpcPortNum = strtol(tmp,0,0);
	if (socklib_initTcpBind(ixpcPortNum) < 0)
		return -1;

	/* IXPC connection ���� ������ ���� shared memory �ʱ�ȭ */
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
		if ( sockRoutTbl[i].sysName[0] == 0 ) {
			ixpcCON->ixpcCon[i].name[0] = 0;
			continue;
		}

		strcpy ( ixpcCON->ixpcCon[i].name, sockRoutTbl[i].sysName);
		ixpcCON->ixpcCon[i].connSts = SFM_LAN_DISCONNECTED;
	}

	if(set_version(SEQ_PROC_IXPC,vERSION) < 0)
	{
        fprintf (stderr,"[ixpc_initial] set_version failed\n");
        return -1;
    }

	/* sockRoutTbl�� setting�� �ּҷ� remote �ý����� ixpc�� connect�Ѵ�.
	*/
#if 0
	if (!strcmp(mySysName, "SCMA")) {
		system ("route add -host DSCM SCMA");
		system ("route add -host DSCM SCMA_S");
	} else {
		system ("route add -host DSCM SCMB");
		system ("route add -host DSCM SCMB_S");
	}
#endif
	

	/* watch-dog ������AAo��| ��������CO �ҡ� ��c��eCO SYSTEM_LABEL AI���� ����CN��U */
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


	ixpc_checkConnections ();

	/* log file���� open�Ѵ�.
	*/
	if (ixpc_initLog () < 0)
		return -1;

	logPrint (trcLogId,FL,"%s startup...\n", myAppName);
	logPrint (trcErrLogId,FL,"%s startup...\n", myAppName);
	return 1;

} /** End of ixpc_initial **/



/*------------------------------------------------------------------------------
* config file���� ��ϵ� �ý��� ���� application���� ������ �о� �̸��� queue key��
*	msgQRoutTbl�� setting�Ѵ�.
* - [APPLICATIONS] section�� �ִ� SYSCONF_MAX_APPL_NUM������ ��ϵ� �� �ִ�.
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

	/* [APPLICATIONS] section���� �̵�
	*/
	if ((lNum = conflib_seekSection(fp,"[APPLICATIONS]")) < 0) {
		fprintf(stderr,"[ixpc_initMsgQRoutTbl] not found section[APPLICATIONS] in file[%s]\n", fname);
		return -1;
	}

	/* ��ϵ� application���� �̸��� Qkey�� �����Ѵ�.
	*/
	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(appCnt < SYSCONF_MAX_APPL_NUM) ) {
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		/* config ���Ͽ� "name = Qkey ����������ġ" format���� ����ִ�.
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
* config file���� tcp�� ������ remote �ý��۵��� ip address�� �о� sockRoutTbl��
*	setting�Ѵ�.
* - [ASSOCIATE_SYSTEMS] section�� �ִ� SYSCONF_MAX_ASSO_SYS_NUM������ ��ϵ� �� �ִ�.
* - config file�� ip address�� �������� �ʴ� ��� �ݵ�� "NULL"�� ��ϵǾ�� �Ѵ�.
------------------------------------------------------------------------------*/
char myip1[BUFSIZ];
char myip2[BUFSIZ];

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

	/* [ASSOCIATE_SYSTEMS] section���� �̵�
	*/
	if ((lNum = conflib_seekSection(fp,"ASSOCIATE_SYSTEMS")) < 0) {
		fprintf(stderr,"[ixpc_initSockRoutTbl] not found section[ASSOCIATE_SYSTEMS] in file[%s]\n", fname);
		return -1;
	}

	/* ��ϵ� system���� �̸��� ip address�� �����Ѵ�.
	*/
	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(sysCnt < SYSCONF_MAX_ASSO_SYS_NUM) ) {
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		/* config ���Ͽ� "name = type group primary_addr seconday_addr" format���� ����ִ�.
		*/
		if (sscanf(getBuf,"%s%s%s%s%s%s",token[0],token[1],token[2],token[3],token[4],token[5]) < 6) {
			fprintf(stderr,"[ixpc_initSockRoutTbl] syntax error; file=%s, lNum=%d\n", fname, lNum);
			return -1;
		}
		if (!strncmp(mySysName,getBuf,strlen(mySysName))) { /* �ڽ��� ������ ������� �ʴ´�.  */
			strcpy (myip1, token[4]);
			strcpy (myip2, token[5]);
			continue;
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
* error �޽����� ���� error log file�� trace �޽����� ���� log file�� open�Ѵ�.
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


