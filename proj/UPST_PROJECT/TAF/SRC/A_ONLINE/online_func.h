#ifndef _ONLINE_FUNC_H_
#define _ONLINE_FUNC_H_

/**
 * Include headers
 */
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

// LIB
#include "typedef.h"
#include "mems.h"
#include "hasho.h"

// PROJECT
#include "common_stg.h"

/**
 * Declare variables
 */
extern S64		curSessCnt;		/* Transaction 개수 */
extern S64		sessCnt;
extern S64		rcvNodeCnt;		/* 받은 NODE 개수 */
extern S64		diffSeqCnt;		/* DIFF SEQ가 된 개수 */
extern S32		__loop_cnt;

/**
 * Declare functions
 */
extern S32 dProcOnlineTrans(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHASH, TCP_INFO *pTCPINFO, U8 *pNode, U8 *pDATA);
extern S32 dCloseSess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHASH, ONLINE_TSESS_KEY *pTSESSKEY, ONLINE_TSESS *pTSESS);


#endif	/* _ONLINE_FUNC_H_ */
