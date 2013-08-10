/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <sys/socket.h>		/* inet_ntoa() */
#include <netinet/in.h>		/* inet_ntoa() */
#include <arpa/inet.h>		/* inet_ntoa() */
#include <string.h>			/* MEMSET() */
#include <unistd.h>			/* WRITE(2) */
#include <errno.h>			/* ERRNO */
#include <stdlib.h>			/* EXIT(3) */
/* LIB HEADER */
#include "commdef.h"		/* MC_INIT_M/F_PRI/SEC */
#include "filedb.h"
#include "loglib.h"
#include "filelib.h"		/* get_db_conf() */
#include "dblib.h"			/* db_conn(), db_disconn(), db_check_alive(), MYSQL  */
#include "verlib.h"			/* set_version() */
/* PRO HEADER */
#include "path.h"
#include "mmcdef.h"			/* st_MngPkt, st_ConTbl, MAX_FDSET2 */
#include "sockio.h"			/* MAGIC_NUMBER */
#include "procid.h"
#include "sshmid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "cond_init.h"		/* dGetSysNo(), dInitIPCs(), dGetBlocks() */
#include "cond_func.h"		/* cvt_ipaddr() */
#include "cond_db.h"		/* dInsert_CONDResult() */
#include "cond_mem.h"		/* dSend_SI_NMS_Alarm(), dSend_SI_NMS() */
#include "cond_msg.h"		/* st_CondCount, Set_Cond_Msg() */
#include "cond_sock.h"		/* dRecvMessage(), dTcpSvrModeInit() */

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
char		szIP[16], szName[32], szPass[32], szAlias[32];
char		gszNTAMName[64];
char		vERSION[7] = "R4.0.0";
int			gdNumfds, gdNumWfds, gdSvrSfd;
int			gdStopFlag;
int			gdMaxCliCnt = MAX_FDSET2;

st_MSG_BUF	gstMsgBuf[MAX_FDSET2];
st_ConTbl	stConTbl[MAX_FDSET2+1];
fd_set		gstReadfds, gstWritefds;
MYSQL		stMySQL;

/** D.2* DECLARATION OF VARIABLES ( External ) *************************/
extern char				SWLST_M_PRIMARY[MAX_SW_COUNT][30];
extern char				SWLST_F_PRIMARY[MAX_SW_COUNT][30];
extern char				SWLST_F_SECONDARY[MAX_SW_COUNT][30];
extern int				gdPortNo;

/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF EXTERN FUNCTIONS **************************/

#ifdef _ENABLE_HEARTBEAT_
void setHeartbeatPacket(st_MngPkt *p)
{
	memset(p, 0x00, sizeof(st_MngPkt));

	p->head.llMagicNumber    = MAGIC_NUMBER;
	p->head.llIndex          = 1;
	p->head.usResult         = 0;

	p->head.usTotPage        = 1;
	p->head.usCurPage        = 1;
	p->head.usBodyLen        = 0;
	p->head.usTotLen         = 0 + MNG_PKT_HEAD_SIZE;

	p->head.ucSvcID          = SID_HEARTBEAT;
	p->head.ucMsgID          = MID_HEARTBEAT_REQ;

	p->head.ucmmlid[0]       = 1;
	p->data[0]               = 0x00;
	
	return;
}

