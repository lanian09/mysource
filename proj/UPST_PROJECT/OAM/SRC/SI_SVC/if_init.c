/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>		/* FILE */
#include <stdlib.h>		/* EXIT */
#include <signal.h>		/* SIGNAL */
#include <string.h>		/* STRERROR(3) */
#include <errno.h>		/* errno */
#include <arpa/inet.h>	/* htonl() BYTEORDER(3) */
/* LIB HEADER */
#include "clisto.h"		/* U8 */
#include "mems.h"		/* stMEMSINFO */
#include "cifo.h"		/* stCIFO */
#include "gifo.h"		/* gifo_init_group() */
#include "nifo.h"		/* nifo_init_zone() */
#include "commdef.h"	/* FILE_SUB_SYS */
#include "filedb.h"		/* MAX_CH_COUNT */
#include "loglib.h"
#include "ipclib.h"		/* shm_init() */
/* PRO HEADER */
#include "sshmid.h"
#include "msgdef.h"		/* S_MSGQ_* */
#include "path.h"
#include "sockio.h"		/* st_subsys */
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "if_init.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
extern stMEMSINFO *gpMEMSINFO;
extern stCIFO	  *gpCIFO;

extern pst_NTAM    fidb;
extern st_subsys   stSubSys[MAX_CH_COUNT];

extern int gdSysNo;
extern int JiSTOPFlag;
extern int Finishflag;

/** E.1* DEFINITION OF FUNCTIONS **************************/
int dInitSubSysInfo();

/** E.2* DEFINITION OF FUNCTIONS **************************/

int Init_Fidb()
{
    int    dRet, i;
    time_t now;

    if( (dRet = shm_init(S_SSHM_FIDB, DEF_WNTAM_SIZE, (void**)&fidb)) < 0 ){
        log_print(LOGN_CRI, LH"FAILED IN shm_init(S_SSHM_FIDB=0x%x], dRet=%d)"EH,
            LT, S_SSHM_FIDB, dRet, ET);
        return -1;
    }

    if( dRet != SHM_EXIST ){
        time(&now);

        for(i = 0; i < 256; i++ ){
            fidb->tEventUpTime[i] = now;
        }

        log_print(LOGN_CRI, LH"Initialized a FIDB tEventUpTime=%ld", LT, now);
    }

    return 0;
}

int dInit_IPC()
{

	gpMEMSINFO = nifo_init_zone((U8*)"SI_SVC", SEQ_PROC_SI_SVC, FILE_NIFO_ZONE);
    if( gpMEMSINFO == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN nifo_init, NULL", LT);
        return -1;
    }   
            
    //GIFO를 사용하기 위한 group 설정
    gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
    if( gpCIFO == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group. cifo=%s, gifo=%s",
                LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }
	
	return 1;
} /* end of dInit_IPC */

int dInitProc()
{
    int         dRet;

    SetUpSignal();

    dRet = dInit_IPC();
    if(dRet < 0)
        return -1;

	dRet = dInitSubSysInfo();
    if(dRet < 0)
        return -2;

    return 0;
} /* end of dInitProc */

void SetUpSignal()
{
	JiSTOPFlag = 1;

    /* WANTED SIGNALS   */
    signal(SIGTERM, UserControlledSignal);
    signal(SIGINT,  UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    /* UNWANTED SIGNALS */
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);

	log_print(LOGN_DEBUG, "SetUpSignal : SIGNAL HANDLER WAS INSTALLED[%d]", JiSTOPFlag);

    return;
} /* end of SetUpSignal */

void UserControlledSignal(int sign)
{
    JiSTOPFlag = 0;
    Finishflag = sign;
} /* end of UserControlledSignal */

void FinishProgram()
{
    log_print(LOGN_CRI,
	"FinishProgram : PROGRAM IS NORMALLY TERMINATED, Cause = %d", Finishflag);
    exit(0);
} /* end of FinishProgram */

