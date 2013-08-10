/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <time.h>		/* time_t */
#include <errno.h>		/* errno */
#include <unistd.h>		/* GETPID(2) */
#include <stdlib.h>		/* EXIT() */
#include <string.h>		/* memset */

/* LIB HEADER */
#include "commdef.h"	/* S_SSHM_* */
#include "filedb.h"		/* MAX_CH_COUNT */
#include "loglib.h"
#include "verlib.h"		/* set_version() */
/* PRO HEADER */
#include "sshmid.h"
#include "msgdef.h"		/* st_MsgQ, S_MSGQ_* */
#include "sockio.h"		/* st_FDInfo, S_PORT_SI_SVC */
#include "path.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "if_init.h"	/* Init_Fidb(), dInitProc() */
#include "if_msgq.h"	/* dHandleMsgAll(), dHandleMsg(), dIsRcvedMessage() */
#include "if_sock.h"	/* Check_ClientEvent(), dSendCheck(), dInitSockFd() */

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
st_WNTAM      stFIDB;
pst_NTAM      fidb = &stFIDB.stNTAM;
st_subsys     stSubSys[MAX_CH_COUNT];
st_ClientInfo stSock[MAX_LINK_COUNT];

int gdSysNo;
int JiSTOPFlag;
int Finishflag;
/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/

int main()
{
	int				i, dRet, dCHNIndex = 0;
	time_t			tCurTime, tLastTime;

	st_MsgQ			stMsgQ;
	st_FDInfo       stFD;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_SI_SVC, LOG_PATH"/SI_SVC", "SI_SVC");

	/* FIDB */
    if( Init_Fidb() < 0 ) {
        log_print(LOGN_CRI,"FAILED IN Init_FIDB()[%d:%s]",errno, strerror(errno));
        return -errno;
    }

	if( (dRet = set_version( S_SSHM_VERSION, SEQ_PROC_SI_SVC, "R4.0.0" )) < 0 ){
        log_print(LOGN_WARN,"MAIN : Failed in Initialize Version Info [%d]", dRet );
        exit(1);
    }

	/* PROCESS INITIAL : Msg_Q */
	dRet = dInitProc();
	if(dRet < 0) {
		log_print(LOGN_CRI, "MAIN : FAILED IN dInitProc dRet[%d]", dRet);
		exit(1);
	}

	memset(&stFD, 0x00, sizeof(st_FDInfo));

	dRet = dInitSockFd(&stFD, S_PORT_SI_SVC);
	if(dRet < 0) {
		log_print(LOGN_CRI, "MAIN : FAILED IN dInitSock dRet[%d]", dRet);
		return -1;
	} /* end of if */

	for(i = 0; i< MAX_CH_COUNT; i++)
	{
		stSock[i].tLastTime=0;
		stSock[i].dSysNo = 0;
		stSock[i].dSfd = 0;
		stSock[i].uiIP = 0;
		stSock[i].dFront = 0;
		stSock[i].dRear = 0;
		stSock[i].dBufSize = 0;
		stSock[i].dLastFlag = SOCKET_CLOSE;
	} /* end of for */

	log_print(LOGN_CRI, "MAIN : SI_SVC PROCESS INITIAL SUCCESS");

	/* PROCESS JOB START */
	tLastTime = time(NULL);
	while(JiSTOPFlag)
	{
		if( (dRet = Check_ClientEvent(stSock, &stFD)) < 0)
		{
			log_print(LOGN_CRI, "MAIN : [CAN'T SELECT LOCAL ADDRESS=%d", dRet);
			FinishProgram();
		}

		for(i = 0; i < 100; i++)
		{
			if( (dRet = dIsRcvedMessage(&stMsgQ)) < 0)
			{
				log_print(LOGN_CRI, "MAIN : FAILED IN dIsRcvedMessage dRet[%d]", dRet);
				FinishProgram();
			}
			else if(dRet > 0)
			{
				if(stMsgQ.ucNTAFID == DEF_BROAD_CAST)
					dHandleMsgAll(stSock, &stMsgQ, &stFD, dCHNIndex);
				else
					dHandleMsg(stSock, &stMsgQ, &stFD, dCHNIndex);
			}
			else
			{
				usleep(0);
				break;
			}
		}

		tCurTime = time(NULL);
		if(tCurTime >= (tLastTime+DEF_TIMER))
		{
			for(i = 0; i < MAX_CH_COUNT; i++)
			{
				if( (stSock[i].dSfd>0) && (tCurTime>=stSock[i].tLastTime+DEF_TIMER))
				dSendCheck(stSock, i, &stFD);
			}
			tLastTime = time(NULL);
		}
	}
	FinishProgram();

	return 0;
} /* end of main */
