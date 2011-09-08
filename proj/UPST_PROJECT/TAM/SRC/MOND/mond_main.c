/*******************************************************************************
			DQMS Project

	Author   : Park Si Woo
	Section  : ALMD
	SCCS ID  : @(#)mond_main.c	1.8
	Date     : 08/7/03
	Revision History :
        '01.  7. 21     Initial
        '03.  1. 15     Initial

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/
#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/msg.h>

#include "almstat.h"
#include "filedb.h"
#include "sshmid.h"
#include "verlib.h"
#include "procid.h"
#include "path.h"
#include "loglib.h"
#include "mmcdef.h"
#include "msgdef.h"
#include "sockio.h"
#include "watch_mon.h"
#include "utillib.h"
#include "mond_init.h"
#include "mond_sock.h"
#include "mond_func.h"
#include "mond_mem.h"
#include "mond_mmc.h"

#define	 CHECK_SEND_TIME		1
#define  CHECK_NTAF_TIME		5
#define	 CHECK_NTAF_SYNC		4
#define  CHECK_MSGLIST_GAP		5
#define	 MAX_NTAF_TIME_GAP		30
#define  MAX_FAIL_COUNT         3

typedef struct {
	unsigned int	min[MAX_NTAF_RSRC_NUM];
	unsigned int	max[MAX_NTAF_RSRC_NUM];
	unsigned long	total[MAX_NTAF_RSRC_NUM];
	unsigned int	count;
	int				last;
} st_IpafRsrcInfo;

extern char *crTime(time_t when);	/* 20040323,poopee */

st_MSG_BUF    gstMsgBuf[MAX_FDSET2];
st_IpafRsrcInfo	gstIpafRsrcInfo[MAX_NTAF_NUM];	/* 20040520,poopee */

extern st_WNTAM 				*fidb;
extern pst_keepalive            gpKeepAlive;
st_WNTAM_LOADSTAT 				*loadstat;
st_NTAF_List_SHM				*gNtafSHM_db;


char    szTempBufferFirst[80000];
fd_set 	gstReadfds, gstWritefds;
char    szTempBufferLast[80000];

int 	gdNumfds;
int		gdNumWfds;
int 	gdSvrSfd;
int		gdClient = 0;

int		gdStopFlag;

int 	gdIndex;

int		gdMaxCliCnt = MAX_FDSET2;

int		gdNTAMcpuFailCnt;
int		gdNTAMmemFailCnt;
int		gdNTAMdskFailCnt[MAX_DISK_COUNT];
int		gdNTAMqueFailCnt;

st_ConTbl  			stConTbl[MAX_FDSET2+1];

char        crmtime_str[255];
char	vERSION[7] = "R4.0.0";

extern  int gdMyQid;
extern  int gdPortNo;
extern	int gdIFQid;
extern 	int gdMmcdQid;
extern  int gdIpaftifQid;
extern	int	gdCondQid;	/* 20040323,poopee */
extern 	int errno;

int		mY_HOST_ID=0;

st_MonTotal		*gMonTotal;

extern int MMC_Handle_Proc( mml_msg *mmsg );
extern int SendToOMP(pst_MngPkt pstSpkt);
extern int SendToOMPMon(int length, char *data);
extern int dSocketCheck(void);
extern int dGetMaxFds();

int 	dGetLoadValueStat( int SysType, int SysNo, int InvType, float fVal );
int 	dSetMinMaxVal( float *MinVal, float *MaxVal, float CurVal );
int 	dSetAvgVal( float *AvgVal, float NewVal, int dCount );
int		dSndStsMsg(int NTAFID, long long cur, long long  max, float percent);	/* 20040323,poopee */
void 	SetFIDBValue( UCHAR *ucFIDB, UCHAR ucNEW );
void	dSndIpafRsrcSts(int dNTAFID);

