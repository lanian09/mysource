/* File Include */
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
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
 
#include "utillib.h"
#include "rana_session.h"
#include "rana.h"


/* Definition of New Constants */
stHASHOINFO             *pHInfo_rad;
stTIMERNINFO 			*pTimer_rad;
time_t					gSessTimeout;
//_mem_check      		gSessShmem;
_mem_check      		*gpShmem;
extern MPTimer			*gpCurMPTimer;

/* MODIFY: by june, 2010-09-17
 * - HASH KEY º¯°æ: FramedIP --> Calling Station ID
 *                  Disconnect_Req ¸Þ½ÃÁö°¡ Ãß°¡µÇ¾úÀ¸³ª ÇØ´ç ¸Þ½ÃÁö¿¡´Â
 *                  FramedIP Field°¡ ¾ø¾î KEY º¯°æ.
 * - HASH BODYº¯°æ: SCE LOGON/OUT Operation¿¡ ÇÊ¿äÇÑ µ¥ÀÌÅ¸ ÇÊµå Ãß°¡.
 * - LINE         :  --> 
 */
int rad_session_timeout (rad_sess_key *pkey) 
{
	st_RADInfo			stRADInfo;
	rad_sess_body 		*pBody;
	SUBS_INFO 			si;
	int 				dRet = 0, tidx = -1; 

	pBody = find_rad_sess (pkey);
	if (pBody == NULL) {
		/* session is not found */
		dAppLog(LOG_CRI, "T:%d] %s", ERR_50009, pkey->szCSID);
		return -1;
	}

	sprintf (si.szMIN, "%s", pkey->szCSID);
	sprintf(si.szFramedIP, "%s", CVT_INT2STR_IP(pBody->uiFramedIP));
	sprintf(si.szDomain, "subscribers");
	si.type = IP_RANGE;
	si.sPkgNo = pBody->sPkgID;
	si.uiCBit = stRADInfo.uiCBIT = pBody->uiCBit;
	si.uiPBit = stRADInfo.uiPBIT = pBody->uiPBit;
	si.uiHBit = stRADInfo.uiHBIT = pBody->uiHBit;
	stRADInfo.uiFramedIP = pBody->uiFramedIP;
	sprintf (stRADInfo.szMIN, "%s", pkey->szCSID);

	tidx = pBody->dConnID;

	if (loc_sadb->loc_system_dup.myLocalDupStatus == SYS_STATE_STANDBY) {
		/* SYSTEM MODE:STANDBY */
		dAppLog(LOG_CRI, "T:%d] %s ",ERR_50012, pkey->szCSID);
		del_rad_sess (pkey, pBody); return -2;
	}
	else if (loc_sadb->loc_system_dup.myLocalDupStatus == SYS_STATE_FAULTED) {
		/* SYSTEM MODE:FAULTED */
		dAppLog(LOG_CRI, "T:%d] %s", ERR_50013, pkey->szCSID);
		del_rad_sess (pkey, pBody); return -3;
	}

	if (pBody->uiDoneLogOnF) {
		if( tidx < 0 || tidx >= MAX_RLEG_CNT )
		{
			/* RouteRLEG FAIL  */
			dAppLog(LOG_CRI, "T:%d] %s tidx=%d", ERR_50010, pkey->szCSID, tidx);
			Trace_LOGOUT (&stRADInfo, TRACE_TYPE_TIMEOUT_NOT_LOGOUT);
			del_rad_sess (pkey, pBody);
			return -4;
		}
		else
		{
			if((dRet = SendToRLEG(tidx, &si, DEF_LOG_OUT)) < 0 )
			{
				/* SendToRLEG FAIL  */
				dAppLog(LOG_CRI, "T:%d] %s tidx=%d(%d)", ERR_50011, pkey->szCSID, tidx, dRet);
				Trace_LOGOUT (&stRADInfo, TRACE_TYPE_TIMEOUT_NOT_LOGOUT);
				del_rad_sess (pkey, pBody);
				return -5;
			}
		}

		Trace_LOGOUT (&stRADInfo, TRACE_TYPE_TIMEOUT_LOGOUT);
		del_rad_sess (pkey, pBody);
	}
	else {
		/* not logon session */
		dAppLog(LOG_CRI, "T:%d] %s tidx=%d", ERR_50014, si.szMIN, tidx);
		Trace_LOGOUT (&stRADInfo, TRACE_TYPE_TIMEOUT_NOT_LOGOUT);
		del_rad_sess (pkey, pBody);
		return -6;
	}

	return 0;
}