void CheckClient(time_t current)
{
	int 		i, dSfd, dRet;
	st_MngPkt 	stPkt;
	fd_set		stWd;
	
	memset(&stPkt, 0x00, sizeof(st_MngPkt));
	setHeartbeatPacket(&stPkt);

	for( i=0; i< gdNumfds; i++ ){
		if( stConTbl[i].dSfd > 0 && ( stConTbl[i].dLastSendTime + HEARTBEAT_TIMEOUT ) < current ){
			dSfd = stConTbl[i].dSfd;
			log_print(LOGN_INFO,"Check Client IDX=%d, SFD=%d, IP=%s", i, dSfd, cvt_ipaddr(stConTbl[i].uiCliIP) ); 
			
			if( (dRet = write( dSfd, (char*)&stPkt, stPkt.head.usTotLen )) < 0 ){
				log_print(LOGN_WARN,"ERROR IN send(), err=%d:%s", errno, strerror(errno));
				if( errno == EAGAIN )
					continue;
				else{
					close(dSfd);
					FD_CLR(dSfd, &gstReadfds);

					/* removed sfd @write fds */
					memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));
					if( FD_ISSET(dSfd, &stWd) ){
						log_print(LOGN_DEBUG, "CLEARED WRITE SOCKET FD, CLOSED SFD=%d", dSfd);
						FD_CLR(dSfd, &gstWritefds);
						if( gdNumWfds > 0 )
							gdNumWfds--;
						stConTbl[i].cSockBlock = 0x00;
					}


					log_print(LOGN_INFO," *** SOCKET FD =%d", gdNumfds);

					stConTbl[i].dSfd 	  = -1;
					stConTbl[i].dBufLen   = 0;
					stConTbl[i].uiCliIP   = 0;				
					stConTbl[i].Reserv[0] = 0;

				}
			}else{
				log_print(LOGN_INFO,"HEARTBEAT] UPDATE LASTSENDTIME=%ld, dRet=%d, SendLen=%d", current, dRet, stPkt.head.usTotLen);
				stConTbl[i].dLastSendTime = (int)current;
			}
		}
	}
}

#endif /* _ENABLE_HEARTBEAT_ */

