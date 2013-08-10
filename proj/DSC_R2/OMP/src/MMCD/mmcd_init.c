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
    
    //  cpu overload check�ϱ� ���ؼ�
    if ( attchSfdb () < 0)
        return -1;
    
    
    //
    // �ʿ��� �������� table ������ �Ҵ��Ѵ�.
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
    
    // user table�� loading�Ѵ�.
    if (mmcd_loadUserTbl () < 0)
        return -1;
    
    // command table�� loading�Ѵ�.
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
    
    // config file���� �ڽŰ� ixpc�� message queue key�� �о�, attach queue
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

    // config file���� �ڽ��� system name�� �д´�.
    // �Ʒ������� sysName�� �д� ���� �ƴϰ�  sysType�� �д� ���̹Ƿ� �ּ�ó���ϰ���.
    //if (conflib_getNthTokenInFileSection (fname, "[ASSOCIATE_SYSTEMS]", "BSDM", 1, mySysName) < 0)
	//	return -1;
    
	// config file���� �ڽ��� bind port number�� �о� binding�Ѵ�.
	//
    if (conflib_getNthTokenInFileSection (fname, "[SOCKET_PORT]", "MMCD", 1, tmp) < 0)
        return -1;
    port = strtol(tmp,0,0);
    if (socklib_initTcpBind(port) < 0)
        return -1;
    
    // ��ɾ� ��� �޽����� header�� ���� system_label�� �д´�.
    //
    if (conflib_getNthTokenInFileSection (fname, "[GENERAL]", "SYSTEM_LABEL", 1, sysLabel) < 0)
        return -1;
    
    
    // log file���� open�Ѵ�.
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
// error �޽����� ���� error log file�� trace �޽����� ���� log file�� open�Ѵ�.
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
// ��ϵ� user ������ ����Ǿ� �ִ� passwd ������ �о� user table�� setting�Ѵ�.
// nmsib���� �����ϴ� userName�� passwd�� ��ϵǾ� �ִ� ���Ͽ��� userName�� �о�
//	nmsibFlag�� setting�Ѵ�.
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
            strncpy( mmcdUserTbl[userIndex].name, token, strlen( token ) - 1 );		// \n ������ -1
        }
        userIndex++;
        
    } //-- end of while(fgets()) --//
    fclose(fp);
    
    //
    // nmsib���� �����ϴ� userName�� passwd�� ��ϵǾ� �ִ� ������ �о� ��ϵ� userName��
    //  ��ġ�ϴ� ���� nmsibFlag�� setting�Ѵ�.
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
// user table�� ����� ������ passwd file�� dump�Ѵ�.
// - ���� ������ ������ �� �ٽ� �����.
// - user_name, passwd, privilege�� ����ȴ�.
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
// cmd file�� syntax error�� ���� �� �����Ƿ� ���ο� ������ �Ҵ��� cmdTbl�� ������ ��
//	�������� ��쿡�� ������ ����ϴ� mmlCmdTbl�� free�ϰ� ���ο� ������ �ٲ۴�.
// GUI���� �����ϴ� ��ɾ� DB table�� update�Ѵ�.
//-----------------------------------------------------------------------------------
int mmcd_rebuildCmdTbl (char *errBuf)
{
    int		cmdCnt;
    
    memset(mmlCmdTbl, 0x00, sizeof(MMLCmdContext) * MML_NUM_TP_CMD_TBL);
    memset(mmlHelpTbl, 0x00, sizeof(MMLHelpContext) * MML_NUM_TP_CMD_TBL);
    
    // ���ο� ������ table�� �ٽ� �����Ѵ�.
    //
    if ((cmdCnt = mmcd_loadCmdTbl (mmlCmdTbl, mmlHelpTbl, errBuf)) < 0) 
    {
        return -1;
    }

    MML_NUM_CMD = cmdCnt;
    
    // ��ɾ� ����Ʈ�� syntax�� DB�� �ִ´�.
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
