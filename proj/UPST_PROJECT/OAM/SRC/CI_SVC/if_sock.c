/** A. FILE INCLUSION *********************************************************/

/* SYS HEADER */
#include <unistd.h>		/* READ(2) */
#include <string.h>		/* memcpy */
#include <errno.h>		/* error */
#include <fcntl.h>		/* fcntl */
#include <sys/select.h>	/* FD_SET() */
/* LIB HEADER */
#include "clisto.h"		/* U8 */
#include "loglib.h"
#include "utillib.h"	/* util_makenid() */
#include "filelib.h"	/* get_ip_conf() */
#include "nsocklib.h"	/* DEF_MAX_SOCK_SIZE, DEF_MAX_BUFLEN  */
/* PRO HEADER */
#include "path.h"
#include "sockio.h"		/* st_NTAFTHeader */
#include "msgdef.h"		/* st_MsgQ */
#include "mmcdef.h"		/* MI_XXX */
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "if_func.h"	/* dMsgsnd(), dGetNode() */
#include "if_sock.h"

extern fd_set        g_readset, g_writeset;
extern st_ClientInfo g_stClientInfo;

int dRecvPacket(int dSocket)
{
    int     		n, size, dRet, sp;
	st_NTAFTHeader	stHeader;

    size = g_stClientInfo.dBufSize;

    n = read(dSocket, &g_stClientInfo.szBuf[size], DEF_MAX_BUFLEN - size);
    if(n <= 0)
    {
        if(n == 0)
		{
			log_print(LOGN_DEBUG,LH"[CLOSE SERVER]",LT);
            return -1;
		}
        else if(errno != EAGAIN)
		{
			log_print(LOGN_DEBUG,
			"READ SOCKERROR[%d][%s]\n", errno, strerror(errno));
            return -1;
		}
        else
            return 0;
    }
	else if(size + n < NTAFT_HEADER_LEN)
	{
		g_stClientInfo.dBufSize = size + n;
		g_stClientInfo.szBuf[g_stClientInfo.dBufSize] = 0;
        return 0;
	}
	else
	{
        size += n;
        sp = 0;
        do {
            memcpy(&stHeader, &g_stClientInfo.szBuf[sp], NTAFT_HEADER_LEN);
            if (stHeader.usTotlLen <= size - sp)
			{
				dRet = dHandleMsg(&stHeader, &g_stClientInfo.szBuf[sp]);
                if (dRet < 0)
                    return -1;
                sp += stHeader.usTotlLen;
            }
			else
                break;
        } while (size >= sp + NTAFT_HEADER_LEN);

        /* Buffer set */
        if (size > sp)
		{
            g_stClientInfo.dBufSize = size - sp;
            memcpy(&g_stClientInfo.szBuf[0], &g_stClientInfo.szBuf[sp], size - sp);
        }
		else
		{
            g_stClientInfo.dBufSize = 0;
        }
    }
    return 0;
}

