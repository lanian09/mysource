/**A.1* FILE INCLUSION ********************************************************/
#include <stdio.h>
#include <sys/time.h>
#include <time.h>		/*	time(2)	*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>

#include "nsocklib.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/


int init_sock(int *dSrvSfd, int dPort, fd_set *Rfds, int *Numfds)
{
	int						sockfd, reuseaddr = 1;
	struct sockaddr_in		stSrvAddr;
	struct linger			ld;

    ld.l_onoff = 0;
    ld.l_linger = 0;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		close(sockfd);
		return E_SOCKET;
	} /* end of if */

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(char *)&reuseaddr, sizeof(reuseaddr)) < 0) {
		close(sockfd);
		return E_REUSEADDR;
    } /* end of if */

    if(setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char *)&ld, sizeof(ld)) < 0) {
		close(sockfd);
        return E_LINGER;
    } /* end of if */

    if(fcntl(sockfd, F_SETFL, O_NDELAY) < 0) {
		close(sockfd);
		return E_NONBLOCK;
    } /* end of if */

	bzero(&stSrvAddr, sizeof(struct sockaddr_in));
    stSrvAddr.sin_family = AF_INET;
    stSrvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    stSrvAddr.sin_port = htons(dPort);

    if(bind(sockfd, (struct sockaddr *)&stSrvAddr, sizeof(struct sockaddr_in)) < 0) {
        close(sockfd);
        return E_BIND;
    } /* end of if */

    if(listen(sockfd, LISTEN_PORT_NUM) < 0) {
        close(sockfd);
        return E_LISTEN;
    } /* end of if */

    add_tup_in_polltable(sockfd, Rfds, Numfds);

	*dSrvSfd = sockfd;

    return SUCCESS;

} /* end of dInitSock */

int accept_sock(stNetTuple *stNet, int sock_sfd, fd_set *Rfds, int *Numfds, int dSockBuf, int *pdPos)
{
    int   				cli_len, newsockfd, sndBuf, rcvBuf, dRet, flags;
	struct sockaddr_in	stCliAddr;
    struct linger		new_ld;
	unsigned int		uiIP;

	bzero(&stCliAddr, sizeof(struct sockaddr_in));
    cli_len = sizeof(struct sockaddr_in);

    newsockfd = accept(sock_sfd, (struct sockaddr *)&stCliAddr, (socklen_t *)&cli_len);
	if(newsockfd <= 0) {
        return E_ACCEPT;
    } /* end of else */

	uiIP = ntohl(stCliAddr.sin_addr.s_addr);
	sndBuf = dSockBuf;
	rcvBuf = dSockBuf;
	if(setsockopt(newsockfd, SOL_SOCKET, SO_RCVBUF, &rcvBuf, sizeof(rcvBuf)) < 0) {
		close(newsockfd);
		return E_SET_RCVBUF;
	} /* end of if */

	if(setsockopt(newsockfd, SOL_SOCKET, SO_SNDBUF, &sndBuf, sizeof(sndBuf)) < 0) {
		close(newsockfd);
		return E_SET_SNDBUF;
	} /* end of if */

	if((flags = fcntl(newsockfd, F_GETFL, 0)) < 0) {
		close(newsockfd);
		return E_GET_FLAGS;
	} /* end of if */

	flags |= O_NDELAY;
	if(fcntl(newsockfd, F_SETFL, flags) < 0) {
		close(newsockfd);
		return E_NONBLOCK;
	} /* end of if */

	new_ld.l_onoff = 0;
	new_ld.l_linger = 0;
	if (setsockopt(newsockfd, SOL_SOCKET, SO_LINGER, (char *)&new_ld, sizeof (new_ld)) < 0) {
		close(newsockfd);
		return E_LINGER;
	} /* end of if */

   	FD_SET(newsockfd, (fd_set *)Rfds);
   	if(newsockfd >= *Numfds)
       	*Numfds = newsockfd + 1;

	dRet = add_tup_in_conntable(stNet, newsockfd, uiIP, Rfds, Numfds, sock_sfd);
	if(dRet < 0) {
		close(newsockfd);
		del_tup_in_polltable(stNet, newsockfd, Rfds, Numfds, sock_sfd);
		return dRet;
	}

	*pdPos = dRet;
	return newsockfd;

} /* end of accept_sock */

