/*******************************************************************************
			DQMS Project

	Author   : Jae Seung Lee
	Section  : NSOCK
	SCCS ID  : @(#)nsock.c	1.1
	Date     : 01/21/05
	Revision History :
		05. 01. 21      Initial
		08. 01. 07		Update By LSH for review
		08. 01. 15		Add stClient Sock Lib By LSH

	Description :
		04/01/23 Initial By Hwang Woo-Hyoung

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/** A.1* FILE INCLUSION ***********************************/
#include <stdio.h>			/*	BUFSIZ	*/
#include <string.h>			/*	strcasestr(3)	*/
#include <ctype.h>			/*	toupper(3)	*/
#include <unistd.h>			/*	fcntl(2), FD_SET(3), FD_CLR(3), read(2), write(2), close(2), select(2)	*/
#include <fcntl.h>			/*	fcntl(2)	*/
#include <errno.h>			/*	errno(3)	*/
#include <string.h>			/*	strerror(3), memcpy(3)	*/
#include <sys/socket.h>		/*	socket(2), setsockopt(2), bind(2), listen(2), inet_aton(3), accept(2)	*/
#include <netinet/in.h>		/*	inet_aton(3)	*/
#include <arpa/inet.h>		/*	htonl(3), htons(3), inet_aton(3)	*/
#include <mysql/mysql.h>	/*	MYSQL		*/

// LIB
#include "loglib.h"

// OAM
#include "path.h"
#include "sockio.h"
#include "filedb.h"
#include "almstat.h"		/* st_almsts */

// PROJECT
#include "msgdef.h"

// .
#include "si_nms_comm.h"
#include "si_nms_sock.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
// Move to si_nms_sock.h

/** C.1* DEFINITION OF NEW TYPES **************************/
// TODO lgt_nms.h 에 정의되어 있음, 다른곳에서는 사용하지 않음. Head, Header, Packet1, Packet2
// TODO lgt_nms.h 에 정의되어 있음, 다른곳에서는 사용하지 않음. Head, Header, Packet1, Packet2
// TODO lgt_nms.h 에 정의되어 있음, 다른곳에서는 사용하지 않음. Head, Header, Packet1, Packet2
typedef struct _Head_
{
#if defined (__LITTLE_ENDIAN_BITFIELD)
	unsigned char		Type:3;
	unsigned char		CtrlBit:1;	// MMC, Console에서 사용됨 1: 다음 패킷이 존재, 0: 마지막 패킷
	unsigned char		Ver:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	unsigned char		Ver:4;
	unsigned char		CtrlBit:1;	// MMC, Console에서 사용됨 1: 다음 패킷이 존재, 0: 마지막 패킷
	unsigned char		Type:3;
#else
#error "Please fix <asm/byteorder.h>"
#endif
	unsigned char		CtrlSeq;	// 여러 패킷 전송시의 순번(0부터 시작): MMC, Console 응답에서 사용됨
	unsigned char		Src;
	unsigned char		Dest;
} Head, *pHead;

typedef struct _Header_
{
	Head				hd;
	unsigned short		Prmt;
	unsigned short		Len;
} Header, *pHeader;
#define LGT_NMS_HDR_SIZE	sizeof(Header)

/* Control Packet */
#define CTRL_PKT_HDR_SIZE			8			/* control packet header size */
typedef struct _Packet1_
{
	Header				hdr;
	char				Padding[MAX_BUF_SIZE - CTRL_PKT_HDR_SIZE];
} Packet1, *pPacket1;

/* Simple Packet */
#define SMPL_PKT_HDR_SIZE			12			/* simple packet header size */
typedef struct _Packet2_
{
	Header				hdr;
	unsigned int		MsgType;
	char				Data[MAX_BUF_SIZE - SMPL_PKT_HDR_SIZE];
} Packet2, *pPacket2;

/* Complex Packet */
#define CMPL_PKT_HDR_SIZE			16			/* complex packet header size */
typedef struct _Packet3_
{
	Header				hdr;
	unsigned int		MsgType;
	unsigned short		SerNo;		// 사용안함 0으로 세팅
	unsigned short		AttrGrpCnt;	// 사용안함 0으로 세팅
	char				Attr[MAX_BUF_SIZE - CMPL_PKT_HDR_SIZE];
} Packet3, *pPacket3;

/** D.1* DECLARATION OF VARIABLES *************************/

/** D.2* DECLARATION OF VARIABLES *************************/
extern MYSQL			stMySQL;
extern st_NMSIPList		gstNMSIPList;							/*	SRC/SI_NMS/si_nms_main.c	*/
extern st_NMSPortInfo	gstNMSPortInfo;							/*	SRC/SI_NMS/si_nms_main.c	*/
extern st_NMSSFdInfo	gstNMSSFdInfo[MAX_CONN];				/*	SRC/SI_NMS/si_nms_main.c	*/

/** E.1* DEFINITION OF FUNCTIONS **************************/
// Move to si_nms_sock.h

/** E.2* DEFINITION OF FUNCTIONS **************************/
extern int dSaveFileList(char *FileName, int dSfd, int dPeriod);											/*	SRC/SI_NMS/si_nms_func.c	*/
extern int dMakeOIDFile(MYSQL *pstMySQL, st_atQueryInfo *pstAtQueryInfo, char *sFileName);					/*	SRC/SI_NMS/si_nms_func.c	*/
extern int dResetFileList(char *sFileName, int dSfd);														/*	SRC/SI_NMS/si_nms_func.c	*/
extern int dReadNTAFName(st_NTafName *pstNTafName);															/*	SRC/SI_NMS/si_nms_func.c	*/
extern void GetAlarmStr(char *sAlias, unsigned char ucLocType, unsigned char ucInvType, char *psBuf);		/*	SRC/SI_NMS/si_nms_func.c	*/
extern int dCheck_Channel(int hdFlag, unsigned int uiIP);													/*	SRC/SI_NMS/si_nms_func.c	*/


int dInitSockFd(st_SelectInfo *stFD, int dListenIdx)
{
	int					sockfd, reuseaddr;
	struct linger		ld;
	struct sockaddr_in	stSrvAddr;
	struct in_addr		addr;

	reuseaddr	= 1;
	ld.l_onoff	= 0;
	ld.l_linger	= 0;

	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN socket(Port[%d]) [errno:%d-%s]", LT,
			gstNMSPortInfo.port[dListenIdx], errno, strerror(errno));
		close(sockfd);
		return -1;
	} /* end of if */

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(char*)&reuseaddr, sizeof(reuseaddr)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN setsockopt(SO_REUSEADDR) [errno:%d-%s]", LT, errno, strerror(errno));
		close(sockfd);
		return -2;
	} /* end of if */

	if(setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char*)&ld, sizeof(ld)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN setsockopt(SO_LINGER) [errno:%d-%s]", LT, errno, strerror(errno));
		close(sockfd);
		return -3;
	} /* end of if */

	if(fcntl(sockfd, F_SETFL, O_NDELAY) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN fcntl(O_NDELAY) [errno:%d-%s]", LT, errno, strerror(errno));
		close(sockfd);
		return -4;
	} /* end of if */

	bzero(&stSrvAddr, sizeof(struct sockaddr_in));
	stSrvAddr.sin_family		= AF_INET;
	stSrvAddr.sin_addr.s_addr	= htonl(INADDR_ANY);
	stSrvAddr.sin_port			= htons(gstNMSPortInfo.port[dListenIdx]);

	if(bind(sockfd, (struct sockaddr*)&stSrvAddr, sizeof(struct sockaddr_in)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN bind() [errno:%d-%s]", LT, errno, strerror(errno));
		close(sockfd);
		return -5;
	} /* end of if */

	if(listen(sockfd, LISTEN_PORT_NUM) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN listen() [errno:%d-%s]", LT, errno, strerror(errno));
		close(sockfd);
		return -6;
	} /* end of if */

	FD_SET(sockfd, (fd_set*)&stFD->Rfds);

	if(sockfd >= stFD->dMaxSfd)
		stFD->dMaxSfd = sockfd+1;

	gstNMSSFdInfo[dListenIdx].dSfd			= sockfd;
	gstNMSSFdInfo[dListenIdx].dListenPort	= gstNMSPortInfo.port[dListenIdx];
	gstNMSSFdInfo[dListenIdx].dType			= FD_TYPE_LISTEN;

	if(inet_aton(gstNMSPortInfo.ipaddr[0], &addr) == 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN inet_aton(%s) [errno:%d-%s]", LT,
			gstNMSPortInfo.ipaddr[0], errno, strerror(errno));
		return -7;
	}

	gstNMSSFdInfo[dListenIdx].uIPAddr = htonl((unsigned int)addr.s_addr);

	if(time(&gstNMSSFdInfo[dListenIdx].tLastTime) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN time() [errno:%d-%s]", LT, errno, strerror(errno));
		return -8;
	}

	log_print(LOGN_DEBUG, LH"LISTEN SFD{dListenIdx[%d])[%d]", LT, dListenIdx, gstNMSSFdInfo[dListenIdx].dSfd);

	return 0;
}