/*******************************************************************************

*******************************************************************************/
int main(int argc, char *argv[])
{
	int				dRet, i, j, loopCnt;
	time_t      	now, check_snd, check_ipaf, check_msglist, chsmd_time, init_time;
	unsigned char   ucNTAMID, ucNTAFID;

	st_MsgQ  		stMsg, stMsg2;	/* stMsg2: SEND TO FSTAT */
	pst_MsgQSub		pstMsgQSub;

	pst_MngPkt		pstMngPkt;
	struct tm 		*t;

	st_SvcMonMsg *pSvcMonMsg;
	st_MonList *pMonList;
	st_MngHead stMsgHead;

	ucNTAMID	= 0;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_MOND, LOG_PATH"/MOND", "MOND");

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_MOND, vERSION)) != 0)
		log_print( LOGN_CRI, "set_version error(ret=%d,idx=%d,ver=%s)", dRet,SEQ_PROC_MOND,vERSION);

	if( init_proc() < 0 ){
		return -1;
	}

	log_print(LOGN_CRI, "######## MOND PROCESS START - PROCID[%d] #########", getpid());

	gdIndex		= 1;
	gdStopFlag	= 1;
	gdNumfds	= 0;
	gdNumWfds	= 0;

	if( (dRet = Init_Fidb()) < 0)
	{
		log_print(LOGN_CRI, "[ERROR] FIDB INIT [%d]", dRet);
		FinishProgram();
		exit(-3);
	}

	if( (dRet = Init_SubFidb()) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN Init_SubFidb() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(-4);
	}

	if( (dRet = Init_LoadStat()) < 0)
	{
		log_print(LOGN_CRI, "[ERROR] LOADSTAT INIT [%d]", dRet);
		FinishProgram();
		exit(-5);
	}

    if( (dRet = Init_Keepalive()) < 0)
	{
		log_print(LOGN_CRI, "[ERROR] KEEPALIVE INIT [%d]", dRet);
		FinishProgram();
		return -6;
    }
	else if(dRet == 0)
		dRet = Load_KeepAlive();

	/* Monitoring Alarm */
	if((dRet = InitMonTotalShm()) < 0)
		log_print(LOGN_CRI, "F=%s:%s.%d InitMonTotalShm dRet=%d:%s", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));

	if( (dRet = dTcpSvrModeInit()) < 0)
	{
		log_print(LOGN_CRI, "[ERROR] dTcpSvrModeInit [%d]", dRet);
		FinishProgram();
		exit(-7);
	}

	for(i = 0; i < MAX_FDSET2; i++)
	{
		stConTbl[i].dSfd		= -1;
		stConTbl[i].uiCliIP		= 0;
		stConTbl[i].dBufLen		= 0;
		stConTbl[i].szSendBuffer[0]	= 0;
		stConTbl[i].cSockBlock	= 0x00;
		gstMsgBuf[i].dWidx		= 0;
		gstMsgBuf[i].dRidx		= 0;
		memset(gstMsgBuf[i].szBuf, 0x00, MAX_MNG_PKT_BUFSIZE);
	}

	/*
	* INITIALIZE FAIL COUNT
	*/
	gdNTAMcpuFailCnt = 0;
	gdNTAMmemFailCnt = 0;
	gdNTAMqueFailCnt = 0;
	for(i=0; i<MAX_DISK_COUNT; i++)
		gdNTAMdskFailCnt[i] = 0;

	now = time(&now);
	init_time = chsmd_time = check_snd = check_ipaf = now;

	check_msglist = (now/300)*300 + 300;

	t = localtime(&now);
	for(i = 0; i < MAX_NTAF_NUM; i++)
	{
		memset(&gstIpafRsrcInfo[i], 0, sizeof(st_IpafRsrcInfo));
		for(j = 0; j < MAX_NTAF_RSRC_NUM; j++)
			gstIpafRsrcInfo[i].min[j]	= 0xffffffff;
		gstIpafRsrcInfo[i].last = t->tm_hour;
	}

	if( (dRet = dGetSYSCFG()) < 0)
	{
		log_print(LOGN_CRI, "FAILED IN dGetSYSCFG()[%d]", dRet);
		FinishProgram();
	}
	else
	{
		ucNTAMID	= dRet;
		mY_HOST_ID	= dRet;
	}
	SetUpSignal();

	log_print(LOGN_CRI, "######## MOND INITILIZE FINISHED : %s  \n", vERSION);

	/*
	* MAIN WHILE LOOP START
	*/
	while(gdStopFlag)
	{

		loopCnt = 0;
		while(1)
		{
			memset(&stMsg, 0x00, sizeof(st_MsgQ));
			memset(&stMsg2, 0x00, sizeof(st_MsgQ));

			if( (dRet = dIsRcvedMessage(&stMsg)) < 0)
			{
				usleep(0);
				loopCnt++;
				if( loopCnt > 100 ){
					break;
				}
			}
			else
			{
				pstMsgQSub = (pst_MsgQSub)&stMsg.llMType;
				log_print(LOGN_INFO, "[RCV MSG QUEUE][%d]:Type[%d]SvcID[%d]MsgID[%d]usBodyLen[%d]", stMsg.ucProID, pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, stMsg.usBodyLen);

				if( (pstMsgQSub->usType==DEF_SYS) && (pstMsgQSub->usSvcID==SID_MML) && (pstMsgQSub->usMsgID==MID_MML_REQ))
				{
					MMC_Handle_Proc((mml_msg*)&stMsg.szBody[0]);
				}
				else if( (pstMsgQSub->usSvcID==SID_STATUS) && (pstMsgQSub->usMsgID==MID_ALARM))
				{
					memcpy(&stMsg2, &stMsg, DEF_MSGHEAD_LEN);
					if(stMsg.ucProID == SEQ_PROC_CHSMD)
					{
						ucNTAFID		= 0;
						stMsg.ucNTAFID	= ucNTAFID;

						/*	SEND NTAM STATUS(FROM CHSMD)	*/
						pstMngPkt		= (pst_MngPkt)&stMsg.szBody[NTAFT_HEADER_LEN];
						log_print(LOGN_INFO, "MESSAGE [NTAM%d SYS]", ucNTAMID);

						fidb->stNTAM.tUpTime	= now;
						dMakeNTAMMngPkt(pstMngPkt, &fidb->stNTAM, ucNTAMID);

						if( (dRet = SendToOMP(pstMngPkt)) < 0)
							log_print(LOGN_WARN, "F=%s:%s.%d: ERROR IN SendToOMP() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);

						dSend_FSTAT(&stMsg2, &pstMngPkt->data[0]);
						continue;
					}
					else if(stMsg.ucProID == SEQ_PROC_SI_SVC)
					{
						ucNTAFID	= stMsg.ucNTAFID;

						/*	SEND NTAF STATUS(FROM SI_SVC)	*/
						pstMngPkt	= (pst_MngPkt)&stMsg.szBody;
						log_print(LOGN_INFO, "MESSAGE [NTAF%d SYS]", stMsg.ucNTAFID);

						/*	STAT 일단 보류	*/
						if(pstMsgQSub->usMsgID == MID_STAT)
						{
							/* BUT!!!! 06.06.17 DON'T USED by uamyd0626 */
							/*****************************************************************
							* NTAF SVCSTAT VALUE BY PASSING
							*****************************************************************/
							log_print(LOGN_INFO, "MESSAGE [NTAF SVCSTAT]");

							dMakeSTATMngPkt(pstMngPkt, stMsg.usBodyLen);
						}
						pstMngPkt->head.ucNTAFID	= ucNTAFID;
						pstMngPkt->head.ucNTAMID	= ucNTAMID;
						if( ((ucNTAFID-1)>=0) && ((ucNTAFID-1)<MAX_NTAF_COUNT))
							memcpy(&gNtafSHM_db->stNTAF[ucNTAFID-1], &pstMngPkt->data[0], sizeof(st_NTAF));
						else
							log_print(LOGN_CRI, "F=%s:%s.%d: UNAVAILABLE ucNTAFID[%d]", __FILE__, __FUNCTION__, __LINE__, ucNTAFID);

						if( (dRet = SendToOMP(pstMngPkt)) < 0)
							log_print(LOGN_WARN, "F=%s:%s.%d: ERROR IN SendToOMP() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);

						dSend_FSTAT(&stMsg2, &pstMngPkt->data[0]);
						continue;
					}
					else
					{
						log_print(LOGN_WARN,"UNVALID PROID[%d]",stMsg.ucProID);
						continue;
					}
				}
				else if( (pstMsgQSub->usSvcID==SID_STATUS_DIRECT) && (pstMsgQSub->usMsgID==MID_ALARM))
				{
					memcpy(&stMsg2, &stMsg, DEF_MSGHEAD_LEN);
					if(stMsg.ucProID == SEQ_PROC_CHSMD)
					{
						pstMngPkt		= (pst_MngPkt)&stMsg.szBody[NTAFT_HEADER_LEN];
						log_print(LOGN_INFO, "MESSAGE [DIRECTOR SYS]");

						dMakeDirectMngPkt(pstMngPkt, &fidb->stDirectTOT);

						if( (dRet = SendToOMP(pstMngPkt)) < 0)
							log_print(LOGN_WARN, "F=%s:%s.%d: ERROR IN SendToOMP() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);

						continue;
					}
				}
				else if( (pstMsgQSub->usSvcID == SID_STATUS) && (pstMsgQSub->usMsgID == MID_TRAFFIC))
				{
					memcpy(&stMsg2, &stMsg, DEF_MSGQ_SIZE);

					dSend_Traffic_FSTAT(&stMsg2);
					continue;
				}
				/*** SERVICE MONITORING **********************************************************/
				else if( (pstMsgQSub->usSvcID == SID_STATUS) && (pstMsgQSub->usMsgID == MID_SVC_MONITOR))
				{
					pSvcMonMsg = (st_SvcMonMsg *)stMsg.szBody;

					log_print(LOGN_INFO, "[SVC MON][RCVDATA]SID[%d] MID[%d] PROID[%d] TIME[%ld] IDX[%u]",
						pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, stMsg.ucProID, pSvcMonMsg->lTime, pSvcMonMsg->uiIdx);

					pMonList = &gMonTotal->stMonList[pSvcMonMsg->uiIdx];

					memset(&stMsgHead, 0x00, MNG_PKT_HEAD_SIZE);

					stMsgHead.llMagicNumber = MAGIC_NUMBER;
					stMsgHead.usBodyLen	= DEF_MONLIST_SIZE;
					stMsgHead.usTotLen	= MNG_PKT_HEAD_SIZE + stMsgHead.usBodyLen;
					stMsgHead.ucNTAMID	= 0;
					stMsgHead.ucNTAFID	= 0;
					stMsgHead.ucSysNo	= 0;
					stMsgHead.ucSvcID	= pstMsgQSub->usSvcID;
					stMsgHead.ucMsgID	= pstMsgQSub->usMsgID;

					log_print(LOGN_DEBUG, "[SVC MON][PKTHDR]MAGICNUMBER[%lld] TL[%d] BL[%d] SYSNO[%d] MID=%d SID=%d",
						stMsgHead.llMagicNumber, stMsgHead.usTotLen, stMsgHead.usBodyLen,
						stMsgHead.ucSysNo, stMsgHead.ucMsgID, stMsgHead.ucSvcID);

					/* send header */
					if((dRet = SendToOMPMon(MNG_PKT_HEAD_SIZE, (char *)&stMsgHead)) < 0)
						log_print(LOGN_WARN, "[SVC MON] HEADER FAILED IN SendToOMP()[%d]", dRet);
					else
						log_print(LOGN_DEBUG, "[SVC MON] HEADER SEND TO OMP SUCCESS[%d]", dRet);

					/* send data */
					if((dRet = SendToOMPMon(DEF_MONLIST_SIZE, (char *)pMonList)) < 0)
						log_print(LOGN_WARN, "[SVC MON] DATA FAILED IN SendToOMP()[%d]", dRet);
					else
						log_print(LOGN_DEBUG, "[SVC MON] DATA SEND TO OMP SUCCESS[%d]", dRet);

					continue;
				}
				else if( (pstMsgQSub->usSvcID == SID_SVC) && (pstMsgQSub->usMsgID == MID_SVC_STATCP))
				{
					pstMngPkt  = (pst_MngPkt)&stMsg.szBody;

					if( (dRet = SendToOMP(pstMngPkt)) < 0)
						log_print(LOGN_WARN, "F=%s:%s.%d: ERROR IN SendToOMP() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
				}
				else
				{
					log_print(LOGN_INFO, "UNKNOWN MSG: PROCID[%d] SID[%d] MID[%d] llNID[%lld]", stMsg.ucProID, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, stMsg.llNID);
					log_hexa( (unsigned char*)&stMsg.llMType,400);
				}
			}
			break;
		} /*** MESSAGE QUEUE WHILE ***/

#if 0
		/*
		* SEND NTAM STATUS
		*/
		if( abs( (now = time(NULL)) - check_snd) > CHECK_SEND_TIME)
		{
			/*
			* SEND ACTIVE DATA
			*/
			pstNTAM		= (pst_NTAM)&stMsg.szBody[NTAFT_HEADER_LEN+MNG_PKT_HEAD_SIZE];
			pstMngPkt	= (pst_MngPkt)&stMsg.szBody[NTAFT_HEADER_LEN];

			fidb->stNTAM.tUpTime	= now;
			fidb->stNTAM.active		= 1;

			dCheckLoadNTAM(&fidb->stNTAM);

			dMakeNTAMMngPkt(pstMngPkt, &fidb->stNTAM, ucNTAMID);

			stMsg2.ucNTAMID		= ucNTAMID;
			stMsg2.ucNTAFID		= 0;
			dSend_FSTAT(&stMsg2, (char*)&fidb->stNTAM);

			dRet = SendToOMP(pstMngPkt);
			if(dRet < 0)
				log_print(LOGN_WARN, "FAILED IN SendToOMP()");
			else
				log_print(LOGN_INFO, "[SUCCESS SEND NTAM STATUS TO OMP (NTAM:%d)]", ucNTAMID);

			/*	DIRECTOR STATUS DATA	*/
			if( (now%5) == 0)
			{
				log_print(LOGN_INFO, "MESSAGE [DIRECTOR SYS]");

				dMakeDirectMngPkt(pstMngPkt, &fidb->stDirectTOT);

				if( (dRet = SendToOMP(pstMngPkt)) < 0)
					log_print(LOGN_WARN, "F=%s:%s.%d: ERROR IN SendToOMP() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
			}

			check_snd = now;
		}
#endif

		/*
		* SEND 5 MIN STATISTICS FOR OMP GRAPH
		*/
		dRet = dSocketCheck();
	} /*** WHILE LOOP END ***/

	log_print(LOGN_CRI, "PROGRAM END !!!\n");
	FinishProgram();

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dAcceptClient( int *dSocketFd, int *dConTblIdx )
{
	int     i;
    int     dSfd, dOpt;
	int		dSBufLen, dSBufLen1, dRBufLen, dRBufLen1;
    int     dCliLen = sizeof(struct sockaddr_in);
    struct  in_addr inaddr;
    struct  linger  stLinger;
    struct  sockaddr_in stCli_addr;

	dSfd = accept( gdSvrSfd, (struct sockaddr*)&stCli_addr, &dCliLen );
    if( dSfd < 0 ) {
        log_print( LOGN_CRI, "[SOCKET] NEW CLIENT ACCEPT FAIL : [%d][%s]",
            errno, strerror(errno));
        return -1;
    }

	dCliLen = sizeof(stLinger);
	getsockopt( dSfd, SOL_SOCKET,SO_LINGER,(void *)&stLinger, &dCliLen);

	log_print(LOGN_DEBUG, "L_ONOFF[%d] L_LINGER[%d]",
			stLinger.l_onoff, stLinger.l_linger );

	/*
	* SET LINGER OPTION
	*/
    dOpt = 1;
    stLinger.l_onoff = 0;
    stLinger.l_linger = 0;

	/*
	* SET SOCKET BUFFER
	*/
	dSBufLen = 40738000;
	dSBufLen1 = sizeof(dSBufLen);
	dRBufLen = 873800;
    dRBufLen1 = sizeof(dRBufLen);


	/*
	* SET SOCKET OPTIONS
	*/
	if(setsockopt(dSfd, SOL_SOCKET, SO_KEEPALIVE , (void *)&dOpt, sizeof(dOpt)) < 0) {
        log_print(LOGN_CRI, "FAIL IN SET TO KEEPALIVE, errno=[%d] %s",
            errno, strerror(errno));
        close(dSfd);
        return -2;
    }
    else if (fcntl(dSfd, F_SETFL, O_NDELAY) < 0) {
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
	else if(setsockopt (dSfd, SOL_SOCKET, SO_SNDBUF, (char *)&dSBufLen, dSBufLen1) < 0) {
		log_print(LOGN_CRI,
            "FAIL IN CHANGING SOCKET OPTION : SEND BUFFER, errno=[%d] %s",
            errno, strerror(errno));
        close (dSfd);
        return -5;
	}
	else if(setsockopt (dSfd, SOL_SOCKET, SO_RCVBUF, (char *)&dRBufLen, dRBufLen1) < 0) {
		log_print(LOGN_CRI,
            "FAIL IN CHANGING SOCKET OPTION : RECEIVE BUFFER, errno=[%d] %s",
            errno, strerror(errno));
        close (dSfd);
        return -6;
	}

	/*
	* GET CONNECTION TABLE INDEX IN EMPTY
	*/
    for(i=0; i<gdClient; i++ ) {
        if( stConTbl[i].dSfd < 0 )
            break;
    }

	/*
	* CASE IN MAX CLIENT
	*/
    if(i == gdMaxCliCnt) {
        log_print(LOGN_CRI,"[INFO] MAX CLIENT OVERFLOW  IDX[%d] sfd[%d]", i, dSfd);
        close(dSfd);
        return -7;
    }

	stConTbl[i].dSfd = dSfd;
	stConTbl[i].cSockBlock = 0x00;
	stConTbl[i].dBufLen = 0;

    gstMsgBuf[i].dWidx=0;
    gstMsgBuf[i].dRidx=0;

    FD_SET( dSfd, &gstReadfds);

    if( dSfd > gdNumfds )
        gdNumfds = dSfd;

    if( i >= gdClient )
        gdClient = i + 1;

    stConTbl[i].uiCliIP = stCli_addr.sin_addr.s_addr;
    inaddr.s_addr = stCli_addr.sin_addr.s_addr;

    log_print( LOGN_CRI, "[SOCKET] [CONNECT] NEW CLIENT IP[%s] STCON_IDX[%d] STCON_Sfd[%d] gdClient[%d]",
          inet_ntoa(inaddr), i, stConTbl[i].dSfd, gdClient );

	*dSocketFd = dSfd;
	*dConTblIdx = i;

    return 0;
}

/*******************************************************************************

*******************************************************************************/
int	dSocketCheck(void)
{
	int					i, dSfd, dIdx, dRecvLen, dSelRet, dRet;
	char				szTcpRmsg[MAX_MNG_PKT_BUFSIZE];

	fd_set				stRd, stWd;
	struct timeval		timeout;
	struct in_addr		inaddr;

	memcpy((char*)&stRd, (char*)&gstReadfds, sizeof(fd_set));
	memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));

	timeout.tv_sec		= 0;
	timeout.tv_usec		= 1000;

	/*
	* MAIN SOCKET SELECT
	*/
	dSelRet = select(gdNumfds+1, &stRd, &stWd, (fd_set*)NULL, &timeout);
	if(dSelRet < 0)
	{
		if( errno != EINTR )
		{
			/*
			* SELECT UNEXPECTED ERROR
			*/
			log_print(LOGN_CRI, "!!! Select Critical Error Invoke [%d] [%s]", errno, strerror(errno));
			FinishProgram();
		}

		return 0;
	}
	else if(dSelRet > 0)
	{
		/*
		* EVENT INVOKE
		*/
		if(FD_ISSET(gdSvrSfd, &stRd))
		{
			/*
			* NEW CLIENT
			*/
			log_print(LOGN_DEBUG, "New Client Connect Trial SvrPort[%d]", gdPortNo);

			dRet = dAcceptClient(&dSfd, &dIdx);
			if(dRet < 0)
			{
				log_print(LOGN_CRI, "[ERROR] dAcceptClient dRet[%d]", dRet);
			}
		}

		/*
		* CHECK FOR READ FD_SET
		*/
		for(i = 0; i < gdClient; i++)
		{
			if(stConTbl[i].dSfd < 0)
				continue;

			dSfd = stConTbl[i].dSfd;

			if(FD_ISSET(dSfd, &stRd))
			{
				/*
				* EXIST CLIENT
				*/
				log_print(LOGN_DEBUG, "Exist Client IDX[%d] Sfd[%d]", i, dSfd);

				szTcpRmsg[0]	= 0x00;
				dRecvLen		= 0;
				dRet = dRecvMessage(dSfd, szTcpRmsg, &dRecvLen);
				if(dRet < 0)
				{
					inaddr.s_addr = stConTbl[i].uiCliIP;
					log_print(LOGN_CRI, "[SOCKET] [CLOSE  ] CONNECTION CLOSE IDX[%d] SFD[%d] IP[%s]", i, dSfd, inet_ntoa(inaddr));
					close(dSfd);

					if(gdClient == 1)
					{
						gdClient--;
						gdNumfds = gdSvrSfd;
					}
					else if( (gdClient - 1) == i)
					{
						gdClient--;

						while(1)
						{
							if(stConTbl[gdClient-1].dSfd > 0)
							{
								gdNumfds = dGetMaxFds();
								break;
							}
							else
							{
								gdClient--;

								if(gdClient == 0)
								{
									gdNumfds = gdSvrSfd;
									break;
								}
							}
						}
					}
					log_print(LOGN_DEBUG, "gdClient: [%d]", gdClient);

					FD_CLR(dSfd, &gstReadfds);
					stConTbl[i].dSfd	= -1;
					stConTbl[i].dBufLen	= 0;

					/*
					* ADD 2003.05.13.
					*/
					memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));
					if(FD_ISSET(dSfd, &stWd))
					{
						log_print(LOGN_DEBUG, "CLEAR WRITEFDS FOR SOCKET CLOSE SFD[%d]", dSfd);
						FD_CLR(dSfd, &gstWritefds);
						gdNumWfds--;
						stConTbl[i].cSockBlock	= 0x00;
					}
				}
				else
				{
					dRet = dMergeBuffer(i, dSfd, szTcpRmsg, dRecvLen);
					if(dRet < 0)
					{
						inaddr.s_addr = stConTbl[i].uiCliIP;
						log_print(LOGN_CRI, "[SOCKET] [CLOSE  ] PACKET ANALYSIS ERROR IDX[%d] sfd[%d] IP[%s]", i, dSfd, inet_ntoa(inaddr));
						close(dSfd);

						if(gdClient == 1)
						{
							gdClient--;
							gdNumfds = gdSvrSfd;
						}
						else if( (gdClient-1) == i)
						{
							gdClient--;

							while(1)
							{
								if(stConTbl[gdClient-1].dSfd > 0)
								{
									gdNumfds = dGetMaxFds();
									break;
								}
								else
								{
									gdClient--;

									if(gdClient == 0)
									{
										gdNumfds = gdSvrSfd;
										break;
									}
								}
							}
						}
						log_print(LOGN_DEBUG, "gdClient : [%d]", gdClient );

						FD_CLR(dSfd, &gstReadfds);
						stConTbl[i].dSfd	= -1;
						stConTbl[i].dBufLen	= 0;

						memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));
						if(FD_ISSET(dSfd, &stWd))
						{
							log_print(LOGN_DEBUG,"CLEAR WRITEFDS FOR SOCKET CLOSE SFD[%d]", dSfd);

							FD_CLR(dSfd, &gstWritefds);

							gdNumWfds--;
							stConTbl[i].cSockBlock	= 0x00;
						}
					}
				}
			}
		}

		if(gdNumWfds > 0)
		{
			/*
			* WRITEFDS EXIST CHECK
			*/
			for(i = 0; i < gdClient; i++)
			{
				if(stConTbl[i].dSfd < 0)
					continue;

				if(stConTbl[i].cSockBlock == 0x00)
					continue;

				dSfd = stConTbl[i].dSfd ;

				if(FD_ISSET(dSfd, &stWd))
				{
					log_print(LOGN_DEBUG, "WriteFds Event Invoke SFD[%d]", dSfd);

					FD_CLR(dSfd, &gstWritefds);

					gdNumWfds--;
					stConTbl[i].cSockBlock = 0x00;

					if(stConTbl[i].dBufLen > 0)
					{
						dRet = dSendMessage(dSfd, stConTbl[i].dBufLen, &stConTbl[i].szSendBuffer[0], i);
						if(dRet < 0)
						{
							inaddr.s_addr = stConTbl[i].uiCliIP;
							log_print(LOGN_CRI, "[SOCKET] [CLOSE  ] SEND MESSAGE FAIL IDX[%d] SFD[%d] IP[%s]", i, dSfd, inet_ntoa(inaddr));

							close(dSfd);
							stConTbl[i].dSfd	= -1;
							stConTbl[i].dBufLen	= 0;
							FD_CLR(dSfd, &gstReadfds);

							if(gdClient == 1)
							{
								gdNumfds = gdSvrSfd;
								gdClient--;
							}
							else if( (gdClient-1) == i)
							{
								gdClient--;

								while(1)
								{
									if(stConTbl[gdClient-1].dSfd > 0)
									{
										gdNumfds = dGetMaxFds();
										break;
									}
									else
									{
										gdClient--;

										if(gdClient == 0)
										{
											gdNumfds = gdSvrSfd;
											break;
										}
									}
								}
							}
							log_print(LOGN_DEBUG, "gdClient: [%d]", gdClient);

							memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));
							if(FD_ISSET(dSfd, &stWd))
							{
								log_print(LOGN_DEBUG,"CLEAR WRITEFDS FOR SOCKET CLOSE SFD[%d]", dSfd);

								FD_CLR(dSfd, &gstWritefds);

								gdNumWfds--;
								stConTbl[i].cSockBlock = 0x00;
							}
						}
						else if(dRet == 2)
						{
							/* re-block */
						}
						else
						{
							log_print(LOGN_DEBUG, "SEND SENDBUFER DATA: SIZE[%d]", stConTbl[i].dBufLen);
							stConTbl[i].dBufLen = 0;
						}
					}
				} /* if( FD_ISSET( dSfd, &stWd ) )*/
			}
		}
	}

	return 0;
}


