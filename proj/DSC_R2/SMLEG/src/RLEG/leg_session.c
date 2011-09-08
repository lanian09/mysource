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
#include "leg_session.h"
#include "leg.h"


/* Definition of New Constants */
stHASHOINFO             *pHInfo_rad;
stTIMERNINFO 			*pTimer_rad;
time_t					gSessTimeout;
_mem_check      		*gpShmem=NULL;

/* MODIFY: by june, 2010-09-17
 * - HASH KEY ����: FramedIP --> Calling Station ID
 *                  Disconnect_Req �޽����� �߰��Ǿ����� �ش� �޽�������
 *                  FramedIP Field�� ���� KEY ����.
 * - HASH BODY����: SCE LOGON/OUT Operation�� �ʿ��� ����Ÿ �ʵ� �߰�.
 * - LINE         :  --> 
 */
int rad_session_timeout (rad_sess_key *pkey) 
{
	st_ACCInfo			AccInfo;
	rad_sess_body 		*pBody;
	SUBS_INFO 			si;
	int 				i, rtn=0, lflag=0;

	pBody = find_rad_sess (pkey);
	if (pBody == NULL) {
		dAppLog(LOG_CRI, "SESS_TO][IMSI:%s][session is not found"
				, pkey->szCSID);
		return -1;
	}

	sprintf (si.szMIN, "%s", pkey->szCSID);
	sprintf(si.szFramedIP, "%s", CVT_INT2STR_IP(pBody->uiFramedIP));
	sprintf(si.szDomain, "subscribers");
	si.type = IP_RANGE;
	si.sPkgNo = pBody->sPkgID;
	si.uiCBit = AccInfo.uiCBIT = pBody->uiCBit;
	si.uiPBit = AccInfo.uiPBIT = pBody->uiPBit;
	si.uiHBit = AccInfo.uiHBIT = pBody->uiHBit;
	AccInfo.uiFramedIP = pBody->uiFramedIP;
	sprintf (AccInfo.szMIN, "%s", pkey->szCSID);

	if (loc_sadb->loc_system_dup.myLocalDupStatus == SYS_STATE_STANDBY) {
		dAppLog(LOG_CRI, "SESS_TO][SYSTEM MODE:STANDBY");
		del_rad_sess (pkey, pBody); return 0;
	}
	else if (loc_sadb->loc_system_dup.myLocalDupStatus == SYS_STATE_FAULTED) {
		dAppLog(LOG_CRI, "SESS_TO][SYSTEM MODE:FAULTED");
		del_rad_sess (pkey, pBody); return 0;
	}

	for(i=0; i < MAX_SM_CONN_COUNT; i++) {
		if ((rtn = checkConnectSCE (i)) == 0) {
			if (pBody->uiDoneLogOnF) {
				dAppLog(LOG_CRI, "SESS_TO][IMSI:%s][IP:%s][C/P/H:%02d %02d %02d][LOG-OUT (SESS_CNT:%0.0f)"
					, si.szMIN, si.szFramedIP, si.uiCBit, si.uiPBit, si.uiHBit, gpShmem->rad_sess);
				logoutSCE (i, &si);
				Trace_LOGOUT (&AccInfo, TRACE_TYPE_LOGOUT_TIMEOUT);
				del_rad_sess (pkey, pBody);
				return 0;
			}
			++lflag;
		}
	}
	del_rad_sess (pkey, pBody);
	if (lflag>0) {
		dAppLog(LOG_CRI, "SESS_TO][IMSI:%s][Don't LOG-OUT, It is not logon session(%d)", si.szMIN, lflag);
		return -1;
	}
	dAppLog(LOG_CRI, "SESS_TO][IMSI:%s][IP:%s][C/P/H:%02d %02d %02d][LOG-OUT FAIL, there is not established-connection"
			, si.szMIN, si.szFramedIP, si.uiCBit, si.uiPBit, si.uiHBit);

	return -1;
}
#endif


void init_session (int shm_key)
{
	//gpShmem->rad_sess = 0;

    pHInfo_rad = hasho_init (shm_key/*SHM_RAD_SESS_KEY*/, sizeof (rad_sess_key), sizeof(rad_sess_key), sizeof(rad_sess_body), MAX_RAD_HASHO_SIZE, NULL, 0);
    if (pHInfo_rad == NULL) {
        dAppLog (LOG_CRI, "FAIL] sid hasho_init ERROR[%d][%s", errno, strerror(errno));
        exit(1);
    }
	pTimer_rad = timerN_init(MAX_RAD_HASHO_SIZE, sizeof (rad_sess_key));
	if (pTimer_rad == NULL) {
        dAppLog (LOG_CRI, "FAIL] sid timerN_init ERROR[%d][%s", errno, strerror(errno));
		exit(1);
	}
	/* restart �� shm�� �ִ� session �� ���Ῡ timer �籸�� */
	rad_timer_reconstructor ();
}


