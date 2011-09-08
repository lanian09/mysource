/**
	@file		fstat_init.h	
	@author		
	@version
	@date		
	@brief		fstat_init.c 헤더파일
*/

/**
 *	Include headers
 */

/* SYS HEADER */
#include <arpa/inet.h>
#include <errno.h>
#include <mysql/mysql.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
/* LIB HEADER */
#include "commdef.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"
#include "dblib.h"
#include "loglib.h"
#include "nsocklib.h"
/* PRO HEADER */
#include "path.h"
#include "msgdef.h"
#include "procid.h"
#include "sshmid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "fstat_init.h"

/**
 *	Declare var.
 */
extern int					JiSTOPFlag;
extern int					FinishFlag;

extern short				gdTimeFlag;	/* First: 0 Second: 1 */
extern unsigned long long	timerNID;

extern int					gdSHMLOADID;
extern int					gdSHMFAULTID;
extern int					gdSHMTRAFFICID;

extern st_LOADLIST			*pstSHMLOADTbl;
extern st_FAULTLIST			*pstSHMFAULTTbl;
extern st_SubSysInfoList	gstSubList;

extern stTIMERNINFO			*pTIMER;
extern MYSQL				stMySQL;

extern stMEMSINFO			*gpRECVMEMS;
extern stCIFO				*pCIFO;

/**
 *	Declare func.
 */
extern void SendMsg_SI_NMS(void *p);
extern stTIMERNINFO * timerN_init(unsigned int uiMaxNodeCnt,unsigned int uiArgMaxSize);
extern void Stat_LOADFunc(void* p);
extern void Stat_FAULTFunc(void* p);
extern void InitSHMLOAD(short TimeFlag, unsigned int uiStatTime);

/**
 *	Implements func.
 */
void InitProc(void)
{
	int			dRet, dStartTime;
	BLOCK_KEY	BLOCKKEY;
	time_t		tCurrent;

	dRet = dReadSubSysInfoFile(&gstSubList);

	if( (pTIMER = timerN_init(MAX_STAY_CNT, sizeof(BLOCK_KEY))) == NULL)
	{
		log_print(LOGN_CRI, "[%s][%s,%d] dtimerN_init NULL", __FILE__, __FUNCTION__, __LINE__);
		exit(0);
	}

	tCurrent = time(NULL);
	if( (tCurrent-(tCurrent/300*300)) > 60)
		dStartTime	= 360;
	else
		dStartTime	= 60;

	BLOCKKEY.usLoadFaultFlag	= MID_STAT_LOAD;
	BLOCKKEY.usgdTimeFlag		= DEF_FIRST_TIMEFLAG;
	timerNID = timerN_add(pTIMER, Stat_LOADFunc, (unsigned char*)&BLOCKKEY, sizeof(BLOCK_KEY), tCurrent/300*300+dStartTime);

	BLOCKKEY.usLoadFaultFlag	= MID_STAT_FAULT;
	BLOCKKEY.usgdTimeFlag		= DEF_FIRST_TIMEFLAG;
	timerNID = timerN_add(pTIMER, Stat_FAULTFunc, (unsigned char*)&BLOCKKEY, sizeof(BLOCK_KEY), tCurrent/300*300+dStartTime);

	timerNID = timerN_add(pTIMER, SendMsg_SI_NMS, (unsigned char*)&BLOCKKEY, sizeof(BLOCK_KEY), tCurrent/300*300+dStartTime+60);

	BLOCKKEY.usgdTimeFlag		= MID_CHG_TIME;
	timerNID = timerN_add(pTIMER, Chg_TimeFlag,  (unsigned char*)&BLOCKKEY, sizeof(BLOCK_KEY), tCurrent/300*300+300);

	InitSHMLOAD(0, time(NULL)/300*300);
	InitSHMLOAD(1, time(NULL)/300*300+300);
	InitSHMFAULT(0, time(NULL)/300*300);
	InitSHMFAULT(1, time(NULL)/300*300+300);

	log_print(LOGN_INFO, "INIT PROC FSTAT");
}

