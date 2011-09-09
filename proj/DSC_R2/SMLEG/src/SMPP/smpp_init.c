/**********************************************************
   Author   : Kim Hong lak
   Section  : SCP for KTF
   SCCS ID  : %W%
   Date     : %G%
   Revision History : 

   Description:

***********************************************************/
/**A.1*  File Inclusion ***********************************/

#include <smpp.h>

extern	char	module_conf[256];
extern  char    sysLabel[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern  int 	errno;
extern	int		numMmcHdlr;
extern  MYSQL   *conn;
extern  SMPP_FD_TBL 	client;
extern 	MmcHdlrVector 	mmcHdlrVector[MML_MAX_MMC_HANDLER];
extern 	SMS_DB_INFO 	sms_db;
extern	SMPP_MSG_INFO 	smpp_msg_info;

extern int		JiSTOPFlag;
extern int		FinishFlag;

int 			smc_shmid;
char			smpp_info_file[256];
SMC_TABLE		*smc_tbl;
SMPP_CID  		smpp_cid[MAX_SMPP_CID];

// TRACE_INFO.conf ±¸A¶A¼ 
st_SESSInfo			*gpCurTrc; // CURRENT OPERATION pointer
// TIMEOUT.conf 구조체 
MPTimer            	*gpCurMPTimer;


extern int InitSHM_TIMER(void);
extern int InitSHM_TRACE(void);

int smpp_init(int pid)
{
    int		i, err; 

	char    queue_temp[1024];
    char    *env, fname[256], tmp[64];
    int     key, num;

	SetUpSignal();
    //commlib_setupSignals (NULL);

    num=0;
    if ((env = getenv (MY_SYS_NAME)) == NULL) {
        fprintf(stderr,"[smpp_init] not found %s environment name\n", MY_SYS_NAME);
        return -1;
    }
	/*## MMC INIT ##*/
	/* MMC 처리시 bsearch 사용을 위한 numMmcHdlr vector align */
	qsort ( (void*)mmcHdlrVector
			, numMmcHdlr
			, sizeof(MmcHdlrVector)
			, smpp_mmcHdlrVector_qsortCmp );

    strcpy (mySysName, env);
    strcpy (myAppName, "SMPP");

	/*## APPLICATION LOG INIT ##*/
	Init_logdebug( pid, "SMPP", "/DSC/APPLOG");

    /*## KEEPALIVE INIT ##*/
    if (keepalivelib_init (myAppName) < 0)
        return -1;

    if ((env = getenv(IV_HOME)) == NULL) {
        dAppLog (LOG_CRI, "[SMPP_INIT] not found %s environment name", IV_HOME);
        return -1;
    }

	/*## SYSTEM CONFIG LOAD ##*/
    sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", myAppName, 1, tmp) < 0) {
		dAppLog (LOG_CRI, "[SMPP_INIT] My System message queue Get Fail = %s[%d]", strerror(errno), errno);
		return -1;
	}
	/* smpp queue */
	key = strtol(tmp,0,0);
    if ((smppQid = msgget (key, IPC_CREAT|0666)) < 0) {
        dAppLog (LOG_CRI, "[SMPP_INIT] msgget fail; key=0x%x, err=%d(%s)", key, errno, strerror(errno));
        return -1;
    }
	/* smpp queue init */
    while (1) {
        if (msgrcv(smppQid, queue_temp, 1024, 0, IPC_NOWAIT) < 0)
            break;
        else
            num++;
    }
    dAppLog (LOG_INFO, "### smpp Queue (count=%d) Cleared !!", num);

	/* ixpc queue */
	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "IXPC", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((ixpcQid = msgget(key,IPC_CREAT|0666)) < 0) {
		dAppLog (LOG_CRI, "[SMPP_INIT] msgget fail; key=0x%x,err=%d(%s)",key,errno,strerror(errno));
		return -1;
	}

	/*## SMPP SHM init */
	/* 공유메모리를 생성하고 Point 지정한다*/
    err = init_smpp_shm();
	if (err < 0) {
        dAppLog(LOG_CRI, "[SMPP_INIT] create failed shm of smpp(%d)", err); 
		return -1;
	}
	else {
		if (smpp_get_smc_info() <0) {
			dAppLog(LOG_CRI, "[SMPP_INIT] loading failed configuration file(%s)", module_conf); 
			return -1;
		}
	}

	/* SMPP CONFIG LOAD */
	if (smpp_get_smpp_conf () < 0) {
		dAppLog(LOG_CRI, "[SMPP_INIT] loading failed configuration file(%s)", module_conf); 
		return -1;
	}

	/*## SQUENCE ID ININT ##*/
	InitCid(20000, 20000 + MAX_SMPP_CID);

	/*## SMSC CONNECTION INFO INIT ##*/
	client.cur_con = 0;
	client.maxfd = 0;
	for (i = 0; i < MAX_SMC_INFO; i++) {
		client.condata[i].fd = 0;
		client.condata[i].bind = 0;
		client.condata[i].bindtry = 0;
		client.condata[i].type = 0;
		client.condata[i].prototype = 0;
		client.condata[i].port_no = 0;
		client.condata[i].writeFailCnt = 0;
	}

    for (i = 0; i < MAX_SMPP_CID; i++) {
		smpp_cid[i].cid = 0;
        smpp_cid[i].conidx = 0;
		smpp_cid[i].prototype = 0;
    }

	return 1;
}


