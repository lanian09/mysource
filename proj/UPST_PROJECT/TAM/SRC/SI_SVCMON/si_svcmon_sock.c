/*******************************************************************************
			DQMS Project

	Author   : Hwang Woo-Hyoung
	Section  : SI_SVCMON
	SCCS ID  : @(#)si_svcmon_sock.c	1.1
	Date     : 01/15/03
	Revision History :
		'03.    01. 15. initial
		'08.	01. 14. Review by LSH and Add IUPS NTAM

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <inttypes.h>

#include "memg.h"
#include "sockio.h"
#include "filedb.h"
#include "loglib.h"
#include "si_svcmon_func.h"
#include "si_svcmon_sock.h"

/***** B.1 *  Definition of New Constants ***************/
/***** B.2 *  Definition of New Type  *******************/
/***** C.1 *  Declaration of Variables  *****************/
extern st_SubSysInfoList	gstSubList;
extern st_Send_Info			gstSendInfo;
extern stMEMGINFO			*pMEMGINFO;
extern st_subsys_mng        stSubSys[MAX_CH_COUNT];
/***** E.1 *  DEFINITION OF FUNCTIONS *******************/
/***** E.2 *  DEFINITION OF FUNCTIONS *******************/
extern int dCheck_Channel(int dSysNo, int dFlag, unsigned int uiIP);

int dRecvPacket(st_ClientInfo *stSock, int dIndex, st_FDInfo *stFD)
{
	int		n, size, dRet, sp;

	size = stSock[dIndex].dBufSize;
	if( (n = read(stSock[dIndex].dSfd, &stSock[dIndex].szBuf[size], (DEF_MAX_BUFLEN-size))) <= 0)
	{
		if(n == 0)
			return -1;
		else if(errno != EAGAIN)
			return -1;
		else
			return 0;
	}
	else if(size + n < RNES_PKT_SIZE)
	{
		stSock[dIndex].dBufSize = size + n;
		stSock[dIndex].szBuf[stSock[dIndex].dBufSize] = 0;
		return 0;
	}
	else
	{
		size	+= n;
		sp		= 0;
		do
		{
			time(&stSock[dIndex].tLastTime);
			log_print(LOGN_INFO,"dRecvPacket : TAFNo [%d] dSfd[%d], uiIP[%u] tLastTime[%ld]",
				stSock[dIndex].dSysNo, stSock[dIndex].dSfd, stSock[dIndex].uiIP, stSock[dIndex].tLastTime);

			if ( (dRet = dHandleSocketMsg(stSock, dIndex, stFD, &stSock[dIndex].szBuf[sp])) < 0)
				return -1;
			stSock[dIndex].dLastFlag = 1;
			sp += RNES_PKT_SIZE;
		} while (size >= sp + RNES_PKT_SIZE); /* end of do while */

		/* Buffer set */
		if(size > sp)
		{
			stSock[dIndex].dBufSize = size - sp;
			memcpy(&stSock[dIndex].szBuf[0], &stSock[dIndex].szBuf[sp], size - sp);
		}
		else
			stSock[dIndex].dBufSize = 0;
	}

	return 0;
}


