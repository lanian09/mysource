/*******************************************************************************
			DQMS Project

	Author   : Park Si Woo
	Section  : ALMD
	SCCS ID  : @(#)mond_sock.c	1.6
	Date     : 09/24/03
	Revision History :
        '01.  7. 21     Initial
        '03.  1. 15     Initial

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/
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

//#include <tam_define.h>
//#include <tam_shm.h>
//#include <tam_sys.h>

#include "mmcdef.h"
#include "msgdef.h"
#include "loglib.h"
#include "mond_sock.h"


int gdPortNo	= S_PORT_MONITOR;

extern int gdSvrSfd;
extern int errno;

extern fd_set gstReadfds;
extern fd_set gstWritefds;
extern int gdNumfds;
extern int gdNumWfds;
extern unsigned char gucNTAMID;
extern unsigned char gucNTAFID;

extern st_ConTbl  stConTbl[];

extern int vdLogLevel;      // LOG WRITE LEVEL

int log_print(int dIndex, char *fmt, ...);

#ifdef DEBUG
void printDirectorStatusValues(st_DIRECT_MNG *pstDIRECT);
#endif


/*******************************************************************************

*******************************************************************************/
int dTcpSvrModeInit()
{
	int				dRet;
	int     		dReuseaddr=1;

    struct linger		stSvr_ld;
    struct sockaddr_in 	stSvr_addr;

    stSvr_ld.l_onoff = 1;
    stSvr_ld.l_linger = 0;

    gdSvrSfd = socket( AF_INET, SOCK_STREAM, 0 );
    if( gdSvrSfd < 0 ) {
        log_print(LOGN_CRI, "SERVER SOCKET ERROR [%d] %s\n", errno, strerror(errno) );
        exit(0);
    }

    log_print( LOGN_CRI,"[SOCKET] [CONNECT] MAIN SERVER SOCKFD[%d]", gdSvrSfd );

    dRet = setsockopt( gdSvrSfd, SOL_SOCKET, SO_LINGER, (char*)&stSvr_ld, sizeof(struct linger));
    if( dRet < 0 ) {
        log_print(LOGN_CRI, "SERVER SETSOCKOPT ERROR : LINGER  [%d] %s", errno, strerror(errno));
        exit(0);
    }

    dRet = setsockopt( gdSvrSfd, SOL_SOCKET, SO_REUSEADDR, (char*)&dReuseaddr, sizeof(int) );
    if( dRet < 0 ) {
        log_print(LOGN_CRI, "SERVER SETSOCKOPT ERROR : REUSEADDR [%d] %s",  errno, strerror(errno));
        exit(0);
    }

    dRet = fcntl( gdSvrSfd, F_SETFL, O_NDELAY );
    if( dRet < 0 ) {
        log_print(LOGN_CRI, "FCNTL ERROR : NONBLOCK [%d] %s",  errno, strerror(errno));
        exit(0);
    }

    bzero( &stSvr_addr, sizeof(struct sockaddr_in) );

    stSvr_addr.sin_family = AF_INET;
    stSvr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    stSvr_addr.sin_port = htons(gdPortNo);


    dRet = bind( gdSvrSfd, (struct sockaddr *)&stSvr_addr, sizeof(struct sockaddr_in) );
    if( dRet < 0 ) {
        log_print(LOGN_CRI, "BIND ERROR : BIND [%d] %s", errno, strerror(errno));
        exit(0);
    }

    dRet = listen( gdSvrSfd, LISTEN_PORT_NUM2 );
    if( dRet < 0 ) {
        log_print(LOGN_CRI, "LISTEN ERROR : LISTEN [%d] %s", errno, strerror(errno));
        exit(0);
    }

    log_print( LOGN_CRI, "PORT [%d] SERVER MODE OK !!!", gdPortNo );

    FD_ZERO( &gstWritefds );
    FD_ZERO( &gstReadfds );
    FD_SET( gdSvrSfd, &gstReadfds );

    gdNumfds = gdSvrSfd;

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dRecvMessage( int dRsfd, char* szTcpRmsg, int* dRecvLen )
{
    int     dRead_size;

    dRead_size = read( dRsfd, (void*)&szTcpRmsg[0], 2048 );

    if( dRead_size <= 0 ) {
		if( errno == EWOULDBLOCK )
			return 0;

        log_print(LOGN_CRI, "SubFunc read error SFD[%d] : [%d] %s", dRsfd, errno, strerror(errno) );
        return -1;
    }

    *dRecvLen = dRead_size;

    return 0;
}


/*******************************************************************************

*******************************************************************************/
int dSendMessage(int dSsfd, int dMsgLen, char *szSmsg, int stConIdx)
{
	int		dRet;

	/*
	* CASE IN SOCKET BLOCK : CANNOT SEND
	*/
	if(stConTbl[stConIdx].cSockBlock == 0x01)
	{
		log_print(LOGN_DEBUG, "CANNOT SEND MESSAGE: BLOCK STATUS IDX[%d]", stConIdx);
		return 1;
	}

	dRet = write(dSsfd, (char*)szSmsg, dMsgLen);
	if(dRet < 0)
	{
		if(errno == EAGAIN)
		{
			log_print(LOGN_CRI, "SUBFUNC WRITE SFD[%d] WRITE EAGAIN : [%d] %s", dSsfd, errno, strerror(errno));

			FD_SET(dSsfd, &gstWritefds);

			gdNumWfds++;

			stConTbl[stConIdx].cSockBlock = 0x01;

			if(dMsgLen == (MAX_MNGPKT_BODY_SIZE + MNG_PKT_HEAD_SIZE))
				stConTbl[stConIdx].dBufLen = 0;

			log_print(LOGN_CRI,"SEND MESSAGE BLOCK IDX[%d] SFD[%d]", stConIdx, dSsfd);

			return 2;
		}

		log_print(LOGN_CRI," SUBFUNC WRITE SFD[%d] WRITE ERROR : [%d] %s", dSsfd, errno, strerror(errno));
		return -1;
	}
	else
	{
		/*
		*	CASE : PARTIAL PACKET SEND
		*/
		if(dRet != dMsgLen)
		{
			log_print(LOGN_CRI, "WRITE MESSAGE SIZE ERROR [%d] [%d] BUF FULL", dRet, dMsgLen);

			FD_SET(dSsfd, &gstWritefds);

			gdNumWfds++;

			if( (dMsgLen-dRet) > MAX_ALMDBUF_LEN)
			{
				log_print(LOGN_CRI, "F=%s:%s.%d: szSmsg left length[%d] is bigger than MAX_ALMDBUF_LEN[%d]", __FILE__, __FUNCTION__, __LINE__,
					(dMsgLen-dRet), MAX_ALMDBUF_LEN);

				return -2;
			}

			memcpy(&stConTbl[stConIdx].szSendBuffer[0], &szSmsg[dRet], dMsgLen - dRet);
			stConTbl[stConIdx].dBufLen		= dMsgLen - dRet;
			stConTbl[stConIdx].cSockBlock	= 0x01;

			log_print(LOGN_CRI, "SEND MESSAGE BLOCK IDX[%d] SFD[%d]", stConIdx, dSsfd);

			return 2;
		}
		else
		{
			log_print(LOGN_INFO, "SFD[%d] SEND_SIZE [%d]", dSsfd, dRet);
			stConTbl[stConIdx].dBufLen		= 0;
		}
	}

	return 1;
}



void u_sleep( long ld_utime )
{
    struct timeval St_Tval;

    St_Tval.tv_sec = 0;
    St_Tval.tv_usec = ld_utime;

    select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &St_Tval );
}

