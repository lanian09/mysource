#include "ifb_proto.h"

int	displayQkeyFlag=0;
IFB_ProcInfoContext	confProcTbl[SYSCONF_MAX_APPL_NUM];


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ifb_checkLogin (void)
{
	char	getBuf[256], *ptr, *next, *acc, *uid;
	char	opAccount[32], login_uid[32];
	FILE	*fp;

	// config file에서 운용계정(operator account)을 읽는다.
	//
	if (ifb_getOpAccount (opAccount) < 0)
		return -1;
	
	// 현재 계정을 확인한다.
	//
	sprintf(login_uid,"%ld",getuid());

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
		if (!strcasecmp (uid, login_uid))
			break;
	}
	fclose(fp);

	if (strcasecmp (opAccount, acc)) {
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
		fclose(fp);
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
		sprintf (confProcTbl[procCnt++].exeName, "%s/%s", env, token[3]);
	}
	fclose(fp);

	return 1;

} //----- End of ifb_setConfProcTbl -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ifb_getProcIndex (char *name)
{
	int		i;

	for (i=0; i<SYSCONF_MAX_APPL_NUM; i++) {
		if (!strcasecmp (confProcTbl[i].procName, name))
			return i;
	}

	return -1;

} //----- End of ifb_getProcIndex -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ifb_getProcStatus (void)
{
	DIR				*dirp;
	struct dirent	*dp;
	char			pname[256], *ver;
	int				fd, procIndex;
	prpsinfo_t		psInfo;

	if ((dirp = opendir(PROC_DIR)) == NULL) {
		fprintf(stderr,"\n opendir fail[%s]; err=%d(%s)\n\n", PROC_DIR, errno, strerror(errno));
		return -1;
	}

	while ((dp = readdir(dirp)) != NULL) {
		if (!strcasecmp(dp->d_name, ".") || !strcasecmp(dp->d_name, ".."))
			continue;

		sprintf(pname,"%s/%s", PROC_DIR, dp->d_name);

		if ((fd = open(pname, O_RDONLY)) < 0)
			continue;
		if (ioctl (fd, PIOCPSINFO, &psInfo) < 0) {
			close(fd);
			continue;
		}
		close(fd);

		// 형상에 등록된 process인지 확인하여 등록된 놈이면 confProcTbl에서의 index를 return한다.
		if ((procIndex = ifb_getProcIndex (psInfo.pr_psargs)) < 0)
			continue;

		confProcTbl[procIndex].runCnt++; // 같은 놈이 여러개 떠 있을 수 있다.
		confProcTbl[procIndex].pid = psInfo.pr_pid;
		strftime (confProcTbl[procIndex].startTime, 32, "%m-%d %H:%M",
					localtime((time_t*)&(psInfo.pr_start.tv_sec)));
		ver = get_proc_version(confProcTbl[procIndex].procName);
		if(!ver){
		//	fprintf(stderr, "It fails to get version information -[%s].\n", confProcTbl[procIndex].procName);  sjjeon 오류로 막는다.
			strncpy(confProcTbl[procIndex].procVersion, "UNKNOWN", 10);
		}else{
			if(strlen(ver) == 0)
				strncpy(confProcTbl[procIndex].procVersion, "UNKNOWN", 10);
			else
				strncpy(confProcTbl[procIndex].procVersion, ver, 10);
		}

	}
	closedir(dirp);

	return 1;

} //----- End of ifb_getProcStatus -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ifb_getGuiStatus (void)
{
    DIR             *dirp;
    struct dirent   *dp;
    char            pname[256];
    int             fd;
    prpsinfo_t      psInfo;
    int             findFlag = -1;

    if ((dirp = opendir(PROC_DIR)) == NULL) {
        fprintf(stderr,"\n opendir fail[%s]; err=%d(%s)\n\n", PROC_DIR, errno, strerror(errno));
        return -1;
    }

    while ((dp = readdir(dirp)) != NULL) {
        if (!strcasecmp(dp->d_name, ".") || !strcasecmp(dp->d_name, ".."))
            continue;

        sprintf(pname,"%s/%s", PROC_DIR, dp->d_name);

        if ((fd = open(pname, O_RDONLY)) < 0)
            continue;
        if (ioctl (fd, PIOCPSINFO, &psInfo) < 0) {
            close(fd);
            continue;
        }
        close(fd);

        if(strncmp(psInfo.pr_psargs,"java",4))
            continue;

        findFlag = psInfo.pr_pid;
    }
    closedir(dirp);

    return findFlag;

} //----- End of ifb_getGuiStatus -----//


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



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void ifb_printProcStatus (void)
{
	int		i, alive=0, dead=0;

	if (displayQkeyFlag) {
        	fprintf(stderr,"=================================================================================================\n");
        	fprintf(stderr," Process  PID      STATUS  MSG_Q_KEY    VERSION  Process  PID      STATUS  MSG_Q_KEY    VERSION  \n");
        	fprintf(stderr,"-------------------------------------------------------------------------------------------------");
	} else {
        	fprintf(stderr,"=================================================================================================\n");
        	fprintf(stderr," Process  PID      STATUS  START-TIME   VERSION  Process  PID      STATUS  START-TIME   VERSION  \n");
        	fprintf(stderr,"-------------------------------------------------------------------------------------------------");
	}
	
	for (i=0; i<SYSCONF_MAX_APPL_NUM; i++)
	{
		if (!strcasecmp (confProcTbl[i].procName, ""))
			break;
		if (i%2==0) fprintf(stderr,"\n");
		if (confProcTbl[i].runCnt) {
			if (displayQkeyFlag) {
				if (confProcTbl[i].runCnt == 1)
                    			fprintf(stderr," %-8s %-8d ALIVE   0x%-10x %-8s", confProcTbl[i].procName,
                            			(int)confProcTbl[i].pid, confProcTbl[i].msgQkey, confProcTbl[i].procVersion);
				else
                    			fprintf(stderr," %-8s %-8d ALIVE(%d)0x%-10x %-8s", confProcTbl[i].procName,
                            			(int)confProcTbl[i].pid, confProcTbl[i].runCnt, confProcTbl[i].msgQkey, confProcTbl[i].procVersion);
			} else {
				if (confProcTbl[i].runCnt == 1)
                    			fprintf(stderr," %-8s %-8d ALIVE   %-12s %-8s", confProcTbl[i].procName,
                            			(int)confProcTbl[i].pid, confProcTbl[i].startTime, confProcTbl[i].procVersion);
				else
                    			fprintf(stderr," %-8s %-8d ALIVE(%d)%-12s %-8s", confProcTbl[i].procName,
                            			(int)confProcTbl[i].pid, confProcTbl[i].runCnt, confProcTbl[i].startTime, confProcTbl[i].procVersion);
			}
			alive += confProcTbl[i].runCnt;
		} else {
			if (displayQkeyFlag) {
                		fprintf(stderr," %-8s -        DEAD    0x%-10x %-8s", confProcTbl[i].procName,
                        	confProcTbl[i].msgQkey, confProcTbl[i].procVersion);
			} else {
                		fprintf(stderr," %-8s -        DEAD    -     -      %-8s", confProcTbl[i].procName, "-");
			}
			dead++;
		}
	}

	fprintf(stderr,"\n=================================================================================================\n");
	fprintf(stderr,"TOTAL:%d (ALIVE:%d, DEAD:%d)\n\n", alive+dead, alive, dead);

	return;

} //----- End of ifb_printProcStatus -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ifb_promptYesNo (void)
{
	char	input[256], *ptr;

	fgets(input, sizeof(input), stdin);
	for (ptr=input; isspace(*ptr); ptr++) ;
	if (*ptr == 'y' || *ptr == 'Y')
		return 1;
	else
		return 0;
} //----- End of ifb_promptYesNo -----//

