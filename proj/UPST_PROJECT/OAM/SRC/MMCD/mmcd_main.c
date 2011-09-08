/**
	@file		mmcd_main.c
	@author		hhbaek
	@date		2011-07-13
	@version
	@brief		mmcd_main.c	
*/

/**
*	Include Headers
*/

/* SYS HEADER */
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <mysql/mysql.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <unistd.h>
/* LIB HEADER */
#include "typedef.h"
#include "commdef.h"
#include "nifo.h"
#include "cifo.h"
#include "gifo.h"
#include "mems.h"
#include "dblib.h"
#include "loglib.h"
#include "verlib.h"
/* PRO HEADER */
#include "procid.h"
#include "path.h"
#include "mmcdef.h"
#include "msgdef.h"
#include "sshmid.h"
#include "sockio.h"
#include "procid.h"
#include "msgdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "cmd_get.h"
#include "mmcd_sock.h"
#include "mmcd_init.h"

/**
* Define constants
*/
#define MMCD_LOG					LOG_PATH"MMCD"
#define MAX_STRING_BUFFER			262144
#define CLIENT_CHECK_TIME			5
#define DEF_SYS_MMCD				7
#define MAX_COM_SYMBOL				512
#define MAX_PARA_SYMBOL     		2048

/**
* Declare variables
*/
stMEMSINFO		*gpRECVMEMS;
stCIFO          *pCIFO;

char	vERSION[7] = "R4.0.0";

extern  int gdPortNo;
extern  int gdClient;
extern  int gdMaxCliCnt;

RUN_TBL     JtRunTable;

extern void SetUpSignal();
extern void FinishProgram();
extern int dTcpSvrModeInit();
extern int dInitIPCs();
extern int GetUserInfo(char *UserName, int Type);
extern int set_version(key_t key, int prc_idx, char *ver);
extern int dIsRcvedMessage();
extern char* util_cvtipaddr(char* szIP, unsigned int uiIP);
extern int dRecvMessage(int dRsfd, char *szTcpRmsg, int* dRecvLen);
extern int dGetMaxFds();
extern int dMergeBuffer(int dIdx, int dSfd, char* szTcpRmsg, int dRecvLen);
extern int dSendBlockMessage(int dSsfd, int dMsgLen, char *szSmsg, int stConIdx);
extern int dSetSockBlock(int dReturn, int dSockFD, int stConIdx);
extern int Init_tbl();
extern int dAdminInfoInit();
extern char *ComposeHead();
extern int dGetConTblIdx(int dSockfd);
extern int dSendMessage(int dSsfd, int dMsgLen, char *szSmsg, int stConIdx);
extern int dGetNMSsfd();
extern int SendToPROC(st_MsgQ *in_msg, int MsgID);
extern void util_makenid(unsigned char ucSysType, long long *pllNID);

int SendToOMP(dbm_msg_t*, unsigned short);
void clear_my_tmr(short i);
int out_print(char *outbuf, dbm_msg_t *msg, unsigned short usLen, short tmrID, short sRet, short cont_flag);
void Timer(time_t tTime);
int dCheckStatFlag(short StatFlag);
int dInsert_MMCD_Result(st_MngPkt *pstMngPkt, short tmrID, char *szBuf, int dSockfd);
int init_mmc_server();
int dSocketCheck();
extern void CheckClient(int dReloadFlag);

/**
 *	Implement func.
 */
