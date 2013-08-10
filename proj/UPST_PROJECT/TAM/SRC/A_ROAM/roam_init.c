/**
 *	INCLUDE HEADER FILES
 */
#include <stdio.h>
#include <errno.h>
#include <signal.h>

// DQMS
#include "common_stg.h"
#include "procid.h"
#include "sshmid.h"

// LIB
#include "mems.h"
#include "memg.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"
#include "loglib.h"
#include "ipclib.h"

// OAM
#include "sshmid.h"
#include "msgdef.h"
#include "path.h"

// TAM
#include "filter.h"
#include "watch_filter.h"
#include "rppi_def.h"

// .
#include "roam_init.h"

/**
 *	DECLARE VARIABLES
 */
extern stMEMSINFO		*pMEMSINFO;
extern stCIFO			*pCIFO;
extern stHASHOINFO		*pHASHOINFO;
extern stHASHOINFO		*pMODELINFO;
extern stHASHOINFO		*pMODELINFO1;
extern stHASHOINFO		*pMODELINFO2;
extern stHASHOINFO		*pIRMINFO;
extern stTIMERNINFO 	*pTIMERNINFO;
extern stHASHOINFO  	*pNASIPINFO;
extern stHASHOINFO  	*pDEFECTINFO;
extern st_Flt_Info		*flt_info;
extern st_WatchFilter	*gWatchFilter;
extern int				giStopFlag;
extern int				giFinishSignal;


/**
 *	Declare funstions
 */
extern void invoke_del_call(void *p);


/**
 *	IMPLEMENT FUNCTIONS
 */
int dInitRPPI()
{
	S32 	dRet;
    
	SetUpSignal();

	// GIFO 를 사용하기 위한 nifo_zone 설정
	pMEMSINFO = nifo_init_zone((U8*)"A_ROAM", SEQ_PROC_A_ROAM, FILE_NIFO_ZONE);
	if(pMEMSINFO == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone, NULL", LT);
		return -1;
	}

	// CIFO 그룹 설정
	pCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
	if(pCIFO == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, RET=NULL, cifo=%s, gifo=%s", LT,
					FILE_CIFO_CONF, FILE_GIFO_CONF);
		return -2;
	}

	/* st_WatchFilter */
	if((dRet = shm_init(S_SSHM_WATCH_FILTER, sizeof(st_WatchFilter), (void**)&gWatchFilter)) < 0)
	{
		log_print(LOG_CRI, LH"dInitWatchFilterShm dRet=%d:%s", LT, dRet, strerror(-dRet));
		return -3;
	}

	/* NEW CREATE */
	gWatchFilter->stModelInfoList.dActiveStatus = 1;
	pMODELINFO = pMODELINFO1;


    /* CALL Hash Table 초기화 */
    /* CALL 관리를 위한 구조체를 만들어야 함 */

	if((pHASHOINFO = hasho_init( S_SSHM_A_ROAM, RPPISESS_KEY_SIZE, RPPISESS_KEY_SIZE,
        HDATA_RPPI_SIZE, MAX_RPPISESS_CNT, 0)) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -5;
    }
    if((pTIMERNINFO = timerN_init(MAX_RPPISESS_CNT, sizeof(RPPI_TIMER))) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] timerN_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -6;
    }

	vRPPITimerReConstruct(pHASHOINFO, pTIMERNINFO);

    /* IRM 관리 Hash Table */
    if((pIRMINFO = hasho_init(0, DEF_IRMHASH_KEY_SIZE, DEF_IRMHASH_KEY_SIZE,
        DEF_IRMHASH_DATA_SIZE, DEF_IRMHASH_CNT, 0)) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -7;
    }

    /* NASIP 관리 Hash Table */
    if((pNASIPINFO = hasho_init( 0, sizeof(unsigned int), sizeof(unsigned int),
        HDATA_NASIP_SIZE, HASH_NASIP_CNT, 0)) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -8;
    }

    /* Defect Code 관리 Hash Table */
    if((pDEFECTINFO = hasho_init( 0, sizeof(unsigned int), sizeof(unsigned int),
        HDATA_DEFECT_SIZE, HASH_DEFECTINFO_CNT, 0)) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -9;
    }

    /* Model 관리 Hash Table */
    if((pMODELINFO1 = hasho_init( S_SSHM_MODELHASH1, RPPISESS_KEY_SIZE, RPPISESS_KEY_SIZE,
        HDATA_MODEL_SIZE, HASH_MODELINFO_CNT, 0)) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -10;
    }

	if((pMODELINFO2 = hasho_init( S_SSHM_MODELHASH2, RPPISESS_KEY_SIZE, RPPISESS_KEY_SIZE,
        HDATA_MODEL_SIZE, HASH_MODELINFO_CNT, 0)) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -11;
    }

	if( gWatchFilter->stModelInfoList.dActiveStatus == 1 )
        pMODELINFO = pMODELINFO1;
	else if( gWatchFilter->stModelInfoList.dActiveStatus == 2 )
		pMODELINFO = pMODELINFO2;

	return 0;
}

