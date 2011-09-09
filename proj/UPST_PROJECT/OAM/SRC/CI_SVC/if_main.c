/** A. FILE INCLUSION *********************************************************/

/* SYS HEADER */
#include <unistd.h>		/* GETPID(2) */
#include <stdlib.h>		/* EXIT(3) */
#include <string.h>		/* MEMCPY(3) */
#include <errno.h>		/* error */
/* LIB HEADER */
#include "commdef.h"	/* S_SSHM_* */
#include "loglib.h"
#include "verlib.h"		/* set_version() */
#include "nsocklib.h"	/* st_ClientInfo */
/* PRO HEADER */
#include "msgdef.h"		/* st_MsgQ */
#include "sshmid.h"
#include "path.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "if_init.h"	/* dInitProc() */
#include "if_sock.h"	/* dInit_Tcp_Client() */
#include "if_msgq.h"	/* dIsRcvedMessage(), dSndMsgProc() */

/**B.1*  Definition of New Constants **********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables ( Local ) ***************/
int g_FinishFlag;
int g_JiSTOPFlag;
fd_set 			g_readset, g_writeset;
st_ClientInfo   g_stClientInfo;

/**C.2*  Declaration of Variables ( External ) ************/
/**D.1*  Definition of Functions  ( Local ) ***************/
/**D.2*  Definition of Functions  ( External ) ************/

int main(void)
{
	int				i, dRet, dNumOfSocket;
	int             dServerSocket, dLogInStatus;
	st_MsgQ		    stMsgQ;
	struct timeval	timeout;
	fd_set			fd_read_tmp, fd_write_tmp;
	time_t			tCurTime, tOldTime;

	dNumOfSocket	= 0;
	tOldTime		= time(NULL);
	dLogInStatus	= DEF_LOGOUT;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_CI_SVC, LOG_PATH"/CI_SVC", "CI_SVC");

	if( (dRet = dInitProc()) < 0)
		return -2;

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_CI_SVC, "R3.0.0")) < 0)
	{
		log_print(LOGN_WARN, "F=%s:%s.%d: ERROR IN Failed set_version() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(-3);
	}
	log_print(LOGN_CRI, "[PROCESS INIT SUCCESS, PROCESS START]");

	while(g_JiSTOPFlag)
	{
		//Log-Out 일 경우 Connect 재요청 하는 부분 //
		if(dLogInStatus == DEF_LOGOUT)
		{
			while(g_JiSTOPFlag)
			{
				time(&tCurTime);
				if(tCurTime > tOldTime)
				{
					tOldTime = tCurTime;
					dRet = dInit_Tcp_Client(&dServerSocket);
					if(dRet < 0)
					{
						close(dServerSocket);
						sleep(1);

						while(1)
						{
							if(dIsRcvedMessage(&stMsgQ) <= 0)
								break;
						}/* for empty MsgQ */
					}
					else
					{
						dNumOfSocket = dServerSocket + 1;
						dLogInStatus = DEF_CONNECT;

						g_stClientInfo.szBuf[0] = 0x00;
						g_stClientInfo.dBufSize = 0;
						g_stClientInfo.dFront = 0;
						g_stClientInfo.dRear = 0;
						g_stClientInfo.szWBuf[0] = 0x00;
						break;
					}
				}
				else if(tOldTime > tCurTime)
				{
					log_print(LOGN_CRI, "F=%s:%s.%d: tOldTime[%lu] is bigger than tCurTime[%lu]", __FILE__, __FUNCTION__, __LINE__,
						tOldTime, tCurTime);
					exit(-4);
				}
				else
					sleep(1);
			} //while-loop end
		} // log-out 일 경우 재요청 처리하는 부분//

		// message queue check
		/*while(1)*/
		for(i=0; i<30; i++)
		{
			dRet = dIsRcvedMessage(&stMsgQ);
			if(dRet > 0)
			{
				dRet = dSndMsgProc(dServerSocket, &stMsgQ);
				if(dRet < 0)
				{
					/* If Client is SND SOCKET ERROR, re-connect server */
					dLogInStatus = DEF_LOGOUT;
					close(dServerSocket);
					break;
				}
			}
			else
				break;
		}

		// loop end check
		if(g_JiSTOPFlag != 1)
			FinishProgram(dServerSocket);
		else if(dRet < 0)
			continue;

		// TAM LOG check
#ifdef DTAF
		dCheckTamLog(dServerSocket);
#endif

		// socket check
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;

		memcpy(&fd_read_tmp, &g_readset, sizeof(fd_set));
		memcpy(&fd_write_tmp, &g_writeset, sizeof(fd_set));

		if( (dRet = select(dNumOfSocket, &fd_read_tmp, &fd_write_tmp, (fd_set *)0, &timeout)) > 0)
		{
			if(FD_ISSET(dServerSocket, &fd_write_tmp)) /* 재전송 처리*/
				write_proc(dServerSocket);

			if(FD_ISSET(dServerSocket, &fd_read_tmp))
			{
				dRet = dRecvPacket(dServerSocket);
				if(dRet < 0)
				{
					FD_CLR(dServerSocket, &g_readset);
					FD_CLR(dServerSocket, &g_writeset);
					close(dServerSocket);
					g_stClientInfo.dLastFlag = SOCKET_CLOSE;
					g_stClientInfo.szBuf[0] = 0x00;
					g_stClientInfo.dBufSize = 0;
					g_stClientInfo.dFront = 0;
					g_stClientInfo.dRear = 0;
					g_stClientInfo.szWBuf[0] = 0x00;
					dNumOfSocket = 0;

					log_print(LOGN_DEBUG,LH"[RECV SOCKET ERROR]:[SERVER CLOSE]",LT);

					dLogInStatus = DEF_LOGOUT;
					continue; /* IPAM SERVER 에 재연결 요청 */
				}
			}
		}
		else if(dRet == 0)
		{
			continue;
		}
		else
		{
			log_print(LOGN_DEBUG, "[CAN'T SELECT LOCAL ADDRESS][%d][%s]",
					errno, strerror(errno));
			FinishProgram(dServerSocket);
		}
	} // while-loop end

	FinishProgram(dServerSocket);

	return 0;
}

