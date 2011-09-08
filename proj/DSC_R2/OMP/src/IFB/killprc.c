#include "ifb_proto.h"
#include "commlib.h"

extern IFB_ProcInfoContext	confProcTbl[SYSCONF_MAX_APPL_NUM];
extern int killprc_initLog();
int		oneProcFlag=0, fastFlag=0, noIFlag=0, guiFlag=0, samdQid;
char	oneProcName[32];
char	*iv_home;
char mySysName[COMM_MAX_NAME_LEN];
int  trcLogId, trcErrLogId;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	int	procIndex;
	int	guiPID;
	char	*env;

    	if ((iv_home = getenv(IV_HOME)) == NULL) {
        	fprintf(stderr, "[killprc_init] not found %s environment name\n", IV_HOME);
        	return -1;
    	}

    	if ((env = getenv(MY_SYS_NAME)) == NULL) {
        	fprintf(stderr, "[killprc_init] not found %s environment name\n", MY_SYS_NAME);
        	return -1;
    	}

    	strcpy (mySysName, env);

    	if(killprc_initLog() < 0)
        	return -1;
	fprintf(stdout, "\n");
	
	// login-name을 확인한다.
	if (ifb_checkLogin () < 0)
		return -1;

	/* id, passwd check mml_passwd 파일 참조 */
	if(interact_w(iv_home) < 0)
		return -1;

	init_version_shm();

	fflush(stdin);

	// input argument 확인
	if (killprc_getArgs (ac, av) < 0)
		return -1;

	if (killprc_getSAMDqid () < 0)
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

		if (killprc_shutdownProc (procIndex) < 0)
			return -1;

		// 해당 process에 등록된 메시지 큐에 남아있는 메시지를 지운다.
		ifb_clearQ(confProcTbl[procIndex].msgQkey, 0);

		commlib_microSleep(300000);

		if(oneProcFlag ==2) {
			// sysconf 파일에서 등록된 process들의 list를 다시 읽는다.
			//
			confProcTbl[procIndex].pid = 0;

			/*  살아 있을 경우 confProcTbl[procIndex].pid에 셋팅하고
			**  죽어있으면 위에서 셋팅한 0 값을 갖는다
			*/
			if (ifb_getProcStatus () < 0)
				return -1;
			/* SAMD  의 콘솔 장애 출력 시간을 감안해 2초 쉰다
			*/
			sleep(1);
			startprc_startupProc (procIndex);
		}

		goto killprc_echoprint;
	}

	if(!noIFlag){
		fprintf(stderr,"\n Do you want shutdown whole Processes ? (y/n) [n] ");
		if (!ifb_promptYesNo())
			exit(0);
		fprintf(stderr,"\n     Shutdown whole Processes now ? (y/n) [n] ");
		if (!ifb_promptYesNo())
			exit(0);
	}

	// 등록된 모든 프로세스를 차례로 죽인다.
	// SAMD, IXPC,COND, FIMD 는 나중에 별도로 종료시킨다. modified by uamyd 20110504
	int samdIndex, fimdIndex, condIndex, ixpcIndex;
	for (procIndex=0; procIndex<SYSCONF_MAX_APPL_NUM; procIndex++) {
		if (!strcasecmp (confProcTbl[procIndex].procName, "")){
			continue;
		}
		if (!strcasecmp (confProcTbl[procIndex].procName, "SAMD")){
			samdIndex = procIndex;
		}
		if (!strcasecmp (confProcTbl[procIndex].procName, "FIMD")){
			fimdIndex = procIndex;
		}
		if (!strcasecmp (confProcTbl[procIndex].procName, "COND")){
			condIndex = procIndex;
		}
		if (!strcasecmp (confProcTbl[procIndex].procName, "IXPC")){
			ixpcIndex = procIndex;
		}
		killprc_notify2samd (procIndex);
	}

	commlib_microSleep(10000);
	for (procIndex=0; procIndex<SYSCONF_MAX_APPL_NUM; procIndex++) {
		if (!strcasecmp (confProcTbl[procIndex].procName, "")){
			continue;
		}

		if( procIndex == samdIndex || procIndex == condIndex ||
			procIndex == ixpcIndex || procIndex == fimdIndex ){
			continue;
		}

		if (killprc_shutdownProc (procIndex) < 0)
			continue;

		// 해당 process에 등록된 메시지 큐에 남아있는 메시지를 지운다.
		//ifb_clearQ(confProcTbl[procIndex].msgQkey, 0);

		commlib_microSleep(100000);
	}

	commlib_microSleep(1500000);
	killprc_shutdownProc(samdIndex);

	commlib_microSleep(1500000);
	killprc_shutdownProc(ixpcIndex);

	commlib_microSleep(1000000);
	killprc_shutdownProc(condIndex);

	//FIMD 는 SAMD가 마지막으로 전송한 process 정보를 OMC로 전송한 뒤 종료되어야 함. 일단, 이렇게 처리 by uamyd 20110505
	commlib_microSleep(1000000);
	killprc_shutdownProc(fimdIndex);

	commlib_microSleep(500000);

    // kill GUI
    if(guiFlag) {
        if ((guiPID = ifb_getGuiStatus ()) > 0) {
            if (ifb_killProc (guiPID) >= 0) {
                fprintf(stderr,"\n SHUTDOWN GUI PROCESS\n"); 
            } else
                fprintf(stderr,"\n CANNOT SHUTDOWN GUI PROCESS\n"); 
        }
    }


killprc_echoprint:
	sleep(1);
