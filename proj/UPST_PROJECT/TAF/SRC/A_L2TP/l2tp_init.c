/**<	
 $Id: l2tp_init.c,v 1.3 2011/09/05 07:33:22 dhkim Exp $
 $Author: dhkim $
 $Revision: 1.3 $
 $Date: 2011/09/05 07:33:22 $
 **/

#include <signal.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "ipclib.h"
#include "mems.h"
#include "memg.h"
#include "nifo.h"
#include "cifo.h"
#include "gifo.h"
#include "timerN.h"

// PROJECT
#include "path.h"
#include "procid.h"
#include "sshmid.h"
#include "filter.h"
#include "common_stg.h"


#include "l2tp_init.h"
#include "l2tp_func.h"

/**
 * Declare variables
 */
extern stCIFO			*gpCIFO;
extern st_TraceList		*pstTRACE;

extern S32				giFinishSignal;
extern S32				giStopFlag;

extern S32				gTIMER_TRANS;

extern st_Flt_Info		*flt_info;

extern stTIMERNINFO		*pstTIMERNINFO;
extern stTIMERNINFO		*pstCTIMERNINFO;
extern stTIMERNINFO		*pstTTIMERNINFO;		/* Tunnel ID Timer */
extern stTIMERNINFO		*pstSTIMERNINFO;		/* Session ID Timer */

extern stHASHOINFO		*pstHASHOINFO;
extern stHASHOINFO		*pstCHASHOINFO;
extern stHASHOINFO		*pstTHASHOINFO;
extern stHASHOINFO		*pstSHASHOINFO;

int Init_TraceShm( )
{		
	if( shm_init(S_SSHM_TRACE_INFO, st_TraceList_SIZE, (void**)&pstTRACE) < 0 ){
		log_print(LOGN_CRI, "FAILED IN shm_init(TRACE_INFO=%d)", S_SSHM_TRACE_INFO);
		return -1;
	}

	return 0;
}