/*******************************************************************************
 SEND TO OMP
*******************************************************************************/
int SendToOMP(pst_MngPkt pstSpkt)
{
	int				i, dRet, dSfd;

	struct in_addr	inaddr;

	for(i = 0; i < gdClient; i++)
	{
		if( (stConTbl[i].dSfd > 0) && (stConTbl[i].cSockBlock != 0x01))
		{
			dSfd	= stConTbl[i].dSfd;

			dRet	= dSendMessage(dSfd, pstSpkt->head.usTotLen, (char*)pstSpkt, i);
			if(dRet < 0)
			{
				inaddr.s_addr	= stConTbl[i].uiCliIP;
				log_print(LOGN_CRI, "[SOCKET] [CLOSE  ] SEND MESSAGE FAIL IDX[%d] SFD[%d] IP[%s]", i, dSfd, inet_ntoa(inaddr));

				close(dSfd);
				stConTbl[i].dSfd	= -1;
				stConTbl[i].dBufLen	= 0;
				FD_CLR(dSfd, &gstReadfds);

				if(gdClient == 1)
				{
					gdClient--;
					gdNumfds = gdSvrSfd;

					log_print(LOGN_DEBUG, "GDNumfds INFO 3: [%d]", gdNumfds);
				}
				else if( (gdClient-1) == i)
				{
					gdClient--;

					while(1)
					{
						if(stConTbl[gdClient-1].dSfd > 0)
						{
							gdNumfds = dGetMaxFds();
							break;
						}
						else
						{
							gdClient--;

							if(gdClient == 0)
							{
								log_print(LOGN_DEBUG, "GDNumfds INFO 4 : [%d]", gdNumfds );
								gdNumfds = gdSvrSfd;

								break;
							}
						}
					}
				}
				log_print(LOGN_DEBUG, "gdClient : [%d]", gdClient );

#if 0
				/*** ADD 2003.05.13. ***/
				memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));
				if(FD_ISSET(dSfd, &stWd))
				{
					log_print(LOGN_DEBUG,"CLEAR WRITEFDS FOR SOCKET CLOSE SFD[%d]", dSfd );

					FD_CLR(dSfd, &gstWritefds);

					gdNumWfds--;
					stConTbl[i].cSockBlock = 0x00;
				}
#endif
				return -1;
			}
		}
	}

	return 1;
}

