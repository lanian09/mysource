#ifndef _SIPT_UTIL_H_
#define _SIPT_UTIL_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"

// .
#include "sipt_func.h"

/**
 *	Declare functions
 */
extern S32 dGetSIPTransKey(U8 *data, U32 len, SIP_INFO_KEY *pSIPINFOKEY, S32 *msgtype, U32 *ip);
extern char *PrintMsgType(S32 msgtype);
extern char *PrintResStr(S32 msgtype);
extern S32 dGetMsgType(S32 msgtype);
extern char *PrintStatus(S32 type);
extern char *PrintTYPE(S32 type);
extern S32 dGetL7TYPE(S32 dRtx);
extern void AddMsg(void *p1, void *p2);
extern void InitLog(void *p1, void *p2);
extern void SetResSkip(void *p1, void *p2);
extern void SetResNormal(void *p1, void *p2);
extern void SetRetransReq(void *p1, void *p2);
extern void SetRetransRes(void *p1, void *p2);
extern void SetAck(void *p1, void *p2);
extern void SetRetransAck(void *p1, void *p2);
extern void SetReqDataSize(void *p1, void *p2);
extern void SetResDataSize(void *p1, void *p2);

#endif	/* _SIPT_UTIL_H_ */
