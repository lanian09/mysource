#ifndef _MSRPM_UTIL_H_
#define _MSRPM_UTIL_H_

#include "typedef.h"
#include "common_stg.h"
#include "msrpm_func.h"

extern void MakeHashKey(TCP_INFO *pstTCPINFO, st_MSRPM_TSESS_KEY *pstMSRPMTSESSKEY);
extern char *PrintRtx(S32 dRtxType);
extern char *PrintMethod(S32 dMethod);
extern S32 dCheckStart(U8 *pData, U32 uiLen, U8 *szTID);
extern void InitCount(st_MSRPM_TSESS *pstMSRPMTSESS, U8 ucRtx);
extern void UpCount(TCP_INFO *pstTCPINFO, st_MSRPM_TSESS *pstMSRPMTSESS);
#endif /* _MSRPM_UTIL_H_ */