int Check_ClientEvent(st_ClientInfo *stSock, st_FDInfo *stFD)
{
	char			buf[BUFSIZ];
	st_sidb_node	*pDATA;
	int				dRet, dSfd, i, dIdx;
	fd_set			fdRead, fdWrite;
	struct timeval	timeout;
	st_SI_DB		*pSIDB;

	timeout.tv_sec	= 0;
	timeout.tv_usec	= 0;

	memcpy( (char*)&fdRead, (char*)&stFD->Rfds, sizeof(fd_set));
	memcpy( (char*)&fdWrite, (char*)&stFD->Wfds, sizeof(fd_set));

	if( (dRet = select(stFD->dMaxSfd, (fd_set*)&fdRead, (fd_set*)&fdWrite, (fd_set*)0, (struct timeval*)&timeout)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN select() [errno:%d-%s]", LT, errno, strerror(errno));
		return -1;
	}

	if(FD_ISSET(stFD->dSrvSfd, (fd_set*)&fdRead))
	{
		if( (dSfd = dAcceptSockFd(stSock, stFD, &dIdx)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dAcceptSockFd() dRet[%d]", LT, dRet);
			return -2;
		} /* end of if */

		for(i = 0; i < MAX_RECORD; i++)
			log_print(LOGN_INFO, "SOCKLIST %d IP=%u FD=%d SYSNO=%d FLAG=%d", i, stSock[i].uiIP, stSock[i].dSfd, stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag);

		if( (dRet = dGetSubSysInfo(stSock[dIdx].uiIP)) < 0)
		{
			log_print(LOGN_CRI, "NOT SUPPORT SUBSYS FAIL : IDX=%d dSfd=%d uiIP=%u dRet=%d", dIdx, stSock[dIdx].dSfd, stSock[dIdx].uiIP, dRet);
			if(dDisConnSock(stSock, dIdx, stFD) < 0)
				log_print(LOGN_CRI, "Check_ClientEvent : FAILED IN dDisConnSock");
			return 0;
		}

		stSock[dIdx].dSysNo		= dRet;
		stSock[dIdx].dLastFlag	= 1;
		dCheck_Channel(stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag, stSock[dIdx].uiIP);
		log_print(LOGN_WARN, "ACCEPT TAFNO=%d IDX=%d MAXSFD=%d SRVSFD=%d SFD=%d IP=%u TIME=%ld", dRet, dIdx, stFD->dMaxSfd, stFD->dSrvSfd, stSock[dIdx].dSfd, stSock[dIdx].uiIP, stSock[dIdx].tLastTime);

		/*
		 *  send backup
		 */
		pDATA = (st_sidb_node *)cmem_ptr(pMEMGINFO, gstSendInfo.offset_Data);

		if(pDATA != NULL)
		{
			pSIDB = &pDATA->stSIDB;
			if (dCheck_File(pSIDB) >= 0) {
				memset(buf, 0x00, RNES_PKT_SIZE);
				buf[0] = RNES_NE_FILENAME;
				memcpy(&buf[1], pSIDB->name, RNES_PKT_SIZE-1);
				pSIDB->send_time = time(NULL);
				log_print(LOGN_DEBUG, "SEND OLD DATA TYPE=%c FILENAME=%.*s", buf[0], RNES_PKT_SIZE-1, &buf[1]);
				if((dRet = dSendPacket(stSock, dIdx, stFD, buf, RNES_PKT_SIZE)) < 0) {
					log_print(LOGN_CRI, LH"dSendPacket dRet=%d", LT, dRet);
				}
			}
		}

	}

	for(i = 0; i < MAX_RECORD; i++)
	{
		if( (stSock[i].dSfd > 0) && (FD_ISSET(stSock[i].dSfd, (fd_set*)&fdRead)))
		{
			if( (dRet = dRecvPacket(stSock, i, stFD)) < 0)
			{
				stSock[i].dLastFlag = 0;
				dCheck_Channel(stSock[i].dSysNo, stSock[i].dLastFlag, stSock[i].uiIP);
				if(dDisConnSock(stSock, i, stFD) < 0)
					log_print( LOGN_CRI, "Check_ClientEvent : FAILED IN dDisconnSock RECV");
			}
		}

		if( (stSock[i].dSfd > 0) && (FD_ISSET(stSock[i].dSfd, (fd_set *)&fdWrite)))
			dRet = dSendBlockPacket(stSock, i, stFD);

	/*
		if( (stSock[i].dSfd > 0) && ((time(NULL)-stSock[i].tLastTime) > MAX_CHANNEL_TIMEOUT))
		{
			stSock[i].dLastFlag = 0;
			if(dDisConnSock(stSock, i, stFD) < 0)
				log_print( LOGN_CRI, "Check_ClientEvent : FAILED IN dDisConnSock TIME");
		}
	*/
	}

	return 1;
} /* end of Check_ClientEvent */

int dSendCheck(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD)
{
	int				dRet;
	char			buf[RNES_PKT_SIZE];

	memset(buf, 0x00, RNES_PKT_SIZE);

	buf[0] = RNES_NE_HEARTBEAT;

#ifdef DNMS_CONN_DISABLE /* added by uamyd.20100927 : moved connection module to total-mon system */
	dRet = 0;
#else
	dRet = write(stSock[dIdx].dSfd, buf, RNES_PKT_SIZE);
#endif
	if(dRet < 0) {
		if(errno != EAGAIN) {
			log_print(LOGN_CRI, "SENDCHK ERR=%d IDX=%d IP=%u FD=%d SYSNO=%d FLAG=%d TIME=%ld",
				errno, dIdx, stSock[dIdx].uiIP, stSock[dIdx].dSfd, stSock[dIdx].dSysNo,
				stSock[dIdx].dLastFlag, stSock[dIdx].tLastTime);
			stSock[dIdx].dLastFlag = 0;
			dCheck_Channel(stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag, stSock[dIdx].uiIP);
			if(dDisConnSock(stSock, dIdx, stFD) < 0)
				log_print( LOGN_CRI, "dSendCheck : FAILED IN dDisConnSock TIME");
			log_print(LOGN_CRI, "dSendCheck : FAILED IN write SFD[%d]", stSock[dIdx].dSfd);
		} /* end of if */
	} else {
		log_print(LOGN_DEBUG, "SENDCHK %d IP=%u FD=%d SYSNO=%d FLAG=%d TIME=%ld",
			dIdx, stSock[dIdx].uiIP, stSock[dIdx].dSfd, stSock[dIdx].dSysNo,
			stSock[dIdx].dLastFlag, stSock[dIdx].tLastTime);
		stSock[dIdx].tLastTime = time(NULL);
	}

	return 1;
} /* end of dSendCheck */

int dSendPacket(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD, char *str, int slen)
{
#ifdef DNMS_CONN_DISABLE /* added by uamyd.20100927 : moved connection module to total-mon system */
	int k;
#else
    int i, k;
#endif

    if (FD_ISSET(stSock[dIdx].dSfd, (fd_set *)&stFD->Wfds)) {
        k = stSock[dIdx].dFront + slen;

        if ((stSock[dIdx].dFront < stSock[dIdx].dRear && k >= stSock[dIdx].dRear) ||
			(stSock[dIdx].dFront > stSock[dIdx].dRear && k >= stSock[dIdx].dRear + DEF_MAX_BUFLEN)) {
            log_print(LOGN_CRI, "Packet discarded : slen=%d, socket=%d", slen, stSock[dIdx].dSfd);
            return 0;
        }

        if (k >= DEF_MAX_BUFLEN) {
            k -= DEF_MAX_BUFLEN;
            memcpy(&stSock[dIdx].szWBuf[stSock[dIdx].dFront], (void *)str, slen - k);
            memcpy(&stSock[dIdx].szWBuf[0], (void *)&str[slen - k], k);
        } else
            memcpy(&stSock[dIdx].szWBuf[stSock[dIdx].dFront], (void *)str, slen);

        stSock[dIdx].dFront = k;
        return 0;
    } else if (stSock[dIdx].dFront != stSock[dIdx].dRear) {
        stSock[dIdx].dFront = stSock[dIdx].dRear = 0;
        log_print(LOGN_CRI,"Undefined error occurres in writeSocket");
    }

#ifdef DNMS_CONN_DISABLE /* added by uamyd.20100927 : moved connection module to total-mon system */
	if( 1 ){
		stSock[dIdx].dRear = slen;
		stSock[dIdx].dFront= slen;
		FD_SET(stSock[dIdx].dSfd, (fd_set *)&stFD->Wfds);
		return 0;
	}
#else
    memcpy(&stSock[dIdx].szWBuf[0], (void *)str, slen);
    if ((i = write(stSock[dIdx].dSfd, &stSock[dIdx].szWBuf[0], slen)) != slen) {
        if (i < 0) {
            if (errno != EAGAIN) {
				stSock[dIdx].dLastFlag = 0;
				dCheck_Channel(stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag, stSock[dIdx].uiIP);
				if(dDisConnSock(stSock, dIdx, stFD) < 0)
					log_print( LOGN_CRI, "dSendPacket : FAILED IN dDisConnSock");

                log_print(LOGN_CRI, "Failure in writing socket 1st errno = %d", errno);
                return -1;
            } else {
                log_print(LOGN_CRI,"SOCK BLOCK [EAGAIN][%s]", strerror(errno));
                i = 0;
            }
        }

        stSock[dIdx].dRear = i;
        stSock[dIdx].dFront = slen;
        FD_SET(stSock[dIdx].dSfd, (fd_set *)&stFD->Wfds);
        return 0;
    }
#endif

    stSock[dIdx].dFront = 0;
    stSock[dIdx].dRear = 0;

    return 1;
}

int dSendBlockPacket(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD)
{
    int k, t;

    if (stSock[dIdx].dFront == stSock[dIdx].dRear) {
        FD_CLR(stSock[dIdx].dSfd, (fd_set *)&stFD->Wfds);
        stSock[dIdx].dFront = stSock[dIdx].dRear = 0;
        log_print(LOGN_CRI,"Invalid Length in socket write buffer");
    } else {
        if (stSock[dIdx].dFront > stSock[dIdx].dRear)
            k = stSock[dIdx].dFront - stSock[dIdx].dRear;
        else
            k = DEF_MAX_BUFLEN - stSock[dIdx].dRear;

#ifdef DNMS_CONN_DISABLE /* added by uamyd.20100927 : moved connection module to total-mon system */
		if( 1 ){
			stSock[dIdx].dRear += t;
			log_print(LOGN_DEBUG,"SOCK BLOCK SEND SIZE[%d] ==> DO NOT HAVE SENT, IT'S FUNCTION DISABLED", t);
			if (stSock[dIdx].dFront == stSock[dIdx].dRear) {
                stSock[dIdx].dFront = stSock[dIdx].dRear = 0;
                FD_CLR(stSock[dIdx].dSfd, (fd_set *)&stFD->Wfds);
            }
            else if (stSock[dIdx].dRear == DEF_MAX_BUFLEN)
                stSock[dIdx].dRear = 0;
		}
#else
        if ((t = write(stSock[dIdx].dSfd, &stSock[dIdx].szWBuf[stSock[dIdx].dRear], k)) <= 0) {
            if ( errno != EAGAIN) {
				stSock[dIdx].dLastFlag = 0;
				dCheck_Channel(stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag, stSock[dIdx].uiIP);
				if(dDisConnSock(stSock, dIdx, stFD) < 0)
					log_print( LOGN_CRI, "dSendBlockPacket : FAILED IN dDisConnSock");
                log_print(LOGN_CRI, "Failure in writing to socket second time, errno = %d", errno);
            } else if( errno == EAGAIN) {
                log_print(LOGN_CRI,"SOCK BLOCK [EAGAIN][%s]", strerror(errno));
                log_print(LOGN_DEBUG,"SOCK BLOCK SEND SIZE [0]");
            }
        } else if (k == t) {
            stSock[dIdx].dRear += t;
            log_print(LOGN_DEBUG,"SOCK BLOCK SEND SIZE [%d]",t);

            if (stSock[dIdx].dFront == stSock[dIdx].dRear) {
                stSock[dIdx].dFront = stSock[dIdx].dRear = 0;
                FD_CLR(stSock[dIdx].dSfd, (fd_set *)&stFD->Wfds);
            }
			else if (stSock[dIdx].dRear == DEF_MAX_BUFLEN)
                stSock[dIdx].dRear = 0;
        } else {
            stSock[dIdx].dRear += t;
            log_print(LOGN_CRI,"SOCK BLOCK [EAGAIN ???]");
            log_print(LOGN_DEBUG,"SOCK BLOCK SEND SIZE [%d]",t);
        }
#endif
    }

	return 0;
}

int dGetSubSysInfo(UINT uiIP)
{
	int		i;

	for(i = 0; i < gstSubList.dCount; i++)
	{
		log_print(LOGN_INFO, "SUBLIST %d IP=%u:%u SYS=%d", i, gstSubList.stInfo[i].uiIP, uiIP, gstSubList.stInfo[i].dNo);
		if(gstSubList.stInfo[i].uiIP == uiIP)
			return gstSubList.stInfo[i].dNo;
	}

	return -1;
}