int init_smpp_shm (void)
{
	int		key;
	char    *env, fname[256], tmp[64];
//	int 	shmId=0;


	if ((env = getenv(IV_HOME)) == NULL) {
        dAppLog(LOG_CRI, "[init_smpp_shm]  not found %s environment name", IV_HOME); 
        return -1;
    }

    sprintf (fname, "%s/%s", env, SYSCONF_FILE);
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "S_SSHM_SMPP", 1, tmp) < 0) {
        dAppLog(LOG_CRI, "[init_smpp_shm] My System sharedmemory Get Fail = %s[%d]", strerror(errno), errno);
        return -1;
     }
    key = strtol(tmp,0,0);

    smc_shmid = shmget(key, sizeof(SMC_TABLE), IPC_EXCL|IPC_CREAT|0666) ;    
    if (smc_shmid >= 0){ /* 새롭게 shared memory가 생성이 된 경우 */
        smc_tbl = (SMC_TABLE *)shmat(smc_shmid, 0, 0);
        if((int)smc_tbl == -1) {
			dAppLog(LOG_CRI, "[init_smpp_shm] shmat error(%s, %d)", strerror(errno), errno);
            return	-1;
        }       
		//return SHM_CREATE;

    } else {
		/* 기존에 shared memory가 생성되어 있었던 경우 */
		if (errno == EEXIST){ 
    		smc_shmid = shmget(key, sizeof(SMC_TABLE), IPC_CREAT|0666) ;    

			/* memory attatch를 할수 없는 경우 */
        	smc_tbl = (SMC_TABLE *)shmat(smc_shmid, 0, 0);
        	if((int)smc_tbl == -1){
				dAppLog(LOG_CRI, "[init_smpp_shm] shmat error(%s, %d)", strerror(errno), errno);
            	return	-1;
        	}       
        	//return	SHM_EXIST;

		} else { /* shared memory를 생성할수 없는 경우 */
			dAppLog(LOG_CRI, "[init_smpp_shm] shmget error(%s, %d)", strerror(errno), errno);
			return -1;
		}
	}

	if( InitSHM_TIMER() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_TIMER() FAIL");
		return -1;
	}

	if( InitSHM_TRACE() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_TIMER() FAIL");
		return -1;
	}

