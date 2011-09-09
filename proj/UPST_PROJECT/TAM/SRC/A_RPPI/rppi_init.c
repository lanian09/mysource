#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// LIB
#include "typedef.h"
#include "commdef.h"
#include "mems.h"
#include "memg.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "hasho.h"
#include "loglib.h"
#include "ipclib.h"

// PROJECT
#include "procid.h"
#include "sshmid.h"
#include "path.h"
#include "common_stg.h"

// TAM
#include "filter.h"			/* st_Flt_Info */
#include "watch_filter.h"	/* st_WatchFilter */
#include "rppi_def.h"		/* RPPISESS_KEY_SIZE, HDATA_RPPI_SIZE */

// . 
#include "rppi_init.h"
#include "rppi_func.h"
#include "rppi_msgq.h"
#include "rppi_util.h"		/* dSetPDSNIFHash() */

int             		giFinishSignal;     

extern st_Flt_Info		*flt_info;
extern st_WatchFilter 	*gWatchFilter;

extern int				giStopFlag;
extern stHASHOINFO  	*pMODELINFO;
extern stHASHOINFO  	*pMODELINFO1;
extern stHASHOINFO  	*pMODELINFO2;
extern stMEMSINFO   	*pMEMSINFO;
extern stCIFO			*gpCIFO;
extern stHASHOINFO  	*pHASHOINFO; 
extern stTIMERNINFO 	*pTIMERNINFO;
extern stHASHOINFO  	*pPCFINFO;
extern stHASHOINFO  	*pDEFECTINFO;
extern stHASHOINFO		*pPDSNIF_HASHO;

#ifdef _RPPI_MULTI_
extern int				gProcNum;
extern UINT				guiSeqProcKey;
extern char				gszMyProc[32];
#endif

int dInitRPPI()
{
	S32 	dRet;
	
	SetUpSignal();

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
        log_print(LOGN_CRI,LH"FAILED IN shm_init(FLT_INFO=0x%x)", LT, S_SSHM_FLT_INFO);
        return -1;
    }

	/* st_WatchFilter */
	if( shm_init(S_SSHM_WATCH_FILTER, sizeof(st_WatchFilter), (void**)&gWatchFilter) < 0 ){
        log_print(LOGN_CRI,LH"FAILED IN shm_init(FLT_INFO=0x%x)", LT, S_SSHM_FLT_INFO);
        return -2;
    }

	gWatchFilter->stModelInfoList.dActiveStatus = 1;
	pMODELINFO = pMODELINFO1;

#ifdef _RPPI_MULTI_
	if((pMEMSINFO = nifo_init_zone((U8*)gszMyProc, guiSeqProcKey, FILE_NIFO_ZONE)) == NULL) {
		log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone NULL", LT);
		return -3;
	}
#else
	if((pMEMSINFO = nifo_init_zone((U8*)"A_RPPI", SEQ_PROC_A_RPPI, FILE_NIFO_ZONE)) == NULL) {
		log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone NULL", LT);
		return -3;
	}
#endif

	if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -4;
    }

    /* CALL Hash Table 초기화 */
    /* CALL 관리를 위한 구조체를 만들어야 함 */