int main(int argc, char *argv[])
{
	int			dRet, i, loopCnt;
	int			dNmsIndex;
	//st_MsgQ  	stMsg;
	pst_MsgQ	pstMsg;				/* msgq ==> gifo */
	pst_MsgQSub pstMsgQSub;
	int			dWarnOutCnt = 0;	/* 20040923,poopee */
	char		szCommand[64];
    time_t		tCurTime, tCheckTime, tClientCheck, tLastPingMySQL;
	OFFSET		offset;

	SetUpSignal();

	gdStopFlag = 1;
	gdNumfds = 0;
	gdNumWfds = 0;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_MMCD, LOG_PATH"/MMCD", "MMCD");

	/*
	AppLogInit(getpid(), SEQ_PROC_MMCD, MMCD_LOG, "MMCD");
	dRet = dInitLogShm();
	if( dRet < 0 )
		exit(dRet);

	dRet = Init_shm_common();
    if(dRet < 0)
    {
        log_print( LOGN_CRI,"MAIN : Failed in Init_shm_common dRet[%d][%s]", dRet, strerror(errno));
        exit(1);

	*/

	if((dRet = get_db_conf(FILE_MYSQL_CONF, szIP, szName, szPass, szAlias)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN get_db_conf() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(-5);
	}

	if((dRet = db_conn(&stMySQL, szIP, szName, szPass, szAlias)) < 0)
	{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN db_conn() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(-6);
	}

	/*
	if( (dRet = dInitialMysqlEnvironment(&stConnInfo)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dInitialMysqlEnvironment() dRet[%d]",
			__FILE__, __FUNCTION__, __LINE__, dRet);
		exit(-2);
	}

	if( (dRet = dConnectMySQL(&stMySQL, &stConnInfo)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dConnectMySQL() dRet[%d]",
			__FILE__, __FUNCTION__, __LINE__, dRet);
		exit(-3);
	}
	*/

	/*
	* INITIALIZE FOR MMC COMMAND
	*/
	dRet = init_mmc_server();
	if( dRet < 0 ) {
		log_print(LOGN_CRI, "[ERROR] INIT_MMC_SERVER RET[%d]\n", dRet );
		FinishProgram();
		exit(0);
	}
	/*
	* INITIALIZE FOR SHARED MEMORY FIDB
	*/
/*
	dRet = Init_Fidb();
    if( dRet < 0 ) {
        log_print(LOGN_CRI,"[ERROR] FIDB INIT [%d]\n", dRet );
        FinishProgram();
        exit(0);
    }
*/

	/*
	* INITIALIZE SOCKET
	*/
	dRet = dTcpSvrModeInit();
	if( dRet < 0 ) {
		FinishProgram();
		exit(0);
	}

	/*
	* INITIALIZE FOR SOCKET CONNECTION TABLE
	*/
	for( i = 0 ; i < MAX_FDSET2	; i++ ) {
		stConTbl[i].dSfd = -1;
		stConTbl[i].uiCliIP = 0;
		stConTbl[i].dBufLen = 0;
        stConTbl[i].szSendBuffer[0] = 0;
        stConTbl[i].cSockBlock = 0x00;
		gstMsgBuf[i].dWidx = 0;
		gstMsgBuf[i].dRidx = 0;
		memset( gstMsgBuf[i].szBuf, 0x00, sizeof(MAX_MNG_PKT_BUFSIZE) );
	}

	/*
	* GET MESSAGE QUEUE ID USING IN MMCD
	*/
    dRet = dInitIPCs();
	if( dRet < 0 ) {
		log_print(LOGN_CRI,"[ERROR] MyQid GET RET[%d]", dRet );
		FinishProgram();
		exit(0);
	}

	/* moved from init_mmc_server() */
	/* CLEAN UP ALL WARN-OUT DATA */
	for(;;) 
	{

/* msgq ==> gifo */
#if 0
		dRet = msgrcv(gdMyQid, &stMsg, sizeof(st_MsgQ) - sizeof(long int), 0, IPC_NOWAIT | MSG_NOERROR);
		if( dRet <= 0) break;
		else dWarnOutCnt++;
#endif
		offset = gifo_read(gpRECVMEMS, pCIFO, SEQ_PROC_MMCD);
		if(offset <= 0)
		{
			usleep(0);
			break;
		}
		else
		{
			dWarnOutCnt++;
			//pstMsg = (pst_MsgQ)nifo_get_value(gpRECVMEMS, DEF_MSGQ_NUM, offset);
			nifo_node_delete(gpRECVMEMS, nifo_ptr(gpRECVMEMS, offset));
		}	
	}
	log_print(LOGN_INFO, "INITIAL MESSAGE QUEUE WARN-OUT COUNT : [%d]", dWarnOutCnt );

	gdIndex = 0;
	dNmsIndex = GetUserInfo("ntasnms", 1 );

	time(&tCurTime);
	tClientCheck = tCheckTime = tCurTime;

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_MMCD, vERSION)) < 0)
	{
		log_print(LOGN_WARN, "set_version error(ret=%d,idx=%d,ver=%s)\n", dRet, SEQ_PROC_MMCD, vERSION);
	}
	log_print(LOGN_CRI, "MMCD %s Started", vERSION);

	/*
	* MAIN WHILE LOOP
	*/
	while(gdStopFlag)
	{

		loopCnt = 0;
		while(1)
		{
			/* msgq ==> gifo */
			if( (dRet = dIsRcvedMessage(&pstMsg)) > 0)
			{
				pstMsgQSub	= (pst_MsgQSub)&pstMsg->llMType;
				log_print(LOGN_INFO, "RECEIVED MSGQ: Type[%d] SvcID[%d] MsgID[%d] usBodyLen[%d]", pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, pstMsg->usBodyLen);
				if( (pstMsgQSub->usSvcID == SID_PATCH) && (pstMsgQSub->usMsgID == MID_SMNG_START))
				{
					sprintf(szCommand, "/WNTAMAPP/BIN/StartMC -b S_MNG");
					system(szCommand);
					log_print(LOGN_INFO, "S_MNG START MESSAGE GET");
					continue;
				}
				SendToOMP( (dbm_msg_t*)&pstMsg->szBody[0], pstMsg->usBodyLen);
				
				// TODO 제대로 된 코드인지 확인 필요
				nifo_node_delete(gpRECVMEMS, nifo_ptr(gpRECVMEMS, nifo_get_offset_node(gpRECVMEMS, (U8*)pstMsg)));
			}
			else
			{
				usleep(0);
				loopCnt++;
				if( loopCnt > 100 ){
					break;
				}
			}

#if 0
			memset(&stMsg, 0x00, sizeof(st_MsgQ));
			if( (dRet = dIsRcvedMessage(&stMsg)) > 0)
			{
				pstMsgQSub	= (pst_MsgQSub)&stMsg.llMType;
				log_print(LOGN_INFO, "RECEIVED MSGQ: Type[%d] SvcID[%d] MsgID[%d] usBodyLen[%d]", pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, stMsg.usBodyLen);
				if( (pstMsgQSub->usSvcID == SID_PATCH) && (pstMsgQSub->usMsgID == MID_SMNG_START))
				{
					sprintf(szCommand, "/WNTAMAPP/BIN/StartMC -b S_MNG");
					system(szCommand);
					log_print(LOGN_INFO, "S_MNG START MESSAGE GET");
					continue;
				}
				SendToOMP( (dbm_msg_t*)&stMsg.szBody[0], stMsg.usBodyLen);

				
				
				nifo_node_delete(gpRECVMEMS, nifo_ptr(gpRECVMEMS, offset));
			}
			else if(dRet < 0)
			{
				log_print(LOGN_CRI, "[ERROR] dIsRcvedMessage PROGRAM FINISHED");
				exit(0);
			}
			else
				break;
#endif

		}

		/*
		* MAIN SOCKET CHECK
		*/
		dRet = dSocketCheck();

		/*
		* CHECK COMMAND TIMEOUT
		*/
		time(&tCurTime);

		if(abs(tCurTime-tCheckTime) >= 1)
		{
			Timer(tCurTime);
			tCheckTime = tCurTime;
		}

		/*
		* CHECK CLIENT FOR UNUSUAL CLOSE : CLIENT_CHECK_TIME(10)
		*/
		if(abs(tCurTime-tClientCheck) > CLIENT_CHECK_TIME)
		{
			CheckClient(0);
			tClientCheck = tCurTime;
		}

		if( (tCurTime - tLastPingMySQL) > SEC_OF_HOUR)
		{
			//if( (dRet = dPingMySQL(&stMySQL)) < 0)
			if((dRet = db_check_alive(&stMySQL)) < 0)
			{
				log_print(LOGN_CRI,"F=%s:%s.%d: ERROR IN dPingMySQL() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
				return -16;
			}
			tLastPingMySQL = tCurTime;
		}
	}
	FinishProgram();
	log_print(LOGN_CRI, "[EXIT] PROGRAM END !!!\n");

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dAcceptClient()
{
	int		i;
	int 	dSfd, dOpt;
	int     dSBufLen, dSBufLen1, dRBufLen, dRBufLen1;
	int		dCliLen = sizeof(struct sockaddr_in);
	struct 	in_addr inaddr;
	struct	linger	stLinger;
	struct 	sockaddr_in stCli_addr;

	dSfd = accept( gdSvrSfd, (struct sockaddr*)&stCli_addr, &dCliLen );
	if( dSfd < 0 ) {
		log_print( LOGN_CRI, "[SOCKET] NEW CLIENT ACCEPT FAIL : no[%d]%s",
			errno, strerror(errno));
		return -1;
	}

	dOpt = 1;
   	stLinger.l_onoff = 0;
   	stLinger.l_linger = 0;

	dSBufLen = 87380;
    dSBufLen1 = sizeof(dSBufLen);

    dRBufLen = 87380;
    dRBufLen1 = sizeof(dRBufLen);

	/*
	*  SET SOCKET OPTIONS
	*/
	if(setsockopt(dSfd, SOL_SOCKET, SO_KEEPALIVE , (void *)&dOpt, sizeof(dOpt)) < 0) {
   		log_print(LOGN_CRI, "FAIL IN SET TO KEEPALIVE, errno=[%d] %s",
			errno, strerror(errno));
       	close(dSfd);
		return -2;
   	}
   	else if(fcntl(dSfd, F_SETFL, O_NDELAY) < 0) {
       	log_print(LOGN_CRI, "FAIL IN SET TO NON-BLOCKED IO, errno =[%d] %s",
			errno, strerror(errno));
       	close(dSfd);
		return -3;
   	}
   	else if(setsockopt(dSfd,SOL_SOCKET,SO_LINGER,(void *)&stLinger,sizeof(stLinger)) < 0) {
       	log_print(LOGN_CRI,
			"FAIL IN CHANGING SOCKET OPTION : LINGER, errno=[%d] %s",
			errno, strerror(errno));
       	close (dSfd);
		return -4;
   	}
	else if(setsockopt(dSfd, SOL_SOCKET, SO_SNDBUF, (char *)&dSBufLen, dSBufLen1) < 0) {
        log_print(LOGN_CRI,
            "FAIL IN CHANGING SOCKET OPTION : SEND BUFFER, errno=[%d] %s",
            errno, strerror(errno));
        close (dSfd);
        return -5;

    }
    else if(setsockopt(dSfd, SOL_SOCKET, SO_RCVBUF, (char *)&dRBufLen, dRBufLen1) < 0) {
        log_print(LOGN_CRI,
            "FAIL IN CHANGING SOCKET OPTION : RECEIVE BUFFER, errno=[%d] %s",
            errno, strerror(errno));
        close (dSfd);
        return -6;
    }

	for(i=0; i<gdClient; i++ ) {
		if( stConTbl[i].dSfd < 0 )
			break;
	}

	if(i == gdMaxCliCnt) {
		log_print(LOGN_CRI, "[INFO] MAX CLIENT OVERFLOW  IDX[%d] sfd[%d]", i, dSfd);
		close(dSfd);
		return -5;
	}

	stConTbl[i].dSfd = dSfd;
	stConTbl[i].cSockBlock = 0x00;	/*** INITIAL NON-BLOCK ***/

	gstMsgBuf[i].dWidx=0;
	gstMsgBuf[i].dRidx=0;

    FD_SET( dSfd, &gstReadfds);

	if( dSfd > gdNumfds )
        gdNumfds = dSfd;

	if( i >= gdClient )
		gdClient = i + 1;

    stConTbl[i].uiCliIP = stCli_addr.sin_addr.s_addr;
	inaddr.s_addr = stCli_addr.sin_addr.s_addr ;

	log_print( LOGN_CRI, "[SOCKET] [CONNECT] NEW CLIENT IP[%s] IDX[%d] Sfd[%d] GDCLIENT[%d]",
		  			   inet_ntoa(inaddr), i, stConTbl[i].dSfd, gdClient );

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dSocketCheck( )
{
	int     dSfd;
	int		dRecvLen = 0;
	int		dSelRet;
	int		dRet, dRet1, i, j;
	int		dAdminIdx;
	int		dSocketChkFlag = 0;
	char	szTcpRmsg[MAX_MNG_PKT_BUFSIZE];

	fd_set  stRd;
	fd_set	stWd;
	struct  timeval		timeout;

	memcpy((char*)&stRd, (char*)&gstReadfds, sizeof(fd_set));
	memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));

	timeout.tv_sec = 0;
	timeout.tv_usec = 10000;

	dSelRet = select( gdNumfds+1, &stRd, &stWd, (fd_set*)NULL, &timeout );
	if( dSelRet < 0 ) {
		if( errno != EINTR ) {
			/*
			* SELECT UNEXPECTED ERROR
			*/
			log_print(LOGN_CRI, "[ERROR] SELECT CRITICAL ERROR INVOKE [%d] [%s]",
							 errno, strerror(errno) );
			FinishProgram();
		}
		return 0;
	}
	else if( dSelRet > 0 ) {					/*** EVENT INVOKE ***/

		if( FD_ISSET( gdSvrSfd, &stRd ) ) {		/*** NEW CLIENT CONNECT ***/

			log_print(LOGN_DEBUG, "[SOCKET] NEW CLIENT CONNECT TRIAL SVRPORT[%d]",
							   gdPortNo );
			dSocketChkFlag++;

			dRet = dAcceptClient();
			if( dRet < 0 ) {
				log_print( LOGN_CRI, "FAIL IN FUNCTION dAcceptClient()");
			}
		}

		for( i=0; i<gdClient; i++ )	{
			if( stConTbl[i].dSfd < 0 )
				continue;

			log_print(LOGN_DEBUG, "[SOCKET] stConTbl IDX[%d] SFD[%d] dSetRet[%d] IP[%s]",
							   i, stConTbl[i].dSfd, dSelRet, util_cvtipaddr(NULL, stConTbl[i].uiCliIP) );

			dSfd = stConTbl[i].dSfd;
			if(FD_ISSET( dSfd, &stRd)) {		/*** EXIST CLIENT EVENT ***/
				log_print(LOGN_DEBUG, "[SOCKET] EXIST CLIENT EVENT IDX[%d] Sfd[%d] IP[%s]",
					i, dSfd, util_cvtipaddr(NULL, stConTbl[i].uiCliIP) );
				dSocketChkFlag++;

				dRecvLen = 0;

				dRet = dRecvMessage(dSfd, &szTcpRmsg[0], &dRecvLen );
				if( dRet < 0 ) {
					log_print(LOGN_CRI, "[SOCKET] [CLOSE  ] CONNECTION CLOSE IDX[%d] SFD[%d] IP[%s]",
									  	i, dSfd, util_cvtipaddr(NULL, stConTbl[i].uiCliIP) );

					close(dSfd);
					FD_CLR( dSfd, &gstReadfds );

					log_print(LOGN_DEBUG, "GDCLIENT INFO : [%d]", gdClient );

					if( gdClient == 1 ) {
                        gdClient--;
						gdNumfds = gdSvrSfd;
					}
                    else if( (gdClient-1) == i ) {
                        gdClient--;

                        while(1) {
                            if( stConTbl[gdClient-1].dSfd > 0 ) {
								gdNumfds = dGetMaxFds();
                                break;
							}
                            else {
                                gdClient--;

                                if( gdClient == 0 ) {
									log_print(LOGN_DEBUG, "GDNumfds INFO 4 : [%d]", gdNumfds );
									gdNumfds = gdSvrSfd;
                                    break;
								}
                            }
                        }
                    }

					/*
					* ADD 2003.05.13.
					* CLEAR WRITE FD_SET FOR SOCKET FD
					*/
            		memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));
            		if( FD_ISSET( dSfd, &stWd ) ) {
                		log_print(LOGN_DEBUG, "CLEAR WRITEFDS FOR SOCKET CLOSE SFD[%d]", dSfd );

                		FD_CLR( dSfd, &gstWritefds );

                		gdNumWfds--;
                		stConTbl[i].cSockBlock = 0x00;
            		}

					log_print(LOGN_DEBUG, "GDCLIENT INFO : [%d]", gdClient );

					stConTbl[i].dSfd = -1;
					stConTbl[i].dBufLen = 0;
					stConTbl[i].uiCliIP = 0;
					stConTbl[i].cRetryCnt = 0;

					dAdminIdx = GetUserInfo( stConTbl[i].adminid, 1 );
					stAdmin.dConnectFlag[dAdminIdx] = 0;

					stConTbl[i].adminid[0] = 0x00;

					for( j=0 ; j < MAX_TMR_REC ; j++ ) {
						if( dSfd == run_tbl->sfd[j] ) {
							clear_my_tmr( j );
						}
					}
				}
				else if(dRecvLen > 0) {
					log_print( LOGN_DEBUG, "RECV SOCK SIZE[%d]", dRecvLen );

					dRet = dMergeBuffer( i, dSfd, szTcpRmsg, dRecvLen );
					if( dRet < 0 ) {
						log_print(LOGN_CRI,
							"[SOCKET] [CLOSE  ] PACKET MERGE ERR->CONNECTION CLOSE IDX[%d] sfd[%d] Ret[%d] IP[%s]",
							i, dSfd, dRet, util_cvtipaddr(NULL, stConTbl[i].uiCliIP) );

						close(dSfd);
						FD_CLR( dSfd, &gstReadfds );

						log_print(LOGN_DEBUG, "GDCLIENT INFO : [%d]", gdClient );

						if( gdClient == 1 ) {
                            gdClient--;
							log_print(LOGN_DEBUG, "GDNumfds INFO : [%d]", gdNumfds );
							gdNumfds = gdSvrSfd;
						}
                        else if( (gdClient-1) == i ) {
                            gdClient--;

                            while(1) {
                                if( stConTbl[gdClient-1].dSfd > 0 ) {
									gdNumfds = dGetMaxFds();
                                    break;
								}
                                else {
                                    gdClient--;

                                    if( gdClient == 0 ) {
										log_print(LOGN_DEBUG, "GDNumfds INFO 2 : [%d]", gdNumfds );
										gdNumfds = gdSvrSfd;
                                        break;
									}
                                }
                            }
                        }

						/*
						* ADD 2003.05.13.
						*/
            			memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));
            			if( FD_ISSET( dSfd, &stWd ) ) {
                			log_print(LOGN_DEBUG,"CLEAR WRITEFDS FOR SOCKET CLOSE SFD[%d]", dSfd );

                			FD_CLR( dSfd, &gstWritefds );

                			gdNumWfds--;
                			stConTbl[i].cSockBlock = 0x00;
            			}

						log_print(LOGN_DEBUG, "GDCLIENT INFO : [%d]", gdClient );

						stConTbl[i].dSfd = -1;
						stConTbl[i].dBufLen = 0;
						stConTbl[i].uiCliIP = 0;
						stConTbl[i].cRetryCnt = 0;

						dAdminIdx = GetUserInfo( stConTbl[i].adminid, 1 );
						stAdmin.dConnectFlag[dAdminIdx] = 0;

						stConTbl[i].adminid[0] = 0x00;
						for( j=0 ; j < MAX_TMR_REC ; j++ ) {
							if( dSfd == run_tbl->sfd[j] ) {
								clear_my_tmr( j );
							}
						}
					}
				}
			}
		}

		/*
		* CHECK WRITE FD_SET
		*/
		if( gdNumWfds > 0 ) {
			for( i=0; i<gdClient; i++ ) {
				if( stConTbl[i].dSfd < 0  )
					continue;

				if( stConTbl[i].cSockBlock == 0x00 )
					continue;

				dSocketChkFlag++;
				dSfd = stConTbl[i].dSfd ;

				if( FD_ISSET( dSfd, &stWd ) ) {
					log_print(LOGN_DEBUG,"WriteFds Event Invoke SFD[%d]", dSfd );

					FD_CLR( dSfd, &gstWritefds );

					gdNumWfds--;
					stConTbl[i].cSockBlock = 0x00;

					/*
					* SEND SENDBUFFER DATA
					*/
					if( stConTbl[i].dBufLen > 0 ) {
						dRet = dSendBlockMessage( dSfd, stConTbl[i].dBufLen, &stConTbl[i].szSendBuffer[0], i );
						dRet1 = dSetSockBlock( dRet, dSfd, i );
					}
				}
			}
		}

		if( dSocketChkFlag == 0 && dSelRet > 0 ) {
			log_print( LOGN_INFO,
				"CHECK SOCKET SELECT dSelRet[%d] gdNumWfds[%d]",
				dSelRet, gdNumWfds);
			sleep(1);
		}
	}
	else if( dSelRet == 0 ) {
		/*
		* NO EVENT
		*/
	}

	return 0;
}


