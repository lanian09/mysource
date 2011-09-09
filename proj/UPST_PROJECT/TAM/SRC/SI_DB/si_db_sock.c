/*******************************************************************************
			DQMS Project

	Author   : Hwang Woo-Hyoung
	Section  : SI_SVC
	SCCS ID  : @(#)si_svc_sock.c	1.1
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

// LIB
#include "loglib.h"
#include "nsocklib.h"
#include "memg.h"

// .
#include "si_db_sock.h"

/***** B.1 *  Definition of New Constants ***************/
/***** B.2 *  Definition of New Type  *******************/
/***** C.1 *  Declaration of Variables  *****************/
extern st_SubSysInfoList	gstSubList;
extern st_Send_Info			gstSendInfo;
extern stMEMGINFO			*pMEMGINFO;
/***** E.1 *  DEFINITION OF FUNCTIONS *******************/
/***** E.2 *  DEFINITION OF FUNCTIONS *******************/
extern int dHandleSocketMsg(st_ClientInfo *stNet, int dIdx, st_FDInfo *stFD, char *szBuf);
extern int dCheck_Channel(int dSysNo, int dFlag, unsigned int uiIP);
extern int dCheck_File(st_SI_DB *pSIDB);


int dRecvPacket(st_ClientInfo *stSock, int dIndex, st_FDInfo *stFD)
{
    int				n, size, dRet, sp;
	
    size = stSock[dIndex].dBufSize;
    n = read(stSock[dIndex].dSfd, &stSock[dIndex].szBuf[size], DEF_MAX_BUFLEN - size);
    if(n <= 0) {
        if(n == 0)
            return -1;
        else if(errno != EAGAIN)
            return -1;
        else
            return 0;
    } else if(size + n < RNES_PKT_SIZE) {
        stSock[dIndex].dBufSize = size + n;
        stSock[dIndex].szBuf[stSock[dIndex].dBufSize] = 0;
        return 0;
    } else {
        size += n;
        sp = 0;
        do 
		{
			time(&stSock[dIndex].tLastTime);
			log_print(LOGN_INFO, LH"dRecvPacket : TAFNo [%d] dSfd[%d], uiIP[%u] tLastTime[%ld] ", LT,
					stSock[dIndex].dSysNo, stSock[dIndex].dSfd, stSock[dIndex].uiIP,
					stSock[dIndex].tLastTime);
			dRet = dHandleSocketMsg(stSock, dIndex, stFD, &stSock[dIndex].szBuf[sp]);
			if (dRet < 0)
				return -1;
			stSock[dIndex].dLastFlag = 1;
			sp += RNES_PKT_SIZE;
        } while (size >= sp + RNES_PKT_SIZE); /* end of do while */

        /* Buffer set */
        if (size > sp) {
            stSock[dIndex].dBufSize = size - sp;
            memcpy(&stSock[dIndex].szBuf[0], &stSock[dIndex].szBuf[sp], size - sp);
        } else
            stSock[dIndex].dBufSize = 0;
    } /* end of else */   
	return 0;
} /* end of dRecvPacket */


