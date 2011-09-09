#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sim.h"
#include "mmc.h"

/**B.1*  Definition of New Constants *********************/

/**B.2*  Definition of New Type  **************************/

/**C.1*  Declaration of Variables  ************************/
extern int 				gdMaxfd;

extern unsigned int 	guiMyIP;

/**D.1*  Definition of Functions  *************************/

/**D.2*  Definition of Functions  *************************/

/***********************************************************
	
	Function Name :   dUdpSockInit
	
	Parameter(s) :

	Function :
		1813, 49987 Port UDP socket initial
	
	Return :

	History :
		01.01.15  Revised by tundra 
***********************************************************/

/* int type 의 IP를 char* 형으로 변환하는 함수 */
char *CVT_INT2STR_IP(unsigned int uiIP)
{
	static char     szIPAddr[MAX_IP_LEN];  /* MAX_IP_LEN is 16 */
	struct in_addr  stAddr;

	stAddr.s_addr = htonl(uiIP);

	inet_ntop(AF_INET, (void *)&stAddr, szIPAddr, MAX_IP_LEN);

	return szIPAddr;
}

int initUdpSock (void)
{
	int sfd; 
	int dRet;

	sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sfd < 0) {
		dAppLog(LOG_CRI, "udp socket error :%s", strerror(errno));
		return -1; 	
	}	

	dRet = fcntl(sfd, F_SETFD, O_NONBLOCK);	
	if(dRet < 0) {
		dAppLog(LOG_CRI, "fcntl error : %s", strerror(errno));
		return -2; 	
	}

	return sfd;
} 

#if 0
int initTcpSock (char *host, int port, struct sockaddr_in *addr)
{
	struct hostent *he;
	int sfd;

	if (host == NULL) {
		dAppLog(LOG_CRI, "host is undefined!");
		return -1;
	}

	sfd = socket ( AF_INET, SOCK_STREAM, 0 );
	if (sfd < 0) {
		dAppLog(LOG_CRI, "socket create failed");
		return -1;
	}
	memset(addr, 0x00, sizeof(*addr));

	he = gethostbyname (host);
	if (he == NULL) {
		dAppLog(LOG_CRI, "gethostbyname, There is no that such host");
		return -1;
	}
	memcpy(&addr->sin_addr, he->h_addr, he->h_length);
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);

	return sfd;
}



int doConnectTcp (int cfd, struct sockaddr_in *addr)
{
	int rtn=0;

	if ((rtn = connect(cfd, (struct sockaddr *) &addr, sizeof(addr))) == -1) {
		if (errno != EWOULDBLOCK && errno != EINPROGRESS) {
			dAppLog(LOG_CRI, "tcp connect error");
			close(cfd);
			return -1;
		}
	}
	dAppLog(LOG_CRI, "FD(%d) tcp connect ok(%d)", cfd, rtn);
	return 0;
}

#endif
#if 0
	struct in_addr inAddr;

    if (inet_aton(pRad->szIP, &inAddr)==0) {
		dAppLog(LOG_WARN, "[sendPacketUDP] inet_aton fail");
		return -1;
	}
#endif

int sendPacketUDP (PRAD_OPT pRad, unsigned int  uiBodyLen, unsigned char *szBody, unsigned int sendCnt)
{
    struct sockaddr_in dstAddr;
	unsigned int dRet, tot_sent=0, i;
	unsigned int succNum=0, tot_succNum=0;
	struct in_addr inAddr;
	time_t	cur_time, old_time;
	struct timeval  start_t, end_t, rst_t;
	struct timeval  now_t, prev_t;
	char logbuf[128];

#if 0
	dAppLog(LOG_DEBUG, "SYSIP[%s] PORT [%d]"
			, pRad->szIP, pRad->usPort);
#endif
 
    dstAddr.sin_family = AF_INET;
    if (inet_aton(pRad->szIP, &inAddr)==0) {
		dAppLog(LOG_WARN, "[sendPacketUDP] inet_aton fail");
		return -1;
	}
    dstAddr.sin_addr.s_addr = inAddr.s_addr;
    dstAddr.sin_port = htons(pRad->usPort);

	gettimeofday (&start_t, NULL);
	prev_t.tv_sec  = start_t.tv_sec;
	prev_t.tv_usec = start_t.tv_usec;

	for(i=0;i<sendCnt;i++)
	{
#if 0
		{
			/* Calling-Station-ID convert */
			int subsID=0;
			char tmpBuf[24];
			strncpy(tmpBuf, szBody+67, 8);
			subsID = atoi(tmpBuf);
			subsID++;
			sprintf(tmpBuf, "%d", subsID);
			strncpy(szBody+67, tmpBuf, 8);

			/* Framed IF convert */
			unsigned int tmpIP=0;
			tmpIP = *(unsigned int*)(szBody+45);
			tmpIP++;
			memcpy(szBody+45, &tmpIP, sizeof(int));
		}
#endif			  
		dRet = sendto(pRad->uiCFD, &szBody[0], uiBodyLen, 0, 
				(struct sockaddr*)&dstAddr, sizeof(struct sockaddr_in));
		if(dRet>0)
			tot_sent += dRet;

		gettimeofday (&now_t, NULL);
		timeSub (&rst_t, &now_t, &prev_t);

		if (rst_t.tv_sec == 1) {
			prev_t.tv_sec  = now_t.tv_sec;
			prev_t.tv_usec = now_t.tv_usec;   
			sprintf(logbuf, " @ SECOND REPORT: send count=%d/%d/%d, send size=%d/%d(tot:%d) delay time=%ld.%ld\n"
				   			, succNum, tot_succNum, sendCnt, dRet, uiBodyLen, tot_sent,  rst_t.tv_sec, rst_t.tv_usec);
			mprintf("%s", logbuf);
			succNum = 0;
		}

		if(dRet == uiBodyLen) {
			dAppLog(LOG_DEBUG, "SendToUDP FD:%d [%s][%d] : send size[%d/%d]" 
					, pRad->uiCFD, pRad->szIP, pRad->usPort, dRet, uiBodyLen);
			++succNum;
			++tot_succNum;
			continue;
		}

		dAppLog(LOG_WARN, "[FAIL] SendTo Fail : send size[%d/%d] " , dRet, uiBodyLen);
	}
	gettimeofday (&end_t, NULL);
	timeSub (&rst_t, &end_t, &start_t);

	sprintf(logbuf, " @ RESULT REPORT: send count=%d/%d, send size=%d/%d delay time=%ld.%ld\n"
			   			, tot_succNum, sendCnt, uiBodyLen, dRet, rst_t.tv_sec, rst_t.tv_usec);
	
	mprintf("%s", logbuf);
	dAppLog(LOG_WARN, "%s", logbuf);

    return (sendCnt - succNum);
}

int timeSub (struct timeval *res,
		const struct timeval *after, const struct timeval *before)
{
	long sec = after->tv_sec - before->tv_sec;
	long usec = after->tv_usec - before->tv_usec;

	if (usec < 0)
		usec += 1000000, --sec;

	res->tv_sec = sec;
	res->tv_usec = usec;

	return (sec < 0) ? (-1) : ((sec == 0 && usec == 0) ? 0 : 1);
}
