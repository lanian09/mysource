/**
 *	Include headers
 */
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// TOP
#include "common_stg.h"
#include "commdef.h"
#include "msgdef.h"
#include "procid.h"
#include "path.h"
#include "sockio.h"

// LIB
#include "gifo.h"		/* gifo_write() */
#include "cifo.h"		/* stCIFO */
#include "mems.h"		/* stMEMSINFO */
#include "loglib.h"
#include "utillib.h"
#include "nsocklib.h"

// .
#include "ci_log_sock.h"

/**
 *	Declare var.
 */
extern stMEMSINFO		*pMEMSINFO;
extern stCIFO			*gpCIFO; 
extern st_ClientInfo	g_stClientInfo;
extern fd_set			g_readset;
extern fd_set			g_writeset;

/**
 *	Implement func.
 */
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
			log_print(LOGN_DEBUG,"[CLOSE SERVER]");
            return -1;
		}
        else if(errno != EAGAIN)
		{
			log_print(LOGN_DEBUG,"READ SOCKERROR[%d][%s]\n", errno, strerror(errno));
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
				dRet = dSendToProc(stHeader.usTotlLen, &g_stClientInfo.szBuf[sp]);

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
} /*** end of dRecvPacket ***/

int dInit_CILOG_Client(int *dServerSocket)
{
	int								dRet, dReUseAddr, flag, kk;
	struct sockaddr_in				stServerAddr;
	struct linger					stLinger;
	int								dBufSize;
	char 							primary_addr[256];
	char 							secondary_addr[256];	

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

	dBufSize = DEF_SOCK_BUF_SIZE; 
    if(setsockopt(*dServerSocket, SOL_SOCKET, SO_SNDBUF, (void *)&dBufSize, sizeof(dBufSize)) < 0) {
        log_print(LOGN_DEBUG,"FAIL IN CHANGING SOCKET OPTION : SEND BUFFER, errno = %d[%s]",
            errno, strerror(errno));
        close(*dServerSocket);
        return -1;
    }

	FD_SET(*dServerSocket, &g_readset);

	dGetIPAddr (FILE_SUP_IP_CONF, primary_addr, secondary_addr);

	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_addr.s_addr = inet_addr(primary_addr);
	stServerAddr.sin_port	= htons(S_PORT_SI_LOG);

	for(kk=0; kk < MAX_RETRY_CNT; kk++)
	{
		alarm(3);
		if((dRet = connect(*dServerSocket, (struct sockaddr *)&stServerAddr, sizeof(stServerAddr))) < 0)
		{ 
		alarm(0);
			
			if(errno == EINTR) {
				pause();
				sleep(1);
				continue;
			} else if(errno == EINPROGRESS || errno == EALREADY) /* SYN_SENT */ {
				sleep(1);
				continue;
			} else if(errno == EISCONN) /* ESTABLISTED */ {
				dRet = 0;
				break;
			} else if(errno == EADDRINUSE) {
				sleep(1);
				continue;
			}
			
			log_print(LOGN_DEBUG,"dInit_CILOG_Client : Can't connect server. [IP]:[%s] [ERRNO]:[%d] [REASON]:[%s]", 
			primary_addr, errno, strerror(errno)); 

			close(*dServerSocket);
			return -1; 
		}
		alarm(0);
	}

	if(kk >= MAX_RETRY_CNT)
	{
		log_print(LOGN_DEBUG,"MAX RETRY COUNT OVER=[%d] CLOSE", kk);
		close(*dServerSocket);
		return -1; 
	}

	return 0;
} /*** end of dInit_CILOG_Client ***/ 

