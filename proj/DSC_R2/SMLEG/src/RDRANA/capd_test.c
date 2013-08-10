/* 
    $Id: capd_test.c,v 1.1.1.1 2011/04/19 14:13:46 june Exp $

    DATE        : 2006.4.18
    FILE_NAME   : 


    Copyright (c) 2005-2006 by uPRESTO Inc, Korea
    All rights reserved.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include <utillib.h>
#include <Analyze_Ext_Abs.h>
#include "eth_capd.h"

#include "mems.h"
#include "nifo.h"


int				gdSockfd;
int				gdMaxfd;
unsigned int	guiMyIP;

//extern int 				dANAQid;

extern stMEMSINFO 		*pstMEMSINFO;
extern UCHAR  			*pstBuffer;
extern UCHAR 			*pstNode;
extern UCHAR 			*pstTLVNode;

extern U64      nifo_create;
extern U64      nifo_del;

int dUdpSockInit(unsigned short usPort, int *pdSock)
{
	int 				dRet;
	int					dSockfd;
	struct sockaddr_in 	stServAddr;
	int bufsize;

	dSockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(dSockfd < 0)
	{
		dAppLog(LOG_CRI, "socket Error :%s", strerror(errno));
		return -1; 	
	}	

	dRet = fcntl(dSockfd, F_SETFD, O_NONBLOCK);	
	if(dRet < 0)
	{
		dAppLog(LOG_CRI, "fcntl Error :%s", strerror(errno));
		return -2; 	
	}

	bufsize = 1000000;
	dRet = setsockopt(dSockfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
	if( dRet < 0 ) {
		dAppLog(LOG_CRI, "setsockopt() RCV BUFFER set error :%s", strerror(errno));
		return -3; 	
	}

	memset(&stServAddr, 0x00, sizeof(struct sockaddr));

	stServAddr.sin_family = AF_INET;
	stServAddr.sin_addr.s_addr = htonl(guiMyIP);
	stServAddr.sin_port = htons(usPort);

	dRet = bind(dSockfd, (struct sockaddr *)&stServAddr, sizeof(stServAddr));
	if(dRet < 0)
	{
		dAppLog(LOG_CRI, "Bind Error :%s", strerror(errno));
		return -3; 	
	}

	dAppLog(LOG_DEBUG, "SUCC SOCK PORT[%hu]", usPort);

	*pdSock = dSockfd;
	
	return 0;
} 
#if 0
int dIsRcvedPacket()
{
    int     nSel, dFromLen, dRcvSize, dRet;
	int		dDataSize;

	int					tmp;
    fd_set  			Rd;
    struct sockaddr_in  stFrom;
    struct timeval 		stTimeOut;
	T_CAPHDR			*pstCAPHead;

	U8		szBuf[BUFSIZ];

    dFromLen = sizeof(stFrom);

    stTimeOut.tv_sec = 0;
    stTimeOut.tv_usec = 100;

    FD_ZERO(&Rd);
    FD_SET(gdSockfd, &Rd);

    nSel = select(gdMaxfd+1, &Rd, NULL, NULL, &stTimeOut);
    if(nSel > 0)
    {
        if(FD_ISSET(gdSockfd, &Rd))
        {
            dRcvSize = recvfrom(gdSockfd, szBuf, BUFSIZ, 0, (struct sockaddr*)&stFrom, &dFromLen);
            if(dRcvSize < 0) {
                dAppLog(LOG_CRI, "recvfrom REASON[%s]", strerror(errno));
                return -1;
            }
            else if(dRcvSize == 0)
                return 0;
            else
            {
#ifdef DEF_FCS
				/* FCS(4)의 사이즈를 뺀다 */
				dDataSize = dRcvSize - 12 - 4;
#else	
				/* struct sim_header(12) 만큼의 사이즈를 뺀다. */
				dDataSize = dRcvSize - 12;
#endif
				dAppLog(LOG_INFO, "[%s.%d] RECEIVE PACKET SIZE[%d]", __FUNCTION__, __LINE__, dDataSize);

				if((pstNode = nifo_node_alloc(pstMEMSINFO)) == NULL) {	
					dAppLog(LOG_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
					return -2;
				}

				if((pstBuffer = nifo_tlv_alloc(pstMEMSINFO, pstNode, ETH_DATA_NUM, dDataSize, DEF_MEMSET_OFF)) == NULL) {
					dAppLog(LOG_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
					return -3;
				}

				memcpy(pstBuffer, &szBuf[12], dDataSize);

				if((pstCAPHead = (T_CAPHDR *)nifo_tlv_alloc(pstMEMSINFO, pstNode, CAP_HEADER_NUM, CAP_HDR_SIZE, DEF_MEMSET_OFF)) == NULL) {
					dAppLog(LOG_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
					return -4;
				}

				memcpy(&tmp, &szBuf[0], 4);
				pstCAPHead->curtime = ntohl(tmp);
				//pcaphead->curtime = tmp;
				memcpy(&tmp, &szBuf[4], 4);
				pstCAPHead->ucurtime = ntohl(tmp);
				//pcaphead->ucurtime = tmp;
				memcpy(&tmp, &szBuf[8], 4);
				pstCAPHead->bRtxType = ntohl(tmp);
				//pcaphead->bRtxType = tmp;
				pstCAPHead->datalen = dDataSize;

// dAppLog(LOG_INFO, "CTIME[%lu]MTIME[%lu]RTX[%d]LEN[%d]OFFSET[%d]", 
//pcaphead->curtime, pcaphead->ucurtime,
//pcaphead->bRtxType, pcaphead->datalen, nifo_offset(pstMEMSINFO, pnode));			

				if((dRet = nifo_msg_write(pstMEMSINFO, dANAQid, pstNode)) < 0) {
					dAppLog(LOG_CRI, "[%s][%s.%d] nifo_msg_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
					return -5;
				} 
/*
				if((dRet = dSend_CAPD_Data(pstMEMSINFO, myqid, pnode, pcaphead->curtime)) < 0) {
					dAppLog(LOG_CRI, "nifo_msg_write fail [%d][%s]", dRet, strerror(-dRet));
					return -5;
				}
*/
				
            }

        }
    }
    else if(nSel < 0) {
        if (nSel == -1) {
            if (errno == EINTR)
                return 0;
        }
        return -1;
    }

    return 0;
}
#endif
void sigtest(int sig) 
{
#ifdef MEM_TEST
	dAppLog(LOG_CRI, "CREATE CNT[%llu] DELETE CNT[%llu]", nifo_create, nifo_del);
#endif
	close(gdSockfd);
	exit(0);
}

