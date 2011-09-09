#ifndef _CLIST_MEMG_H_
#define _CLIST_MEMG_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "clisto.h"
#include "memg.h"


/**
 *	Declare functions
 */
#define cmem_head_init(infoptr, ptr)					CINIT_LIST_HEAD(infoptr, ptr)
#define cmem_for_each_start(infoptr, pos, head)			clist_for_each_start(infoptr, pos, head)
#define cmem_for_each_end(infoptr, pos, head)			clist_for_each_end(infoptr, pos, head)
#define cmem_entry(ptr, type, member)					clist_entry(ptr, type, member)
#define cmem_offset(infoptr, ptr)						clisto_offset(infoptr, ptr)
#define cmem_ptr(infoptr, offset)						(U8 *)clisto_ptr(infoptr, offset)

extern stMEMGINFO *cmem_init(S32 uiShmKey, U32 uiBodySize, U32 uiCnt);
extern U8 *cmem_alloc(stMEMGINFO *pMEMGINFO);
extern void cmem_link_prev(stMEMGINFO *pMEMGINFO, U8 *pHead, U8 *pNew);
extern void cmem_link_next(stMEMGINFO *pMEMGINFO, U8 *pHead, U8 *pNew);
extern void cmem_unlink(stMEMGINFO *pMEMGINFO, U8 *pDel);
extern void cmem_free(stMEMGINFO *pMEMGINFO, U8 *pFree);
extern void cmem_delete(stMEMGINFO *pMEMGINFO, U8 *pDel);

#endif	/* _CLIST_MEMG_H_ */