#if 0
	/* INIT_SHM: TIMER */
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TIMER", 1, tmp) < 0) {
        dAppLog(LOG_CRI, "[init_smpp_shm] timeout config not exist  %s[%d]", strerror(errno), errno);
		return -1;
	}

	key = strtol(tmp,0,0);
	if ((shmId = (int)shmget(key, sizeof(MPTimer), 0666|IPC_CREAT)) < 0) {
		if (errno != ENOENT) {
			dAppLog(LOG_CRI,"[mmcr_init] shmget fail4, key=0x%x, err=%d(%s)", key, errno, strerror(errno));
			return -1;
		}
	}
	if ((gpMPTimer = (MPTimer*) shmat (shmId,0,0)) == (MPTimer*)-1) {
		dAppLog(LOG_CRI,"[mmcr_init] shmat fail5, key=0x%x, err=%d(%s)", key, errno, strerror(errno));
		return -1;
	}
	dAppLog(LOG_CRI,"[mmcr_init] timeout shm info: sms timeout = %u", gpMPTimer->sms_timeout);
	smpp_dLoadTimeOut();
#endif
	return 1;
}


int smpp_get_smc_info (void)
{
	int		lNum, rowCnt=0, ret;
	char	*env;
	char	iv_home[64];
	char    getBuf[256], token[9][64];
	FILE	*fp=NULL;
	int 	isInfo=0;

	if( (env = getenv(IV_HOME)) == NULL) {
		dAppLog(LOG_CRI, "[%s:%s:%d] not found %s environment name", __FILE__, __FUNCTION__, __LINE__, IV_HOME);
		return -1;
	}
	strcpy(iv_home, env);
	sprintf(module_conf, "%s/%s", iv_home, SMPP_CONF_FILE);

	if ((fp = fopen(module_conf, "r")) == NULL) {
		dAppLog(LOG_CRI, "[smpp_get_smc_info] fopen fail[%s]; err=%d(%s)", module_conf, errno, strerror(errno));
		return -1;
	}

	/* [SMSC_INFO] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"SMSC_INFO")) < 0) {
		return -1;
	}

	/* 등록된 시스템들의 이름과 IP_ADDRESS를 저장한다. */
	while (fgets(getBuf,sizeof(getBuf),fp) != NULL) 
	{
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if ((ret = sscanf(getBuf,"%s%s%s%s%s%s%s", token[0],token[1],token[2],token[3],token[4],token[5],token[6])) < 7) {
			dAppLog(LOG_DEBUG, "[smpp_get_smc_info] syntax error; file=%s, lNum=%d, ret = %d", module_conf , lNum, ret);
			fclose(fp);
			return -1;
		}

		if (!strcasecmp(token[6], mySysName)) {
			strcpy(smc_tbl->smc_info[0].smsc_id, token[0]);
			strcpy(smc_tbl->smc_info[0].ip_addr, token[2]);
			smc_tbl->smc_info[0].port_no = atoi(token[3]);
			strcpy(smc_tbl->smc_info[0].user_id, token[4]);
			strcpy(smc_tbl->smc_info[0].passwd, token[5]);
			strcpy(smc_tbl->smc_info[0].use_scm, token[6]);

			dAppLog(LOG_DEBUG, "### SMSC SERVER INFO : NAME:%s  IP:%s  PORT:%d  USER:%s  PASS:%s, USE_SCM:%s"
					, smc_tbl->smc_info[0].smsc_id
					, smc_tbl->smc_info[0].ip_addr
					, smc_tbl->smc_info[0].port_no
					, smc_tbl->smc_info[0].user_id
					, smc_tbl->smc_info[0].passwd
					, smc_tbl->smc_info[0].use_scm);

			isInfo = 1;
			break;
		}
		rowCnt++;
	}
	if (isInfo == 0) {
		fclose(fp); return -1;
	}

	fclose(fp);
	return 0;
}


