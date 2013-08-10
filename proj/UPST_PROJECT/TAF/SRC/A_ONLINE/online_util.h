#ifndef _ONLINE_UTIL_H_
#define _ONLINE_UTIL_H_


// LIB
#include "typedef.h"
#include "common_stg.h"


/**
 * Define constants
 */
#define MIN_MIN_NUM				0
#define MAX_MIN_NUM				22222222222LL
#define ONLINE_MAGIC_CODE		0xF00FF00F

/**
 * Declare functions
 */
extern void MakeHashKey(TCP_INFO *pTCPINFO, ONLINE_TSESS_KEY *pTSESSKEY);
extern S32 dGetCALLProcID(U32 uiClientIP);
extern void UpCount(TCP_INFO *pTCPINFO, LOG_ONLINE_TRANS *pLOG);
extern S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime);
extern U16 GetFailCode(LOG_ONLINE_TRANS *pLOG, S32 failcode);
extern U8 *PrintRtx(U8 ucRtxType);
extern S32 GetOnlineType(S32 l4code);
extern S32 GetSvcType(ONLINE_TSESS *pTSESS, TCP_INFO *pTCPINFO);
extern S32 CheckWicgsBillComHeader(void *input);
extern S32 CheckWicgsServerHeader(void *input);
extern S32 CheckMacsBillComHeader(void *input);
extern S32 CheckMacsServerHeader(void *input);


#endif	/* _ONLINE_UTIL_H_ */
