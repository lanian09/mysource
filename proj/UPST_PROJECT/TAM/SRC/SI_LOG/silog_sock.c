/** A.1* FILE INCLUSION ***********************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>

#include "commdef.h"
#include "procid.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

#include "sockio.h"

#include "loglib.h"
#include "nsocklib.h"	/* stNetTuple */

#include "silog_func.h"	/* dHandle_RecvMsg() */

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
extern st_subsys_mng	*stSubSys;
/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/


/** 
 * 특정 client의 ci_log로 부터 받은 메시지를 머지하는 함수
 * 
 * @param stNet 
 * @param dIndex 
 * 
 * @return 
 */
int dRecvPacket(stNetTuple *stNet, int dIndex)
{
    int				n, size, dRet, sp;
    st_NTAFTHeader	stHeader;

    size = stNet[dIndex].dBufSize;
    n = read(stNet[dIndex].dSfd, &stNet[dIndex].szBuf[size], DEF_MAX_BUFLEN - size);
    if(n <= 0) {
        if(n == 0)
            return -1;
        else if(errno != EAGAIN)
            return -1;
        else
            return 0;
    } else if(size + n < NTAFT_HEADER_LEN) {
        stNet[dIndex].dBufSize = size + n;
        stNet[dIndex].szBuf[stNet[dIndex].dBufSize] = 0;
        return 0;
    } else {
        size += n;
        sp = 0;
        do 
		{
            memcpy(&stHeader, &stNet[dIndex].szBuf[sp], NTAFT_HEADER_LEN);
			if (stHeader.usTotlLen <= size - sp) {
                time(&stNet[dIndex].tLastTime);
                dRet = dHandle_RecvMsg(stHeader.usTotlLen,  &stNet[dIndex].szBuf[sp], &stHeader, stNet[dIndex].dIdx);
                if (dRet < 0)
                    return -1;

                sp += stHeader.usTotlLen;
            } else
                break;
        } while (size >= sp + NTAFT_HEADER_LEN); /* end of do while */

        /* Buffer set */
        if (size > sp) {
            stNet[dIndex].dBufSize = size - sp;
            memcpy(&stNet[dIndex].szBuf[0], &stNet[dIndex].szBuf[sp], size - sp);
        } else 
            stNet[dIndex].dBufSize = 0;
    } /* end of else */

    return 0;
} /* end of dRecvPacket */


/** 
 * 클아이언트의 접속허용 및 연결된 클라이언트들의 소켓에 이벤트의 발생여부 체크
 * 
 * @param stNet 
 * @param dSrvSfd 
 * @param Rfds 
 * @param Numfds 
 * 
 * @return 
 */
int Check_ClientEvent(stNetTuple *stNet, int *dSrvSfd, fd_set *Rfds, int *Numfds)
{
    int     		dRet, dSfd, i, dIdx;
    struct 	timeval timeout;
    fd_set  		fdRead;

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    memcpy((char *)&fdRead, (char *)Rfds, sizeof(*Rfds));
 	dRet = select(*Numfds, (fd_set *)&fdRead, (fd_set *)0,(fd_set *)0, (struct timeval *)&timeout);
   	if(dRet < 0) {
		log_print(LOGN_CRI,"select : FAILED IN select [%s]", strerror(errno));
        return -1;
    } /* end of if */

    if(FD_ISSET(*dSrvSfd, (fd_set *)&fdRead)) {
        dSfd = accept_sock(stNet, *dSrvSfd, Rfds, Numfds, DEF_MAX_BUFLEN, &dIdx);
        if(dSfd < 0) {
            log_print(LOGN_CRI, "Check_ClientEvent : FAILED IN accept_sockfd, [%d:%s]", errno, strerror(errno));
            return -1;
        } /* end of if */

        log_print(LOGN_DEBUG, "Check_ClientEvent: AFTER ACCEPT *Numfds=%d *dSrvSfd=%d IDX=%d",
            *Numfds, *dSrvSfd, dIdx);

		for(i = 0; i < MAX_RECORD; i++)
		{
			if(stSubSys->sys[i].uiIP == stNet[dIdx].uiIP) /* 등록된 IP인지 검사 */
				break;
		}

		if(i == MAX_RECORD) { /* 등록 안된 IP에 대한 예외 처리 */
			log_print(LOGN_CRI, "NOT SUPPORT TAF NO dGetNtafNo : IDX=%d dSfd=%d uiIP=%u i=%d", 
				dIdx, stNet[dIdx].dSfd, stNet[dIdx].uiIP, i);
			if(disconn_sock(stNet, stNet[dIdx].dSfd, Rfds, Numfds, *dSrvSfd) < 0)
				log_print(LOGN_CRI, "Check_ClientEvent : FAILED IN disconn_sock");
			return 0;
		}

		stNet[dIdx].dIdx = stSubSys->sys[i].usSysNo;
		log_print(LOGN_CRI, "ACCEPT IDX=%d Numfds[%d] *dSrvSfd[%d] dSfd[%d] uiIP[%u] SysNo[%d]", 
			dIdx, *Numfds, *dSrvSfd, stNet[dIdx].dSfd, stNet[dIdx].uiIP, stNet[dIdx].dIdx);
    } /* end of if */

    for(i = 0; i < MAX_RECORD; i++)
    {
        if(	(stNet[i].dSfd > 0) && (FD_ISSET(stNet[i].dSfd, (fd_set *)&fdRead))) { /* 이벤트가 발생한 소켓에 대한 처리 */
            dRet = dRecvPacket(stNet, i);
            if(dRet < 0) {
                if(disconn_sock(stNet, stNet[i].dSfd, Rfds, Numfds, *dSrvSfd) < 0)
					log_print(LOGN_CRI, "Check_ClientEvent : RCV PCK FAILED IN DISCONN");
			} /* end of if */
        } /* end of if */
    } /* end of for */

    return 1;

} /* end of Check_ClientEvent */


/** 
 * 클라이언트를 체크하는 함수 
 * 클라이언트로 메시지를 보낸후 응답하면 tLastTime를 연장 
 * 그렇지 않으면 disconn_sock를 호출하여 연결을 끊는다.
 * 
 * @param stNet 		
 * @param dIdx 			stNet index
 * @param fdSet 
 * @param NumFds 		
 * @param dSrvSfd 		
 * 
 * @return 
 */
int dSendCheck(stNetTuple *stNet, int dIdx, fd_set *fdSet, int *NumFds, int dSrvSfd)
{
	int				dRet;
	st_NTAFTHeader	stHeader;

    stHeader.llMagicNumber = MAGIC_NUMBER;
    stHeader.usBodyLen = 0;
    stHeader.usTotlLen = NTAFT_HEADER_LEN;
	dRet = write(stNet[dIdx].dSfd, (char *)&stHeader, NTAFT_HEADER_LEN);
	if(dRet < 0) {
		if(errno != EAGAIN) {
			log_print(LOGN_CRI, "dSendCheck : FAILED IN write SFD[%d]", 
			stNet[dIdx].dSfd);
			disconn_sock(stNet, stNet[dIdx].dSfd, fdSet, NumFds, dSrvSfd);
		} 
	} else
		stNet[dIdx].tLastTime = time(NULL);

	return 1;
} /* end of dSendCheck */