#ifdef _RPPI_MULTI_
	if((pHASHOINFO = hasho_init( (S_SSHM_A_RPPI + gProcNum), RPPISESS_KEY_SIZE, RPPISESS_KEY_SIZE, HDATA_RPPI_SIZE, MAX_RPPISESS_CNT, 0)) == NULL) {
#else
	if((pHASHOINFO = hasho_init( S_SSHM_A_RPPI, RPPISESS_KEY_SIZE, RPPISESS_KEY_SIZE, HDATA_RPPI_SIZE, MAX_RPPISESS_CNT, 0)) == NULL) {
#endif
        log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -5;
    }

	log_print(LOGN_CRI, "A_RPPI ProcNum:%d S_SSHM_A_RPPI:%d", gProcNum, S_SSHM_A_RPPI + gProcNum);
    if((pTIMERNINFO = timerN_init(MAX_RPPISESS_CNT, sizeof(RPPI_TIMER))) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] timerN_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -6;
    }


	vRPPITimerReConstruct(pHASHOINFO, pTIMERNINFO);


    /* PCFIP 관리 Hash Table */
    if((pPCFINFO = hasho_init( 0, sizeof(unsigned int), sizeof(unsigned int),
        HDATA_PCF_SIZE, HASH_PCFINFO_CNT, 0)) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -7;
    }

    /* Defect Code 관리 Hash Table */
    if((pDEFECTINFO = hasho_init( 0, sizeof(unsigned int), sizeof(unsigned int),
        HDATA_DEFECT_SIZE, HASH_DEFECTINFO_CNT, 0)) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -8;
    }

    /* Model 관리 Hash Table */
    if((pMODELINFO1 = hasho_init( S_SSHM_MODELHASH1, RPPISESS_KEY_SIZE, RPPISESS_KEY_SIZE,
        HDATA_MODEL_SIZE, HASH_MODELINFO_CNT, 0)) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -9;
    }

	if((pMODELINFO2 = hasho_init( S_SSHM_MODELHASH2, RPPISESS_KEY_SIZE, RPPISESS_KEY_SIZE,
        HDATA_MODEL_SIZE, HASH_MODELINFO_CNT, 0)) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -10;
    }

	pPDSNIF_HASHO = hasho_init(0, DEF_PDSNIF_KEY_SIZE, DEF_PDSNIF_KEY_SIZE, DEF_PDSNIF_DATA_SIZE, MAX_PDSNIF_HASH_COUNT, 0);
	if(pPDSNIF_HASHO == NULL){
		return -11;
	}

	if( gWatchFilter->stModelInfoList.dActiveStatus == 1 )
        pMODELINFO = pMODELINFO1;
	else if( gWatchFilter->stModelInfoList.dActiveStatus == 2 )
		pMODELINFO = pMODELINFO2;

	dRet = dGetMapping();
	if(dRet < 0){
		return -12;
	}


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
	log_print(LOGN_INFO, "REBUILD RPPI TIMER hashcnt=%u", pRPPIHASH->uiHashSize);

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
					log_print(LOGN_INFO, "REBUILD RPPI TIMER BEFORE IMSI=%s CALLTIME=%u.%u", 
						pKey->szIMSI, pData->before.uiCallTime, pData->before.uiCallMTime);

					memcpy(&RPPITIMER.RPPIKEY.szIMSI, pKey->szIMSI, MAX_MIN_SIZE-1);
					RPPITIMER.RPPIKEY.szIMSI[MAX_MIN_SIZE-1] = 0x00;
					RPPITIMER.uiCallTime = pData->before.uiCallTime;
					RPPITIMER.uiCallMTime = pData->before.uiCallMTime;
					pData->before.timerNID = timerN_add(pTIMER, invoke_del_call, (U8*)&RPPITIMER, sizeof(RPPI_TIMER), time(NULL) + flt_info->stTimerInfo.usTimerInfo[RPPI_CALL_TIMEOUT]);
				}

				if(pData->after.dOffset != 0) {
					log_print(LOGN_INFO, "REBUILD RPPI TIMER AFTER IMSI=%s CALLTIME=%u.%u", 
						pKey->szIMSI, pData->after.uiCallTime, pData->after.uiCallMTime);

					memcpy(&RPPITIMER.RPPIKEY.szIMSI, pKey->szIMSI, MAX_MIN_SIZE-1);
					RPPITIMER.RPPIKEY.szIMSI[MAX_MIN_SIZE-1] = 0x00;
					RPPITIMER.uiCallTime = pData->after.uiCallTime;
					RPPITIMER.uiCallMTime = pData->after.uiCallMTime;
					pData->after.timerNID = timerN_add(pTIMER, invoke_del_call, (U8*)&RPPITIMER, sizeof(RPPI_TIMER), time(NULL) + flt_info->stTimerInfo.usTimerInfo[RPPI_CALL_TIMEOUT]);
				}
			}
		
			offset = p->offset_next;
		}
	}
}

