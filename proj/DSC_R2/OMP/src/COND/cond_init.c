#include "cond_proto.h"

extern int	condQid, ixpcQid, nmsifQid;
extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern char	trcBuf[4096], trcTmp[1024];
extern int	almLogId, stsLogId, traceId;
extern int	trcLogId, trcErrLogId, trcFlag, trcLogFlag;

extern InhMsgTbl *inhMsg;
char iNHMSG_FILENAME[256];
int shmFLAG;

//char	ver[8] = "R2.2.3";
char	ver[8] = "R1.0.0";

InhMsgTbl *InitInhMsgTbl(void);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int cond_initial (void)
{
	char	*env, tmp[64], fname[256];
	int	key, portNum;

	if ((env = getenv(MY_SYS_NAME)) == NULL) {
		fprintf(stderr,"[cond_init] not found %s environment name\n", MY_SYS_NAME);
		return -1;
	}
	strcpy (mySysName, env);
	strcpy (myAppName, "COND");
	commlib_setupSignals (NULL);

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[cond_init] not found %s environment name\n", IV_HOME);
		return -1;
	}

	if(set_proc_version(OMP_VER_INX_COND, ver) < 0){
		fprintf(stderr, "[cond_init] setting process version failed\n");
		return -1;
	}
	
	sprintf (fname, "%s/%s", env, SYSCONF_FILE);
	sprintf (iNHMSG_FILENAME, "%s/%s", env, INH_MSG_INFO);
	
	printf("fname = %s\n", fname);

	// config file에서 message queue key를 읽어, attach
	//
	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "COND", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((condQid = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[cond_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "IXPC", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((ixpcQid = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[cond_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	// GUI가 접속할 자신의 bind port를 읽어 binding한다.
	//
	if (conflib_getNthTokenInFileSection (fname, "SOCKET_PORT", "COND", 1, tmp) < 0)
		return -1;
	portNum = strtol(tmp,0,0);
	if (socklib_initTcpBind (portNum) < 0)
		return -1;

	//jean add
    	if ((inhMsg = (InhMsgTbl*) malloc (sizeof(InhMsgTbl))) == NULL) {
        	fprintf(stderr,"[cond_initial] malloc fail (inhMsg)\n");
        	return -1;
    	}
    
	memset(inhMsg, 0x00, sizeof(InhMsgTbl));
    	// inhMsg table을 loading한다.
    	Load_InhMsg_Info((void *)0);
#if 0 //jean
	inhMsg = (InhMsgTbl *)InitInhMsgTbl();
	if ( inhMsg == NULL ){
		fprintf (stderr,"[COND] ERROR INH MSG TABLE SHM ERROR\n");
		exit (1);
	}
#endif

	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "NMSIF", 1, tmp) < 0)
        	return -1;
    
	key = strtol(tmp,0,0);
    	if ((nmsifQid = msgget (key, IPC_CREAT|0666)) < 0) {
        	fprintf(stderr,"[cond_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        	return -1;
    	}

	// log file들을 open한다.
	//
	if (cond_initLog () < 0)
		return -1;

	//
	if (keepalivelib_init (myAppName) < 0)
		return -1;

	logPrint (trcLogId,FL,"%s startup...\n", myAppName);
	logPrint (trcErrLogId,FL,"%s startup...\n", myAppName);

	cond_transmit_initReqMsg();

	return 1;
	
} //----- End of cond_initial -----//


#if 1 //jean
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*
 ** Shared Memory 생성
 */
int InitShmm( int key, int size, void **shmPtr)
{
	int   shmId;

   	shmFLAG=0;
   	shmId = shmget(key, size, IPC_CREAT|IPC_EXCL|0644);

   	if (shmId < 0) {
      		if (errno == EEXIST) {
         		shmId = shmget(key, size, 0);
         		if (shmId < 0) {
            			fprintf (stderr,"[InitShm] : shmget failed(1) key=0x%x size=%d (%s)\n",key,size,strerror(errno));
            			return (-1);
         		}
         
			*shmPtr = (void *)shmat(shmId, NULL, 0);

         		if (*shmPtr == (void *)-1) {
            			fprintf (stderr,"[InitShm] : shmat failed(1) key=0x%x size=%d (%s)\n",key,size,strerror(errno));
            			return (-1);
         		}
         
			return shmId; // 음수 값이 리턴된다
      		} else {
         		fprintf (stderr,"[InitShm] : shmget failed(2) key=0x%x size=%d (%s)\n",key,size,strerror(errno));
         		return (-1);
      		}
   	}
   
	*shmPtr = (void *)shmat(shmId, NULL, 0);

   	if (*shmPtr == (void *)-1) {
      		fprintf (stderr,"[InitShm] : shmat failed(2) key=0x%x size=%d (%s)\n",key,size,strerror(errno));
      		return (-1);
   	}
   	fprintf (stderr," shared memory key 0x%x cleared\n", key );

   	memset (*shmPtr, 0x0, size);
   	shmFLAG=1;
   	return shmId;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int Load_InhMsg_Info (void* arg)
{
   FILE     *fp;
   char buff[1024],token[4][1024];
   int     count=0;

   memset(inhMsg, 0x00, sizeof(InhMsgTbl));

   if ( (fp = fopen (iNHMSG_FILENAME,"r" )) == NULL ){
      fprintf (stderr,"[INH_MSG_INFO] ERROR loading %s file not found\n",iNHMSG_FILENAME );
      exit (0);
   }

   while ( fgets ( buff, 1024, fp ) != NULL )
   {
      if ( buff[0] == '#' || buff[0] == '\n' ) continue;
      if ( !strncasecmp ( buff, "END", 3  ) ) break;

      memset ( token, 0, sizeof(token) );
      sscanf (buff,"%s %s %s %s", token[0], token[1], token[2], token[3]);

      inhMsg->msgFlag[count] = atoi(token[0]);
      strncpy(inhMsg->msgType[count],token[1],strlen(token[1]));
      strncpy(inhMsg->msgNum[count],token[2],strlen(token[2]));
      strncpy(inhMsg->msgInfo[count],token[3],strlen(token[3]));
      count++;
   }

   //fprintf(stderr,"INH MSG CNT = %d LOAD\n",count);

   fclose (fp);

   return 1;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define     TMP_INH_MSG_INFO      "/tmp/InhMsgInfo.tmp"

int Write_InhMsg_Info (InhMsgTbl *InhMsgt)
{
   int     i,count=0;
   FILE     *fp,*rp;
   char buff[1024];

   if ( (rp = fopen (iNHMSG_FILENAME,"r" )) == NULL )
   {
      fprintf (stderr,"[INH_MSG_INFO] ERROR loading %s file not found\n",iNHMSG_FILENAME );
      exit (0);
   }

   if ( (fp = fopen (TMP_INH_MSG_INFO,"w" )) == NULL )
   {
      fprintf (stderr,"[TMP_INH_MSG_INFO] ERROR loading %s file not found\n",TMP_INH_MSG_INFO );
      return -1;
   }

   while ( fgets ( buff, 1024, rp ) != NULL )
   {
      if ( buff[0] == '#' || buff[0] == '\n' )
      {
         fprintf (fp,"%s",buff );
      }
      else break;
   }
   fclose (rp );

   for(i = 0 ; i< MAX_INH_MSG_CNT; i++){
   	  if( InhMsgt->msgType[i][0] != 0 ){
      	fprintf(fp,"%d %s %s %s\n", InhMsgt->msgFlag[i], InhMsgt->msgType[i], InhMsgt->msgNum[i], InhMsgt->msgInfo[i]);
      	count++;
      }
   }

   fprintf (fp,"%s\n","END" );

   fclose (fp );

   sprintf (buff,"cp %s %s",TMP_INH_MSG_INFO,iNHMSG_FILENAME );

   return system ( buff );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
InhMsgTbl *InitInhMsgTbl(void)
{
    InhMsgTbl *InhMsgt=NULL;

    shmFLAG=0;
    if( InitShmm(S_SHM_INH_MSG_INFO, sizeof(InhMsgTbl), (void **)&InhMsgt)<0 )
        return((InhMsgTbl *)0);


    if( shmFLAG ) {
        printf("load_InhMsg_Info_Tbl\n");
        Load_InhMsg_Info((void *)InhMsgt);
    }

    return (InhMsgt);
}
#endif //jean 

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int cond_initLog (void)
{
	char	*env, fname[256];

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[cond_initLog] not found %s environment name\n", IV_HOME);
		return -1;
	}

	sprintf (fname, "%s/%s.%s", env, COND_TRCLOG_FILE, mySysName);
	if ((trcLogId = loglib_openLog (fname,
			LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
			LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
		fprintf(stderr,"[cond_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	sprintf (fname, "%s/%s.%s", env, COND_ERRLOG_FILE, mySysName);
	if ((trcErrLogId = loglib_openLog (fname,
			LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
			LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
		fprintf(stderr,"[cond_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	sprintf (fname, "%s/%s", env, COND_ALMLOG_FILE);
	if ((almLogId = loglib_openLog (fname,
			LOGLIB_MODE_ONE_DIR | LOGLIB_FLUSH_IMMEDIATE)) < 0) {
		fprintf(stderr,"[cond_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	sprintf (fname, "%s/%s", env, COND_STSLOG_FILE);
	if ((stsLogId = loglib_openLog (fname,
			LOGLIB_MODE_ONE_DIR | LOGLIB_FLUSH_IMMEDIATE)) < 0) {
		fprintf(stderr,"[cond_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	/*** TRACE 메시지를 받았을 경우 */ 
	sprintf (fname, "%s/%s", env, COND_TRACE_FILE);
	if ((traceId = loglib_openLog (fname,
			LOGLIB_MODE_ONE_DIR | LOGLIB_FLUSH_IMMEDIATE)) < 0) {
		fprintf(stderr,"[cond_initLog] Tracing Log openLog fail[%s]\n", fname);
		return -1;
	}

	return 1;

} //----- End of cond_initLog -----//

void cond_transmit_initReqMsg()
{
    GeneralQMsgType     txGenQMsg;
    IxpcQMsgType        *txIxpcMsg;
    char                *sysName[2] = {"SCMA", "SCMB"};
    int                 i, txLen;
    
    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;

    memset((void *)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    txGenQMsg.mtype = MTYPE_TRANINIT_REQUEST;

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);

    for ( i = 0; i < 2 ; i++)
    {
        strcpy (txIxpcMsg->head.dstSysName, sysName[i] );
        strcpy (txIxpcMsg->head.dstAppName, "SAMD");
        txLen = sizeof(txIxpcMsg->head);
        if(trcLogId) {
            sprintf(trcBuf, "[ReqMsg] SysName = %s, AppName = %s\n", txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName);
            trclib_writeLog(FL, trcBuf);
        }
        if ( msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0 )
        {
            sprintf(trcBuf, "NO Transmit log init Req Error[%d] = %s\n", errno, strerror(errno));
            trclib_writeErr (FL,trcBuf);
        }
        commlib_microSleep(100);
    }

}

