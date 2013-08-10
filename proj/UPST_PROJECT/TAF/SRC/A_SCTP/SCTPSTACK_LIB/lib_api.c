/******************************************************************************* 
        @file   lib_api.c
 *      - A_SCTP 프로세스를 초기화 하는 함수들
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *      $Id: lib_api.c,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 *
 *      @Author     $Author: dcham $
 *      @version    $Revision: 1.1.1.1 $
 *      @date       $Date: 2011/08/29 05:56:42 $
 *      @ref        lib_api.c
 *
 *      @section    Intro(소개)
 *      - ASSOCIATION 관련 MMDB
 *
 *      @section    Requirement
 *
*******************************************************************************/

/**A.1*  File Inclusion *******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include <sctpstack.h>
#include "loglib.h"

/**B.1*  Definition of New Constants ******************************************/

/**B.2*  Definition of New Type  **********************************************/

/**C.1*  Declaration of Variables  ********************************************/

/**D.1*  Definition of Functions  *********************************************/

/**D.2*  Definition of Functions  *********************************************/

/*******************************************************************************
* MMDB User Function
*******************************************************************************/
int dInsertMMDB(PASSO_DATA pstSrc, PASSO_DATA *ppstDst)
{
	int				dRet = 0;
	PASSO_DATA		pstCheck;

	dRet = Insert_SESS(pstSrc);
	if(dRet < 0)	/* RESOURCE ERROR */
	{
		log_print( LOGN_WARN, "MMDB RESOURCE ERROR INSERT[%d][%u][%u][%u]", 
							dRet, pstSrc->stKey.usSrcPort, pstSrc->stKey.usDestPort, pstSrc->stKey.uiSysInfo );
		return -2;
	}
	else if(dRet > 0)   /* EXIST DATA */
    {
 		log_print( LOGN_WARN, "MMDB ALEADY EXIST INSERT[%d][%u][%u][%u]",
     						dRet, pstSrc->stKey.usSrcPort, pstSrc->stKey.usDestPort, pstSrc->stKey.uiSysInfo );
    	return -100;
    }

	pstCheck = Search_SESS(&pstSrc->stKey);
	if(pstCheck == NULL)
	{
		log_print( LOGN_WARN, "MMDB NOT EXIST[%u][%u][%u]", 
							pstSrc->stKey.usSrcPort, pstSrc->stKey.usDestPort, pstSrc->stKey.uiSysInfo );
		return -3;
	}

	/* Session Create Time Setting : USING PACKET TIME 
	gettimeofday(&pstCheck->stSessTime, NULL );
	gettimeofday(&pstCheck->stUpdateTime, NULL );
	*/

	/* Transaction Setting */
	pstCheck->uiReqCount 	= 0;
	pstCheck->uiReqFirst 	= 0;
	pstCheck->uiReqLast		= 0;
	pstCheck->uiResCount    = 0;
    pstCheck->uiResFirst    = 0;
    pstCheck->uiResLast     = 0;
	
	/* Current Session Count + 1 */
	pstAssoTbl->uiAssoCount++;

	*ppstDst = pstCheck;

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dSetMMDB(PASSO_KEY pstKey, PASSO_DATA *pstMMDB)
{
	/* MSG DB INSERT */
	*pstMMDB = Search_SESS(pstKey);
	if(*pstMMDB == NULL)
	{
		log_print( LOGN_WARN, "MMDB SEARCH NOT EXIST[%u][%u[%u]", 
							pstKey->usSrcPort, pstKey->usDestPort, pstKey->uiSysInfo );
		return -1;
	}

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dFreeMMDB(PASSO_DATA pstMMDB)
{
	int				i, dRet;
	UINT			uiIndex, uiCount;
	pSTACK_LIST   	pstNode;
	
	/* MMDB KEY VALIDATION */
	if(	((pstMMDB->stKey.usSrcPort == 0) && (pstMMDB->stKey.usDestPort == 0) && (pstMMDB->stKey.uiSysInfo == 0))  || 
		((pstMMDB->stKey.usSrcPort == 0xFFFF) && (pstMMDB->stKey.usDestPort == 0xFFFF) && (pstMMDB->stKey.uiSysInfo == 0xFFFFFFFF)) )
		return -2;
		
	/* MMDB Transaction DELETE Flow */
	uiIndex = pstMMDB->uiReqFirst; 
	uiCount = pstMMDB->uiReqCount; /* Don't Touch pstMMDB->uiCount-- in dDelTran() */
	for(i = 0; i < uiCount; i++) {
		pstNode = &pstStackTbl->stNode[uiIndex];
		uiIndex = pstNode->uiStackNext;
		//dDelTCP(pstMMDB, pstNode);
		dDelStack( pstMMDB, DEF_STACK_REQ, pstNode ); 
	}

	uiIndex = pstMMDB->uiResFirst;
    uiCount = pstMMDB->uiResCount; /* Don't Touch pstMMDB->uiCount-- in dDelTran() */
    for(i = 0; i < uiCount; i++) {
        pstNode = &pstStackTbl->stNode[uiIndex];
        uiIndex = pstNode->uiStackNext;
        //dDelTCP(pstMMDB, pstNode);
		dDelStack( pstMMDB, DEF_STACK_RES, pstNode );
    }

	/* MMDB Session DELETE Flow */
	dRet = Delete_SESS(&pstMMDB->stKey);
	if(dRet <  0)
	{
		log_print( LOGN_WARN, "MMDB_DELETE ERROR[%d][%u][%u][%u]", 
							dRet, pstMMDB->stKey.usSrcPort, pstMMDB->stKey.usDestPort, pstMMDB->stKey.uiSysInfo );
		return -1;
	}
	else if(dRet == 1)
	{
		log_print( LOGN_WARN, "MMDB_DELETE NOT FOUND[%d][%u][%u][%u]", 
							dRet, pstMMDB->stKey.usSrcPort, pstMMDB->stKey.usDestPort, pstMMDB->stKey.uiSysInfo );
		return 1;
	}

	/* Current Session Count - 1 */
	pstAssoTbl->uiAssoCount--;


	return 0;	
}

/*********************************/
/* TCP LINEKD_LIST User Function */
/*********************************
int dGetTCP(PASSO_DATA pstMMDB, pTCP_KEY pstTCPKey, pTCP_LIST *pstOutput)
{
	UINT			uiIndex;
	pTCP_LIST 		pstPrev;
	pTCP_LIST 		pstNode;

	pstNode = pGetNode(&uiIndex);	
	if(pstNode == NULL)
	{
		log_print(LOGN_WARN,"LIST GET NULL");
		return -1;
	}

	pstNode->stPKey = pstMMDB->stKey;
	pstNode->stTKey = *pstTCPKey;
	pstNode->uiTCPNext = pstMMDB->uiTCPFirst;
	pstNode->uiTCPPrev = 0;
	pstNode->uiReqCount = 0;
	pstNode->uiReqFirst = 0;
	pstNode->uiReqLast = 0;
	pstNode->uiResCount = 0;
	pstNode->uiResFirst = 0;
	pstNode->uiResLast = 0;

	pstNode->ucStatus = 0;
	pstNode->usSessStatus = 0;

	pstNode->uiNoSndReqIdx = 0;
	pstNode->uiNoSndResIdx = 0;
	pstNode->uiNoSndCount = 0;

	memset(&pstNode->stTCPSessInfo, 0x00, sizeof(st_TCPSessInfo));
	memset(&pstNode->stTCPSessLog, 0x00, sizeof(st_Tcp_Sess_Log));

	if(pstMMDB->uiTCPCount > 0)
		pstTCPTbl->stNode[pstMMDB->uiTCPFirst].uiTCPPrev = uiIndex;
	else 
		pstMMDB->uiTCPLast = uiIndex;

	pstMMDB->uiTCPFirst = uiIndex;
	pstMMDB->uiTCPCount++;
	
	*pstOutput = pstNode;

	return 0;
}

int dSetTCPForward(PASSO_DATA pstMMDB, pTCP_KEY pstTCPKey, pTCP_LIST *pstTCP)
{
	int				i;
	UINT			uiIndex;
	pTCP_LIST	pstNode;

	if(pstMMDB->uiTCPCount < 1)
	{
		log_print(LOGN_WARN,"MMDBFORWARD::LIST PARAM ERROR TNC[%u] TRNF[%u]", 
			pstMMDB->uiTCPCount, pstMMDB->uiTCPFirst);
		return -1;
	}

	uiIndex = pstMMDB->uiTCPFirst;	
	for(i = 0; i < pstMMDB->uiTCPCount; i++)
	{
		pstNode = &pstTCPTbl->stNode[uiIndex];
		if(!memcmp(pstTCPKey, &pstNode->stTKey, TCP_KEY_SIZE))
		{
			*pstTCP = pstNode;
			return 0;
		}

		uiIndex = pstNode->uiTCPNext;	
	}

	return -100;
}

int dSetTCPBackword(PASSO_DATA pstMMDB, pTCP_KEY pstTCPKey, pTCP_LIST *pstTCP)
{
	int				i;
	UINT			uiIndex;
	pTCP_LIST	pstNode;

	if(pstMMDB->uiTCPCount < 1)
	{
		log_print(LOGN_WARN,"MMDBBACKWORD::LIST PARAM ERROR TNC[%u] TRNF[%u]", 
			pstMMDB->uiTCPCount, pstMMDB->uiTCPFirst);
		return -1;
	}

	uiIndex = pstMMDB->uiTCPLast;	
	for(i = pstMMDB->uiTCPCount; i > 0; i--)
	{
		pstNode = &pstTCPTbl->stNode[uiIndex];
		if(!memcmp(pstTCPKey, &pstNode->stTKey, TCP_KEY_SIZE))
		{
			*pstTCP = pstNode;
			return 0;
		}

		uiIndex = pstNode->uiTCPPrev;	
	}

	return -100;
}

int dSetTCPIndex(PASSO_DATA pstMMDB, UINT uiIndex, pTCP_LIST *pstTCP)
{
	if(uiIndex >= MAX_TCP_LIST || uiIndex == 0)
		return -1;

	*pstTCP = &pstTCPTbl->stNode[uiIndex];

	return 0;
}

int dDelTCP(PASSO_DATA pstMMDB, pTCP_LIST pstNode)
{
	int				i;
	UINT 			uiIndex, uiCount;
	pSTACK_LIST		pstStack;

	if(pstNode->uiIndex >= MAX_TCP_LIST || pstNode->uiIndex == 0)
	{
		log_print(LOGN_WARN, 
			"TCP Linked List Index Corrupted Node Index=%u Prev=%u Next=%u",
			pstNode->uiIndex, pstNode->uiTCPPrev, pstNode->uiTCPNext); 
		return -1;
	}

	if(memcmp(&pstMMDB->stKey, &pstNode->stPKey, ASSO_KEY_SIZE))
	{
		log_print(LOGN_WARN, 
			"DELETE TCP Linked List Parents Key Not Match Index=%u Prev=%u Next=%u",
			pstNode->uiIndex, pstNode->uiTCPPrev, pstNode->uiTCPNext); 
		return -2;
	}

	uiIndex = pstNode->uiReqFirst;
    uiCount = pstNode->uiReqCount; * Don't Touch pstNode->uiReqCount-- in dDelStack() *
    for(i = 0; i < uiCount; i++) {
        pstStack = &pstStackTbl->stNode[uiIndex];
        uiIndex = pstStack->uiStackNext;
        dDelStack(pstNode, DEF_STACK_REQ, pstStack);
    }

	* Delete Response Stack List *
	uiIndex = pstNode->uiResFirst;
    uiCount = pstNode->uiResCount; * Don't Touch pstNode->uiResCount-- in dDelStack() *
    for(i = 0; i < uiCount; i++) {
        pstStack = &pstStackTbl->stNode[uiIndex];
        uiIndex = pstStack->uiStackNext;
        dDelStack(pstNode, DEF_STACK_RES, pstStack);
    }
		
	* Rebuild TCP Linked List *
	if(pstNode->uiTCPPrev > 0) {
		if(pstTCPTbl->stNode[pstNode->uiTCPPrev].uiTCPNext == pstNode->uiIndex)
			pstTCPTbl->stNode[pstNode->uiTCPPrev].uiTCPNext = pstNode->uiTCPNext;
		else
			log_print(LOGN_WARN, 
				"TCP Linked List Corrupted Node Index=%u Prev=%u Next=%u PrevNext=%u",
				pstNode->uiIndex, pstNode->uiTCPPrev, pstNode->uiTCPNext, 
				pstTCPTbl->stNode[pstNode->uiTCPPrev].uiTCPNext);
	} else if(pstMMDB->uiTCPFirst == pstNode->uiIndex) {
		pstMMDB->uiTCPFirst = pstNode->uiTCPNext;
	} else {
		log_print(LOGN_WARN, 
			"TCP Linked List Corrupted Node Prev Index=%u Prev=%u Next=%u TrnFirst=%u",
			pstNode->uiIndex, pstNode->uiTCPPrev, pstNode->uiTCPNext, 
			pstMMDB->uiTCPFirst);
	}

	if(pstNode->uiTCPNext > 0) {
		if(pstTCPTbl->stNode[pstNode->uiTCPNext].uiTCPPrev == pstNode->uiIndex)
			pstTCPTbl->stNode[pstNode->uiTCPNext].uiTCPPrev = pstNode->uiTCPPrev;
		else
			log_print(LOGN_WARN, 
				"TCP Linked List Corrupted Node Index=%u Prev=%u Next=%u NextPrev=%u",
				pstNode->uiIndex, pstNode->uiTCPPrev, pstNode->uiTCPNext, 
				pstTCPTbl->stNode[pstNode->uiTCPNext].uiTCPPrev);
	} else if(pstMMDB->uiTCPLast == pstNode->uiIndex) {
		pstMMDB->uiTCPLast = pstNode->uiTCPPrev;
	} else {
		log_print(LOGN_WARN, 
			"TCP Linked List Corrupted Node Next Index=%u Prev=%u Next=%u TrnLast=%u",
			pstNode->uiIndex, pstNode->uiTCPPrev, pstNode->uiTCPNext, 
			pstMMDB->uiTCPLast);
	}
		
	FreeNode(pstNode);

	pstMMDB->uiTCPCount--;

	return 0;
}
*/

/*******************************************************************************
* TCP STACK LINEKD_LIST User Function *
*******************************************************************************/
int dGetStackOnly(pSTACK_LIST *pstStack)
{
	UINT			uiIndex;
	pSTACK_LIST		pstNode;
	
	pstNode = pGetStackNode(&uiIndex);	
	if(pstNode == NULL)
	{
		log_print(LOGN_WARN,"STACKONLY::LIST GET NULL");
		return -1;
	}

	*pstStack = pstNode;

	return 0;
}


/*******************************************************************************

*******************************************************************************/
void FreeStackOnly(pSTACK_LIST pstStack)
{
	FreeStackNode(pstStack);
}


/*******************************************************************************
 Key 정보를 이용하여 하나의 Stack Node를 추가.
*******************************************************************************/
int dGetStack(PASSO_DATA pstMMDB, UCHAR ucType, pSTACK_KEY pstSKey, pSTACK_LIST *pstStack)
{
	UINT		uiIndex;
	pSTACK_LIST pstNode;

	pstNode = pGetStackNode(&uiIndex);	
	if(pstNode == NULL)
	{
		log_print(LOGN_WARN,"GETSTACK::LIST GET NULL");
		return -1;
	}

	/* Transaction Memory Setting */
	pstNode->stTKey = pstMMDB->stKey;
	pstNode->stSKey = *pstSKey;
	pstNode->uiStackNext = 0;
	if(ucType == DEF_STACK_REQ)
	{
		pstNode->uiStackPrev = pstMMDB->uiReqLast;
		/* Request Stack Linked List Add FIFO of TCP SESSION */
		if(pstMMDB->uiReqCount > 0)
			pstStackTbl->stNode[pstMMDB->uiReqLast].uiStackNext = uiIndex;
		else 
			pstMMDB->uiReqFirst = uiIndex;

		pstMMDB->uiReqLast = uiIndex;
		pstMMDB->uiReqCount++;
	
	}
	else if(ucType == DEF_STACK_RES)
	{
		pstNode->uiStackPrev = pstMMDB->uiResLast;
		/* Response Stack Linked List Add FIFO of TCP SESSION */
		if(pstMMDB->uiResCount > 0)
			pstStackTbl->stNode[pstMMDB->uiResLast].uiStackNext = uiIndex;
		else  
			pstMMDB->uiResFirst = uiIndex;

		pstMMDB->uiResLast = uiIndex;
		pstMMDB->uiResCount++;
	}
	else
	{
		log_print(LOGN_CRI, "GETSTACK::PARAM ERROR NO SUPPORTED TYPE=[%d]", ucType);
		return -2;
	}

	*pstStack = pstNode;

	return 0;
}


/*******************************************************************************

*******************************************************************************
int dAddStack(PASSO_DATA pstMMDB, UCHAR ucType, pSTACK_LIST pstAddStack)
{
	if(pstAddStack->uiIndex >= MAX_STACK_LIST || pstAddStack->uiIndex == 0)
	{
		log_print(LOGN_CRI, "ADDSTACK::Linked List Error Index=%u", pstAddStack->uiIndex);
		return -1;
	}

	if(ucType == DEF_STACK_REQ && pstMMDB->uiReqCount == 0)
	{
		pstMMDB->uiReqFirst = pstAddStack->uiIndex;
		pstMMDB->uiReqLast = pstAddStack->uiIndex;
		pstMMDB->uiReqCount++;
	}
	else if(ucType == DEF_STACK_RES && pstMMDB->uiResCount == 0)
	{
		pstMMDB->uiResFirst = pstAddStack->uiIndex;
		pstMMDB->uiResLast = pstAddStack->uiIndex;
		pstMMDB->uiResCount++;
	}
	else
	{
		log_print(LOGN_WARN, 
			"ADDSTACK::THIS FUNCTION ONLY COUNT=0:TYPE[%d]REQC[%u]RESC[%u]",
			ucType, pstMMDB->uiReqCount, pstMMDB->uiResCount);
		return -2;
	}

	return 0;
}
*/


/*******************************************************************************

*******************************************************************************/
int dAddStackNext(PASSO_DATA pstMMDB, UCHAR ucType, pSTACK_LIST pstStack, pSTACK_LIST pstAddStack)
{
	if(pstAddStack->uiIndex >= MAX_STACK_LIST || pstAddStack->uiIndex == 0)
	{
		log_print(LOGN_CRI, "ADDSTACKNEXT::Linked List Error Index=%u", pstAddStack->uiIndex);
		return -1;
	}

	if(ucType == DEF_STACK_REQ)
		pstMMDB->uiReqCount++;
	else if(ucType == DEF_STACK_RES)
		pstMMDB->uiResCount++;
	else
	{
		log_print(LOGN_CRI, 
			"ADDSTACKNEXT::INPUT PARAM ERROR TYPE[%d] INDEX[%u]", ucType, pstAddStack->uiIndex);
		return -3;
	}

	pstAddStack->uiStackPrev = pstStack->uiIndex;
	pstAddStack->uiStackNext = pstStack->uiStackNext;

	if(pstStack->uiStackNext > 0) {
		if(pstStackTbl->stNode[pstStack->uiStackNext].uiStackPrev == pstStack->uiIndex)
			pstStackTbl->stNode[pstStack->uiStackNext].uiStackPrev = pstAddStack->uiIndex;
		else
			log_print(LOGN_CRI, "STACKNextLinked List Corrupted Index=%u PREV=%u Next=%u",
				pstStack->uiIndex, pstStack->uiStackPrev, pstStack->uiStackNext);
	} else if(pstStack->uiIndex == pstMMDB->uiReqLast && ucType == DEF_STACK_REQ) {
		pstMMDB->uiReqLast = pstAddStack->uiIndex;
	} else if(pstStack->uiIndex == pstMMDB->uiResLast && ucType == DEF_STACK_RES) {
		pstMMDB->uiResLast = pstAddStack->uiIndex;
	} else {
		log_print(LOGN_CRI, 
			"STACKNextLinked List Corrupted TYPE[%d] INDEX=%u PREV=%u Next=%u",
			ucType, pstStack->uiIndex, pstStack->uiStackPrev, pstStack->uiStackNext);
	}

	pstStack->uiStackNext = pstAddStack->uiIndex;

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dAddStackPrev(PASSO_DATA pstMMDB, UCHAR ucType, pSTACK_LIST pstStack, pSTACK_LIST pstAddStack)
{
	if(pstAddStack->uiIndex >= MAX_STACK_LIST || pstAddStack->uiIndex == 0)
	{
		log_print(LOGN_CRI, "ADDSTACKPREV::Linked List Error Index=%u", pstAddStack->uiIndex); 
		return -1;
	}

	if(ucType == DEF_STACK_REQ)
		pstMMDB->uiReqCount++;
	else if(ucType == DEF_STACK_RES)
		pstMMDB->uiResCount++;
	else
	{
		log_print(LOGN_CRI, 
			"ADDSTACKPREV::INPUT PARAM ERROR TYPE[%d] INDEX[%u]", ucType, pstAddStack->uiIndex);
		return -3;
	}

	pstAddStack->uiStackNext = pstStack->uiIndex;
	pstAddStack->uiStackPrev = pstStack->uiStackPrev;

	if(pstStack->uiStackPrev > 0) {
		if(pstStackTbl->stNode[pstStack->uiStackPrev].uiStackNext == pstStack->uiIndex)
			pstStackTbl->stNode[pstStack->uiStackPrev].uiStackNext = pstAddStack->uiIndex;
		else
			log_print(LOGN_CRI, "STACKPrevLinked List Corrupted Index=%u PREV=%u Next=%u",
				pstStack->uiIndex, pstStack->uiStackPrev, pstStack->uiStackNext);
	} else if(pstStack->uiIndex == pstMMDB->uiReqFirst && ucType == DEF_STACK_REQ) {
		pstMMDB->uiReqFirst = pstAddStack->uiIndex;
	} else if(pstStack->uiIndex == pstMMDB->uiResFirst && ucType == DEF_STACK_RES) {
		pstMMDB->uiResFirst = pstAddStack->uiIndex;
	} else {
		log_print(LOGN_CRI, 
			"STACKFirstLinked List Corrupted TYPE[%d] INDEX=%u PREV=%u Next=%u",
			ucType, pstStack->uiIndex, pstStack->uiStackPrev, pstStack->uiStackNext);
	}

	pstStack->uiStackPrev = pstAddStack->uiIndex;

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dSetStackNext(UINT uiIndex, pSTACK_LIST *pstNextStack)
{
	UINT uiNext;

	if(uiIndex >= MAX_STACK_LIST || uiIndex == 0)
	{
		log_print(LOGN_CRI, "SETSTACKNEXT::Linked List Error Index=%u", uiIndex); 
		return -1;
	}

	if((uiNext = pstStackTbl->stNode[uiIndex].uiStackNext) < 1) {
		log_print(LOGN_WARN, "SETSTACKNEXT::NO DATA::INDEX=%u Next=%u", uiIndex, uiNext);
		return -100;
	} else
		*pstNextStack = &pstStackTbl->stNode[uiNext];

	return 0;

}


/*******************************************************************************

*******************************************************************************/
int dSetStackPrev(UINT uiIndex, pSTACK_LIST *pstPrevStack)
{
	UINT uiPrev;

	if(uiIndex >= MAX_STACK_LIST || uiIndex == 0)
	{
		log_print(LOGN_CRI, "SETSTACKPREV::Linked List Error Index=%u", uiIndex); 
		return -1;
	}

	if((uiPrev = pstStackTbl->stNode[uiIndex].uiStackPrev) < 1) {
		log_print(LOGN_WARN, "SETSTACKPREV::NO DATA::INDEX=%u Prev=%u", uiIndex, uiPrev);
		return -100;
	} else
		*pstPrevStack = &pstStackTbl->stNode[uiPrev];

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dSetStackForward(PASSO_DATA pstMMDB, UCHAR ucType, pSTACK_KEY pstSKey, pSTACK_LIST *pstStack)
{
	int				i;
	UINT			uiIndex, uiCount = 0;
	pSTACK_LIST		pstNode;

	if(ucType == DEF_STACK_REQ) {
		uiIndex = pstMMDB->uiReqFirst;
		uiCount = pstMMDB->uiReqCount;
	} else if(ucType == DEF_STACK_RES) {
		uiIndex = pstMMDB->uiResFirst;
		uiCount = pstMMDB->uiResCount;
	} 
	
	if(uiCount < 1) {
		log_print(LOGN_WARN,"SET STACK FORWARD LIST PARAM ERROR TYPE[%d] RQC[%u]RSC[%u]", 
			ucType, pstMMDB->uiReqCount, pstMMDB->uiResCount);
		return -1;
	}

	for(i = 0; i < uiCount; i++)
	{
		pstNode = &pstStackTbl->stNode[uiIndex];
		if(!memcmp(pstSKey, &pstNode->stSKey, STACK_KEY_SIZE))
		{
			*pstStack = pstNode;
			return 0;
		}

		uiIndex = pstNode->uiStackNext;	
	}

	return -100;
}


/*******************************************************************************

*******************************************************************************/
int dSetStackBackword(PASSO_DATA pstMMDB, UCHAR ucType,  pSTACK_KEY pstSKey, pSTACK_LIST *pstStack)
{
	int				i;
	UINT			uiIndex, uiCount = 0;
	pSTACK_LIST		pstNode;

	if(ucType == DEF_STACK_REQ) {
		uiIndex = pstMMDB->uiReqLast;
		uiCount = pstMMDB->uiReqCount;
	} else if(ucType == DEF_STACK_RES) {
		uiIndex = pstMMDB->uiResLast;
		uiCount = pstMMDB->uiResCount;
	} 
	
	if(uiCount < 1) {
		log_print(LOGN_WARN,"SET STACK FORWARD LIST PARAM ERROR TYPE[%d] RQC[%u]RSC[%u]", 
			ucType, pstMMDB->uiReqCount, pstMMDB->uiResCount);
		return -1;
	}

	for(i = uiCount; i < 1; i--)
	{
		pstNode = &pstStackTbl->stNode[uiIndex];
		if(!memcmp(pstSKey, &pstNode->stSKey, STACK_KEY_SIZE))
		{
			*pstStack = pstNode;
			return 0;
		}

		uiIndex = pstNode->uiStackPrev;	
	}

	return -100;
}


/*******************************************************************************

*******************************************************************************/
int dSetStackIndex(UINT uiIndex, pSTACK_LIST *pstStack)
{
	if(uiIndex >= MAX_STACK_LIST || uiIndex == 0)
		return -1;

	*pstStack = &pstStackTbl->stNode[uiIndex];

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dDelStack(PASSO_DATA pstMMDB, UCHAR ucType, pSTACK_LIST pstStack)
{
	if(pstStack->uiIndex >= MAX_STACK_LIST || pstStack->uiIndex ==0)
	{
		log_print(LOGN_WARN, 
			"STACKDEL Linked List Parents Key Corrupted Node Index=%u Prev=%u Next=%u",
			pstStack->uiIndex, pstStack->uiStackPrev, pstStack->uiStackNext); 
		return -1;
	}

	if(ucType == DEF_STACK_REQ)
		pstMMDB->uiReqCount--;
	else if(ucType == DEF_STACK_RES)
		pstMMDB->uiResCount--;
	else
	{
		log_print(LOGN_CRI, 
			"DELSTACK::INPUT PARAM ERROR TYPE[%d] INDEX[%u]", ucType, pstStack->uiIndex);
		return -2;
	}

	log_print(LOGN_INFO, 
			"STACKDEL Linked List Corrupted Node Index=%u Prev=%u Next=%u",
			pstStack->uiIndex, pstStack->uiStackPrev, pstStack->uiStackNext);

	if(pstStack->uiStackNext > 0) {
		if(pstStackTbl->stNode[pstStack->uiStackNext].uiStackPrev == pstStack->uiIndex)
			pstStackTbl->stNode[pstStack->uiStackNext].uiStackPrev = pstStack->uiStackPrev;
		else
			log_print(LOGN_INFO, 
					"STACKDEL Linked List Corrupted Node Index=%u Prev=%u Next=%u NextPrev=%u",
					pstStack->uiIndex, pstStack->uiStackPrev, pstStack->uiStackNext, 
					pstStackTbl->stNode[pstStack->uiStackNext].uiStackPrev);
	} else if(ucType == DEF_STACK_REQ && pstMMDB->uiReqLast == pstStack->uiIndex) {
		pstMMDB->uiReqLast = pstStack->uiStackPrev;
	} else if(ucType == DEF_STACK_RES && pstMMDB->uiResLast == pstStack->uiIndex) {
		pstMMDB->uiResLast = pstStack->uiStackPrev;
	} else {
		log_print( LOGN_INFO, 
				"STACKDEL Linked List Corrupted Node Next Index=%u Prev=%u Next=%u Req=%u Res=%u",
				pstStack->uiIndex, pstStack->uiStackPrev, pstStack->uiStackNext, 
				pstMMDB->uiReqLast, pstMMDB->uiResLast);
	}
		
	if(pstStack->uiStackPrev > 0) {
		if(pstStackTbl->stNode[pstStack->uiStackPrev].uiStackNext == pstStack->uiIndex)
			pstStackTbl->stNode[pstStack->uiStackPrev].uiStackNext = pstStack->uiStackNext;
		else
			log_print( LOGN_INFO, 
					"STACKDEL Linked List Corrupted Node Index=%u Prev=%u Next=%u PrevNext=%u",
					pstStack->uiIndex, pstStack->uiStackPrev, pstStack->uiStackNext, 
					pstStackTbl->stNode[pstStack->uiStackPrev].uiStackNext);
	} else if(ucType == DEF_STACK_REQ && pstMMDB->uiReqFirst == pstStack->uiIndex) {
		pstMMDB->uiReqFirst = pstStack->uiStackNext;
	} else if(ucType == DEF_STACK_RES && pstMMDB->uiResFirst == pstStack->uiIndex) {
		pstMMDB->uiResFirst = pstStack->uiStackNext;
	} else {
		log_print(LOGN_INFO, 
			"STACKDEL Linked List Corrupted Node Prev Index=%u Prev=%u Next=%u req=%u res=%u",
			pstStack->uiIndex, pstStack->uiStackPrev, pstStack->uiStackNext, 
			pstMMDB->uiReqFirst, pstMMDB->uiResFirst);
	}

	FreeStackNode(pstStack);

	return 0;
}


/*******************************************************************************

********************************************************************************
int dTimerTCPLastProc(time_t stCurrTime, UINT uiTimeOut, UINT uiCheck)
{
	int					dRet;
    UINT                uiIndex, uiCount = 0;
    pTCP_LIST       pstNode;
	PASSO_DATA 			pstMMDB;

    uiIndex = pstTCPTbl->uiUsedLast;
    while (uiIndex > 0 && uiCheck > uiCount)
    {
        if(stCurrTime > uiTimeOut + pstTCPTbl->stNode[uiIndex].stTCPTime.tv_sec)
        {
            pstNode = &pstTCPTbl->stNode[uiIndex];
            uiIndex = pstNode->prev;

    		pstMMDB = Search_SESS(&pstNode->stPKey);
    		if(pstMMDB == NULL)
    		{
        		log_print(LOGN_WARN," Timer Last MMDB SEARCH NOT EXIST[%u]",
            		pstNode->stPKey.uiSrcIP);
				FreeNode(pstNode);
        		return -1;
			}



			dRet = dDelTCP(pstMMDB, pstNode);
			if(dRet < 0)
			{
        		log_print(LOGN_WARN," Timer Transaction Error Node Index[%u]",
            		pstNode->uiIndex);
        		return -2;
			}

			if(pstMMDB->uiTCPCount == 0)
				;
        }
        else
            break;

		uiCount++;
    }

	if(uiCheck == uiCount)
		return 100;

	return 0;
}

int dTimerTCPFullProc(time_t stCurrTime, UINT uiTimeOut, UINT uiCheck)
{
	int					dRet;
    UINT                uiIndex, uiCount = 0;
    pTCP_LIST       pstNode;
    PASSO_DATA  		pstMMDB;

    uiIndex = pstTCPTbl->uiUsedFirst;
    while (uiIndex > 0 && uiCheck > uiCount)
    {
        if(stCurrTime > uiTimeOut + pstTCPTbl->stNode[uiIndex].stTCPTime.tv_sec)
        {
            pstNode = &pstTCPTbl->stNode[uiIndex];
            uiIndex = pstNode->next;

    		pstMMDB = Search_SESS(&pstNode->stPKey);
    		if(pstMMDB == NULL)
    		{
        		log_print(LOGN_WARN," Timer MMDB SEARCH NOT EXIST[%u]", pstNode->stPKey.uiSrcIP);
				FreeNode(pstNode);
        		return -1;
			}



			dRet = dDelTCP(pstMMDB, pstNode);
			if(dRet < 0)
			{
        		log_print(LOGN_WARN," Timer Full Transaction Error Node Index[%u]",
            		pstNode->uiIndex);
        		return -2;
			}

			if(pstMMDB->uiTCPCount == 0)
				;

        }
        else
            uiIndex = pstTCPTbl->stNode[uiIndex].next;

		uiCount++;
    }

	if(uiCheck == uiCount)
		return 100;

	return 0;
}

int dTimerASSOProc(
		time_t stTime, PASSO_KEY pstFKey, PASSO_KEY pstLKey, UINT uiCheck, UINT uiTimeOut)
{
	int			dRet, i;
	UINT		uiCount = 0;
    PASSO_DATA  pstMMDB;

	pTCP_LIST   pstTCPNode;
	UINT        uiTCPIdx;
	INT64		llNID;

	pstMMDB = pstSelectMMDB(pstFKey, pstLKey);
    while(pstMMDB != NULL && uiCount < uiCheck)
    {
        pstFKey->uiSrcIP = pstMMDB->stKey.uiSrcIP;

        if(stTime > (uiTimeOut + pstMMDB->tLastUpdateTime)) {
			if(pstMMDB->uiTCPCount > 0) {

				uiTCPIdx = pstMMDB->uiTCPFirst;
				for(i=0; i<pstMMDB->uiTCPCount; i++)
				{
					dRet = dSetTCPIndex(pstMMDB, uiTCPIdx, &pstTCPNode);
					if(dRet < 0) {
						log_print(LOGN_CRI, "[TIMERASSOPROC] [SET_TCP_ERROR]");
						break;
					}

					dMakeNID(SEQ_PROC_A_TCP, &llNID);
					dRet = dSndSvcBlkStopMsgNoRDR(pstMMDB, pstTCPNode->uiSndMsgQ, llNID);
				}

			}

			dRet = dFreeMMDB(pstMMDB);
			if(dRet < 0) {
				log_print(LOGN_CRI, "[TIMERASSOPROC] [ASSO SESSION DELETE FAIL][RET]:[%d]", dRet);
				return 0;
			} else if(dRet == 1) {
				log_print(LOGN_DEBUG, "[TIMERASSOPROC] [NOT FOUND ASSO SESSION]");
				return 0;
			}

        }

		pstMMDB = pstSelectMMDB(pstFKey, pstLKey);
        uiCount++;
    }

    if(uiCount == uiCheck)
        return 100;

    return 0;
}
*/

/*
* $Log: lib_api.c,v $
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
* Revision 1.5  2007/06/01 15:46:56  doit1972
* MODIFY LOG
*
* Revision 1.4  2007/05/04 12:36:05  doit1972
* MODIFY INIT MMDB INFO
*
* Revision 1.3  2007/05/03 11:45:56  doit1972
* 로그 정보 추가
*
* Revision 1.2  2007/04/29 13:09:34  doit1972
* CVS LOG 정보 추가
*
*/