/*******************************************************************************
 SEND TO OMP
*******************************************************************************/
int SendToOMP( dbm_msg_t *rmesg , unsigned short usLen )
{
	int  dBodyLen;
	int  idx, realmsgID;

	short	CurCount;
	short	TotCount;

	static char  prn_buf2[16384];

	idx = rmesg->head.cmd_id;
	realmsgID = rmesg->head.msg_id;

	log_print(LOGN_INFO, "[INFO] PROC RECV MSG MSGID:[%d] IDX:[%d] USLEN:[%d] ", realmsgID, idx, usLen );
	log_print(LOGN_INFO, "[INFO] COMMON ERR[%d]", rmesg->common.mml_err );

	dBodyLen = usLen;

	/*
    * CASE : INVALID RUN_TBL INDEX
    */
    if( idx > (MAX_TMR_REC - 1) || idx < 0 ) {
        log_print(LOGN_CRI, "[ERROR] INVALID RUN_TBL INDEX FROM PROCESS IDX[%d] REALMSGID[%d]",
                         idx, realmsgID );

        return 0;
    }

	log_print( LOGN_INFO, "RUN_CMDID:[%d] RUN_MSGID:[%d] REALMSGID:[%d]",
						run_tbl->cmd_id[idx], run_tbl->msg_id[idx], realmsgID );

	if( run_tbl->cmd_id[idx] != 0 && run_tbl->msg_id[idx] == realmsgID ) {
		if( rmesg->common.mml_err < 0 )
        {

        }
        else {
            if( rmesg->common.curr_cnt != 0 ) {
                run_tbl->stat_cur_cnt[idx] += rmesg->common.curr_cnt;
            }

            if( rmesg->common.total_cnt != 0 ) {
                run_tbl->stat_tot_cnt[idx] = rmesg->common.total_cnt;
            }
        }

		if( run_tbl->ucbinflag[idx] == 0x00 ) {
			if( lib_tbl[realmsgID%MAX_LIB_TABLE].mmc_res != NULL ) {
				sprintf(prn_buf2, "M%04d  %s",
					lib_tbl[realmsgID%MAX_LIB_TABLE].mcode,
					lib_tbl[realmsgID%MAX_LIB_TABLE].msg_header );

				//CurCount = run_tbl->stat_TOT_NUM[idx] - run_tbl->stat_TOTAL[idx]+1;
				//TotCount = run_tbl->stat_TOT_NUM[idx];
				CurCount = run_tbl->stat_cur_cnt[idx];
				TotCount = run_tbl->stat_tot_cnt[idx];

				log_print(LOGN_DEBUG, "TUNDRA ");
				(*lib_tbl[realmsgID%MAX_LIB_TABLE].mmc_res)(prn_buf2, rmesg, &CurCount, &TotCount  );
			}
		}
		else {
			if( rmesg->common.mml_err < 0 )
			{
				sprintf(prn_buf2, "M%04d  %s",
                    lib_tbl[realmsgID%MAX_LIB_TABLE].mcode,
                    lib_tbl[realmsgID%MAX_LIB_TABLE].msg_header );

                //CurCount = run_tbl->stat_TOT_NUM[idx] - run_tbl->stat_TOTAL[idx]+1;
                //TotCount = run_tbl->stat_TOT_NUM[idx];
				CurCount = run_tbl->stat_cur_cnt[idx];
                TotCount = run_tbl->stat_tot_cnt[idx];

                log_print(LOGN_DEBUG, "TUNDRA ");
                (*lib_tbl[realmsgID%MAX_LIB_TABLE].mmc_res)(prn_buf2, rmesg, &CurCount, &TotCount  );
			}
			else
			{
				log_print(LOGN_DEBUG, "TUNDRA 3");
				dBodyLen = rmesg->head.msg_len;
				memcpy( &prn_buf2[0], &rmesg->data[0], dBodyLen );
				prn_buf2[dBodyLen] = 0x00;
			}
		}

		/*
		* SEND RESULT TO OMP, RMI, OR NMS
		*/
		out_print(&prn_buf2[0], rmesg, dBodyLen, rmesg->head.cmd_id, rmesg->common.mml_err, rmesg->common.cont_flag);
	}
	else if( run_tbl->cmd_id[idx] == 0 && run_tbl->msg_id[idx] == 0 ) {
		log_print(LOGN_DEBUG,
			"ALREADY DELETED COMMAND IDX[%d] CMD_ID[%d] MSGID[%d]",
			idx, run_tbl->cmd_id[idx], run_tbl->msg_id[idx]);

		clear_my_tmr( idx );

		return 0;
	}

	if( rmesg->common.mml_err < 0 ) {
		if( dCheckStatFlag(rmesg->common.StatFlag) == 1 ) {
            run_tbl->stat_TOTAL[idx] = run_tbl->stat_TOTAL[idx] - 1;
            run_tbl->time[idx] = TIME_OUT + run_tbl->period[idx];

            if( run_tbl->stat_TOTAL[idx] == 0 )
                clear_my_tmr( idx );
        }
        else {
			clear_my_tmr( idx );
		}
	}
	else if( rmesg->common.cont_flag == DBM_END ) {
		if( dCheckStatFlag(rmesg->common.StatFlag) == 1 ) {
			run_tbl->stat_TOTAL[idx] = run_tbl->stat_TOTAL[idx] - 1;
			run_tbl->time[idx] = TIME_OUT + run_tbl->period[idx];

			if( run_tbl->stat_TOTAL[idx] == 0 )
				clear_my_tmr( idx );
		}
		else {
			clear_my_tmr( idx );
		}
	}
	else {
		if( dCheckStatFlag(rmesg->common.StatFlag) == 1 ) {
			run_tbl->stat_cur_cnt[idx] += rmesg->common.curr_cnt;
            run_tbl->stat_tot_cnt[idx] = rmesg->common.total_cnt;
            run_tbl->stat_cur_page[idx] = rmesg->common.CurPage;
            run_tbl->stat_tot_page[idx] = rmesg->common.TotPage;
		}
	}

	return 0;
}