/*******************************************************************************
 SEND TO OMP : Monitoring Data
*******************************************************************************/
int SendToOMPMon(int length, char *data)
{
	int				i, dRet, dSfd;

	struct in_addr	inaddr;

	for(i = 0; i < gdClient; i++)
	{
		if( (stConTbl[i].dSfd > 0) && (stConTbl[i].cSockBlock != 0x01))
		{
			dSfd	= stConTbl[i].dSfd;

			dRet	= dSendMessage(dSfd, length, data, i);
			if(dRet < 0)
			{
				inaddr.s_addr	= stConTbl[i].uiCliIP;
				log_print(LOGN_CRI, "[SOCKET] [CLOSE  ] SEND MESSAGE FAIL IDX[%d] SFD[%d] IP[%s]", i, dSfd, inet_ntoa(inaddr));

				close(dSfd);
				stConTbl[i].dSfd	= -1;
				stConTbl[i].dBufLen	= 0;
				FD_CLR(dSfd, &gstReadfds);

				if(gdClient == 1)
				{
					gdClient--;
					gdNumfds = gdSvrSfd;

					log_print(LOGN_DEBUG, "GDNumfds INFO 3: [%d]", gdNumfds);
				}
				else if( (gdClient-1) == i)
				{
					gdClient--;

					while(1)
					{
						if(stConTbl[gdClient-1].dSfd > 0)
						{
							gdNumfds = dGetMaxFds();
							break;
						}
						else
						{
							gdClient--;

							if(gdClient == 0)
							{
								log_print(LOGN_DEBUG, "GDNumfds INFO 4 : [%d]", gdNumfds );
								gdNumfds = gdSvrSfd;

								break;
							}
						}
					}
				}
				log_print(LOGN_DEBUG, "gdClient : [%d]", gdClient );
#if 0
				/*** ADD 2003.05.13. ***/
				memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));
				if(FD_ISSET(dSfd, &stWd))
				{
					log_print(LOGN_DEBUG,"CLEAR WRITEFDS FOR SOCKET CLOSE SFD[%d]", dSfd );

					FD_CLR(dSfd, &gstWritefds);

					gdNumWfds--;
					stConTbl[i].cSockBlock = 0x00;
				}
#endif
				return -1;
			}
		}
	}

	return 1;
}


