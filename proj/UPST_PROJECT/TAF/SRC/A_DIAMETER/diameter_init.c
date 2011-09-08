/**<  
 $Id: diameter_init.c,v 1.3 2011/09/05 01:35:32 uamyd Exp $
 $Author: uamyd $
 $Revision: 1.3 $
 $Date: 2011/09/05 01:35:32 $
 **/

/**
 * Include headers
 */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>

// DQMS headers
#include "procid.h"
#include "sshmid.h"
#include "commdef.h"
#include "common_stg.h"

// LIB headers
#include "typedef.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"
#include "ipclib.h"
#include "filelib.h"

// OAM
#include "path.h"

// TAF headers
#include "memg.h"
#include "filter.h"

// .
#include "diameter_comm.h"
#include "diameter_init.h"

/**
 * Declare variables
 */
extern stCIFO			*gpCIFO;
extern st_TraceList		*pstTRACE;
extern st_Flt_Info  	*flt_info;

extern S32		gACALLCnt;

extern S32		giFinishSignal;
extern S32		giStopFlag;

extern S32 		gTIMER_TRANS;

/**
 *	Declare extern func.
 */
extern int cb_timeout_transaction(HKey_Trans *pstTransKey);

/**
 *	Implement func.
 */
int Init_TraceShm( )
{       
	int     dRet;
	if( (dRet = shm_init(S_SSHM_TRACE_INFO, st_TraceList_SIZE, (void**)&pstTRACE)) < 0){
		log_print(LOGN_CRI, "FAILED IN shm_init(TRACE LIST=%d)", S_SSHM_TRACE_INFO);
		return -1;
	}
	return 0;
}


S32	dInitDIAMETERProc(stMEMSINFO **pstMEMS, stHASHOINFO **pstHASHO, stTIMERNINFO **pstTIMER)
{
	int 	dRet;

	SetUpSignal();

	if((*pstMEMS = nifo_init_zone((U8*)"A_DIAMETER", SEQ_PROC_A_DIAMETER, FILE_NIFO_ZONE)) == NULL) {
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

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN shm_init(FLT_INFO)", LT);
		return -3;
	}


	/**<  INIT HASHO  **/
	if((*pstHASHO = hasho_init(S_SSHM_DIAMETER, DEF_HKEY_TRANS_SIZE, DEF_HKEY_TRANS_SIZE, DEF_HDATA_TRANS_SIZE, DIAMETER_TRANS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -4;
	}

	/**<  INIT TIMER **/
	if((*pstTIMER = timerN_init(DIAMETER_TRANS_CNT, DEF_TDATA_TRANS_TIMER_SIZE)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] timerN_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -5;
	}

	vDIATRANSTimerReConstruct(*pstHASHO, *pstTIMER);

	/* TRACE INFO */
	if( (dRet = Init_TraceShm()) < 0) {
		log_print( LOGN_CRI, "[%s.%d] ERROR IN Init_TraceShm dRet:%d", __FUNCTION__, __LINE__, dRet );
		return -6;
	}

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			tcp_init.c tcp_main.c tcp_api.h
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
 *  UserControlledSignal Function
 *
 *	@param	isign	:	signal
 *
 *  @return			void
 *  @see			tcp_init.c tcp_main.c tcp_api.h
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
 *  FinishProgram Function
 *
 *  @return			void
 *  @see			tcp_init.c tcp_main.c
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
 *  IgnoreSignal Function
 *
 *	@param	isign	:	signal
 *
 *  @return			void
 *  @see			tcp_init.c tcp_main.c tcp_api.h
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


void vDIATRANSTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int                 i;
	OFFSET              offset;
	stHASHONODE         *p;
	stMEMGNODEHDR       *pMEMGNODEHDR;

	TData_Trans			COMMON;
	HKey_Trans			*pKey;
	HData_Trans			*pData;


	gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_DIA_TIMEOUT];

	/* DIAMETER TRANS */
	log_print(LOGN_INFO, "REBUILD DIAMETER TRANS TIMER hashcnt=%u", pHASH->uiHashSize);
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (HKey_Trans *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (HData_Trans *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD DIAMETER TRANS TIMER SPORT=%d SYSID=%u HHID=%u",
						pKey->usSrcPort, pKey->uiSystemId, pKey->uiHHID);

				memcpy(&COMMON.key_trans, pKey, DEF_HKEY_TRANS_SIZE);
				pData->ulTimerNID = 
					timerN_add(pTIMER, (void *)&cb_timeout_transaction, (U8*)&COMMON, DEF_TDATA_TRANS_TIMER_SIZE, time(NULL) + gTIMER_TRANS);
			}
			offset = p->offset_next;
		}
	}
}



/**<  
  $Log: diameter_init.c,v $
  Revision 1.3  2011/09/05 01:35:32  uamyd
  modified to runnable source

  Revision 1.2  2011/09/04 12:16:49  hhbaek
  *** empty log message ***

  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
  NEW OAM SYSTEM

  Revision 1.3  2011/08/17 12:54:29  hhbaek
  A_DIAMETER

  Revision 1.2  2011/08/09 08:17:40  uamyd
  add blocks

  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
  init DQMS2

  Revision 1.12  2011/05/09 13:32:34  dark264sh
  A_DIAMETER: A_CALL multi 처리

  Revision 1.11  2011/01/11 04:09:06  uamyd
  modified

  Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
  DQMS With TOTMON, 2nd-import

  Revision 1.10  2009/10/08 07:18:14  pkg
  A_DIAMETER hasho local => shm, cont 20000 => 20011

  Revision 1.9  2009/08/12 12:34:56  dqms
  DIAMETER 디버그

  Revision 1.8  2009/08/04 12:08:17  dqms
  TIMER를 공유메모리로 변경

  Revision 1.7  2009/07/15 15:48:43  dqms
  ADD vDIATRANSTimerReConstruct()

  Revision 1.6  2009/07/08 08:35:14  dqms
  ADD TRACE INFO

  Revision 1.5  2009/06/29 13:19:38  dark264sh
  *** empty log message ***

  Revision 1.4  2009/06/04 12:03:12  jsyoon
  *** empty log message ***

  Revision 1.3  2009/06/02 17:35:55  jsyoon
  *** empty log message ***

  Revision 1.2  2007/05/23 05:26:55  dark264sh
  modify S_MSGQ_CILOG => S_MSGQ_CI_LOG

  Revision 1.1  2007/05/09 08:17:47  jsyoon
  ADD ADD A_DIAMETER

 **/

