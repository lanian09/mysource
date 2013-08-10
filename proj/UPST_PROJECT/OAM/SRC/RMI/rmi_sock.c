/*******************************************************************************
			DQMS Project

	Author   :
	Section  : RMI
	SCCS ID  : @(#)rmi_sock.c	1.1
	Date     : 07/21/01
	Revision History :
        '01.  7. 21     Initial
        '03.  1. 15     Initial

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/

/* SYS HEADER */
/* LIB HEADER */
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "rmi_sock.h"

int dSockInit(char *szIPAddr)
{
    int client_len;

    struct sockaddr_in clientaddr;

    dSfd = socket(AF_INET, SOCK_STREAM, 0);
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_addr.s_addr = inet_addr(szIPAddr);
    clientaddr.sin_port = htons(S_PORT_MMCD);

    client_len = sizeof(clientaddr);

    if (connect(dSfd, (struct sockaddr *)&clientaddr, client_len) < 0)
    {
        printf("Connect error: [%d] [%s]\n", errno, strerror(errno));
        return -1;
    }


	FD_ZERO(&stRfds);

	FD_SET( dSfd, &stRfds );

	return 0;
}

void SockCheck(int dSig)
{
	int ret;
	time_t	tTime;

	time(&tTime);

	if( (tTime - tCheck_time) > 3600 ) {
		logout_win();

		send_login_sts(MI_LOGOUT, "SUCCESS");

        close(dSfd);
        printf("[%s] LogOut. Good bye !!!\n\n", SmsName);
        exit (0);
	}


	signal( SIGALRM, SockCheck );

	alarm(2);

	while(1)
	{
		ret = dSockCheck();

		if( ret < 0 )
		{
			printf("MMCD SOCKET CLOSE!! RMI END!!\n");
			exit(0);
		}

		if( ret == 1 )
			break;
	}
}

int dSockCheck(void)
{
	int				dRet, dRecvLen, dSelRet;
	char			szTcpRmsg[3000];
	fd_set			stRd;
	struct timeval	timeout;

	memcpy( (char*)&stRd, (char*)&stRfds, sizeof(fd_set));

	timeout.tv_sec	= 1;
	timeout.tv_usec	= 0;

	if( (dSelRet = select(dSfd+1, &stRd, NULL, NULL, &timeout)) <  0)
	{
		if(errno != EINTR)
		{
			printf("## Socket Select Error[%d] [%s]\n", errno, strerror(errno));
			exit(0);
		}

		return 1;
	}

	if(dSelRet == 0)
		return 1;
	else
	{
		if(FD_ISSET(dSfd, &stRd))
		{
			szTcpRmsg[0]	= 0x00;
			dRecvLen		= 0 ;

			if( (dRet = dRecvMessage(dSfd, szTcpRmsg, &dRecvLen)) < 0)
			{
				printf("Connection Close sfd[%d]\n", dSfd);
				close(dSfd);
				FD_CLR(dSfd, &stRfds);

				printf("[%s] Logout !!!", SmsName);
				exit(0);
			}
			else
			{
				if( (dRet = dMergeBuffer(dSfd, szTcpRmsg, dRecvLen)) < 0)
				{
					printf("Packet Analysis Error -> Connection Close sfd[%d]\n", dSfd);
					close(dSfd);
					FD_CLR(dSfd, &stRfds);
					dSfd	= -1;

					return -1;
				}
			}
		}
		else
		{
			printf("## Socket Select Event Invoke  Error\n");
			exit(0);
		}
		return 0;
	}
}

int dRecvMessage(int dRsfd, char *szTcpRmsg, int *dRecvLen)
{
	int		dRead_size;

	if( (dRead_size = read(dRsfd, (void*)&szTcpRmsg[0], 2048)) <= 0 )
	{
		if(errno == EWOULDBLOCK)
			return 0;

		printf("SubFunc read error SFD[%d][%d]: [%d]\n%s", dRsfd, dRead_size, errno, strerror(errno));
		return -1;

	}
	*dRecvLen = dRead_size;
/*	prn_hexa( &szTcpRmsg[0], *dRecvLen );	*/

	return 0;
}