int dInit_Tcp_Client(int *dServerSocket)
{
	int					dRet, dReUseAddr, flag, kk;
	struct sockaddr_in	stServerAddr;
	struct linger		stLinger;
	int                 dBufSize;
	char 				primary_addr[256];
	char 				secondary_addr[256];

	FD_ZERO(&g_readset);
	FD_ZERO(&g_writeset);

	if( (*dServerSocket = socket(AF_INET, SOCK_STREAM, 0) ) < 0)
	{
		log_print(LOGN_DEBUG,"[CAN'T OPEN SOCKET] errno=%d[%s]\n",
			errno, strerror(errno));
		return -1;
	}

	if((flag = fcntl(*dServerSocket, F_GETFL, 0)) < 0)
	{
        log_print(LOGN_DEBUG,"FAIL IN GET SOCKET OPTION : flag, errno = %d[%s]",
			errno, strerror(errno));
		close(*dServerSocket);
        return -1;
	}

	flag |= O_NONBLOCK;
	if(fcntl(*dServerSocket, F_SETFL, flag) < 0)
	{
        log_print(LOGN_DEBUG,"FAIL IN CHANG NONBLOCK OPTION : NONBLOCK, errno = %d[%s]",
			errno, strerror(errno));
		close(*dServerSocket);
        return -1;
	}

	dReUseAddr = 1;
    if (setsockopt (*dServerSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&dReUseAddr, sizeof(int)) < 0)
	{
        log_print(LOGN_DEBUG,"FAIL IN CHANGING SOCKET OPTION : REUSE, errno = %d[%s]",
			errno, strerror(errno));
		close(*dServerSocket);
        return -1;
    }

    dReUseAddr = sizeof(struct linger);
    getsockopt (*dServerSocket, SOL_SOCKET, SO_LINGER, (void *)&stLinger, (socklen_t*)&dReUseAddr);
    stLinger.l_onoff = 0;
    stLinger.l_linger = 0;

    if (setsockopt (*dServerSocket, SOL_SOCKET, SO_LINGER, (void *)&stLinger, sizeof(stLinger)) < 0)
	{
        log_print(LOGN_DEBUG,"FAIL IN CHANGING SOCKET OPTION : LINGER, errno = %d[%s]",
			errno, strerror(errno));
		close(*dServerSocket);
        return -1;
    }

	dBufSize = DEF_MAX_SOCK_SIZE;
    if(setsockopt(*dServerSocket, SOL_SOCKET, SO_SNDBUF, (void *)&dBufSize, sizeof(dBufSize)) < 0) {
        log_print(LOGN_DEBUG,"FAIL IN CHANGING SOCKET OPTION : SEND BUFFER, errno = %d[%s]",
            errno, strerror(errno));
        close(*dServerSocket);
        return -1;
    }

	FD_SET(*dServerSocket, &g_readset);

	get_ip_conf (FILE_SUP_IP_CONF, primary_addr, secondary_addr);

	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_addr.s_addr = inet_addr(primary_addr);
	stServerAddr.sin_port	= htons(S_PORT_SI_SVC);

	for(kk=0; kk < MAX_RETRY_CNT; kk++)
	{
		if((dRet = connect(*dServerSocket, (struct sockaddr *)&stServerAddr, sizeof(stServerAddr))) < 0)
		{
			if((errno == EALREADY) || (errno == EINPROGRESS))
			{
				sleep(1);
				continue;
			}
			else if(errno == EISCONN)
			{
            	log_print(LOGN_DEBUG, "CONNECTED SERVER IP[%u]",
            	ntohl(stServerAddr.sin_addr.s_addr));
				break;
			}
			else
			{
            	log_print(LOGN_DEBUG, LH"RECV CONNECT RET IP:[%s:%u] errno=[%d][%s]",
            	LT, util_cvtipaddr(NULL,stServerAddr.sin_addr.s_addr),
				ntohl(stServerAddr.sin_addr.s_addr), errno, strerror(errno));
				close(*dServerSocket);
				return -1;
			}
		}
		else
		{
			log_print(LOGN_DEBUG, LH"CONNECTED SERVER IP[%s:%u]",
			LT, util_cvtipaddr(NULL,stServerAddr.sin_addr.s_addr),
			ntohl(stServerAddr.sin_addr.s_addr));
			break;
		}
	}

    if(kk >= MAX_RETRY_CNT)
    {
        log_print(LOGN_DEBUG,LH"MAX RETRY COUNT OVER=[%d] CLOSE",LT,kk);
        close(*dServerSocket);
        return -1;
    }

	return 0;
} /*** end of dInit_Tcp_Client ***/

int dHandleMsg(st_NTAFTHeader *pstHeader, char *szBuf)
{
	int			 dRet, dSeqProcID, dMsgLen, dMCode;
	pst_MsgQ	 pstMsgQ;
	pst_MsgQSub	 pstMsgQSub;
	U8			*pNODE;

	/* select MSGQ ID */
	switch(pstHeader->ucSvcID)
	{

		case SID_CHKREQ:
			log_print(LOGN_INFO, "SID_CHKREQ");
		case SID_FLT:
		case SID_PATCH:
			dSeqProcID = SEQ_PROC_S_MNG;
			break;

		case SID_MML:
			dMCode = ((mml_msg *)&szBuf[NTAFT_HEADER_LEN])->msg_id;

			switch(dMCode)
			{
				case MI_ACT_SUB_PRC:
				case MI_DACT_SUB_PRC:
				case MI_DIS_SUB_PRC:
				case MI_DIS_SUB_SYS_INFO:
					dSeqProcID = SEQ_PROC_CHSMD;
					break;

				case MI_DIS_SUB_SESS:
				case MI_SUB_TIMER:
				case MI_DIS_NTAF:
				case MI_CHG_NTAF :
				case MI_DIS_ALM_NTAF :
				case MI_CHG_ALM_NTAF :
					dSeqProcID = SEQ_PROC_S_MNG;
					break;

				default:
					log_print(LOGN_WARN, "dRecvProc : UNKNOWN MCODE [%d]",
							dMCode);
					return 0;
					break;
			}
			break;

		case SID_CHECK_MSG:
			log_print(LOGN_DEBUG,LH"HEART BEAT MSG RCVED", LT);
			return 0;

		default:
			log_print(LOGN_WARN, "dRecvProc : WRONG SVCID [%d] MID [%d] SYSID [%d]",
					pstHeader->ucSvcID,pstHeader->ucMsgID, pstHeader->ucNTAFID );
			return -1;
	}

	if( dGetNode( &pNODE, &pstMsgQ ) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dGetNode(CI_SVC)", LT);
		return -2;
	}

	pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;

	pstMsgQSub->usType  = DEF_SYS;
	pstMsgQSub->usSvcID = pstHeader->ucSvcID;
	pstMsgQSub->usMsgID = pstHeader->ucMsgID;
	pstMsgQ->llIndex    = pstHeader->llIndex;

	util_makenid(SEQ_PROC_CI_SVC, &pstMsgQ->llNID);

	pstMsgQ->usBodyLen = pstHeader->usBodyLen;
	pstMsgQ->ucProID   = SEQ_PROC_CI_SVC;
	pstMsgQ->dMsgQID   = 0;

	memcpy(pstMsgQ->szBody, &szBuf[NTAFT_HEADER_LEN], pstHeader->usBodyLen);

	if( (dRet = dMsgsnd( dSeqProcID, pNODE )) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(to SeqProcID=%d)", LT, dSeqProcID);
		return -3;
	}

	log_print(LOGN_INFO, "dHandleMsg : SND MSGQ [SIZE]:[%d] [SVC ID]:[%d] [MSG ID]:[%d]",
	dMsgLen, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);

	return 0;
}

