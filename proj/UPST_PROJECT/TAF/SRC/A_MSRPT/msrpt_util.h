#ifndef __MSRPT_UTIL_H__
#define __MSRPT_UTIL_H__

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"

// .
#include "msrpt_func.h"

/**
 *	Define constants
 */
#define MSRPEND_STATE_EMPTY				0
#define MSRPEND_STATE_0D				1
#define MSRPEND_STATE_0D0A				2
#define MSRPEND_STATE_0D0A0D			3
#define MSRPEND_STATE_0D0A0D0A			4

/**
 *	Declare func.
 */
extern char *PrintType(S32 type);
extern S32 dGetMSRPTINFO(U8 *pDATA, U32 uiLen, st_MSRPT_INFO *pstMSRPTINFO);
extern S32 dGetMsgStatus(S32 dMethod, S32 dEndFlag);
extern char *PrintStatus(S32 dStatus);
extern char *PrintMsgStatus(S32 dMsgStatus);
extern char *PrintMethod(S32 dMethod);
extern char *PrintReport(S32 dFlag);
extern char *PrintEndFlag(S32 dEndFlag);
extern char *PrintFinish(S32 dFinish);
extern char *PrintResStr(S32 msgtype);
extern S32 dGetL7TYPE(S32 dRtx);
extern int dGetLen(int dDataLen, char *data);
extern void InitLog(stMEMSINFO *pstMEMSINFO, st_MSRPT_INFO *pstMSRPTINFO, TEXT_INFO *pstTEXTINFO, LOG_MSRP_TRANS *pLOG);
extern void SetNormalReq(stMEMSINFO *pstMEMSINFO, st_MSRPT_INFO *pstMSRPTINFO, TEXT_INFO *pstTEXTINFO, LOG_MSRP_TRANS *pLOG);
extern void SetNormalRes(stMEMSINFO *pstMEMSINFO, st_MSRPT_INFO *pstMSRPTINFO, TEXT_INFO *pstTEXTINFO, LOG_MSRP_TRANS *pLOG);
extern void SetNormalReport(stMEMSINFO *pstMEMSINFO, st_MSRPT_INFO *pstMSRPTINFO, TEXT_INFO *pstTEXTINFO, LOG_MSRP_TRANS *pLOG);

#endif	/* __MSRPT_UTIL_H__ */
