/** A.1 * File Include *******************************/

/* SYS HEADER */
#include <errno.h>		/* errno */
#include <unistd.h>		/* UNISTD(2) */
#include <string.h>		/* STRERROR(3) */
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
/* LIB HEADER */
#include "filedb.h"		/* MAX_CH_COUNT */
#include "loglib.h"
#include "nsocklib.h"	/* st_ClientInfo */
#include "utillib.h"	/* util_cvtipaddr() */
/* PRO HEADER */
#include "sockio.h"		/* NTAFT, MAGIC_NUMBER */
#include "msgdef.h"		/* SID_CHECK_MSG, MID_SOCK_CHECK */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "if_chnl.h"	/* dCheck_Channel() */
#include "if_msgq.h"	/* dSendToProc(), dGetSysNoWithIP() */
#include "if_sock.h"

/** B.1 *  Definition of New Constants ***************/
/** B.2 *  Definition of New Type  *******************/
/** C.1 *  Declaration of Variables  *****************/
/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/
int dAddSockInTable(st_ClientInfo *stSock, int dSfd, unsigned int uiIP, st_FDInfo *stFD)
{
    int     i;

    for(i = 0; i < MAX_RECORD; i++)
    {
        if(stSock[i].uiIP == uiIP)
        {
            log_print(LOGN_CRI, LH"Aleady IP In Sock Before Client Disconn IP=%s:%u", 
				LT, util_cvtipaddr(NULL, htonl(uiIP)), uiIP);
            dDisConnSock(stSock, i, stFD);
            stSock[i].dSfd      = dSfd;
            stSock[i].uiIP      = uiIP;
            stSock[i].dBufSize  = 0;
            stSock[i].dFront    = 0;
            stSock[i].dRear     = 0;
            stSock[i].tLastTime = time(NULL);
            return i;
        }
    }

    for(i = 0; i < MAX_RECORD; i++)
    {
        if(stSock[i].dSfd == 0)
        {
            stSock[i].dSfd      = dSfd;
            stSock[i].uiIP      = uiIP;
            stSock[i].dBufSize  = 0;
            stSock[i].dFront    = 0;
            stSock[i].dRear     = 0;
            stSock[i].tLastTime = time(NULL);
            return i;
        }
    }

    return -1;
} /* dAddSockInTable */

