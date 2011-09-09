#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "mmcd_proto.h"


extern int		mmcdQid, condQid, ixpcQid, nmsibQid, MML_NUM_CMD, cmdLogId, mmcLogId;
extern int		trcLogId, trcErrLogId, trcLogFlag;
extern time_t	currentTime;
extern char		trcBuf[4096], trcTmp[1024];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern MMLCmdContext		*mmlCmdTbl;
extern MMLHelpContext		*mmlHelpTbl;
extern MmcdJobTblContext	*mmcdJobTbl;
extern MmcdUserTblContext	*mmcdUserTbl;
extern MmcdCliTblContext	*mmcdCliTbl;
extern MMcdUserIPTblContext	*mmcdIPTbl;		// 2009.07.17 by sjs
extern int		MMCD_NUM_BUILTIN_CMD;
extern MmcdBuiltInCmdVector	mmcdBuiltInCmdVector[MMCD_MAX_BUILTIN_CMD];

extern SFM_sfdb     *sfdb;
int attchSfdb (void);
char    ver[8] = "R1.0.2";

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_initial (void)
{
    char    *env,tmp[64],fname[256],errBuf[256];
    int     key,port;

    if ((env = getenv(MY_SYS_NAME)) == NULL) {
        fprintf(stderr,"[mmcd_init] not found %s environment name\n", MY_SYS_NAME);
        return -1;
    }
    strcpy (mySysName, env);
    strcpy (myAppName,"MMCD");
    currentTime = time(0);
    commlib_setupSignals (NULL);
    
    if(set_proc_version(OMP_VER_INX_MMCD, ver) < 0){
        fprintf(stderr, "[mmcd_initial] setting process version failed\n");
        return -1;
    }
    
    
    qsort ((void*)mmcdBuiltInCmdVector,
           MMCD_NUM_BUILTIN_CMD,
           sizeof(MmcdBuiltInCmdVector),
           mmcd_qsortCmp);
    
    //  cpu overload check하기 위해서
    if ( attchSfdb () < 0)
        return -1;
    
    
    //
    // 필요한 여러가지 table 영역을 할당한다.
    //
    if ((mmlCmdTbl = (MMLCmdContext*) calloc (MML_NUM_TP_CMD_TBL, sizeof(MMLCmdContext))) == NULL) 
    {
        fprintf(stderr,"[mmcd_initial] calloc fail (mmlCmdTbl)\n");
        return -1;
    }
    if ((mmlHelpTbl = (MMLHelpContext*) calloc (MML_NUM_TP_CMD_TBL, sizeof(MMLHelpContext))) == NULL) 
    {
        fprintf(stderr,"[mmcd_initial] calloc fail (mmlHelpTbl)\n");
        return -1;
    }
    if ((mmcdJobTbl = (MmcdJobTblContext*) calloc (MML_NUM_TP_JOB_TBL, sizeof(MmcdJobTblContext))) == NULL) 
    {
        fprintf(stderr,"[mmcd_initial] calloc fail (mmcdJobTbl)\n");
        return -1;
    }
    if ((mmcdUserTbl = (MmcdUserTblContext*) calloc (MML_NUM_TP_USER_TBL, sizeof(MmcdUserTblContext))) == NULL) 
    {
        fprintf(stderr,"[mmcd_initial] calloc fail (mmcdUserTbl)\n");
        return -1;
    }
    if ((mmcdCliTbl = (MmcdCliTblContext*) calloc (MML_NUM_TP_CLIENT_TBL, sizeof(MmcdCliTblContext))) == NULL) 
    {
        fprintf(stderr,"[mmcd_initial] calloc fail (mmcdCliTbl)\n");
        return -1;
    }

    if( ( mmcdIPTbl = ( MMcdUserIPTblContext* )calloc( MML_NUM_IP_TBL, sizeof( MMcdUserIPTblContext ) ) ) == NULL )	//2009.07.17 by sjs
    {
        fprintf(stderr,"[mmcd_initial] calloc fail (mmcdIPTbl)\n");
        return -1;
    }
    
    // user table을 loading한다.
    if (mmcd_loadUserTbl () < 0)
        return -1;
    
    // command table을 loading한다.
    if ((MML_NUM_CMD = mmcd_loadCmdTbl (mmlCmdTbl, mmlHelpTbl, errBuf)) < 0) 
    {
        fprintf(stderr,"%s",errBuf);
        return -1;
    }

	if( mmcd_loadIPTbl() < 0 )
		return -1;
    
    if ((env = getenv(IV_HOME)) == NULL) 
    {
        fprintf(stderr,"[mmcd_init] not found %s environment name\n", IV_HOME);
        return -1;
    }
    sprintf(fname,"%s/%s", env, SYSCONF_FILE);
    
    // config file에서 자신과 ixpc의 message queue key를 읽어, attach queue
    //
    if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "MMCD", 1, tmp) < 0)
        return -1;
    key = strtol(tmp,0,0);
    if ((mmcdQid = msgget(key,IPC_CREAT|0666)) < 0) 
    {
        fprintf(stderr,"[mmcd_init] msgget fail; key=0x%x,err=%d(%s)\n",key,errno,strerror(errno));
        return -1;
    }
    
    if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "IXPC", 1, tmp) < 0)
        return -1;
    key = strtol(tmp,0,0);
    if ((ixpcQid = msgget(key,IPC_CREAT|0666)) < 0) 
    {
        fprintf(stderr,"[mmcd_init] msgget fail; key=0x%x,err=%d(%s)\n",key,errno,strerror(errno));
        return -1;
    }
    
    if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "COND", 1, tmp) < 0)
        return -1;
    key = strtol(tmp,0,0);
    if ((condQid = msgget(key,IPC_CREAT|0666)) < 0) 
    {
        fprintf(stderr,"[mmcd_init] msgget fail; key=0x%x,err=%d(%s)\n",key,errno,strerror(errno));
        return -1;
    }