int ifb_promptYesNo2 (char *choice)
{
        if (*choice == 'y' || *choice == 'Y')
                return 1;
        else
                return 0;
} //----- End of ifb_promptYesNo -----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ifb_killProc (pid_t pid)
{
	if (kill(pid, SIGTERM) < 0) {
		if (kill(pid, SIGKILL) < 0) {
			fprintf(stderr," Can't kill running process (pid=%ld)\n\n", pid);
			return -1;
		}
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
			fprintf(stderr,"msgrcv len=%d\n", ret);
		}
		cnt++;
	}
	return cnt;
} //----- End of ifb_clearQ -----//

/* killprc, rmsysqm login check */
int check_user_valid(FILE *fp, char *username, char *password)
{
	char	line[1024], *lasts, *tok;
	int	match;

	rewind(fp);

	match = 0;

	while(fgets(line, sizeof(line), fp)){
		tok = strtok_r(line, ":", &lasts);
		if(strcasecmp(tok, username)){
			continue;
		}

		tok = strtok_r(NULL, ":", &lasts);
		if(!strcasecmp(tok, (char *)crypt(password, password))){
			match = 1;
		}
	}

	if(match)
		return 1;
	else
		return 0;
}

/* killprc, rmsysqm Id, Passwd delivery */
int interact_w(char *ivhome)
{

	int	i;
	FILE	*fp;
	char	username[50], password[50];
	char	passfile[100];

	sprintf(passfile, "%s/%s", ivhome, PASSWORD_FILE);
	fp = fopen(passfile, "r");
	if(fp == NULL){
		fprintf(stderr, "Cannot open the password file\n");
		exit(1);
	}
	
	for(i = 0; i < RETRY_COUNT; i++){
		fprintf(stderr, "user id :");
		scanf("\n%s", username);
		if(username[strlen(username)-1] == '\n')
			username[strlen(username)-1] = 0x00;
		fprintf(stderr, "password :");
		system("stty -echo");
		scanf("\n%s", password);
		system("stty echo");
		fprintf(stderr, "\n");
		if(password[strlen(password)-1] == '\n')
			password[strlen(password)-1] = 0x00;
	
		if(check_user_valid(fp, username, password))
			break;


	}

	fclose(fp);

	if(i >= RETRY_COUNT)
		return -1;

	return 0;
}