S32	dInitL2TPProc(stMEMSINFO **pstMEMSINFO)
{
	int 	dRet;


	SetUpSignal();

	if((*pstMEMSINFO = nifo_init_zone((U8*)"A_L2TP", SEQ_PROC_A_L2TP, FILE_NIFO_ZONE)) == NULL ){
		log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone NULL", LT);
		return -1;
	}

	if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
		log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
			LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
		return -2;
	}

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
		log_print(LOGN_CRI, "FAILED IN shm_init(FLT_INFO=%d)", S_SSHM_FLT_INFO);
		return -3;
	}

	/* CONTROL HASH */
	if((pstCHASHOINFO = hasho_init(S_SSHM_CONTROL, DEF_CONTROL_KEY_SIZE, DEF_CONTROL_KEY_SIZE, DEF_CONTROL_DATA_SIZE, L2TP_CONTROL_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init(CONTROL) NULL", LT);
		return -4;
	}
	
	/* CALL SESS HASH */
	if((pstHASHOINFO = hasho_init(S_SSHM_A_L2TP, DEF_CALLSESS_KEY_SIZE, DEF_CALLSESS_KEY_SIZE, DEF_CALLSESS_DATA_SIZE, L2TP_CALLSESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init(A_L2TP) NULL", LT);
		return -5;
	}
	
	/* TUNNEL ID HASH */
	if((pstTHASHOINFO = hasho_init(S_SSHM_TUNNELID, DEF_TUNNELID_KEY_SIZE, DEF_TUNNELID_KEY_SIZE, DEF_TUNNELID_DATA_SIZE, L2TP_TRANS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init(TUNNELID) NULL", LT);
		return -6;
	}
	
	/* SESSION ID HASH */
	if((pstSHASHOINFO = hasho_init(S_SSHM_SESSION, DEF_SESSID_KEY_SIZE, DEF_SESSID_KEY_SIZE, DEF_SESSID_DATA_SIZE, L2TP_TRANS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init(SESSION) NULL", LT);
		return -7;
	}

	/*	INIT CONTROL TIMER */
	if((pstCTIMERNINFO = timerN_init(L2TP_TRANS_CNT, DEF_TDATA_CONTROL_TIMER_SIZE)) == NULL) {
		log_print(LOGN_CRI, LH"timerN_init(pstCTIMERNINFO) NULL", LT);
		return -8;
	}
	
	/*	INIT CALLSESS TIMER */
	if((pstTIMERNINFO = timerN_init(L2TP_TRANS_CNT, DEF_TDATA_CALLSESS_TIMER_SIZE)) == NULL) {
		log_print(LOGN_CRI, LH"timerN_init(pstTIMERNINFO) NULL", LT);
		return -9;
	}

	/*	INIT TUNNELID TIMER */
	if((pstTTIMERNINFO = timerN_init(L2TP_TRANS_CNT, DEF_TDATA_TUNNELID_TIMER_SIZE)) == NULL) {
		log_print(LOGN_CRI, LH"timerN_init(pstTTIMERNINFO) NULL", LT);
		return -10;
	}
	
	/*	INIT SESSID TIMER */
	if((pstSTIMERNINFO = timerN_init(L2TP_TRANS_CNT, DEF_TDATA_SESSID_TIMER_SIZE)) == NULL) {
		log_print(LOGN_CRI, LH"timerN_init(pstSTIMERNINFO) NULL", LT);
		return -11;
	}
	
	vTunnelIDTimerReConstruct(pstTHASHOINFO, pstTTIMERNINFO);
	vSessionIDTimerReConstruct(pstSHASHOINFO, pstSTIMERNINFO);
	vControlTimerReConstruct(pstCHASHOINFO, pstCTIMERNINFO);
	vCallsessTimerReConstruct(pstHASHOINFO, pstTIMERNINFO);

	/* TRACE INFO */
	if( (dRet = Init_TraceShm()) < 0) {
		log_print( LOGN_CRI, "[%s.%d] ERROR IN Init_TraceShm dRet:%d", __FUNCTION__, __LINE__, dRet );
		return -12;
	}


	return 0;
}

/** SetUpSignal function.
 *
 *	SetUpSignal Function
 *
 *	@return			void
 *	@see			tcp_init.c tcp_main.c tcp_api.h
 *
 **/
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

/** UserControlledSignal function.
 *
 *	UserControlledSignal Function
 *
 *	@param	isign	:	signal
 *
 *	@return			void
 *	@see			tcp_init.c tcp_main.c tcp_api.h
 *
 **/
void UserControlledSignal(S32 isign)
{
	giStopFlag = 0;
	giFinishSignal = isign;
	log_print(LOGN_CRI, "User Controlled Signal Req = %d", isign);
}

/** FinishProgram function.
 *
 *	FinishProgram Function
 *
 *	@return			void
 *	@see			tcp_init.c tcp_main.c
 *
 **/
void FinishProgram(void)
{
#ifdef MEM_TEST
	log_print(LOGN_CRI, "CREATE CNT[%llu] DELETE CNT[%llu]", nifo_create, nifo_del);
#endif
	log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", giFinishSignal);
	exit(0);
}

/** IgnoreSignal function.
 *
 *	IgnoreSignal Function
 *
 *	@param	isign	:	signal
 *
 *	@return			void
 *	@see			tcp_init.c tcp_main.c tcp_api.h
 *
 **/
void IgnoreSignal(S32 isign)
{
	if (isign != SIGALRM)
	{
		log_print(LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", isign);
	}

	signal(isign, IgnoreSignal);
}


void vTunnelIDTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int				 i;
	OFFSET				offset;
	stHASHONODE		 *p;
	stMEMGNODEHDR		*pMEMGNODEHDR;

	st_TUNNELID_KEY		*pKey;
	st_TUNNELID_DATA	*pData;

#ifdef SIM 
	gTIMER_TRANS = 20;
#else				 
	gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
#endif 

	log_print(LOGN_INFO, "REBUILD TUNNELID TIMER hashcnt=%u", pHASH->uiHashSize);
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (st_TUNNELID_KEY *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (st_TUNNELID_DATA *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if( (pData->usLastMessageType < L2TP_MSG_SCCCN) && (MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD TUNNELID TIMER SIP=%d.%d.%d.%d DIP=%d.%d.%d.%d SRCPORT=%d DESTPORT=%d TID=%d",
						HIPADDR(pKey->uiSrcIP), HIPADDR(pKey->uiDestIP), pKey->usSrcPort, pKey->usDestPort, pKey->usTunnelID);

				pData->ulTimerNID = timerN_add(pTIMER, (void *)&cb_timeout_tunnelid, (U8*)pKey, DEF_TDATA_TUNNELID_TIMER_SIZE, time(NULL) + gTIMER_TRANS);
			}
			offset = p->offset_next;
		}
	}
}

void vSessionIDTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int				 i;
	OFFSET				offset;
	stHASHONODE		 *p;
	stMEMGNODEHDR		*pMEMGNODEHDR;

	st_SESSID_KEY		*pKey;
	st_SESSID_DATA		*pData;

#ifdef SIM 
	gTIMER_TRANS = 20;
#else				 
	gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
#endif 

	log_print(LOGN_INFO, "REBUILD SESSID TIMER hashcnt=%u", pHASH->uiHashSize);
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (st_SESSID_KEY *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (st_SESSID_DATA *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if( (pData->usLastMessageType < L2TP_MSG_ICCN) && (MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD SESSID TIMER SIP=%d.%d.%d.%d DIP=%d.%d.%d.%d SRCPORT=%d DESTPORT=%d SID=%d",
						HIPADDR(pKey->uiSrcIP), HIPADDR(pKey->uiDestIP), pKey->usSrcPort, pKey->usDestPort, pKey->usSessionID);

				pData->ulTimerNID = timerN_add(pTIMER, (void *)&cb_timeout_Sessid, (U8*)pKey, DEF_TDATA_SESSID_TIMER_SIZE, time(NULL) + gTIMER_TRANS);
			}
			offset = p->offset_next;
		}
	}
}

void vControlTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int				 i;
	OFFSET				offset;
	stHASHONODE		 *p;
	stMEMGNODEHDR		*pMEMGNODEHDR;

	st_CONTROL_KEY		*pKey;
	st_CONTROL_DATA		*pData;

#ifdef SIM 
	gTIMER_TRANS = 20;
#else				 
	gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
#endif 

	log_print(LOGN_INFO, "REBUILD CONTROL SESSION TIMER hashcnt=%u", pHASH->uiHashSize);
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (st_CONTROL_KEY *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (st_CONTROL_DATA *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD CONTROL SESSION TIMER SIP=%d.%d.%d.%d DIP=%d.%d.%d.%d REQTID=%d RESTID=%d",
						HIPADDR(pKey->uiSrcIP), HIPADDR(pKey->uiDestIP), pKey->usReqTunnelID, pKey->usResTunnelID);

				pData->ulTimerNID = timerN_add(pTIMER, (void *)&cb_timeout_Control, (U8*)pKey, DEF_TDATA_CONTROL_TIMER_SIZE, time(NULL) + gTIMER_TRANS);
			}
			offset = p->offset_next;
		}
	}
}

void vCallsessTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int				 i;
	OFFSET				offset;
	stHASHONODE		 *p;
	stMEMGNODEHDR		*pMEMGNODEHDR;

	st_CALLSESS_KEY		*pKey;
	st_CALLSESS_DATA	*pData;

#ifdef SIM 
	gTIMER_TRANS = 20;
#else				 
	gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT]; 
#endif

	log_print(LOGN_INFO, "REBUILD CALL SESSION TIMER hashcnt=%u", pHASH->uiHashSize);
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (st_CALLSESS_KEY *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (st_CALLSESS_DATA *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD CALL SESSION TIMER IMSI=%s SIP=%d.%d.%d.%d DIP=%d.%d.%d.%d REQSID=%d RESSID=%d",
						pKey->szIMSI, HIPADDR(pKey->uiSrcIP), HIPADDR(pKey->uiDestIP), pKey->usReqSessID, pKey->usResSessID);

				pData->ulTimerNID = timerN_add(pTIMER, (void *)&cb_timeout_CallSess, (U8*)pKey, DEF_TDATA_CALLSESS_TIMER_SIZE, time(NULL) + gTIMER_TRANS);
			}
			offset = p->offset_next;
		}
	}
}