int add_tup_in_conntable(stNetTuple *stNet, int dSfd,
		unsigned long uiIP, fd_set *fdSet, int *NumFds, int dSrvSfd)
{
    int     i;

    for (i=0; i<MAX_RECORD; i++) {
        if (stNet[i].uiIP == uiIP) {
			disconn_sock(stNet, stNet[i].dSfd, fdSet, NumFds, dSrvSfd);
			stNet[i].tLastTime = time(NULL);
			stNet[i].dStatus   = 0;
			stNet[i].dSfd      = dSfd;
			stNet[i].uiIP      = uiIP;
			stNet[i].dBufSize  = 0;
			stNet[i].dIdx      = 0;
			return i;
		} /* end of if */
    } /* end of for */

    for (i=0; i<MAX_RECORD; i++) {
        if (stNet[i].dSfd == 0) {
			stNet[i].tLastTime = time(NULL);
			stNet[i].dStatus   = 1;
            stNet[i].dSfd      = dSfd;
            stNet[i].uiIP      = uiIP;
			stNet[i].dBufSize  = 0;
			stNet[i].dIdx      = 0;
			return i;
        } /* end of if */
    } /* end of for */

    if (i >= MAX_RECORD){
        return E_MAX_TUPPLE;
	}

    return i;

} /* add_tup_in_conntable */


int add_tup_in_polltable(int fd, fd_set *Rfds, int *Numfds)
{
    FD_SET(fd, (fd_set *)Rfds);
    if(fd >= *Numfds){
        *Numfds = fd+1;
	}

    return SUCCESS;
} /* add_tup_in_polltable */


int del_tup_in_conntable(stNetTuple *stNet, int del_sfd)
{
    int     i;

    for(i=0; i<MAX_RECORD; i++) {
        if(stNet[i].dSfd == del_sfd) {
			stNet[i].tLastTime = time(NULL);
			stNet[i].dStatus   = 0;
            stNet[i].dSfd      = 0;
			stNet[i].uiIP      = 0;
			stNet[i].dBufSize  = 0;
			stNet[i].dIdx      = 0;
            break;
        } /* end of if */
    } /* end of for */

    if(i == MAX_RECORD) {
        return E_NO_ENTRY;
    } /* end of if */

    return SUCCESS;

} /* del_tup_in_conntable */


int del_tup_in_polltable(stNetTuple *stNet, int fd, fd_set *fdSet, int *NumFds, int dSrvSfd)
{
	int		i, dMaxFds;

	dMaxFds = 0;

	for(i = 0; i < MAX_RECORD; i++) {
		if(dMaxFds < stNet[i].dSfd){
			dMaxFds = stNet[i].dSfd;
		}
	}

	if(dMaxFds == 0){
		*NumFds = dSrvSfd + 1;
	} else {
		if(dSrvSfd > dMaxFds){
			*NumFds = dSrvSfd + 1;
		} else {
			*NumFds = dMaxFds + 1;
		}
	}
	FD_CLR(fd, (fd_set*)fdSet);

	return SUCCESS;
} /* del_tup_in_polltable */

int disconn_sock(stNetTuple *stNet, int dSfd, fd_set *fdSet, int *NumFds, int dSrvSfd)
{
	int	i, dRet;

	for(i = 0; i < MAX_RECORD; i++) {
		if(stNet[i].dSfd == dSfd) {
			if(close(dSfd) < 0) {
				return E_SOCK_CLOSE;
			} /* end of if */

			if( (dRet = del_tup_in_conntable(stNet, dSfd)) < 0 ){
				return E_NO_ENTRY;
			}

			del_tup_in_polltable(stNet, dSfd, fdSet, NumFds, dSrvSfd);

			return SUCCESS;
		} /* end of if */
	} /* end of for */

	return E_NO_ENTRY2;
} /* disconn_sock */

int dInitSockFd(st_FDInfo *stFD, int dPort)
{
	int					sockfd, reuseaddr = 1;
	struct linger		ld;
	struct sockaddr_in	stSrvAddr;

	ld.l_onoff	= 0;
	ld.l_linger	= 0;

	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return -1;
	} /* end of if */

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(char*)&reuseaddr, sizeof(reuseaddr)) < 0)
	{
		close(sockfd);
		return -2;
	} /* end of if */

	if(setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char *)&ld, sizeof(ld)) < 0)
	{
		close(sockfd);
		return -3;
	} /* end of if */

	if(fcntl(sockfd, F_SETFL, O_NDELAY) < 0)
	{
		close(sockfd);
		return -4;
	} /* end of if */

	bzero(&stSrvAddr, sizeof(struct sockaddr_in));
	stSrvAddr.sin_family		= AF_INET;
	stSrvAddr.sin_addr.s_addr	= htonl(INADDR_ANY);
	stSrvAddr.sin_port			= htons(dPort);

	if(bind(sockfd, (struct sockaddr*)&stSrvAddr, sizeof(struct sockaddr_in)) < 0)
	{
		close(sockfd);
		return -5;
	} /* end of if */

	if(listen(sockfd, LISTEN_PORT_NUM) < 0)
	{
		close(sockfd);
		return -6;
	} /* end of if */

	FD_SET(sockfd, (fd_set*)&stFD->Rfds);
	if(sockfd >= stFD->dMaxSfd)
		stFD->dMaxSfd	= sockfd+1;

	stFD->dSrvSfd	= sockfd;

	return 0;
} /* end of dInitSock */