int dGetMapping(void)
{
	FILE            *fa;
	char            szBuf[1024];
	unsigned int    pdsnID;
	char            strPIIP[32], strRPIP[32];
	int             i, dRet = 0;
	UINT            piIP, rpIP;
	struct in_addr  inaddr;

	fa = fopen(FILE_PIRP_MAP, "r");
	if(fa == NULL){
		log_print(LOGN_CRI , "[%s][%d][ERROR] fopen() Fail. FILE[%s] CAUSE[%s]", __FUNCTION__, __LINE__
				, FILE_PIRP_MAP, strerror(errno));
		return -1;
	}

	/* init */
	hasho_reset(pPDSNIF_HASHO);	

	i = 0;

	while(fgets(szBuf, 1024, fa) != NULL)
	{
		if(szBuf[0] != '#')
		{
			log_print(LOGN_CRI, "[ERROR] INVALID FORMAT CONGIG FILE[%s]. LINE=%d", FILE_PIRP_MAP, i + 1);
			continue;
		}
		i++;

		if(szBuf[1] == '#') continue;
		else if(szBuf[1] == 'E') break;
		else if(szBuf[1] == '@')
		{
			if( (dRet = sscanf(&szBuf[2], "%u %s %s", &pdsnID, strPIIP, strRPIP)) == 3);
			{
				printf("NO = %u RPIP = %s PIIP= %s \n", pdsnID, strPIIP, strRPIP);

				inet_aton(strPIIP, &inaddr);
				piIP = ntohl(inaddr.s_addr);
				inet_aton(strRPIP, &inaddr);
				rpIP = ntohl(inaddr.s_addr);

				dRet = dSetPDSNIFHash(pdsnID, piIP, rpIP);
				if(dRet < 0){
					log_print(LOGN_CRI,"dSetPDSNIFHash() Fail. RET[%d]", dRet);
				}
			}
		}
	}/* while */

	fclose(fa);

	return 1;
}