/*******************************************************************************

*******************************************************************************/
int init_mmc_server ()
{
	int			dRet;

    run_tbl = &JtRunTable;

    memset((char *)run_tbl, 0, sizeof (RUN_TBL));

    /*
	* LOADING IPAM_ENUM IPAM_COM
	*/
	dRet = Init_tbl();
    if (dRet <= 0) {
        log_print(LOGN_CRI,
			"[ERROR] SYSTEM CONFIGURATION : Init_tbl() RET[%d]", dRet);

        return -1;
    }

#if 0
    dRet = get_smsname();
	if( dRet < 0 ) {
		return -1;
	}
#endif

	dRet =  dAdminInfoInit();
	if( dRet < 0 ) {
		log_print(LOGN_CRI,"[ERROR] dAdminInfoInit() RET[%d]", dRet);
		return -1;
	}

    return 1;
}

#if 0
/*******************************************************************************
 GET SYSTEM NAME FOR FILE(IPAM_IP.dat)
*******************************************************************************/
int get_smsname()
{
	FILE 	*fp;

	char	szBuffer[256];
	char	szName[16];
	char	szString[16];

	if( (fp = fopen( DEF_IPAM_IP_FILE, "r" )) == NULL ) {
		log_print( LOGN_CRI, "FILE OPEN ERROR [%s]", DEF_IPAM_IP_FILE );
		return -1;
	}

	while( fgets(szBuffer, 256, fp) != NULL ) {
		if( szBuffer[0] != '#' ) {
			log_print( LOGN_CRI, "INVALID FILE FORMAT" );
			return -1;
        }

        if(szBuffer[1] == '#')
            continue;
        else if(szBuffer[1] == 'E')
            break;
        else
        {
			sscanf( &szBuffer[2], "%s %s", szName, szString );

			if( strcasecmp( szName, "LOC-IPAM") == 0 ) {
				strcpy(SmsName, szString);
				break;
			}
			else
				continue;
		}
	}

    sprintf(logfilepath, "%s", MMCD_LOG );
    return 1;
}
#endif