int main(int argc, char *argv[])
{
	int				i, dLen, dRet;
	unsigned char	ucTAFID, ucTAMID;
	st_MsgQ			stMsg;
	pst_MsgQSub		pstMsgQSub;
	st_almsts	*palm;
	st_CondCount	stCnt;
	st_MngPkt		stPkt;
	time_t			tCurTime, tLastPingMySQL;

    log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_COND, LOG_PATH"/COND", "COND");

	SetUpSignal();

    if( (dRet = dGetBlocks(FILE_MC_INIT_M_PRI, SWLST_M_PRIMARY)) < 0)
        log_print(LOGN_CRI, LH"ERROR IN dGetBlocks(%s) dRet[%d]", LT, FILE_MC_INIT_M_PRI, dRet);

    if( (dRet = dGetBlocks(FILE_MC_INIT_F_PRI, SWLST_F_PRIMARY)) < 0)
        log_print(LOGN_CRI, LH"ERROR IN dGetBlocks(%s) dRet[%d]", LT, FILE_MC_INIT_F_PRI, dRet);

    if( (dRet = dGetBlocks(FILE_MC_INIT_F_SEC, SWLST_F_SECONDARY)) < 0)
        log_print(LOGN_CRI, LH"ERROR IN dGetBlocks(%s) dRet[%d]", LT, FILE_MC_INIT_F_SEC, dRet);

	gdStopFlag	= 1;
	gdNumfds	= 0;
	gdNumWfds	= 0;

	if( (dRet = get_db_conf(FILE_MYSQL_CONF, szIP, szName, szPass, szAlias)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN get_db_conf() dRet[%d]", LT, dRet);
		exit(-3);
	}

	if( (dRet = db_conn(&stMySQL, szIP, szName, szPass, szAlias)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN db_conn() dRet[%d]", LT, dRet);
		exit(-4);
	}

	if( (dRet = Init_Fidb()) < 0)
	{
        log_print(LOGN_CRI, LH"ERROR IN Init_Fidb() dRet[%d]", LT, dRet);
		FinishProgram();
	}

	memset(&gszNTAMName[0], 0x00, sizeof(gszNTAMName));
	if( (dRet = dGetHostName(gszNTAMName)) < 0)
		strcpy(gszNTAMName, "TAM_APP");

	if( (dRet = dTcpSvrModeInit()) < 0)
	{
        log_print(LOGN_CRI, LH"ERROR IN dTcpSvrModeInit() dRet[%d]", LT, dRet);
		FinishProgram();
	}

	for(i = 0; i < MAX_FDSET2; i++)
	{
		stConTbl[i].dSfd	= -1;
		stConTbl[i].uiCliIP	= 0;
		gstMsgBuf[i].dWidx	= 0;
		gstMsgBuf[i].dRidx	= 0;
		memset(gstMsgBuf[i].szBuf, 0x00, sizeof(MAX_MNG_PKT_BUFSIZE));
	}

	if( (dRet = dInitIPCs()) < 0)
	{
        log_print(LOGN_CRI, LH"ERROR IN dInitIPCs() dRet[%d]", LT, dRet);
		FinishProgram();
	}

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_COND, vERSION)) != 1)
		log_print(LOGN_WARN, "set_version error(ret=%d,idx=%d,ver=%s)\n", dRet, SEQ_PROC_COND, vERSION);

	if( (dRet = dGetSysNo()) < 0)
	{
        log_print(LOGN_CRI, LH"ERROR IN dGetSysNo() dRet[%d]", LT, dRet);
		FinishProgram();
	}
	else
		ucTAMID = dRet;

	log_print(LOGN_CRI, "COND %s Started\n", vERSION);
	while(gdStopFlag)
	{
		while(1)
		{
			if( (dRet = dIsRcvedMessage(&stMsg)) > 0)
			{
				pstMsgQSub	= (pst_MsgQSub)&stMsg.llMType;
				log_print(LOGN_DEBUG,"[GET MSG] PRCID=%d SID=%d MID=%d BLEN=%d GETLEN=%d NID=%llu",
					stMsg.ucProID, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, stMsg.usBodyLen, dRet, stMsg.llNID);

				if(pstMsgQSub->usSvcID == SID_STATUS)
				{
					if(pstMsgQSub->usMsgID == MID_CONSOL)
					{
						/*
						*	SETTING COND MESSAGE : CREATE
						*	into 'stPkt.data'<- COND MESSAGE
						*/
						palm = (st_almsts*)&stMsg.szBody[0];

						if( (dRet = dSend_SI_NMS_Alarm(palm)) < 0)
							log_print(LOGN_CRI, LH"ERROR IN dSend_SI_NMS_Alarm() dRet[%d]", LT, dRet);
						else
							log_print(LOGN_DEBUG, LH"SUCCESS IN dSend_SI_NMS_Alarm()", LT);

						if(stMsg.ucProID == SEQ_PROC_CHSMD)
						{
							ucTAFID		= 0;
							log_print(LOGN_DEBUG, "[CONSOLE ALARM][FROM NTAM:%d]", ucTAMID);
							if( (palm->ucSysType != SYSTYPE_DIRECT) && (palm->ucSysType != SYSTYPE_SWITCH))
								palm->ucSysNo	= ucTAMID;
						}
						else
						{
							ucTAFID = stMsg.ucNTAFID;
							log_print(LOGN_DEBUG,"[CONSOL ALARM][FROM NTAF:%d][NTAM:%d]", ucTAFID,ucTAMID );
						}

						memset(&stPkt, 0x00, sizeof(st_MngPkt));
						if( (dRet = Set_Cond_Msg(&stMsg, &stPkt.data[0], &stCnt, ucTAMID, ucTAFID)) < 0)
						{
							log_print(LOGN_WARN, "FAIL IN Set_Cond_Msg: SYSTYPE=%x LOCTYPE=%x INVTYPE=%x ALMLVL=%d",
								palm->ucSysType, palm->ucLocType, palm->ucInvType, palm->ucAlmLevel);
							continue;
						}

						/* memorial value of cond -message to 'dLen' */
						dLen = dRet;

						/*
						 *	SEND TO OMP
						 */
						stPkt.head.ucNTAFID	= ucTAFID;
						stPkt.head.ucNTAMID	= ucTAMID;

						if( (dRet = SendToOMP(pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, &stCnt, dRet, &stPkt)) < 0)
						{
							log_print(LOGN_WARN,"[FAIL:%d] Send To OMP",dRet);
							continue;
						}

						if( (dRet = dSend_FSTAT(&stMsg)) < 0)
							log_print(LOGN_CRI, LH"ERROR IN dSend_FSTAT() dRet[%d]", LT, dRet);

						if( (dRet = dSend_SI_NMS(stPkt.data, dLen)) < 0)
							log_print(LOGN_CRI, LH"ERROR IN dSend_SI_NMS() dRet[%d] CondMsg[%s]", LT, dRet, &stPkt.data[0]);
						else
							log_print(LOGN_DEBUG, LH"SUCCESS IN dSend_SI_NMS() CondMsg[%s]", LT, &stPkt.data[0]);

						if( (dRet = dInsert_CONDResult( (char*)palm,(char*)&stPkt.data[0], dLen, ucTAMID, ucTAFID)) < 0)
							log_print(LOGN_CRI, LH"ERROR IN dInsert_CONDResult(CondMsg[%s]) dRet[%d]", LT, &stPkt.data[0], dRet);
					}
					else
					{
						log_print(LOGN_DEBUG,"UNVALID MID[%d]", pstMsgQSub->usMsgID );
						continue;
					}
				}
				else
				{
					log_print(LOGN_WARN,"UNVALID SID[%d]", pstMsgQSub->usSvcID );
					continue;
				}
			}/* Received */
			else
				break;
		}/* while(flag) */
		dRet = dSocketCheck();

		if( ((tCurTime = time(NULL)) - tLastPingMySQL) > SEC_OF_HOUR)
		{
			if( (dRet = db_check_alive(&stMySQL)) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN db_check_alive() dRet[%d]", LT, dRet);
				return -5;
			}
			tLastPingMySQL = tCurTime;
		}

#ifdef _ENABLE_HEARTBEAT_
		CheckClient(tCurTime);
#endif /* _ENABLE_HEARTBEAT_ */

	}/* main while */
	log_print(LOGN_CRI,"################ Program End !!!\n");
	db_disconn(&stMySQL);

	return 0;
}

