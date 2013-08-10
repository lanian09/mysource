#include "ifb_proto.h"
#include "commlib.h"

extern IFB_ProcInfoContext	confProcTbl[SYSCONF_MAX_APPL_NUM];
extern int startprc_initLog(void);
extern int startprc_getSAMDqid (void);
int		oneProcFlag=0, fastFlag=0, noIFlag=0, samdQid;
char	oneProcName[32];
char    *iv_home;      
char mySysName[COMM_MAX_NAME_LEN];
int  trcLogId, trcErrLogId;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	int		procIndex;
	char	*env;

	freopen ("/dev/null", "w", stdout);
	signal (SIGHUP, SIG_IGN);
	signal (SIGCHLD, SIG_IGN);

    if ((iv_home = getenv(IV_HOME)) == NULL) {
        fprintf(stderr, "[startprc_init] not found %s environment name\n", IV_HOME);
        return -1;
    }

    if ((env = getenv(MY_SYS_NAME)) == NULL) {
        fprintf(stderr, "[startprc_init] not found %s environment name\n", MY_SYS_NAME);
        return -1;
    }

    strcpy (mySysName, env);

    if(startprc_initLog() < 0)
        return -1;

	init_version_shm();

	// login-name을 확인한다.
	if (ifb_checkLogin () < 0)
		return -1;

	/* id, passwd check mml_passwd 파일 참조 */	
	//if(interact_w(iv_home) < 0)
        //	return -1;

	// input argument 확인
	if (startprc_getArgs (ac, av) < 0)
		return -1;

	// 040920.hlr.cjs
    if (startprc_getSAMDqid () < 0)
        return -1;

	// sysconf 파일에서 등록된 process들의 list를 confProcTbl에 저장한다.
	//
	if (ifb_setConfProcTbl () < 0)
		return -1;

	// /proc를 검색하여 실행중인 process들의 정보를 confProcTbl에 저장한다.
	if (ifb_getProcStatus () < 0)
		return -1;

	if (oneProcFlag) {
		// confProcTbl에서의 index를 찾는다. -> 대소문자 구분 안한다.
		if ((procIndex = ifb_getProcIndex (oneProcName)) < 0) {
			fprintf(stderr,"\n Not Registered process\n\n");
			return -1;
		}
		startprc_startupProc (procIndex);

		commlib_microSleep(300000);
		goto startprc_echoprint;
	}

	// 등록된 프로세스 리스트를 출력하고 Confirm을 위해 묻는다.
	if(!fastFlag && !noIFlag) {
		if (startprc_printProcListPrompt () < 0)
			return -1;
	}

	// 기동 중간에 Ctl-C에 의해 중지 할 수 있도록
	signal (SIGINT, startprc_interrupt);

	// 등록된 모든 프로세스를 차례로 기동시킨다.
	//
	for (procIndex=0; procIndex<SYSCONF_MAX_APPL_NUM; procIndex++) {
		if (!strcasecmp (confProcTbl[procIndex].procName, ""))
			continue;
		// 프로세스를 기동시키고 결과를 출력한다.
		// - 이미 running 중이면 죽이고 다시 띄울것인지 묻는다.
		// - 새로이 기동된 pid가 confProcTbl에 저장된다.
		startprc_startupProc (procIndex);
		commlib_microSleep(500000);
	}