int smpp_get_smpp_conf (void)
{
	int		lNum, ret;
	char	*env;
	char	iv_home[64];
	char    getBuf[256], token[9][64];
	FILE	*fp=NULL;

	if( (env = getenv(IV_HOME)) == NULL) {
		dAppLog (LOG_DEBUG, "[%s:%d] not found %s environment name", __FUNCTION__, __LINE__, IV_HOME);
		return -1;
	}
	strcpy(iv_home, env);
	sprintf(module_conf, "%s/%s", iv_home, SMPP_CONF_FILE);

	if ((fp = fopen(module_conf, "r")) == NULL) {
		dAppLog (LOG_DEBUG, "[%s] fopen fail[%s]; err=%d(%s)", module_conf, errno, strerror(errno));
		return -1;
	}

	/* [DB_INFO] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"DB_INFO")) < 0) {
		return -1;
	}

	/* 등록된 시스템들의 이름과 IP_ADDRESS를 저장한다. */
	while (fgets(getBuf,sizeof(getBuf),fp) != NULL) 
	{
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if ((ret = sscanf(getBuf,"%s%s%s%s%s",token[0],token[1],token[2],token[3], token[4])) < 5) {
			dAppLog (LOG_DEBUG, "[smpp_get_smppp_conf][DB_INFO] syntax error; file=%s, lNum=%d, ret = %d"
					, module_conf , lNum, ret);
			fclose(fp);
			return -1;
		}

		strcpy(sms_db.name, token[0]);
		strcpy(sms_db.ipaddr, token[2]);
		strcpy(sms_db.user, token[3]);
		strcpy(sms_db.passwd, token[4]);
		strcpy(sms_db.table, SMS_TBL);
		dAppLog(LOG_DEBUG, "### DB INFO : NAME:%s  IP:%s  USER:%s  PASS:%s  TABLE:%s"
						, sms_db.name
						, sms_db.ipaddr
						, sms_db.user
						, sms_db.passwd
						, sms_db.table);
	}
	

	/* [SMPP_MSG] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"SMPP_MSG")) < 0) {
		return -1;
	}

	while (fgets(getBuf,sizeof(getBuf),fp) != NULL) 
	{
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if ((ret = sscanf(getBuf,"%s%s%s%s%s", token[0],token[1],token[2],token[3],token[4])) < 5) {
			fprintf(stderr,"[smpp_get_smppp_conf][SMPP_MSG] syntax error; file=%s, lNum=%d, ret = %d\n"
					, module_conf , lNum, ret);
			fclose(fp);
			return -1;
		}
		strcpy(smpp_msg_info.sysid, token[0]);
		smpp_msg_info.tid = atoi(token[2]);
		strcpy(smpp_msg_info.org_addr, token[3]);
		strcpy(smpp_msg_info.callback, token[4]);
		dAppLog(LOG_DEBUG, "### SMS MSG INFO : SYSID:%s  TID:%d  ORG_ADDR:%s  CALLBACK::%s"
						, smpp_msg_info.sysid
						, smpp_msg_info.tid
						, smpp_msg_info.org_addr
						, smpp_msg_info.callback);
	}

	fclose(fp);
	return 0;
}

void smpp_finProc(void)
{
	int i;

	dAppLog(LOG_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", FinishFlag);

	/* socket close */
	for(i=0; i<MAX_SMC_INFO; i++)
	{
		int cfd=0;

		cfd = client.condata[i].fd;
		if(cfd!=NULL)
			close(cfd);	
	}
	
	/* db handle close */
	if (conn!=NULL)
		mysql_close(conn);

	exit(0);
}

/************************************************************************************
	SIGNAL FUNCTION
***********************************************************************************/
void UserControlledSignal(int sign)
{
    JiSTOPFlag = 0;
    FinishFlag = sign;
}

void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        dAppLog(LOG_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
}

void SetUpSignal(void)
{
    JiSTOPFlag = 1;

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
    signal(SIGCLD, SIG_IGN);

    dAppLog(LOG_CRI, "SIGNAL HANDLER WAS INSTALLED");
}

#if 0
int smpp_dLoadTimeOut(void)
{
	FILE        *fa;    
	char        szBuffer[1024], szFName[256];
	unsigned int	sess_to=0, sms_to=0;
	char		*env;

	if ((env = getenv(IV_HOME)) == NULL){
		dAppLog(LOG_DEBUG, "[smpp_dLoadTimeOut] getenv error! \n" );
		return -1;
	}
	sprintf (szFName, "%s/%s", env, TIMEOUT_FILE);

	if ((fa=fopen(szFName, "r")) == NULL) { 
		dAppLog( LOG_CRI, "[smpp_dLoadTimeOut] %s file not found", szFName);
		return -1;
	}       

	fseek(fa, 0, SEEK_SET);
	while (fgets( szBuffer, 1024, fa ) != NULL)
	{
		if((szBuffer[0] == '@')||(szBuffer[0] == '#')) 
			continue;

		if( sscanf( &szBuffer[0], "%d %d", &sess_to, &sms_to) == 2 ) {
			gpMPTimer->sess_timeout = sess_to;
			gpMPTimer->sms_timeout = sms_to;
		}
		else {
			dAppLog( LOG_CRI, "[smpp_dLoadTimeOut] %s file format error", szFName);
			fclose(fa);
			return -1;
		}       
	}

	fclose(fa);
	smpp_dLogTimeOut(LOG_DEBUG);

	return 0;
}
#endif

