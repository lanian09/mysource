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
#include "comm_session.h"
#include "leg_sm_sess.h"
#include "leg.h"
#include "ipaf_sem.h"


/* Definition of New Constants */
stHASHOINFO             *gpHashInfo_SM;
stTIMERNINFO 			*gpHashTimer_SM;
extern _mem_check		*gpShmem;
extern MPTimer            	*gpCurMPTimer;

extern LEG_TOT_STAT_t		*gpstTotStat[DEF_STAT_SET_CNT];
extern unsigned int			gSIdx;
extern unsigned int			gCIdx;

extern unsigned int			gMyIdx;

int sm_sess_timeout (SM_SESS_KEY *pkey) 
{
	SM_SESS_BODY 		*pBody;

	pBody = find_sm_sess (pkey);
	if (pBody == NULL) {
		dAppLog(LOG_CRI, "T:%d] %u", ERR_50009 , pkey->uiSID);
		return -1;
	}

	// LOGON STATISTIC: SM_TIMEOUT UPDATE
	if( pBody->uiOperMode <= DEF_LOG_OUT ) {
		gpstTotStat[gSIdx]->stLogon[gMyIdx][pBody->uiOperMode].uiLogOn_Fail++;
		gpstTotStat[gSIdx]->stLogon[gMyIdx][pBody->uiOperMode].uiLogOn_APITimeout++;
	}
	else {
		dAppLog(LOG_CRI, "T:%d] %u %u", ERR_50015, pkey->uiSID, pBody->uiOperMode);
	}

	// DEL SM SESSION
	del_sm_sess (pkey, pBody);
	
	dAppLog(LOG_DEBUG, "T] %u %u", pkey->uiSID, pBody->uiOperMode);

	return 0;
}


void init_sm_sess (int shm_key)
{
	//gpShmem->sm_sess = 0;
	// SM HASH INIT
    gpHashInfo_SM = hasho_init( shm_key
								, SM_SESS_KEY_SIZE
								, SM_SESS_KEY_SIZE
								, SM_SESS_BODY_SIZE
								, MAX_SM_HASHO_SIZE
								, NULL
								, 0);
    if (gpHashInfo_SM==NULL) {
        dAppLog (LOG_CRI, "FAIL] hasho_init ERR=%d(%s)", errno, strerror(errno));
        exit(1);
    }

	// SM TIMER INIT
	gpHashTimer_SM = timerN_init (MAX_SM_HASHO_SIZE, SM_SESS_KEY_SIZE);
	if (gpHashTimer_SM == NULL) {
        dAppLog (LOG_CRI, "FAIL] timerN_init ERR=%d(%s)", errno, strerror(errno));
		exit(1);
	}

	// SESSION TIMER 재구성.
	// restart 후 shm에 있는 session 에 대햐여 timer 재구성
	//sm_timer_reconstructor ();
}


SM_SESS_BODY * get_sm_sess (SM_SESS_KEY *pkey)
{
	SM_SESS_BODY 		*pBody, Body;

	pBody = find_sm_sess (pkey);
	if (pBody == NULL) {
		pBody = add_sm_sess (pkey, &Body);
		if (pBody == NULL) {
        	dAppLog (LOG_CRI, "FAIL] add_sm_sess ERR=%d(%s) key[%u,"
					, errno, strerror(errno), pkey->uiSID);
			return NULL;
		}
		pBody->ullTimerID = timerN_add( gpHashTimer_SM
										, (void *)&sm_sess_timeout
										, (U8 *)pkey, SM_SESS_KEY_SIZE
										, time(0) + gpCurMPTimer->sm_sess_timeout );
		if (pBody->ullTimerID == 0) {
			hasho_del (gpHashInfo_SM, (U8 *)pkey);
        	dAppLog (LOG_CRI, "FAIL] tid=0, timerN_add ERR=%d(%s)", errno, strerror(errno));
			return NULL;
		}
        dAppLog (LOG_INFO, "add_sm_sess key=[%u] timeout[%d:%d"
				, pkey->uiSID, gpCurMPTimer->sm_sess_timeout, time(0) + gpCurMPTimer->sm_sess_timeout);
	} else {
		del_sm_sess (pkey, pBody);
		pBody = get_sm_sess (pkey);
		return pBody;
	}

	return pBody;
}