int Check_ClientEvent(st_ClientInfo *stSock, st_FDInfo *stFD)
{
    int     		dRet, dSfd, i, dIdx;
    fd_set  		fdRead, fdWrite;
    struct timeval  timeout;
	st_sidb_node	*pDATA;
	st_SI_DB		*pSIDB;
	char			buf[BUFSIZ];

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    memcpy((char *)&fdRead, (char *)&stFD->Rfds, sizeof(fd_set));
    memcpy((char *)&fdWrite, (char *)&stFD->Wfds, sizeof(fd_set));
    dRet = select(stFD->dMaxSfd, (fd_set *)&fdRead, (fd_set *)&fdWrite, (fd_set *)0, (struct timeval *)&timeout);
    if(dRet < 0) {
		log_print(LOGN_CRI, LH"Check_ClientEvent : FAILED IN select [%s]", LT, strerror(errno));
        return -1;
    } /* end of if */

    if(FD_ISSET(stFD->dSrvSfd, (fd_set *)&fdRead)) {
        dSfd = dAcceptSockFd(stSock, stFD, &dIdx);
        if(dSfd < 0) {
            log_print(LOGN_CRI, LH"Check_ClientEvent : FAILED IN dAcceptSockFd, [%s]", LT, strerror(errno));
            return -1;
        } /* end of if */

		for(i=0; i < MAX_RECORD; i++)
		{
			log_print(LOGN_INFO, LH"SOCKLIST %d IP=%u FD=%d SYSNO=%d FLAG=%d", LT,
				i, stSock[i].uiIP, stSock[i].dSfd, stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag);
		}

		dRet = dGetSubSysInfo(stSock[dIdx].uiIP);
		if(dRet < 0) {
			log_print(LOGN_CRI, LH"NOT SUPPORT SUBSYS FAIL : IDX=%d dSfd=%d uiIP=%u dRet=%d", LT,
                dIdx, stSock[dIdx].dSfd, stSock[dIdx].uiIP, dRet);
			if(dDisConnSock(stSock, dIdx, stFD) < 0) 
				log_print(LOGN_CRI, LH"Check_ClientEvent : FAILED IN dDisConnSock", LT);
			return 0;
		} 

		stSock[dIdx].dSysNo = dRet;
		stSock[dIdx].dLastFlag = 1;
		dCheck_Channel(stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag, stSock[dIdx].uiIP);
		log_print(LOGN_WARN, LH"ACCEPT TAFNO=%d IDX=%d MAXSFD=%d SRVSFD=%d SFD=%d IP=%u TIME=%ld", LT,
			dRet, dIdx, stFD->dMaxSfd, stFD->dSrvSfd, stSock[dIdx].dSfd, stSock[dIdx].uiIP, 
			stSock[dIdx].tLastTime);

		/*
		 *	send backup
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
				log_print(LOGN_DEBUG, LH"SEND OLD DATA TYPE=%c FILENAME=%.*s", LT, buf[0], RNES_PKT_SIZE-1, &buf[1]);
           		if((dRet = dSendPacket(stSock, dIdx, stFD, buf, RNES_PKT_SIZE)) < 0) {
               		log_print(LOGN_CRI, LH"dSendPacket dRet=%d", LT, dRet);
           		}
			}
        }

    } /* end of if */

    for(i = 0; i < MAX_RECORD; i++)
    {
        if((stSock[i].dSfd > 0) && (FD_ISSET(stSock[i].dSfd, (fd_set *)&fdRead))) {
            dRet = dRecvPacket(stSock, i, stFD);
            if(dRet < 0) {
				stSock[i].dLastFlag = 0;
				dCheck_Channel(stSock[i].dSysNo, stSock[i].dLastFlag, stSock[i].uiIP);
                if(dDisConnSock(stSock, i, stFD) < 0)
					log_print(LOGN_CRI, LH"Check_ClientEvent : FAILED IN dDisconnSock RECV", LT);
			}/* end of if */
        } 

		if((stSock[i].dSfd > 0) && (FD_ISSET(stSock[i].dSfd, (fd_set *)&fdWrite)))
			dRet = dSendBlockPacket(stSock, i, stFD);

/*
		if(stSock[i].dSfd > 0 && (time(NULL) - stSock[i].tLastTime > MAX_CHANNEL_TIMEOUT )) {
			stSock[i].dLastFlag = 0;
			dCheck_Channel(stSock[i].dSysNo, stSock[i].dLastFlag, stSock[i].uiIP);
			if(dDisConnSock(stSock, i, stFD) < 0)
				log_print(LOGN_CRI, "Check_ClientEvent : FAILED IN dDisConnSock TIME");
		}
*/
    } /* end of for */

    return 1;

} /* end of Check_ClientEvent */

int dSendCheck(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD)
{
	int				dRet;
	char			buf[RNES_PKT_SIZE];

	memset(buf, 0x00, RNES_PKT_SIZE);

	buf[0] = RNES_NE_HEARTBEAT;

	dRet = write(stSock[dIdx].dSfd, buf, RNES_PKT_SIZE);
	if(dRet < 0) {
		if(errno != EAGAIN) {
			log_print(LOGN_CRI, LH"SENDCHK ERR=%d IDX=%d IP=%u FD=%d SYSNO=%d FLAG=%d TIME=%ld", LT,
				errno, dIdx, stSock[dIdx].uiIP, stSock[dIdx].dSfd, stSock[dIdx].dSysNo, 
				stSock[dIdx].dLastFlag, stSock[dIdx].tLastTime);
			stSock[dIdx].dLastFlag = 0;
			dCheck_Channel(stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag, stSock[dIdx].uiIP);
			if(dDisConnSock(stSock, dIdx, stFD) < 0)
				log_print(LOGN_CRI, LH"dSendCheck : FAILED IN dDisConnSock TIME", LT);
			log_print(LOGN_CRI, LH"dSendCheck : FAILED IN write SFD[%d]", LT, stSock[dIdx].dSfd);
		} /* end of if */
	} else {
		log_print(LOGN_DEBUG, LH"SENDCHK INDEX=%d IP=%u FD=%d SYSNO=%d FLAG=%d TIME=%ld TYPE=%c", LT,
			dIdx, stSock[dIdx].uiIP, stSock[dIdx].dSfd, stSock[dIdx].dSysNo, 
			stSock[dIdx].dLastFlag, stSock[dIdx].tLastTime, buf[0]);
		stSock[dIdx].tLastTime = time(NULL);
	}

	return 1;
} /* end of dSendCheck */

int dSendPacket(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD, char *str, int slen)
{
    int i, k;

    if (FD_ISSET(stSock[dIdx].dSfd, (fd_set *)&stFD->Wfds)) {
        k = stSock[dIdx].dFront + slen;

        if ((stSock[dIdx].dFront < stSock[dIdx].dRear && k >= stSock[dIdx].dRear) || 
			(stSock[dIdx].dFront > stSock[dIdx].dRear && k >= stSock[dIdx].dRear + DEF_MAX_BUFLEN)) {
            log_print(LOGN_CRI, LH"Packet discarded : slen=%d, socket=%d", LT, slen, stSock[dIdx].dSfd);
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
        log_print(LOGN_CRI, LH"Undefined error occurres in writeSocket", LT);
    }

    memcpy(&stSock[dIdx].szWBuf[0], (void *)str, slen);
    if ((i = write(stSock[dIdx].dSfd, &stSock[dIdx].szWBuf[0], slen)) != slen) {
        if (i < 0) {
            if (errno != EAGAIN) {
				stSock[dIdx].dLastFlag = 0;
				dCheck_Channel(stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag, stSock[dIdx].uiIP);
				if(dDisConnSock(stSock, dIdx, stFD) < 0) 
					log_print(LOGN_CRI, LH"dSendPacket : FAILED IN dDisConnSock", LT);
					
                log_print(LOGN_CRI, LH"Failure in writing socket 1st errno = %d", LT, errno);
                return -1;
            } else {
                log_print(LOGN_CRI, LH"SOCK BLOCK [EAGAIN][%s]", LT, strerror(errno));
                i = 0;
            }
        }

        stSock[dIdx].dRear = i;
        stSock[dIdx].dFront = slen;
        FD_SET(stSock[dIdx].dSfd, (fd_set *)&stFD->Wfds);
        return 0;
    }

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
        log_print(LOGN_CRI, LH"Invalid Length in socket write buffer", LT);
    } else {
        if (stSock[dIdx].dFront > stSock[dIdx].dRear)
            k = stSock[dIdx].dFront - stSock[dIdx].dRear;
        else
            k = DEF_MAX_BUFLEN - stSock[dIdx].dRear;

        if ((t = write(stSock[dIdx].dSfd, &stSock[dIdx].szWBuf[stSock[dIdx].dRear], k)) <= 0) {
            if ( errno != EAGAIN) {
				stSock[dIdx].dLastFlag = 0;
				dCheck_Channel(stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag, stSock[dIdx].uiIP);
				if(dDisConnSock(stSock, dIdx, stFD) < 0) 
					log_print(LOGN_CRI, LH"dSendBlockPacket : FAILED IN dDisConnSock", LT);
                log_print(LOGN_CRI, LH"Failure in writing to socket second time, errno = %d", LT, errno);
            } else if( errno == EAGAIN) {
                log_print(LOGN_CRI, LH"SOCK BLOCK [EAGAIN][%s]", LT, strerror(errno));
                log_print(LOGN_DEBUG, LH"SOCK BLOCK SEND SIZE [0]", LT);
            }
        } else if (k == t) {
            stSock[dIdx].dRear += t;
            log_print(LOGN_DEBUG, LH"SOCK BLOCK SEND SIZE [%d]", LT, t);

            if (stSock[dIdx].dFront == stSock[dIdx].dRear) {
                stSock[dIdx].dFront = stSock[dIdx].dRear = 0;
                FD_CLR(stSock[dIdx].dSfd, (fd_set *)&stFD->Wfds);
            } 
			else if (stSock[dIdx].dRear == DEF_MAX_BUFLEN)
                stSock[dIdx].dRear = 0;
        } else {
            stSock[dIdx].dRear += t;
            log_print(LOGN_CRI, LH"SOCK BLOCK [EAGAIN ???]", LT);
            log_print(LOGN_DEBUG, LH"SOCK BLOCK SEND SIZE [%d]", LT, t);
        }
    }

	return 0;
}

int dGetSubSysInfo(UINT uiIP)
{
	int		i;

	for(i = 0; i < gstSubList.dCount; i++)
	{
		log_print(LOGN_INFO, LH"SUBLIST %d IP=%u:%u SYS=%d", LT, 
			i, gstSubList.stInfo[i].uiIP, uiIP, gstSubList.stInfo[i].dNo);
		if(gstSubList.stInfo[i].uiIP == uiIP)
			return gstSubList.stInfo[i].dNo;
	}

	return -1;
}
