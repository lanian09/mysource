#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include "ifb_proto.h"

#define PROC_NAME_LEN	128
#define MAXLINE			1024

int	displayQkeyFlag=0;
IFB_ProcInfoContext	confProcTbl[SYSCONF_MAX_APPL_NUM];
int pCnt = 0;
char trcBuf[1024];
SFM_SysCommMsgType   *loc_sadb;

// by sjjeon
int getProcessID(char *procName);
int getPidInfo(int pid, char *sttime);


/*
VersionIndexTable vit[] = {
    {"IXPC",     SEQ_PROC_IXPC},
    {"SAMD",     SEQ_PROC_SAMD},
    {"MMCR",     SEQ_PROC_MMCR},
    {"STMM",     SEQ_PROC_STMM},
    {"CAPD",     SEQ_PROC_CAPD},
    {"ANA",      SEQ_PROC_ANA},
    {"CDR",      SEQ_PROC_CDR},
    {"TRCDR",    SEQ_PROC_SESSANA},
    {"WAP1ANA",  SEQ_PROC_WAP1ANA},
    {"UAWAPANA", SEQ_PROC_UAWAPANA},
    {"WAP2ANA",  SEQ_PROC_MESVC},
    {"HTTPANA",  SEQ_PROC_KUNSVC},
    {"UDRGEN",   SEQ_PROC_UDRGEN},
    {"AAAIF",    SEQ_PROC_AAAIF},
    {"SDMD",     SEQ_PROC_SDMD},
    {"LOGM",     SEQ_PROC_LOGM},
    {"VODSANA",  SEQ_PROC_VODSANA},
    {"WIPINWANA",  SEQ_PROC_WIPINWANA},
    {"JAVANWANA",   SEQ_PROC_KVMANA},
	{"VTANA", SEQ_PROC_VTANA},
	{"CDR2", SEQ_PROC_CDR2},
	{NULL, -1}
};
*/

VersionIndexTable vit[] = {
	{"IXPC",	SEQ_PROC_IXPC},
	{"SAMD",	SEQ_PROC_SAMD},
	{"MMCR",	SEQ_PROC_MMCR},
//	{"STMM",	SEQ_PROC_STMM},
	{"CAPD",	SEQ_PROC_CAPD},
	{"PANA",	SEQ_PROC_ANA},
//	{"CDR",		SEQ_PROC_CDR},
	{"RLEG0",	SEQ_PROC_SESSANA0},
	// added by dcham 20110530 for SM connection 축소(5=>1)
//	{"RLEG1",	SEQ_PROC_SESSANA1},
//	{"RLEG2",	SEQ_PROC_SESSANA2},
//	{"RLEG3",	SEQ_PROC_SESSANA3},
//	{"RLEG4",	SEQ_PROC_SESSANA4},
//	{"WAP1ANA",	SEQ_PROC_WAP1ANA},
//	{"UAWAPANA",SEQ_PROC_UAWAPANA},
//	{"WAP2ANA",  SEQ_PROC_MESVC},
	{"RDRCAPD",	SEQ_PROC_RDRCAPD}, 	/* by yhshin,  2009.04.26 */
//	{"HTTPANA",  SEQ_PROC_KUNSVC},
//	{"VODSANA",  SEQ_PROC_VODSANA},
//	{"WIPINWANA",  SEQ_PROC_WIPINWANA},
//	{"JAVANWANA",   SEQ_PROC_KVMANA},
//	{"REANA",	SEQ_PROC_REANA},
//	{"CDR2", SEQ_PROC_CDR2},
//	{"VTANA", SEQ_PROC_VTANA},
//	{"UDRGEN",   SEQ_PROC_UDRGEN},
//	{"AAAIF",    SEQ_PROC_AAAIF},
//	{"SDMD",     SEQ_PROC_SDMD},
//	{"LOGM",     SEQ_PROC_LOGM},
	{"RANA",	SEQ_PROC_RANA},
//	{"FBANA",	SEQ_PROC_FBANA},
//	{"MEMD",	SEQ_PROC_MEM},
	{"SMPP",	SEQ_PROC_SMPP}, 	/* by june,  2009.04.26 */
	{"RDRANA",	SEQ_PROC_RDRANA}, 	// ADD by jjinri 2009.04.24
	{NULL, -1}
};

#ifndef __LINUX__
//------------------------------------------------------------------------------
// 관리대상 프로세스인지 확인하고 해당 프로세스의 index를 return한다.
//------------------------------------------------------------------------------
int getListID(prpsinfo_t *ps_info);
#endif

int getCM_Pid(void);
int smLiveCheck();    /* SM process live check */
int cmLiveCheck();    /* CM process live check */

//------------------------------------------------------------------------------
int ifb_checkLogin (void)
{
	char	getBuf[256], *ptr, *next, *acc = NULL, *uid;
	char	opAccount[32], login_uid[32];
	FILE	*fp;

	// config file에서 운용계정(operator account)을 읽는다.
	//
	if (ifb_getOpAccount (opAccount) < 0)
		return -1;

	// 현재 계정을 확인한다.
	//
	sprintf(login_uid,"%d",(int)getuid());

	if ((fp = fopen(PASSWD_FILE,"r")) == NULL) {
		fprintf(stderr,"\n fopen fail[%s]; err=%d(%s)\n\n", PASSWD_FILE, errno, strerror(errno));
		return -1;
	}
	while (fgets(getBuf,sizeof(getBuf),fp) != NULL) {
		ptr = getBuf;
		acc = (char*)strtok_r(ptr,":",&next);
		ptr = next;
		strtok_r(ptr,":",&next);
		ptr = next;
		uid = (char*)strtok_r(ptr,":",&next);
		if (!strcmp (uid, login_uid))
			break;
	}
	fclose(fp);

	if (strcmp (opAccount, acc)) {
		fprintf(stderr,"\n not operator account(%s); operator is %s\n\n", acc, opAccount);
		return -1;
	}

	return 1;

} //----- End of ifb_checkLogin -----//