void init_session (int shm_key)
{
	//gpShmem->rad_sess = 0;

    pHInfo_rad = hasho_init( shm_key/*SHM_RAD_SESS_KEY*/
							, sizeof(rad_sess_key)
							, sizeof(rad_sess_key)
							, sizeof(rad_sess_body)
							, MAX_RAD_HASHO_SIZE
							, NULL
							, 0);
    if (pHInfo_rad == NULL) {
        dAppLog (LOG_CRI, "FAIL] sid hasho_init ERR=%d(%s)", errno, strerror(errno));
        exit(1);
    }
	pTimer_rad = timerN_init(MAX_RAD_HASHO_SIZE, sizeof (rad_sess_key));
	if (pTimer_rad == NULL) {
        dAppLog (LOG_CRI, "FAIL] sid timerN_init ERROR[%d][%s", errno, strerror(errno));
		exit(1);
	}
	/* restart ÈÄ shm¿¡ ÀÖ´Â session ¿¡ ´ëÇá¿© timer Àç±¸¼º */
	rad_timer_reconstructor ();
}


rad_sess_body * get_rad_sess (rad_sess_key *pkey)
{
	rad_sess_body 		*pBody, Body;

	pBody = find_rad_sess (pkey);
	if (pBody == NULL) {
		pBody = add_rad_sess (pkey, &Body);
		if (pBody == NULL) {
        	dAppLog (LOG_CRI, "FAIL] add_rad_sess(%s) ERR=%d(%s)", pkey->szCSID, errno, strerror(errno));
			return NULL;
		}

		pBody->timerID = timerN_add( pTimer_rad
									, (void *)&rad_session_timeout
									, (U8 *)pkey
									, RAD_SESS_KEY_SIZE
									, time(0) + gpCurMPTimer->sess_timeout);
		if (pBody->timerID == 0) {
			hasho_del (pHInfo_rad, (U8 *)pkey);
        	dAppLog (LOG_CRI, "FAIL] timerN_add(%d) ERR=%d(%s)", pBody->timerID, errno, strerror(errno));
			return NULL;
		}
#ifdef PRT_SESS_CNT_LV2
		dAppLog(LOG_CRI, ">>> TIMER COUNT[ADD] : %u", pTimer_rad->uiNodeCnt);
#endif
	} else {
		del_rad_sess (pkey, pBody);
		pBody = get_rad_sess (pkey);
		return pBody;
	}

	return pBody;
}


/* 	INTERIM processing
	1. sess ¾øÀ¸¸é
	   - sess add, timer add, login
	2. sess ÀÖ°í imsi ÀÏÄ¡ÇÏ¸é,
	   - packageID compare
	3. sess ÀÖ°í imsi ÀÏÄ¡ÇÏ°í packageID ÀÏÄ¡ÇÏ¸éé,
	   - timer update
	4. sess ÀÖ°í imsi ÀÏÄ¡ÇÏ°í packageID ºÒÀÏÄ¡ÇÏ¸é?
	   - sess update(packageID), timer update, login
	3. sess ÀÖ°í imsi ºÒÀÏÄ¡ÇÏ¸é
	   - ÀÌÀü sess del, timer del
	   - ÇöÀç sess add, timer add, login
 */
/* get_rad_interim_sess
 * Descr  : ACC_INTERIM session processing
 *	  flag : decide whether session is non-existent, if session exist 1, else 0 
 * return : 
 *   -1 : session create fail
 *    0 : session new create
 *    1 : already session
 */

rad_sess_body *get_rad_interim_sess (rad_sess_key *pkey, int *flag)
{
	rad_sess_body 	Body, *pBody;

	pBody = find_rad_sess (pkey);
	if (pBody == NULL) {
		pBody = add_rad_sess (pkey, &Body);
		if (pBody == NULL){
        	dAppLog (LOG_CRI, "FAIL] add_rad_sess ERR[%d][%s", errno, strerror(errno));
			return NULL;
		}
		pBody->timerID = timerN_add( pTimer_rad
									, (void *)&rad_session_timeout
									, (U8 *)pkey
									, RAD_SESS_KEY_SIZE
									, time(0) + gpCurMPTimer->sess_timeout);
		if (pBody->timerID == 0) {
			hasho_del (pHInfo_rad, (U8 *)pkey);
        	dAppLog (LOG_CRI, "FAIL] timerN_add(%d) ERR[%d][%s", pBody->timerID, errno, strerror(errno));
			return NULL;
		}
		*flag = 0;
	}
	else {
		*flag = 1;
	}
	return pBody;
}