#ifdef NMSIB
	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "NMSIB", 1, tmp) < 0)
        return -1;
    key = strtol(tmp,0,0);
    if ((nmsibQid = msgget(key,IPC_CREAT|0666)) < 0) 
    {
        fprintf(stderr,"[mmcd_init] msgget fail; key=0x%x,err=%d(%s)\n",key,errno,strerror(errno));
        return -1;
    }
#endif

    // config file에서 자신의 system name을 읽는다.
    // 아래로직은 sysName을 읽는 것이 아니고  sysType을 읽는 것이므로 주석처리하겠음.
    //if (conflib_getNthTokenInFileSection (fname, "[ASSOCIATE_SYSTEMS]", "BSDM", 1, mySysName) < 0)
	//	return -1;
    
	// config file에서 자신의 bind port number를 읽어 binding한다.
	//
    if (conflib_getNthTokenInFileSection (fname, "[SOCKET_PORT]", "MMCD", 1, tmp) < 0)
        return -1;
    port = strtol(tmp,0,0);
    if (socklib_initTcpBind(port) < 0)
        return -1;
    
    // 명령어 출력 메시지의 header에 붙일 system_label을 읽는다.
    //
    if (conflib_getNthTokenInFileSection (fname, "[GENERAL]", "SYSTEM_LABEL", 1, sysLabel) < 0)
        return -1;
    
    
    // log file들을 open한다.
    //
    if (mmcd_initLog () < 0)
        return -1;
    
    //
    if (keepalivelib_init (myAppName) < 0)
        return -1;
    
    logPrint (trcLogId,FL,"%s startup...\n", myAppName);
    logPrint (trcErrLogId,FL,"%s startup...\n", myAppName);
    
    return 1;
    
} //----- End of mmcd_initial -----//