int dAcceptSockFd(st_ClientInfo *stSock, st_FDInfo *stFD, int *pdPos)
{
	int                 cli_len, newsockfd, sndBuf, rcvBuf, dRet, flags;
    unsigned int        uiIP;
    struct sockaddr_in  stCliAddr;
    struct linger       new_ld;
        
    bzero(&stCliAddr, sizeof(struct sockaddr_in));
    cli_len = sizeof(struct sockaddr_in);
        
    if( (newsockfd = accept(stFD->dSrvSfd, (struct sockaddr*)&stCliAddr, (socklen_t*)&cli_len)) <= 0)
    {       
        log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN accept() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        return -1;
    }           
            
    uiIP    = ntohl(stCliAddr.sin_addr.s_addr);
    sndBuf  = DEF_MAX_SOCK_SIZE;
    rcvBuf  = DEF_MAX_SOCK_SIZE; 
    if(setsockopt(newsockfd, SOL_SOCKET, SO_RCVBUF, &rcvBuf, sizeof(rcvBuf)) < 0)
    {   
        log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN setsockopt(SO_RCVBUF) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        close(newsockfd);
        return -2;
    }       

    if(setsockopt(newsockfd, SOL_SOCKET, SO_SNDBUF, &sndBuf, sizeof(sndBuf)) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN setsockopt(SO_SNDBUF) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        close(newsockfd);
        return -3;
    }

    if( (flags = fcntl(newsockfd, F_GETFL, 0)) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fcntl() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        close(newsockfd);
        return -4;
    }

    flags |= O_NDELAY;
    if(fcntl(newsockfd, F_SETFL, flags) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fcntl(O_NDELAY) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        close(newsockfd);
        return -5;
    }

    new_ld.l_onoff  = 0;
    new_ld.l_linger = 0;
    if(setsockopt(newsockfd, SOL_SOCKET, SO_LINGER, (char *)&new_ld, sizeof (new_ld)) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN setsockopt(SO_LINGER) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        close(newsockfd);
        return -6;
    } /* end of if */

    if( (dRet = dAddSockInTable(stSock, newsockfd, uiIP, stFD)) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dAddSockInTable() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
        close(newsockfd);
        return -1;
    }

    FD_SET(newsockfd, (fd_set*)&stFD->Rfds);
    if(newsockfd >= stFD->dMaxSfd)
        stFD->dMaxSfd = newsockfd+1;

    *pdPos = dRet;
    log_print(LOGN_DEBUG, LH"CONNECT NEW SFD=%d POS=%d IP=%s:%u:%u", 
		LT, newsockfd, *pdPos, util_cvtipaddr(NULL, htonl(uiIP)), uiIP, stSock[dRet].uiIP);

    return newsockfd;
}
int dDisConnSock(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD)
{
	int     i, dMaxFds = 0;

    if(stSock[dIdx].dLastFlag == 1)
    {
        stSock[dIdx].dLastFlag = 0;
        log_print(LOGN_CRI, LH"IDX=%d dSfd=%d CliIP=%s:%u TAFNo=%d", 
            LT, dIdx, stSock[dIdx].dSfd, util_cvtipaddr(NULL, htonl(stSock[dIdx].uiIP)),
			stSock[dIdx].uiIP, stSock[dIdx].dSysNo);
    }
    else
    {
        log_print(LOGN_CRI, LH"Aleady IDX=%d dSfd=%d CliIP=%s:%u TAFNo=%d",
            LT, dIdx, stSock[dIdx].dSfd, util_cvtipaddr(NULL, htonl(stSock[dIdx].uiIP)), 
			stSock[dIdx].uiIP, stSock[dIdx].dSysNo);
    }

    FD_CLR(stSock[dIdx].dSfd, (fd_set*)&stFD->Rfds);
    FD_CLR(stSock[dIdx].dSfd, (fd_set*)&stFD->Wfds);

    if(close(stSock[dIdx].dSfd) < 0)
    {
        log_print(LOGN_CRI, LH"FAILED IN close(SFD[%d])"EH, LT, stSock[dIdx].dSfd, ET);
        return -1;
    } /* end of if */

    for(i = 0; i < MAX_RECORD; i++)
    {
        if(dMaxFds < stSock[i].dSfd)
            dMaxFds = stSock[i].dSfd;
    }

    if(dMaxFds == 0)
        stFD->dMaxSfd = stFD->dSrvSfd + 1;
    else
    {
        if(stFD->dSrvSfd > dMaxFds)
            stFD->dMaxSfd = stFD->dSrvSfd + 1;
        else
            stFD->dMaxSfd = dMaxFds + 1;
    }

    stSock[dIdx].tLastTime  = 0;
    stSock[dIdx].dSysNo     = 0;
    stSock[dIdx].dSfd       = 0;
    stSock[dIdx].uiIP       = 0;
    stSock[dIdx].dBufSize   = 0;
    stSock[dIdx].dFront     = 0;
    stSock[dIdx].dRear      = 0;
    stSock[dIdx].dLastFlag  = 0;

    return 0;
}

int dInitSockFd(st_FDInfo *stFD, int dPort)
{
	int                 sockfd, reuseaddr = 1;
    struct linger       ld;
    struct sockaddr_in  stSrvAddr;

    ld.l_onoff  = 0;
    ld.l_linger = 0;

    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN socket() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        return -1;
    } /* end of if */

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(char*)&reuseaddr, sizeof(reuseaddr)) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN setsockopt(SO_REUSEADDR) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        close(sockfd);
        return -2;
    } /* end of if */

    if(setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char *)&ld, sizeof(ld)) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN setsockopt(SO_LINGER) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        close(sockfd);
        return -3;
    } /* end of if */

    if(fcntl(sockfd, F_SETFL, O_NDELAY) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fcntl(F_SETFL, O_NDELAY) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        close(sockfd);
        return -4;
    } /* end of if */

    bzero(&stSrvAddr, sizeof(struct sockaddr_in));
    stSrvAddr.sin_family        = AF_INET;
    stSrvAddr.sin_addr.s_addr   = htonl(INADDR_ANY);
    stSrvAddr.sin_port          = htons(dPort);

    if(bind(sockfd, (struct sockaddr*)&stSrvAddr, sizeof(struct sockaddr_in)) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN bind() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        close(sockfd);
        return -5;
    } /* end of if */

    if(listen(sockfd, LISTEN_PORT_NUM) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN listen() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        close(sockfd);
        return -6;
    } /* end of if */

    FD_SET(sockfd, (fd_set*)&stFD->Rfds);
    if(sockfd >= stFD->dMaxSfd)
        stFD->dMaxSfd   = sockfd+1;

    stFD->dSrvSfd   = sockfd;
    log_print(LOGN_DEBUG, "F=%s:%s.%d: LISTEN SFD[%d]", __FILE__, __FUNCTION__, __LINE__, sockfd);

    return 0;
}

