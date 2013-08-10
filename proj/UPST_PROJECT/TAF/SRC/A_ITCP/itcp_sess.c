/**
 * Include headers
 */
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

// TOP
#include "common_stg.h"

// LIB 
#include "loglib.h"
#include "typedef.h"

// .
#include "itcp_sess.h"

/**
 * Declare variables
 */
extern stMEMSINFO 			*pMEMSINFO;
extern CALL_KEY 			stCALLKEY;
extern SESSKEY_TBL 			*pstSESSKEYTBL;
extern SESSKEY_LIST 		*pSESSKEYLIST;
extern stHASHONODE 			*pSESSKEYNODE;

void InitTCPKeyList()
{
	int 			i;
//	SESSKEY_TBL 	*pstSESSKEYTBL = &stSESSKEYTBL;

	for (i=0; i < MAX_SESSKEY_LIST-1; i++)
	{
		pstSESSKEYTBL->stNode[i].prev = 0;
		pstSESSKEYTBL->stNode[i].next = i + 1;
		pstSESSKEYTBL->stNode[i].uiIndex = i;
		pstSESSKEYTBL->stNode[i].uiTCPNext = 0;
		pstSESSKEYTBL->stNode[i].uiTCPPrev = 0;
		memset(&pstSESSKEYTBL->stNode[i].stSessKey, 0x00, TCP_SESS_KEY_SIZE);
	}

	pstSESSKEYTBL->stNode[i].prev = 0;
	pstSESSKEYTBL->stNode[i].next = 0; 
	pstSESSKEYTBL->stNode[i].uiIndex = MAX_SESSKEY_LIST - 1;
	pstSESSKEYTBL->stNode[i].uiTCPNext = 0;
	pstSESSKEYTBL->stNode[i].uiTCPPrev = 0;
	memset(&pstSESSKEYTBL->stNode[i].stSessKey, 0x00, TCP_SESS_KEY_SIZE);

	pstSESSKEYTBL->uiFreeList = 1;
	pstSESSKEYTBL->uiUsedFirst = 0;
	pstSESSKEYTBL->uiUsedLast = 0;
	pstSESSKEYTBL->uiCurrCount = 0;
}   


pSESSKEY_LIST pGetSessKeyList(UINT *puiIndex)
{
    pSESSKEY_LIST 	node;
	pSESSKEY_TBL 	pstStackTbl = pstSESSKEYTBL;

    if(pstStackTbl->stNode[pstStackTbl->uiFreeList].next < 1 )
    {
        log_print(LOGN_CRI, "#### Stack::Next Node Free=%u idx=%u prev=%u next=%u",
            pstStackTbl->uiFreeList,
            pstStackTbl->stNode[pstStackTbl->uiFreeList].uiIndex,
            pstStackTbl->stNode[pstStackTbl->uiFreeList].next,
            pstStackTbl->stNode[pstStackTbl->uiFreeList].prev);
        return NULL;
    }

    if( pstStackTbl->stNode[pstStackTbl->uiFreeList].uiIndex != pstStackTbl->uiFreeList)
    {
        log_print(LOGN_CRI, "#### Stack::GET Index Corrupted NodeIndex=%u FreeIndex=%u",
            pstStackTbl->stNode[pstStackTbl->uiFreeList].uiIndex,
            pstStackTbl->uiFreeList);
        return NULL;
    }

    *puiIndex = pstStackTbl->uiFreeList;
    node = &pstStackTbl->stNode[pstStackTbl->uiFreeList];
    pstStackTbl->uiFreeList = pstStackTbl->stNode[pstStackTbl->uiFreeList].next;

    // Initialize Local Memory
    node->next = pstStackTbl->uiUsedFirst;
    node->prev = 0;

    if (pstStackTbl->uiUsedFirst > 0)
        pstStackTbl->stNode[pstStackTbl->uiUsedFirst].prev = *puiIndex;

    pstStackTbl->uiUsedFirst = *puiIndex;

    if (pstStackTbl->uiUsedLast == 0)
        pstStackTbl->uiUsedLast = *puiIndex;

    //log_print(LOGN_INFO, "Stack::Get Node idx=%u First=%u Last=%u Free=%u", 
        //*puiIndex, pstStackTbl->uiUsedFirst, pstStackTbl->uiUsedLast, 
        //pstStackTbl->uiFreeList);

    pstStackTbl->uiCurrCount++;

    return node;
}

