/******************************************************************************* 
        @file   stack_list.c
 *      - A_SCTP 프로세스를 초기화 하는 함수들
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *      $Id: stack_list.c,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 *
 *      @Author     $Author: dcham $
 *      @version    $Revision: 1.1.1.1 $
 *      @date       $Date: 2011/08/29 05:56:42 $
 *      @ref        stack_list.c
 *
 *      @section    Intro(소개)
 *      - ASSOCIATION 관련 MMDB
 *
 *      @section    Requirement
 *
*******************************************************************************/

/**A.1*  File Inclusion *******************************************************/
#include <sys/time.h>
#include <sctpstack.h>
#include "loglib.h"

/**B.1*  Definition of New Constants ******************************************/

/**B.2*  Definition of New Type  **********************************************/

/**C.1*  Declaration of Variables  ********************************************/

/**D.1*  Definition of Functions  *********************************************/

/**D.2*  Definition of Functions  *********************************************/

/*******************************************************************************

*******************************************************************************/
pSTACK_LIST pGetStackFirstNode()
{
	if (pstStackTbl->uiUsedFirst <= 0)
		return NULL;
	return &pstStackTbl->stNode[pstStackTbl->uiFreeList];
}


/*******************************************************************************

*******************************************************************************/
void InitStackList()
{
	int	i;

	for (i = 0;i < MAX_STACK_LIST - 1;i++) 
	{
		pstStackTbl->stNode[i].prev = 0;
		pstStackTbl->stNode[i].next = i + 1;
		pstStackTbl->stNode[i].uiIndex = i;
		pstStackTbl->stNode[i].uiStackNext = 0;
		pstStackTbl->stNode[i].uiStackPrev = 0;
		memset(&pstStackTbl->stNode[i].stTKey, 0x00, ASSO_KEY_SIZE);
		memset(&pstStackTbl->stNode[i].stSKey, 0x00, STACK_KEY_SIZE);
		pstStackTbl->stNode[i].stStackTime.tv_sec = 0;
		pstStackTbl->stNode[i].stStackTime.tv_usec = 0;
	}

	pstStackTbl->stNode[i].prev = 0;
	pstStackTbl->stNode[i].next = 0;
	pstStackTbl->stNode[i].uiIndex = MAX_STACK_LIST - 1;
	pstStackTbl->stNode[i].uiStackNext = 0;
	pstStackTbl->stNode[i].uiStackPrev = 0;
	memset(&pstStackTbl->stNode[i].stTKey, 0x00, ASSO_KEY_SIZE);
	memset(&pstStackTbl->stNode[i].stSKey, 0x00, STACK_KEY_SIZE);
	pstStackTbl->stNode[i].stStackTime.tv_sec = 0;
	pstStackTbl->stNode[i].stStackTime.tv_usec = 0;

	pstStackTbl->uiFreeList = 1;
	pstStackTbl->uiUsedFirst = 0;
	pstStackTbl->uiUsedLast = 0;
	pstStackTbl->uiCurrCount = 0;

}


/*******************************************************************************

*******************************************************************************/
pSTACK_LIST pGetStackNode(UINT *puiIndex)
{
	pSTACK_LIST node;

	if(pstStackTbl->stNode[pstStackTbl->uiFreeList].next < 1 ) 
	{
    	log_print(LOGN_CRI, "Stack::Next Node Free=%u idx=%u prev=%u next=%u", 
			pstStackTbl->uiFreeList,
			pstStackTbl->stNode[pstStackTbl->uiFreeList].uiIndex, 
			pstStackTbl->stNode[pstStackTbl->uiFreeList].next,
			pstStackTbl->stNode[pstStackTbl->uiFreeList].prev);
		return NULL;
	}

	if(	pstStackTbl->stNode[pstStackTbl->uiFreeList].uiIndex != 
		pstStackTbl->uiFreeList)
	{
		log_print(LOGN_CRI, "Stack::GET Index Corrupted NodeIndex=%u FreeIndex=%u",
			pstStackTbl->stNode[pstStackTbl->uiFreeList].uiIndex,
			pstStackTbl->uiFreeList);
		return NULL;
	}

	*puiIndex = pstStackTbl->uiFreeList;
	node = &pstStackTbl->stNode[pstStackTbl->uiFreeList];
	pstStackTbl->uiFreeList = 
		pstStackTbl->stNode[pstStackTbl->uiFreeList].next;

	// Initialize Local Memory
	node->next = pstStackTbl->uiUsedFirst;
	node->prev = 0;
	gettimeofday(&node->stStackTime, NULL);

	if (pstStackTbl->uiUsedFirst > 0) 
		pstStackTbl->stNode[pstStackTbl->uiUsedFirst].prev = *puiIndex;

	pstStackTbl->uiUsedFirst = *puiIndex;

	if (pstStackTbl->uiUsedLast == 0)
        pstStackTbl->uiUsedLast = *puiIndex;

    log_print(LOGN_INFO, "Stack::Get Node idx=%u First=%u Last=%u Free=%u", 
		*puiIndex, pstStackTbl->uiUsedFirst, pstStackTbl->uiUsedLast, 
		pstStackTbl->uiFreeList);

	pstStackTbl->uiCurrCount++;

	return node;
}


