/**
	@file		mmcd_sock.c
	@author		
	@version
	@date		2011-07-14
	@brief		mmcd_sock.c
*/

/**
	Include headers
*/

/* SYS HEADER */
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
/* LIB HEADER */
#include "commdef.h"
#include "config.h"
#include "loglib.h"
/* PRO HEADER */
#include "mmcdef.h"
#include "sockio.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "mmcd_sock.h"

/**
	Declare variables
*/
int 			    gdPortNo    =S_PORT_MMCD;
int	                gdMaxCliCnt = MAX_FDSET2;
extern int			gdSvrSfd;					/*< mmcd_main.h */
extern fd_set		gstReadfds, gstWritefds;	/*< mmcd_main.h */
extern int			gdNumfds;					/*< mmcd_main.h */
extern st_ConTbl	stConTbl[];					/*< mmcd_main.h */
extern int 			gdNumWfds;					/*< mmcd_main.h */
extern int			gdClient;					/*< mmcd_main.h */

/**
 *	Declare extern func.
 */
extern int dGetMaxFds(void);

/**
 *	Implement func.
 */
int dTcpSvrModeInit()
{
	int		dRet;
	int     dReuseaddr=1;
    struct	linger stSvr_ld;
    struct  sockaddr_in stSvr_addr;
	
    stSvr_ld.l_onoff = 1;
    stSvr_ld.l_linger = 0;

    /* 
	* INET, TCP SOCKET 
	*/
    gdSvrSfd = socket( AF_INET, SOCK_STREAM, 0 );
    if( gdSvrSfd < 0 ) {
        log_print( LOGN_CRI, "SOCKET ERROR [%d] [%s]", errno, strerror(errno) );
        exit(0);
    }

	log_print( LOGN_CRI,"[SOCKET] [CONNECT] MAIN SERVER SOCKFD[%d]", gdSvrSfd );

    if( setsockopt( gdSvrSfd, SOL_SOCKET, SO_LINGER, (char*)&stSvr_ld, sizeof(struct linger)) < 0 ) {
        log_print( LOGN_CRI," SETSOCKOPT ERROR : LINGER  [%d] [%s]", errno, strerror(errno));
        exit(0);
    }
    else if( setsockopt( gdSvrSfd, SOL_SOCKET, SO_REUSEADDR, (char*)&dReuseaddr, sizeof(int) ) < 0 ) {
        log_print( LOGN_CRI, "SETSOCKOPT ERROR : REUSEADDR [%d] [%s]",  errno, strerror(errno));
        exit(0);
    }
    else if( fcntl( gdSvrSfd, F_SETFL, O_NDELAY ) < 0 ) {
        log_print( LOGN_CRI, "FCNTL ERROR : NONBLOCK [%d] [%s]",  errno, strerror(errno));
        exit(0);
    }

    bzero( &stSvr_addr, sizeof(struct sockaddr_in) );

    stSvr_addr.sin_family = AF_INET;
    stSvr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    stSvr_addr.sin_port = htons(gdPortNo);


    dRet = bind( gdSvrSfd, (struct sockaddr *)&stSvr_addr, sizeof(struct sockaddr_in) );
    if( dRet < 0 ) {
        log_print( LOGN_CRI, "BIND ERROR : BIND [%d] [%s]", errno, strerror(errno));
        exit(0);
    }

    dRet = listen( gdSvrSfd, LISTEN_PORT_NUM2 );
    if( dRet < 0 ) {
        log_print( LOGN_CRI, "LISTEN ERROR : LISTEN [%d] [%s]", errno, strerror(errno));
        exit(0);
    }


    /* 
	* INET TCP socket Server State COMPLETED 
	*/
    log_print(LOGN_CRI, "PORT[%d] SERVER MODE OK !!!", gdPortNo );

    FD_ZERO( &gstReadfds );
    FD_SET( gdSvrSfd, &gstReadfds );

	FD_ZERO( &gstWritefds );

    gdNumfds = gdSvrSfd;

	log_print( LOGN_DEBUG, "INITIAL : gdNumfds[%d] gdSvrSfd[%d]", gdNumfds, gdSvrSfd );

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dRecvMessage( int dRsfd, char *szTcpRmsg, int* dRecvLen )
{	
    int     dRead_size;

	dRead_size = read( dRsfd, (void*)&szTcpRmsg[0], MAX_MNG_PKT_BUFSIZE-1);
    if( dRead_size <= 0 ) {
        if(dRead_size == 0) {
            log_print(LOGN_CRI, "CLOSE READSIZE : NO MESSAGE" );
            return -1;
        }
        else if(errno != EAGAIN) {
     		log_print(LOGN_CRI, "SUBFUNC READ ERROR dRead_size[%d] SFD[%d] : [%d] %s",
 				dRead_size, dRsfd, errno, strerror(errno) );
            return -2;
        }
        else
            return 0;
    }

    *dRecvLen = dRead_size;
    return 0;
}


/*******************************************************************************

*******************************************************************************/
int dSendMessage( int dSsfd, int dMsgLen, char *szSmsg, int stConIdx )
{
    int     dRet;

	if( stConTbl[stConIdx].cSockBlock == 0x01 ) {
        log_print( LOGN_DEBUG, "CANNOT SEND MESSAGE : BLOCK STATUS IDX[%d] SFD[%d] SIZE[%d]", 
							stConIdx, dSsfd, dMsgLen );

		if( (stConTbl[stConIdx].dBufLen + dMsgLen) > MAX_SENDBUF_LEN ) {
			log_print(LOGN_DEBUG, "DISCARD PACKET : SFD[%d] SIZE[%d]",
								dSsfd, dMsgLen );
			return 1;
		}

		memcpy( &stConTbl[stConIdx].szSendBuffer[stConTbl[stConIdx].dBufLen], szSmsg, dMsgLen );
		stConTbl[stConIdx].dBufLen += dMsgLen;

        return 1;
    }

    dRet = write(dSsfd, (char*)&szSmsg[0], dMsgLen );
    if( dRet < 0 ) {
		if( errno == EAGAIN ) {
        	log_print(LOGN_CRI,
				"SUBFUNC WRITE SFD[%d] WRITE EAGAIN stConIdx[%d] : [%d] [%s]", 
				dSsfd, stConIdx, errno, strerror(errno));

			FD_SET( dSsfd, &gstWritefds );
	
			gdNumWfds++;

			if( stConIdx < 0 )
				return -1;

			stConTbl[stConIdx].cSockBlock = 0x01;

			/* 
			* ADD WRITE FD_SET 
			*/
			if( (stConTbl[stConIdx].dBufLen+dMsgLen) > MAX_SENDBUF_LEN ) {
				log_print(LOGN_CRI,"SEND MESSAGE BLOCK IDX[%d] SFD[%d] CURSIZE[%d]", stConIdx, dSsfd, stConTbl[stConIdx].dBufLen );
			}
			else {
				/* 
				* ADD SEND BUFFER 
				*/
				memcpy( &stConTbl[stConIdx].szSendBuffer[stConTbl[stConIdx].dBufLen], szSmsg, dMsgLen );
            	stConTbl[stConIdx].dBufLen += dMsgLen;

				log_print(LOGN_CRI,"SEND MESSAGE BLOCK IDX[%d] SFD[%d] CURSIZE[%d]", stConIdx, dSsfd, stConTbl[stConIdx].dBufLen );
			}

			return 2;
		}

        log_print(LOGN_CRI,
			" SUBFUNC WRITE SFD[%d] WRITE ERROR stConIdx[%d] : [%d] %s", 
			dSsfd, stConIdx, errno, strerror(errno));
        return -1;
    }
    else {
        if( dRet != dMsgLen ) {
            log_print(LOGN_CRI,
				"WRITE MESSAGE SIZE ERROR stConIdx[%d] [%d] [%d] BUFFER FULL", stConIdx, dRet, dMsgLen);
			/* 
			* ADD WRITE FD_SET 
			*/
			FD_SET( dSsfd, &gstWritefds );

			gdNumWfds++;

			/*** add by tundra 0725 ***/
			if( stConIdx < 0 )
				return -1;

			stConTbl[stConIdx].cSockBlock = 0x01;
			log_print(LOGN_CRI,"SEND MESSAGE BLOCK IDX[%d] SFD[%d] CURSIZE[%d]", stConIdx, dSsfd, stConTbl[stConIdx].dBufLen );

			if( (stConTbl[stConIdx].dBufLen+(dMsgLen-dRet)) > MAX_SENDBUF_LEN ) {
				log_print(LOGN_CRI,"SEND MESSAGE BLOCK IDX[%d] SFD[%d] CURSIZE[%d]", stConIdx, dSsfd, stConTbl[stConIdx].dBufLen );
            }
            else {
				/* 
				* ADD SEND BUFFER 
				*/
                memcpy( &stConTbl[stConIdx].szSendBuffer[stConTbl[stConIdx].dBufLen], &szSmsg[dRet], (dMsgLen-dRet) );
                stConTbl[stConIdx].dBufLen += (dMsgLen-dRet);

				log_print(LOGN_CRI,"SEND MESSAGE BLOCK IDX[%d] SFD[%d] CURSIZE[%d]", stConIdx, dSsfd, stConTbl[stConIdx].dBufLen );
            }

			return 2;
        }
		else {
        	log_print(LOGN_INFO, "SFD:[%d] SEND_SIZE:[%d]", dSsfd, dRet );
			stConTbl[stConIdx].dBufLen = 0;
		}
    }

    return 1;
}

/*******************************************************************************

*******************************************************************************/
int dSendBlockMessage( int dSsfd, int dMsgLen, char *szSmsg, int stConIdx )
{
    int     dRet;

    dRet = write(dSsfd, (char*)&szSmsg[0], dMsgLen );
    if( dRet < 0 ) {
        if( errno == EAGAIN ) {
            log_print(LOGN_CRI,
                "SUBFUNC WRITE SFD[%d] WRITE EAGAIN stConIdx[%d]: [%d] %s",
                dSsfd, stConIdx, errno, strerror(errno));
            /*** ADD WRITE FD_SET ***/ 
            FD_SET( dSsfd, &gstWritefds );

            gdNumWfds++;

			/*** add by tundra 0725 ***/
			if( stConIdx < 0 )
				return -1;	

            stConTbl[stConIdx].cSockBlock = 0x01;

			log_print(LOGN_CRI,"SEND MESSAGE BLOCK IDX[%d] SFD[%d] CURSIZE[%d]", stConIdx, dSsfd, stConTbl[stConIdx].dBufLen );

            return 2;
        }

        log_print(LOGN_CRI,
            " SUBFUNC WRITE SFD[%d] WRITE ERROR stConIdx[%d]: [%d] %s",
            dSsfd, stConIdx, errno, strerror(errno));
        return -1;
    }
	else {
        if( dRet != dMsgLen ) {
            log_print(LOGN_CRI,
                "WRITE MESSAGE SIZE ERROR stConidx[%d] [%d] [%d] BUFFER FULL", stConIdx,  dRet, dMsgLen);
            /*** ADD WRITE FD_SET ***/
			FD_SET( dSsfd, &gstWritefds );
			gdNumWfds++;

			/*** add by tundra 0725 ***/
			if( stConIdx < 0 )
				return -1;

			stConTbl[stConIdx].cSockBlock = 0x01;

			stConTbl[stConIdx].dBufLen = dMsgLen - dRet;

			memcpy( &stConTbl[stConIdx].szSendBuffer[0], &szSmsg[dRet], (dMsgLen-dRet) );
			stConTbl[stConIdx].dBufLen = (dMsgLen-dRet);

			log_print(LOGN_CRI,"SEND MESSAGE BLOCK IDX[%d] SFD[%d] CURSIZE[%d]", stConIdx, dSsfd, stConTbl[stConIdx].dBufLen );
        }
        else {
            log_print(LOGN_INFO, "SFD:[%d] stConIdx[%d] SEND_SIZE:[%d]\n", dSsfd, stConIdx, dRet );
			if( stConIdx < 0 )
				return -1;
			stConTbl[stConIdx].dBufLen = 0;
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

/*******************************************************************************
 RETURN : < 0 => SOCKET CLOSE
*******************************************************************************/
int dSetSockBlock( int dReturn, int dSockFD, int stConIdx )
{
	fd_set	stWd;

	if( dReturn < 0 ) {
		log_print(LOGN_CRI,"SEND MESSAGE FAIL IDX[%d] SFD[%d]", stConIdx, dSockFD );

		/*** add by tundra 0725 ***/
		if( stConIdx < 0 )
			return -1;

		close( dSockFD );
		stConTbl[stConIdx].dSfd = -1;
		stConTbl[stConIdx].dBufLen = 0;
		stConTbl[stConIdx].cRetryCnt = 0;
        FD_CLR( dSockFD, &gstReadfds );

		if( gdClient == 1 ) {
            gdClient--;
			gdNumfds = gdSvrSfd;
		}
        else if( (gdClient-1) == stConIdx ) {
            gdClient--;

            while(1) {
                if( stConTbl[gdClient-1].dSfd > 0 ) {
					gdNumfds = dGetMaxFds();
                    break;
                }
                else {
                    gdClient--;

                    if( gdClient == 0 ) {
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
        if( FD_ISSET( dSockFD, &stWd ) ) {
            log_print(LOGN_DEBUG, "CLEAR WRITEFDS FOR SOCKET CLOSE SFD[%d]", dSockFD );

            FD_CLR( dSockFD, &gstWritefds );

            gdNumWfds--;
            stConTbl[stConIdx].cSockBlock = 0x00;
        }

		return -1;
	}

	return 1;
}