/*******************************************************************************

*******************************************************************************/
int dCheckLoadNTAM( st_NTAM *stNTAM )
{
	int			i;
    char        cCurrStatus;
    long long   llCurVal;

	float		fPercent;

	cCurrStatus	= 0;

    /*** CHECK CPU ************************************************************/
	if( stNTAM->cpusts.llCur > 1000 )
		stNTAM->cpusts.llCur = 1000;

    llCurVal = ((float)stNTAM->cpusts.llCur)/10.0;
	fPercent = ((float)stNTAM->cpusts.llCur)/10.0;

	if( gpKeepAlive->stTAMLoad.cpu.usMinor == 0 && gpKeepAlive->stTAMLoad.cpu.usMajor == 0 &&
		gpKeepAlive->stTAMLoad.cpu.usCritical == 0 )
		cCurrStatus = NORMAL;
    else if( llCurVal < gpKeepAlive->stTAMLoad.cpu.usMinor )
        cCurrStatus = NORMAL;
    else if( llCurVal >= gpKeepAlive->stTAMLoad.cpu.usMinor &&
             llCurVal < gpKeepAlive->stTAMLoad.cpu.usMajor )
        cCurrStatus = MINOR;
    else if( llCurVal >= gpKeepAlive->stTAMLoad.cpu.usMajor &&
             llCurVal < gpKeepAlive->stTAMLoad.cpu.usCritical )
        cCurrStatus = MAJOR;
    else if( llCurVal >= gpKeepAlive->stTAMLoad.cpu.usCritical )
        cCurrStatus = CRITICAL;


    /*** SEND TO COND *****/
	if( fidb->stNTAM.cpu == NOT_EQUIP )
    {
        if( cCurrStatus > NORMAL )
        {
            Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_CPU, 0, cCurrStatus, NORMAL, llCurVal );

            stNTAM->cpu = cCurrStatus;
        }
        else
            stNTAM->cpu = cCurrStatus;
    }
    else if( cCurrStatus > fidb->stNTAM.cpu )
    {
        gdNTAMcpuFailCnt++;

        if( gdNTAMcpuFailCnt > MAX_FAIL_COUNT )
        {
            if( fidb->stNTAM.cpu == NORMAL )
            {
                Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_CPU, 0, cCurrStatus, NORMAL, llCurVal );
            }
            else
            {
                Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_CPU, 0, NORMAL, fidb->stNTAM.cpu, llCurVal );

                Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_CPU, 0, cCurrStatus, NORMAL, llCurVal );
            }

            gdNTAMcpuFailCnt = 0;

            stNTAM->cpu = cCurrStatus;
        }
		else
		{
			stNTAM->cpu = fidb->stNTAM.cpu;
		}
    }
	else if( cCurrStatus < fidb->stNTAM.cpu )
    {
        gdNTAMcpuFailCnt = 0;

        if( cCurrStatus == NORMAL )
        {
            Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_CPU, 0, NORMAL, fidb->stNTAM.cpu, llCurVal );
        }
        else
        {
            Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_CPU, 0, NORMAL, fidb->stNTAM.cpu, llCurVal );

            Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_CPU, 0, cCurrStatus, NORMAL, llCurVal );
        }

        stNTAM->cpu = cCurrStatus;
    }
    else if( cCurrStatus == fidb->stNTAM.cpu )
    {
        stNTAM->cpu = cCurrStatus;
    }

	dGetLoadValueStat( SYSTYPE_TAM, 1, INVTYPE_CPU, fPercent );


	/*** CHECK MEM ************************************************************/
	if( stNTAM->memsts.lMax != 0 )
	{
		llCurVal = ((float)stNTAM->memsts.llCur/(float)stNTAM->memsts.lMax)*100.0;
		fPercent = ((float)stNTAM->memsts.llCur/(float)stNTAM->memsts.lMax)*100.0;

		if( fPercent > 100 ) {
			stNTAM->memsts.llCur = stNTAM->memsts.lMax;
			llCurVal = ((float)stNTAM->memsts.llCur/(float)stNTAM->memsts.lMax)*100.0;
			fPercent = ((float)stNTAM->memsts.llCur/(float)stNTAM->memsts.lMax)*100.0;
		}

		if( gpKeepAlive->stTAMLoad.mem.usMinor == 0 && gpKeepAlive->stTAMLoad.mem.usMajor == 0 &&
			gpKeepAlive->stTAMLoad.mem.usCritical == 0 )
			cCurrStatus = NORMAL;
		else if( llCurVal < gpKeepAlive->stTAMLoad.mem.usMinor )
			cCurrStatus = NORMAL;
		else if( llCurVal >= gpKeepAlive->stTAMLoad.mem.usMinor &&
				 llCurVal < gpKeepAlive->stTAMLoad.mem.usMajor )
			cCurrStatus = MINOR;
		else if( llCurVal >= gpKeepAlive->stTAMLoad.mem.usMajor &&
				 llCurVal < gpKeepAlive->stTAMLoad.mem.usCritical )
			cCurrStatus = MAJOR;
		else if( llCurVal >= gpKeepAlive->stTAMLoad.mem.usCritical )
			cCurrStatus = CRITICAL;


		/*** SEND TO COND *****/
		if( fidb->stNTAM.mem == NOT_EQUIP )
		{
			if( cCurrStatus > NORMAL )
			{
				Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_MEMORY, 0, cCurrStatus, NORMAL, llCurVal );

				stNTAM->mem = cCurrStatus;
			}
			else
				stNTAM->mem = cCurrStatus;
		}
		else if( cCurrStatus > fidb->stNTAM.mem )
		{
			gdNTAMmemFailCnt++;

			if( gdNTAMmemFailCnt > MAX_FAIL_COUNT )
			{
				if( fidb->stNTAM.mem == NORMAL )
				{
					Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_MEMORY, 0, cCurrStatus, NORMAL, llCurVal );
				}
				else
				{
					Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_MEMORY, 0, NORMAL, fidb->stNTAM.mem, llCurVal );

					Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_MEMORY, 0, cCurrStatus, NORMAL, llCurVal );
				}

				gdNTAMmemFailCnt = 0;

				stNTAM->mem = cCurrStatus;
			}
			else
			{
				stNTAM->mem = fidb->stNTAM.mem;
			}

		}
		else if( cCurrStatus < fidb->stNTAM.mem )
		{
			gdNTAMmemFailCnt = 0;

			if( cCurrStatus == NORMAL )
			{
				Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_MEMORY, 0, NORMAL, fidb->stNTAM.mem, llCurVal );
			}
			else
			{
				Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_MEMORY, 0, NORMAL, fidb->stNTAM.mem, llCurVal );

				Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_MEMORY, 0, cCurrStatus, NORMAL, llCurVal );
			}

			stNTAM->mem = cCurrStatus;
		}
		else if( cCurrStatus == fidb->stNTAM.mem )
		{
			stNTAM->mem = cCurrStatus;
		}

		dGetLoadValueStat( SYSTYPE_TAM, 1, INVTYPE_MEMORY, fPercent );

	}


    /*** CHECK DISK ***********************************************************/
	for( i=0; i<MAX_DISK_COUNT; i++)
	{
		if( stNTAM->disksts[i].lMax == 0 )
			continue;

    	llCurVal = ((float)stNTAM->disksts[i].llCur/(float)stNTAM->disksts[i].lMax)*100.0;
		fPercent = ((float)stNTAM->disksts[i].llCur/(float)stNTAM->disksts[i].lMax)*100.0;

		if( fPercent > 100 ) {
			stNTAM->disksts[i].llCur = stNTAM->disksts[i].lMax;
			llCurVal = ((float)stNTAM->disksts[i].llCur/(float)stNTAM->disksts[i].lMax)*100.0;
			fPercent = ((float)stNTAM->disksts[i].llCur/(float)stNTAM->disksts[i].lMax)*100.0;
		}

		if( gpKeepAlive->stTAMLoad.disk.usMinor == 0 && gpKeepAlive->stTAMLoad.disk.usMajor == 0 &&
			gpKeepAlive->stTAMLoad.disk.usCritical == 0 )
			cCurrStatus = NORMAL;
    	else if( llCurVal < gpKeepAlive->stTAMLoad.disk.usMinor )
        	cCurrStatus = NORMAL;
    	else if( llCurVal >= gpKeepAlive->stTAMLoad.disk.usMinor &&
             	 llCurVal < gpKeepAlive->stTAMLoad.disk.usMajor )
        	cCurrStatus = MINOR;
    	else if( llCurVal >= gpKeepAlive->stTAMLoad.disk.usMajor &&
             	 llCurVal < gpKeepAlive->stTAMLoad.disk.usCritical )
        	cCurrStatus = MAJOR;
    	else if( llCurVal >= gpKeepAlive->stTAMLoad.disk.usCritical )
        	cCurrStatus = CRITICAL;

		log_print(LOGN_DEBUG,"[START DSK CHK %d]:[%6.2f] {%d}{%d}", i, fPercent, cCurrStatus, fidb->stNTAM.disk[i] );

    	/*** SEND TO FSTATD *****/
		if( fidb->stNTAM.disk[i] == NOT_EQUIP )
        {
            if( cCurrStatus > NORMAL )
            {
                Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_DISK, i, cCurrStatus, NORMAL, llCurVal );

                stNTAM->disk[i] = cCurrStatus;
            }
            else
                stNTAM->disk[i] = cCurrStatus;
        }
        else if( cCurrStatus > stNTAM->disk[i] )
        {
            gdNTAMdskFailCnt[i]++;

            if( gdNTAMdskFailCnt[i] > MAX_FAIL_COUNT )
            {
                if( fidb->stNTAM.disk[i] == NORMAL )
                {
                    Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_DISK, i, cCurrStatus, NORMAL, llCurVal );
                }
                else
                {
                    Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_DISK, i, NORMAL, fidb->stNTAM.disk[i], llCurVal );

                    Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_DISK, i, cCurrStatus, NORMAL, llCurVal );
                }

                gdNTAMdskFailCnt[i] = 0;

                stNTAM->disk[i] = cCurrStatus;
            }
			else
			{
				stNTAM->disk[i] = stNTAM->disk[i];
			}
        }
		else if( cCurrStatus < stNTAM->disk[i] )
        {
            gdNTAMdskFailCnt[i] = 0;

            if( cCurrStatus == NORMAL )
            {
                Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_DISK, i, NORMAL, fidb->stNTAM.disk[i], llCurVal );
            }
            else
            {
                Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_DISK, i, NORMAL, fidb->stNTAM.disk[i], llCurVal );

                Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_DISK, i, cCurrStatus, NORMAL, llCurVal );
            }

            stNTAM->disk[i] = cCurrStatus;
        }
        else if( cCurrStatus == stNTAM->disk[i] )
        {
            stNTAM->disk[i] = cCurrStatus;
        }

	}

    /*** CHECK QUEUE **********************************************************/
	if( stNTAM->queuests.lMax != 0 )
	{
		llCurVal = ((float)stNTAM->queuests.llCur/(float)stNTAM->queuests.lMax)*100.0;
		fPercent = ((float)stNTAM->queuests.llCur/(float)stNTAM->queuests.lMax)*100.0;

		if( fPercent > 100 ) {
			stNTAM->queuests.llCur = stNTAM->queuests.lMax;
			llCurVal = ((float)stNTAM->queuests.llCur/(float)stNTAM->queuests.lMax)*100.0;
			fPercent = ((float)stNTAM->queuests.llCur/(float)stNTAM->queuests.lMax)*100.0;
		}

		if( gpKeepAlive->stTAMLoad.que.usMinor == 0 && gpKeepAlive->stTAMLoad.que.usMajor == 0 &&
			gpKeepAlive->stTAMLoad.que.usCritical == 0 )
			cCurrStatus = NORMAL;
		else if( llCurVal < gpKeepAlive->stTAMLoad.que.usMinor )
			cCurrStatus = NORMAL;
		else if( llCurVal >= gpKeepAlive->stTAMLoad.que.usMinor &&
				 llCurVal < gpKeepAlive->stTAMLoad.que.usMajor )
			cCurrStatus = MINOR;
		else if( llCurVal >= gpKeepAlive->stTAMLoad.que.usMajor &&
				 llCurVal < gpKeepAlive->stTAMLoad.que.usCritical )
			cCurrStatus = MAJOR;
		else if( llCurVal >= gpKeepAlive->stTAMLoad.que.usCritical )
			cCurrStatus = CRITICAL;

		log_print(LOGN_DEBUG,"[START QUE CHECK]:[%6.2f] {%d}{%d}", fPercent, cCurrStatus, fidb->stNTAM.queue );

		/*** SEND TO FSTATD *****/
		if( fidb->stNTAM.queue == NOT_EQUIP )
		{
			if( cCurrStatus > NORMAL )
			{
				Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_QUEUE, 0, cCurrStatus, NORMAL, llCurVal );

				stNTAM->queue = cCurrStatus;
			}
			else
				stNTAM->queue = cCurrStatus;
		}
		else if( cCurrStatus > fidb->stNTAM.queue )
		{
			gdNTAMqueFailCnt++;

			if( gdNTAMqueFailCnt > MAX_FAIL_COUNT )
			{
				if( fidb->stNTAM.queue == NORMAL )
				{
					Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_QUEUE, 0, cCurrStatus, NORMAL, llCurVal );
				}
				else
				{
					Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_QUEUE, 0, NORMAL, fidb->stNTAM.queue, llCurVal );

					Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_QUEUE, 0, cCurrStatus, NORMAL, llCurVal );
				}

				gdNTAMqueFailCnt = 0;

				stNTAM->queue = cCurrStatus;
			}
			else
			{
				stNTAM->queue = fidb->stNTAM.queue;
			}
		}
		else if( cCurrStatus < fidb->stNTAM.queue )
		{
			gdNTAMqueFailCnt = 0;

			if( cCurrStatus == NORMAL )
			{
				Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_QUEUE, 0, NORMAL, fidb->stNTAM.queue, llCurVal );
			}
			else
			{
				Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_QUEUE, 0, NORMAL, fidb->stNTAM.queue, llCurVal );

				Send_CondMess(SYSTYPE_TAM, 0, LOCTYPE_LOAD, INVTYPE_QUEUE, 0, cCurrStatus, NORMAL, llCurVal );
			}

			stNTAM->queue = cCurrStatus;
		}
		else if( cCurrStatus == fidb->stNTAM.queue )
		{
			stNTAM->queue = cCurrStatus;
		}

		dGetLoadValueStat( SYSTYPE_TAM, 1, INVTYPE_QUEUE, fPercent );

	}

    return 1;
}