//	commlib_microSleep(150000);
	// 프로세스들의 정보를 다시 기록하기 위해 clear한다.
	memset ((void*)confProcTbl, 0, sizeof(confProcTbl));

	// 프로세스의 상태를 다시 읽어 출력한다.
	ifb_setConfProcTbl ();
	ifb_getProcStatus ();
	// disprc
	ifb_printProcStatus ();

	detatch_version_shm();

	exit(0);

} //----- End of main -----//




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int killprc_getArgs (int ac, char *av[])
{
	int		a;
	char	*usage =
"  killprc [blockName] [-b blockName] [-r blockName] [-g] [-f] \n\
          blockName : kill & restart process block name\n\
          -b blockName : kill process block name\n\
          -r blockName : kill & restart process block name\n\
          -g : kill all processes & GUI\n\
          -v : kill all processes without interacting with user\n\
          -f : fast\n";

	if (ac == 2 && av[1][0] != '-') {
		oneProcFlag = 2;
		strcpy (oneProcName, av[1]);
		return 1;
	}

	while ((a = getopt(ac,av,"b:r:fgv")) != EOF)
	{
		switch (a) {
			case 'b':
				oneProcFlag = 1;
				strcpy (oneProcName, optarg);
				break;

			case 'r':
				oneProcFlag = 2;
				strcpy (oneProcName, optarg);
				break;

			case 'f':
				fastFlag = 1;
				break;

			case 'g':
				guiFlag = 1;
				break;

			case 'v':
				noIFlag = 1;
				break;
			
			default:
				noIFlag = 1;
#if 0
			default:
				fprintf(stderr,"%s",usage);
				return -1;
#endif		

		}
	}

	if (optind < ac) {
		fprintf(stderr,"%s",usage);
		return -1;
	}

	return 1;

} //----- End of killprc_getArgs -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int killprc_shutdownProc (int index)
{
	// 이미 죽은놈인지 확인
	if (!confProcTbl[index].pid) {
		fprintf(stderr,"\n %s is not running\n\n", confProcTbl[index].procName);
		return -1;
	}

	/*
		순서를 바꾼다. added by uamyd 20110427
		block 을 먼저 종료시키고, noti를 하는 경우,
		자주 발생하는 상황은 아니지만, samd 에서 noti를 받아서 masking 처리를 하기 전에
		재기동을 하는 상황이 발생할 수 있기 때문에 수정함.	
		
	if (ifb_killProc (confProcTbl[index].pid) < 0)
		return -1;

	// samd에서 auto-restart 하지 않도록 통보한다.
	killprc_notify2samd (index);
	*/

	killprc_notify2samd (index);
	if (ifb_killProc (confProcTbl[index].pid) < 0){
		return -1;
	}

    logPrint(trcLogId,FL, "[SHUTDOWN] PROCESS : %s, PID : %ld COMPLETED\n",
            confProcTbl[index].procName, confProcTbl[index].pid);
	fprintf(stderr,"\n SHUTDOWN PROCESS\n   PROCESS : %s\n   PID : %ld\n COMPLETED\n\n",
			confProcTbl[index].procName, confProcTbl[index].pid);
	return 1;

} //----- End of killprc_shutdownProc -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int killprc_getSAMDqid (void)
{
	char	*env, fname[256], tmp[32];
	int		key;

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

} //----- End of killprc_getSAMDqid -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void killprc_notify2samd (int index)
{
	int		txLen;
	GeneralQMsgType			txMsg;
	IFB_KillPrcNotiMsgType	*notiMsg;

	txMsg.mtype = MTYPE_STATUS_REPORT;
	notiMsg = (IFB_KillPrcNotiMsgType*)txMsg.body;

	strcpy (notiMsg->processName, confProcTbl[index].procName);
	notiMsg->type = 0;
	notiMsg->Pid = confProcTbl[index].pid;
	txLen = sizeof(IFB_KillPrcNotiMsgType);

	if ( msgsnd (samdQid, (char*)&txMsg, txLen, IPC_NOWAIT)< 0 ){ 
		fprintf(stderr,"\n SAMD msgsnd fail; to SAMD  err=%d(%s)\n\n", errno, strerror(errno));
		fprintf(stderr,"\n FAILED IN MASKING, Process : %s, PID : %ld\n",
			confProcTbl[index].procName, confProcTbl[index].pid);
	}

	return;

} //----- End of killprc_notify2samd -----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void startprc_startupProc (int index)
{
    pid_t   pid;
    char    pname[32];
	int     i;

    // 이미 running 중인지 확인
    if (confProcTbl[index].pid) {
        fprintf(stderr,"\n %s is already running\n", confProcTbl[index].procName);
        fprintf(stderr," Replace with New Process ? (y/n) [n] ");
        if (!ifb_promptYesNo())
            return;
        // 기존것을 죽인다.
        if (ifb_killProc (confProcTbl[index].pid) < 0)
            return;
        fprintf(stderr," Process %s(pid=%ld) was killed\n",
                confProcTbl[index].procName, confProcTbl[index].pid);
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
            fprintf(stderr,"\n Can't Execute New Process : %s (%s)\n\n",
                    confProcTbl[index].procName, confProcTbl[index].exeName);
        }
        exit(0);
    }

    fprintf(stderr,"\n STARTUP NEW PROCESS\n   PROCESS : %s\n   PID : %ld\n COMPLETED\n\n",
            confProcTbl[index].procName, pid);
    return;

} //----- End of startprc_startupProc -----//

int killprc_initLog(void)
{
    char    fname[256];

    sprintf(fname, "%s/%s.%s", iv_home, KILLPRC_LOG_FILE, mySysName);

    if ((trcLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr, "[killprc_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    sprintf(fname, "%s/%s.%s", iv_home, KILLPRC_ERRLOG_FILE, mySysName);
    if ((trcErrLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr, "[killprc_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    return 1;
}