int get_smsname ()
{
    FILE *fa;
    char szBuf[1024];
    char szType[64];
    char szName[64];
    char szValue[64];

    fa = fopen("/WNTAMAPP/DATA/SYS_CFG", "r");
    if(fa == NULL)
    {
        log_print(LOGN_CRI,"get_smsname : %s FILE OPEN FAIL [%s]","SYS_CFG" ,strerror(errno));
        return -1;
    }
    while(fgets(szBuf,1024,fa) != NULL)
    {
        log_print(LOGN_DEBUG, "ReadBuf (%s)", &szBuf[2]);
        if(szBuf[0] != '#')
        {
            log_print(LOGN_CRI,"get_smsname : %s FILE format error", "SYS_CFG");
            fclose(fa);
            return -1;
        }

        if(szBuf[1] == '#')
            continue;
        else if(szBuf[1] == 'E')
            break;
        else if(szBuf[1] == '@')
        {
            if(sscanf(&szBuf[2],"%s %s %s",szType,szName,szValue) == 3)
            {
                if((strcmp(szType,"SMS") == 0) && (strcmp(szName,"NAME") == 0))
                {
                    sprintf (SmsName, "%s", szValue);
                }
            }
        }
    }
    fclose(fa);
    return 1;
}


/*******************************************************************************
 CLEAR [run_tbl] VALUE USING INDEX
*******************************************************************************/
void clear_my_tmr (short i)
{
    log_print(LOGN_DEBUG,
		"CLEAR MMC TMR (cmdID=%d, msgID=%d, srcProc=%d)...",
        run_tbl->cmd_id[i], run_tbl->msg_id[i], run_tbl->inv_proc[i]);
	run_tbl->llIndex[i] = 0;
	run_tbl->exe_proc[i] = 0;
    run_tbl->cmd_id[i] = 0;
    run_tbl->msg_id[i] = 0;
    run_tbl->sfd[i] = 0;
    run_tbl->time_stamp[i][0] = 0x00;
    run_tbl->user_name[i][0] = 0x00;
    run_tbl->inv_proc[i] = 0;
    run_tbl->time[i] = 0;
    run_tbl->type[i][0] = 0;
    run_tbl->period[i] = 0;
	run_tbl->stat_TOTAL[i] = 0;
	run_tbl->stat_TOT_NUM[i] = 0;
    run_tbl->stat_tot_cnt[i] = 0;
    run_tbl->stat_cur_cnt[i] = 0;
	run_tbl->stat_tot_page[i] = 0;
	run_tbl->stat_cur_page[i] = 0;
	run_tbl->usmmlid[i] = 0;
	run_tbl->blockcode[i] = 0;
	run_tbl->ucbinflag[i] = 0;
	run_tbl->stat_check_time[i] = 0;
	memset( &run_tbl->stMML_MSG[i], 0x00, sizeof(mml_msg) );
}


