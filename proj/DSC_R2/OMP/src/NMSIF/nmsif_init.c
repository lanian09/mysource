#include <nmsif_proto.h>
#include <nmsif.h>
#include <omp_filepath.h>

int	nmsifQid, ixpcQid;
int	MY_NE_ID, STAT_ID_BASE, OID_BASE;
int	mySysEntry;
char	mySysName[20], myAppName[20];
char   	traceBuf[4096];

ListenInfo	ne_info;
OidInfo		oid_info[NMS_STAT_NUM];

extern	HoldFList	hold_list[FILE_NUM_5MIN+FILE_NUM_HOUR];

extern  SFM_sfdb	*sfdb;

extern	MYSQL		*conn, sql;
extern	fd_set		rfd_set, wfd_set, efd_set;
extern	int     	trcFlag, trcLogId, trcErrLogId, trcLogFlag;

char    ver[8] = "R2.0.0"; // BEFORE: R1.2.0 (2011-03-02) -> R2.0.0 (2011-05-05)

init ()
{
	char	*env, tmp[80], fname[200];
	int		key;

	/*
	if (nmsif_initLog () < 0)
		return -1;
	*/

	/* get environment variable (MY_SYS_NAME) */
	if ((env = getenv (MY_SYS_NAME)) == NULL) {
		sprintf (traceBuf, "not found %s env_variable\n", MY_SYS_NAME);
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	strcpy (mySysName, env);

	strcpy (myAppName, "NMSIF");
//  main 에서 nmsif_setupSignals(NULL) 호출 
//	commlib_setupSignals (NULL);

	if (nmsif_initLog () < 0)
		return -1;

	/* get environment variable (IV_HOME) */
	if ((env = getenv (IV_HOME)) == NULL) {
		sprintf (traceBuf, "not found %s env_variable\n", IV_HOME);
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	/* get environment variable (MY_SYS_ENTRY) */
	if ((env = getenv (MY_SYS_ENTRY)) == NULL) {
		sprintf (traceBuf, "not found %s env_variable\n", MY_SYS_ENTRY);
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	mySysEntry = atoi (env);

	if(set_proc_version(OMP_VER_INX_NMSIF, ver) < 0){
		sprintf (traceBuf, "setting process version failed\n");
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	/* get rx message queue id */
	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "NMSIF", 1, tmp) < 0) {
		sprintf (traceBuf, "fail get keyword (nmsif)\n");
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	key = strtol (tmp, 0, 0);
	if ((nmsifQid = msgget (key, IPC_CREAT|0666)) < 0) {
		sprintf (traceBuf, "rx msgget fail : %s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	/* get tx message queue id */
	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "IXPC", 1, tmp) < 0) {
		sprintf (traceBuf, "fail get keyword (ixpc)\n");
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	key = strtol (tmp, 0, 0);
	if ((ixpcQid = msgget (key, IPC_CREAT|0666)) < 0) {
		sprintf (traceBuf, "[*] tx msgget fail : %s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	/* get shared memory pointer */
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_SFDB", 1, tmp) < 0) {
		sprintf (traceBuf, "[*] fail get keyword (shm_sfdb)\n");
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	key = strtol (tmp, 0, 0);

	if (get_nms_con (key) < 0)
		return -1;

	/* get listen ipaddr */
	if (get_listen_ipaddr (fname) < 0)
		return -1;


	/* get listen port for NMS I/F ********************************
	 * -----------------------------------------------------------
	 *            Port   Purpose             Direction (DSC<->NMS)
	 * -----------------------------------------------------------
	 *  Console   5021   Alarm/Fault/Status    ->
	 *  Alarm     5023   Current Alarm        <->
	 *  Perform   6110   Statistics Data       ->
	 *  Config    5041   H/W, S/W Config      <->
	 *  MMC       5061   MMC In/Output        <->
	 **************************************************************/
	if (conflib_getNthTokenInFileSection (fname, "SOCKET_PORT", "NMS_ALM", 1, tmp) < 0) {
		sprintf (traceBuf, "fail get keyword (socket/alm_port)\n");
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	ne_info.port[PORT_IDX_ALM] = strtol (tmp, 0, 0);

	FD_ZERO (&rfd_set);
	FD_ZERO (&wfd_set);
	FD_ZERO (&efd_set);

	if (init_listen (PORT_IDX_ALM) < 0)
		return -1;


	if (conflib_getNthTokenInFileSection (fname, "SOCKET_PORT", "NMS_CONS", 1, tmp) < 0) {
		sprintf (traceBuf, "fail get keyword (socket/console_port)\n");
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	ne_info.port[PORT_IDX_CONS] = strtol (tmp, 0, 0);

	if (init_listen (PORT_IDX_CONS) < 0)
		return -1;


	if (conflib_getNthTokenInFileSection (fname, "SOCKET_PORT", "NMS_CONF", 1, tmp) < 0) {
		sprintf (traceBuf, "fail get keyword (socket/config_port)\n");
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	ne_info.port[PORT_IDX_CONF] = strtol (tmp, 0, 0);

	if (init_listen (PORT_IDX_CONF) < 0)
		return -1;

	if (conflib_getNthTokenInFileSection (fname, "SOCKET_PORT", "NMS_MMC", 1, tmp) < 0) {
		sprintf (traceBuf, "fail get keyword (socket/mmc_port)\n");
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	ne_info.port[PORT_IDX_MMC] = strtol (tmp, 0, 0);

	if (init_listen (PORT_IDX_MMC) < 0)
		return -1;

	if (conflib_getNthTokenInFileSection (fname, "SOCKET_PORT", "NMS_STAT", 1, tmp) < 0) {
		sprintf (traceBuf, "fail get keyword (socket/stat_port)\n");
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	ne_info.port[PORT_IDX_STAT] = strtol (tmp, 0, 0);

	if (init_oid_info () < 0)
		return -1;

	if (init_listen (PORT_IDX_STAT) < 0)
		return -1;

	if (keepalivelib_init (myAppName) < 0)
		return -1;

	if (init_mysql () < 0)
		return -1;

	memset (&hold_list, 0, sizeof (HoldFList)*(FILE_NUM_5MIN+FILE_NUM_HOUR));

	return 1;

} /* End of init () */


get_nms_con (int s_key)
{
	int		shmid;

	if ((shmid = shmget (s_key, sizeof (SFM_sfdb), 0666)) < 0) {
		if (errno != ENOENT) {
			sprintf (traceBuf, "fail shmget :%s\n", strerror (errno));
			trclib_writeLogErr (FL, traceBuf);
			return -1;
		}
		if ((shmid = shmget (s_key, sizeof (SFM_sfdb), IPC_CREAT|0666)) < 0) {
			sprintf (traceBuf, "fail shmget :%s\n", strerror (errno));
			trclib_writeLogErr (FL, traceBuf);
			return -1;
		}
	}

	if ((sfdb = (SFM_sfdb *)shmat (shmid, 0, 0)) == (SFM_sfdb *)-1) {
		sprintf (traceBuf, "fail shmat :%s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	memset (&(sfdb->nmsInfo.fd[0]), 0, sizeof (AppFdTbl) - (sizeof(char) * (MAX_NMS_CON * 2)));

	return 1;

} /* End of get_nms_con () */


init_mysql ()
{
	if( conn != NULL )
		return 0;

	mysql_init (&sql);

	if ((conn = mysql_real_connect (&sql, "localhost", "root", "mysql",
								STM_STATISTIC_DB_NAME, 0, 0, 0)) == NULL) {
		sprintf (traceBuf, "fail mysql_connect. err=%d:%s\n", mysql_errno(&sql), mysql_error(&sql));
		trclib_writeLogErr (FL, traceBuf);
		sprintf (traceBuf, "nmsif terminate.\n"); trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	return 1;

} /* End of init_mysql () */

nmsif_mysql_query(MYSQL *conn, char *query)
{
	if(mysql_query(conn, query) != 0){
		sprintf(traceBuf, "mysql_query fail: err=%d:%s\n query=%s\n", mysql_errno(conn), mysql_error(conn), query);
		trclib_writeLogErr(FL, traceBuf);
		return -1;
		//sprintf(traceBuf, "nmsif terminate.\n"); trclib_writeLogErr(FL, traceBuf);
		//exit(-1);
	}
	return 0;
}


get_listen_ipaddr (char *f_name)
{
	int		ret=0;
	int		lnum=0, sys_cnt=0;
	char	lbuf[200];
	char	token[6][20];
	FILE	*fp;

	if ((fp = fopen (f_name, "r")) == NULL) {
		sprintf (traceBuf, "can't open file (%s) :%s\n", f_name, strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	if ((lnum = conflib_seekSection (fp,"ASSOCIATE_SYSTEMS")) < 0) {
		sprintf (traceBuf, "not found keyword (associate_system)\n");
		trclib_writeLogErr (FL, traceBuf);
		fclose(fp);
		return -1;
	}

	while ((fgets (lbuf, 200, fp) != NULL) &&
			(sys_cnt < SYSCONF_MAX_ASSO_SYS_NUM)) {

		lnum++;

		/* end of section */
		if (lbuf[0] == '[')
			break;

		/* comment line or empty */
		if (lbuf[0] == '#' || lbuf[0] == '\n')
			continue;

		/* find local my system name */
		if (!strncmp (mySysName, lbuf, strlen (mySysName))) {

			ret = sscanf (lbuf, "%s%s%s%s%s%s", token[0], token[1], token[2],
						token[3], token[4], token[5]);

			if (ret < 6) {
				sprintf (traceBuf, "syntax error line (%s)\n", lbuf);
				trclib_writeLogErr (FL, traceBuf);
				if(fp) fclose(fp);
				return -1;
			}
			memset (&ne_info, 0, sizeof (ListenInfo));
			strcpy (ne_info.ipaddr[0], token[4]);
			strcpy (ne_info.ipaddr[1], token[5]);
		}
	}
	if(fp) fclose(fp);
	return 1;

} /* End of get_listen_ipaddr () */


init_oid_info ()
{
	int		cnt=0, ret=0, i;
	char	ibuf[200];
	char	finame[200];
	char	tp[5][40];
	char	tmp[40];
	char	*env;
	FILE	*fptr;

	memset (&oid_info, 0, sizeof (OidInfo)*NMS_STAT_NUM);

	if ((env = getenv (IV_HOME)) == NULL) {
		sprintf (traceBuf, "not found %s env_variable\n", IV_HOME);
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	sprintf (finame, "%s/%s", env, SYSCONF_FILE);

	/* NE_ID (Network Element ID) = DSCM Identifier */
	if (conflib_getNthTokenInFileSection (finame, "NMS_INFO", "NE_ID", 1, tmp) < 0) {
		sprintf (traceBuf, "fail get keyword (transform/ne_id)\n");
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	MY_NE_ID = atoi (tmp);

	/* STATID_BASE = DSCM 통계 식별자 시작 번호 */
	if (conflib_getNthTokenInFileSection (finame, "NMS_INFO", "STATID_BASE", 1, tmp) < 0) {
		sprintf (traceBuf, "fail get keyword (transform/stat_id)\n");
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	STAT_ID_BASE = atoi (tmp);

	/* SID_BASE = SID(통계 출력 파라미터 식별자) 시작 번호 */
	if (conflib_getNthTokenInFileSection (finame, "NMS_INFO", "SID_BASE", 1, tmp) < 0) {
		sprintf (traceBuf, "fail get keyword (transform/oid_base)\n");
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	OID_BASE = atoi (tmp);


	sprintf (finame, "%s/%s", env, OIDCONF_FILE);

	/* ~/DATA/oidconfig 화일을 open한다. */
	if ((fptr = fopen (finame, "r")) == NULL) {
		sprintf (traceBuf, "can't open file (%s) : %s\n", finame, strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	while(fgets(ibuf, 200, fptr) != NULL)
	{
		if(ibuf[0] == '#')
			continue;

		/*	tp[0]: 5분 통계 DB 테이블명
		*	tp[1]: 1시간 통계 DB 테이블명
		*	tp[2]: 해당 통계 sid 시작번호
		*	tp[3]: 통계 식별자
		*/
		ret = sscanf(ibuf, "%s %s %s %s", tp[0], tp[1], tp[2], tp[3]);

		/* NULL로 표시된 경우 해당 5분 통계 처리 skip */
		if(strcmp(tp[0], "NULL"))
			strcpy((char *)&oid_info[cnt].cTableName[0], tp[0]);

		/* NULL로 표시된 경우 해당 1시간 통계 처리 skip */
		if(strcmp(tp[1], "NULL"))
			strcpy((char *)&oid_info[cnt].cTableName[1], tp[1]);

		oid_info[cnt].dSidFirstNum	= atoi(tp[2]);
		oid_info[cnt].dObjectID		= atoi(tp[3]);

		cnt++;
	}
	if(fptr) fclose(fptr);

	{
		int		k;
		for(k = 0; k < cnt; k++)
		{
			printf("[%d] db1=(%s)\n", k, oid_info[k].cTableName[0]);
			printf("     db2=(%s)\n", oid_info[k].cTableName[1]);
		}
	}
	return 1;
}


nmsif_initLog ()
{
    char    *env, fname[256];

    if ((env = getenv(IV_HOME)) == NULL) {
        fprintf(stderr,"[stmd_initLog] not found %s environment name\n", IV_HOME);
        return -1;
    }

    sprintf(fname,"%s/%s.%s", env, NMSIF_TRCLOG_FILE, mySysName);
    if ((trcLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[stmd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    sprintf(fname,"%s/%s.%s", env, NMSIF_ERRLOG_FILE, mySysName);
    if ((trcErrLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[stmd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    return 1;

} /* End of nmsif_initLog () */