void test_func(char *ip, unsigned short usPort)
{
	dAppLog(LOG_CRI, "CAPD TEST START");
  	signal(SIGINT, sigtest);

	guiMyIP = ntohl(inet_addr(ip));

	if(dUdpSockInit(usPort, &gdSockfd) < 0) {
		dAppLog(LOG_CRI, "[%s.%d] dUdpSockInit", __FUNCTION__, __LINE__);
		exit(0);
	}

	gdMaxfd = gdSockfd;

	while(1) {
		if(dIsRcvedPacket() < 0)
			exit(0);
	}
}

/*
	$Log: capd_test.c,v $
	Revision 1.1.1.1  2011/04/19 14:13:46  june
	성능 패키지
	
	Revision 1.1.1.1  2011/01/20 12:18:54  june
	DSC CVS RECOVERY
	
	Revision 1.1  2009/07/19 06:25:00  jjinri
	RDRANA CSV PARSING VERSION
	
	Revision 1.3  2009/05/29 09:30:04  dsc
	Insert_Block 중복 코드 제거
	
	Revision 1.2  2009/04/29 05:04:03  june
	20090429
	
	Revision 1.1  2009/04/25 12:15:16  june
	rdrana
	
	Revision 1.1.1.1  2009/04/06 13:02:06  june
	LGT DSC project init
	
	Revision 1.1.1.1  2009/04/06 09:10:25  june
	LGT DSC project start
	
	Revision 1.2  2009/02/23 08:30:07  jwkim96
	*** empty log message ***
	
	Revision 1.1.1.1  2008/12/30 02:33:51  upst_cvs
	BSD R3.0.0
	
	Revision 1.2  2008/10/20 01:01:15  jsyoon
	*** empty log message ***
	
	Revision 1.1  2008/10/16 12:59:35  jsyoon
	Add Test Function
	
	Revision 1.2  2008/07/07 13:59:20  jsyoon
	시뮬레이트할때 버퍼링하지 않게 수정
	
	Revision 1.1.1.1  2008/06/09 08:17:17  jsyoon
	WATAS3 PROJECT START
	
	Revision 1.2  2007/08/27 14:00:10  dark264sh
	*** empty log message ***
	
	Revision 1.1  2007/08/21 12:55:29  dark264sh
	no message
	
	Revision 1.8  2006/11/22 07:30:55  dark264sh
	*** empty log message ***
	
	Revision 1.7  2006/11/08 14:46:43  cjlee
	*** empty log message ***
	
	Revision 1.6  2006/11/08 14:45:21  cjlee
	*** empty log message ***
	
	Revision 1.5  2006/11/06 07:32:29  dark264sh
	nifo NODE size 4*1024 => 6*1024로 변경하기
	nifo_tlv_alloc에서 argument로 memset할지 말지 결정하도록 수정
	nifo_node_free에서 semaphore 삭제
	
	Revision 1.4  2006/11/02 05:40:57  dark264sh
	*** empty log message ***
	
	Revision 1.3  2006/10/20 10:04:07  dark264sh
	*** empty log message ***
	
	Revision 1.2  2006/10/18 08:53:31  dark264sh
	nifo debug 코드 추가
	
	Revision 1.1.1.1  2006/10/11 07:45:49  dark264sh
	no message
	
	Revision 1.8  2006/10/10 07:00:30  dark264sh
	A_CALL에 전송하는 부분 추가
	nifo_node_alloc 함수 변경에 따른 변경
	A_TCP에서 timerN_update의 리턴으로 timerNID 업데이트 하도록 변경
	
	Revision 1.7  2006/10/04 01:49:43  dark264sh
	에러 체크 추가
	
	Revision 1.5  2006/09/13 05:25:55  dark264sh
	UDP로 테스트 데이터를 받아서 처리 하는 과정에서 DataSize를
	잘못 처리한 부분 수정
	
	Revision 1.4  2006/09/13 04:30:25  dark264sh
	strerror 잘못 찍는 부분 수정
	
	Revision 1.3  2006/09/11 09:05:31  dark264sh
	*** empty log message ***
	
	Revision 1.2  2006/09/11 05:50:11  dark264sh
	*** empty log message ***
	
	Revision 1.1  2006/09/11 05:10:54  dark264sh
	no message
	
*/
