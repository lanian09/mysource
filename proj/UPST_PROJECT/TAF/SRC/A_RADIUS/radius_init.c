/**<	
 $Id: radius_init.c,v 1.3 2011/09/05 09:01:46 dhkim Exp $
 $Author: dhkim $
 $Revision: 1.3 $
 $Date: 2011/09/05 09:01:46 $
 **/



/**
 * Include headers
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

// LIB headers
#include "typedef.h"
#include "loglib.h"
#include "ipclib.h"
#include "filelib.h"
#include "mems.h"
#include "memg.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

#include "Analyze_Ext_Abs.h"

// PROJECT
#include "path.h"
#include "procid.h"
#include "sshmid.h"
#include "capdef.h"
#include "commdef.h"
#include "filter.h"
#include "common_stg.h"

// .
#include "radius_init.h"
#include "radius_func.h"

/**
 * Declare variables
 */
extern stCIFO			*gpCIFO;
extern st_TraceList	 	*pstTRACE;
extern S32				gACALLCnt;
extern S32				giFinishSignal;
extern S32				giStopFlag;
extern S32				gTIMER_TRANS;
extern st_Flt_Info		*flt_info;
extern st_ippool		stIPPool;

S32	dInitRADIUSProc(stMEMSINFO **pstMEMSINFO, stHASHOINFO **pstHASHOINFO, stTIMERNINFO **pstTIMERNINFO)
{
	int 	dRet;

	SetUpSignal();

	if((*pstMEMSINFO = nifo_init_zone((U8*)"A_RADIUS", SEQ_PROC_A_RADIUS, FILE_NIFO_ZONE)) == NULL) {
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

	/**<	INIT HASHO	**/
	if((*pstHASHOINFO = hasho_init(S_SSHM_RADIUS, DEF_HKEY_TRANS_SIZE, DEF_HKEY_TRANS_SIZE, DEF_HDATA_TRANS_SIZE, RADIUS_TRANS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -4;
	}

	/**<	INIT TIMER **/
	if((*pstTIMERNINFO = timerN_init(RADIUS_TRANS_CNT, DEF_TDATA_TRANS_TIMER_SIZE)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] timerN_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -5;
	}
	
	vRADIUSTimerReConstruct(*pstHASHOINFO, *pstTIMERNINFO);

	/* TRACE INFO */
	if( shm_init(S_SSHM_TRACE_INFO, st_TraceList_SIZE, (void**)&pstTRACE) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN shm_init(TRACE_INFO)", LT);
		return -6;
	}

	/* Read IP Pool for RADIUS Signal */
	dRet = dReadIPPool();
	if(dRet < 0)
	{
		log_print(LOGN_CRI, "[%s.%d] ERROR dReadIPPool dRet:%d",	__FUNCTION__, __LINE__, dRet);
		return -7;
	}

	return 0;
}

/** 
 * A_RADIUS분석 대역에 대한 IPPOOL을 설정파일로 부터 읽어와서 세팅한다.
 * 
 * 
 * @return Success = 0, Fail = -1
 */
int dReadIPPool(void)
{
	FILE *fp;
	char szBuffer[128];
	char szIP[32], szNetmask[32];
	int	dCount = 0;
	struct in_addr	stAddr;

	if( (fp = fopen(FILE_IPPOOLINFO, "r")) == NULL ) {
		log_print( LOGN_WARN, "[%s.%d] FILE OPEN FAIL[%s]", __FUNCTION__, __LINE__, strerror( errno ) );
		return -1;
	}

	while ( fgets( szBuffer, 1024, fp )!=NULL )
	{
		if( szBuffer[0] == '#' && szBuffer[1] == 'E' )
			break;

		if ( szBuffer[0] == '#' && szBuffer[1]=='@' ) {
			sscanf( &szBuffer[2], "%s %s", szIP, szNetmask);

#ifdef _IPPOOL_TYPE_
			if(!strcmp(szIP, "Type"))
			{
				stIPPool.dFlag = atoi(szNetmask);
				log_print( LOGN_CRI, "IPPOOL %s %s", szIP, szNetmask);
				continue;
			}
#endif
			log_print( LOGN_CRI, "IPPOOL ADD IP %s %s", szIP, szNetmask);
			inet_pton(AF_INET, szIP, &stAddr);
			stIPPool.stIPPool[dCount].uiFIP 	= ntohl(stAddr.s_addr);
			stIPPool.stIPPool[dCount].usNetMask = atoi(szNetmask);

			log_print(LOGN_CRI, "IPPOOL ADD IP:%d.%d.%d.%d NET:%u LIST COUNT:%d",
					HIPADDR(stIPPool.stIPPool[dCount].uiFIP), stIPPool.stIPPool[dCount].usNetMask, dCount);
			
			dCount++;
			stIPPool.uiCount = dCount;
		}
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

void vRADIUSTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int				 i;
	OFFSET				offset;
	stHASHONODE		 *p;
	stMEMGNODEHDR		*pMEMGNODEHDR;

	HKey_Trans			*pKey;
	HData_Trans			*pData;


	gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_RAD_TIMEOUT];

	/* IPFRAG */
	log_print(LOGN_INFO, "REBUILD RADIUS TIMER hashcnt=%u", pHASH->uiHashSize);
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
				log_print(LOGN_INFO, "REBUILD RADIUS TIMER SIP=%d.%d.%d.%d DIP=%d.%d.%d.%d ID=%u",
						HIPADDR(pKey->uiSrcIP), HIPADDR(pKey->uiDestIP), pKey->ucID);

				pData->ulTimerNID = timerN_add(pTIMER, (void *)&cb_timeout_transaction, (U8*)pKey, DEF_TDATA_TRANS_TIMER_SIZE, time(NULL) + gTIMER_TRANS);
			}
			offset = p->offset_next;
		}
	}
}