void SetUpSignal(void)
{
    giStopFlag = 1;

    /* WANTED SIGNALS */
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
    signal(SIGCLD, IgnoreSignal);
}

void UserControlledSignal(int isign)
{
    giStopFlag = 0;
    giFinishSignal = isign;
    log_print(LOGN_CRI, "User Controlled Signal Req = %d", isign);
}

void FinishProgram(void)
{
    log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", giFinishSignal);
    exit(0);
}

void IgnoreSignal(int isign)
{
    if (isign != SIGALRM)
    {
        log_print(LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", isign);
    }

    signal(isign, IgnoreSignal);
}

void vRPPITimerReConstruct(stHASHOINFO *pRPPIHASH, stTIMERNINFO *pTIMER)
{
	int					i;
	OFFSET				offset;
	stHASHONODE			*p;
	stMEMGNODEHDR		*pMEMGNODEHDR;

	RPPI_TIMER			RPPITIMER;
	RPPISESS_KEY		*pKey;
	HData_RPPI			*pData;

	/* IPFRAG */
	log_print(LOGN_INFO, "REBUILD ROAM TIMER hashcnt=%u", pRPPIHASH->uiHashSize);
	for(i = 0; i < pRPPIHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pRPPIHASH, pRPPIHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pRPPIHASH, offset);

			pKey = (RPPISESS_KEY *)HASHO_PTR(pRPPIHASH, p->offset_Key);
			pData = (HData_RPPI *)HASHO_PTR(pRPPIHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				if(pData->before.dOffset != 0) {
					log_print(LOGN_INFO, "REBUILD ROAM TIMER BEFORE IMSI=%s CALLTIME=%u.%u", 
						pKey->szIMSI, pData->before.uiCallTime, pData->before.uiCallMTime);

					memcpy(&RPPITIMER.RPPIKEY.szIMSI, pKey->szIMSI, MAX_MIN_SIZE-1);
					RPPITIMER.RPPIKEY.szIMSI[MAX_MIN_SIZE-1] = 0x00;
					RPPITIMER.uiCallTime = pData->before.uiCallTime;
					RPPITIMER.uiCallMTime = pData->before.uiCallMTime;
					pData->before.timerNID = timerN_add(pTIMER, invoke_del_call, (U8*)&RPPITIMER, sizeof(RPPI_TIMER), time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT]);
				}

				if(pData->after.dOffset != 0) {
					log_print(LOGN_INFO, "REBUILD ROAM TIMER AFTER IMSI=%s CALLTIME=%u.%u", 
						pKey->szIMSI, pData->after.uiCallTime, pData->after.uiCallMTime);

					memcpy(&RPPITIMER.RPPIKEY.szIMSI, pKey->szIMSI, MAX_MIN_SIZE-1);
					RPPITIMER.RPPIKEY.szIMSI[MAX_MIN_SIZE-1] = 0x00;
					RPPITIMER.uiCallTime = pData->after.uiCallTime;
					RPPITIMER.uiCallMTime = pData->after.uiCallMTime;
					pData->after.timerNID = timerN_add(pTIMER, invoke_del_call, (U8*)&RPPITIMER, sizeof(RPPI_TIMER), time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT]);
				}
			}
			offset = p->offset_next;
		}
	}
}
