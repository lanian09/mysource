#ifndef _INET_FUNC_H_
#define _INET_FUNC_H_

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"
#include "capdef.h"

// LIB
#include "mems.h"
#include "timerN.h"
#include "hasho.h"
#include "Analyze_Ext_Abs.h"

/**
 * Define structures
 */
typedef struct _st_timer_common {
	INET_KEY			INETKEY;
} TIMER_COMMON;

/**
 * Declare functions
 */
extern S32 dProcINETCallStart(stMEMSINFO *pMEMSINFO, stTIMERNINFO *pTIMER, stHASHOINFO *pCALLHASH, stHASHOINFO *pINETHASH, CALL_KEY *pKEY);
extern S32 dProcINETCallStop(stMEMSINFO *pMEMSINFO, stTIMERNINFO *pTIMER, stHASHOINFO *pCALLHASH, stHASHOINFO *pINETHASH, CALL_KEY *pKEY);
extern S32 dProcINETData(stMEMSINFO *pMEMSINFO, stTIMERNINFO *pTIMER, stHASHOINFO *pCALLHASH, stHASHOINFO *pINETHASH, Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH);
extern INET_CALL_DATA *pCreateCall(stHASHOINFO *pCALLHASH, INET_CALL_KEY *pCALLKEY, U32 uiSessStartTime);
extern S32 dDelCall(stMEMSINFO *pMEMSINFO, stTIMERNINFO *pTIMER, stHASHOINFO *pCALLHASH, stHASHOINFO *pINETHASH, INET_CALL_KEY *pCALLKEY, INET_CALL_DATA *pCALLDATA);



#endif	/* _INET_FUNC_H_ */