int dMakeNTAMMngPkt( st_MngPkt *stPkt, pst_NTAM pstNTAM, int dHostNo)
{
	/* NTAM STATUS */
	stPkt->head.llMagicNumber = MAGIC_NUMBER;
	stPkt->head.usTotLen = MNG_PKT_HEAD_SIZE + sizeof(st_NTAM);
	stPkt->head.usBodyLen = sizeof(st_NTAM);
	stPkt->head.ucNTAMID = dHostNo;
	stPkt->head.ucNTAFID= 0;
	stPkt->head.ucSysNo	= dHostNo;
	stPkt->head.ucSvcID = SID_STATUS;
	stPkt->head.ucMsgID = MID_ALARM;

	memcpy( stPkt->data, pstNTAM, sizeof(st_NTAM) );

	return 1;
}

int dMakeDirectMngPkt(st_MngPkt *stPkt, st_DIRECT_MNG *pstDIRECT)
{
	/* DIRECTOR STATUS */
	stPkt->head.llMagicNumber	= MAGIC_NUMBER;
	stPkt->head.usTotLen		= MNG_PKT_HEAD_SIZE + sizeof(st_DIRECT_MNG);
	stPkt->head.usBodyLen		= sizeof(st_DIRECT_MNG);
	stPkt->head.ucNTAMID		= 0;
	stPkt->head.ucNTAFID		= 0;
	stPkt->head.ucSysNo			= 0;
	stPkt->head.ucSvcID			= SID_STATUS_DIRECT;
	stPkt->head.ucMsgID			= MID_ALARM;

#ifdef DEBUG
	printDirectorStatusValues(pstDIRECT);
#endif
	memcpy(stPkt->data, pstDIRECT, sizeof(st_DIRECT_MNG));

	return 0;
}

int dMakeNTAFMngPkt( st_MngPkt *stPkt, pst_NTAF pstNTAF, unsigned char ucIpafID )
{
	/* NTAF STATUS */
    stPkt->head.llMagicNumber = MAGIC_NUMBER;
    stPkt->head.usTotLen = MNG_PKT_HEAD_SIZE + sizeof(st_NTAF);
    stPkt->head.usBodyLen = sizeof(st_NTAF);
	stPkt->head.ucNTAMID = 1;
	stPkt->head.ucNTAFID= 1;
    stPkt->head.ucSysNo = ucIpafID+1;
    stPkt->head.ucSvcID = SID_STATUS;
    stPkt->head.ucMsgID = MID_ALARM;

    memcpy( stPkt->data, pstNTAF, sizeof(st_NTAF) );

    return 1;
}