int dInit_ipcs(void)
{
	/* LOAD DATA MANAGEMENT SHARED MEMORY CREATE or ATTACH*/
	if( (gdSHMLOADID = (int)shmget(S_SSHM_LOAD, sizeof(st_LOADLIST), IPC_CREAT|0666)) <0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN shmget(S_SSHM_LOAD[0x%04X]) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, S_SSHM_LOAD, errno, strerror(errno));
		return -1;
	}

	if( (pstSHMLOADTbl=(st_LOADLIST*)shmat(gdSHMLOADID, 0, 0)) == (void*)-1)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN shmat(gdSHMLOADID[%d]) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, gdSHMLOADID, errno, strerror(errno));
		return -2;
	}

	/* FAULT DATA MANAGEMENT SHARED MEMORY CREATE or ATTACH*/
	if( (gdSHMFAULTID = (int)shmget(S_SSHM_FAULT, sizeof(st_FAULTLIST), IPC_CREAT|0666)) <0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN shmget(S_SSHM_FAULT[0x%04X]) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, S_SSHM_FAULT, errno, strerror(errno));
		return -3;
	}

	if( (pstSHMFAULTTbl=(st_FAULTLIST*)shmat(gdSHMFAULTID, 0, 0)) == (void*)-1)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN shmat(gdSHMFAULTID[%d]) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, gdSHMFAULTID, errno, strerror(errno));
		return -4;
	}

	/* TRAFFIC DATA MANAGEMENT SHARED MEMORY CREATE or ATTACH*/
	if( (gdSHMTRAFFICID = (int)shmget(S_SSHM_TRAFFICSTAT, sizeof(st_FAULTLIST), IPC_CREAT|0666)) <0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN shmget(S_SSHM_TRAFFICSTAT[0x%04X]) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, S_SSHM_TRAFFICSTAT, errno, strerror(errno));
		return -5;
	}

	/* msgq ==> gifo */
	// GIFO 를 사용하기 위한 nifo_zone 설정
	gpRECVMEMS = nifo_init_zone((U8*)"MMCD", SEQ_PROC_MMCD, FILE_NIFO_ZONE);
	if(gpRECVMEMS == NULL)
	{
		log_print(LOGN_CRI, "FAILED IN nifo_init_zone, NULL");
		return -6;
	}

	// GIFO를 사용하기 위한 group 설정
	pCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
	if(pCIFO == NULL)
	{
		log_print(LOGN_CRI, "FAILED IN gifo_init_group, RET=NULL, cifo=%s, gifo=%s",
					FILE_CIFO_CONF, FILE_GIFO_CONF);
		return -7;
	}

	/*
	if( (gd_MyQid = msgget(S_MSGQ_FSTAT, 0666 | IPC_CREAT)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN msgget(S_MSGQ_FSTAT[0x%04X]) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			S_MSGQ_FSTAT, errno, strerror(errno));
		return -5;
	}

	if( (gdSINMSQid = msgget(S_MSGQ_SI_NMS, 0666 | IPC_CREAT)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN msgget(S_MSGQ_SI_NMS[0x%04X]) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			S_MSGQ_SI_NMS, errno, strerror(errno));
		return -6;
	}
	*/	

	return 0;
}


/*******************************************************************************

*******************************************************************************/
void SetUpSignal(void)
{
	JiSTOPFlag = 1;

	/* WANTED SIGNALS   */
	signal(SIGTERM, UserControlledSignal);
	signal(SIGINT, UserControlledSignal);
	signal(SIGQUIT, UserControlledSignal);

	/* UNWANTED SIGNALS */
	signal(SIGHUP, IgnoreSignal);
	signal(SIGALRM, IgnoreSignal);
	signal(SIGPIPE, IgnoreSignal);
	signal(SIGPOLL, IgnoreSignal);
	signal(SIGPROF, IgnoreSignal);
	signal(SIGUSR1, IgnoreSignal);
	signal(SIGUSR2, IgnoreSignal);
	signal(SIGVTALRM, IgnoreSignal);
	signal(SIGCLD, SIG_IGN);

	log_print(LOGN_CRI, "F:%s:%s.%d: SUCCESS IN SetUpSignal() JiSTOPFlag[%d]", __FILE__, __FUNCTION__, __LINE__, JiSTOPFlag);
} /* end of SetUpSignal */


/*******************************************************************************

*******************************************************************************/
void UserControlledSignal(int sign)
{
    JiSTOPFlag = 0;
    FinishFlag = sign;

    log_print(LOGN_CRI, "UserControlledSignal : [%d]", sign);
} /* end of UserControlledSignal */


/*******************************************************************************

*******************************************************************************/
void FinishProgram(void)
{
    log_print(LOGN_CRI, "F=%s:%s.%d: PROGRAM IS NORMALLY TERMINATED, Cause = signal number(%d)", __FILE__, __FUNCTION__, __LINE__,
		FinishFlag);

	//dDisConnectMySQL(&stMySQL);
	db_disconn(&stMySQL);

} /* end of FinishProgram */


/*******************************************************************************

*******************************************************************************/
void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print(LOGN_CRI,
        "IgnoreSignal : UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
} /* end of IgnoreSignal */

