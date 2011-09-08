#include "cscm.h"

extern int		ixpcQID, cscmQID, mcdmQID, trcLogId, trcErrLogId, queCNT;
extern char		*iv_home, l_sysconf[256];
extern char		trcBuf[4096], trcTmp[1024];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern char		szl2swFileName[256];
extern T_keepalive  *keepalive;
extern SFM_L2Dev	*g_pstL2Dev;
extern SFM_SysCommMsgType   *loc_sadb;

extern char         g_szL2_IPaddr[MAX_PROBE_DEV_NUM][20];
extern int          L2COUNT;
extern char         g_szRDR_IPAddr[MAX_PROBE_DEV_NUM][20];

#ifdef DATA_SCP
#include "system.h"
extern int		shmKey_dscp_cliTbl, shmKey_dscp_srvTbl;
#endif

char    ver[8] = "R2.0.0";		// initial	

extern int init_L2_snmp();

int InitSys (void)
{
	char	*env, tmp[32]; 
	int		key, ret, shmId;

	if ((env = getenv(MY_SYS_NAME)) == NULL) {
		fprintf(stderr,"[cscm_init] not found %s environment name\n", MY_SYS_NAME);
		return -1;
	}
	strcpy (mySysName, env);
	strcpy (myAppName, "CSCM");
/*
	notMaskSig[0] = SIGCHLD;
	notMaskSig[1] = 0;
	commlib_setupSignals (notMaskSig);
*/
	commlib_setupSignals (NULL);

	// ping_test할때 wait로 child 프로세스의 종료 조건을 확인해야하는데,
	//	commlib_setupSignals에서 SIGCHLD를 catch하도록 되어 있는 것을 release한다.
	sigrelse (SIGCHLD);

	if ((iv_home = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[cscm_init] not found %s environment name\n", IV_HOME);
		return -1;
	}

	if(set_proc_version(OMP_VER_INX_CSCM, ver) < 0){
		fprintf(stderr, "[InitSys] setting process version failed\n");
		return -1;
	}

	if (cscm_initLog() < 0)
		return -1;

	sprintf(l_sysconf, "%s/%s", iv_home, SYSCONF_FILE);

	if ((ret = keepalivelib_init("CSCM")) < 0)
		return -1;
	memset ((char*)keepalive, 0, sizeof(T_keepalive));

	/* IXPC MSG Qid를 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "IXPC", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((ixpcQID = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[cscm_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}
//	fprintf(stdout, "ixpcQID: %d\n", ixpcQID); // by helca
	/* CSCM MSG Qid를 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "CSCM", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((cscmQID = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[cscm_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}
//	fprintf(stdout, "cscmQID: %d\n", cscmQID); // by helca
	/* MCDM MSG Qid를 구한다 : sjjeon*/
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "MCDM", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((mcdmQID = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[cscm_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	if (conflib_getNthTokenInFileSection (l_sysconf, "SHARED_MEMORY_KEY", "SHM_LOC_SADB", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	if ((shmId = (int)shmget (key, sizeof(SFM_SysCommMsgType), 0666|IPC_CREAT)) < 0) {
		if (errno != ENOENT) {
			fprintf (stderr,"[cscm_init] shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
	}
	if ((loc_sadb = (SFM_SysCommMsgType*) shmat (shmId,0,0)) == (SFM_SysCommMsgType*)-1) {
		fprintf (stderr,"[cscm_init] shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// L2SW Device INIT
	if (conflib_getNthTokenInFileSection (l_sysconf, "SHARED_MEMORY_KEY", "SHM_L2SW", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	sprintf (szl2swFileName, "%s/%s", iv_home, SFM_L2SW_FILE); // by helca 09.11
	if (cscm_getL2SW(key) < 0)
		return -1;

	if(cscm_get_snmp_L2_ipaddress(l_sysconf) < 0){
		fprintf(stderr,"[cscm_init] cscm_get_snmp_L2_ipaddress fails\n");
		return -1;
	}

	if(init_L2_snmp() < 0){
		return -1;
	}

#if TRU64
	loc_sadb->cpuCount = 1; /* 일단 무조건 1로 설정한다.*/
#endif

	/* disk partition 갯수와 이름을 저장한다.
	*/
	logPrint (trcLogId,FL,"%s startup...\n", myAppName);
	logPrint (trcErrLogId,FL,"%s startup...\n", myAppName);

	return 1;
}

int cscm_initLog (void)
{
    char    fname[256];

    sprintf(fname,"%s/%s.%s", iv_home, CSCM_LOG_FILE, mySysName);

    if ((trcLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[cscm_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    sprintf(fname,"%s/%s.%s", iv_home, CSCM_ERRLOG_FILE, mySysName);
    if ((trcErrLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[cscm_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    return 1;

} /** End of cscm_initLog **/

int cscm_get_snmp_L2_ipaddress(char *fname)
{
	char    getBuf[256], IPAddr[20];
	int     IPcnt=0, lNum, i, j;
	FILE    *fp;

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[cscm_get_snmp_L2_ipaddress] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* ["L2_IP_ADDRESS"] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"L2_IP_ADDRESS")) < 0) {
		if(fp)fclose(fp);
		return -1;
	}

	memset(g_szL2_IPaddr, 0x00, sizeof(g_szL2_IPaddr));
	L2COUNT = 0;

	/* 등록된 시스템들의 이름과 IP_ADDRESS를 저장한다. */
	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(IPcnt < MAX_PROBE_DEV_NUM) )
	{
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if(inet_addr(getBuf) < 0){
			fprintf(stderr,"[cscm_get_snmp_L2_ipaddress] Wrong L2 IP Address %s\n", getBuf);
			fclose(fp);
			return -1;
		}

		memset(IPAddr, 0x00, sizeof(IPAddr));
		for(i = 0; getBuf[i] == ' ';i++);
		for(i = i, j = 0; (isdigit(getBuf[i]) || getBuf[i] == '.'); i++, j++)
			IPAddr[j] = getBuf[i];

		IPAddr[j] = 0x00;

		strncpy(g_szL2_IPaddr[IPcnt], IPAddr, 20);
		IPcnt++;
	}
	fclose(fp);
	L2COUNT = IPcnt;

	return 0;
}

int cscm_getL2SW (int key)
{
	int	ret, fd, shmId, dLoadFlag=0;
	char	*env;

	if ((env = getenv(IV_HOME)) == NULL)
        return -1;
	// attach
	if ((shmId = (int)shmget (key, SFM_L2DEV_SIZE, 0666)) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[cscm_getL2SW] L2SW shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		// shared memory가 아직 만들어 지지 않은 경우이면 shared memory를 create한 후
		//	이전에 file로 backup해둔 놈에서 정보를 읽어 들인다.
		if ((shmId = (int)shmget (key, SFM_L2DEV_SIZE, IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[cscm_getL2SW] L2SW shmget fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		dLoadFlag = 1;
	}
	if ((g_pstL2Dev = (SFM_L2Dev*) shmat (shmId,0,0)) == (SFM_L2Dev*)-1) {
		fprintf(stderr,"[cscm_getL2SW] L2SW shmat fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	// share memory가 새로 만들어졌으면 backup file에서 읽어 들인다.
	// - backup file이 없으면 default 값으로 setting한다.
	//
	
	if (dLoadFlag) {
		if ((fd = open (szl2swFileName, O_RDONLY, 0666)) >= 0) {
			lseek(fd,0,0);
			if ((ret = read (fd, (void*)g_pstL2Dev, SFM_L2DEV_SIZE)) < 0) {
				fprintf(stderr,"[cscm_getL2SW] open fail[%s]; err=%d(%s)\n", szl2swFileName, errno, strerror(errno));
				close(fd);
				return -1;
			}
			close(fd);
		} else {
			if (errno != ENOENT) {
				fprintf(stderr,"[cscm_getL2SW] open fail[%s]; err=%d(%s)\n", szl2swFileName, errno, strerror(errno));
				return -1;
			} else {
				memset ( g_pstL2Dev, 0 ,sizeof(SFM_L2Dev) );
				
			}
		}
	}
	return 1;
}

/* 2009.04.16 by dhkim */
int cscm_get_snmp_rdr_ipaddress(char *fname)
{
	char    getBuf[256],IPAddr[20];
	int     IPcnt=0,lNum, i, j;
	FILE    *fp;

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[cscm_get_snmp_rdr_ipaddress] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* ["RDR_IP_ADDRESS"] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"RDR_IP_ADDRESS")) < 0) {
		if(fp)fclose(fp);
		return -1;
	}

	memset(g_szRDR_IPAddr, 0x00, sizeof(g_szRDR_IPAddr));

	/* 등록된 시스템들의 이름과 IP_ADDRESS를 저장한다. */
	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(IPcnt < MAX_PROBE_DEV_NUM) )
	{
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if(inet_addr(getBuf) < 0){
			fprintf(stderr,"[cscm_get_snmp_rdr_ipaddress] Wrong RDR IP Address %s\n", getBuf);
			fclose(fp);
			return -1;
		}

		memset(IPAddr, 0x00, sizeof(IPAddr));
		for(i = 0; getBuf[i] == ' ';i++);
		for(i = i, j = 0; (isdigit(getBuf[i]) || getBuf[i] == '.'); i++, j++)
			IPAddr[j] = getBuf[i];

		IPAddr[j] = 0x00;

		strncpy(g_szRDR_IPAddr[IPcnt], IPAddr, 20);
		IPcnt++;
	}
	fclose(fp);

	return 0;
}