int dMakeSTATMngPkt( st_MngPkt *stPkt, int dBodyLen )
{
	/* STAT STATUS */
	stPkt->head.llMagicNumber = MAGIC_NUMBER;
    stPkt->head.usTotLen = dBodyLen;
    stPkt->head.usBodyLen = dBodyLen - MNG_PKT_HEAD_SIZE;
	stPkt->head.ucNTAMID = 1;
	stPkt->head.ucNTAFID= 1;
    stPkt->head.ucSysNo = 0;
    stPkt->head.ucSvcID = SID_STATUS;
    stPkt->head.ucMsgID = MID_STAT;

	return 1;
}

#ifdef DEBUG
void printDirectorStatusValues(st_DIRECT_MNG *pstDIRECT)
{
	int		i;

	log_print(LOGN_DEBUG, "tUpTime[%lu] cDirectorMask[0x%02X:0x%02X:0x%02X:0x%02X:0x%02X]",
		pstDIRECT->tUpTime, pstDIRECT->cDirectorMask[0], pstDIRECT->cDirectorMask[1],
		pstDIRECT->cDirectorMask[2], pstDIRECT->cDirectorMask[3], pstDIRECT->cDirectorMask[4]);

	log_print(LOGN_DEBUG, "cReserved[0x%02X:0x%02X:0x%02X:0x%02X]",
		pstDIRECT->cReserved[0], pstDIRECT->cReserved[1], pstDIRECT->cReserved[2], pstDIRECT->cReserved[3]);

	for(i = 0; i < MAX_DIRECT; i++)
	{
		log_print(LOGN_DEBUG, "IDX[%d] tEachUpTime[%lu]", i, pstDIRECT->stDIRECT[i].tEachUpTime);

		log_print(LOGN_DEBUG, "cReserved[0x%02X:0x%02X:0x%02X:0x%02X]",
			pstDIRECT->stDIRECT[i].cReserved[0], pstDIRECT->stDIRECT[i].cReserved[1],
			pstDIRECT->stDIRECT[i].cReserved[2], pstDIRECT->stDIRECT[i].cReserved[3]);

		log_print(LOGN_DEBUG, "Monitor Port[0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X]",
			pstDIRECT->stDIRECT[i].cMonitorPort[0], pstDIRECT->stDIRECT[i].cMonitorPort[1], pstDIRECT->stDIRECT[i].cMonitorPort[2],
			pstDIRECT->stDIRECT[i].cMonitorPort[3], pstDIRECT->stDIRECT[i].cMonitorPort[4], pstDIRECT->stDIRECT[i].cMonitorPort[5],
			pstDIRECT->stDIRECT[i].cMonitorPort[6], pstDIRECT->stDIRECT[i].cMonitorPort[7], pstDIRECT->stDIRECT[i].cMonitorPort[8],
			pstDIRECT->stDIRECT[i].cMonitorPort[9], pstDIRECT->stDIRECT[i].cMonitorPort[10], pstDIRECT->stDIRECT[i].cMonitorPort[11]);

		log_print(LOGN_DEBUG, "Mirror Port_1[0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X]",
			pstDIRECT->stDIRECT[i].cMirrorPort[0], pstDIRECT->stDIRECT[i].cMirrorPort[1], pstDIRECT->stDIRECT[i].cMirrorPort[2],
			pstDIRECT->stDIRECT[i].cMirrorPort[3], pstDIRECT->stDIRECT[i].cMirrorPort[4], pstDIRECT->stDIRECT[i].cMirrorPort[5],
			pstDIRECT->stDIRECT[i].cMirrorPort[6], pstDIRECT->stDIRECT[i].cMirrorPort[7], pstDIRECT->stDIRECT[i].cMirrorPort[8],
			pstDIRECT->stDIRECT[i].cMirrorPort[9], pstDIRECT->stDIRECT[i].cMirrorPort[10], pstDIRECT->stDIRECT[i].cMirrorPort[11]);

		log_print(LOGN_DEBUG, "Mirror Port_2[0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X]",
			pstDIRECT->stDIRECT[i].cMirrorPort[12], pstDIRECT->stDIRECT[i].cMirrorPort[13], pstDIRECT->stDIRECT[i].cMirrorPort[14],
			pstDIRECT->stDIRECT[i].cMirrorPort[15], pstDIRECT->stDIRECT[i].cMirrorPort[16], pstDIRECT->stDIRECT[i].cMirrorPort[17],
			pstDIRECT->stDIRECT[i].cMirrorPort[18], pstDIRECT->stDIRECT[i].cMirrorPort[19], pstDIRECT->stDIRECT[i].cMirrorPort[20],
			pstDIRECT->stDIRECT[i].cMirrorPort[21], pstDIRECT->stDIRECT[i].cMirrorPort[22], pstDIRECT->stDIRECT[i].cMirrorPort[23]);
	}
}
#endif