int dGetSessKeyList(pSESSKEY_LIST *pstStack)
{                           
	UINT 			uiIndex;    
	pSESSKEY_LIST 	pstNode;    

	pstNode = pGetSessKeyList(&uiIndex);
	if(pstNode == NULL)     
	{                       
		log_print(LOGN_WARN,"#### STACKONLY::LIST GET NULL");
		return -1;
	}                       

	*pstStack = pstNode;    

	return 0;                   
}                        

int dAddSessKeyNext(pSESSKEY_LIST pstHashStack, pSESSKEY_LIST pstAddStack)
{
	pSESSKEY_TBL 	pstStackTbl = pstSESSKEYTBL;
	pSESSKEY_LIST   pstStack;

	if(pstAddStack->uiIndex >= MAX_SESSKEY_LIST || pstAddStack->uiIndex == 0)
	{
		log_print(LOGN_CRI, "#### ADDSTACKNEXT::Linked List Error Index=%u", pstAddStack->uiIndex);
		return -1;
	}

	pstStack = &pstStackTbl->stNode[pstHashStack->uiIndex];
	if(!memcmp(&pstStack->stSessKey, &pstAddStack->stSessKey, TCP_SESS_KEY_SIZE))
	{
		log_print(LOGN_CRI, "#### ADDSTACKNEXT::SAME TCP SESSION KEY S1(I=%u P=%u N=%u) S2(I=%u P=%u N=%u)",
				pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext,
				pstAddStack->uiIndex, pstAddStack->uiTCPPrev, pstAddStack->uiTCPNext);
		return -2;
	}

	pstAddStack->uiTCPPrev = pstStack->uiIndex;
	pstAddStack->uiTCPNext = pstStack->uiTCPNext;

	if(pstStack->uiTCPNext > 0) {
		if(pstStackTbl->stNode[pstStack->uiTCPNext].uiTCPPrev == pstStack->uiIndex)
			pstStackTbl->stNode[pstStack->uiTCPNext].uiTCPPrev = pstAddStack->uiIndex;
		else
			log_print(LOGN_CRI, "#### STACKNextLinked List Corrupted Index=%u PREV=%u Next=%u",
					pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext);
	} else {
		log_print(LOGN_DEBUG, "#### STACKNextLinked List Corrupted INDEX=%u PREV=%u Next=%u", 
				pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext);
	}

	pstStack->uiTCPNext = pstAddStack->uiIndex;

	log_print(LOGN_DEBUG, "#### ADDSTACKNEXT OLD: %d OLD->NEXT: %d NEW: %d NEW->NEXT: %d",
			pstStack->uiIndex, pstStack->uiTCPNext, pstAddStack->uiIndex, pstAddStack->uiTCPNext);
	log_print(LOGN_DEBUG, "#### ADDSTACKNEXT old->next: %d old->prev: %d new->next: %d new->prev: %d",
			pstStack->next, pstStack->prev, pstAddStack->next, pstAddStack->prev);


	return 0;
}

