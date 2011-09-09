#include "smsc_proto.h"

extern int		ixpcPortNum, trcLogId, trcErrLogId, trcLogFlag;
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern char		trcBuf[4096], trcTmp[1024];

char vERSION[8] = "R1.0.0";

/*------------------------------------------------------------------------------*/
int smsc_initial (void)
{
	char	*env,tmp[64],fname[256];
	int		key,i,id, ret;

	if ((env = getenv(MY_SYS_NAME)) == NULL) {
		fprintf(stderr,"[ixpc_init] not found %s environment name\n", MY_SYS_NAME);
		return -1;
	}
	strcpy (mySysName, env);
	strcpy (myAppName, "SMSC");
	commlib_setupSignals (NULL);

	socklib_initial();

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[ixpc_init] not found %s environment name\n", IV_HOME);
		return -1;
	}
	sprintf(fname,"%s/%s", env, SYSCONF_FILE);

	/* config file에서 자신의 bind port number를 읽어 binding한다.
	*/
	if (conflib_getNthTokenInFileSection (fname, "[SMSC_INFO]", "SMSC1", 2, tmp) < 0)
		return -1;
	ixpcPortNum = strtol(tmp,0,0);
	if (socklib_initTcpBind(ixpcPortNum) < 0)
		return -1;

	/* log file들을 open한다.
	*/
	if (smsc_initLog () < 0)
		return -1;

	logPrint (trcLogId,FL,"%s startup...\n", myAppName);
	logPrint (trcErrLogId,FL,"%s startup...\n", myAppName);
	return 1;

} /** End of ixpc_initial **/


/*------------------------------------------------------------------------------
* error 메시지를 남길 error log file과 trace 메시지를 남길 log file을 open한다.
------------------------------------------------------------------------------*/
int smsc_initLog (void)
{
	char	*env,fname[256];

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[ixpc_initLog] not found %s environment name\n", IV_HOME);
		return -1;
	}
	sprintf(fname,"%s/%s.%s", env, SMSC_LOG_FILE, mySysName);

	if ((trcLogId = loglib_openLog (fname,
			LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
			LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
		fprintf(stderr,"[ixpc_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	sprintf(fname,"%s/%s.%s", env, SMSC_ERRLOG_FILE, mySysName);
	if ((trcErrLogId = loglib_openLog (fname,
			LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
			LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
		fprintf(stderr,"[ixpc_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	return 1;

} /** End of ixpc_initLog **/


