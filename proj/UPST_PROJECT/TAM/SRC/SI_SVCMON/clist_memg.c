/**		@file	inet_msgq.c
 *		- Internet 사용량을 계산 하는 프로세스
 *
 *		Copyright (c) 2006~ by Upresto Inc. Korea
 *		All rights reserved
 *
 *		$Id: clist_memg.c,v 1.1 2011/08/31 05:42:31 dcham Exp $
 *
 *		@Author		$Author: dcham $
 *		@version	$Revision: 1.1 $
 *		@date		$Date: 2011/08/31 05:42:31 $
 *		@warning	Nothing
 *		@ref		inet_main.c
 *		@todo		Nothing
 *
 *		@section	Intro(소개)
 *		- Internet 사용량을 계산 하는 프로세스
 *
 *		@section	Requirement
 *			@li library 생성 이후 함수 대치
 *
 **/

#include "nsocklib.h"
#include "clist_memg.h"

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			inet_main.c
 *
 *  @exception		Nothing
 *  @note			Nothing
 **/
stMEMGINFO *cmem_init(S32 uiShmKey, U32 uiBodySize, U32 uiCnt)
{
	int		type;
	if(uiShmKey == 0) {
		type = MEMG_MAIN_MEM;
	} else {
		type = MEMG_SHARED_MEM;
	}	
	//return memg_init((uiShmKey == 0) ? MEMG_MAIN_MEM : MEMG_SHARED_MEM, uiShmKey, 0, uiBodySize, uiCnt);
	return memg_init(type, uiShmKey, 0, uiBodySize, uiCnt);
}

U8 *cmem_alloc(stMEMGINFO *pMEMGINFO)
{
	U8		*p;

	if((p = memg_alloc(pMEMGINFO, pMEMGINFO->uiMemNodeBodySize, NULL)) != NULL) {
		cmem_head_init(pMEMGINFO, &(((st_sidb_node *)p)->list));
	}
	
	return p;
}

void cmem_link_prev(stMEMGINFO *pMEMGINFO, U8 *pHead, U8 *pNew)
{
	clist_add_tail(pMEMGINFO, &(((st_sidb_node *)pNew)->list), &(((st_sidb_node *)pHead)->list));
}

void cmem_link_next(stMEMGINFO *pMEMGINFO, U8 *pHead, U8 *pNew)
{
	clist_add_head(pMEMGINFO, &(((st_sidb_node *)pNew)->list), &(((st_sidb_node *)pHead)->list));
}

void cmem_unlink(stMEMGINFO *pMEMGINFO, U8 *pDel)
{
	clist_del_init(pMEMGINFO, &(((st_sidb_node *)pDel)->list));
}

void cmem_free(stMEMGINFO *pMEMGINFO, U8 *pFree)
{
	memg_free(pMEMGINFO, pFree);
}

void cmem_delete(stMEMGINFO *pMEMGINFO, U8 *pDel)
{
	U8 *pNode;
	U8 *pNext;

	pNode = pDel;

	do {
		pNext = (U8 *)cmem_entry(cmem_ptr(pMEMGINFO, ((st_sidb_node *)pNode)->list.offset_next), st_sidb_node, list);

		cmem_unlink(pMEMGINFO, pNode);

		if(pNext == pNode)
			pNext = NULL;

		cmem_free(pMEMGINFO, pNode);
	} while((pNode = pNext) != NULL);
}

/*
 *  $Log: clist_memg.c,v $
 *  Revision 1.1  2011/08/31 05:42:31  dcham
 *  SI_SVCMON added
 *
 *  Revision 1.2  2011/01/11 04:09:18  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:11  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1  2009/06/26 11:31:12  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2009/06/20 12:45:27  dark264sh
 *  *** empty log message ***
 *
 *
 */