SM_SESS_BODY *add_sm_sess (SM_SESS_KEY *pkey, SM_SESS_BODY *pbody)
{
	stHASHONODE			*pHashNode;
	SM_SESS_BODY		*pData;

	pHashNode = hasho_add (gpHashInfo_SM, (U8 *)pkey, (U8 *)pbody);
	if (pHashNode == NULL) {
		return NULL;
	}

	pData = (SM_SESS_BODY *)RAD_HASHO_PTR (gpHashInfo_SM, pHashNode->offset_Data);

	gpShmem->sm_sess[gMyIdx]++;
#ifdef PRT_SM_SESS_CNT
	dAppLog(LOG_CRI, ">>> SM SESSION ADD COUNT : %0.0f", gpShmem->sm_sess[gMyIdx]);
	U32	uiNodeCnt=0;
	uiNodeCnt = ((stMEMGINFO *)RAD_HASHO_PTR(gpHashInfo_SM, gpHashInfo_SM->offset_memginfo))->uiMemNodeAllocedCnt;
	dAppLog(LOG_CRI, ">>> [%u] SM SESSION NODE COUNT[ADD] : %u", pkey->uiSID, uiNodeCnt);
#endif
	return pData;
}


SM_SESS_BODY *find_sm_sess (SM_SESS_KEY *pkey)
{
	stHASHONODE			*pHashNode;
	SM_SESS_BODY		*pData;

	pHashNode = hasho_find (gpHashInfo_SM, (U8 *)pkey);
	if (pHashNode == NULL) {
		return NULL;
	}

	pData = (SM_SESS_BODY *)RAD_HASHO_PTR (gpHashInfo_SM, pHashNode->offset_Data);
	
	return pData;
}


void del_sm_sess (SM_SESS_KEY *pkey, SM_SESS_BODY *pbody) 
{
	if (pbody->ullTimerID != 0) {
		timerN_del (gpHashTimer_SM, pbody->ullTimerID);
		pbody->ullTimerID = 0;
	}
	hasho_del (gpHashInfo_SM, (U8 *)pkey);
	if (gpShmem->sm_sess[gMyIdx] > 0) {
		gpShmem->sm_sess[gMyIdx]--;
	}
#ifdef PRT_SM_SESS_CNT
	dAppLog(LOG_CRI, ">>> SM SESSION DEL COUNT : %0.0f", gpShmem->sm_sess[gMyIdx]);
	U32	uiNodeCnt=0;
	//U32 uiTMNodeCnt=0;
	uiNodeCnt = ((stMEMGINFO *)RAD_HASHO_PTR(gpHashInfo_SM, gpHashInfo_SM->offset_memginfo))->uiMemNodeAllocedCnt;
	//uiTMNodeCnt = ((stMEMGINFO *)RAD_HASHO_PTR(gpHashTimer_SM, gpHashTimer_SM->offset_memginfo))->uiMemNodeAllocedCnt;
	//dAppLog(LOG_CRI, ">>> SM SESSION NODE COUNT[DEL] : %u(%u)", uiNodeCnt, uiTMNodeCnt);
	dAppLog(LOG_CRI, "<<< [%u] SM SESSION NODE COUNT[DEL] : %u", pkey->uiSID, uiNodeCnt);
#endif
}


int del_sm_sess_key (SM_SESS_KEY *pkey)
{
	SM_SESS_BODY       *pBody;

	pBody = find_sm_sess (pkey);
	if (pBody != NULL) {
		//if (pBody->uiOperMode == OPER_MODE_LOGIN)
		del_sm_sess (pkey, pBody);
		return 0;
	}
	return -1;
}

void sm_timer_reconstructor (void)
{
	int                 i;
	long long           timer_node_cnt=0;
	OFFSET              offset;
	stHASHONODE         *p;
	stMEMGNODEHDR       *pMEMGNODEHDR;

	SM_SESS_KEY        	*pstKey;
	SM_SESS_BODY       	*pstData;

	for(i = 0; i < gpHashInfo_SM->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(gpHashInfo_SM, gpHashInfo_SM->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(gpHashInfo_SM, offset);

			pstKey  = (SM_SESS_KEY *) HASHO_PTR (gpHashInfo_SM, p->offset_Key);
			pstData = (SM_SESS_BODY*) HASHO_PTR (gpHashInfo_SM, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				dAppLog (LOG_DEBUG, "%s][SID:%u][OperMode:%u][Timer Add -- TID: %lld"
						, __FUNCTION__, pstKey->uiSID, pstData->uiOperMode, pstData->ullTimerID);
				pstData->ullTimerID = timerN_add( gpHashTimer_SM
												, (void *)&sm_sess_timeout
												, (U8 *)pstKey
												, SM_SESS_KEY_SIZE
												, time(0) + gpCurMPTimer->sm_sess_timeout);
				timer_node_cnt++;
			}
			offset = p->offset_next;
		} /* while end */
	}
#ifdef PRT_SM_SESS_CNT
	U32	uiNodeCnt=0;
	uiNodeCnt = ((stMEMGINFO *)RAD_HASHO_PTR(gpHashInfo_SM, gpHashInfo_SM->offset_memginfo))->uiMemNodeAllocedCnt;
	dAppLog(LOG_CRI, "TM REBUILD] SESSION TIMER COUNT : %lld", timer_node_cnt);
#endif
}