/*******************************************************************************

*******************************************************************************/
int dGetLoadValueStat( int SysType, int SysNo, int InvType, float fVal )
{
	int				i;
    time_t          t;
    unsigned short  usCurIdx;
    int             dTmpCnt;

    t = time(&t);

    usCurIdx = ((t/300)%12);
	if( usCurIdx != loadstat->usCurIdx )
	{
		log_print(LOGN_DEBUG, "CURR IDX[%d] : SHM IDX[%d]", usCurIdx, loadstat->usCurIdx );

		loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stCpu.fMin = 100.0;
		loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stMem.fMin = 100.0;
		loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stDisk.fMin = 100.0;
		loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stQueue.fMin = 100.0;

		for(i=0; i<MAX_NTAF_COUNT;i++) {
			loadstat->stWNTAMLOAD[usCurIdx].stNTAF[i].stCpu.fMin = 100.0;
			loadstat->stWNTAMLOAD[usCurIdx].stNTAF[i].stMem.fMin = 100.0;
			loadstat->stWNTAMLOAD[usCurIdx].stNTAF[i].stDisk.fMin = 100.0;
			loadstat->stWNTAMLOAD[usCurIdx].stNTAF[i].stQueue.fMin = 100.0;
		}
	}

    loadstat->usCurIdx = usCurIdx ;

    loadstat->stWNTAMLOAD[usCurIdx].tWhen = ((t/300)*300);

    switch( SysType )
    {
        case SYSTYPE_TAM:
            switch( InvType )
            {
                case INVTYPE_CPU:
					dTmpCnt = loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stCpu.dCnt;
					loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stCpu.dCnt++;

                    dSetAvgVal( &loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stCpu.fAvg, fVal, dTmpCnt );
                    dSetMinMaxVal( &loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stCpu.fMin,
                                   &loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stCpu.fMax, fVal );
                    break;

                case INVTYPE_MEMORY:
                    dTmpCnt = loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stMem.dCnt;
					loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stMem.dCnt++;

                    dSetAvgVal( &loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stMem.fAvg, fVal, dTmpCnt );
                    dSetMinMaxVal( &loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stMem.fMin,
                                   &loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stMem.fMax, fVal );

                    break;

                case INVTYPE_DISK:
                    dTmpCnt = loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stDisk.dCnt;
					loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stDisk.dCnt++;

                    dSetAvgVal( &loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stDisk.fAvg, fVal, dTmpCnt );
                    dSetMinMaxVal( &loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stDisk.fMin,
                                   &loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stDisk.fMax, fVal );
                    break;

                case INVTYPE_QUEUE:
                    dTmpCnt = loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stQueue.dCnt;
					loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stQueue.dCnt++;

                    dSetAvgVal( &loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stQueue.fAvg, fVal, dTmpCnt );
                    dSetMinMaxVal( &loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stQueue.fMin,
                                   &loadstat->stWNTAMLOAD[usCurIdx].stNTAM.stQueue.fMax, fVal );
                    break;

                default:

                    break;
            }

            break;

		case SYSTYPE_TAF:
				log_print(LOGN_WARN,"DON'T MORE MANAGE NTAF");

            break;

        default:
			break;
    }

    return 1;
}


