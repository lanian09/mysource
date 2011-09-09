/* 
    $Id: capd_test.c,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $

    DATE        : 2006.4.18
    FILE_NAME   : 


    Copyright (c) 2005-2006 by uPRESTO Inc, Korea
    All rights reserved.
*/

#include "capd_test.h"

int dUdpSockInit(unsigned short usPort, int *pdSock)
{
	int 				dRet;
	int					dSockfd;
	struct sockaddr_in 	stServAddr;

	dSockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(dSockfd < 0)
	{
		log_print(LOGN_CRI, "socket Error :%s", strerror(errno));
		return -1; 	
	}	

	dRet = fcntl(dSockfd, F_SETFD, O_NONBLOCK);	
	if(dRet < 0)
	{
		log_print(LOGN_CRI, "fcntl Error :%s", strerror(errno));
		return -2; 	
	}

	memset(&stServAddr, 0x00, sizeof(struct sockaddr));

	stServAddr.sin_family = AF_INET;
	stServAddr.sin_addr.s_addr = htonl(guiMyIP);
	stServAddr.sin_port = htons(usPort);

	dRet = bind(dSockfd, (struct sockaddr *)&stServAddr, sizeof(stServAddr));
	if(dRet < 0)
	{
		log_print(LOGN_CRI, "Bind Error :%s", strerror(errno));
		return -3; 	
	}

	log_print(LOGN_DEBUG, "SUCC SOCK PORT[%hu]", usPort);

	*pdSock = dSockfd;
	
	return 0;
} 

int dIsRcvedPacket()
{
    int     			nSel, dFromLen, dRcvSize;
	int					dDataSize;
	int					tmp;
    fd_set  			Rd;
    struct sockaddr_in  stFrom;
    struct timeval 		stTimeOut;
	Capture_Header_Msg	*pcaphead;
	U8					*pData, *pnode;
	U8					szBuf[BUFSIZ];
	OFFSET				offset;

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
                log_print(LOGN_CRI, "recvfrom REASON[%s]", strerror(errno));
                return -1;
            }
            else if(dRcvSize == 0)
                return 0;
            else
            {
#ifdef DEF_FCS
				dDataSize = dRcvSize - 12 - 4; /* remove for fcs */
#else
				dDataSize = dRcvSize - 12;
#endif
				if((pnode = nifo_node_alloc(pmem)) == NULL) {	
					log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
					return -2;
				}
				if((pData = nifo_tlv_alloc(pmem, pnode, ETH_DATA_NUM, dDataSize, DEF_MEMSET_OFF)) == NULL) {
					log_print(LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
					return -3;
				}

				memcpy(pData, &szBuf[12], dDataSize);

				if((pcaphead = (Capture_Header_Msg *)nifo_tlv_alloc(pmem, pnode, CAP_HEADER_NUM, CAP_HRD_LEN, DEF_MEMSET_OFF)) == NULL) {
					log_print(LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
					return -4;
				}

				memcpy(&tmp, &szBuf[0], 4);
				pcaphead->curtime = ntohl(tmp);
				//pcaphead->curtime = tmp;
				memcpy(&tmp, &szBuf[4], 4);
				pcaphead->ucurtime = ntohl(tmp);
				//pcaphead->ucurtime = tmp;
				memcpy(&tmp, &szBuf[8], 4);
				pcaphead->bRtxType = ntohl(tmp);
				//pcaphead->bRtxType = tmp;
				pcaphead->datalen = dDataSize;

log_print(LOGN_INFO, "CTIME[%lu]MTIME[%lu]RTX[%d]LEN[%d]OFFSET[%d]", 
pcaphead->curtime, pcaphead->ucurtime,
pcaphead->bRtxType, pcaphead->datalen, nifo_offset(pmem, pnode));			

				offset = nifo_offset(pmem, pnode);
				if(gifo_write(pmem, pcifo, guiSeqProcID, guiSeqProcID, offset) < 0)
				{
					log_print(LOGN_CRI, "[ERROR] gifo_write(from=%d:CAPD, to=%d:CAPD), offset=%d",
								SEQ_PROC_CAPD, SEQ_PROC_CAPD, offset);
					nifo_node_delete(pmem, pnode);
					usleep(0);
				}

				/*
				if((dRet = nifo_msg_write(pmem, myqid, pnode)) < 0) {
					log_print(LOGN_CRI, "[%s][%s.%d] nifo_msg_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
					return -5;
				}
				*/
/*
				if((dRet = dSend_CAPD_Data(pmem, myqid, pnode, pcaphead->curtime)) < 0) {
					log_print(LOGN_CRI, "nifo_msg_write fail [%d][%s]", dRet, strerror(-dRet));
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

void sigtest(int sig) 
{
#ifdef MEM_TEST
	log_print(LOGN_CRI, "CREATE CNT[%llu] DELETE CNT[%llu]", nifo_create, nifo_del);
#endif
	close(gdSockfd);
	exit(0);
}

void test_func(char *ip, unsigned short usPort)
{

	log_print(LOGN_CRI, "CAPD TEST START");
  	signal(SIGINT, sigtest);

	guiMyIP = ntohl(inet_addr(ip));
	guiSeqProcID = SEQ_PROC_CAPD;
	memcpy(&gszMyProc[0], "CAPD", 4);
	
	// NIFO ZONE
	if((pmem = nifo_init_zone((U8*)gszMyProc, guiSeqProcID, FILE_NIFO_ZONE)) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone NULL", LT);
		return;
	}

	// NIFO GROUP
	if((pcifo = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL )
	{
		log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
			LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
		return;
	}
	
	/*
	pmem = nifo_init(S_SSHM_NIFO, S_SEMA_NIFO, "CAPD", SEQ_PROC_CAPD);
	if((myqid = nifo_msgq_init(S_MSGQ_PRE_A)) < 0) {
		log_print(LOGN_CRI, "[%s.%d] nifo_msgq_init [%d][%s]", __FUNCTION__, __LINE__, myqid, strerror(-myqid));
		exit(0);
	}
	*/

	if(dUdpSockInit(usPort, &gdSockfd) < 0) {
		log_print(LOGN_CRI, "[%s.%d] dUdpSockInit", __FUNCTION__, __LINE__);
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
	Revision 1.1.1.1  2011/08/29 05:56:42  dcham
	NEW OAM SYSTEM
	
	Revision 1.2  2011/08/21 07:19:15  hhbaek
	Commit CAPD_SIM
	
	Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
	init DQMS2
	
	Revision 1.2  2011/01/11 04:09:12  uamyd
	modified
	
	Revision 1.2  2010/11/14 14:31:10  jwkim96
	STP 작업 내용 반영.
	
	Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
	DQMS With TOTMON, 2nd-import
	
	Revision 1.1  2010/03/05 07:00:19  jsyoon
	*** empty log message ***
	
	Revision 1.2  2009/06/10 21:02:29  jsyoon
	*** empty log message ***
	
	Revision 1.1.1.1  2009/05/26 02:14:29  dqms
	Init TAF_RPPI
	
	Revision 1.4  2008/11/17 11:15:14  dark264sh
	*** empty log message ***
	
	Revision 1.3  2008/11/17 09:06:17  dark264sh
	64bits 작업
	
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
	
	Revision 1.6  2006/09/13 11:38:29  dark264sh
	hasho_add, hasho_find를 잘못 사용한 부분 수정
	
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
