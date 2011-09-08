#ifndef _SIPT_SESS_H_
#define _SIPT_SESS_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"

// .
#include "sipt_func.h"

/**
 *	Define cons.
 */
#define MAX_SESSKEY_LIST	 	20002

/**
 *	Define structures
 */
typedef struct _st_SESSKEY_LIST {
	SIP_INFO_KEY 	stSessKey;
	UINT 			next;
	UINT 			prev;
	UINT 			uiNext;
	UINT 			uiPrev;
	UINT 			uiIndex;
	UINT 			SessStartTime;
} SESSKEY_LIST, *pSESSKEY_LIST;
#define SESSKEY_LIST_SIZE  	sizeof(SESSKEY_LIST)

typedef struct _st_SESSKEY_TABLE {
    UINT            uiFreeList;     /* Free Node List */
    UINT            uiUsedFirst;    /* Used Fist Node Index */
    UINT            uiUsedLast;     /* Used Last Node Index */
    UINT            uiCurrCount;    /* Current Used Node Count */
    CHAR            szReserved[128]; 
    SESSKEY_LIST 	stNode[MAX_SESSKEY_LIST];
} SESSKEY_TBL, *pSESSKEY_TBL;
#define SESSKEY_TBL_SIZE  	sizeof(SESSKEY_TBL)

/**
 * Declare functions
 */
extern void InitSIPTKeyList();
extern pSESSKEY_LIST pGetSessKeyList(UINT *puiIndex);
extern int dGetSessKeyList(pSESSKEY_LIST *pstStack);
extern int dAddSessKeyNext(pSESSKEY_LIST pstHashStack, pSESSKEY_LIST pstAddStack);
extern void FreeSessKeyList(pSESSKEY_LIST node);
extern int dDelSessKeyList(pSESSKEY_LIST pstDelStack, stHASHOINFO *pSIPHASH, stMEMSINFO *pMEMSINFO, UINT uiLastLogTime);
extern int dDelSessKeyNext(pSESSKEY_LIST pstDelStack, SIP_INFO_KEY *pSIPINFOKEY);

#endif	/* _SIPT_SESS_H_ */