int dAcceptSockFd(st_ClientInfo *stSock, st_FDInfo *stFD, int *pdPos)
{
	int					cli_len, newsockfd, sndBuf, rcvBuf, dRet, flags;
	unsigned int		uiIP;
	struct sockaddr_in	stCliAddr;
	struct linger		new_ld;

	bzero(&stCliAddr, sizeof(struct sockaddr_in));
	cli_len = sizeof(struct sockaddr_in);

	if( (newsockfd = accept(stFD->dSrvSfd, (struct sockaddr*)&stCliAddr, (socklen_t*)&cli_len)) <= 0)
	{
		return -1;
	}

	uiIP	= ntohl(stCliAddr.sin_addr.s_addr);
	sndBuf	= DEF_MAX_SOCK_SIZE;
	rcvBuf	= DEF_MAX_SOCK_SIZE;
	if(setsockopt(newsockfd, SOL_SOCKET, SO_RCVBUF, &rcvBuf, sizeof(rcvBuf)) < 0)
	{
		close(newsockfd);
		return -2;
	}

	if(setsockopt(newsockfd, SOL_SOCKET, SO_SNDBUF, &sndBuf, sizeof(sndBuf)) < 0)
	{
		close(newsockfd);
		return -3;
	}

	if( (flags = fcntl(newsockfd, F_GETFL, 0)) < 0)
	{
		close(newsockfd);
		return -4;
	}

	flags |= O_NDELAY;
	if(fcntl(newsockfd, F_SETFL, flags) < 0)
	{
		close(newsockfd);
		return -5;
	}

	new_ld.l_onoff	= 0;
	new_ld.l_linger	= 0;
	if(setsockopt(newsockfd, SOL_SOCKET, SO_LINGER, (char *)&new_ld, sizeof (new_ld)) < 0)
	{
		close(newsockfd);
		return -6;
	} /* end of if */

	if( (dRet = dAddSockInTable(stSock, newsockfd, uiIP, stFD)) < 0)
	{
		close(newsockfd);
		return -1;
	}

	FD_SET(newsockfd, (fd_set*)&stFD->Rfds);
	if(newsockfd >= stFD->dMaxSfd)
		stFD->dMaxSfd = newsockfd+1;

	*pdPos = dRet;

	return newsockfd;
} /* end of dAcceptSockFd */

int dDisConnSock(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD)
{
	int		i, dMaxFds = 0;

	if(stSock[dIdx].dLastFlag == 1)
	{
		stSock[dIdx].dLastFlag = 0;
	}
	else
	{
	}

	FD_CLR(stSock[dIdx].dSfd, (fd_set*)&stFD->Rfds);
	FD_CLR(stSock[dIdx].dSfd, (fd_set*)&stFD->Wfds);

	if(close(stSock[dIdx].dSfd) < 0)
	{
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

	stSock[dIdx].tLastTime	= 0;
	stSock[dIdx].dSysNo		= 0;
	stSock[dIdx].dSfd		= 0;
	stSock[dIdx].uiIP		= 0;
	stSock[dIdx].dBufSize	= 0;
	stSock[dIdx].dFront		= 0;
	stSock[dIdx].dRear		= 0;
	stSock[dIdx].dLastFlag	= 0;

	return 0;
} /* dDisConnSock */

int dAddSockInTable(st_ClientInfo *stSock, int dSfd, unsigned int uiIP, st_FDInfo *stFD)
{
	int		i;

	for(i = 0; i < MAX_RECORD; i++)
	{
		if(stSock[i].uiIP == uiIP)
		{
			dDisConnSock(stSock, i, stFD);
			stSock[i].dSfd		= dSfd;
			stSock[i].uiIP		= uiIP;
			stSock[i].dBufSize	= 0;
			stSock[i].dFront	= 0;
			stSock[i].dRear		= 0;
			stSock[i].tLastTime	= time(NULL);
			return i;
		}
	}

	for(i = 0; i < MAX_RECORD; i++)
	{
		if(stSock[i].dSfd == 0)
		{
			stSock[i].dSfd		= dSfd;
			stSock[i].uiIP		= uiIP;
			stSock[i].dBufSize	= 0;
			stSock[i].dFront	= 0;
			stSock[i].dRear		= 0;
			stSock[i].tLastTime	= time(NULL);
			return i;
		}
	}

	return -1;
} /* dAddSockInTable */

