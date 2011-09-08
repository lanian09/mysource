/** A. FILE INCLUSION *********************************************************/
#include <signal.h>

// LIB
#include "typedef.h"
#include "commdef.h"
#include "loglib.h"
#include "ipclib.h"
#include "filelib.h"
#include "mems.h"
#include "memg.h"
#include "nifo.h"
#include "cifo.h"
#include "gifo.h"

#include "Analyze_Ext_Abs.h"

// PROJECT
#include "path.h"
#include "procid.h"
#include "sshmid.h"
#include "filter.h"
#include "common_stg.h"
#include "capdef.h"

#include "adns_init.h"
#include "adns_serv.h"

/** B. DEFINITION OF NEW CONSTANTS ********************************************/

/** C. DEFINITION OF NEW TYPES ************************************************/

/** D. DECLARATION OF VARIABLES ***********************************************/
extern stMEMSINFO		*pMEMSINFO;
extern stCIFO			*gpCIFO;
extern stHASHOINFO		*pDNSSESSHASH;
extern stTIMERNINFO	 *pTIMERNINFO;

extern int				gACALLCnt;
extern st_Flt_Info		*flt_info;

extern int				JiSTOPFlag;
extern int				FinishFlag;
extern int				guiTimerValue;

/** E.1 DEFINITION OF FUNCTIONS ***********************************************/

/** E.2 DEFINITION OF FUNCTIONS ***********************************************/

/*******************************************************************************
 INIT_IPCS 
*******************************************************************************/
int Init_DNSIPCS()
{
	if((pMEMSINFO = nifo_init_zone((U8*)"A_DNS", SEQ_PROC_A_DNS, FILE_NIFO_ZONE)) == NULL) {
		log_print(LOGN_CRI, LH"FAILED IN nifo_init NULL", LT);
		return -1;
	}

	if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
		log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
			LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
		return -2;
	}

	gACALLCnt = get_block_num(FILE_MC_INIT, "A_CALL");
	log_print(LOGN_INFO, "INIT., A_CALL ProcCount[%d]", gACALLCnt);


	pDNSSESSHASH = hasho_init(S_SSHM_DNS_SESS, DNS_SESS_KEY_SIZE, DNS_SESS_KEY_SIZE, DNS_SESS_SIZE, DNS_SESS_CNT, 0 );
	if( pDNSSESSHASH == NULL ) {
		log_print(LOGN_CRI, "[%s][%s.%d] hasho_init LPREA_CONF NULL", __FILE__, __FUNCTION__, __LINE__);
		return -3;
	}

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
		log_print(LOGN_CRI, "FAILED IN shm_init(FLT_INFO=%d)", S_SSHM_FLT_INFO);
		return -4;
	}

	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_DNS_TIMEOUT];

	log_print( LOGN_CRI, "TIMER:%u", guiTimerValue );

	if((pTIMERNINFO = timerN_init(DNS_SESS_CNT, sizeof(DNS_SESS_KEY))) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] timerN_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -5;
	}

	vDNSSESSTimerReConstruct( pDNSSESSHASH, pTIMERNINFO );	
	
	return 0;
}


/*******************************************************************************
 USERCONTROLLED SIGNAL 
*******************************************************************************/
void UserControlledSignal(int sign)
{
	JiSTOPFlag = 0;
	FinishFlag = sign;
}


