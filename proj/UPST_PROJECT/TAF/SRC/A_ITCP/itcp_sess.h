#ifndef _ITCP_SESS_H_
#define _ITCP_SESS_H_
/**
 *	Include headers
 */
// LIB
#include "typedef.h"

/**
 * Define constants
 */
#define DEF_NORETRANS_OVER          1
#define DEF_NORETRANS_UNDER         2
#define DEF_RETRANS_OVER            3
#define DEF_RETRANS_UNDER           4
#define DEF_RETRANS_SUBSET          5
#define DEF_RETRANS_RSUBSET         6

#define MAX_SESSKEY_LIST			100002

/**
 * Define structures
 */
typedef struct _st_Tcp_Common {
	TCP_SESS_KEY	TCPSESSKEY;
} TCP_COMMON;


typedef struct _st_SESSKEY_LIST {
	TCP_SESS_KEY 	stSessKey;
	UINT 			next;
	UINT 			prev;
	UINT 			uiTCPNext;
	UINT 			uiTCPPrev;
	UINT 			uiIndex;
	UINT 			SessStartTime;
} SESSKEY_LIST, *pSESSKEY_LIST;

typedef struct _st_SESSKEY_TABLE {
    UINT            uiFreeList;     /* Free Node List */
    UINT            uiUsedFirst;    /* Used Fist Node Index */
    UINT            uiUsedLast;     /* Used Last Node Index */
    UINT            uiCurrCount;    /* Current Used Node Count */
    CHAR            szReserved[128]; 
    SESSKEY_LIST 	stNode[MAX_SESSKEY_LIST];
} SESSKEY_TBL, *pSESSKEY_TBL;
#define SESSKEY_LIST_SIZE  	sizeof(SESSKEY_LIST)
#define SESSKEY_TBL_SIZE  	sizeof(SESSKEY_TBL)


/**
 * Declare functions
 */
void InitTCPKeyList();
int dGetSessKeyList(pSESSKEY_LIST *pstStack);
int dAddSessKeyNext(pSESSKEY_LIST pstStack, pSESSKEY_LIST pstAddStack);
int dDelSessKeyList(pSESSKEY_LIST pstDelStack, stHASHOINFO *pTCPHASH, stMEMSINFO *pMEMSINFO, UINT uiLastLogTime, stTIMERNINFO *pTIMER);
int dDelSessKeyNext(pSESSKEY_LIST pstDelStack, TCP_SESS_KEY *pTCPSESSKEY);
int Delete_SessList(TCP_SESS_KEY *pTCPSESSKEY);
void FreeSessKeyList(pSESSKEY_LIST node);

extern S32 dCloseSession(stMEMSINFO *pMEMSINFO, stHASHOINFO *pTCPHASH, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS);

#endif
