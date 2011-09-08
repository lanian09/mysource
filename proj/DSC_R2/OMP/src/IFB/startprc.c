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

	// login-name�� Ȯ���Ѵ�.
	if (ifb_checkLogin () < 0)
		return -1;

	/* id, passwd check mml_passwd ���� ���� */	
	//if(interact_w(iv_home) < 0)
        //	return -1;

	// input argument Ȯ��
	if (startprc_getArgs (ac, av) < 0)
		return -1;

	// 040920.hlr.cjs
    if (startprc_getSAMDqid () < 0)
        return -1;

	// sysconf ���Ͽ��� ��ϵ� process���� list�� confProcTbl�� �����Ѵ�.
	//
	if (ifb_setConfProcTbl () < 0)
		return -1;

	// /proc�� �˻��Ͽ� �������� process���� ������ confProcTbl�� �����Ѵ�.
	if (ifb_getProcStatus () < 0)
		return -1;

	if (oneProcFlag) {
		// confProcTbl������ index�� ã�´�. -> ��ҹ��� ���� ���Ѵ�.
		if ((procIndex = ifb_getProcIndex (oneProcName)) < 0) {
			fprintf(stderr,"\n Not Registered process\n\n");
			return -1;
		}
		startprc_startupProc (procIndex);

		commlib_microSleep(300000);
		goto startprc_echoprint;
	}

	// ��ϵ� ���μ��� ����Ʈ�� ����ϰ� Confirm�� ���� ���´�.
	if(!fastFlag && !noIFlag) {
		if (startprc_printProcListPrompt () < 0)
			return -1;
	}

	// �⵿ �߰��� Ctl-C�� ���� ���� �� �� �ֵ���
	signal (SIGINT, startprc_interrupt);

	// ��ϵ� ��� ���μ����� ���ʷ� �⵿��Ų��.
	//
	for (procIndex=0; procIndex<SYSCONF_MAX_APPL_NUM; procIndex++) {
		if (!strcasecmp (confProcTbl[procIndex].procName, ""))
			continue;
		// ���μ����� �⵿��Ű�� ����� ����Ѵ�.
		// - �̹� running ���̸� ���̰� �ٽ� �������� ���´�.
		// - ������ �⵿�� pid�� confProcTbl�� ����ȴ�.
		startprc_startupProc (procIndex);
		commlib_microSleep(500000);
	}

startprc_echoprint:
	// ������ �⵿��Ų ���μ������� ������ �ٽ� ����ϱ� ���� clear�Ѵ�.
	memset ((void*)confProcTbl, 0, sizeof(confProcTbl));

	// ���μ����� ���¸� �ٽ� �о� ����Ѵ�.
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

	// �̹� running ������ Ȯ��
	if (confProcTbl[index].pid) {
		fprintf(stderr,"\n %s is already running\n", confProcTbl[index].procName);
		return;
/* LG Request that when the process is already running, do not ask the question
   of process replacement, do not invoke the new process, return at once." - samuel 2006.09.26 */
/*		fprintf(stderr," Replace with New Process ? (y/n) [n] ");
		if (!ifb_promptYesNo())
			return;
		// �������� ���δ�.
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
		// �ش� process�� ��ϵ� �޽��� ť�� �ִ� ���� �޽����� ������� �⵿ ��Ų��.
		ifb_clearQ(confProcTbl[index].msgQkey, 0);

		// sysconf���� �빮�ڷ� ��ϵǾ� �ְ�, �������� �̸��� �ҹ��ڷ� �Ǿ� �ִ�.
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