/*******************************************************************************

*******************************************************************************/
int dSetMinMaxVal( float *MinVal, float *MaxVal, float CurVal )
{
	if( *MinVal > CurVal ) {
		*MinVal = CurVal;
	}
	else if( *MaxVal < CurVal ) {
		*MaxVal = CurVal;
	}

	return 1;
}


/*******************************************************************************

*******************************************************************************/
int dSetAvgVal( float *AvgVal, float NewVal, int dCount )
{
	float	fLastSum;
	float	fAverage;

	fLastSum = (*AvgVal)*dCount;

	fAverage = ( fLastSum + NewVal )/(dCount+1);

	*(AvgVal) = fAverage;

	return 1;
}

/*******************************************************************************
 GET MAXIMUM FDS, SET gdNumfds.
*******************************************************************************/
int dGetMaxFds()
{
    int         i;
    int         dMaxFds = 0;

    for( i=0; i<gdClient; i++) {
        log_print(LOGN_DEBUG, "[dGetMaxFds] stConTbl IDX[%d], SFD[%d]", i, stConTbl[i].dSfd );
        if( dMaxFds < stConTbl[i].dSfd )
            dMaxFds = stConTbl[i].dSfd;
    }

	/*
	* CHECK MAX FDS INCLUDING SERVER FDS
	* ADDED 2003.09.23
	*/
	if( gdSvrSfd > dMaxFds )
		dMaxFds = gdSvrSfd;

    log_print(LOGN_DEBUG, "[dGetMaxFds] MAX SFD[%d]", dMaxFds );

    return dMaxFds;
}

void SetFIDBValue( UCHAR *ucFIDB, UCHAR ucNEW )
{
    unsigned char ucTmp;

    ucTmp = *ucFIDB;

    if( ucTmp >= MASK_VALUE ) {
        ucTmp = ucNEW;
        ucTmp |= 0x80;
    }
    else {
        ucTmp = ucNEW;
    }

    *ucFIDB = ucTmp;
}

int MMC_Handle_Proc( mml_msg *mmsg )
{
    dbm_msg_t   smsg;

    log_print(LOGN_CRI, "[MMCD]->[CHSMD] [msg_id] = %d [cmd_id] = %d [LEN] = %d [Dummy] = %d",
        mmsg->msg_id, mmsg->cmd_id, mmsg->msg_len, mmsg->dummy);

    switch (mmsg->msg_id) {
        case MI_MASK_NTP_ALM:
            mask_ntp_alm( mmsg, &smsg );
            break;

        default:

            break;
    }

    //SendMess(mmsg, &smsg);

    return 1;
}


