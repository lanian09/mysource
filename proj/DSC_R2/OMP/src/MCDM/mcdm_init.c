#include "mcdm_proto.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

extern int		mcdmQid, ixpcQid;
extern time_t	currentTime;
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern char		trcBuf[4096], trcTmp[1024];
extern int		trcLogId, trcErrLogId, trcFlag, trcLogFlag;
extern McdmOwnMmcHdlrVector		ownMmcHdlrVector[MCDM_MAX_OWN_MMC_HANDLER];
extern int		numOwnMmcHdlr;
extern McdmDistribMmcTblContext	mcdmDistrMmcTbl[MCDM_MAX_DISTRIB_MMC];
extern int		numDistrMmc;
extern	const	McdmMmcType		mcdmTYPE[];
extern SFM_sfdb	*sfdb;
extern IxpcConSts *ixpcCON;	
int mcdm_setDistrMmcTbl (void);

//char    ver[8] = "R2.2.3";
char    ver[8] = "R1.0.0";

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mcdm_initial (void)
{
	char	*env, tmp[64], fname[256];
	int		key,id;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[mcdm_init] not found %s environment name\n", IV_HOME);
		return -1;
	}
	sprintf (fname, "%s/%s", env, SYSCONF_FILE);

    // 자신의 시스템 이름을 읽는다.
	if ((env = getenv(MY_SYS_NAME)) == NULL) {
		fprintf(stderr,"[samd_init] not found %s environment name\n", MY_SYS_NAME);
		return -1;
	}
	strcpy ( mySysName, env );
	strcpy (myAppName, "MCDM");
	commlib_setupSignals (NULL);
	currentTime = time(0);

	if(set_proc_version(OMP_VER_INX_MCDM, ver) < 0){
		fprintf(stderr, "[mcdm_initial] setting process version failed\n");
		return -1;
	}


	// 자신이 직접 처리할 명령어 리스트가 등록된 ownMmcHdlrVector를 bsearch로
	//	찾기 위해 sort한다.
	//
	qsort ((void*)ownMmcHdlrVector,
			numOwnMmcHdlr,
			sizeof(McdmOwnMmcHdlrVector),
			mcdm_ownMmcHdlrVector_qsortCmp);


	// config file에서 message queue key를 읽어, attach
	//
	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "MCDM", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((mcdmQid = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[mcdm_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "IXPC", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((ixpcQid = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[mcdm_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

    if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_SFDB", 1, tmp) < 0)
        return -1;
    key = strtol(tmp,0,0);

    if ((id = (int)shmget (key, SFM_SFDB_SIZE, 0666)) < 0) {
        fprintf(stderr,"SFDB shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return -1;
    }

    if ((sfdb = (SFM_sfdb*) shmat (id,0,0)) == (SFM_sfdb*)-1) {
        fprintf(stderr,"SFDB shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return -1;
    }

    if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_IXPC_CON", 1, tmp) < 0)
        return -1;
    key = strtol(tmp,0,0);

    if ((id = (int)shmget (key, sizeof(IxpcConSts), IPC_CREAT|0666)) < 0) {
        fprintf(stderr,"IXPC conn shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return -1;
    }

    if ((ixpcCON = (IxpcConSts*) shmat (id,0,0)) == (IxpcConSts*)-1) {
        fprintf(stderr,"IXPC conn shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return -1;
    }
	

	// config 파일을 읽어 여러개 MP들로 보내져야 하는 명령어의 정보를 저장되는
	// mcdmDistrMmcTbl을 setting한다.
	// cmdName 순으로 sort한다.
	//
	if (mcdm_setDistrMmcTbl () < 0)
		return -1;


	// log file들을 open한다.
	//
	if (mcdm_initLog () < 0)
		return -1;

	//
	if (keepalivelib_init (myAppName) < 0)
		return -1;

	logPrint (trcLogId,FL,"%s startup...\n", myAppName);
	logPrint (trcErrLogId,FL,"%s startup...\n", myAppName);


	return 1;
	
} //----- End of mcdm_initial -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mcdm_initLog (void)
{
	char	*env, fname[256];

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[mcdm_initLog] not found %s environment name\n", IV_HOME);
		return -1;
	}

	sprintf (fname, "%s/%s.%s", env, MCDM_TRCLOG_FILE, mySysName);
	if ((trcLogId = loglib_openLog (fname,
			LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
			LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
		fprintf(stderr,"[mcdm_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	sprintf (fname, "%s/%s.%s", env, MCDM_ERRLOG_FILE, mySysName);
	if ((trcErrLogId = loglib_openLog (fname,
			LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
			LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
		fprintf(stderr,"[mcdm_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	return 1;

} //----- End of mcdm_initLog -----//



//------------------------------------------------------------------------------
// config 파일을 읽어 여러개 MP들로 보내져야 하는 명령어의 정보를 저장되는
//	mcdmDistrMmcTbl을 setting한다.
// mcdmDistrMmcTbl을 cmdName 순으로 sort한다.
//------------------------------------------------------------------------------
int mcdm_setDistrMmcTbl (void)
{
	int		i, tokenCnt, lNum, sysCnt;
	FILE	*fp;
	char	*env, fname[256], getBuf[256], token[16][64];;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[mcdm_setDistrMmcTbl] not found %s environment name\n", IV_HOME);
		return -1;
	}

	sprintf (fname, "%s/%s", env, MCDM_DISTRIB_MMC_FILE);

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf (stderr,"[mcdm_setDistrMmcTbl] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	numDistrMmc = 0;

	while ((fgets (getBuf, sizeof(getBuf), fp) != NULL) &&
			(numDistrMmc < MCDM_MAX_DISTRIB_MMC))
	{
		lNum++;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if ((tokenCnt = sscanf (getBuf, "%s%s%s%s%s %s%s%s%s%s %s%s%s%s%s",
					token[0], token[1], token[2], token[3], token[4],
					token[5], token[6], token[7], token[8], token[9],
					token[10],token[11],token[12],token[13],token[14])) < 3)
		{
			fclose(fp);
			fprintf (stderr,"[mcdm_setDistrMmcTbl] systax error; [%s] line=%d\n", fname, lNum);
			return -1;
		}
		sysCnt = tokenCnt - 3; // cmdName, dstAppName, TYPE을 제외하면 나열된 갯수를 알 수 있다.
		mcdmDistrMmcTbl[numDistrMmc].sysCnt = sysCnt;
		strcpy (mcdmDistrMmcTbl[numDistrMmc].cmdName, token[0]);
		strcpy (mcdmDistrMmcTbl[numDistrMmc].dstAppName, token[1]);

		/* type 추가 2003/07/31 kimjm */
		for (i=0; ; i++){
			if (mcdmTYPE[i].str[0] == NULL)
				break;
			if (!strcasecmp (mcdmTYPE[i].str, token[tokenCnt-1])){
				mcdmDistrMmcTbl[numDistrMmc].type = mcdmTYPE[i].type;
			}
		}

	//	printf (" %s %d\n", token[0], mcdmDistrMmcTbl[numDistrMmc].type );

		for (i=0; i<sysCnt; i++) {
			strcpy (mcdmDistrMmcTbl[numDistrMmc].dstSysName[i], token[i+2]);
		}
		numDistrMmc++;

	} //-- end of while() --//
	fclose(fp);

	// mcdmDistrMmcTbl을 bsearch로 찾기 위해 cmdName 순으로 sort한다.
	//
	qsort ((void*)mcdmDistrMmcTbl,
			numDistrMmc,
			sizeof(McdmDistribMmcTblContext),
			mcdm_distrMmcTbl_qsortCmp);

	return 1;

} //----- End of mcdm_setDistrMmcTbl -----//