#if 0
void smpp_dLogTimeOut(int level)
{
	dAppLog(level, "sess timeout: %u", gpMPTimer->sess_timeout);
	dAppLog(level, "sms  timeout: %u", gpMPTimer->sms_timeout);
}
#endif

int dGetTraceData(pst_SESSInfo pstInfo)
{
    FILE    *fp;
    int     dIndex = 0;
    char    szBuf[1024];
    char    ppstData[64][64];
    int     iRet = 0;
    int     iCount = 0;
    char    fileName[64] = {0,}, *env = NULL;

    pstInfo->dTCount = 0;

	memset(pstInfo, 0x00, sizeof(st_SESSInfo));

    if ((env = getenv(IV_HOME)) == NULL) {
        dAppLog(LOG_DEBUG,"dGetTraceData() not found %s environment name", IV_HOME);
        return -1;
    }
    sprintf (fileName, "%s/%s", env, TRACE_INFO_FILE);

    fp = fopen(fileName, "r");
    if( fp == NULL ) {
        if( errno == ENOENT ) {
            dAppLog(LOG_DEBUG, "[ERROR] GET TRACE FILE NOT EXIST[%s][%s]",
                TRACE_INFO_FILE, strerror(errno));
            return 100;
        }
        dAppLog( LOG_DEBUG, "[ERROR] TRACE INFO FILE OPEN[%s][%s]",         
                TRACE_INFO_FILE, strerror(errno));
        return -1;
    }

    while( fgets(szBuf, 1024, fp) != NULL )
    {
        if( dIndex >= MAX_TRACE_NUM )
            break;
        if(szBuf[0] == '#' || szBuf[0] == '\t'
            || szBuf[0] == ' ' || szBuf[0] == '\n')
            continue;

        memset(ppstData, 0x00, sizeof(ppstData));
        iRet = GetConfData(szBuf, (char **)ppstData, &iCount);
        if(!strcmp(ppstData[0], "@START")) {                                                                     
            continue;                                                         
        }   
        else if(!strcmp(ppstData[0], "@END")) {
            break;
        }

        if( iCount < 3 ) {
            return -1;
        }
		
        pstInfo->stTrc[dIndex].dType = strtol(ppstData[0], NULL, 10);
        //pstInfo->stTrc[dIndex].llInfo = strtoll(ppstData[1], NULL, 10); // IMSI, IP
        strncpy(pstInfo->stTrc[dIndex].szImsi, ppstData[1], strlen(ppstData[1]));
        pstInfo->stTrc[dIndex].tRegTime = strtol(ppstData[2], NULL,10);
        pstInfo->stTrc[dIndex].dDura = strtol(ppstData[3], NULL,10);
		
        dAppLog(LOG_INFO, "TRACE IN MEMORY INDEX[%d] TYPE[%d] IMSI[%s] TIME[%d]"
				, dIndex
				, pstInfo->stTrc[dIndex].dType
				, pstInfo->stTrc[dIndex].szImsi
				, pstInfo->stTrc[dIndex].tRegTime);
        dIndex++;
    }

    fclose(fp);
    pstInfo->dTCount = dIndex;

    return 0;
}