/*******************************************************************************

*******************************************************************************/
int	dCheckCHSMDStatus()
{
	int			fd;
	time_t		tNow;
	char		szProcName[32];

	if( fidb->stNTAM.mpswinfo[0].pid == 0 ) {
		log_print( LOGN_INFO, "CHSMD DEAD STATUS" );

		return 1;
	}

	sprintf( szProcName, "/proc/%lld", fidb->stNTAM.mpswinfo[0].pid );

	fd = open( szProcName, O_RDONLY );
	if( fd < 0 ) {
		log_print( LOGN_CRI, "CHSMD DEAD" );

		time( &tNow );

		fidb->stNTAM.mpsw[0] = CRITICAL;
		fidb->stNTAM.mpswinfo[0].pid = 0;
		fidb->stNTAM.mpswinfo[0].when = tNow;

		Send_CondMess( SYSTYPE_TAM, 0, LOCTYPE_PROCESS, INVTYPE_USERPROC, 0, CRITICAL, NORMAL, 0 );
	}
	else {
		log_print( LOGN_INFO, "CHSMD ALIVE" );

		close( fd );
	}

	return 1;
}

void GetNTAFSessLoad(st_NTAF *stNTAF, int dNTAFID)
{
	time_t		now;
	struct tm	*t;
	int			i;

	now = time(&now);
	t = (struct tm *) localtime(&now);
	if (gstIpafRsrcInfo[dNTAFID].last != t->tm_hour)
	//if (dNTAFID == 0)		FOR TEST
	{
		// make status message, send it to console
		if (gstIpafRsrcInfo[dNTAFID].count != 0)
			dSndIpafRsrcSts(dNTAFID);

		// reset
		memset(&gstIpafRsrcInfo[dNTAFID],0,sizeof(st_IpafRsrcInfo));
		for (i=0; i<MAX_NTAF_RSRC_NUM; i++)
			gstIpafRsrcInfo[dNTAFID].min[i] = 0xffffffff;
	}

	for (i=0; i<MAX_NTAF_RSRC_NUM; i++)
	{
		if (stNTAF->rsrcload[i] < gstIpafRsrcInfo[dNTAFID].min[i])
			gstIpafRsrcInfo[dNTAFID].min[i] = stNTAF->rsrcload[i];

		if (stNTAF->rsrcload[i] > gstIpafRsrcInfo[dNTAFID].max[i])
			gstIpafRsrcInfo[dNTAFID].max[i] = stNTAF->rsrcload[i];

		gstIpafRsrcInfo[dNTAFID].total[i] += stNTAF->rsrcload[i];
	}
	gstIpafRsrcInfo[dNTAFID].count++;
	gstIpafRsrcInfo[dNTAFID].last = t->tm_hour;
}

void dSndIpafRsrcSts(int dNTAFID)
{
	unsigned char   *pNODE;
    pst_MsgQ        pstMsgQ;
    pst_MsgQSub     pstMsgQSub;
    int             dRet, size;
    char            szResultBuf[3096], temp[1024];
    time_t          stTime;

	if( (dRet = dGetNode(&pNODE, &pstMsgQ)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN dGetNode(MOND), errno=%d:%s",
				LT,errno,strerror(errno));
		exit(-1);
	}

    pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;

    pstMsgQSub->usType = DEF_SYS;
    pstMsgQSub->usSvcID = SID_NTAF_MNG;
    pstMsgQSub->usMsgID = MID_CHG_NTAF;

/*
ILSAN-TB 2004-05-20 15:43:29 THU
S9001 NTAF MMDB SESSION LOAD HOURLY REPORT
   SYSTYPE=NTAF SYSNO=00
   -----------------------------------------------------
         TCP   CALL  CDR   TRCAL ME    KUN   ADS   MARS
   -----------------------------------------------------
   MIN   00000 00000 00000 00000 00000 00000 00000 00000
   MAX
   AVG
   -----------------------------------------------------
*/
    time(&stTime);
    sprintf(szResultBuf,"%s\nS9001 NTAF MMDB SESSION LOAD HOURLY REPORT\n", crTime(stTime));

    sprintf(temp,"   SYSTYPE=NTAF SYSNO=%02d\n",dNTAFID+1);
    strcat(szResultBuf, temp);

    sprintf(temp,"   -----------------------------------------------------\n         TCP   CALL  CDR   TRCAL ME    KUN   ADS   MARS\n   -----------------------------------------------------\n");
    strcat(szResultBuf, temp);

/*	0:MMDB_SESS, 1:MMDB_OBJ, 2:MMDB_CDR, 5:MMDB_OBJ2, 6:MMDB_ME, 7:MMDB_KUN, 8:MMDB_ADS, 9:MMDB_MARS */
    sprintf(temp,"   MIN   %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d\n",
		gstIpafRsrcInfo[dNTAFID].min[0],gstIpafRsrcInfo[dNTAFID].min[1],gstIpafRsrcInfo[dNTAFID].min[2],
		gstIpafRsrcInfo[dNTAFID].min[5],gstIpafRsrcInfo[dNTAFID].min[6],gstIpafRsrcInfo[dNTAFID].min[7],
		gstIpafRsrcInfo[dNTAFID].min[8],gstIpafRsrcInfo[dNTAFID].min[9]);
    strcat(szResultBuf, temp);

    sprintf(temp,"   MAX   %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d\n",
		gstIpafRsrcInfo[dNTAFID].max[0],gstIpafRsrcInfo[dNTAFID].max[1],gstIpafRsrcInfo[dNTAFID].max[2],
		gstIpafRsrcInfo[dNTAFID].max[5],gstIpafRsrcInfo[dNTAFID].max[6],gstIpafRsrcInfo[dNTAFID].max[7],
		gstIpafRsrcInfo[dNTAFID].max[8],gstIpafRsrcInfo[dNTAFID].max[9]);
    strcat(szResultBuf, temp);

    sprintf(temp,"   AVG   %-5ld %-5ld %-5ld %-5ld %-5ld %-5ld %-5ld %-5ld\n",
		gstIpafRsrcInfo[dNTAFID].total[0]/gstIpafRsrcInfo[dNTAFID].count,
		gstIpafRsrcInfo[dNTAFID].total[1]/gstIpafRsrcInfo[dNTAFID].count,
		gstIpafRsrcInfo[dNTAFID].total[2]/gstIpafRsrcInfo[dNTAFID].count,
		gstIpafRsrcInfo[dNTAFID].total[5]/gstIpafRsrcInfo[dNTAFID].count,
		gstIpafRsrcInfo[dNTAFID].total[6]/gstIpafRsrcInfo[dNTAFID].count,
		gstIpafRsrcInfo[dNTAFID].total[7]/gstIpafRsrcInfo[dNTAFID].count,
		gstIpafRsrcInfo[dNTAFID].total[8]/gstIpafRsrcInfo[dNTAFID].count,
		gstIpafRsrcInfo[dNTAFID].total[9]/gstIpafRsrcInfo[dNTAFID].count);
    strcat(szResultBuf, temp);

    sprintf(temp,"   -----------------------------------------------------\n");
    strcat(szResultBuf, temp);

    sprintf(temp, "COMPLETED\n");
    strcat(szResultBuf, temp);

    memcpy(pstMsgQ->szBody, szResultBuf, strlen(szResultBuf));
    pstMsgQ->szBody[strlen(szResultBuf)] = 0x00;

    pstMsgQ->usRetCode = 0;
    pstMsgQ->usBodyLen = strlen(szResultBuf);

	size = DEF_MSGHEAD_LEN + pstMsgQ->usBodyLen;

	if( (dRet = dMsgsnd(SEQ_PROC_COND, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(COND)", LT);
		exit(-1);
	}

    log_print(LOGN_DEBUG,"[SUCCESS] MSGSND FSTAT TYPE=%d,SID=%d,MID=%d,BODY=%d,TAFID=%d,TAMID=%d",
        pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID,
        pstMsgQ->usBodyLen, pstMsgQ->ucNTAFID, pstMsgQ->ucNTAMID );
}

char *crTime(time_t when)
{
	memset (crmtime_str, 0, 255);
	strftime(crmtime_str, 80, "%Y-%m-%d %T %a", localtime((time_t *)&when));
	crmtime_str[21] = toupper(crmtime_str[21]);
	crmtime_str[22] = toupper(crmtime_str[22]);
	return crmtime_str;
}