int dSendToProc(int dLen, char *szBuf)
{
	pst_MsgQ            pstMsgQ;
    pst_MsgQSub         pstMsgQSub;
    st_NTAFTHeader      stHeader;
    UCHAR               ucSvcID, ucMsgID;
	U8					*pNODE;
    
    memcpy(&stHeader, szBuf, NTAFT_HEADER_LEN);
    if(stHeader.llMagicNumber != MAGIC_NUMBER)
    {   
        log_print(LOGN_CRI, "dSendToProc0 : MAGIC NUMBER ERROR, Will be Reset [%lld][%lld]",
            stHeader.llMagicNumber, MAGIC_NUMBER);
        return -1; 
    } /* end of if */
    
    ucSvcID = stHeader.ucSvcID;
    ucMsgID = stHeader.ucMsgID;
    
    switch(ucSvcID)
    {   
        case SID_SESS_INFO :

			if( (pNODE = nifo_node_alloc(pMEMSINFO)) == NULL ){
				log_print(LOGN_WARN, "FAILED IN nifo_node_alloc, errno=%d:%s", errno, strerror(errno));
				return -2;
			}

			if( (pstMsgQ = (pst_MsgQ)nifo_tlv_alloc(pMEMSINFO, pNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF)) == NULL ){
				log_print(LOGN_WARN, "FAILED IN nifo_tlv_alloc, return NULL");
				return -3;
			}

            pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;
            pstMsgQSub->usType = DEF_SVC;
            pstMsgQSub->usSvcID = ucSvcID;
            pstMsgQSub->usMsgID = ucMsgID;
            
            pstMsgQ->ucNTAFID = stHeader.ucNTAFID;
            
            pstMsgQ->llIndex = stHeader.llIndex ;

            pstMsgQ->usBodyLen = stHeader.usBodyLen;
            pstMsgQ->usRetCode = 0;
            pstMsgQ->ucProID = SEQ_PROC_CI_LOG;
            pstMsgQ->dMsgQID = 0;

            util_makenid(SEQ_PROC_CI_LOG, &pstMsgQ->llNID);

            memcpy(pstMsgQ->szBody, szBuf+NTAFT_HEADER_LEN, stHeader.usBodyLen);

            log_print(LOGN_INFO, "[dSendToProc] Rcv SVCID[%d] MSGID[%d] From SysNo[%d] ",
                    ucSvcID, ucMsgID, pstMsgQ->ucNTAFID);

            switch( ucMsgID )
            {
                case MID_SESS_START:
                case MID_SESS_STOP:
					if(gifo_write( pMEMSINFO, gpCIFO, SEQ_PROC_CI_LOG, SEQ_PROC_A_TCP, nifo_offset(pMEMSINFO,pNODE)) < 0 ){
						log_print(LOGN_CRI, "FAILED IN gifo_write. TO A_TCP=%d, offset=%ld",
							SEQ_PROC_A_TCP, nifo_offset(pMEMSINFO, pNODE));
						usleep(0);
					}

                default:
                    log_print(LOGN_CRI, "dSendToProc ERROR : invalid MID [%d] ",ucMsgID);
                    break;
            }
            break;

        default:
            log_print(LOGN_CRI,
                    "dSendToProc : [SVCID][%d] [MSGID][%d] [ERROR]", ucSvcID, ucMsgID);
            break;
    } /* end of switch */

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
				log_print(LOGN_WARN,"SOCK BLOCK [EAGAIN][%s]", strerror(errno));
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
			log_print(LOGN_WARN,"SOCK BLOCK [EAGAIN ???] size=%d", t);
		}
	}
} /*** end of write_proc ***/

int writeSocket(int dSocket, char *str, int slen)
{
	int i, k;
	if (FD_ISSET(dSocket, &g_writeset)) 
		{

		k = g_stClientInfo.dFront + slen;
		log_print(LOGN_DEBUG,"g_stClientInfo.dFront + slen [%d]",k);
		if ((g_stClientInfo.dFront < g_stClientInfo.dRear && k >= g_stClientInfo.dRear)
		|| (g_stClientInfo.dFront > g_stClientInfo.dRear && k >= g_stClientInfo.dRear + DEF_MAX_BUFLEN)) 
		{
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
				log_print(LOGN_CRI,"Failure in writing socket 1st errno = %d [%s]", errno,strerror(errno));
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


int dGetIPAddr( char *conf_file, char *primary_addr, char *secondary_addr )
{
    FILE *fp;
    int cnt = 0;
    char  szStr[257];
    int d1, d2, d3, d4;

    primary_addr[0] = '\0';
    secondary_addr[0] = '\0';

    fp = fopen( conf_file, "r" );

    if( fp == NULL )
    {
        return -1;
    }

    while( fgets(szStr, 256, fp) != NULL )
    {
        if( szStr[0] == '#' || szStr[0] == '/' )
        {
            continue;
        }

        szStr[strlen(szStr)-1] = '\0';

        if( sscanf(szStr, "%d.%d.%d.%d", &d1, &d2, &d3, &d4 ) != 4 )
            continue;
        if( d1<0 || d1>255 || d2<0 || d2>255 || d3<0 || d3>255 || d4<0 || d4>255 )
            continue;
		
		log_print(LOGN_INFO, "NTAM IP ADDRESS : [%s]", szStr);

        if( cnt == 0 )
        {
            sprintf( primary_addr, "%s", szStr );
        }
        else if( cnt == 1 )
        {
            sprintf( secondary_addr, "%s", szStr );
        }
        else
            break;

        cnt++;
    }

    fclose(fp);

    if( primary_addr[0] == '\0' )
        return 0;

    return 1;

} /**** end of dGetIPAddr *****/