int dMergeBuffer(int dSfd, char *szTcpRmsg, int dRecvLen)
{
	long long	llMagicNo;
	int			dPktSize, dRet, dCurWidx;
	st_MngPkt	stRpkt;

	dRet		= 0;
	llMagicNo	= MAGIC_NUMBER;
	dCurWidx	= gstMsgBuf.dWidx;

	if( (dRecvLen + gstMsgBuf.dWidx) < MAX_MNG_PKT_BUFSIZE)
	{
		memcpy(&gstMsgBuf.szBuf[gstMsgBuf.dWidx], &szTcpRmsg[0], dRecvLen);
		gstMsgBuf.dWidx += dRecvLen;

		if(gstMsgBuf.dWidx < 8)
			return 0;

		if( *(long long*)&gstMsgBuf.szBuf[0] == llMagicNo)
		{
			if(gstMsgBuf.dWidx > MNG_PKT_HEAD_SIZE)
			{
				memset(&stRpkt, 0x00, sizeof(stRpkt));

				stRpkt.head	= *(pst_MngHead)&gstMsgBuf.szBuf[0];
				dPktSize	= stRpkt.head.usBodyLen + MNG_PKT_HEAD_SIZE;

				if(dPktSize <= gstMsgBuf.dWidx)
				{
					memcpy(&stRpkt.data[0], &gstMsgBuf.szBuf[MNG_PKT_HEAD_SIZE], stRpkt.head.usBodyLen);
				/*
					printf("RecvPKT : BodyLen[%d] SysNo[%d] SvcID[%x] MsgID[%x] MagicNumber[%llx]\n",
						stRpkt.head.usBodyLen, stRpkt.head.ucSysNo, stRpkt.head.ucSvcID, stRpkt.head.ucMsgID, stRpkt.head.llMagicNumber);
				*/

					memcpy(&cli_msg, &stRpkt, sizeof(st_MngPkt));

					if(stRpkt.head.usBodyLen != 0)
						dRet = dRcvPkt_Handle(dSfd, stRpkt);

					gstMsgBuf.dWidx -= dPktSize ;

					if(dRet < 0)
						return -1;

					if(gstMsgBuf.dWidx > 0)
					{
						memcpy(&gstMsgBuf.szBuf[0], &gstMsgBuf.szBuf[dPktSize], gstMsgBuf.dWidx);

						while(gstMsgBuf.dWidx > MNG_PKT_HEAD_SIZE)
						{
							if( *(long long*)&gstMsgBuf.szBuf[0] == llMagicNo)
							{
								memset(&stRpkt, 0x00, sizeof(stRpkt));
								stRpkt.head	= *(pst_MngHead)&gstMsgBuf.szBuf[0];
								dPktSize	= stRpkt.head.usBodyLen;

								if( (dPktSize+MNG_PKT_HEAD_SIZE) <= gstMsgBuf.dWidx)
								{
									memcpy(&stRpkt.data[0], &gstMsgBuf.szBuf[MNG_PKT_HEAD_SIZE], dPktSize);
									gstMsgBuf.dWidx -= MNG_PKT_HEAD_SIZE+dPktSize;
									memcpy(&cli_msg, &stRpkt, sizeof(st_MngPkt));

									if(stRpkt.head.usBodyLen != 0)
										dRet = dRcvPkt_Handle(dSfd, stRpkt);

									if(dRet < 0)
										return -1;

									if(gstMsgBuf.dWidx > 0)
										memcpy(&gstMsgBuf.szBuf[0], &gstMsgBuf.szBuf[MNG_PKT_HEAD_SIZE+dPktSize], gstMsgBuf.dWidx);
								}
								else
									break;
							}
							else
							{
								printf("$$$$$$$$$$$$$$$$$ fail dBufinIdx[%d]\n", gstMsgBuf.dWidx);
								return -1;
							}
						}
					}
					else
						gstMsgBuf.dWidx = 0;

					return 1;
				}
			}
		}
		else
		{
			printf("LOST FRAME\n");
			gstMsgBuf.dWidx = 0;
			return -1;
		}
	}
	else
	{
		printf("BUFFER OVERFLOW INVOKE\n");
		return -2;
	}

	return 0;
}


int dRcvPkt_Handle(int dSsfd, st_MngPkt stMyPkt)
{
	if(stMyPkt.head.usBodyLen == 0)
		return 0;

	printf("\n%s\n[%s] ", stMyPkt.data, SmsName);
	fflush(0);

	return 0;
}


int dSendMessage(int dSsfd, int dMsgLen, char *szSmsg)
{
	int		dRet;

	if( (dRet = write(dSsfd, &szSmsg[0], dMsgLen)) < 0)
	{
		if(errno == EAGAIN)
		{
			printf(" SubFunc Write sfd[%d] write EAGAIN : [%d] %s", dSsfd, errno, strerror(errno));
			return 2;
		}
		printf(" SubFunc Write sfd[%d] write error : [%d] %s", dSsfd, errno, strerror(errno));
		return -1;
	}
	else
	{
		if(dRet != dMsgLen)
		{
			printf("write message size error [%d] [%d] BUF FULL", dRet, dMsgLen);
			return 2;
		}
	}

	return 0;
}