/*******************************************************************************

*******************************************************************************/
int out_print(char *outbuf, dbm_msg_t *msg, unsigned short usLen, short tmrID, short sRet, short cont_flag)
{
	short		CurCount, TotCount;
	int			dRet, dRet1, dNmsSfd, dNmsUserIdx, dConTblIdx, dLen;
	char		szTempBuf[MAX_MMCD_MSG_SIZE*2];
	st_MngPkt	output;

	memset(&output, 0x00, sizeof(st_MngPkt));

	if(run_tbl->ucbinflag[tmrID] == 0x00)
	{
		if(cont_flag == DBM_CONTINUE)
			sprintf(szTempBuf, "\n%s %s\n%s\nCONTINUED\n", SmsName, (char*)ComposeHead(), outbuf);
		else
			sprintf(szTempBuf, "\n%s %s\n%s\nCOMPLETED\n", SmsName, (char*)ComposeHead(), outbuf);
		output.head.usBodyLen = strlen(szTempBuf);
	}
	else
	{
		if(sRet < 0)
		{
			if(cont_flag == DBM_CONTINUE)
				sprintf(szTempBuf, "\n%s %s\n%s\nCONTINUED\n", SmsName, (char*)ComposeHead(), outbuf);
			else
				sprintf(szTempBuf, "\n%s %s\n%s\nCOMPLETED\n", SmsName, (char*)ComposeHead(), outbuf);
			output.head.usBodyLen = strlen(szTempBuf);
		}
		else
		{
			memcpy(szTempBuf, &outbuf[0], usLen);
			output.head.usBodyLen = usLen;
		}
	}

	if(output.head.usBodyLen >= MAX_MNGPKT_BODY_SIZE)
	{
		log_print(LOGN_WARN, "MMCD OUTPUT PRINT BODY MESSAGE OVER FLOW MAX=%d CUR=%d", MAX_MMCD_MSG_SIZE, output.head.usBodyLen);
		output.head.usBodyLen = MAX_MNGPKT_BODY_SIZE - 1;
	}

	memcpy(output.data, szTempBuf, output.head.usBodyLen);
	output.data[output.head.usBodyLen] = 0x00;
	if( (run_tbl->ucbinflag[tmrID]==1) && (sRet >= 0))
		log_print(LOGN_DEBUG, "BinResult=%d BODYLEN=%d BINARY FORMAT", run_tbl->ucbinflag[tmrID], output.head.usBodyLen);
	else
		log_print(LOGN_DEBUG, "BinResult=%d BODYLEN=%d BODY=[%s]", run_tbl->ucbinflag[tmrID], output.head.usBodyLen, output.data);

	output.head.ucBinFlag		= run_tbl->ucbinflag[tmrID];
	output.head.llMagicNumber	= MAGIC_NUMBER;
	output.head.llIndex			= run_tbl->llIndex[tmrID];

	/*** STATISTIC INFORMATION FOR NMS ****************************************/
	output.head.usTotPage	= msg->common.TotPage;
	output.head.usCurPage	= msg->common.CurPage;
	output.head.usStatFlag	= msg->common.StatFlag;

	if(sRet == DBM_SUCCESS)
		output.head.usResult = cont_flag;
	else
		output.head.usResult = sRet;

	output.head.usTotLen = output.head.usBodyLen+MNG_PKT_HEAD_SIZE;

	output.head.usSrcProc = run_tbl->inv_proc[tmrID];
	memcpy(output.head.TimeStamp, run_tbl->time_stamp[tmrID], sizeof(run_tbl->time_stamp[tmrID]));
	memcpy(output.head.userName, run_tbl->user_name[tmrID], 24);

	output.head.ucSvcID = SID_MML;
	output.head.ucMsgID = MID_MML_RST;
	memcpy(&output.head.ucmmlid[0], &run_tbl->usmmlid[tmrID], sizeof(unsigned short));

	log_print(LOGN_DEBUG, "**********TOTAL=%d BODY=%d", output.head.usTotLen, output.head.usBodyLen);

	dConTblIdx = dGetConTblIdx(run_tbl->sfd[tmrID]);

	dRet	= dSendMessage(run_tbl->sfd[tmrID], output.head.usTotLen, (char*)&output, dConTblIdx);
	dRet1	= dSetSockBlock(dRet, run_tbl->sfd[tmrID], dConTblIdx);

	// If This CMD for Binary Format, This Message will be maden for String MSG
	if( (run_tbl->ucbinflag[tmrID]==1) && (sRet>=0))
	{
		strcpy(szTempBuf, (char*)ComposeHead());
		dLen = strlen(szTempBuf);
		if(lib_tbl[msg->head.msg_id%MAX_LIB_TABLE].mmc_res != NULL)
		{
			sprintf(szTempBuf+dLen, "M%04d  %s", lib_tbl[msg->head.msg_id%MAX_LIB_TABLE].mcode, lib_tbl[msg->head.msg_id%MAX_LIB_TABLE].msg_header);

			CurCount = run_tbl->stat_cur_cnt[tmrID];
			TotCount = run_tbl->stat_tot_cnt[tmrID];

			(*lib_tbl[msg->head.msg_id%MAX_LIB_TABLE].mmc_res)(szTempBuf+dLen, msg, &CurCount, &TotCount);
		}

		if(cont_flag == DBM_CONTINUE)
			strcat(szTempBuf, "\nCONTINUED\n");
		else
			strcat(szTempBuf, "\nCOMPLETED\n");

		output.head.usBodyLen = strlen(szTempBuf);
		if(output.head.usBodyLen >= MAX_MMCD_MSG_SIZE)
		{
			log_print(LOGN_WARN, "MMCD OUTPUT PRINT BIN to TEXT BODY MESSAGE OVER FLOW MAX=%d CUR=%d", MAX_MMCD_MSG_SIZE, output.head.usBodyLen);
			output.head.usBodyLen = MAX_MMCD_MSG_SIZE - 1;
		}
		memcpy(output.data, szTempBuf, output.head.usBodyLen);
		output.data[output.head.usBodyLen] = 0x00;
		output.head.usTotLen = output.head.usBodyLen+MNG_PKT_HEAD_SIZE;
		log_print(LOGN_INFO, "BINARY FORMAT to STRING FORMAT BODYLEN=[%d][%s]", output.head.usBodyLen, output.data);
	}

	dNmsSfd = dGetNMSsfd();
	dNmsUserIdx = GetUserInfo("nmsnms", 1);
	log_print(LOGN_DEBUG, "NMS SFD:[%d] CURRENT SFD:[%d]", dNmsSfd, run_tbl->sfd[tmrID]);
	if( (dNmsSfd>0) && (dNmsSfd!=run_tbl->sfd[tmrID]) && (stAdmin.dConnectFlag[dNmsUserIdx]==1))
	{
		log_print(LOGN_INFO, "[INFO] SEND RESULT MESSAGE TO NMS Len=%d[%s]", output.head.usBodyLen, output.data);

		dConTblIdx	= dGetConTblIdx(dNmsSfd);
		dRet		= dSendMessage(dNmsSfd, output.head.usTotLen, (char*)&output, dConTblIdx);
		dRet1		= dSetSockBlock(dRet, dNmsSfd, dConTblIdx);
	}
	dRet = dInsert_MMCD_Result(&output, tmrID, outbuf, run_tbl->sfd[tmrID]);

	return 0;
}