void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print(LOGN_CRI,
		"IgnoreSignal : UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
} /* end of IgnoreSignal */

int dReadSubSysInfo(char *szFileName)
{
    int   htohIP, dCnt, dLine;
    FILE *fa;
    char  szBuf[1024], szInfo_1[64], szInfo_2[64], szInfo_3[64];
    char  szInfo_4[64], szInfo_5[64], szInfo_6[64];

    fa = fopen(szFileName, "r");
    if(fa == NULL) {
        log_print(LOGN_CRI,"FAILED IN SUBSYS_INFO.dat FILE OPEN FAIL (%s)",
        	strerror(errno));
        return -1;
    }

	dCnt	= 0;
	dLine	= 0;
	while(fgets(szBuf,1024,fa) != NULL)
	{
		if(szBuf[0] != '#')
		{
            log_print(LOGN_CRI, "F=%s:%s.%d: File(%s) %d row format error", __FILE__, __FUNCTION__, __LINE__, szFileName, dCnt+1);
			fclose(fa);
			return -1;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if(sscanf(&szBuf[2], "%s %s %s %s %s %s", szInfo_1, szInfo_2, szInfo_3, szInfo_4, szInfo_5, szInfo_6) == 6) {

				if( !strcmp(szInfo_1,"TMF") || !strcmp(szInfo_1, "TMP") ) {

					if(dCnt >= MAX_CH_COUNT) {
						log_print(LOGN_CRI, "SUB MAX DISCARD i=%d INFO1=%s INFO2=%s INFO3=%s INFO4=%s INFO5=%s",
							dLine, szInfo_1, szInfo_2, szInfo_3, szInfo_4, szInfo_5);
						break;
					}
					stSubSys[dCnt].dType   = atoi(szInfo_2);
					stSubSys[dCnt].usSysNo = atoi(szInfo_3);
					if( stSubSys[dCnt].usSysNo < 1 ){
						log_print(LOGN_CRI, "SUB NUM VALUE DISCARD i=%d INFO1=%s INFO2=%s INFO3=%s INFO4=%s INFO5=%s",
							dLine, szInfo_1, szInfo_2, szInfo_3, szInfo_4, szInfo_5);
						continue;
					}

					htohIP						= inet_addr(szInfo_4);
					stSubSys[dCnt].uiIP  = ntohl(htohIP);
					stSubSys[dCnt].dFlag = atoi(szInfo_5);
					strcpy(stSubSys[dCnt].szDesc, szInfo_6);
					log_print(LOGN_CRI, "SUB TAF INFO POS=%d TYPE=%d NO=%d IP=%u:%s FLAG=%d DESC=%s",
						dCnt, stSubSys[dCnt].dType, stSubSys[dCnt].usSysNo, stSubSys[dCnt].uiIP, szInfo_4, 
						stSubSys[dCnt].dFlag, stSubSys[dCnt].szDesc);
					dCnt++;
				} else{

					log_print(LOGN_CRI, "SUB DISCARD i=%d INFO1=%s INFO2=%s INFO3=%s INFO4=%s INFO5=%s INFOO6=%s", 
						dLine, szInfo_1, szInfo_2, szInfo_3, szInfo_4, szInfo_5, szInfo_6);
				}
			}
		}
		dLine++;
	}

	fclose(fa);
	if( dCnt < 1 ){
		log_print(LOGN_CRI, "READ SUBSYS INFO NO DATA : CNT=%d", dCnt);
		return -1;
	}

	log_print(LOGN_CRI, "READ SUBSYS INFO DATA : CNT=%d", dCnt);
	return dCnt;
}

int dInitSubSysInfo()
{
	int	  dRet;

	if( (dRet = dReadSubSysInfo(FILE_SUB_SYS)) < 0)
	{
		log_print(LOGN_CRI,"FAILED IN dReadSubSysInfo. dRet=%d", dRet);
		return -1;
	}
	return 0;
}