//------------------------------------------------------------------------------
// config file에서 운용계정(operator account)을 읽는다.
//------------------------------------------------------------------------------
int ifb_getOpAccount (char *opAccount)
{
	char	*env, fname[256];

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"\n not found %s environment name\n\n", IV_HOME);
		return -1;
	}
	sprintf(fname,"%s/%s",env,SYSCONF_FILE);
	if (conflib_getNthTokenInFileSection (fname, "GENERAL", "OP_ACCOUNT", 1, opAccount) < 0) {
		return -1;
	}

	return 1;

} //----- End of ifb_getOpAccount -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ifb_setConfProcTbl (void)
{
	char	*env, fname[256], getBuf[256], token[8][64];
	int		lNum=0, procCnt=0;
	FILE	*fp;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"\n not found %s environment name\n\n", IV_HOME);
		return -1;
	}

	sprintf(fname,"%s/%s",env,SYSCONF_FILE);

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"\n fopen fail[%s]; err=%d(%s)\n\n", fname, errno, strerror(errno));
		return -1;
	}

	// [APPLICATIONS] section으로 이동
	if ((lNum = conflib_seekSection(fp,"APPLICATIONS")) < 0) {
		fprintf(stderr,"\n not found section[APPLICATIONS] in file[%s]\n\n", fname);
		return -1;
	}

	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(procCnt < SYSCONF_MAX_APPL_NUM) ) {
		lNum++;
		if (getBuf[0] == '[') // end of section
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') // comment line or empty
			continue;

		// config 파일에 "name = Qkey 실행파일위치" format으로 들어있다.
		if (sscanf(getBuf,"%s%s%s%s",token[0],token[1],token[2],token[3]) < 4) {
			fprintf(stderr,"[ixpc_initMsgQRoutTbl] syntax error; file=%s, lNum=%d\n", fname, lNum);
			return -1;
		}
		strcpy (confProcTbl[procCnt].procName, token[0]);
		confProcTbl[procCnt].msgQkey = strtol(token[2],0,0);
		if(!strcasecmp(confProcTbl[procCnt].procName, "CM") || !strcasecmp(confProcTbl[procCnt].procName, "SMSERVER"))
			sprintf(confProcTbl[procCnt++].exeName, "%s", token[3]);
		else
			sprintf(confProcTbl[procCnt++].exeName, "%s/%s", env, token[3]);
	}
	fclose(fp);
	pCnt = procCnt;
	return 1;

} //----- End of ifb_setConfProcTbl -----//


//------------------------------------------------------------------------------
int ifb_getProcIndex (char *name)
{
	int		i;

	for(i = 0; i < pCnt; i++)
	{
		if(!strcasecmp(confProcTbl[i].procName, name))
			return i;
	}

	return -1;
} //----- End of ifb_getProcIndex -----//


int getProcParents(int ppid)
{
	int		i;

	for(i = 0;i<SYSCONF_MAX_APPL_NUM; i++)
	{
		if(confProcTbl[i].pid == ppid)
			return confProcTbl[i].pid;
	}

	return 0;
}

