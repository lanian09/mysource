/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdlib.h>		/* EXIT(3) */
#include <string.h>		/* STRERROR(3) */
#include <unistd.h>		/* WRITE(2), READ(2) */
#include <fcntl.h>		/* O_NDELAY, F_SETFL */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
/* LIB HEADER */
#include "loglib.h"
/* PRO HEADER */
#include "sockio.h"
#include "mmcdef.h"	 /* st_ConTbl */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "cond_sock.h"
#include "cond_func.h" /* cvt_ipaddr */

int gdPortNo=S_PORT_COND;

extern int 		gdSvrSfd;

extern fd_set gstReadfds;
extern fd_set gstWritefds;
extern int gdNumfds;
extern int gdNumWfds;

extern st_ConTbl  stConTbl[MAX_FDSET2+1];

int dTcpSvrModeInit(void)
{
	int		dRet;
    struct linger   stSvr_ld;
    struct  sockaddr_in stSvr_addr;
	int     dReuseaddr=1;

    stSvr_ld.l_onoff = 1;
    stSvr_ld.l_linger = 0;

        /* INET, TCP socket 을 획득한다.  */
    gdSvrSfd = socket( AF_INET, SOCK_STREAM, 0 );

    if( gdSvrSfd < 0 )
    {
        log_print(LOGN_CRI, " socket error [%d] %s\n", errno, strerror(errno) );
        exit(0);
    }

    log_print(LOGN_DEBUG," Main Sockfd[%d]", gdSvrSfd );

    dRet = setsockopt( gdSvrSfd, SOL_SOCKET, SO_LINGER, (char*)&stSvr_ld, sizeof(struct linger));


    if( dRet < 0 )
    {
        log_print(LOGN_DEBUG,"setsockopt error : linger  [%d] %s", errno, strerror(errno));
        exit(0);
    }


    dRet = setsockopt( gdSvrSfd, SOL_SOCKET, SO_REUSEADDR, (char*)&dReuseaddr, sizeof(int) );

    if( dRet < 0 )
    {
        log_print(LOGN_DEBUG,"setsockopt error : reuseaddr [%d] %s",  errno, strerror(errno));
        exit(0);
    }



    dRet = fcntl( gdSvrSfd, F_SETFL, O_NDELAY );

    if( dRet < 0 )
    {
        log_print(LOGN_DEBUG," fcntl error : NONBLOCK [%d] %s",  errno, strerror(errno));
        exit(0);
    }


    bzero( &stSvr_addr, sizeof(struct sockaddr_in) );

    stSvr_addr.sin_family = AF_INET;
    stSvr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    stSvr_addr.sin_port = htons(gdPortNo);


    dRet = bind( gdSvrSfd, (struct sockaddr *)&stSvr_addr, sizeof(struct sockaddr_in) );

    if( dRet < 0 )
    {
        log_print(LOGN_CRI," BIND ERROR : BIND [%d] %s", errno, strerror(errno));
        exit(0);
    }

    dRet = listen( gdSvrSfd, LISTEN_PORT_NUM2 );

    if( dRet < 0 )
    {
        log_print(LOGN_CRI,"LISTEN ERROR : LISTEN [%d] %s", errno, strerror(errno));
        exit(0);
    }


    /* INET TCP socket Server State 완성 */
    log_print(LOGN_CRI," PORT [%d] Server Mode OK !!!", gdPortNo );



    FD_ZERO( &gstWritefds );
    FD_ZERO( &gstReadfds );
    FD_SET( gdSvrSfd, &gstReadfds );


    gdNumfds = gdSvrSfd;
  //  gdNumWfds = gdSvrSfd;


	return 0;
}


/********************************************************************
	int         dRsfd: (input) 입력 socket fd.
	char*       szTcpRmsg; (output) recv한 버퍼.
	int*        dRecvLen; (output) recv 한 버퍼의 길이.
*********************************************************************/
int dRecvMessage(int dRsfd, char *szTcpRmsg, int *dRecvLen)
{
	int		dRead_size;

	fcntl(dRsfd, F_SETFL, O_NDELAY);
	dRead_size = read(dRsfd, (void*)&szTcpRmsg[0], 2048);
	if(dRead_size <= 0)
	{
		if(errno == EWOULDBLOCK)
			return 0;

		log_print(LOGN_DEBUG, "SUbFUNC READ ERROR SFD[%d]: [%d] %s", dRsfd, errno, strerror(errno));
		return -1;
	}
	*dRecvLen = dRead_size;

	return 0;
}

int dSendMessage(int idx, int dSsfd, int dMsgLen, char *szSmsg)
{
	int		dRet;

	fcntl(dSsfd, F_SETFL, O_NDELAY);
	if( (dRet = write(dSsfd, (char*)szSmsg, dMsgLen)) < 0)
	{
		if(errno == EAGAIN)
		{
			log_print(LOGN_DEBUG," SUbFUNC WRITE SFD[%d] WRITE EAGAIN : [%d] %s", dSsfd, errno, strerror(errno));
			/*  writefds 추가 */

			FD_SET(dSsfd, &gstWritefds);
			gdNumWfds++;
			stConTbl[idx].cSockBlock	= 0x01;
			log_print(LOGN_CRI, "SEND MESSAGE BLOCK IDX[%d] SFD[%d] IP[%s]", idx, dSsfd, cvt_ipaddr(stConTbl[idx].uiCliIP));
			memcpy(&stConTbl[idx].szSendBuffer[0], &szSmsg[0], dMsgLen);
			stConTbl[idx].dBufLen		= dMsgLen ;

			return 2;
		}
		log_print(LOGN_CRI," SUBFUNC WRITE IDX[%d] SFD[%d] IP[%s] WRITE ERROR : [%d] %s", idx, dSsfd, cvt_ipaddr(stConTbl[idx].uiCliIP), errno, strerror(errno));

		return -1;
	}
	else
	{
		if(dRet != dMsgLen)
		{
			log_print(LOGN_CRI, "WRITE MESSAGE SIZE ERROR [%d] [%d] BUF FULL", dRet, dMsgLen);

			FD_SET(dSsfd, &gstWritefds);
			gdNumWfds++;
			stConTbl[idx].cSockBlock	= 0x01;
			log_print(LOGN_CRI,"SEND MESSAGE BLOCK IDX[%d] SFD[%d] IP[%s]", idx, dSsfd, cvt_ipaddr(stConTbl[idx].uiCliIP));
			memcpy( &stConTbl[idx].szSendBuffer[0], &szSmsg[dRet], dMsgLen-dRet);
			stConTbl[idx].dBufLen		= dMsgLen - dRet ;

			return 2;
		}
		else
			log_print(LOGN_DEBUG, "SEND COND MESSAGE IDX[%d] IP[%s] SFD[%d] LEN[%d] SUCCESS", idx, cvt_ipaddr(stConTbl[idx].uiCliIP), dSsfd,  dMsgLen);
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