int dSocketCheck(void)
{
	char				szTcpRmsg[MAX_MNG_PKT_BUFSIZE];
	int 				i, dSfd, dRecvLen, dSelRet, dRet, dCliLen, dSBufLen, dSBufLen1;
	fd_set				stRd, stWd;

	struct timeval		timeout;
	struct sockaddr_in	stCli_addr;
	struct in_addr		inaddr;

	memcpy((char*)&stRd, (char*)&gstReadfds, sizeof(fd_set));
	memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));

	dCliLen			= sizeof(struct sockaddr_in);
	timeout.tv_sec	= 0;
	timeout.tv_usec	= 1000;

	if( (dSelRet = select(gdNumfds+1, &stRd, &stWd, (fd_set*)NULL, &timeout)) < 0)
	{
		if(errno != EINTR)
		{
			// SELECT UNEXPECTED ERROR
			log_print(LOGN_CRI, LH"FAILED IN select() [errno:%d-%s]", LT, errno, strerror(errno));
			FinishProgram();
		}
		return 0;
	}
	else if(dSelRet > 0)
	{
		//	EVENT INVOKE
		if(FD_ISSET(gdSvrSfd, &stRd))
		{
			log_print(LOGN_CRI, "NEW CLIENT CONNECT TRIAL SVRPORT[%d] MAXLIMITCNT[%d]", gdPortNo, gdMaxCliCnt);
			if( (dSfd = accept(gdSvrSfd, (struct sockaddr*)&stCli_addr, (socklen_t*)&dCliLen)) < 0)
			{
				log_print(LOGN_CRI, "NEW CLIENT ACCEPT FAIL: %s", strerror(errno));
				return -1;
			}
			else
			{
				for(i = 0; i < gdMaxCliCnt; i++)
				{
					if(stConTbl[i].dSfd < 0)
					{
						stConTbl[i].dSfd	= dSfd ;
						gstMsgBuf[i].dWidx	= 0;
						gstMsgBuf[i].dRidx	= 0;
						break;
					}
				}

				if(i == gdMaxCliCnt)
				{
					log_print(LOGN_CRI, "MAX CLIENT OVERFLOW  IDX[%d] SFD[%d]", i, dSfd);
					close(dSfd);
					stConTbl[i].dSfd	= -1;
					dSfd				= -1;

					return -1;
				}

				dSBufLen	= 873800;
				dSBufLen1	= sizeof(dSBufLen);

				if(setsockopt(dSfd, SOL_SOCKET, SO_SNDBUF, (char*)&dSBufLen, dSBufLen1) < 0)
				{
					log_print(LOGN_CRI, "FAIL IN CHANGING SOCKET OPTION: SEND BUFFER, errno=[%d] %s", errno, strerror(errno));
					close(dSfd);
					stConTbl[i].dSfd	= -1;
					dSfd				= -1;

					return -1;
				}

				FD_SET(dSfd, &gstReadfds);

				if(dSfd > gdNumfds)
					gdNumfds = dSfd;

				stConTbl[i].uiCliIP		= stCli_addr.sin_addr.s_addr;
				stConTbl[i].cSockBlock	= 0x00;
				stConTbl[i].dBufLen		= 0;
				inaddr.s_addr = stCli_addr.sin_addr.s_addr;

				/*
				* SEND FALUT LOG TO OMP
				*/
				if(stConTbl[i].Reserv[0] == 0)
				{
					log_print(LOGN_INFO, "RESERVED SEND TO OMP FLTINFO");
					/*** reserved by uamyd0626 ***************************************
					dRet = dSendToOMPFLTInfo( dSfd, i );
					if( dRet < 0 ) {
					log_print( LOGN_CRI, "ERROR IN dSendToOMPFLTInfo RET[%d]", dRet );

					return -1;
					}
					******************************************************************/
				}
				log_print(LOGN_CRI, "CONNECT CLIENT IP[%s] IDX[%d] SFD[%d] NUMFDS[%d] NUMWFDS[%d]", inet_ntoa(inaddr), i, stConTbl[i].dSfd, gdNumfds, gdNumWfds);
			}
		}

		for(i = 0; i < gdMaxCliCnt; i++)
		{
			if(stConTbl[i].dSfd < 0)
				continue;

			dSfd = stConTbl[i].dSfd;

			if(FD_ISSET(dSfd, &stRd))
			{
				log_print(LOGN_DEBUG, "EXIST CLIENT IDX[%d] SFD[%d]", i, dSfd);

				szTcpRmsg[0]	= 0x00;
				dRecvLen		= 0;

				if( (dRet = dRecvMessage(dSfd, szTcpRmsg, &dRecvLen)) < 0)
				{
					log_print(LOGN_CRI, "CONNECTION RESET CLOSE IDX[%d] SFD[%d] IP[%s]", i, dSfd, cvt_ipaddr(stConTbl[i].uiCliIP));

					close(dSfd);
					FD_CLR(dSfd, &gstReadfds);

					stConTbl[i].dSfd		= -1;
					stConTbl[i].uiCliIP		= 0;
					stConTbl[i].cSockBlock	= 0x00;
					stConTbl[i].Reserv[0]	= 0;

					// Add by Tundra 0515 15:00
					memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));

					if( (gdNumWfds > 0) && FD_ISSET(dSfd, &stWd))
					{
						log_print(LOGN_CRI, "RECVFAIL WRITE FDSET CLEAR IDX[%d]", i);
						FD_CLR(dSfd, &gstWritefds);
						gdNumWfds--;
						log_print(LOGN_CRI, "REMAIN WRITE BLOCK FDSET COUNT [%d]", gdNumWfds);
					}
					log_print(LOGN_INFO," *** NUMFDS=%d, NUMWFDS=%d", gdNumfds, gdNumWfds);
				}
			}
		}

		if(gdNumWfds > 0)
		{
			// WRITEFDS EXIST CHECK
			for(i = 0; i < MAX_FDSET2; i++)
			{
				if(stConTbl[i].dSfd < 0)
					continue;

				// Add by Tundra 0731
				dSfd = stConTbl[i].dSfd ;

				if(stConTbl[i].cSockBlock == 0x00)
					continue;

				if(FD_ISSET(dSfd, &stWd))
				{
					log_print(LOGN_CRI, "WRITE FDSET BLOCK CLEAR EVENT INVOKE IP[%s] IDX[%d] SFD[%d]", cvt_ipaddr(stConTbl[i].uiCliIP), i, dSfd);

					FD_CLR(dSfd, &gstWritefds);
					// Add by Tundra 0731
					gdNumWfds--;

					stConTbl[i].cSockBlock = 0x00;

					if(stConTbl[i].dBufLen > 0)
					{
						if( (dRet = dSendMessage(i, dSfd, stConTbl[i].dBufLen, (char*)&stConTbl[i].szSendBuffer[0])) < 0)
						{
							log_print(LOGN_CRI,"BLOCK CLEAR EVENT INVOKE SEND MESSAGE FAIL IDX[%d] SFD[%d] IP[%s]", i, dSfd, cvt_ipaddr(stConTbl[i].uiCliIP));

							close(dSfd);
							FD_CLR(dSfd, &gstReadfds);

							stConTbl[i].dSfd		= -1;
							stConTbl[i].uiCliIP		= 0;
							stConTbl[i].Reserv[0]	= 0;
						}
						else if(dRet == 1)
							stConTbl[i].dBufLen     = 0;
					}
				}
			}
			log_print(LOGN_INFO," *** CHECK CURRENT SOCKET FD NUM=%d, WFD NUM=%d", gdNumfds, gdNumWfds);
		}
	}
	else if(dSelRet == 0)
	{
		// NO EVENT
	}

	return 0;
}


