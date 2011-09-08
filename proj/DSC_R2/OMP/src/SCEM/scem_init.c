#include "scem.h"

extern int		ixpcQID, scemQID, mcdmQID, trcLogId, trcErrLogId;
extern char		*iv_home, l_sysconf[256];
extern char		trcBuf[4096], trcTmp[1024];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern char		sceFileName[256];
extern T_keepalive  *keepalive;
extern SFM_sfdb     *sfdb;
extern SFM_SCE      *g_pstSCEInfo;
extern char     	g_szSCE_IPAddr[MAX_PROBE_DEV_NUM][20];
extern char         g_szRDR_IPAddr[MAX_PROBE_DEV_NUM][20];
extern int      	SCECOUNT;


#ifdef DATA_SCP
#include "system.h"
extern int		shmKey_dscp_cliTbl, shmKey_dscp_srvTbl;
#endif

char    ver[8] = "R2.0.0";		// initial

extern int init_L2_snmp();

int InitSys (void)
{
	char	*env, tmp[32];
	int		key, ret;

	if ((env = getenv(MY_SYS_NAME)) == NULL) {
		fprintf(stderr,"[scem_init] not found %s environment name\n", MY_SYS_NAME);
		return -1;
	}
	strcpy (mySysName, env);
	strcpy (myAppName, "SCEM");
	commlib_setupSignals (NULL);

	// ping_test할때 wait로 child 프로세스의 종료 조건을 확인해야하는데,
	//	commlib_setupSignals에서 SIGCHLD를 catch하도록 되어 있는 것을 release한다.
	sigrelse (SIGCHLD);

	if ((iv_home = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[scem_init] not found %s environment name\n", IV_HOME);
		return -1;
	}

	if(set_proc_version(OMP_VER_INX_SCEM, ver) < 0){
		fprintf(stderr, "[InitSys] setting process version failed\n");
		return -1;
	}

	if (scem_initLog() < 0)
		return -1;

	sprintf(l_sysconf, "%s/%s", iv_home, SYSCONF_FILE);

	if ((ret = keepalivelib_init("SCEM")) < 0)
		return -1;
	memset ((char*)keepalive, 0, sizeof(T_keepalive));

	/* IXPC MSG Qid를 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "IXPC", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((ixpcQID = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[scem_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}
	/* SCEM MSG Qid를 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "SCEM", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((scemQID = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[scem_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	/* MCDM MSG Qid를 구한다 : sjjeon*/
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "MCDM", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((mcdmQID = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[scem_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SCE Device INIT
	if (conflib_getNthTokenInFileSection (l_sysconf, "SHARED_MEMORY_KEY", "SHM_SCE", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	sprintf (sceFileName, "%s/%s", iv_home, SFM_SCE_FILE); // by helca 09.11
	if (scem_getSCE(key) < 0)
		return -1;

	if(scem_get_snmp_sce_ipaddress(l_sysconf) < 0){
		fprintf(stderr,"[scem_init] scem_get_snmp_sce_ipaddress fails\n");
		return -1;
	}

	if(scem_get_snmp_rdr_ipaddress(l_sysconf) < 0){
		fprintf(stderr,"[scem_init] scem_get_snmp_rdr_ipaddress fails\n");
		return -1;
	}

	if(init_SCE_snmp() < 0){
		return -1;
	}

#if TRU64
	loc_sadb->cpuCount = 1; /* 일단 무조건 1로 설정한다.*/
#endif

	logPrint (trcLogId,FL,"%s startup...\n", myAppName);
	logPrint (trcErrLogId,FL,"%s startup...\n", myAppName);

	return 1;

}

int scem_initLog(void)
{
    char    fname[256];

    sprintf(fname,"%s/%s.%s", iv_home, SCEM_LOG_FILE, mySysName);

    if ((trcLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[scem_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    sprintf(fname,"%s/%s.%s", iv_home, SCEM_ERRLOG_FILE, mySysName);
    if ((trcErrLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[scem_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    return 1;

} /** End of scem_initLog **/

/* 2009.04.14 by dhkim */
int scem_get_snmp_sce_ipaddress(char *fname)
{
	char    getBuf[256],IPAddr[20];
	int     IPcnt=0,lNum, i, j;
	FILE    *fp;


	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[scem_get_snmp_sce_ipaddress] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* ["SCE_IP_ADDRESS"] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"SCE_IP_ADDRESS")) < 0) {
		if(fp)fclose(fp);
		return -1;
	}

	memset(g_szSCE_IPAddr, 0x00, sizeof(g_szSCE_IPAddr));
	SCECOUNT = 0;

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
			fprintf(stderr,"[scem_get_snmp_sce_ipaddress] Wrong SCE IP Address %s\n", getBuf);
			fclose(fp);
			return -1;
		}

		memset(IPAddr, 0x00, sizeof(IPAddr));
		for(i = 0; getBuf[i] == ' ';i++);
		for(i = i, j = 0; (isdigit(getBuf[i]) || getBuf[i] == '.'); i++, j++)
			IPAddr[j] = getBuf[i];

		IPAddr[j] = 0x00;

		//strncpy(g_szSCE_IPAddr[IPcnt], getBuf, 20);
		strncpy(g_szSCE_IPAddr[IPcnt], IPAddr, 20); /* by dhkim */
		IPcnt++;
	}
	fclose(fp);
	SCECOUNT = IPcnt;

	return 0;
}

int scem_getSCE (int key)
{
	int	 ret, fd, shmId, SCELoadFlag=0;
	char	*env;

	if ((env = getenv(IV_HOME)) == NULL)
		return -1;
	// attach
	//
	if ((shmId = (int)shmget (key, SFM_SCE_SIZE, 0666)) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[scem_getSCE] SCE shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		// shared memory가 아직 만들어 지지 않은 경우이면 shared memory를 create한 후
		//	이전에 file로 backup해둔 놈에서 정보를 읽어 들인다.
		if ((shmId = (int)shmget (key, SFM_SCE_SIZE, IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[scem_getSCE] SCE shmget fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		SCELoadFlag = 1;
	}
	if ((g_pstSCEInfo = (SFM_SCE*) shmat (shmId,0,0)) == (SFM_SCE*)-1) {
		fprintf(stderr,"[scem_getSCE] SCE shmat fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	// share memory가 새로 만들어졌으면 backup file에서 읽어 들인다.
	// - backup file이 없으면 default 값으로 setting한다.
	//

	if (SCELoadFlag) {


		if ((fd = open (sceFileName, O_RDONLY, 0666)) >= 0) {
			lseek(fd,0,0);
			if ((ret = read (fd, (void*)g_pstSCEInfo, SFM_SCE_SIZE)) < 0) {
				fprintf(stderr,"[scem_getSCE] open fail[%s]; err=%d(%s)\n", sceFileName, errno, strerror(errno));
				close(fd);
				return -1;
			}
			close(fd);
		} else {
			if (errno != ENOENT) {
				fprintf(stderr,"[scem_getSCE] open fail[%s]; err=%d(%s)\n", sceFileName, errno, strerror(errno));
				return -1;
			} else {
				memset ( g_pstSCEInfo, 0x00 ,sizeof(SFM_SCE) );

			}
		}
	}
	return 0;
}

/* 2009.04.16 by dhkim */
int scem_get_snmp_rdr_ipaddress(char *fname)
{
	char    getBuf[256],IPAddr[20];
	int     IPcnt=0,lNum, i, j;
	FILE    *fp;

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[scem_get_snmp_rdr_ipaddress] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
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
			fprintf(stderr,"[scem_get_snmp_rdr_ipaddress] Wrong RDR IP Address %s\n", getBuf);
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