int dRecvPacket(st_ClientInfo *stSock, int dIndex)
{
	int				n, size, dRet, sp;
	pst_NTAFTHeader	pstHeader;

	size = stSock[dIndex].dBufSize;
	if( (n = read(stSock[dIndex].dSfd, &stSock[dIndex].szBuf[size], DEF_MAX_BUFLEN - size)) <= 0)
	{
		if(n == 0)
			return -1;
		else if(errno != EAGAIN)
			return -1;
		else
			return 0;
	}
	else if(size + n < NTAFT_HEADER_LEN)
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
			pstHeader				= (pst_NTAFTHeader)&stSock[dIndex].szBuf[sp];
			stSock[dIndex].dSysNo	= pstHeader->ucNTAFID;
			if(pstHeader->usTotlLen <= (size - sp))
			{
				time(&stSock[dIndex].tLastTime);
				log_print(LOGN_INFO,"dRecvPacket : TAFNo [%d] dSfd[%d], uiIP[%u] tLastTime[%ld] ",
					stSock[dIndex].dSysNo, stSock[dIndex].dSfd, stSock[dIndex].uiIP, stSock[dIndex].tLastTime);
				if ( (dRet = dSendToProc(stSock, dIndex, &stSock[dIndex].szBuf[sp], pstHeader)) < 0)
					return -1;
				stSock[dIndex].dLastFlag	= SOCKET_OPEN;
				sp							+= pstHeader->usTotlLen;
			}
			else
				break;
		} while (size >= (sp+NTAFT_HEADER_LEN));

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
	int     		dRet, dSfd, i, dIdx;
	fd_set  		fdRead, fdWrite;
	struct timeval  timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	memcpy( (char*)&fdRead, (char*)&stFD->Rfds, sizeof(fd_set));
	memcpy( (char*)&fdWrite, (char*)&stFD->Wfds, sizeof(fd_set));
	if( (	dRet = select(stFD->dMaxSfd, (fd_set*)&fdRead, (fd_set*)&fdWrite, (fd_set*)0, (struct timeval*)&timeout)) < 0)
	{
		log_print(LOGN_CRI, "Check_ClientEvent : FAILED IN select [%s]", strerror(errno));
		return -1;
	} /* end of if */

    if(FD_ISSET(stFD->dSrvSfd, (fd_set *)&fdRead))
	{
		if( (dSfd = dAcceptSockFd(stSock, stFD, &dIdx)) < 0)
		{
			log_print(LOGN_CRI, "Check_ClientEvent : FAILED IN dAcceptSockFd, [%s]",strerror(errno));
			return -1;
		} /* end of if */

		for(i=0; i < MAX_RECORD; i++){
			log_print(LOGN_INFO, "SOCKLIST %d IP=%s:%u FD=%d SYSNO=%d FLAG=%d", 
				i, util_cvtipaddr(NULL, htonl(stSock[i].uiIP)), stSock[i].uiIP, 
				stSock[i].dSfd, stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag);
		}

		if( (dRet = dGetSysNoWithIP(stSock[dIdx].uiIP)) < 0)
		{
			log_print(LOGN_CRI, "NOT SUPPORT SUBSYS FAIL : IDX=%d dSfd=%d uiIP=%s:%u dRet=%d", 
				dIdx, stSock[dIdx].dSfd, util_cvtipaddr(NULL, htonl(stSock[dIdx].uiIP)), stSock[dIdx].uiIP, dRet);

			if(dDisConnSock(stSock, dIdx, stFD) < 0)
				log_print(LOGN_CRI, "Check_ClientEvent : FAILED IN dDisConnSock");
			return 0;
		}

		stSock[dIdx].dSysNo		= dRet;
		stSock[dIdx].dLastFlag	= SOCKET_OPEN;
		if(stSock[dIdx].dSysNo < MAX_CH_COUNT + 1)
			dCheck_Channel(stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag, stSock[dIdx].uiIP);

		log_print(LOGN_WARN, LH"ACCEPT TAFNO=%d IDX=%d MAXSFD=%d SRVSFD=%d SFD=%d IP=%s:%u TIME=%ld",
			LT, dRet, dIdx, stFD->dMaxSfd, stFD->dSrvSfd, stSock[dIdx].dSfd, 
			util_cvtipaddr(NULL,htonl(stSock[dIdx].uiIP)),stSock[dIdx].uiIP, stSock[dIdx].tLastTime);
    } /* end of if */

	for(i = 0; i < MAX_RECORD; i++)
	{
		if( (stSock[i].dSfd > 0) && (FD_ISSET(stSock[i].dSfd, (fd_set *)&fdRead)))
		{
			if( (dRet = dRecvPacket(stSock, i)) < 0)
			{
				stSock[i].dLastFlag = SOCKET_CLOSE;
				if(stSock[i].dSysNo < (MAX_CH_COUNT+1))
					dCheck_Channel(stSock[i].dSysNo, stSock[i].dLastFlag, stSock[i].uiIP);

				if(dDisConnSock(stSock, i, stFD) < 0)
					log_print( LOGN_CRI, "Check_ClientEvent : FAILED IN dDisconnSock RECV");
			}/* end of if */
		}

		if( (stSock[i].dSfd > 0) && (FD_ISSET(stSock[i].dSfd, (fd_set *)&fdWrite)))
			dRet = dSendBlockPacket(stSock, i, stFD);

		if( (stSock[i].dSfd > 0) && ((time(NULL)-stSock[i].tLastTime) > MAX_CHANNEL_TIMEOUT))
		{
			stSock[i].dLastFlag = SOCKET_CLOSE;
			if(stSock[i].dSysNo < MAX_CH_COUNT + 1)
				dCheck_Channel(stSock[i].dSysNo, stSock[i].dLastFlag, stSock[i].uiIP);

			if(dDisConnSock(stSock, i, stFD) < 0)
				log_print( LOGN_CRI, "Check_ClientEvent : FAILED IN dDisConnSock TIME");
		}
	} /* end of for */

    return 1;

} /* end of Check_ClientEvent */

