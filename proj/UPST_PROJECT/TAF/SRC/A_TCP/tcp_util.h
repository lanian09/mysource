#ifndef _TCP_UTIL_H_
#define _TCP_UTIL_H_

#include "typedef.h"
#include "common_stg.h"
#include "capdef.h"
#include "Analyze_Ext_Abs.h"

#define DEF_NORETRANS_OVER          1
#define DEF_NORETRANS_UNDER         2
#define DEF_RETRANS_OVER            3
#define DEF_RETRANS_UNDER           4
#define DEF_RETRANS_SUBSET          5
#define DEF_RETRANS_RSUBSET         6


extern U8 *PrintFinStatus(U8 ucFinStatus);
extern U8 *PrintEndStatus(U8 ucEndStatus);
extern U8 *PrintRtx(U8 ucRtxType);
extern void SetData(TCP_SESS *pTCPSESS, TCP_INFO *pTCPINFO);
extern void MakeTCPHashKey(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, TCP_SESS_KEY *pTCPSESSKEY);
extern U8 GetTCPControl(U8 control);
extern U8 *PrintControl(U8 ucControl);
extern U16 GetFailCode(TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS);
extern S32 CalcBase(TCP_INFO *pNEW, U32 baseSEQ, U32 oldSEQ, U32 oldDataSize, S32 *pRetransSize);
extern S32 CalcRetrans(TCP_INFO *pNEW, U32 baseSEQ, U32 oldSEQ, U32 oldDataSize, S32 *pRetransSize);


#endif /* _TCP_UTIL_H_ */