//------------------------------------------------------------------------------
#ifndef __LINUX__
int getPPID(int pid)
{
	char		fname[256];
	int			fd;
	prpsinfo_t	psInfo;

	sprintf(fname, "%s/%d", PROC_DIR, pid);
	if( (fd = open(fname, O_RDONLY)) < 0)
	{
		if( (errno != EACCES) && (errno != ENOENT))
			fprintf(stderr, "[%s] open fail; (%s) err=%d[%s]\n", __FUNCTION__, fname, errno, strerror(errno));	/*	Permission denied 는 skip하기 위해	*/

		return -1;
	}

	if(ioctl(fd, PIOCPSINFO, &psInfo) < 0)
	{
		fprintf(stderr, "[%s] ioctl err=%d[%s]\n", __FUNCTION__, errno, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return psInfo.pr_ppid;
}

//------------------------------------------------------------------------------
// 관리대상 프로세스인지 확인하고 해당 프로세스의 index를 return한다.
//------------------------------------------------------------------------------
int getListID(prpsinfo_t *ps_info)
{
	int		i;
    char    *DAppArgs, *DAppToken, tmpPSArgs[PRARGSZ];

    DAppToken   = NULL;
    DAppArgs    = NULL;

    memset(tmpPSArgs, 0x00, PRARGSZ);
    strcpy(tmpPSArgs, ps_info->pr_psargs);
    DAppArgs    = strstr(tmpPSArgs, "DAPP");
	for(i = 0; i < pCnt; i++)
	{
	    if(DAppArgs)
	    {
	        if( (DAppToken = strchr(DAppArgs, '=')) == NULL)
	        {
			#ifdef DEBUG
				fprintf(stderr, "[%s] check args(%s)\n", __FUNCTION__, DAppArgs);
			#endif
				return -1;
	        }
			else
				DAppToken++;

	        while( (char)*DAppToken == ' ')
	            DAppToken++;

	        strtok(DAppToken, " ");
	        if( !strcasecmp(DAppToken, confProcTbl[i].procName))
	        {
			#if 0
	            fprintf(stderr, "[%s] %s find\n", __FUNCTION__, DAppToken);
			#endif
				return i;
	        }
			else
			{
			#if 0
	            fprintf(stderr, "[%s] %s NOT find\n", __FUNCTION__, DAppToken);
			#endif
			}
	    }
		else if(!strcasecmp(ps_info->pr_fname, confProcTbl[i].procName))
			return i;
	}

	return -1;
}

int ifb_getProcStatus(int type)
{
	int				fd, listID;
	DIR				*dirp;
	char			pathName[256], *ver;
	struct dirent	*direntp;
	prpsinfo_t		psInfo;
	size_t			dLength_ProcName;

	if( (dirp = opendir(PROC_DIR)) == (DIR *)NULL)
	{
		fprintf(stderr,"\n opendir fail[%s]; err=%d(%s)\n\n", PROC_DIR, errno, strerror(errno));
		return -1;
	}

	while( (direntp = readdir(dirp)) != NULL)
	{
		if(!strcasecmp(direntp->d_name, PARENT_PATH) || !strcasecmp(direntp->d_name, HOME_PATH))
			continue;

		sprintf(pathName, "%s/%s", PROC_DIR, direntp->d_name);

		if( (fd = open(pathName, O_RDONLY)) < 0)
		{
			if( (errno != EACCES) && (errno != ENOENT))
				fprintf(stderr, "[%s] open fail; (%s) err=%d[%s]\n", __FUNCTION__, pathName, errno, strerror(errno));	/*	Permission denied 는 skip하기 위해	*/

			continue;
		}

		if(ioctl(fd, PIOCPSINFO, &psInfo) < 0)
		{
			fprintf(stderr, "[%s] ioctl err=%d[%s]\n", __FUNCTION__, errno, strerror(errno));
			close(fd);
			continue;
		}
		close(fd);

		if(strlen(psInfo.pr_psargs) == 0)
			continue;

		/*	관리대상 프로세스인지 확인하고, 내부적으로 관리하는 해당 프로세스의 index가 return한다.	*/
		if( (listID = getListID(&psInfo)) < 0)
			continue;

		confProcTbl[listID].runCnt++;
		confProcTbl[listID].pid						= psInfo.pr_pid;

		if(strcasecmp(confProcTbl[listID].procName, "CM") && strcasecmp(confProcTbl[listID].procName, "SMSERVER"))
		{
			dLength_ProcName = strlen(psInfo.pr_fname);
			if(dLength_ProcName < 16)
				strncpy(confProcTbl[listID].procName, psInfo.pr_fname, 16);
			else
			{
				strncpy(confProcTbl[listID].procName, psInfo.pr_fname, 15);
				confProcTbl[listID].procName[16] = 0x00;
			}
		}

		strftime(confProcTbl[listID].startTime, 32, "%m-%d %H:%M", localtime((time_t*)&(psInfo.pr_start.tv_sec)));
	#if 0
		loc_sadb->loc_process_sts[listID].uptime = htons(psInfo.pr_start.tv_sec);
		printf("uptime: %d\n", psInfo.pr_start.tv_sec);
	#endif
		ver = get_ver_str(confProcTbl[listID].procName);
		if(!ver)
		{
			fprintf(stderr, "It fails to get version information -[%s].\n", confProcTbl[listID].procName);
			strncpy(confProcTbl[listID].procVersion, "UNKNOWN", 10);
		}
		else
		{
			if(strlen(ver) == 0)
				strncpy(confProcTbl[listID].procVersion, "UNKNOWN", 10);
			else
				strncpy(confProcTbl[listID].procVersion, ver, 10);
		}
	}
	closedir(dirp);

	return 1;
}

pid_t ifb_getPid(char *procName)
{
	char			pname[PROC_NAME_LEN], *DAppArgs, *DAppToken, tmpPSArgs[PRARGSZ];
	int				fd;
	DIR				*dirp;
	prpsinfo_t		psInfo;
	struct dirent	*direntp;

	if( (dirp = opendir(PROC_DIR)) == NULL)
	{
		fprintf(stderr,"\n opendir fail[%s]; err=%d(%s)\n\n", PROC_DIR, errno, strerror(errno));
		return -1;
	}

	while( (direntp = readdir(dirp)) != NULL)
	{
		if(!strcasecmp(direntp->d_name, PARENT_PATH) || !strcasecmp(direntp->d_name, HOME_PATH))
			continue;

		sprintf(pname, "%s/%s", PROC_DIR, direntp->d_name);
		if( (fd = open(pname, O_RDONLY)) < 0)
		{
			if( (errno != EACCES) && (errno != ENOENT))
				fprintf(stderr, "[%s] open fail; (%s) err=%d[%s]\n", __FUNCTION__, pname, errno, strerror(errno));	/*	Permission denied 는 skip하기 위해	*/

			continue;
		}

		if(ioctl(fd, PIOCPSINFO, &psInfo) < 0)
		{
			fprintf(stderr, "[%s] ioctl err=%d[%s]\n", __FUNCTION__, errno, strerror(errno));
			close(fd);
			continue;
		}
		close(fd);

		if(strlen(psInfo.pr_psargs) == 0)
			continue;

		/*	관리대상 프로세스인지 확인하고, 내부적으로 관리하는 해당 프로세스의 index가 return한다.	*/
	    DAppToken   = NULL;
	    DAppArgs    = NULL;
	    memset(tmpPSArgs, 0x00, PRARGSZ);
	    strcpy(tmpPSArgs, psInfo.pr_psargs);
	    DAppArgs    = strstr(tmpPSArgs, "DAPP");
	    if(DAppArgs)
	    {
	        if( (DAppToken = strchr(DAppArgs, '=')) == NULL)
	        {
			#ifdef DEBUG
				fprintf(stderr, "[%s] check args(%s)\n", __FUNCTION__, DAppArgs);
			#endif
				continue;
	        }
			else
				DAppToken++;

	        while( (char)*DAppToken == ' ')
	            DAppToken++;

	        strtok(DAppToken, " ");
	        if( !strcasecmp(DAppToken, procName))
	        {
			#if 0
	            fprintf(stderr, "[%s] %s find\n", __FUNCTION__, DAppToken);
			#endif
				closedir(dirp);
				return psInfo.pr_pid;
	        }
	    }
		else if(!strcasecmp(psInfo.pr_fname, procName))
		{
			closedir(dirp);
			return psInfo.pr_pid;
		}
	}
	closedir(dirp);

	return -1;
} //----- End of ifb_getPid -----//
#else
int getPPID(int pid)
{
	FILE	*fp;
	char	buf[256], token[3][30], fname[256];
	int		i;

	sprintf(fname, "/proc/%d/status", pid);
	if( (fp = fopen(fname,"r")) == NULL)
	{
		fprintf(stderr, "[%s] fopen(%s) fail\n", __FUNCTION__, fname);
		return -1;
	}

	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		i = 0;
		while( (buf[i] == ' ') && (buf[i] != '\n'))
			i++;

		if( (buf[i] == '#') || (buf[i] == '\n'))
			continue;

		sscanf(buf, "%s %s %s", token[0], token[1], token[2]);

		if(!strcasecmp(token[0], "PPid:"))
		{
		//	printf("ppid = %s\n", token[1]);
			fclose(fp);
			return atoi(token[1]);
		}
	}
	fclose(fp);

	return -1;
}

int ifb_getProcStatus(int type)
{
	char			*dname, *ver, fname[100], pname[PROC_NAME_LEN];
	int				fd, status, pid, procIndex;
	DIR				*dirp;
	time_t			sysuptime;
	struct dirent	*direntp;
	struct stat		st;

#if 0
	if(get_system_uptime(&sysuptime))
		return -1;
#endif

	if( (dirp = opendir(PROC_DIR)) == NULL)
	{
		fprintf(stderr, "\n opendir fail[%s]; err=%d(%s)\n\n", PROC_DIR, errno, strerror(errno));
		return -1;
	}

	while( (direntp = readdir(dirp)) != NULL)
	{
		dname = direntp->d_name;
		if(!isdigit(*dname))
			continue;
		pid = atoi(dname);

		sprintf(fname, "/proc/%d/cmdline", pid);

		/* get the process owner */
		if( (status = stat(fname, &st)) != 0)
			continue;

		if( (fd = open(fname, O_RDONLY)) < 0)
			continue;
		else
		{
			memset(pname, 0x00, PROC_NAME_LEN);
			if(read(fd, pname, PROC_NAME_LEN-1) < 0)
			{
				close(fd);
				continue;
			}
			else
			{
				close(fd);
				if( (procIndex = ifb_getProcIndex (pname)) < 0)
					continue;

				if(confProcTbl[procIndex].runCnt > 0)
				{
					if(getPPID(pid) != 1)
						continue;
				}
				confProcTbl[procIndex].runCnt++;
				confProcTbl[procIndex].pid = pid;

				strcpy(confProcTbl[procIndex].startTime, "-");
			#if 0
				get_proc_starttime(pid, sysuptime, NULL, confProcTbl[procIndex].startTime);
				strftime (confProcTbl[procIndex].startTime, 32, "%m-%d %H:%M",localtime((time_t*)&(st.st_atime)));
			#endif

				if( (ver = get_ver_str(pname)) != NULL)
				{
					fprintf(stderr, "It fails to get version information -[%s].\n", pname);
					strncpy(confProcTbl[procIndex].procVersion, "UNKNOWN", 10);
				}
				else
				{
					if(strlen(ver) == 0)
						strncpy(confProcTbl[procIndex].procVersion, "UNKNOWN", 10);
					else
						strncpy(confProcTbl[procIndex].procVersion, ver, 10);
				}
			}
		}
	}
	closedir(dirp);

	return 1;
} //----- End of ifb_getProcStatus -----//

pid_t ifb_getPid (char *procName)
{
	char			*dname, fname[100], pname[PROC_NAME_LEN];
	int				fd, status, pid;
	DIR				*dirp;
	struct stat		st;
	struct dirent	*direntp;

	if( (dirp = opendir(PROC_DIR)) == NULL)
	{
		fprintf(stderr,"\n opendir fail[%s]; err=%d(%s)\n\n", PROC_DIR, errno, strerror(errno));
		return -1;
	}

	while( (direntp = readdir(dirp)) != NULL)
	{
		dname = direntp->d_name;
		if(!isdigit(*dname))
			continue;
		pid = atoi(dname);
		sprintf(fname, "/proc/%d/cmdline", pid);

		/* get the process owner */
		if( (status = stat(fname, &st)) != 0)
			continue;

		if( (fd = open(fname, O_RDONLY)) < 0)
			continue;
		else
		{
			memset(pname, 0x00, PROC_NAME_LEN);
			if(read(fd, pname, PROC_NAME_LEN-1) < 0)
			{
				close(fd);
				continue;
			}
			else
			{
				close(fd);

				if(!strcasecmp(procName, pname))
				{
				//	fprintf (stderr,"[SFMD:GET_PROC_ID]%s pid = %d\n",procName, pid);
					closedir(dirp);
					return pid;
				}
			}
		}
	}
	closedir(dirp);

	return -1;
} //----- End of ifb_getPid -----//
#endif

//------------------------------------------------------------------------------
// 지정된 pid의 프로세스가 살아 있는지 확인한다.
// /proc에 해당 pid 이름의 file이나 directory가 있는지만 확인한다.
//------------------------------------------------------------------------------
int ifb_isAlivePid (int pid)
{
	DIR				*dirp;
	struct dirent	*dp;

	if ((dirp = opendir(PROC_DIR)) == NULL) {
		fprintf(stderr,"\n opendir fail[%s]; err=%d(%s)\n\n", PROC_DIR, errno, strerror(errno));
		return 0;
	}

	while ((dp = readdir(dirp)) != NULL) {
		if (pid == atoi(dp->d_name)) {
			closedir(dirp);
			return 1;
		}
	}
	closedir(dirp);

	return 0;

} //----- End of ifb_isAlivePid -----//

char * ifb_getSysOperMode (int mode)
{
	switch (mode)
	{
		case 1:
			return  "ACTIVE";
		case 2:
			return  "STANDBY";
		case 3:
			return  "FAULT";
		default:
			fprintf(stderr, "mode:%d\n", mode);
			return  "UNKNOWN";
	}
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void ifb_printProcStatus (void)
{
	int		i, alive=0, dead=0;
#if 0
	FILE *fp;
	int state;
	char buff[MAXLINE];
	char proc_usr[8], proc_stime[8], proc_tmptime[16];
	int  proc_pid;
	char cmd[] = "ps -ef|grep DAPP=|awk '{ print $1, $2, $5}'";
    char *pos = NULL;
	int  pos_idx=0;
#endif
	if (displayQkeyFlag) {
		fprintf(stderr,"===================================================================================================\n");
		fprintf(stderr," SYSTEM OPERATION MODE : %s\n", ifb_getSysOperMode(loc_sadb->loc_system_dup.myLocalDupStatus));
		fprintf(stderr,"===================================================================================================\n");
		fprintf(stderr," Process    PID      STATUS  MSG_Q_KEY    VERSION  Process    PID      STATUS  MSG_Q_KEY    VERSION  \n");
		fprintf(stderr,"---------------------------------------------------------------------------------------------------");
	} else {
		fprintf(stderr,"===================================================================================================\n");
		fprintf(stderr," SYSTEM OPERATION MODE : %s\n", ifb_getSysOperMode(loc_sadb->loc_system_dup.myLocalDupStatus));
		fprintf(stderr,"===================================================================================================\n");
		fprintf(stderr," Process    PID      STATUS  START-TIME   VERSION  Process    PID      STATUS  START-TIME   VERSION  \n");
		fprintf(stderr,"---------------------------------------------------------------------------------------------------");
	}

	for (i=0; i<SYSCONF_MAX_APPL_NUM; i++)
	{
		if (!strcasecmp (confProcTbl[i].procName, ""))
			break;
		if (i%2==0) fprintf(stderr,"\n");
		if (confProcTbl[i].runCnt) {
			if (displayQkeyFlag) {
				if (confProcTbl[i].runCnt == 1)
					fprintf(stderr," %-10s %-8d ALIVE   0x%-10x %-8s", confProcTbl[i].procName,
							(int)confProcTbl[i].pid, confProcTbl[i].msgQkey, confProcTbl[i].procVersion);
				else
					fprintf(stderr," %-10s %-8d ALIVE(%d)0x%-10x %-8s", confProcTbl[i].procName,
							(int)confProcTbl[i].pid, confProcTbl[i].runCnt, confProcTbl[i].msgQkey, confProcTbl[i].procVersion);
			} else {
				if (confProcTbl[i].runCnt == 1)
					fprintf(stderr," %-10s %-8d ALIVE   %-12s %-8s", confProcTbl[i].procName,
							(int)confProcTbl[i].pid, confProcTbl[i].startTime, confProcTbl[i].procVersion);
				else
					fprintf(stderr," %-10s %-8d ALIVE(%d)%-12s %-8s", confProcTbl[i].procName,
							(int)confProcTbl[i].pid, confProcTbl[i].runCnt, confProcTbl[i].startTime, confProcTbl[i].procVersion);
			}
			alive += confProcTbl[i].runCnt;
		} else {
			if (displayQkeyFlag) {
				fprintf(stderr," %-10s -        DEAD    0x%-10x %-8s", confProcTbl[i].procName,
						confProcTbl[i].msgQkey, confProcTbl[i].procVersion);
			} else {
				fprintf(stderr," %-10s -        DEAD    -     -      %-8s", confProcTbl[i].procName, "-");
			}
			dead++;
		}
	}
#if 1
	/* SM/CM Process Status Management, by sjjeon */
	{
		int sm_pid,cm_pid, rv;
		char startTM[32];
		char version[]="R1.0.0";

		// SM PID, TIME 을 구한다.
		sm_pid = getProcessID("SMSERVER");
 		rv = getPidInfo(sm_pid, startTM);
 		if(rv >= 0){
			fprintf(stderr,"\n %-10s %-8d ALIVE   %-12s %-8s","SM",sm_pid, startTM, version);
			alive += 1;
		}
		else{
			fprintf(stderr,"\n %-10s -        DEAD    -      %-8s","SM", version);
			dead +=1;
 		}

		// CM PID, TIME 을 구한다.
 		memset(startTM,0x00,sizeof(startTM));
 		cm_pid = getProcessID("CM");
 		rv = getPidInfo(cm_pid, startTM);
 		if(rv >= 0){
			fprintf(stderr," %-10s %-8d ALIVE   %-12s %-8s","CM",cm_pid, startTM, version);
     		alive += 1;
 		}
 		else{
			fprintf(stderr," %-10s -        DEAD    -      %-8s","CM", version);
			dead +=1;
		}
	}
	
#else
	/* SM/CM Process Status Management, by june */
	fp = popen(cmd, "r");
	if (fp == NULL) {
		perror("erro : "); exit(0);
	}
	fprintf(stderr,"\n");
	while(fgets(buff, MAXLINE, fp) != NULL)
	{
		if( sscanf( &buff[0], "%s %d %s", proc_usr, &proc_pid, proc_tmptime)==3) {
//////////////////////////////////////////////////////////////////////////////////////////////////
			pos = strrchr(proc_tmptime, ':');
			if (pos != NULL) {
				pos_idx = pos - proc_tmptime;
				proc_tmptime[pos_idx] = '\0';
			}
			sprintf(proc_stime, "%s", proc_tmptime);
//////////////////////////////////////////////////////////////////////////////////////////////////
			if (!strncmp(proc_usr, "pcube", 5)) {
				strncpy(proc_stime, proc_tmptime, 5);
				if (smLiveCheck()==0) {
					fprintf(stderr," %-10s %-8d ALIVE         %-5s  %-8s", "SMSERVER", proc_pid, proc_stime, "R1.0.0");
					alive++;
				} else {
					fprintf(stderr," %-10s %-8d DEAD          %-5s  %-8s", "SMSERVER", proc_pid, proc_stime, "R1.0.0");
					dead++;
				}
			}
			if (!strncmp(proc_usr, "scmscm", 6)) {
				if (cmLiveCheck()==0) {
					fprintf(stderr," %-10s %-8d ALIVE         %-5s  %-8s", "CM", proc_pid, proc_stime, "R1.0.0");
					alive++;
				} else {
					fprintf(stderr," %-10s %-8d DEAD          %-5s  %-8s", "CM", proc_pid, proc_stime, "R1.0.0");
					dead++;
				}
			}
		}
	}

	state = pclose(fp);
#endif
	fprintf(stderr,"\n===================================================================================================\n");
	fprintf(stderr,"TOTAL:%d (ALIVE:%d, DEAD:%d)\n\n", alive+dead, alive, dead);

	return;

} //----- End of ifb_printProcStatus -----//

int findIndex(char *syntax, char *idex)
{           
	char    *token;
	char    search[]=" ";

	if( (syntax == NULL) || (idex == NULL))
		return -1;
	/*첫번째 설정*/
	token = strtok((char*)syntax, (char*)search);
	if(!strncasecmp(token,idex,strlen(idex))) {
		//printf("find OK.. %s\n",token);
		return 1;
	}

	while(1)
	{
		token = strtok(NULL, search);
		if(token == NULL)
			break;
		//printf("%s\n",token);
		if(!strncasecmp(token,idex,strlen(idex))) {                                                                                                                                                  
			//printf("find OK...%s\n",token);                                                                                                              
			return 1;                                                                                                                                      
		}                                                                                                                                                  
	}                                                                                                                                                      
	return 0;                                                                                                                                              
}   


int smLiveCheck()    /* SM process live check : sjjeon */
{
	const int _BUFSIZ = 512;
	char chkFile[]="/DSC/NEW/STATUS/SMLIVE_CHECK";
	char buf[_BUFSIZ];
	FILE *fp=NULL;

	/*SM LIVE CHECKING시에 5초 정도 걸린다. 프로세스로 뺀다.  */
	fp = fopen(chkFile, "r");
	if(fp==NULL) {
		printf("%s: sm check file open fail[%s]\n", __FUNCTION__, chkFile);
		return -1;
	}
	bzero(buf, _BUFSIZ);

	while (fgets(buf,_BUFSIZ, fp) != NULL)
	{
		if(findIndex(buf,"Active")) {
			//printf("sm running.. OK\n");                                                                                                                  
			fclose(fp);
			return 0;
		}
		//printf("%s\n",buf);                                                                                                                              
	} 
	//printf("[%s] SM process not Active.\n",__FUNCTION__ );

	fclose(fp);
	return -1;
}

int cmLiveCheck()    /* CM process live check : sjjeon */
{   
	int pid_t;
	pid_t=getCM_Pid();
	//printf("CM pid : %d\n", pid_t);

	if(pid_t) return 0;
	return 1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ifb_promptYesNo (void)
{
	char	input[256], *ptr;

	fgets(input, sizeof(input), stdin);
	for (ptr=input; isspace((int)*ptr); ptr++) ;
	if (*ptr == 'y' || *ptr == 'Y')
		return 1;
	else
		return 0;
} //----- End of ifb_promptYesNo -----//

int ifb_promptYesNo2 (char choice)
{
     if (choice == 'y' || choice == 'Y')
         return 1;
      else
         return 0;
} //----- End of ifb_promptYesNo -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ifb_killProc (pid_t pid)
{
	if (kill(pid, SIGTERM) < 0) {
#ifdef DEBUG
			fprintf(stderr," Can't kill(KILL-TERM:%d) running process (pid=%d)\n\n", SIGTERM,(int)pid);
			return -1;
#else
		if (kill(pid, SIGKILL) < 0) {
			fprintf(stderr," Can't kill running process (pid=%d)\n\n", (int)pid);
			return -1;
		}
#endif
	}
	return 1;

} //----- End of ifb_killProc -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ifb_clearQ (int qkey, int debug)
{
	char	buf[65536];
	int		qid,cnt=0,ret;

	if ((qid = msgget (qkey, 0)) < 0) {
		if (debug) {
			fprintf(stderr,"msgget fail; qkey=0x%x, err=%d(%s)\n", qkey, errno, strerror(errno));
		}
		return -1;
	}
	while (1) {
		if ((ret = msgrcv(qid, (void*)buf, sizeof(buf), 0, IPC_NOWAIT)) < 0) {
			if (debug && errno != ENOMSG) {
				fprintf(stderr,"msgget fail; qkey=0x%x, err=%d(%s)\n", qkey, errno, strerror(errno));
			}
			break;
		}
		if (debug) {
			;
			//fprintf(stderr,"msgrcv len=%d\n", ret);
		}
		cnt++;
	}
	return cnt;
} //----- End of ifb_clearQ -----//

char *get_ver_str(char *procname)
{
	int			i;
	static char	ver[10];

	memset(ver, 0x00, sizeof(ver));
	if(!strcasecmp(procname, "CM") || !strcasecmp(procname, "SMSERVER"))
	{
		strcpy(ver, "-");
		return ver;
	}

	for(i = 0; vit[i].name != NULL; i++)
	{
		if(!strncasecmp(procname, vit[i].name, strlen(vit[i].name)))
		{
			get_version(vit[i].index, ver);
			if(strlen(ver) < 1)
				return NULL;

			return ver;
		}
	}

	return NULL;
}

int check_user_valid(FILE *fp, char *username, char *password)
{
        char    line[1024], *lasts, *tok;
        int     match;

        rewind(fp);

        match = 0;

        while(fgets(line, sizeof(line), fp)){
                tok = strtok_r(line, ":", &lasts);
                if(strcmp(tok, username)){
                        continue;
                }

                tok = strtok_r(NULL, ":", &lasts);
                if(!strcmp(tok, (char *)crypt(password, password))){
                        match = 1;
                }
        }

        if(match)
                return 1;
        else
                return 0;
}

int interact_w(char *ivhome)
{

	int     i;
	FILE    *fp;
	char    username[50], password[50];
	char    passfile[100];

	sprintf(passfile, "%s/%s", ivhome, PASSWORD_FILE);
	fp = fopen(passfile, "r+");
	if(fp == NULL){
		fprintf(stderr, "Cannot open the password file\n");
		exit(1);
	}

	for(i = 0; i < RETRY_COUNT; i++){
		fprintf(stderr, "user id :");
		scanf("%s", username);
		if(username[strlen(username)-1] == '\n')
			username[strlen(username)-1] = 0x00;
		fprintf(stderr, "password :");
		system("stty -echo");
		scanf("%s", password);
		system("stty echo");
		fprintf(stderr, "\n");
		if(password[strlen(password)-1] == '\n')
			password[strlen(password)-1] = 0x00;

		if(check_user_valid(fp, username, password)){
			return 1;
		}
	}

	fclose(fp);

	if(i >= RETRY_COUNT)
		return -1;

	return -1;
}

int getCM_Pid(void)
{
	int				fd;
	DIR				*dirp;
	char			*DAppArgs, tmpPSArgs[PRARGSZ], pathName[256];
	struct dirent	*direntp;
	prpsinfo_t		psInfo;
	pid_t			ret_pid;

	ret_pid	= 0;

	if( (dirp = opendir(PROC_DIR)) == NULL)
	{
		fprintf(stderr, "[%s] opendir fail; err=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
		return -1;
	}

	while( (direntp = readdir(dirp)) != NULL)
	{
		if(!strcasecmp(direntp->d_name, PARENT_PATH) || !strcasecmp(direntp->d_name, HOME_PATH))
			continue;

		sprintf(pathName, "%s/%s", PROC_DIR, direntp->d_name);

		if( (fd = open(pathName, O_RDONLY)) < 0)
			continue;
		if(ioctl(fd, PIOCPSINFO, &psInfo) < 0)
		{
			close(fd);
			continue;
		}
		close(fd);

	    DAppArgs    = NULL;

	    memset(tmpPSArgs, 0x00, PRARGSZ);
	    strcpy(tmpPSArgs, psInfo.pr_psargs);
	    if( (DAppArgs = strstr(tmpPSArgs, "-DADAPTER=")) == NULL)
			continue;
		else
		{
			ret_pid	= psInfo.pr_pid;
			break;
		}
	}
	closedir(dirp);
#if 0
	sprintf(trcBuf, "[%s] CM ADAPTER pid(%lu)\n", __FUNCTION__, ret_pid);
	fprintf(stderr, trcBuf);
#endif
	return ret_pid;
}



#define PROC_PATH "/proc"
//------------------------------------------------------------------------------
// 특정 프로세스의 PID를 찾는다.
//------------------------------------------------------------------------------
int getProcessID(char *procName)
{
    int             fd;  
    DIR             *dirp;
    FILE            *fp; 
    char            pathName[256];//, pName[256];
    struct dirent   *direntp;
    prpsinfo_t      psInfo;
    char            *DAppArgs, *DAppToken, tmpPSArgs[PRARGSZ];//, buff[256], buff2[256];

    if( (dirp = opendir(PROC_PATH)) == NULL)
    {    
        sprintf(trcBuf, "[getProcessID] opendir fail; err=%d(%s)\n", errno, strerror(errno));
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }    

    while( (direntp = readdir(dirp)) != NULL)
    {    
        if(!strcasecmp(direntp->d_name, PARENT_PATH) || !strcasecmp(direntp->d_name, HOME_PATH))
            continue;

        sprintf(pathName, "%s/%s", PROC_PATH, direntp->d_name);

        if( (fd = open(pathName, O_RDONLY)) < 0) 
            continue;

        if(ioctl(fd, PIOCPSINFO, &psInfo) < 0) 
        {    
            close(fd);
            continue;
        }    
        close(fd);

        if(!strcasecmp(procName, "CM") || !strcasecmp(procName, "SMSERVER"))
        {    
            fp          = NULL;
            DAppToken   = NULL;                                                         
            DAppArgs    = NULL;                                                         
                                                                                        
            memset(tmpPSArgs, 0x00, PRARGSZ);                                           
            strcpy(tmpPSArgs, psInfo.pr_psargs);                                        
            DAppArgs    = strstr(tmpPSArgs, "DAPP");                                    
            if(DAppArgs)                                                                
            {                                                                           
                if( (DAppToken = strchr(DAppArgs, '=')) == NULL)                        
                {                                                                       
                //#ifdef DEBUG                                                          
                    sprintf(trcBuf, "[%s] check args(%s)\n", __FUNCTION__, DAppArgs);   
                    trclib_writeLogErr(FL, trcBuf);                                     
                //#endif                                                                
					closedir(dirp);
                    return -1;                                                          
                }                                                                       
                else                                                                    
                    DAppToken++;                                                        
                                                                                        
                while( (char)*DAppToken == ' ')                                         
                    DAppToken++;                                                        
                                                                                        
                strtok(DAppToken, " ");                                                 
                if( !strcasecmp(DAppToken, procName))                                   
                {                             
					closedir(dirp);
					return psInfo.pr_pid;
				}
			}
			else	
				 continue;
		}
	}
	closedir(dirp);
	return -1;
}



/*
by sjjeon
port 정보로 구동 시간을 얻어온다.
*/
int getPidInfo(int pid, char *sttime)
{                                                                                       
    char trcBuf[1024];                                                                  
    char pathName[128];                                                                 
    prpsinfo_t    psInfo;                                                               
    int fd;                                                                             
                                                                                        
    bzero(pathName,sizeof(pathName));                                                   
                                                                                        
    sprintf(pathName, "%s/%d", PROC_PATH,pid );                                         
                                                                                        
    if((fd = open(pathName, O_RDONLY)) < 0)                                             
    {                                                                                   
        //if( (errno != EACCES) && (errno != ENOENT))                                   
        {                                                                               
            /*  Permission denied 는 skip하기 위해  */                                  
            sprintf(trcBuf, "[%s] open fail; (%s) err=%d[%s]\n",                        
                    __FUNCTION__, pathName, errno, strerror(errno));                    
            trclib_writeLogErr(FL, trcBuf);                                             
        }                                                                               
                                                                                        
        return -1;                                                                      
    }                                                                                   
                                                                                        
    if(ioctl(fd, PIOCPSINFO, &psInfo) < 0)                                              
    {                                                                                   
        sprintf(trcBuf, "[%s] ioctl err=%d[%s]\n", __FUNCTION__, errno, strerror(errno));
        trclib_writeLogErr(FL, trcBuf);                                                 
        close(fd);                                                                      
        return -1;                                                                      
    }                                                                                   
    close(fd);                                                                          
                                                                                        
    strftime (sttime, 32, "%m-%d %H:%M", localtime((time_t*)&(psInfo.pr_start.tv_sec)));
    //strftime (startTime, 32, "%m-%d %H:%M", localtime((time_t*)&(psInfo.pr_start.tv_sec)));
    //printf("p_name: %s, p_id : %d, time : %s\n",psInfo.pr_fname, psInfo.pr_pid,startTime );
    return 0;                                                                           
                                                                                        
}                  