int Check_ClientEvent(st_NMSSFdInfo *stSock, st_SelectInfo *stFD)
{
	int				i, dRet, dSfd, dIdx;
	fd_set			fdRead, fdWrite;
	struct timeval	timeout;

	timeout.tv_sec	= 0;
	timeout.tv_usec	= SELECT_TIMEOUT;	/*	This is defined in "lgt_nms.h" file.	*/

	memcpy((char*)&fdRead, (char*)&stFD->Rfds, sizeof(fd_set));
	memcpy((char*)&fdWrite, (char*)&stFD->Wfds, sizeof(fd_set));

	if( (dRet = select(stFD->dMaxSfd, (fd_set*)&fdRead, (fd_set*)&fdWrite, (fd_set*)0, (struct timeval*)&timeout)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN select() [errno:%d-%s]", LT, errno, strerror(errno));
		return -1;
	}

	for(i = 0; i < PORT_IDX_MAX; i++)
	{
		if(FD_ISSET(stSock[i].dSfd, (fd_set*)&fdRead))
		{
			if( (dSfd = dAcceptSockFd(stSock, stFD, i, &dIdx)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dAcceptSockFd() dRet[%d]", LT, dRet);
				return -2;
			} /* end of if */

			log_print(LOGN_INFO, "SOCKLIST %d IP[%u] Port[%d] FD[%d] TYPE[%s]", i, stSock[i].uIPAddr, stSock[i].dListenPort, stSock[i].dSfd,
				(stSock[i].dType == FD_TYPE_LISTEN) ? "FD_TYPE_LISTEN" : "FD_TYPE_DATA");

			stSock[i].dLastFlag	= 1;
			dCheck_Channel(stSock[i].dLastFlag, stSock[i].uIPAddr);

			log_print(LOGN_WARN, "ACCEPT IDX[%d] MAXSFD[%d] ListenPort[%d] SFD[%d] IP[%u] TIME[%ld]",
				dIdx, stFD->dMaxSfd, stSock[i].dListenPort, stSock[i].dSfd, stSock[i].uIPAddr, stSock[i].tLastTime);
		}
	}

	for(i = PORT_IDX_MAX; i < MAX_CONN; i++)
	{
		if( (stSock[i].dSfd > 0) && (FD_ISSET(stSock[i].dSfd, (fd_set*)&fdRead)))
		{
			if( (dRet = dRecvPacket(stSock, i, stFD)) < 0)
			{
				stSock[i].dLastFlag = 0;
				dCheck_Channel(stSock[i].dLastFlag, stSock[i].uIPAddr);

				if(dDisConnSock(stSock, i, stFD) < 0)
					log_print(LOGN_CRI, LH"FAILED IN dDisconnSock RECV", LT);
			}
		}

		if( (stSock[i].dSfd>0) && (FD_ISSET(stSock[i].dSfd, (fd_set*)&fdWrite)))
			dRet = dSendBlockPacket(stSock, i, stFD);

		if( (stSock[i].stMMCInfo.dMMCSfd > 0) && (FD_ISSET(stSock[i].stMMCInfo.dMMCSfd, (fd_set*)&fdRead)))
		{
			if( (dRet = dRecvMMCResponse(stSock, i, stFD)) < 0)
				log_print(LOGN_CRI, LH"FAILED IN dRecvMMCResponse(i[%d]) dRet[%d]", LT, i, dRet);
		}
	}

	return 0;
}

int dSendPacket(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD, char *str, int slen)
{
	int			dRet;
	ssize_t		sszWrLen, sszTotalWrLen;

	if(FD_ISSET(stSock[dCommunicatedIdx].dSfd, (fd_set*)&stFD->Wfds))
	{
		sszTotalWrLen = stSock[dCommunicatedIdx].dWriteStartPos + slen;

		if( ((stSock[dCommunicatedIdx].dWriteStartPos<stSock[dCommunicatedIdx].dWriteEndPos) && (sszTotalWrLen>=stSock[dCommunicatedIdx].dWriteEndPos)) ||
			((stSock[dCommunicatedIdx].dWriteStartPos>stSock[dCommunicatedIdx].dWriteEndPos) && (sszTotalWrLen>=(stSock[dCommunicatedIdx].dWriteEndPos+MAX_BUF_SIZE))))
		{
			log_print(LOGN_CRI, LH"Packet discarded slen[%d] socket[%d]", LT,
				slen, stSock[dCommunicatedIdx].dSfd);
			return -1;
		}

		if(sszTotalWrLen >= MAX_BUF_SIZE)
		{
			sszTotalWrLen -= MAX_BUF_SIZE;
			memcpy(&stSock[dCommunicatedIdx].sWriteBuf[stSock[dCommunicatedIdx].dWriteStartPos], (void*)str, slen-sszTotalWrLen);
			memcpy(&stSock[dCommunicatedIdx].sWriteBuf[0], (void*)&str[slen-sszTotalWrLen], sszTotalWrLen);
		}
		else
			memcpy(&stSock[dCommunicatedIdx].sWriteBuf[stSock[dCommunicatedIdx].dWriteStartPos], (void *)str, slen);

		stSock[dCommunicatedIdx].dWriteStartPos = sszTotalWrLen;

		return 0;
	}
	else if(stSock[dCommunicatedIdx].dWriteStartPos != stSock[dCommunicatedIdx].dWriteEndPos)
	{
		stSock[dCommunicatedIdx].dWriteStartPos = stSock[dCommunicatedIdx].dWriteEndPos = 0;
		log_print(LOGN_CRI, LH"Undefined error occurres in writeSocket", LT);
	}

	memcpy(&stSock[dCommunicatedIdx].sWriteBuf[0], (void*)str, slen);
	if( (sszWrLen = write(stSock[dCommunicatedIdx].dSfd, &stSock[dCommunicatedIdx].sWriteBuf[0], slen)) != slen)
	{
		if(sszWrLen < 0)
		{
			log_print(LOGN_CRI, LH"FAILED IN write() [errno:%d-%s]", LT, errno, strerror(errno));
			if(errno != EAGAIN)
			{
				stSock[dCommunicatedIdx].dLastFlag = 0;
				dCheck_Channel(stSock[dCommunicatedIdx].dLastFlag, stSock[dCommunicatedIdx].uIPAddr);
				if( (dRet = dDisConnSock(stSock, dCommunicatedIdx, stFD)) < 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN dDisConnSock() dRet[%d]", LT, dRet);
					return -2;
				}

				return -3;
			}
			else
				sszWrLen = 0;
		}
		stSock[dCommunicatedIdx].dWriteEndPos	= sszWrLen;
		stSock[dCommunicatedIdx].dWriteStartPos	= slen;
		FD_SET(stSock[dCommunicatedIdx].dSfd, (fd_set*)&stFD->Wfds);

		return 0;
	}
	stSock[dCommunicatedIdx].dWriteStartPos	= 0;
	stSock[dCommunicatedIdx].dWriteEndPos	= 0;

	return 0;
}

int dSendBlockPacket(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD)
{
	ssize_t		t;
	int			k, dRet;

	if(stSock[dCommunicatedIdx].dWriteStartPos == stSock[dCommunicatedIdx].dWriteEndPos)
	{
		FD_CLR(stSock[dCommunicatedIdx].dSfd, (fd_set*)&stFD->Wfds);
		stSock[dCommunicatedIdx].dWriteStartPos = stSock[dCommunicatedIdx].dWriteEndPos = 0;
		log_print(LOGN_CRI, LH"Invalid Length in socket write buffer Position[%ld]", LT, stSock[dCommunicatedIdx].dWriteStartPos);
	}
	else
	{
		if(stSock[dCommunicatedIdx].dWriteStartPos > stSock[dCommunicatedIdx].dWriteEndPos)
			k = stSock[dCommunicatedIdx].dWriteStartPos - stSock[dCommunicatedIdx].dWriteEndPos;
		else
			k = MAX_BUF_SIZE - stSock[dCommunicatedIdx].dWriteEndPos;

		if( (t = write(stSock[dCommunicatedIdx].dSfd, &stSock[dCommunicatedIdx].sWriteBuf[stSock[dCommunicatedIdx].dWriteEndPos], k)) <= 0)
		{
			if(errno != EAGAIN)
			{
				stSock[dCommunicatedIdx].dLastFlag = 0;
				dCheck_Channel(stSock[dCommunicatedIdx].dLastFlag, stSock[dCommunicatedIdx].uIPAddr);

				if( (dRet = dDisConnSock(stSock, dCommunicatedIdx, stFD)) < 0)
					log_print(LOGN_CRI, LH"FAILED IN dDisConnSock() dRet[%d]", LT, dRet);

				log_print(LOGN_CRI, LH"FAILED IN write(Sfd[%d] ListenPort[%d]) [errno:%d-%s]", LT,
					stSock[dCommunicatedIdx].dSfd, stSock[dCommunicatedIdx].dListenPort, errno, strerror(errno));
			}
			else if(errno == EAGAIN)
			{
				log_print(LOGN_CRI, LH"SOCK BLOCK [EAGAIN][%s]", LT, strerror(errno));
				log_print(LOGN_DEBUG, LH"SOCK BLOCK SEND SIZE [0]", LT);
			}
		}
		else if(k == t)
		{
			stSock[dCommunicatedIdx].dWriteEndPos += t;
			log_print(LOGN_DEBUG, "SOCK BLOCK SEND SIZE [%ld]", t);

			if(stSock[dCommunicatedIdx].dWriteStartPos == stSock[dCommunicatedIdx].dWriteEndPos)
			{
				stSock[dCommunicatedIdx].dWriteStartPos	= 0;
				stSock[dCommunicatedIdx].dWriteEndPos	= 0;
				FD_CLR(stSock[dCommunicatedIdx].dSfd, (fd_set*)&stFD->Wfds);
			}
			else if (stSock[dCommunicatedIdx].dWriteEndPos == MAX_BUF_SIZE)
				stSock[dCommunicatedIdx].dWriteEndPos = 0;
		}
		else
		{
			stSock[dCommunicatedIdx].dWriteEndPos += t;
			log_print(LOGN_CRI, "SOCK BLOCK [EAGAIN ???]");
			log_print(LOGN_DEBUG, "SOCK BLOCK SEND SIZE [%ld]",t);
		}
	}

	return 0;
}

int dRecvPacket(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD)
{
	ssize_t		szRdBytes;
	char		cFlag;
	int			dRet;

	if( (szRdBytes = read(stSock[dCommunicatedIdx].dSfd, &cFlag, 1)) <= 0)
	{
		if(szRdBytes == 0)
		{
			log_print(LOGN_INFO, LH"Client Sfd[%d] is normally close", LT,
				stSock[dCommunicatedIdx].dSfd);
			return -1;
		}
		else
		{
			log_print(LOGN_CRI, LH"FAILED IN read(dSfd[%d]) szRdBytes[%ld] [errno:%d-%s]", LT,
				stSock[dCommunicatedIdx].dSfd, szRdBytes, errno, strerror(errno));
			if(errno != EAGAIN)
				return -2;
			else
				return 0;
		}
	}
	else
	{
		time(&stSock[dCommunicatedIdx].tLastTime);
		log_print(LOGN_INFO,LH"dSfd[%d] uiIP[%u] dListenPort[%d] tLastTime[%ld]", LT,
			stSock[dCommunicatedIdx].dSfd, stSock[dCommunicatedIdx].uIPAddr, stSock[dCommunicatedIdx].dListenPort, stSock[dCommunicatedIdx].tLastTime);

		switch(cFlag)
		{
			case 'I':
				if( (dRet = dSendHeartBeat(stSock, dCommunicatedIdx, stFD)) < 0)
					log_print(LOGN_CRI, LH"ERROR IN dSendHeartBeat() dRet[%d]", LT, dRet);
				break;
			case 'Y':
				if( (dRet = dHandleStatisticMsg(stSock, dCommunicatedIdx, stFD)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dHandleStatisticMsg(dCommunicatedIdx[%d]) dRet[%d]", LT,
						dCommunicatedIdx, dRet);
					return -3;
				}
				break;
			default:
				if( (dRet = dHandleSocketMsg(stSock, dCommunicatedIdx, stFD, cFlag)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dHandleSocketMsg(dCommunicatedIdx[%d]) dRet[%d]", LT,
						dCommunicatedIdx, dRet);
					return -4;
				}
				break;
		}
	}

	return 0;
}

int dHandleStatisticMsg(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD)
{
	ssize_t		sszTotalRdLen, sszTempRdLen;
	int			dRet, dReceivingCount;
	char		cRdBuf[MAX_BUF_SIZE];

	sszTotalRdLen	= 0;
	memset(cRdBuf, 0x00, MAX_BUF_SIZE);
	for(dReceivingCount = 0; dReceivingCount < MAX_RECEIVING_COUNT; dReceivingCount++)
	{
		if( (sszTempRdLen = read(stSock[dCommunicatedIdx].dSfd, cRdBuf, (MAX_BUF_SIZE-sszTotalRdLen))) < 0)
		{
			log_print(LOGN_CRI, LH"FAILED IN read(dCommunicatedIdx[%d]) [errno:%d-%s]", LT, dCommunicatedIdx, errno, strerror(errno));
			return -1;
		}
		else if(!sszTempRdLen)
		{
			log_print(LOGN_CRI, LH"Client(dCommunicatedIdx[%d]) is abnormally close the socket[%d]", LT, dCommunicatedIdx, stSock[dCommunicatedIdx].dSfd);
			return -2;
		}
		else
		{
			if( (sszTotalRdLen += sszTempRdLen) >= (MAX_STATISTICS_MSG_LEN-1))
			{
				log_print(LOGN_DEBUG, LH"Get a statistics message(sszTotalRdLen[%ld])", LT, sszTotalRdLen);
				break;
			}
			else
			{
				log_print(LOGN_DEBUG, LH"Should get a left statistics message: sszTotalRdLen[%ld] dReceivingCount[%d]", LT, sszTotalRdLen, dReceivingCount);
				continue;
			}
		}
	}

	if( (dReceivingCount == MAX_RECEIVING_COUNT) && (sszTotalRdLen < MAX_STATISTICS_MSG_LEN - 1))
	{
		log_print(LOGN_CRI, LH"dReceivingCount[%d] is over MAX_RECEIVING_COUNT[%d] (ReceivedLength[%ld] is less than (MAX_STATISTICS_MSG_LEN-1[%d])", LT,
			dReceivingCount, MAX_RECEIVING_COUNT, sszTotalRdLen, MAX_STATISTICS_MSG_LEN-1);
		return -3;
	}

	if( (dRet = dResetFileList(cRdBuf, stSock[dCommunicatedIdx].dSfd)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dResetFileList(%s) dRet[%d]", LT, cRdBuf, dRet);
		return -3;
	}

	return 0;
}

int dHandleSocketMsg(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD, char cFlag)
{
	ssize_t			sszTotalRdLen, sszTempRdLen;
	unsigned short	huPacketLen;
	int				dRet, dReceivingCount, dMsgType;
	char			cRdBuf[MAX_BUF_SIZE];
	Header			*pstCommonHd;
	Packet1			stPacket1;
	Packet2			stPacket2;
#ifdef _WARNING_
	Packet3		stPacket3;
#endif	/*	_WARNING_	*/

	memset(cRdBuf, 0x00, MAX_BUF_SIZE);
	memcpy(cRdBuf, &cFlag, sizeof(char));
	sszTotalRdLen	= sizeof(char);
	for(dReceivingCount = 0; dReceivingCount < MAX_RECEIVING_COUNT; dReceivingCount++)
	{
		if( (sszTempRdLen = read(stSock[dCommunicatedIdx].dSfd, &cRdBuf[sszTotalRdLen], CTRL_PKT_HDR_SIZE-sszTotalRdLen)) < 0)
		{
			log_print(LOGN_CRI, LH"FAILED IN read(dCommunicatedIdx[%d]) [errno:%d-%s]", LT, dCommunicatedIdx, errno, strerror(errno));
			return -1;
		}
		else if(!sszTempRdLen)
		{
			log_print(LOGN_CRI, LH"Client(dCommunicatedIdx[%d]) closed the socket abnormally[%d]", LT, dCommunicatedIdx, stSock[dCommunicatedIdx].dSfd);
			return -2;
		}
		else
		{
			if( (sszTotalRdLen += sszTempRdLen) >= CTRL_PKT_HDR_SIZE)
			{
				log_print(LOGN_DEBUG, LH"Get a statistics message(sszTotalRdLen[%ld])", LT, sszTotalRdLen);
				break;
			}
			else
			{
				log_print(LOGN_DEBUG, LH"Should get a left statistics message: sszTotalRdLen[%ld] dReceivingCount[%d]", LT, sszTotalRdLen, dReceivingCount);
				continue;
			}
		}
	}

	if( (dReceivingCount == MAX_RECEIVING_COUNT) && (sszTotalRdLen < CTRL_PKT_HDR_SIZE))
	{
		log_print(LOGN_CRI, LH"dReceivingCount[%d] is over MAX_RECEIVING_COUNT[%d], AND ReceivedLength[%ld] is less than CTRL_PKT_HDR_SIZE[%d]", LT,
			dReceivingCount, MAX_RECEIVING_COUNT, sszTotalRdLen, CTRL_PKT_HDR_SIZE);
		return -3;
	}

	pstCommonHd	= (Header*)cRdBuf;
	huPacketLen = ntohs(pstCommonHd->Len);
	for(dReceivingCount = 0; dReceivingCount < MAX_RECEIVING_COUNT; dReceivingCount++)
	{
		if( (sszTempRdLen = read(stSock[dCommunicatedIdx].dSfd, &cRdBuf[sszTotalRdLen], (huPacketLen-sszTotalRdLen))) < 0)
		{
			log_print(LOGN_CRI, LH"FAILED IN read(dCommunicatedIdx[%d]) [errno:%d-%s]", LT, dCommunicatedIdx, errno, strerror(errno));
			return -4;
		}
		else if(!sszTempRdLen)
		{
			log_print(LOGN_CRI, LH"Client(dCommunicatedIdx[%d]) closed the socket abnormally[%d]", LT, dCommunicatedIdx, stSock[dCommunicatedIdx].dSfd);
			return -5;
		}
		else
		{
			if( (sszTotalRdLen += sszTempRdLen) >= huPacketLen)
			{
				log_print(LOGN_DEBUG, LH"Get a statistics message(sszTotalRdLen[%ld])", LT, sszTotalRdLen);
				break;
			}
			else
			{
				log_print(LOGN_DEBUG, LH"Should get a left statistics message: sszTotalRdLen[%ld] dReceivingCount[%d]", LT, sszTotalRdLen, dReceivingCount);
				continue;
			}
		}
	}

	if( (dReceivingCount == MAX_RECEIVING_COUNT) && (sszTotalRdLen < huPacketLen))
	{
		log_print(LOGN_CRI, LH"Can't receive data body len[%hu] realReceivedBytes[%lu]", LT, huPacketLen, sszTotalRdLen);
		return -6;
	}

	switch(pstCommonHd->hd.Type)
	{
		case CONTROL_PACKET:
			log_print(LOGN_INFO, LH"PACKET CONTROL PACKET TYPE[0x%02X]", LT, pstCommonHd->hd.Type);
			memcpy(&stPacket1, cRdBuf, sizeof(Packet1));
			break;
		case SIMPLE_PACKET:
			memcpy(&stPacket2, cRdBuf, sizeof(Packet2));
			dMsgType = ntohl(stPacket2.MsgType);
			switch(dMsgType)
			{
				case InitialHwAlarmFaultRequest:
					if( (dRet = dSendPrimitive(stSock, dCommunicatedIdx, stFD, InitialHwAlarmFaultConfirm)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendPrimitive(dCommunicatedIdx[%d], InitialHwAlarmFaultConfirm) dRet[%d]", LT, dCommunicatedIdx, dRet);
						return -7;
					}

					if( (dRet = dSendPrimitive(stSock, dCommunicatedIdx, stFD, InitialHwAlarmFaultData)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendPrimitive(dCommunicatedIdx[%d], InitialHwAlarmFaultData) dRet[%d]", LT, dCommunicatedIdx, dRet);
						return -8;
					}
					break;
				case InitialHwAlarmFaultDataReceivedOk:
					log_print(LOGN_INFO, LH"Received a InitialHwAlarmFaultDataReceivedOk[%u] packet", LT, InitialHwAlarmFaultDataReceivedOk);
					break;

				case InitialSwAlarmFaultRequest:
					if( (dRet = dSendPrimitive(stSock, dCommunicatedIdx, stFD, InitialSwAlarmFaultConfirm)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendPrimitive(dCommunicatedIdx[%d], InitialSwAlarmFaultConfirm[%u]) dRet[%d]", LT,
							dCommunicatedIdx, InitialSwAlarmFaultConfirm, dRet);
						return -9;
					}

					if( (dRet = dSendPrimitive(stSock, dCommunicatedIdx, stFD, InitialSwAlarmFaultData)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendPrimitive(dCommunicatedIdx[%d], InitialSwAlarmFaultData[%u]) dRet[%d]", LT,
							dCommunicatedIdx, InitialSwAlarmFaultData, dRet);
						return -10;
					}
					break;
				case InitialSwAlarmFaultDataReceivedOk:
					log_print(LOGN_INFO, LH"Received a InitialSwAlarmFaultDataReceivedOk[%u] packet", LT, InitialSwAlarmFaultDataReceivedOk);
					break;
				case MmcRequest:
					if(huPacketLen-SMPL_PKT_HDR_SIZE == 0)
					{
						log_print(LOGN_CRI, LH"sCommand is empty!", LT);
						return -11;
					}
					else if( (dRet = dDealMMCMsg(stSock, dCommunicatedIdx, stFD, stPacket2.Data, huPacketLen-SMPL_PKT_HDR_SIZE)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dDealMMCMsg(dCommunicatedIdx[%d] SerialNo[%hu] Privilege[%hu] Cmd[%s]) dRet[%d]", LT,
							dCommunicatedIdx, (char)stPacket2.Data[0], (char)stPacket2.Data[1], (char*)&stPacket2.Data[2], dRet);
						return -12;
					}
					break;
				default:
					log_print(LOGN_CRI, LH"Unknown Message Type[%hu]", LT, dMsgType);
					return -13;
			}
			break;
		case COMPLEX_PACKET:
			break;
		default:
			log_print(LOGN_CRI, LH"UNAVAILABLE Message HEADER TYPE[%hu]", LT, pstCommonHd->hd.Type);
			return -13;
	}

	return 0;
}

int dAcceptSockFd(st_NMSSFdInfo *stSock, st_SelectInfo *stFD, int dListenIdx, int *pdPos)
{
	int					dNewSFd, dSndBufLen, dRcvBufLen, dRet, flags;
	unsigned int		uiIP;
	socklen_t			skClientLen;
	struct sockaddr_in	stCliAddr;
	struct linger		new_ld;

	bzero(&stCliAddr, sizeof(struct sockaddr_in));
	skClientLen = sizeof(struct sockaddr_in);

	if( (dNewSFd = accept(stSock[dListenIdx].dSfd, (struct sockaddr*)&stCliAddr, (int*)&skClientLen)) <= 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN accept() [errno:%d-%s]", LT, errno, strerror(errno));
		return -1;
	}

	uiIP		= ntohl(stCliAddr.sin_addr.s_addr);
	dSndBufLen	= MAX_BUF_SIZE*20;
	dRcvBufLen	= MAX_BUF_SIZE*20;
	if(setsockopt(dNewSFd, SOL_SOCKET, SO_RCVBUF, &dRcvBufLen, sizeof(dRcvBufLen)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN setsockopt(SO_RCVBUF) [errno:%d-%s]", LT, errno, strerror(errno));
		close(dNewSFd);
		return -2;
	}

	if(setsockopt(dNewSFd, SOL_SOCKET, SO_SNDBUF, &dSndBufLen, sizeof(dSndBufLen)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN setsockopt(SO_SNDBUF) [errno:%d-%s]", LT, errno, strerror(errno));
		close(dNewSFd);
		return -3;
	}

	if( (flags = fcntl(dNewSFd, F_GETFL, 0)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN fcntl() [errno:%d-%s]", LT, errno, strerror(errno));
		close(dNewSFd);
		return -4;
	}

	flags |= O_NDELAY;
	if(fcntl(dNewSFd, F_SETFL, flags) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN fcntl(O_NDELAY) [errno:%d-%s]", LT, errno, strerror(errno));
		close(dNewSFd);
		return -5;
	}

	new_ld.l_onoff	= 0;
	new_ld.l_linger	= 0;
	if(setsockopt(dNewSFd, SOL_SOCKET, SO_LINGER, (char *)&new_ld, sizeof (new_ld)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN setsockopt(SO_LINGER) [errno:%d-%s]", LT, errno, strerror(errno));
		close(dNewSFd);
		return -6;
	} /* end of if */

	if( (dRet = dAddSockInTable(stSock, dNewSFd, uiIP, stSock[dListenIdx].dListenPort, stFD)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dAddSockInTable() dRet[%d]", LT, dRet);
		FD_CLR(dNewSFd, (fd_set*)&stFD->Rfds);
		close(dNewSFd);
		return -1;
	}

	FD_SET(dNewSFd, (fd_set*)&stFD->Rfds);
    if(dNewSFd >= stFD->dMaxSfd)
        stFD->dMaxSfd = dNewSFd+1;

	*pdPos = dRet;
	log_print(LOGN_DEBUG, LH"CONNECT NEW SFD=%d POS=%d IP=%u:%u", LT, dNewSFd, *pdPos, uiIP, stSock[dRet].uIPAddr);

	return dNewSFd;
}

int dAddSockInTable(st_NMSSFdInfo *stSock, int dSfd, unsigned int uiIP, int dPort, st_SelectInfo *stFD)
{
	char				cIsRightIP;
	int					i;
	socklen_t			skClientLen;
	struct sockaddr_in	scClientAddr;

	if(dPort == stSock[PORT_IDX_STAT].dListenPort)
	{
		for(cIsRightIP = 0, i = 0; i < gstNMSIPList.dCount; i++)
		{
			if(uiIP == gstNMSIPList.stNMSIPInfo[i].uIP)
			{
				cIsRightIP = 1;
				break;
			}
		}

		if(cIsRightIP != 1)
		{
			log_print(LOGN_CRI, LH"NOT STATISTICS uiIP[%u] dPort[%d]", LT, uiIP, dPort);
			return -1;
		}
	}

	for(i = PORT_IDX_MAX; i < MAX_CONN; i++)
	{
		if( (stSock[i].uIPAddr==uiIP) && (stSock[i].dListenPort==dPort))
		{
			log_print(LOGN_CRI, LH"Aleady IP In Socket list Before Client Disconn uIPAddr[%u] dListenPort[%u]", LT, uiIP, dPort);
			dDisConnSock(stSock, i, stFD);
			stSock[i].dSfd				= dSfd;
			stSock[i].dListenPort		= dPort;
			stSock[i].uIPAddr			= uiIP;
			stSock[i].dType				= FD_TYPE_DATA;
			stSock[i].dWriteStartPos	= 0;
			stSock[i].dWriteEndPos		= 0;
			stSock[i].tLastTime			= time(NULL);
			return i;
		}
	}

	for(i = PORT_IDX_MAX; i < MAX_CONN; i++)
	{
		if(stSock[i].dSfd == 0)
		{
			stSock[i].dSfd				= dSfd;
			stSock[i].dListenPort		= dPort;
			if(stSock[i].dListenPort == gstNMSPortInfo.port[PORT_IDX_MMC])
			{
				stSock[i].stMMCInfo.dMMCSfd		= socket(AF_INET, SOCK_STREAM, 0);
				scClientAddr.sin_family			= AF_INET;
				scClientAddr.sin_addr.s_addr	= inet_addr(gstNMSPortInfo.ipaddr[0]);
				scClientAddr.sin_port			= htons(S_PORT_MMCD);

				skClientLen = sizeof(scClientAddr);
				if (connect(stSock[i].stMMCInfo.dMMCSfd, (struct sockaddr*)&scClientAddr, skClientLen) < 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN connect() errno[%d-%s]", LT,
						errno, strerror(errno));
					return -2;
				}
			}
			else
				memset(&stSock[i].stMMCInfo, 0x00, sizeof(st_MMCInfo));

			stSock[i].uIPAddr			= uiIP;
			stSock[i].dType				= FD_TYPE_DATA;
			stSock[i].dWriteStartPos	= 0;
			stSock[i].dWriteEndPos		= 0;
			stSock[i].tLastTime			= time(NULL);
			return i;
		}
	}

	return -1;
}

int dDisConnSock(st_NMSSFdInfo *stSock, int dIdx, st_SelectInfo *stFD)
{
	int		i, dMaxFds = 0;

	if(stSock[dIdx].dLastFlag == 1)
	{
		stSock[dIdx].dLastFlag = 0;
		log_print(LOGN_CRI, LH"IDX[%d] dSfd[%d] CliIP[%u] ListenPort[%d]", LT,
			dIdx, stSock[dIdx].dSfd, stSock[dIdx].uIPAddr, stSock[dIdx].dListenPort);
	}
	else
	{
		log_print(LOGN_CRI, LH"Aleady IDX[%d] dSfd[%d] CliIP[%u] ListenPort[%d]", LT,
			dIdx, stSock[dIdx].dSfd, stSock[dIdx].uIPAddr, stSock[dIdx].dListenPort);
	}

	FD_CLR(stSock[dIdx].dSfd, (fd_set*)&stFD->Rfds);
	FD_CLR(stSock[dIdx].dSfd, (fd_set*)&stFD->Wfds);
	if( (stSock[dIdx].dListenPort == gstNMSPortInfo.port[PORT_IDX_MMC]) && (stSock[dIdx].stMMCInfo.dMMCSfd != 0))
	{
		FD_CLR(stSock[dIdx].stMMCInfo.dMMCSfd, (fd_set*)&stFD->Rfds);
		if(close(stSock[dIdx].stMMCInfo.dMMCSfd) == -1)
		{
			log_print(LOGN_CRI, LH"FAILED IN close(stSock[%d].stMMCInfo.dMMCSfd[%d]) errno[%d-%s]", LT,
				dIdx, stSock[dIdx].stMMCInfo.dMMCSfd, errno, strerror(errno));
			return -1;
		}
	}

	if(close(stSock[dIdx].dSfd) == -1)
	{
		log_print(LOGN_CRI, LH"FAILED IN close(SFD[%d]) [errno:%d-%s]", LT, stSock[dIdx].dSfd, errno, strerror(errno));
		return -2;
	}

	for(i = 0; i < MAX_CONN; i++)
	{
		if(dMaxFds < stSock[i].dSfd)
			dMaxFds = stSock[i].dSfd;

		if(dMaxFds < stSock[i].stMMCInfo.dMMCSfd)
			dMaxFds = stSock[i].stMMCInfo.dMMCSfd;
	}

	if( (dMaxFds+1) != stFD->dMaxSfd)
		stFD->dMaxSfd = dMaxFds + 1;

	stSock[dIdx].tLastTime			= 0;
	stSock[dIdx].cMask				= 0;
	stSock[dIdx].cLevel				= 0;
	stSock[dIdx].dSfd				= 0;
	stSock[dIdx].dListenPort		= 0;
	stSock[dIdx].uIPAddr			= 0;
	stSock[dIdx].dType				= FD_TYPE_DATA;
	stSock[dIdx].dWriteStartPos		= 0;
	stSock[dIdx].dWriteEndPos		= 0;
	stSock[dIdx].dLastFlag			= 0;
	memset(&stSock[dIdx].stMMCInfo, 0x00, sizeof(st_MMCInfo));

	return 0;
}

int dSendHeartBeat(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD)
{
	int		dRet;
	char	buf[MAX_BUF_SIZE];

	memset(buf, 0x00, MAX_BUF_SIZE);

	buf[0] = 'A';
	if( (dRet = write(stSock[dCommunicatedIdx].dSfd, buf, 1)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN write(SFD[%d]) [errno:%d-%s]", LT,
			stSock[dCommunicatedIdx].dSfd, errno, strerror(errno));

		if(errno != EAGAIN)
		{
			stSock[dCommunicatedIdx].dLastFlag = 0;
			dCheck_Channel(stSock[dCommunicatedIdx].dLastFlag, stSock[dCommunicatedIdx].uIPAddr);

			if( (dRet = dDisConnSock(stSock, dCommunicatedIdx, stFD)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dDisConnSock() dRet[%d]", LT, dRet);
		}
		else
			return -1;
	}
	else
	{
		log_print(LOGN_INFO, "SEND Heart-Beat dCommunicatedIdx[%d] IP[%u] FD[%d] FLAG[%hd] TIME[%lu]",
			dCommunicatedIdx, stSock[dCommunicatedIdx].uIPAddr, stSock[dCommunicatedIdx].dSfd, stSock[dCommunicatedIdx].dLastFlag, stSock[dCommunicatedIdx].tLastTime);

		stSock[dCommunicatedIdx].tLastTime = time(NULL);
	}

	return 0;
}

int dHandleFSTATMsgQ(st_NMSSFdInfo *stSock, st_atQueryInfo *pstAtQueryInfo, st_SelectInfo *stFD)
{
	Packet1		*finfo;
	int			i, j, dRet, dSendFlag, dPeriod;
	char		sFileName[PATH_MAX], sBuf[MAX_BUF_SIZE];

	memset(sFileName, 0x00, PATH_MAX);
	memset(sBuf, 0x00, MAX_BUF_SIZE);

	finfo = (Packet1*)sBuf;

	/* 1. DB query
	 * 2. make file
	 */
	if( (dRet = dMakeOIDFile(&stMySQL, pstAtQueryInfo, sFileName)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dMakeOIDFile(sFileName[%s]) dRet[%d]", LT, sFileName, dRet);
		return -1;
	}

	/* make NMS signal (file information)
	*/
	switch(pstAtQueryInfo->cPeriod)
	{
		case STAT_PERIOD_5MIN:
			sprintf(sBuf, "F%s/%s/5MIN/%s", START_PATH, NMS_STATISTICS_DIR, sFileName);
			break;
		case STAT_PERIOD_HOUR:
			sprintf(sBuf, "F%s/%s/HOUR/%s", START_PATH, NMS_STATISTICS_DIR, sFileName);
			break;
		default:
			log_print(LOGN_CRI, LH"Unknown period[%d]", LT, pstAtQueryInfo->cPeriod);
			break;
	}

	/* send signal to NMS (file information)
	*/
	for(i = PORT_IDX_MAX; i < MAX_CONN; i++)
	{
		if( (stSock[i].dSfd>0) && (stSock[i].dType==FD_TYPE_DATA) && (stSock[i].dListenPort==stSock[PORT_IDX_STAT].dListenPort))
		{
			for(dPeriod = -1, j = 0; j < gstNMSIPList.dCount; j++)
			{
				if(gstNMSIPList.stNMSIPInfo[j].uIP == stSock[i].uIPAddr)
				{
					dPeriod = gstNMSIPList.stNMSIPInfo[j].cType;
					break;
				}
			}

			if(dPeriod != pstAtQueryInfo->cPeriod)
				continue;

			dSendFlag	= 1;
			if( (dRet = dSendPacket(stSock, i, stFD, sBuf, MAX_STATISTICS_MSG_LEN+1)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dSendPacket() dRet[%d]", LT, dRet);

			dSaveFileList(sFileName, stSock[i].dSfd, pstAtQueryInfo->cPeriod);
		}
	}

	/* not exist NMS connection
	*/
	if(dSendFlag == 0)
	{
		log_print(LOGN_DEBUG, LH"There is no NMS STATISTICS CONNECTION", LT);
		dSaveFileList(sFileName, 0, pstAtQueryInfo->cPeriod);
	}

	return 0;
}

int dSendPrimitive(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD, int dMsgType)
{
	int			dRet;
	char		sWrBuf[MAX_BUF_SIZE];
#ifdef _WARNING_
	Header		stCommonHd;
	Packet1		*pstPacket1;
#endif	/*	_WARNING_	*/

	Packet2		*pstPacket2;
	Packet3		*pstPacket3;

	memset(sWrBuf, 0x00, MAX_BUF_SIZE);
	switch(dMsgType)
	{
		case InitialHwAlarmFaultConfirm:
		case InitialSwAlarmFaultConfirm:
		case MmcConfirm:
			pstPacket2 = (Packet2*)sWrBuf;

			pstPacket2->hdr.hd.Ver		= 0;
			pstPacket2->hdr.hd.CtrlBit	= 0;
			pstPacket2->hdr.hd.Type		= SIMPLE_PACKET;
			pstPacket2->hdr.hd.CtrlSeq	= 0;
			pstPacket2->hdr.hd.Src		= 0x02;		/*	CHECK THIS FIELD???	*/
			pstPacket2->hdr.hd.Dest		= 0x01;		/*	NMS		*/
			pstPacket2->hdr.Prmt		= 0;
			pstPacket2->hdr.Len			= htons(SMPL_PKT_HDR_SIZE);
			pstPacket2->MsgType			= htonl(dMsgType);

			if( (dRet = dSendPacket(stSock, dCommunicatedIdx, stFD, sWrBuf, SMPL_PKT_HDR_SIZE)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dSendPacket() dRet[%d]", LT, dRet);
			break;

		case InitialHwAlarmFaultData:
		case InitialSwAlarmFaultData:
			pstPacket3 = (Packet3*)sWrBuf;

			pstPacket3->hdr.hd.Ver		= 0;
			pstPacket3->hdr.hd.CtrlBit	= 0;
			pstPacket3->hdr.hd.Type		= COMPLEX_PACKET;
			pstPacket3->hdr.hd.CtrlSeq	= 0;
			pstPacket3->hdr.hd.Src		= 0x02;		/*	CHECK THIS FIELD???	*/
			pstPacket3->hdr.hd.Dest		= 0x01;		/*	NMS		*/
			pstPacket3->hdr.Prmt		= 0;
			pstPacket3->hdr.Len			= htons(CMPL_PKT_HDR_SIZE);

			if(dMsgType == InitialHwAlarmFaultData)
				pstPacket3->MsgType			= htonl(InitialHwAlarmFaultDataEnd);
			else
				pstPacket3->MsgType			= htonl(InitialSwAlarmFaultDataEnd);

			if( (dRet = dSendPacket(stSock, dCommunicatedIdx, stFD, sWrBuf, SMPL_PKT_HDR_SIZE)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dSendPacket() dRet[%d]", LT, dRet);
			break;
		default:
			break;
	}

	return 0;
}

int dHandleCONDMsgQ(st_NMSSFdInfo *stSock, st_SelectInfo *stFD, char *psData, size_t szStrLen)
{
	int			i, dRet;
	char		sWrBuf[MAX_BUF_SIZE];
	Packet2		*pstPacket2;

	memset(sWrBuf, 0x00, MAX_BUF_SIZE);
	pstPacket2	= (Packet2*)sWrBuf;

	strncpy(pstPacket2->Data, psData, szStrLen);

	pstPacket2->hdr.hd.Ver		= 0;
	pstPacket2->hdr.hd.CtrlBit	= 0;
	pstPacket2->hdr.hd.Type		= 2;		/*	packet2	*/
	pstPacket2->hdr.hd.CtrlSeq	= 0;
	pstPacket2->hdr.hd.Src		= 0x02;		/*	CHECK THIS FIELD???	*/
	pstPacket2->hdr.hd.Dest		= 0x01;		/*	NMS		*/
	pstPacket2->hdr.Prmt		= 0;
	pstPacket2->hdr.Len			= htons(SMPL_PKT_HDR_SIZE+szStrLen);
	pstPacket2->MsgType			= htonl(ALARM_FAULT);

	for(i = PORT_IDX_MAX; i < MAX_CONN; i++)
	{
		if( (stSock[i].dSfd>0) && (stSock[i].dType==FD_TYPE_DATA) && (stSock[i].dListenPort==stSock[PORT_IDX_CONS].dListenPort))
		{
			if( (dRet = dSendPacket(stSock, i, stFD, sWrBuf, SMPL_PKT_HDR_SIZE+szStrLen)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dSendPacket() dRet[%d]", LT, dRet);
		}
	}

	return 0;
}

int dHandleALMDMsgQ(st_NMSSFdInfo *stSock, st_SelectInfo *stFD, st_almsts *pstAlmStatus)
{
	int			i, dRet;
	char		sWrBuf[MAX_BUF_SIZE], sAlias[64], sBuf[MAX_BUF_SIZE];
	st_NTafName	stNTafNames[MAX_NTAF_COUNT];
	Packet3		*pstPacket3;
	size_t		szLen;
	struct tm	stTmAlarm;

	memset(sWrBuf, 0x00, MAX_BUF_SIZE);
	pstPacket3	= (Packet3*)sWrBuf;

	pstPacket3->hdr.hd.Ver		= 0;
	pstPacket3->hdr.hd.CtrlBit	= 0;
	pstPacket3->hdr.hd.Type		= 3;		/*	packet3	*/
	pstPacket3->hdr.hd.CtrlSeq	= 0;
	pstPacket3->hdr.hd.Src		= 0x02;		/*	CHECK THIS FIELD???	*/
	pstPacket3->hdr.hd.Dest		= 0x01;		/*	NMS		*/
	pstPacket3->hdr.Prmt		= 0;
	pstPacket3->MsgType			= htonl(ALARM_FAULT);

	memset(stNTafNames, 0x00, sizeof(st_NTafName)*MAX_NTAF_COUNT);
	if( (dRet = dReadNTAFName(stNTafNames)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN dReadNTAFName() dRet[%d]", LT, dRet);
		return -1;
	}

	szLen = 0;
	switch(pstAlmStatus->ucSysType)
	{
		//case SYSTYPE_NTAM:
		case SYSTYPE_TAM:
			sprintf(sAlias, "%s", "TAM_APP");
			sprintf(&pstPacket3->Attr[szLen], "10001:C:%s:", sAlias);
			szLen = strlen(pstPacket3->Attr);
			break;
		//case SYSTYPE_NTAF:
		case SYSTYPE_TAF:
			for(i = 0; i < dRet; i++)
			{
				if(stNTafNames[i].dSysNo == pstAlmStatus->ucSysNo)
				{
					sprintf(sAlias, "%s", stNTafNames[i].sNTAFName);
					sprintf(&pstPacket3->Attr[szLen], "%d:C:%s:", 10002+i, sAlias);
					szLen = strlen(pstPacket3->Attr);
				}
			}
			break;
	}

	if(pstAlmStatus->ucLocType == LOCTYPE_PROCESS)
	{
		//if(pstAlmStatus->ucSysType == SYSTYPE_NTAF)
		if(pstAlmStatus->ucSysType == SYSTYPE_TAF)
		{
			if(pstAlmStatus->ucSysNo <= MAX_NTAF_RP_NO)
				sprintf (&pstPacket3->Attr[szLen], "10040:C:A%hu%hu%02hu:", pstAlmStatus->ucLocType, pstAlmStatus->ucSysType, pstAlmStatus->ucInvNo+1);
			else if(pstAlmStatus->ucSysNo <= MAX_NTAF_PI_NO)
				sprintf (&pstPacket3->Attr[szLen], "10040:C:A%hu%hu%02hu:", pstAlmStatus->ucLocType, pstAlmStatus->ucSysType+1, pstAlmStatus->ucInvNo+1);
		}
		else
			sprintf (&pstPacket3->Attr[szLen], "10040:C:A%hu%hu%02hu:", pstAlmStatus->ucLocType, pstAlmStatus->ucSysType, pstAlmStatus->ucInvNo+1);
	}
	else if( (pstAlmStatus->ucLocType == LOCTYPE_PHSC) &&
		(((pstAlmStatus->ucSysType == SYSTYPE_DIRECT) && (pstAlmStatus->ucInvType == INVTYPE_POWER_DIRECTOR)) ||
		((pstAlmStatus->ucSysType == SYSTYPE_SWITCH) && (pstAlmStatus->ucInvType == INVTYPE_PORT_SWITCH))))
		sprintf (&pstPacket3->Attr[szLen], "10040:C:A%hu%hu%hu:", pstAlmStatus->ucLocType, pstAlmStatus->ucSysType, pstAlmStatus->ucInvType);
	else
		sprintf (&pstPacket3->Attr[szLen], "10040:C:A%hu%hu%hu0:", pstAlmStatus->ucLocType, pstAlmStatus->ucSysType, pstAlmStatus->ucInvType);
	szLen = strlen(pstPacket3->Attr);

	switch(pstAlmStatus->ucAlmLevel)
	{
		case CRITICAL:
			sprintf (&pstPacket3->Attr[szLen], "10042:I:%d:", 1);
			szLen = strlen(pstPacket3->Attr);
			break;
		case MAJOR:
			sprintf (&pstPacket3->Attr[szLen], "10042:I:%d:", 2);
			szLen = strlen(pstPacket3->Attr);
			break;
		case MINOR:
			sprintf (&pstPacket3->Attr[szLen], "10042:I:%d:", 3);
			szLen = strlen(pstPacket3->Attr);
			break;
		case STOP:
			sprintf (&pstPacket3->Attr[szLen], "10042:I:%d:", 4);
			szLen = strlen(pstPacket3->Attr);
			break;
		case NORMAL:
			sprintf (&pstPacket3->Attr[szLen], "10042:I:%d:", 5);
			szLen = strlen(pstPacket3->Attr);
			break;
	}

	memset(sBuf, 0x00, MAX_BUF_SIZE);
	GetAlarmStr(sAlias, pstAlmStatus->ucLocType, pstAlmStatus->ucInvType, sBuf);
	sprintf(&pstPacket3->Attr[szLen], "10041:C:%s:", sBuf);
	szLen = strlen(pstPacket3->Attr);

	localtime_r(&pstAlmStatus->tWhen, &stTmAlarm);
	sprintf (&pstPacket3->Attr[szLen], "10043:C:%04d%02d%02d%02d%02d%02d:",
		stTmAlarm.tm_year+1900, stTmAlarm.tm_mon+1, stTmAlarm.tm_mday, stTmAlarm.tm_hour, stTmAlarm.tm_min, stTmAlarm.tm_sec);
	szLen = strlen(pstPacket3->Attr);

	pstPacket3->hdr.Len			= htons(CMPL_PKT_HDR_SIZE+szLen);
	for(i = PORT_IDX_MAX; i < MAX_CONN; i++)
	{
		if( (stSock[i].dSfd>0) && (stSock[i].dType==FD_TYPE_DATA) && (stSock[i].dListenPort==stSock[PORT_IDX_ALM].dListenPort))
		{
			if( (dRet = dSendPacket(stSock, i, stFD, sWrBuf, CMPL_PKT_HDR_SIZE+szLen)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dSendPacket() dRet[%d]", LT, dRet);
		}
	}

	return 0;
}

int dDealMMCMsg(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD, char *sCommand, int dCmdLen)
{
	unsigned char	cSerialNo, cPrivilege, sUserName[MAX_USER_NAME_LEN], sMtime[MAX_MTIME_LEN];
	unsigned char	sResponse[MAX_BUF_SIZE-SMPL_PKT_HDR_SIZE-1], sWrBuf[MAX_BUF_SIZE];
	unsigned short	huBodyLen;
	st_MngPkt		stMngPkt;
	int				dRet;
	Packet2			*pstPacket2;

	memcpy(&cSerialNo, &sCommand[0], sizeof(char));
	memcpy(&cPrivilege, &sCommand[1], sizeof(char));

	memset(sWrBuf, 0x00, MAX_BUF_SIZE);

	if(strlen(stSock[dCommunicatedIdx].stMMCInfo.sAdminName) == 0)
	{
		if(strcasestr(&sCommand[2], "user-login") != NULL)
		{
			if( (dRet = sscanf(&sCommand[2], "%*s %s", sUserName)) == 1)
			{
				strtok(sUserName, ",");
				log_print(LOGN_DEBUG, LH"sUserName[%s]", LT, sUserName);
			}
			else
			{
				log_print(LOGN_CRI, LH"FAILED IN sscanf() dRet[%d] errno[%d-%s]", LT,
					dRet, errno, strerror(errno));
				return -1;
			}

			memset(&stMngPkt, 0x00, MNG_PKT_HEAD_SIZE+MAX_MMCD_MSG_SIZE);
			stMngPkt.head.llMagicNumber	= MAGIC_NUMBER;
			stMngPkt.head.llIndex		= stSock[dCommunicatedIdx].stMMCInfo.gllTid++;
			stMngPkt.head.usResult		= 0;
			stMngPkt.head.usSrcProc		= 1;

			memset(sMtime, 0x00, MAX_MTIME_LEN);
			mtime2(sMtime);
			sprintf(stMngPkt.head.TimeStamp, "%s", sMtime);

			sprintf(stMngPkt.data, "%s", &sCommand[2]);

			stMngPkt.head.usBodyLen	= strlen(stMngPkt.data);
			stMngPkt.head.usTotLen	= strlen(stMngPkt.data)+MNG_PKT_HEAD_SIZE;
			strcpy(stMngPkt.head.userName, sUserName);

			if( (dRet = dSendMessage(stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd, stMngPkt.head.usTotLen, (char*)&stMngPkt)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMessage(%s[%d]) dRet[%d]", LT,
					stMngPkt.data, stMngPkt.head.usBodyLen, dRet);
				return -2;
			}
			else
			{
				FD_SET(stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd, (fd_set*)&stFD->Rfds);

				if(stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd >= stFD->dMaxSfd)
					stFD->dMaxSfd = stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd+1;

				stSock[dCommunicatedIdx].stMMCInfo.cSerialNo = cSerialNo;
			}
		}
		else
		{
			pstPacket2 = (Packet2*)sWrBuf;

			pstPacket2->hdr.hd.Ver		= 0;
			pstPacket2->hdr.hd.CtrlBit	= 0;
			pstPacket2->hdr.hd.Type		= SIMPLE_PACKET;	/*	packet2	*/
			pstPacket2->hdr.hd.CtrlSeq	= 0;
			pstPacket2->hdr.hd.Src		= 0x02;				/*	CHECK THIS FIELD???	*/
			pstPacket2->hdr.hd.Dest		= 0x01;				/*	NMS		*/
			pstPacket2->hdr.Prmt		= 0;
			pstPacket2->MsgType			= ntohl(MmcConfirm);
			memcpy(&pstPacket2->Data[0], &cSerialNo, sizeof(char));
			strcpy(sResponse, "You should login first.\nCommand: user-login [UserName],[Password]");
			memcpy(&pstPacket2->Data[1], &sResponse, strlen(sResponse));

			huBodyLen					= SMPL_PKT_HDR_SIZE+strlen(sResponse)+sizeof(char);
			pstPacket2->hdr.Len			= htons(huBodyLen);

			if( (dRet = dSendPacket(stSock, dCommunicatedIdx, stFD, (char*)pstPacket2, (int)huBodyLen)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendPacket() dRet[%d]", LT, dRet);
				return -3;
			}
		}
	}
	else
	{
		memset(&stMngPkt, 0x00, MNG_PKT_HEAD_SIZE+MAX_MMCD_MSG_SIZE);

		stMngPkt.head.llMagicNumber	= MAGIC_NUMBER;
		stMngPkt.head.llIndex		= stSock[dCommunicatedIdx].stMMCInfo.gllTid++;
		stMngPkt.head.usResult		= 0;
		stMngPkt.head.usSrcProc		= 1;
		memset(sMtime, 0x00, MAX_MTIME_LEN);
		mtime2(sMtime);
		sprintf(stMngPkt.head.TimeStamp, "%s", sMtime);
		sprintf(stMngPkt.data, "%s", &sCommand[2]);

		stMngPkt.head.usBodyLen	= strlen(stMngPkt.data);
		stMngPkt.head.usTotLen	= strlen(stMngPkt.data)+MNG_PKT_HEAD_SIZE;

		strcpy(stMngPkt.head.userName, stSock[dCommunicatedIdx].stMMCInfo.sAdminName);
		if( (dRet = dSendMessage(stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd, (int)stMngPkt.head.usTotLen, (char*)&stMngPkt)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMessage(%s[%d]) dRet[%d]", LT,
				stMngPkt.data, stMngPkt.head.usBodyLen, dRet);
			return -4;
		}
		else
		{
			FD_SET(stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd, (fd_set*)&stFD->Rfds);

			if(stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd >= stFD->dMaxSfd)
				stFD->dMaxSfd = stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd+1;

			stSock[dCommunicatedIdx].stMMCInfo.cSerialNo = cSerialNo;
		}
	}

	return 0;
}

int dSendMessage(int dSfd, int dMsgLen, char *sSdMsg)
{
	int		dRet;

	if( (dRet = write(dSfd, &sSdMsg[0], dMsgLen)) < 0)
	{
		if(errno == EAGAIN)
		{
			log_print(LOGN_CRI, LH"ERROR IN write() errno[%s-%s]", LT, "EAGAIN", strerror(errno));
			return 0;
		}
		log_print(LOGN_CRI, LH"ERROR IN write() errno[%d-%s]", LT, errno, strerror(errno));
		return -1;
	}
	else if(dRet != dMsgLen)
	{
		log_print(LOGN_CRI, LH"Write byte number[%d] is not equal to MsgLen[%d]", LT, dRet, dMsgLen);
		return -2;
	}

	return 0;
}

void mtime2(char *sMtime)
{
	time_t	tNow;

	tNow = time(NULL);
	strftime(sMtime, 80, "%Y-%m-%d %T %a", localtime(&tNow));
	sMtime[21]	= toupper(sMtime[21]);
	sMtime[22]	= toupper(sMtime[22]);
}

int dRecvMMCResponse(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD)
{
	ssize_t			sszTotalRdLen, sszTempRdLen;
	int				dRet, dTry;
	unsigned char	sRdBuf[MNG_PKT_HEAD_SIZE+MAX_MNGPKT_BODY_SIZE];
	unsigned short	huPacketLen;
	st_MngHead		*pstMngHead;
	st_MngPkt		*pstMngPkt;
	Packet2			stPacket2;

	while(TRUE)
	{
		sszTotalRdLen	= 0;
		sszTempRdLen	= 0;
		memset(&sRdBuf, 0x00, MNG_PKT_HEAD_SIZE+MAX_MNGPKT_BODY_SIZE);
		for(dTry = 0; dTry < MAX_RECEIVING_COUNT; dTry++)
		{
			if( (sszTempRdLen = read(stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd, &sRdBuf[sszTotalRdLen], MNG_PKT_HEAD_SIZE-sszTotalRdLen)) <= 0)
			{
				if(sszTempRdLen == 0)
				{
					log_print(LOGN_INFO, LH"Client(stSock[%d].stMMCInfo.dMMCSfd[%d]) closed Abnormally", LT,
						dCommunicatedIdx, stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd);
					return -1;
				}
				else
				{
					log_print(LOGN_CRI, LH"FAILED IN read(stSock[%d].stMMCInfo.dMMCSfd[%d]) sszTempRdLen[%lu] [errno:%d-%s]", LT,
						dCommunicatedIdx, stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd, sszTempRdLen, errno, strerror(errno));
					if(errno != EAGAIN)
						return -2;
					else
						return 0;
				}
			}
			else
			{
				if( (sszTotalRdLen+=sszTempRdLen) < MNG_PKT_HEAD_SIZE)
					continue;
				else
					break;
			}
		}

		if( (dTry == MAX_RECEIVING_COUNT) && (sszTotalRdLen < MNG_PKT_HEAD_SIZE))
		{
			log_print(LOGN_CRI, LH"read count[%d] is over MAX_RECEIVING_COUNT[%d], AND received bytes[%lu] is less than MNG_PKT_HEAD_SIZE[%lu]", LT,
				dTry, MAX_RECEIVING_COUNT, sszTotalRdLen, MNG_PKT_HEAD_SIZE);
			return -3;
		}

		pstMngHead	= (st_MngHead*)sRdBuf;
		if(pstMngHead->usTotLen > MNG_PKT_HEAD_SIZE)
		{
			for(dTry = 0; dTry < MAX_RECEIVING_COUNT; dTry++)
			{
				if( (sszTempRdLen = read(stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd, &sRdBuf[sszTotalRdLen], pstMngHead->usTotLen-sszTotalRdLen)) <= 0)
				{
					if(sszTempRdLen == 0)
					{
						log_print(LOGN_INFO, LH"Client(stSock[%d].stMMCInfo.dMMCSfd[%d]) closed Abnormally", LT,
							dCommunicatedIdx, stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd);
						return -4;
					}
					else
					{
						log_print(LOGN_CRI, LH"FAILED IN read(stSock[%d].stMMCInfo.dMMCSfd[%d]) sszTempRdLen[%lu] [errno:%d-%s]", LT,
							dCommunicatedIdx, stSock[dCommunicatedIdx].stMMCInfo.dMMCSfd, sszTempRdLen, errno, strerror(errno));
						if(errno != EAGAIN)
							return -5;
						else
							return 0;
					}
				}
				else
				{
					if( (sszTotalRdLen+=sszTempRdLen) < pstMngHead->usTotLen)
						continue;
					else
						break;
				}
			}

			if( (dTry == MAX_RECEIVING_COUNT) && (sszTotalRdLen < pstMngHead->usTotLen))
			{
				log_print(LOGN_CRI, LH"read count[%d] is over MAX_RECEIVING_COUNT[%d], AND received bytes[%lu] is less than pstMngHead->usTotLen[%u]", LT,
					dTry, MAX_RECEIVING_COUNT, sszTotalRdLen, pstMngHead->usTotLen);
				return -6;
			}

			pstMngPkt	= (st_MngPkt*)sRdBuf;
			memset(&stPacket2, 0x00, MAX_BUF_SIZE);

			if( (strlen(stSock[dCommunicatedIdx].stMMCInfo.sAdminName) == 0) && (strcmp(pstMngPkt->data, "SUCCESS") == 0))
				sprintf(stSock[dCommunicatedIdx].stMMCInfo.sAdminName, "%s", pstMngPkt->head.userName);

			stPacket2.hdr.hd.Type		= SIMPLE_PACKET;		/*	packet2	*/

			if(pstMngPkt->head.usTotPage != pstMngPkt->head.usCurPage)
				stPacket2.hdr.hd.CtrlBit	= 1;

			stPacket2.hdr.hd.Ver		= 0;
			stPacket2.hdr.hd.CtrlSeq	= (unsigned char)pstMngPkt->head.usCurPage-1;
			stPacket2.hdr.hd.Src		= 0x02;					/*	CHECK THIS FIELD???	*/
			stPacket2.hdr.hd.Dest		= 0x01;					/*	NMS		*/
			stPacket2.hdr.Prmt			= 0;
			stPacket2.MsgType			= htonl(MmcConfirm);
			memcpy(&stPacket2.Data[0], &stSock[dCommunicatedIdx].stMMCInfo.cSerialNo, sizeof(char));
			memcpy(&stPacket2.Data[1], pstMngPkt->data, pstMngPkt->head.usBodyLen);
			huPacketLen					= SMPL_PKT_HDR_SIZE+pstMngPkt->head.usBodyLen+sizeof(char);
			stPacket2.hdr.Len			= htons(huPacketLen);

			if( (dRet = dSendPacket(stSock, dCommunicatedIdx, stFD, (char*)&stPacket2, huPacketLen)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendPacket() dRet[%d]", LT, dRet);
				return -7;
			}

			log_print(LOGN_DEBUG, LH" MMCD response usCurPage[%hu] usTotPage[%hu]", LT,
				pstMngPkt->head.usCurPage, pstMngPkt->head.usTotPage);

			if(pstMngPkt->head.usTotPage == pstMngPkt->head.usCurPage)
				break;
		}
		else
		{
			log_print(LOGN_DEBUG, LH" MMCD Packet size[%u] is equal to MNG_PKT_HEAD_SIZE[%ld]", LT,
				pstMngHead->usTotLen, MNG_PKT_HEAD_SIZE);
			break;
		}
 	}

	return 0;
}
