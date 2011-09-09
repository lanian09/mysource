/*******************************************************************************
			DQMS Project

	Author   :
	Section  : SI_SVCMON
	SCCS ID  : @(#)clist_memg.h	1.1
	Date     : 01/21/05
	Revision History :

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __CLIST_MEMG_H__
#define __CLIST_MEMG_H__

#include "memg.h"

extern void cmem_unlink(stMEMGINFO *pMEMGINFO, U8 *pDel);
extern void cmem_delete(stMEMGINFO *pMEMGINFO, U8 *pDel);
extern stMEMGINFO *cmem_init(S32 uiShmKey, U32 uiBodySize, U32 uiCnt);
extern U8 *cmem_alloc(stMEMGINFO *pMEMGINFO);
extern void cmem_link_prev(stMEMGINFO *pMEMGINFO, U8 *pHead, U8 *pNew);

#endif	/*	__SI_SVCMON_INIT_H__	*/