/*******************************************************************************

*******************************************************************************/

int dReadSubSysInfoFile(pst_SubSysInfoList pstList)
{
    int     htohIP, dCnt, dLine;
    FILE    *fa;
    char    szBuf[1024], szInfo_1[64], szInfo_2[64], szInfo_3[64];
    char    szInfo_4[64], szInfo_5[64], szInfo_6[64];

    fa = fopen(DATA_PATH"/SUBSYS_INFO.dat", "r");
    if(fa == NULL) {
        log_print(LOGN_CRI,"dRead_FLT_Tmf : %sSUBSYS_INFO.dat FILE OPEN FAIL (%s)",
            DATA_PATH, strerror(errno));
        return -1;
    }

    dCnt = 0; dLine = 0;
    while(fgets(szBuf,1024,fa) != NULL)
    {
        if(szBuf[0] != '#') {
            log_print(LOGN_CRI,"dRead_FLT_Tmf : %sSUBSYS_INFO.dat File [%d] ROW FORMAT ERR", DATA_PATH, dCnt+1);
            fclose(fa);
            return -1;
        }

        if(szBuf[1] == '#')
            continue;
        else if(szBuf[1] == 'E')
            break;
        else if(szBuf[1] == '@') {
            if(sscanf(&szBuf[2],"%s %s %s %s %s %s",
                szInfo_1, szInfo_2, szInfo_3, szInfo_4, szInfo_5, szInfo_6) == 6) {
                if(strcmp(szInfo_1,"TMF") == 0 || strcmp(szInfo_1, "TMP") == 0) {
                    if(dCnt >= MAX_SUBSYS_NUM) {
                        log_print(LOGN_CRI,
                            "SUB MAX DISCARD i=%d INFO1=%s INFO2=%s INFO3=%s INFO4=%s INFO5=%s",
                            dLine, szInfo_1, szInfo_2, szInfo_3, szInfo_4, szInfo_5);
                        break;
                    }
                    pstList->stInfo[dCnt].dType = atoi(szInfo_2);
                    pstList->stInfo[dCnt].dNo = atoi(szInfo_3);
                    if(pstList->stInfo[dCnt].dNo < 1) {
                        log_print(LOGN_CRI,
                            "SUB NUM VALUE DISCARD i=%d INFO1=%s INFO2=%s INFO3=%s INFO4=%s INFO5=%s",
                            dLine, szInfo_1, szInfo_2, szInfo_3, szInfo_4, szInfo_5);
                        continue;
                    }

                    htohIP =  inet_addr(szInfo_4);
                    pstList->stInfo[dCnt].uiIP =  ntohl(htohIP);
                    pstList->stInfo[dCnt].dFlag = atoi(szInfo_5);
                    strcpy(pstList->stInfo[dCnt].szDesc, szInfo_6);
                    log_print(LOGN_CRI, "SUB TAF INFO POS=%d TYPE=%d NO=%d IP=%u:%s FLAG=%d DESC=%s",
                        dCnt, pstList->stInfo[dCnt].dType, pstList->stInfo[dCnt].dNo,
                        pstList->stInfo[dCnt].uiIP, szInfo_4, pstList->stInfo[dCnt].dFlag,
                        pstList->stInfo[dCnt].szDesc);
                    dCnt++;
                } else {
                    log_print(LOGN_CRI,
                        "SUB DISCARD i=%d INFO1=%s INFO2=%s INFO3=%s INFO4=%s INFO5=%s INFOO6=%s",
                        dLine, szInfo_1, szInfo_2, szInfo_3, szInfo_4, szInfo_5, szInfo_6);
                }
            }
        }
        dLine++;
    }

    fclose(fa);

    pstList->dCount = dCnt;
    if(pstList->dCount < 1) {
        log_print(LOGN_CRI, "READ SUBSYS INFO NO DATA : CNT=%d", pstList->dCount);
        return -1;
    }

    log_print(LOGN_CRI, "READ SUBSYS INFO DATA : CNT=%d", pstList->dCount);

    return 0;
}

/*******************************************************************************

*******************************************************************************/
void Chg_TimeFlag(void *p)
{
	BLOCK_KEY	BLOCKKEY;

	gdTimeFlag	= gdTimeFlag^0x01;

	BLOCKKEY.usLoadFaultFlag	= MID_CHG_TIME;
	BLOCKKEY.usgdTimeFlag		= gdTimeFlag;

	timerNID = timerN_add(pTIMER, Chg_TimeFlag, (unsigned char*)&BLOCKKEY, sizeof(BLOCK_KEY), time(NULL)/300*300+300);
}