/**<	
	$Log: radius_init.c,v $
	Revision 1.3  2011/09/05 09:01:46  dhkim
	*** empty log message ***
	
	Revision 1.2	2011/09/05 01:35:33	uamyd
	modified to runnable source

	Revision 1.1.1.1	2011/08/29 05:56:42	dcham
	NEW OAM SYSTEM

	Revision 1.3	2011/08/17 13:08:45	hhbaek
	A_RADIUS

	Revision 1.2	2011/08/09 08:17:41	uamyd
	add blocks

	Revision 1.1.1.1	2011/08/05 00:27:17	uamyd
	init DQMS2

	Revision 1.16	2011/07/21 04:06:47	night1700
	*** empty log message ***

	Revision 1.15	2011/07/19 09:12:46	night1700
	*** empty log message ***

	Revision 1.14	2011/07/19 09:11:00	night1700
	*** empty log message ***

	Revision 1.13	2011/07/19 08:51:13	night1700
	*** empty log message ***

	Revision 1.12	2011/07/14 14:13:55	night1700
	*** empty log message ***

	Revision 1.11	2011/07/12 10:15:02	night1700
	*** empty log message ***

	Revision 1.10	2011/07/12 07:24:17	jsyoon
	*** empty log message ***

	Revision 1.9	2011/05/09 13:18:34	dark264sh
	A_RADIUS: A_CALL multi 처리

	Revision 1.8	2011/01/11 04:09:09	uamyd
	modified

	Revision 1.1.1.1	2010/08/23 01:13:00	uamyd
	DQMS With TOTMON, 2nd-import

	Revision 1.7	2009/10/08 07:23:20	pkg
	A_RADIUS hasho local => shm, RADIUS_TRANS_CNT 20000 => 20011

	Revision 1.6	2009/08/12 12:34:56	dqms
	DIAMETER 디버그

	Revision 1.5	2009/08/04 12:08:17	dqms
	TIMER를 공유메모리로 변경

	Revision 1.4	2009/07/15 16:58:28	dqms
	ADD vRADIUSTimerReConstruct()

	Revision 1.3	2009/07/08 08:34:35	dqms
	ADD TRACE INFO

	Revision 1.2	2009/06/29 13:24:13	dark264sh
	*** empty log message ***

	Revision 1.1	2009/06/03 18:48:43	jsyoon
	*** empty log message ***

	Revision 1.3	2009/06/02 17:35:55	jsyoon
	*** empty log message ***

	Revision 1.2	2007/05/23 05:26:55	dark264sh
	modify S_MSGQ_CILOG => S_MSGQ_CI_LOG

	Revision 1.1	2007/05/09 08:17:47	jsyoon
	ADD ADD A_DIAMETER

 **/