void write_proc(int fd)
{
	int k, t;

	if (g_stClientInfo.dFront == g_stClientInfo.dRear) {
		FD_CLR(fd, &g_writeset);
		g_stClientInfo.dFront = g_stClientInfo.dRear = 0;
		log_print(LOGN_CRI,"Invalid Length in socket write buffer");
	} else {

		if (g_stClientInfo.dFront > g_stClientInfo.dRear)
			k = g_stClientInfo.dFront - g_stClientInfo.dRear;
		else
			k = DEF_MAX_BUFLEN - g_stClientInfo.dRear;

		if ((t = write(fd, &g_stClientInfo.szWBuf[g_stClientInfo.dRear], k)) <= 0) {
			if (t && errno != EAGAIN) {
				g_stClientInfo.dFront = g_stClientInfo.dRear = 0;
				FD_CLR(fd, &g_writeset);
				log_print(LOGN_CRI,"Failure in writing to socket second time, errno = %d", errno);
			}
			else if( errno == EAGAIN)
			{
				log_print(LOGN_CRI,"SOCK BLOCK [EAGAIN][%s]", strerror(errno));
				log_print(LOGN_DEBUG,"SOCK BLOCK SEND SIZE [0]");
			}
		} else if (k == t) {
			g_stClientInfo.dRear += t;
			log_print(LOGN_DEBUG,"SOCK BLOCK SEND SIZE [%d]",t);

			if (g_stClientInfo.dFront == g_stClientInfo.dRear) {
				g_stClientInfo.dFront = g_stClientInfo.dRear = 0;
				FD_CLR(fd, &g_writeset);
			} else if (g_stClientInfo.dRear == DEF_MAX_BUFLEN)
				g_stClientInfo.dRear = 0;
		}
		else
		{
			g_stClientInfo.dRear += t;
			log_print(LOGN_CRI,"SOCK BLOCK [EAGAIN ???]");
			log_print(LOGN_DEBUG,"SOCK BLOCK SEND SIZE [%d]",t);
		}
	}
} /*** end of write_proc ***/

int writeSocket(int dSocket, void *buffer, int slen)
{
	int i, k;
	char *str = (char *)buffer;

	if (FD_ISSET(dSocket, &g_writeset)) {

		k = g_stClientInfo.dFront + slen;
		if ((g_stClientInfo.dFront < g_stClientInfo.dRear && k >= g_stClientInfo.dRear)
		|| (g_stClientInfo.dFront > g_stClientInfo.dRear && k >= g_stClientInfo.dRear + DEF_MAX_BUFLEN)) {
			log_print(LOGN_CRI,"Packet discarded : slen=%d, socket=%d", slen, dSocket);
			return SOCK_FALSE;
		}

		if (k >= DEF_MAX_BUFLEN) {
			k -= DEF_MAX_BUFLEN;
			memcpy(&g_stClientInfo.szWBuf[g_stClientInfo.dFront], (void *)str, slen - k);
			memcpy(&g_stClientInfo.szWBuf[0], (void *)&str[slen - k], k);
		} else
			memcpy(&g_stClientInfo.szWBuf[g_stClientInfo.dFront], (void *)str, slen);
		g_stClientInfo.dFront = k;
		return SOCK_FALSE;

	} else if (g_stClientInfo.dFront != g_stClientInfo.dRear) {
		g_stClientInfo.dFront = g_stClientInfo.dRear = 0;
		log_print(LOGN_CRI,"Undefined error occurres in writeSocket");
	}

	memcpy(&g_stClientInfo.szWBuf[0], (void *)str, slen);

	if ((i = write(dSocket, &g_stClientInfo.szWBuf[0], slen)) != slen) {
		if (i < 0) {
			if (errno != EAGAIN) {
				g_stClientInfo.dFront = g_stClientInfo.dRear = 0;
				log_print(LOGN_CRI,"Failure in writing socket 1st errno = %d", errno);
				return -1;
			}
			else
			{
				log_print(LOGN_CRI,"SOCK BLOCK [EAGAIN][%s]", strerror(errno));
				i = 0;
			}
		}
		g_stClientInfo.dRear = i;
		g_stClientInfo.dFront = slen;
		FD_SET(dSocket, &g_writeset);
		return SOCK_FALSE;
	}

	g_stClientInfo.dFront = 0;
	g_stClientInfo.dRear = 0;

	return SOCK_TRUE;
} /*** end of writeSocket ***/