/*******************************************************************************
 IGNORE SIGNAL 
*******************************************************************************/
void IgnoreSignal(int sign)
{
	if (sign != SIGALRM)
		log_print( LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
	signal(sign, IgnoreSignal);
}


/*******************************************************************************
 SETUP SIGNAL 
*******************************************************************************/
void SetUpSignal()
{
	JiSTOPFlag = 1;

	/* WANTED SIGNALS	*/
	signal(SIGTERM, UserControlledSignal);
	signal(SIGINT,	UserControlledSignal);
	signal(SIGQUIT, UserControlledSignal);

	/* UNWANTED SIGNALS */
	signal(SIGHUP,	IgnoreSignal);
	signal(SIGALRM, IgnoreSignal);
	signal(SIGPIPE, IgnoreSignal);
	signal(SIGPOLL, IgnoreSignal);
	signal(SIGPROF, IgnoreSignal);
	signal(SIGUSR1, IgnoreSignal);
	signal(SIGUSR2, IgnoreSignal);
	signal(SIGVTALRM, IgnoreSignal);
	signal(SIGCLD, SIG_IGN);
}


/*******************************************************************************
 dInitProc 
*******************************************************************************/
int dInitDNSProc()
{
	int		dRet;
	time_t	tTime;

	time(&tTime);

	SetUpSignal();

	dRet = Init_DNSIPCS();
	if(dRet < 0)
		return -1;

	log_print( LOGN_CRI, "[dInitProc] [PROCESS INIT SUCCESS]");
	return 0;
}


/*******************************************************************************
 FinishProgram 
*******************************************************************************/
void FinishProgram()
{
	log_print( LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", FinishFlag);
	exit(0);
}


/*******************************************************************************

*******************************************************************************/
void vDNSSESSTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int					 i;
	OFFSET					offset;
	stHASHONODE			 *p;
	stMEMGNODEHDR			*pMEMGNODEHDR;

	DNS_SESS_KEY			COMMON;
	DNS_SESS_KEY			*pKey;
	DNS_SESS				*pData;

	/* IM SESS */
	log_print(LOGN_INFO, "REBUILD DNSSESS TIMER hashcnt=%u", pHASH->uiHashSize);
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (DNS_SESS_KEY *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (DNS_SESS *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD DNSSESS TIMER SIP=%d.%d.%d.%d", HIPADDR(pKey->SIP));

				memcpy(&COMMON, pKey, DNS_SESS_KEY_SIZE ); 
				pData->timerNID = timerN_add(pTIMER, invoke_del_DNS, (U8*)&COMMON, DNS_SESS_KEY_SIZE, time(NULL) + guiTimerValue );
			}
			offset = p->offset_next;
		}
	}
}


/*******************************************************************************

*******************************************************************************/
void invoke_del_DNS( void *p)
{
	int						dRet;
	DNS_SESS_KEY			*pstCALLTimer;
	DNS_SESS				*pstDNSSESS;
	stHASHONODE			 *pHASHNODE;
	
	pstCALLTimer = (DNS_SESS_KEY *) p;
		
	log_print(LOGN_DEBUG, "@@@ TIMER TIMEOUT CIP[%d.%d.%d.%d]", HIPADDR(pstCALLTimer->SIP) );
		
	if((pHASHNODE = hasho_find(pDNSSESSHASH, (U8 *)pstCALLTimer)) != NULL) {
		pstDNSSESS = (DNS_SESS *)nifo_ptr(pDNSSESSHASH, pHASHNODE->offset_Data);
		log_print(LOGN_DEBUG, "@@@ INVOKE TIMEOUT CIP[%d.%d.%d.%d]	NTIME[%ld]", HIPADDR(pstCALLTimer->SIP) , time(NULL) );

		/* 정의 필요 */
		pstDNSSESS->ucErrorCode = 22;
			
		if( (dRet = dSend_DNSLog( pstCALLTimer, pstDNSSESS)) < 0 )
			log_print(LOGN_INFO, "[%s.%d] ERROR IN dSend_DNSLog dRet:%d", __FUNCTION__, __LINE__, dRet );

		hasho_del(pDNSSESSHASH, (U8 *)pstCALLTimer);
	} else {	
		log_print(LOGN_CRI, "INVOKE TIMEOUT BUT NODE NULL CIP[%d.%d.%d.%d]	NTIME[%ld]", HIPADDR(pstCALLTimer->SIP) , time(NULL) );
	}		
}