void invoke_del_call(void *p)
{
	S32				isFirstServ;
    RPPISESS_KEY    *pstRPPIKey;
	STIME			uiCallTime;
	MTIME			uiCallMTime;
	S64				llBeforeGapTime, llAfterGapTime;
	
	stHASHONODE     *pHASHONODE;
    LOG_RPPI        *pstRPPILog;

    HData_RPPI      *pstRPPIHash;
    HData_RPPI      stRPPIHash;
	
	pstRPPIHash = &stRPPIHash;

    pstRPPIKey = &(((RPPI_TIMER *)p)->RPPIKEY);
    uiCallTime = ((RPPI_TIMER *)p)->uiCallTime;
    uiCallMTime = ((RPPI_TIMER *)p)->uiCallMTime;

	log_print(LOGN_DEBUG, " INVOKE TIMER IMSI[%s] CallTime[%d.%d]", pstRPPIKey->szIMSI, uiCallTime, uiCallMTime);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) )
    {
        pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		if (pstRPPIHash->after.dOffset == 0)
		{
			STG_DeltaTIME64(uiCallTime, uiCallMTime, pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime, &llBeforeGapTime);
			if (llBeforeGapTime == 0)
			{
				pstRPPILog = (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);
				isFirstServ = pstRPPIHash->before.uiFirstServFlag;
				log_print(LOGN_DEBUG, "Before RPPI LOG OFFSET[%ld] TIMER[%lld]", pstRPPIHash->before.dOffset, pstRPPIHash->before.timerNID);
			}
			else
			{
				log_print(LOGN_CRI,"[%s][%s.%d] Before[%d.%d]TIMER TIME[%d.%d]", __FILE__, __FUNCTION__, __LINE__,
                        pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime,
                        uiCallTime, uiCallMTime);
                return;	
			}	
		}
		else
		{
			STG_DeltaTIME64(uiCallTime, uiCallMTime, pstRPPIHash->after.uiCallTime, pstRPPIHash->after.uiCallMTime, &llAfterGapTime);
        	STG_DeltaTIME64(uiCallTime, uiCallMTime, pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime, &llBeforeGapTime);

        	if (llAfterGapTime == 0)
        	{
				log_print(LOGN_DEBUG, "After RPPI LOG OFFSET[%ld] TIMER[%lld]", pstRPPIHash->after.dOffset, pstRPPIHash->after.timerNID);
				
				/* AFTER 삭제 할때 BEFORE도 같이 삭제 */
				/* AFTER 먼저 삭제 */
            	pstRPPILog = (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->after.dOffset);
				isFirstServ = pstRPPIHash->after.uiFirstServFlag;
				if (pstRPPILog->uiReleaseTime == 0) {
					if(!GET_ISRECALL(pstRPPILog->usCallType))
					{
//						pstRPPILog->usCallState = TIMEOUT_STATE;
						pstRPPILog->uiLastFailReason = A11_DEFECT + CALL_SETUP + ERR_CALL_TIMEOUT;	
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(GET_ISRECALL(pstRPPILog->usCallType), pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					}
					pstRPPILog->stopFlag = TIMER_STOP_CALL_NUM;
				}
				struct timeval  stNowTime;
				gettimeofday(&stNowTime, NULL);
				pstRPPILog->uiOpEndTime = stNowTime.tv_sec;
				pstRPPILog->uiOpEndMTime = stNowTime.tv_usec;

				dProcCallStop(pstRPPIHash, pstRPPILog, isFirstServ);
				dSendLogRPPI(pstRPPIHash, pstRPPILog);
            	
				pstRPPILog = (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);
				isFirstServ = pstRPPIHash->before.uiFirstServFlag;
				
				
        	}
			else if (llBeforeGapTime == 0 )
            {
                pstRPPILog = (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);
				isFirstServ = pstRPPIHash->before.uiFirstServFlag;
                log_print(LOGN_DEBUG, "Before RPPI LOG OFFSET[%ld] TIMER[%lld]",  pstRPPIHash->before.dOffset, pstRPPIHash->before.timerNID);
            }
        	else
        	{
                log_print(LOGN_CRI,"[%s][%s.%d] Before[%d.%d]After[%d.%d]TIMER TIME[%d.%d]", __FILE__, __FUNCTION__, __LINE__,
                        pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime,
                        pstRPPIHash->after.uiCallTime, pstRPPIHash->after.uiCallMTime,
                        uiCallTime, uiCallMTime);
                return;
        	}	
		}

		if (pstRPPILog->uiReleaseTime == 0) {
			if(!GET_ISRECALL(pstRPPILog->usCallType))
			{
//				pstRPPILog->usCallState = TIMEOUT_STATE;
				pstRPPILog->uiLastFailReason = A11_DEFECT + CALL_SETUP + ERR_CALL_TIMEOUT;
				pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(GET_ISRECALL(pstRPPILog->usCallType), pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
			}
			pstRPPILog->stopFlag = TIMER_STOP_CALL_NUM;
		}
		struct timeval  stNowTime;
    	gettimeofday(&stNowTime, NULL);
    	pstRPPILog->uiOpEndTime = stNowTime.tv_sec;
    	pstRPPILog->uiOpEndMTime = stNowTime.tv_usec;

		dProcCallStop(pstRPPIHash, pstRPPILog, isFirstServ);
		dSendLogRPPI(pstRPPIHash, pstRPPILog);
        
    }
	else
	{
		log_print(LOGN_CRI, "[%s][%s.%d] NOT FOUND HASH IMSI[%s] CreateTime[%d.%d]", __FILE__, __FUNCTION__, __LINE__,
                pstRPPIKey->szIMSI, uiCallTime, uiCallMTime); 
	}

}
