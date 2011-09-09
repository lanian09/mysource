#ifndef __RPPI_UTIL_H__
#define __RPPI_UTIL_H__

#include "typedef.h"
#include "rppi_def.h"	/* SERVICE_TYPE */

extern U16 dGetBranchID(U32 uiPCFIP);
extern U32 dGetThreshold(SERVICE_TYPE SvcType, ALARM_TYPE AlramType);
extern U8  dGetPCFType(U32 uiPCFIP);
extern S32 dGetModelInfo(U8 *pIMSI, U8 *pModel);
extern S32 dGetMINInfo(U8 *pIMSI, U8 *pMIN);
extern S32 dMakePCFHash();
extern S32 dMakeDefectHash();
extern S32 dMakeModelHash();
extern char *PrintMonType(U16 Montype);
extern char *PrintOFFICE(U8 office);
extern char *PrintSVC(S32 svc);
extern S32 dGetSvcIndex(S32 isReCall, S32 PlatformType, U32 L7Type);
extern char *PrintMsg(LOG_SIGNAL *pstSIGNAL);
extern S32 dGetCallState(S32 dCurState, S32 dInputState);
extern U32 uiGetSetupFailReason(S32 isReCall, S32 dCallState, U32 uiLastFailReason, U32 uiSetupFailReason);
extern void vGetLCPDuration(LOG_RPPI *pLOG);
extern void vGetIPCPDuration(LOG_RPPI *pLOG);
extern U32 uiGetLCPFail(LOG_RPPI *pLOG);
extern U32 uiGetCHAPFail(LOG_RPPI *pLOG);
extern U32 uiGetIPCPFail(LOG_RPPI *pLOG);
extern S32 dGetIsRecall(S32 type);
extern char *PrintStopType(S32 type);
extern int dSetPDSNIFHash(int pdsnID, UINT piIP, UINT rpIP);
extern UINT dGetPDSNIFHash(UINT piIP);


#endif /* __RPPI_UTIL_H__ */