/*******************************************************************************

*******************************************************************************/
void Timer ( time_t tTime )
{
    int 	i;
	int 	len;
	int		dRet;

	MML_RESULT  	*mml;
    st_MsgQ   		stMq;
    st_MsgQSub 		*stSubQ;

	pst_MsgQ		pstMq;

	/* msgq ==> gifo */
	U8				*pNODE;
	OFFSET			offset;

    for (i=1; i<=MAX_TMR_REC; i++) {
        if (run_tbl->cmd_id[i] > 0) {
            run_tbl->time[i] -= 1;  /* Decrease Time Value */
            if (run_tbl->time[i] <= 0) {
				/*
				* TIME OUT
				*/
				mml = (MML_RESULT*)&stMq.szBody[0];

                mml->head.msg_id = run_tbl->msg_id[i];
                mml->head.cmd_id = run_tbl->cmd_id[i];

                mml->common.mml_err = eMMCDTimeOut;
                mml->common.cont_flag = DBM_END;

				mml->common.TotPage = 1;
				mml->common.CurPage = 1;

				stSubQ = (st_MsgQSub*)&stMq.llMType;
				stSubQ->usType = DEF_SYS;
				stSubQ->usSvcID = SID_MML;
				stSubQ->usMsgID = MID_MML_RST;

				/* msgq ==> gifo */
				//stMq.dMsgQID = gdMyQid;
				stMq.dMsgQID = 0;
				stMq.ucProID = SEQ_PROC_MMCD;
				stMq.usRetCode = 0;

				memcpy( stMq.szBody, mml, sizeof( MML_RESULT ) );
				stMq.usBodyLen = sizeof( MML_RESULT );

				len = stMq.usBodyLen + DEF_MSGHEAD_LEN;

/* msgq ==> gifo */
#if 0
                if(msgsnd (gdMyQid, (char*)&stMq, len, IPC_NOWAIT) < 0) {
                    log_print(LOGN_CRI, "[ERROR] MSGSND TIME_OUT MESSAGE [%d] [%s]",
									errno, strerror(errno));
				}
				else {
					log_print(LOGN_DEBUG, "TIMEOUT MESSAGE INVOKE IDX[%d] CMD_ID[%d] MSG_ID[%d] TIME[%d]",
									i, run_tbl->cmd_id[i], run_tbl->msg_id[i], run_tbl->time[i]);
				}
#endif
				pNODE = nifo_node_alloc(gpRECVMEMS);
				if(pNODE == NULL)
				{
					log_print(LOGN_CRI, "[ERROR] nifo_node_alloc, NULL");
					//return -1;
				}

				pstMq = (pst_MsgQ)nifo_tlv_alloc(gpRECVMEMS, pNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF);
				if(pstMq == NULL)
				{
					log_print(LOGN_CRI, "[ERROR] nifo_tlv_alloc, NULL");
					nifo_node_delete(gpRECVMEMS, pNODE);
					//return -2;
				}

				// TODO pstMq 에 데이터 쓰기
				/* msgq ==> gifo */
				memcpy(pstMq, &stMq, DEF_MSGQ_SIZE);

				offset = nifo_offset(gpRECVMEMS, pNODE);
				if(gifo_write(gpRECVMEMS, pCIFO, SEQ_PROC_MMCD, SEQ_PROC_MMCD, offset) < 0)
				{
					// TODO 실패시 gifo_write 재시도 루틴

					log_print(LOGN_CRI, "[ERROR] gifo_write(from=%d:MMCD, to=%d:MMCD), offset=%ld",
								SEQ_PROC_MMCD, SEQ_PROC_MMCD, offset);
					nifo_node_delete(gpRECVMEMS, pNODE);
					usleep(0);
					//return -5;
				}
            }

			if( run_tbl->stat_TOTAL[i] > 0 && run_tbl->stat_check_time[i] != 0 ) {
				if( ((tTime - run_tbl->stat_check_time[i])%10 ) == 0 )
					log_print(LOGN_DEBUG, "[STAT INFO] RUN[%d] DIFF[%ld] TIME[%d]",
						i, (run_tbl->stat_check_time[i] - tTime), run_tbl->time[i] );

				if( (tTime - run_tbl->stat_check_time[i]) > 0 ) {
					stSubQ = (st_MsgQSub*)&stMq.llMType;

        			stSubQ->usType = DEF_SYS;
        			stSubQ->usSvcID = SID_MML;
        			stSubQ->usMsgID = MID_MML_REQ;

					/* hhbaek - 함수 변경 : 리던값 없음
        			dRet = dMakeNID( DEF_SYS_MMCD, &stMq.llNID );
					if( dRet < 0 ) {
            			log_print(LOGN_DEBUG,"MAKE NID FAIL" );
        			}
					*/
        			util_makenid( DEF_SYS_MMCD, &stMq.llNID );
        			
        			//stMq.dMsgQID = gdMyQid;
        			stMq.dMsgQID = 0;
        			stMq.ucProID = SEQ_PROC_MMCD;
        			stMq.usRetCode = 0;
        			stMq.usBodyLen = sizeof(mml_msg);
					stMq.llIndex = run_tbl->llIndex[i];

					memcpy( &stMq.szBody[0], &run_tbl->stMML_MSG[i], sizeof(mml_msg) );

					dRet = SendToPROC( &stMq, run_tbl->blockcode[i] );

					run_tbl->stat_check_time[i] = run_tbl->stat_check_time[i] + run_tbl->period[i];

					run_tbl->time[i] = TIME_OUT;

					log_print(LOGN_INFO, "[INFO] STATISTIC DATA SEND (%d)th ", run_tbl->stat_TOTAL[i] );

				}
			}
        }
	}
}