startprc_echoprint:
	// 새로이 기동시킨 프로세스들의 정보를 다시 기록하기 위해 clear한다.
	memset ((void*)confProcTbl, 0, sizeof(confProcTbl));

	// 프로세스의 상태를 다시 읽어 출력한다.
	ifb_setConfProcTbl ();
	ifb_getProcStatus ();
	ifb_printProcStatus ();

	detatch_version_shm();

	return 0;

} //----- End of main -----//




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int startprc_getArgs (int ac, char *av[])
{
	int		a;
	char	*usage =
"  startprc [-b blockName] [-f]\n\
          -f           : startup processes immediately\n\
          -v           : startup processes without interacting with user\n\
          -b blockName : startup process block name\n\n";

	while ((a = getopt(ac,av,"b:fv")) != EOF)
	{
		switch (a) {
			case 'b':
				oneProcFlag = 1;
				strcpy (oneProcName, optarg);
				break;

			case 'f':
                fastFlag = 1;
                break;

			case 'v':
                noIFlag = 1;
                break;

			default:
				fprintf(stderr,"%s",usage);
				return -1;
		}
	}

	if (optind < ac) {
		fprintf(stderr,"%s",usage);
		return -1;
	}

	return 1;

} //----- End of startprc_getArgs -----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//040920.hlr.cjs
void startprc_notify2samd (int index)
{
	int		txLen;
	GeneralQMsgType			txMsg;
	IFB_KillPrcNotiMsgType	*notiMsg;

	txMsg.mtype = MTYPE_STATUS_REPORT;
	notiMsg = (IFB_KillPrcNotiMsgType*)txMsg.body;

	strcpy (notiMsg->processName, confProcTbl[index].procName);
	notiMsg->Pid = confProcTbl[index].pid;
	notiMsg->type = 1; //start-prc
	txLen = sizeof(IFB_KillPrcNotiMsgType);

	if (!strcasecmp(confProcTbl[index].procName, "SAMD")) return;

	if ( msgsnd (samdQid, (char*)&txMsg, txLen, IPC_NOWAIT)< 0 ){ 
		fprintf(stderr,"\n SAMD msgsnd fail; to SAMD  err=%d(%s)\n\n", errno, strerror(errno));
	}

	return;

} //----- End of killprc_notify2samd -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void startprc_startupProc (int index)
{
	pid_t	pid;
	char	pname[32];
	int     i;

	// 이미 running 중인지 확인
	if (confProcTbl[index].pid) {
		fprintf(stderr,"\n %s is already running\n", confProcTbl[index].procName);
		return;
/* LG Request that when the process is already running, do not ask the question
   of process replacement, do not invoke the new process, return at once." - samuel 2006.09.26 */
/*		fprintf(stderr," Replace with New Process ? (y/n) [n] ");
		if (!ifb_promptYesNo())
			return;
		// 기존것을 죽인다.
		if (ifb_killProc (confProcTbl[index].pid) < 0)
			return;

        logPrint(trcLogId,FL," Process %s(pid=%d) was killed\n",
               confProcTbl[index].procName, confProcTbl[index].pid);

		fprintf(stderr," Process %s(pid=%d) was killed\n",
				confProcTbl[index].procName, confProcTbl[index].pid);
*/
	}

	if ((pid = fork()) < 0) {
		fprintf(stderr,"\n Can't Create New Process\n\n");
		return;
	}

	if (pid == 0) {
		// 해당 process에 등록된 메시지 큐에 있는 예전 메시지를 지운다음 기동 시킨다.
		ifb_clearQ(confProcTbl[index].msgQkey, 0);

		// sysconf에는 대문자로 등록되어 있고, 실행파일 이름은 소문자로 되어 있다.
		for (i=0; i<strlen(confProcTbl[index].procName); i++ )
			pname[i] = tolower(confProcTbl[index].procName[i]);
		pname[i] = 0x00;
		if (execl(confProcTbl[index].exeName, pname, NULL) < 0) {
            logPrint(trcErrLogId,FL,"Can't Execute New Process : %s (%s), REASON : %s\n",
                    confProcTbl[index].procName, confProcTbl[index].exeName, strerror(errno));

			fprintf(stderr,"\n Can't Execute New Process : %s (%s)\n\n",
					confProcTbl[index].procName, confProcTbl[index].exeName);
		}
		exit(0);
	}

	startprc_notify2samd (index);// 040920.hlr.cjs log for kill/start prc

    logPrint(trcLogId,FL,"[STARTUP] PROCESS : %s, PID : %ld COMPLETED\n",
             confProcTbl[index].procName, pid);

	fprintf(stderr,"\n STARTUP NEW PROCESS\n   PROCESS : %s\n   PID : %ld\n COMPLETED\n\n",
			confProcTbl[index].procName, pid);
	return;

} //----- End of startprc_startupProc -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int startprc_printProcListPrompt (void)
{
	int		index;

	fprintf(stderr,"============================================================\n");
	fprintf(stderr," PROCESS_NAME   WHERE \n");
	fprintf(stderr,"============================================================\n");

	for (index=0; index<SYSCONF_MAX_APPL_NUM; index++) {
		if (strcasecmp(confProcTbl[index].procName, ""))
			fprintf(stderr," %-12s   %s\n", confProcTbl[index].procName, confProcTbl[index].exeName);
	}
	fprintf(stderr,"============================================================\n");
	fprintf(stderr,"\n Do you want startup whole Processes ? (y/n) [n] ");
	if (!ifb_promptYesNo())
		exit(0);
	fprintf(stderr,"\n     Startup whole Processes now ? (y/n) [n] ");
	if (!ifb_promptYesNo())
		exit(0);
	return 1;

} //----- End of startprc_printProcListPrompt -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void startprc_interrupt (int signo)
{
	fprintf(stderr,"\n     Do you want Abort startup procedure ? (y/n) [n] ");
	if (!ifb_promptYesNo())
		exit(0);
	signal (SIGINT, startprc_interrupt);
	return;
} //----- End of startprc_interrupt -----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int startprc_getSAMDqid (void)
{
    char    *env, fname[256], tmp[32];
    int     key;

    if ((env = getenv(IV_HOME)) == NULL) {
        fprintf(stderr,"\n not found %s environment name\n\n", IV_HOME);
        return -1;
    }
    sprintf(fname,"%s/%s",env,SYSCONF_FILE);
    if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "SAMD", 1, tmp) < 0)
        return -1;

    key = strtol(tmp,0,0);
    if ((samdQid = msgget (key, IPC_CREAT|0666)) < 0) {
        fprintf(stderr,"\n SAMD msgget fail; key=0x%x, err=%d(%s)\n\n", key, errno, strerror(errno));
        return -1;
    }
    return 1;

} //----- End of startprc_getSAMDqid -----//

int startprc_initLog(void)
{
    char    fname[256];

    sprintf(fname, "%s/%s.%s", iv_home, STARTPRC_LOG_FILE, mySysName);

    if ((trcLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr, "[killprc_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    sprintf(fname, "%s/%s.%s", iv_home, STARTPRC_ERRLOG_FILE, mySysName);
    if ((trcErrLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr, "[killprc_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    return 1;
}
