#ifndef _ROAM_UTIL_H_
#define _ROAM_UTIL_H_

/**
 *	INCLUDE HEADER FILES
 */
#include <time.h>

// DQMS
#include "common_stg.h"			/* LOG_RPPI, LOG_SIGNAL */

// LIB
#include "typedef.h"

// TAM
#include "rppi_def.h"			/* SERVICE_TYPE, ALARM_TYPE */

/**
 *	DECLARE FUNCTIONS
 */
extern U32 dGetThreshold(SERVICE_TYPE SvcType, ALARM_TYPE AlarmType);
extern S32 dGetNationID(U32 uiNASIP);
extern S32 dGetModelInfo(U8 *pIMSI, U8 *pModel);
extern S32 dGetMINInfo(U8 *pIMSI, U8 *pMIN);
extern S32 dMakeNASIPHash();
extern S32 dMakeDefectHash();
extern S32 dMakeModelHash();
extern U8 *PrintMonType(U16 MonType);
extern U8 *PrintOFFICE(U8 office);
extern U8 *PrintSVC(S32 svc);
extern S32 dGetSvcIndex(S32 PlatformType);
extern U8 *PrintMsg(LOG_SIGNAL *pstSIGNAL);
extern S32 dGetCallState(S32 dCurState, S32 dInputState);
extern void vGetLCPDuration(LOG_RPPI *pLOG);
extern void vGetIPCPDuration(LOG_RPPI *pLOG);
extern U32 uiGetLCPFail(LOG_RPPI *pLOG);
extern U32 uiGetCHAPFail(LOG_RPPI *pLOG);
extern U32 uiGetIPCPFail(LOG_RPPI *pLOG);
extern U32 uiGetL2TPFail(LOG_RPPI *pLOG);
extern S32 dGetCallControl(S32 dID);
extern unsigned int GetSubNet(unsigned int ip, int netmask);
extern int GetSubNetLoopCnt(int netmask);
extern S32 dGetCallType(S32 nat);
extern S32 dMakeIRMHash();
extern S32 dCvtIRM(U8 *szIMSI);
extern U32 uiGetSetupFailReason(S32 dCallState, U32 uiLastFailReason, U32 uiSetupFailReason);

#endif	/* _ROAM_UTIL_H_ */