int SendToOMP( unsigned short usSvcID, unsigned short usMsgID, pst_CondCount pstCnt, int MsgLen, st_MngPkt *pstSpkt )
{
	int   i=0;
	int   dSfd;
	int   dRet=0;
	char  *str;

	fd_set  stWd;

	pstSpkt->head.llMagicNumber = MAGIC_NUMBER;
	pstSpkt->head.llIndex = 1;
	pstSpkt->head.usResult = 0;

	pstSpkt->head.usTotPage = pstCnt->usTotPage;
	pstSpkt->head.usCurPage = pstCnt->usCurPage;
	pstSpkt->head.usBodyLen = MsgLen;
	pstSpkt->head.usTotLen = MsgLen + MNG_PKT_HEAD_SIZE;

	pstSpkt->head.ucSvcID = usSvcID;
	pstSpkt->head.ucMsgID = usMsgID;

	memcpy( &pstSpkt->head.ucmmlid[0], &pstCnt->usSerial, 2);
	pstSpkt->data[MsgLen] = 0x00;

	/* DEBUG */
	log_print(LOGN_DEBUG," COND MESSAGE : \n%s",pstSpkt->data );

	str = &pstSpkt->data[0];

	for( i=0; i<MAX_FDSET2; i++ )
	{
		if( stConTbl[i].dSfd > 0 && stConTbl[i].cSockBlock == 0x00 )
		{
				dSfd = stConTbl[i].dSfd;

				dRet = dSendMessage( i, dSfd, pstSpkt->head.usTotLen, (char*)pstSpkt );

				if( dRet < 0 )
				{
					log_print(LOGN_CRI,"SEND MESSAGE FAIL IDX[%d] SFD[%d] IP[%s]", i, dSfd, cvt_ipaddr(stConTbl[i].uiCliIP) );

					close(dSfd);
					stConTbl[i].dSfd = -1;
            		stConTbl[i].uiCliIP = 0;
                    stConTbl[i].cSockBlock = 0x00;

					stConTbl[i].Reserv[0] = 0;

                    FD_CLR( dSfd, &gstReadfds );

                    memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));

                    if( gdNumWfds >0 && FD_ISSET(dSfd, &stWd ) )
                    {
                        log_print(LOGN_CRI,"SENDFAIL WRITE FDSET CLEAR");
                        FD_CLR( dSfd, &gstWritefds );
						gdNumWfds--;
                    }
				}
#ifdef _ENABLE_HEARTBEAT_
				else if( 1 == dRet ){
					stConTbl[i].dLastSendTime = time(NULL);
				}
#endif /* _ENABLE_HEARTBEAT_ */
		}
		else if( stConTbl[i].dSfd > 0 && stConTbl[i].cSockBlock == 0x01 )
		{
			AppendNonSendMsg( i, pstSpkt->head.usTotLen, (char*)pstSpkt );
		}
	}

	return 0;
}

int AppendNonSendMsg(int idx, int dMsgLen, char *szMsg)
{
	if( (dMsgLen + stConTbl[idx].dBufLen) > MAX_SENDBUF_LEN )
	{
		log_print(LOGN_CRI, "TO_SYS[%s] CURBUFFLEN[%d] LOSS MASSGE LEN[%d]",
			cvt_ipaddr(stConTbl[idx].uiCliIP ), stConTbl[idx].dBufLen, dMsgLen );
		return -1;
	}

	memcpy( &stConTbl[idx].szSendBuffer[stConTbl[idx].dBufLen], szMsg, dMsgLen );

	stConTbl[idx].dBufLen += dMsgLen ;

	log_print(LOGN_WARN, "TO_SYS[%s] CURBUFFLEN[%d] BUFFERBLOCK MASSGE LEN[%d]",
		cvt_ipaddr(stConTbl[idx].uiCliIP ), stConTbl[idx].dBufLen, dMsgLen );

	return 0;
}