/*******************************************************************************

*******************************************************************************/
int dCheckStatFlag( short StatFlag )
{
	int dResult = 0;

	switch( StatFlag ) {
#if 0
		case MID_LOAD_STERM:
		case MID_FLT_STERM:
		case MID_INF_STERM:
		case MID_IPAF_STERM:
#endif
		case 1:
			dResult = 1;
			break;

		default:
			break;
	}

	return dResult;
}

int dInsert_MMCD_Result(st_MngPkt *pstMngPkt, short tmrID, char *szBuf, int dSockfd)
{
	int					dRet;
	size_t				szLen;
	st_SysMMCDMsg		stData;
	struct sockaddr_in	sa;
	socklen_t			sock_len;

	memcpy(stData.szUserName, pstMngPkt->head.userName, MAX_USER_NAME-1);
	stData.szUserName[MAX_USER_NAME-1] = 0x00;
	stData.uiTime = time(NULL);
	g_usSeq++;

	stData.usSeq = g_usSeq;

	if(g_usSeq >= 999)
		g_usSeq = 0;

	strcpy( (char*)stData.szCommand, g_cmd_line);
	szLen = strlen(szBuf);
	if(szLen >= MAX_MMCD_MSG_SIZE)
	{
		log_print(LOGN_WARN, "MMCD RESULT DB INSERT BUFFER OVERFLOW MAX:%d CUR=%ld", MAX_MMCD_MSG_SIZE, szLen);
		szLen = MAX_MMCD_MSG_SIZE - 1;
	}

	memcpy(stData.szMessage, szBuf, szLen);
	stData.szMessage[szLen] = 0x00;

	if(pstMngPkt->head.usResult > 0)
		stData.usResult = 1;
	else
		stData.usResult = 0;

	if(dSockfd != 0)
	{
	    /* get a informations of this clients.	*/
	    sock_len = (socklen_t)sizeof(sa);
	    if(getpeername(dSockfd, (struct sockaddr*)&sa, &sock_len) == -1)
	    {
			if(strcasestr(stData.szCommand, "USER-LOGOUT") == NULL)
			{
				log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN getpeername(run_tbl->sfd[%hd][%d]) errno[%d-%s]", __FILE__, __FUNCTION__, __LINE__,
					tmrID, dSockfd, errno, strerror(errno));
				return -1;
			}
			else
				stData.uiUserBIP = stAdmin.stUserList[GetUserInfo(stData.szUserName,1)].lLastLoginIP;
	    }
	    stData.uiUserBIP	= htonl(sa.sin_addr.s_addr);
	}
	else
		stData.uiUserBIP	= 0;

	if( (dRet = dInsert_MMCDMsg(&stMySQL, &stData)) < 0)
	{
		if(dRet == -1)
		{
			if( (dRet = dCreate_MMCDMsg(&stMySQL)) < 0)
			{
				log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN dCreate_MMCDMsg() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
				return -2;
			}

			if( (dRet = dInsert_MMCDMsg(&stMySQL, &stData)) < 0)
			{
				log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN dInsert_MMCDMsg() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
				return -3;
			}
		}
		else if(dRet == -2)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN dInsert_MMCDMsg() dRet[%d-DB_STOP]", __FILE__, __FUNCTION__, __LINE__, dRet);
			exit(1);
		}
		else
			log_print(LOGN_DEBUG, "F=%s:%s.%d: FAILED IN dInsert_MMCDMsg() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
	}

	return 0;
}