rad_sess_body * get_rad_sess (rad_sess_key *pkey)
{
	rad_sess_body 		*pBody, Body;

	pBody = find_rad_sess (pkey);
	if (pBody == NULL) {
		pBody = add_rad_sess (pkey, &Body);
		if (pBody == NULL) {
        	dAppLog (LOG_CRI, "FAIL] add_rad_sess ERROR, key[%s],[%d][%s", pkey->szCSID, errno, strerror(errno));
			return NULL;
		}
		pBody->timerID = timerN_add (pTimer_rad, (void *)&rad_session_timeout, (U8 *)pkey, RAD_SESS_KEY_SIZE, time(0) + gpMPTimer->sess_timeout);
		if (pBody->timerID == 0) {
			hasho_del (pHInfo_rad, (U8 *)pkey);
        	dAppLog (LOG_CRI, "FAIL] tid=0, timerN_add ERROR[%d][%s", errno, strerror(errno));
			return NULL;
		}
#ifdef SESS_CNT
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
	1. sess ������
	   - sess add, timer add, login
	2. sess �ְ� imsi ��ġ�ϸ�,
	   - packageID compare
	3. sess �ְ� imsi ��ġ�ϰ� packageID ��ġ�ϸ��,
	   - timer update
	4. sess �ְ� imsi ��ġ�ϰ� packageID ����ġ�ϸ�?
	   - sess update(packageID), timer update, login
	3. sess �ְ� imsi ����ġ�ϸ�
	   - ���� sess del, timer del
	   - ���� sess add, timer add, login
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
        	dAppLog (LOG_CRI, "FAIL] add_rad_sess ERROR[%d][%s", errno, strerror(errno));
			return NULL;
		}
		pBody->timerID = timerN_add (pTimer_rad, (void *)&rad_session_timeout, (U8 *)pkey, RAD_SESS_KEY_SIZE, time(0) + gpMPTimer->sess_timeout);
		if (pBody->timerID == 0) {
			hasho_del (pHInfo_rad, (U8 *)pkey);
        	dAppLog (LOG_CRI, "FAIL] tid=0, timerN_add ERROR[%d][%s", errno, strerror(errno));
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
#ifdef SESS_CNT
		dAppLog(LOG_CRI, ">>> TIMER COUNT[DEL] : %u", pTimer_rad->uiNodeCnt);
#endif
	}
	hasho_del (pHInfo_rad, (U8 *)pkey);
	if (gpShmem->rad_sess > 0) {
		gpShmem->rad_sess--;
#ifdef SESS_CNT
		dAppLog(LOG_CRI, ">>> SESSION COUNT[DEL] : %0.0f(%u)"
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
#ifdef SESS_CNT
	//((stMEMGINFO *)HASHO_PTR(pCALLHASH, pCALLHASH->offset_memginfo))->uiMemNodeAllocedCnt

	dAppLog(LOG_CRI, ">>> SESSION COUNT[ADD] : %0.0f(%u)"
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

// by june, 2010-10-12
#ifndef DISCONN_REQ 
char * find_imsi_rad_sess (rad_sess_key *pkey)
{
	rad_sess_body           *pBody;

	pBody = find_rad_sess(pkey); 
	if (pBody != NULL) {
		return (pBody->IMSI);
	}

	return NULL;
}
#endif

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
//#ifndef DISCONN_REQ 
#if 0
				dAppLog (LOG_DEBUG, "IMSI:%s][IP: %s][Timer Add -- TID: %lld"
						, pstData->IMSI, CVT_INT2STR_IP(pstKey->mobIP), pstData->timerID);
				pstData->timerID = timerN_add (pTimer_rad, (void *)&rad_session_timeout, (U8 *)pstKey, RAD_SESS_KEY_SIZE, time(0) + gpMPTimer->sess_timeout);
#else

				dAppLog (LOG_DEBUG, "IMSI:%s][IP: %s][Timer Add -- TID: %lld"
						, pstKey->szCSID, CVT_INT2STR_IP(pstData->uiFramedIP), pstData->timerID);
				pstData->timerID = timerN_add (pTimer_rad, (void *)&rad_session_timeout, (U8 *)pstKey, RAD_SESS_KEY_SIZE, time(0) + gpMPTimer->sess_timeout);
#endif
				timer_node_cnt++;
			}
			offset = p->offset_next;
		} /* while end */
	}
#ifdef SESS_CNT
		dAppLog(LOG_CRI, ">>> SESSION TIMER COUNT : %lld", timer_node_cnt);
#endif
}