void FreeSessKeyList(pSESSKEY_LIST node)
{
	int dPos;
	pSESSKEY_TBL 	pstStackTbl = pstSESSKEYTBL;


	dPos = node - &pstStackTbl->stNode[0];
	if (dPos < 1 || dPos > MAX_SESSKEY_LIST)
	{
		log_print(LOGN_CRI, "#### Stack::Free Index[%u] Pos Must (1 < Node=%d < %d)",
			node->uiIndex, dPos, MAX_SESSKEY_LIST-1);
		return;
	}

	if (node->prev > 0) {
		if (pstStackTbl->stNode[node->prev].next == node->uiIndex)
			pstStackTbl->stNode[node->prev].next = node->next;
		else
			log_print(LOGN_CRI, "#### ERROR Stack::List Corrupted Node=%d INDEX=%u PREV=%u NEXT=%u",
					dPos, node->uiIndex, node->prev, node->next);
	} else if (node->uiIndex == pstStackTbl->uiUsedFirst) {
		pstStackTbl->uiUsedFirst = node->next;
	} else {
		log_print(LOGN_CRI, "#### Stack::List Corrupted Node=%d INDEX=%u PREV=%u NEXT=%u FREE=%u",
			dPos, node->uiIndex, node->prev, node->next, pstStackTbl->uiUsedFirst);
	}

	if (node->next > 0) {
		if (pstStackTbl->stNode[node->next].prev == node->uiIndex)
			pstStackTbl->stNode[node->next].prev = node->prev;
		else
			log_print(LOGN_CRI, "#### Stack::List Corrupted Node=%d INDEX=%u Prev=%u NEXT=%u",
					dPos, node->uiIndex, node->prev, node->next);
	} else if (node->uiIndex == pstStackTbl->uiUsedLast) {
		pstStackTbl->uiUsedLast = node->prev;
		log_print(LOGN_DEBUG, "#### Stack::List uiUsedLast=%d Node=%d INDEX=%u Prev=%u NEXT=%u",
					pstStackTbl->uiUsedLast, dPos, node->uiIndex, node->prev, node->next);
	} else {
		log_print(LOGN_CRI, "#### Stack::List INFO Node=%d INDEX=%u PREV=%d Next=%d LAST=%u",
            dPos, node->uiIndex, node->prev, node->next, pstStackTbl->uiUsedLast);
	}

	log_print(LOGN_DEBUG, "#### Stack::Free Node idx=%u Index=%u First=%u Last=%u Free=%u FIndex=%u",
        dPos, node->uiIndex, pstStackTbl->uiUsedFirst, 
		pstStackTbl->uiUsedLast, pstStackTbl->uiFreeList, 
		pstStackTbl->stNode[pstStackTbl->uiFreeList].uiIndex);

	/* Node Memory Clear */
	node->prev = 0;
	node->next = pstStackTbl->uiFreeList;
	node->uiTCPNext = 0;
	node->uiTCPPrev = 0;
	memset(&node->stSessKey, 0x00, TCP_SESS_KEY_SIZE);
	pstStackTbl->uiFreeList = node->uiIndex;
	pstStackTbl->uiCurrCount--;

	log_print(LOGN_DEBUG, "#### Stack::Free Node idx=%u Index=%u First=%u Last=%u Free=%u FNextIndex=%u",
        dPos, node->uiIndex, pstStackTbl->uiUsedFirst, 
		pstStackTbl->uiUsedLast, pstStackTbl->uiFreeList, 
		pstStackTbl->stNode[pstStackTbl->uiFreeList].next);
}