//------------------------------------------------------------------------------
// error 메시지를 남길 error log file과 trace 메시지를 남길 log file을 open한다.
//------------------------------------------------------------------------------
int mmcd_initLog (void)
{
    char	*env,fname[256];
    
    if ((env = getenv(IV_HOME)) == NULL) 
    {
        fprintf(stderr,"[mmcd_initLog] not found %s environment name\n", IV_HOME);
        return -1;
    }
    sprintf(fname,"%s/%s.%s", env, MMCD_TRCLOG_FILE, mySysName);
    
    if ((trcLogId = loglib_openLog (fname,
                                    LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
                                    LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) 
    {
        fprintf(stderr,"[mmcd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }
    
    sprintf(fname,"%s/%s.%s", env, MMCD_ERRLOG_FILE, mySysName);
    if ((trcErrLogId = loglib_openLog (fname,
                                       LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
                                       LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) 
    {
        fprintf(stderr,"[mmcd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }
    
    sprintf(fname,"%s/%s", env, MMCD_CMDHIS_FILE);
    if ((cmdLogId = loglib_openLog (fname,
                                    LOGLIB_MODE_ONE_DIR | LOGLIB_FLUSH_IMMEDIATE)) < 0) 
    {
        fprintf(stderr,"[mmcd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }
    
    sprintf(fname,"%s/%s", env, MMCD_MMCLOG_FILE);
    if ((mmcLogId = loglib_openLog (fname,
                                    LOGLIB_MODE_ONE_DIR | LOGLIB_FLUSH_IMMEDIATE)) < 0) 
    {
        fprintf(stderr,"[mmcd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }
    
    return 1;

} //----- End of mmcd_initLog -----//



//------------------------------------------------------------------------------
// 등록된 user 정보가 저장되어 있는 passwd 파일을 읽어 user table을 setting한다.
// nmsib에서 접속하는 userName과 passwd가 기록되어 있는 파일에서 userName을 읽어
//	nmsibFlag를 setting한다.
//------------------------------------------------------------------------------
int mmcd_loadUserTbl (void)
{
    FILE	*fp;
    int		i, userIndex=0;
    char	fname[256],lineBuf[256],*env,*token,*ptr,*next, tmp[32];

    if ((env = getenv(IV_HOME)) == NULL) 
    {
        fprintf(stderr,"[mmcd_loadUserTbl] not found %s environment name\n", IV_HOME);
        return -1;
    }
    sprintf(fname,"%s/%s", env, MML_PASSWD_FILE);
    
    if ((fp = fopen(fname,"r")) == NULL) 
    {
        fprintf(stderr,"[mmcd_loadUserTbl] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
        return -1;
    }
    
    while (fgets(lineBuf,sizeof(lineBuf),fp) != NULL)
    {
        ptr = lineBuf;
        
        // user_name
        token = (char*)strtok_r(ptr,":",&next);
        strcpy (mmcdUserTbl[userIndex].userName, token);
        
        // password
        ptr = next;
        token = (char*)strtok_r(ptr,":",&next);
        strcpy (mmcdUserTbl[userIndex].passwd, token);
        
        // privilege
        ptr = next;
        token = (char*)strtok_r(ptr,":",&next);
        mmcdUserTbl[userIndex].privilege = strtol(token,0,0);
        
        // lastLoginTime
        ptr = next;
        token = (char*)strtok_r(ptr,":",&next);
        mmcdUserTbl[userIndex].lastLoginTime = strtol(token,0,0);
        
        // lastLogoutTime
        ptr = next;
        token = (char*)strtok_r(ptr,":",&next);
        mmcdUserTbl[userIndex].lastLogoutTime = strtol(token,0,0);

        //name							//2009.07.17 by sjs
        ptr = next;
        token = (char*)strtok_r(ptr,":",&next);
        if( token != ( char* )NULL && strcmp( token, "\n" ) )
        {
            strncpy( mmcdUserTbl[userIndex].name, token, strlen( token ) - 1 );		// \n 때문에 -1
        }
        userIndex++;
        
    } //-- end of while(fgets()) --//
    fclose(fp);
    
    //
    // nmsib에서 접속하는 userName과 passwd가 기록되어 있는 파일을 읽어 등록된 userName과
    //  일치하는 놈은 nmsibFlag를 setting한다.
    //
    sprintf(fname,"%s/%s", env, MML_NMSIB_USER_FILE);
    
    if ((fp = fopen(fname,"r")) == NULL) 
    {
        fprintf(stdout,"[mmcd_loadUserTbl] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
        return 1;
    }

    while (fgets(lineBuf,sizeof(lineBuf),fp) != NULL)
    {
        if (lineBuf[0]=='#' || lineBuf[0]=='\n')
            continue;
        sscanf (lineBuf, "%s", tmp);
        for (i=0; i<userIndex; i++) 
        {
            if (!strcmp (mmcdUserTbl[i].userName, tmp)) 
            {
                mmcdUserTbl[i].nmsibFlag = 1;
                break;
            }
        }
    }
    fclose(fp);

    return 1;

} //----- End of mmcd_loadUserTbl -----//


//------------------------------------------------------------------------------
// user table의 사용자 정보를 passwd file에 dump한다.
// - 기존 파일을 삭제한 후 다시 만든다.
// - user_name, passwd, privilege가 저장된다.
//------------------------------------------------------------------------------
int mmcd_savePasswdFile (void)
{
    FILE	*fp;
    int		userIndex;
    char	fname[256],*env;
    
    if ((env = getenv(IV_HOME)) == NULL) 
    {
        fprintf(stderr,"[mmcd_savePasswdFile] not found %s environment name\n", IV_HOME);
        return -1;
    }
    sprintf(fname,"%s/%s", env, MML_PASSWD_FILE);
    
    // delete old_file
    unlink(fname);
#if 0
    if (unlink(fname) < 0) 
    {
        fprintf(stderr,"[mmcd_savePasswdFile] unlink fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
        return -1;
    }
#endif
    
    if ((fp = fopen(fname,"w")) == NULL) 
    {
        fprintf(stderr,"[mmcd_savePasswdFile] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
        return -1;
    }
    
#if 0 /* jhnoh : 030917 */
    for (userIndex=0; userIndex<MML_NUM_TP_USER_TBL && strcmp(mmcdUserTbl[userIndex].userName, ""); userIndex++) 
    {
        fprintf(fp,"%s:%s:%d:%d:%d\n",
                mmcdUserTbl[userIndex].userName,
                mmcdUserTbl[userIndex].passwd,
                mmcdUserTbl[userIndex].privilege,
                mmcdUserTbl[userIndex].lastLoginTime,
                mmcdUserTbl[userIndex].lastLogoutTime);
    }
#else
    for (userIndex=0; userIndex<MML_NUM_TP_USER_TBL; userIndex++) 
    {
        if (strcmp(mmcdUserTbl[userIndex].userName, "")) 
        {
            fprintf( fp, "%s:%s:%d:%d:%d:%s\n",
                     mmcdUserTbl[userIndex].userName, mmcdUserTbl[userIndex].passwd, 
                     mmcdUserTbl[userIndex].privilege, ( int )mmcdUserTbl[userIndex].lastLoginTime, 
                     ( int )mmcdUserTbl[userIndex].lastLogoutTime, mmcdUserTbl[userIndex].name );		//2009.07.17 by sjs
        }
    }
#endif
    
    fclose(fp);
    
    return 1;
    
} //----- End of mmcd_savePasswdFile -----//


//-----------------------------------------------------------------------------------
// cmd file에 syntax error가 있을 수 있으므로 새로운 영역을 할당해 cmdTbl을 구성한 후
//	성공적인 경우에만 이전에 사용하던 mmlCmdTbl을 free하고 새로운 놈으로 바꾼다.
// GUI에서 참조하는 명령어 DB table을 update한다.
//-----------------------------------------------------------------------------------
int mmcd_rebuildCmdTbl (char *errBuf)
{
    int		cmdCnt;
    
    memset(mmlCmdTbl, 0x00, sizeof(MMLCmdContext) * MML_NUM_TP_CMD_TBL);
    memset(mmlHelpTbl, 0x00, sizeof(MMLHelpContext) * MML_NUM_TP_CMD_TBL);
    
    // 새로운 영역에 table을 다시 구성한다.
    //
    if ((cmdCnt = mmcd_loadCmdTbl (mmlCmdTbl, mmlHelpTbl, errBuf)) < 0) 
    {
        return -1;
    }

    MML_NUM_CMD = cmdCnt;
    
    // 명령어 리스트와 syntax를 DB에 넣는다.
    //
    mmcd_saveCmdInfo2DB (mmlCmdTbl, mmlHelpTbl, MML_NUM_CMD);
    
    return cmdCnt;
    
} //----- End of mmcd_rebuildCmdTbl -----//


//------------------------------------------------------------------------------
int attchSfdb ()
{
    char    *env, tmp[64], fname[256];
    int     shmId, key;

    if ((env = getenv(IV_HOME)) == NULL) 
    {
        fprintf(stderr,"not found %s environment name\n", IV_HOME);
        return -1;
    }
    sprintf (fname, "%s/%s", env, SYSCONF_FILE);

    if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_SFDB", 1, tmp) < 0)
        return -1;
    key = strtol(tmp,0,0);

    if ((shmId = (int)shmget (key, SFM_SFDB_SIZE, 0666)) < 0) 
    {
        fprintf(stderr,"SFDB shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return -1;
    }

    if ((sfdb = (SFM_sfdb*) shmat (shmId,0,0)) == (SFM_sfdb*)-1) 
    {
        fprintf(stderr,"SFDB shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return -1;
    }
    return 1;
} //----- End of attchSfdb -----//


int mmcd_loadIPTbl( void )
{
    FILE	*fp;
    int		tblIndex = 0;
    char	fname[256],lineBuf[256],*env,*token,*ptr,*next;

    if ((env = getenv(IV_HOME)) == NULL) 
    {
        fprintf(stderr,"[mmcd_loadIPTbl] not found %s environment name\n", IV_HOME);
        return -1;
    }
    sprintf(fname,"%s/%s", env, MML_IP_FILE);
  
    if ((fp = fopen(fname,"r")) == NULL) 
    {
        fprintf(stderr,"[mmcd_loadIPTbl] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
        return -1;
    }
    
    while(fgets(lineBuf,sizeof(lineBuf),fp) != NULL)
    {
        ptr = lineBuf;
        
        // ipaddress
        token = (char*)strtok_r(ptr,":",&next);
        strcpy(mmcdIPTbl[tblIndex].ipAddress, token);
        
        // add user_name
        ptr = next;
        token = (char*)strtok_r(ptr,":",&next);
        strcpy(mmcdIPTbl[tblIndex].userName, token);

        ptr = next;
        token = (char*)strtok_r(ptr,":",&next);
        mmcdIPTbl[tblIndex].listFlag = ( char )strtol(token,0,0);
		tblIndex++;
    } //-- end of while(fgets()) --//
    fclose(fp);

    return 1;
}

int mmcd_saveIPFile( void )
{    
    FILE	*fp;
    int		tblIndex;
    char	fname[256],*env;

    if ((env = getenv(IV_HOME)) == NULL) 
    {
        fprintf(stderr,"[mmcd_saveIPFile] not found %s environment name\n", IV_HOME);
        return -1;
    }

    sprintf(fname,"%s/%s", env, MML_IP_FILE);
    
    // delete old_file
    unlink(fname);
    
    if ((fp = fopen(fname,"w")) == NULL) 
    {
        fprintf(stderr,"[mmcd_saveIPFile] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
        return -1;
    }
    
    for( tblIndex=0; tblIndex < MML_NUM_IP_TBL; tblIndex++ ) 
    {
        if( mmcdIPTbl[tblIndex].listFlag == 1 )
        {
            fprintf( fp, "%s:%s:%d\n", mmcdIPTbl[tblIndex].ipAddress, 
                     mmcdIPTbl[tblIndex].userName, mmcdIPTbl[tblIndex].listFlag );
        }
    }
    fclose(fp);
    return 1;
}