int dSendCheck(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD)
{
	int				dRet;
	st_NTAFTHeader	stHeader;

	stHeader.llMagicNumber	= MAGIC_NUMBER;
	stHeader.usBodyLen	= 0;
	stHeader.usTotlLen	= sizeof(st_NTAFTHeader);
	stHeader.ucSvcID	= SID_CHECK_MSG;
	stHeader.ucMsgID	= MID_SOCK_CHECK;

	if( (dRet = write(stSock[dIdx].dSfd, (char*)&stHeader, sizeof(st_NTAFTHeader))) < 0) {
		if(errno != EAGAIN) {
			log_print(LOGN_CRI, LH"SENDCHK ERR=%d IDX=%d IP=%s:%u FD=%d SYSNO=%d FLAG=%d TIME=%ld",
				LT, errno, dIdx, util_cvtipaddr(NULL,htonl(stSock[dIdx].uiIP)),stSock[dIdx].uiIP, 
				stSock[dIdx].dSfd, stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag, stSock[dIdx].tLastTime);

			stSock[dIdx].dLastFlag = SOCKET_CLOSE;

			if(stSock[dIdx].dSysNo < (MAX_CH_COUNT+1))
				dCheck_Channel(stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag, stSock[dIdx].uiIP);

			if(dDisConnSock(stSock, dIdx, stFD) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dDisConnSock() TIME", LT);
			log_print(LOGN_CRI, LH"FAILED IN write(SFD[%d])"EH, LT, stSock[dIdx].dSfd, ET);
		}

	} else {
		log_print(LOGN_INFO, "SENDCHK %d dRet=%d IP=%s:%u FD=%d SYSNO=%d FLAG=%d TIME=%ld",
			dIdx, dRet, util_cvtipaddr(NULL, htonl(stSock[dIdx].uiIP)), stSock[dIdx].uiIP, stSock[dIdx].dSfd, 
			stSock[dIdx].dSysNo, stSock[dIdx].dLastFlag, stSock[dIdx].tLastTime);
		//불필요한 부분으로 파악됨. 이미 함수 호출 조건에 lasttime을 사용. noted by uamyd 20110628
		//stSock[dIdx].tLastTime = time(NULL);
	}

	return 0;
}

int dSendPacket(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD, char *str, int slen)
{
    int i, k;

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

    memcpy(&stSock[dIdx].szWBuf[0], (void *)str, slen);
    if ((i = write(stSock[dIdx].dSfd, &stSock[dIdx].szWBuf[0], slen)) != slen) {
        if (i < 0) {
            if (errno != EAGAIN) {
				stSock[dIdx].dLastFlag = SOCKET_CLOSE;
				if(stSock[dIdx].dSysNo < MAX_CH_COUNT + 1)
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

        if ((t = write(stSock[dIdx].dSfd, &stSock[dIdx].szWBuf[stSock[dIdx].dRear], k)) <= 0) {
            if ( errno != EAGAIN) {
				stSock[dIdx].dLastFlag = SOCKET_CLOSE;
				if(stSock[dIdx].dSysNo < MAX_CH_COUNT + 1)
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
    }

	return 0;
}