int GetConfData(char *sParseStr, char **ppstData, int *iCount)                                                                                      
{                                                                                                                                                         
	int     e_Ret = 0;                                                                                                                                    
	int i = 0;                                                                                                                                            
	char *tmpStr = NULL;                                                                                                                                  
	char getData[64];                                                                                                                                     

	tmpStr = sParseStr;                                                                                                                                   
	for(i = 0; i<64; i++)                                                                                                                                 
	{
		memset(getData, 0x00, 64);
		tmpStr = TreamNullData(tmpStr);
		Get1Data(tmpStr, getData);
		if(getData[0] == '\0')                                                                                                                            
		{                                                                                                                                                 
			break;                                                                                                                                        
		}                                                                                                                                                 
		snprintf(ppstData[i], 64, "%s", getData);                                                                                                         
		tmpStr += strlen(ppstData[i]);                                                                                                                    
	}                                                                                                                                                     

	*iCount = i;                                                                                                                                          

	return e_Ret;                                                                                                                                         
} 

char * TreamNullData(char *sParseStr)
{
    int i = 0;

    for(i = 0; i<(int)strlen(sParseStr); i++)
    {
        if( sParseStr[i] == ' ' || sParseStr[i] == '\t' )
        {
            continue;
        }
        else
        {
            break;
        }
    }
    return (sParseStr + i);
}

void Get1Data(char *sParseStr, char *getData)
{
    int     i = 0;


    for(i = 0; i<(int)strlen(sParseStr); i++)
    {
        if( sParseStr[i] == ' ' || sParseStr[i] == '\t'
            || sParseStr[i] == '\0' || sParseStr[i] == '\n')
        {
            break;
        }
        else
        {
            getData[i] = sParseStr[i];
        }
    }
    getData[i] = '\0';

    return;
}


int dLoad_TrcConf(st_SESSInfo  *pstCallTrcList)
{
	FILE        *fa;
	char        szBuffer[1024], szFName[256];
	char        TYPE[2], IMSI[17], REGTIME[14], DURATION[5];
	char 		*env=NULL;
	int         cnt = 0;

	pstCallTrcList->dTCount=0;
	
	if ((env = getenv(IV_HOME)) == NULL) {
        dAppLog(LOG_DEBUG,"[dLoad_TrcConf] not found %s environment name", IV_HOME);
        return -1;
    }

	sprintf(szFName, "%s/%s",env, TRACE_INFO_FILE );

	if ((fa=fopen(szFName, "r")) == NULL) {
		dAppLog(LOG_DEBUG, "[dLoad_TrcConf] fopen fail.[%s]\n",szFName );
		return -1;
	}

	fseek(fa, 0, SEEK_SET);
	cnt = 0;
	while (fgets( szBuffer, 1024, fa ) != NULL)
	{                                                                                                                                                                    
		memset(TYPE, 0x00, sizeof(TYPE));                                                                                                                                
		memset(IMSI, 0x00, sizeof(IMSI));                                                                                                                                
		memset(REGTIME, 0x00, sizeof(REGTIME));                                                                                                                          
		memset(DURATION, 0x00, sizeof(DURATION));                                                                                                                        

		if(szBuffer[0] == '@') continue;                                                                                                                                                    

		if( sscanf( &szBuffer[0], "%s %s %s %s", TYPE, IMSI, REGTIME, DURATION) == 4 ) {                                                                                 
			pstCallTrcList->stTrc[cnt].dType = atoi(TYPE);                                                                                                               
			pstCallTrcList->stTrc[cnt].dDura = atoi(DURATION);                                                                                                           
			sprintf(pstCallTrcList->stTrc[cnt].szImsi, "%s", IMSI);                                                                                                      
			pstCallTrcList->stTrc[cnt].tRegTime = atoi(REGTIME);                                                                                                         

			dAppLog(LOG_DEBUG, "[dLoad_TrcConf] %-12d %-18s %-16d %-4d\n",
					pstCallTrcList->stTrc[cnt].dType,
					pstCallTrcList->stTrc[cnt].szImsi,
					pstCallTrcList->stTrc[cnt].tRegTime,
					pstCallTrcList->stTrc[cnt].dDura);
		}
		else {
			dAppLog(LOG_DEBUG, "[dLoad_TrcConf] %s FILE FORMAT ERROR! \n", szFName );
			fclose(fa);
			return -1;
		}
		cnt++;
	}
	pstCallTrcList->dTCount = cnt;
	fclose(fa);
	return 0;
}