/*******************************************************************************

*******************************************************************************/
void FreeStackNode(pSTACK_LIST node)
{
	int dPos;

	dPos = node - &pstStackTbl->stNode[0];
	if (dPos < 1 || dPos > MAX_STACKREAL_LIST)
	{
		log_print(LOGN_CRI, "Stack::Free Index[%u] Pos Must (1 < Node=%d < %d)",
			node->uiIndex, dPos, MAX_STACKREAL_LIST-1);
		return;
	}

	if (node->prev > 0) {
		if (pstStackTbl->stNode[node->prev].next == node->uiIndex)
			pstStackTbl->stNode[node->prev].next = node->next;
		else
			log_print(LOGN_CRI, "Stack::List Corrupted Node=%d Index=%u PREV=%u Next=%u",
				dPos, node->uiIndex, node->prev, node->next);
	} else if (node->uiIndex == pstStackTbl->uiUsedFirst) {
		pstStackTbl->uiUsedFirst = node->next;
	} else {
		log_print(LOGN_CRI, "Stack::List Corrupted Node=%d INDEX=%u PREV=%u Next=%u Free=%u",
			dPos, node->uiIndex, node->prev, node->next, pstStackTbl->uiUsedFirst);
	}

	if (node->next > 0) {
		if (pstStackTbl->stNode[node->next].prev == node->uiIndex)
			pstStackTbl->stNode[node->next].prev = node->prev;
		else
			log_print(LOGN_CRI, "Stack::List Corrupted Node=%d INDEX=%u Prev=%u NEXT=%u",
				dPos, node->uiIndex, node->prev, node->next);
	} else if (node->uiIndex == pstStackTbl->uiUsedLast) {
		pstStackTbl->uiUsedLast = node->prev;
	} else {
		log_print(LOGN_CRI, "Stack::List Corrupted Node=%d INDEX=%u PREV=%d Next=%d LAST=%u",
            dPos, node->uiIndex, node->prev, node->next, pstStackTbl->uiUsedLast);
	}

	/*
	log_print(LOGN_INFO, "Stack::Free Node idx=%u Index=%u First=%u Last=%u Free=%u FIndex=%u",
        dPos, node->uiIndex, pstStackTbl->uiUsedFirst, 
		pstStackTbl->uiUsedLast, pstStackTbl->uiFreeList, 
		pstStackTbl->stNode[pstStackTbl->uiFreeList].uiIndex);
	*/

	/* Node Memory Clear */
	node->prev = 0;
	node->next = pstStackTbl->uiFreeList;
	node->uiStackNext = 0;
	node->uiStackPrev = 0;
	memset(&node->stTKey, 0x00, ASSO_KEY_SIZE);
	memset(&node->stSKey, 0x00, STACK_KEY_SIZE);
	pstStackTbl->uiFreeList = node->uiIndex;
	pstStackTbl->uiCurrCount--;

	/*
	log_print(LOGN_INFO, "Stack::Free Node idx=%u Index=%u First=%u Last=%u Free=%u FNextIndex=%u",
        dPos, node->uiIndex, pstStackTbl->uiUsedFirst, 
		pstStackTbl->uiUsedLast, pstStackTbl->uiFreeList, 
		pstStackTbl->stNode[pstStackTbl->uiFreeList].next);
	*/
}


/*******************************************************************************

*******************************************************************************/
pSTACK_LIST pSetStackNode(UINT uiIndex)
{
	if (uiIndex < MAX_STACK_LIST - 1 && uiIndex != 0)
		return &pstStackTbl->stNode[uiIndex];
	return NULL;
}

/*
* $Log: stack_list.c,v $
* Revision 1.1.1.1  2011/08/29 05:56:42  dcham
* NEW OAM SYSTEM
*
* Revision 1.1  2011/08/05 02:38:57  uamyd
* A_SCTP modified
*
* Revision 1.2  2011/01/11 04:09:05  uamyd
* modified
*
* Revision 1.1.1.1  2010/08/23 01:13:04  uamyd
* DQMS With TOTMON, 2nd-import
*
* Revision 1.1  2009/05/27 07:42:36  dqms
* Makefile
*
* Revision 1.1  2009/05/13 11:42:49  upst_cvs
* NEW
*
* Revision 1.1  2008/01/11 12:09:02  pkg
* import two-step by uamyd
*
* Revision 1.3  2007/06/01 15:47:06  doit1972
* MODIFY LOG
*
* Revision 1.2  2007/04/29 13:09:34  doit1972
* CVS LOG 정보 추가
*
*/
