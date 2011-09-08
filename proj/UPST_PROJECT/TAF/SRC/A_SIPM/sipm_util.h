#ifndef _SIPM_UTIL_H_
#define _SIPM_UTIL_H_

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"
#include "capdef.h"

// LIB
#include "typedef.h"
#include "Analyze_Ext_Abs.h"

// .
#include "sipm_func.h"

/**
 *	Declare functions
 */
extern void MakeHashKey(TCP_INFO *pstTCPINFO, st_SIPM_TSESS_KEY *pstSIPMTSESSKEY);
extern char *PrintRtx(S32 dRtxType);
extern char *PrintEndStatus(S32 dEndStatus);
extern S32 dCheckStart(U8 *pData, U32 uiLen);
extern S32 dGetHDRINFO(U8 *pData, U32 uiLen);
extern void InitCount(st_SIPM_TSESS *pstSIPMTSESS, U8 ucRtx);
extern void UpCount(TCP_INFO *pstTCPINFO, st_SIPM_TSESS *pstSIPMTSESS);
extern void SetTextInfo(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, TEXT_INFO *pTEXTINFO);

#endif	/* _SIPM_UTIL_H_ */