int dDelSessKeyList(pSESSKEY_LIST pstDelStack, stHASHOINFO *pTCPHASH, stMEMSINFO *pMEMSINFO, UINT uiLastLogTime, stTIMERNINFO *pTIMER)
{
	UINT 			uiIndex;
	pSESSKEY_TBL 	pstStackTbl = pstSESSKEYTBL;
	pSESSKEY_LIST 	pstStack;
	stHASHONODE 	*pHASHNODE;
	TCP_SESS 		*pTCPSESS;
	int 			i, dSessCnt=0, dCurrSessCnt = 0;

    if(pstDelStack->uiIndex >= MAX_SESSKEY_LIST || pstDelStack->uiIndex ==0) {
        log_print(LOGN_WARN, "#### STACKDEL Linked List Parents Key Corrupted Node Index=%u Prev=%u Next=%u",
            pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext);
        return -1;
    }

//	pstStack = pstDelStack;
	uiIndex = pstDelStack->uiIndex;

	while(uiIndex != 0) {
		dSessCnt++;
		pstStack = &(pstStackTbl->stNode[uiIndex]);
		log_print(LOGN_DEBUG, "#### WHILE SESSCNT: %d DEL->NEXT: %d INDEX: %d INDEX: %d NEXT: %d", 
				dSessCnt, pstDelStack->uiTCPNext, uiIndex, pstStack->uiIndex, pstStack->uiTCPNext );
		if(pstStack->uiTCPNext > 0) {
			uiIndex = pstStack->uiTCPNext;
		} else 
			break;
	}
	log_print(LOGN_DEBUG, "#### STACKDEL SESSCNT: %d INDEX: %d", dSessCnt, uiIndex );
			
	for(i=0; i<dSessCnt && uiIndex > 0; i++) {
		pstStack = &pstStackTbl->stNode[uiIndex];
		uiIndex = pstStack->uiTCPPrev;

		if(uiLastLogTime == 0 || pstStack->SessStartTime < uiLastLogTime) {
			if(pstStack->uiTCPPrev > 0) {
				if(pstStackTbl->stNode[pstStack->uiTCPPrev].uiTCPNext == pstStack->uiIndex)
					pstStackTbl->stNode[pstStack->uiTCPPrev].uiTCPNext = pstStack->uiTCPNext;
				else
					log_print(LOGN_WARN, "#### ERROR : STACKDEL Linked List Corrupted Node Index=%u Prev=%u Next=%u NextPrev=%u",
							pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext,
							pstStackTbl->stNode[pstStack->uiTCPPrev].uiTCPNext);
			} else {
				log_print(LOGN_DEBUG, "#### STACKDEL Linked List Node First Index=%u Prev=%u Next=%u i=%d",
						pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext, i);
			}
			if(pstStack->uiTCPNext > 0) {
				if(pstStackTbl->stNode[pstStack->uiTCPNext].uiTCPPrev == pstStack->uiIndex)
					pstStackTbl->stNode[pstStack->uiTCPNext].uiTCPPrev = pstStack->uiTCPPrev;
				else
					log_print(LOGN_WARN, "#### ERROR : STACKDEL Linked List Corrupted Node Index=%u Prev=%u Next=%u NextPrev=%u",
							pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext,
							pstStackTbl->stNode[pstStack->uiTCPNext].uiTCPPrev);
			} else {
				log_print(LOGN_DEBUG, "#### STACKDEL Linked List Node First Index=%u Prev=%u Next=%u i=%d",
						pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext, i);
			}

			if((pHASHNODE = hasho_find(pTCPHASH, (U8 *)&pstStack->stSessKey)) != NULL) {
				pTCPSESS = (TCP_SESS *)nifo_ptr(pTCPHASH, pHASHNODE->offset_Data);
				log_print(LOGN_DEBUG, "#### dCloseSession SRC[%d.%d.%d.%d][%d] DEST[%d.%d.%d.%d][%d]", 
						HIPADDR(pstStack->stSessKey.uiSIP), pstStack->stSessKey.usSPort, 
						HIPADDR(pstStack->stSessKey.uiDIP), pstStack->stSessKey.usDPort);
				pTCPSESS->ucEndStatus = DEF_END_LONGLAST;
				dCloseSession(pMEMSINFO, pTCPHASH, &pstStack->stSessKey, pTCPSESS);
				timerN_del(pTIMER, pTCPSESS->timerNID);
			} else {
				log_print(LOGN_DEBUG, "#### NOT FOUND HASH SRC[%d.%d.%d.%d][%d] DEST[%d.%d.%d.%d][%d]", 
						HIPADDR(pstStack->stSessKey.uiSIP), pstStack->stSessKey.usSPort, 
						HIPADDR(pstStack->stSessKey.uiDIP), pstStack->stSessKey.usDPort);
			}
			log_print(LOGN_DEBUG, "#### OLD: %d OLD->NEXT: %d old->next: %d old->prev: %d", 
					pstStack->uiIndex, pstStack->uiTCPNext, pstStack->next, pstStack->prev);

			if((i == dSessCnt - 1) && (pstStack->uiTCPNext > 0)) {
				pSESSKEY_LIST 	pstHeadStack;
				pstHeadStack = &pstStackTbl->stNode[pstStack->uiTCPNext];
				memcpy(pstDelStack, pstHeadStack, SESSKEY_LIST_SIZE);
			}

			FreeSessKeyList(pstStack);
		} else {
			dCurrSessCnt++;
			log_print(LOGN_DEBUG, "#### TIME NOT MATCH HASH STIME:%u LTIME:%u SRC[%d.%d.%d.%d][%d] DEST[%d.%d.%d.%d][%d] CNT:%d", 
					pstStack->SessStartTime, uiLastLogTime,
					HIPADDR(pstStack->stSessKey.uiSIP), pstStack->stSessKey.usSPort, 
					HIPADDR(pstStack->stSessKey.uiDIP), pstStack->stSessKey.usDPort,
					dCurrSessCnt);
		}
	}

    return dCurrSessCnt;
}