/**<	
	$Log: l2tp_init.c,v $
	Revision 1.3  2011/09/05 07:33:22  dhkim
	*** empty log message ***
	
	Revision 1.2	2011/09/05 01:35:32	uamyd
	modified to runnable source

	Revision 1.1.1.1	2011/08/29 05:56:42	dcham
	NEW OAM SYSTEM

	Revision 1.3	2011/08/17 13:04:54	hhbaek
	A_L2TP

	Revision 1.2	2011/08/11 04:38:30	uamyd
	modified and block added

	Revision 1.1.1.1	2011/08/05 00:27:18	uamyd
	init DQMS2

	Revision 1.8	2011/05/13 11:52:12	jsyoon
	*** empty log message ***

	Revision 1.7	2011/05/06 05:59:36	jsyoon
	*** empty log message ***

	Revision 1.6	2011/01/11 04:09:08	uamyd
	modified

	Revision 1.1.1.1	2010/08/23 01:12:59	uamyd
	DQMS With TOTMON, 2nd-import

	Revision 1.5	2010/03/18 12:57:39	jsyoon
	TIMER 재구성 함수 추가

	Revision 1.4	2010/03/18 12:34:12	jsyoon
	*** empty log message ***

	Revision 1.3	2010/03/12 13:54:37	jsyoon
	SID 타이머 추가

	Revision 1.2	2010/03/05 07:29:06	jsyoon
	ZLB Ack 패킷 처리

	Revision 1.1	2010/03/03 07:33:42	jsyoon
	A_L2TP PROCESS 추가

 **/