void del_rad_sess (rad_sess_key *pkey, rad_sess_body *pbody) 
{
	if (pbody->timerID != 0) {
		timerN_del (pTimer_rad, pbody->timerID);
		pbody->timerID = 0;
#ifdef PRT_SESS_CNT_LV2
		dAppLog(LOG_CRI, ">>> TIMER COUNT[DEL] : %u", pTimer_rad->uiNodeCnt);
#endif
	}
	hasho_del (pHInfo_rad, (U8 *)pkey);
	if (gpShmem->rad_sess > 0) {
		gpShmem->rad_sess--;
#ifdef PRT_SESS_CNT_LV2
		dAppLog(LOG_CRI, ">>> SESSION COUNT[DEL] : %u(%u)"
				, gpShmem->rad_sess
				, ((stMEMGINFO *)RAD_HASHO_PTR(pHInfo_rad, pHInfo_rad->offset_memginfo))->uiMemNodeAllocedCnt);
#endif
	}
}


int del_rad_sess_key (rad_sess_key *pkey) 
{
	rad_sess_body 		*pBody;

	pBody = find_rad_sess (pkey);
	if (pBody != NULL) { 
		del_rad_sess (pkey, pBody);
		return 1;
	}
	return 0;
}


rad_sess_body *add_rad_sess (rad_sess_key *pkey, rad_sess_body *pbody)
{
	stHASHONODE                     *pstHashNode;
	rad_sess_body 					*pstData;

	pstHashNode = hasho_add (pHInfo_rad, (U8 *)pkey, (U8 *)pbody);
	if (pstHashNode == NULL) {
		return NULL;
	}

	pstData = (rad_sess_body *)RAD_HASHO_PTR (pHInfo_rad, pstHashNode->offset_Data);

	gpShmem->rad_sess++;
#ifdef PRT_SESS_CNT_LV2
	//((stMEMGINFO *)HASHO_PTR(pCALLHASH, pCALLHASH->offset_memginfo))->uiMemNodeAllocedCnt

	dAppLog(LOG_CRI, ">>> SESSION COUNT[ADD] : %u(%u)"
			, gpShmem->rad_sess
			, ((stMEMGINFO *)RAD_HASHO_PTR(pHInfo_rad, pHInfo_rad->offset_memginfo))->uiMemNodeAllocedCnt);
#endif
	return pstData;
}


rad_sess_body *find_rad_sess (rad_sess_key *pkey)
{
	stHASHONODE             *pstHashNode;
	rad_sess_body			*pBody;

	pstHashNode = hasho_find (pHInfo_rad, (U8 *)pkey);
	if (pstHashNode == NULL) {
		return NULL;
	}

	pBody = (rad_sess_body *)RAD_HASHO_PTR (pHInfo_rad, pstHashNode->offset_Data);
	
	return pBody;
}

void rad_timer_reconstructor (void)
{
	int                 i;
	long long 			timer_node_cnt=0;
	OFFSET              offset;
	stHASHONODE         *p;
	stMEMGNODEHDR       *pMEMGNODEHDR;

	rad_sess_key        *pstKey;
	rad_sess_body       *pstData;

	for(i = 0; i < pHInfo_rad->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHInfo_rad, pHInfo_rad->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHInfo_rad, offset);

			pstKey  = (rad_sess_key *) HASHO_PTR (pHInfo_rad, p->offset_Key);
			pstData = (rad_sess_body*) HASHO_PTR (pHInfo_rad, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				dAppLog (LOG_DEBUG, "TM REBUILD] %s %s TID: %lld"
						, pstKey->szCSID, CVT_INT2STR_IP(pstData->uiFramedIP), pstData->timerID);
				pstData->timerID = timerN_add( pTimer_rad
											 , (void *)&rad_session_timeout
											 , (U8 *)pstKey
											 , RAD_SESS_KEY_SIZE
											 , time(0) + gpCurMPTimer->sess_timeout);
				timer_node_cnt++;
			}
			offset = p->offset_next;
		} /* while end */
	}
#ifdef PRT_SESS_CNT_LV2
	dAppLog(LOG_CRI, "TM REBUILD] SESSION TIMER COUNT : %lld", timer_node_cnt);
#endif
}