int dDelSessKeyNext(pSESSKEY_LIST pstDelStack, TCP_SESS_KEY *pTCPSESSKEY)
{
	UINT 			uiIndex;
	pSESSKEY_TBL 	pstStackTbl = pstSESSKEYTBL;
	pSESSKEY_LIST 	pstStack;
	int 			i, dSessCnt=0, dCurrSessCnt = 0;


    if(pstDelStack->uiIndex >= MAX_SESSKEY_LIST || pstDelStack->uiIndex ==0) {
        log_print(LOGN_WARN, "#### STACKDEL Linked List Parents Key Corrupted Node Index=%u Prev=%u Next=%u",
            pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext);
        return -1;
    }

	uiIndex = pstDelStack->uiIndex;

	while(uiIndex != 0) {
		dSessCnt++;
		pstStack = &(pstStackTbl->stNode[uiIndex]);
		log_print(LOGN_DEBUG, "#### WHILE SESSCNT: %d DEL->NEXT: %d INDEX: %d INDEX: %d NEXT: %d", 
				dSessCnt, pstDelStack->uiTCPNext, uiIndex, pstStack->uiIndex, pstStack->uiTCPNext );

		if(pstStack->uiTCPNext > 0) {
			uiIndex = pstStack->uiTCPNext;
		} else 
			break;
	}
	log_print(LOGN_DEBUG, "#### STACKDEL SESSCNT: %d INDEX: %d", dSessCnt, uiIndex );
			
	for(i=0; i<dSessCnt && uiIndex > 0; i++) {
		pstStack = &pstStackTbl->stNode[uiIndex];
		uiIndex = pstStack->uiTCPPrev;
		
		if(!memcmp(&pstStack->stSessKey, pTCPSESSKEY, TCP_SESS_KEY_SIZE)) {
			log_print(LOGN_DEBUG, "#### FOUND TCPSESSKEY SRC[%d.%d.%d.%d][%d] DEST[%d.%d.%d.%d][%d]", 
					HIPADDR(pstStack->stSessKey.uiSIP), pstStack->stSessKey.usSPort, 
					HIPADDR(pstStack->stSessKey.uiDIP), pstStack->stSessKey.usDPort);

			if(pstStack->uiTCPPrev > 0) {
				if(pstStackTbl->stNode[pstStack->uiTCPPrev].uiTCPNext == pstStack->uiIndex)
					pstStackTbl->stNode[pstStack->uiTCPPrev].uiTCPNext = pstStack->uiTCPNext;
				else
					log_print(LOGN_WARN, "#### ERROR : STACKDEL Linked List Corrupted Node Index=%u Prev=%u Next=%u NextPrev=%u",
							pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext,
							pstStackTbl->stNode[pstStack->uiTCPPrev].uiTCPNext);
			} else {
				log_print(LOGN_DEBUG, "#### STACKDEL Linked List Node First Index=%u Prev=%u Next=%u i=%d",
						pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext, i);
			}
			if(pstStack->uiTCPNext > 0) {
				if(pstStackTbl->stNode[pstStack->uiTCPNext].uiTCPPrev == pstStack->uiIndex)
					pstStackTbl->stNode[pstStack->uiTCPNext].uiTCPPrev = pstStack->uiTCPPrev;
				else
					log_print(LOGN_WARN, "#### ERROR : STACKDEL Linked List Corrupted Node Index=%u Prev=%u Next=%u NextPrev=%u",
							pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext,
							pstStackTbl->stNode[pstStack->uiTCPNext].uiTCPPrev);
			} else {
				log_print(LOGN_DEBUG, "#### STACKDEL Linked List Node First Index=%u Prev=%u Next=%u i=%d",
						pstStack->uiIndex, pstStack->uiTCPPrev, pstStack->uiTCPNext, i);
			}

			if((i == dSessCnt - 1) && (pstStack->uiTCPNext > 0)) {
				pSESSKEY_LIST 	pstHeadStack;
				pstHeadStack = &pstStackTbl->stNode[pstStack->uiTCPNext];
				memcpy(pstDelStack, pstHeadStack, SESSKEY_LIST_SIZE);
			}

			FreeSessKeyList(pstStack);
		} else {
			dCurrSessCnt++;
			log_print(LOGN_DEBUG, "#### TCPSESSKEY NOT MATCH SRC[%d.%d.%d.%d][%d] DEST[%d.%d.%d.%d][%d] CNT:%d", 
					HIPADDR(pstStack->stSessKey.uiSIP), pstStack->stSessKey.usSPort, 
					HIPADDR(pstStack->stSessKey.uiDIP), pstStack->stSessKey.usDPort,
					dCurrSessCnt);
		}
	}

    return dCurrSessCnt;
}
