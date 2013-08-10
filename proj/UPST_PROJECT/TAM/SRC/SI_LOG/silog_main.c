
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

#include "path.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

//#include "msgdef.h"	/* S_PORT_SI_LOG */
#include "sockio.h"	/* S_PORT_SI_LOG */

#include "loglib.h"
#include "verlib.h"
#include "nsocklib.h"	/* stNetTuple */

#include "silog_init.h"	/* dInitProc() */
#include "silog_sock.h"	/* Check_Client() */

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
int  			JiSTOPFlag;
int  			Finishflag;

stNetTuple  	stNet[MAX_RECORD];

int 			dSrvSfd;
int				Rfds;

int				Numfds;
time_t			tLastTime;

stMEMSINFO		*pMEMSINFO;
stCIFO			*gpCIFO;

/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/

int main()
{
	int		dRet, i, dSrvSfd, Numfds=0;
	time_t	tCurTime, tLastTime;
	fd_set	Rfds;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_SI_LOG, LOG_PATH"/SI_LOG", "SI_LOG");


	dRet = dInitProc();
	if(dRet < 0) {
		log_print(LOGN_CRI, "MAIN : FAILED IN dInitProc dRet[%d]", dRet);
		exit(1);
	}
	
	memset(&Rfds, 0, sizeof(Rfds));
	dRet = init_sock(&dSrvSfd, S_PORT_SI_LOG, &Rfds, &Numfds); 
	if(dRet < 0) {
		log_print(LOGN_CRI, "MAIN : FAILED IN dInitSock dRet[%d]", dRet);
		return -1;
	} /* end of if */

	for(i = 0; i< MAX_RECORD; i++)
	{
		stNet[i].tLastTime = 0;
		stNet[i].dStatus   = 0;
		stNet[i].dSfd      = 0;
		stNet[i].uiIP      = 0;
		stNet[i].dBufSize  = 0;
		stNet[i].dIdx      = 0;
	} /* end of for */

	dRet = set_version( S_SSHM_VERSION, SEQ_PROC_SI_LOG, "R4.0.0" );
    if(dRet < 0) {
        log_print(LOGN_WARN,"MAIN : Failed in Initialize Version Info [%d]", dRet );
        exit(1);
    }

	log_print(LOGN_CRI, "MAIN : [SI_LOG PROCESS INITIAL SUCCESS]");
			
	tLastTime = time(NULL);

	while(JiSTOPFlag)
    {
		for(i = 0; i < 100000; i++)
		{
			dRet = Check_ClientEvent(stNet, &dSrvSfd, &Rfds, &Numfds);
			if(dRet < 0) {
				log_print(LOGN_CRI, "MAIN : CHECK CLIENT=%d", dRet);
				FinishProgram();
        	}
		}

		tCurTime = time(NULL);
		if(tCurTime >= tLastTime + DEF_TIMER) {
            /**
             * 클라이언트 체크 부분 
             * 5초(DEF_TIMER)가 지날때 마다 클라이언트를 체크 한다.
             */
			for(i = 0; i < MAX_RECORD; i++)
			{
				if((stNet[i].dSfd > 0) && (tCurTime >= stNet[i].tLastTime + DEF_TIMER))
					dSendCheck(stNet, i, &Rfds, &Numfds, dSrvSfd);	
			}
			tLastTime = time(NULL);
		}
		
		usleep(0);
	}

	FinishProgram();

	return 0;
}

