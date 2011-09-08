#ifndef _INET_UTIL_H_
#define _INET_UTIL_H_

/**
 *	Include headers
 */
#include <time.h>

// TOP
#include "capdef.h"
#include "common_stg.h"

// LIB
#include "typedef.h"
#include "Analyze_Ext_Abs.h"	/* INFO_ETH */

/**
 * Declare functions
 */
extern U8 *CVT_TIME(time_t t, U8 *szDest);
extern void MakeCALLHashKey(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, INET_CALL_KEY *pCALLKEY);
extern void MakeINETHashKey(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, INET_KEY *pINETKEY);
extern S32 dGetProcID(U32 uiClientIP, U8 ucProtocol);
extern S32 dGetCALLProcID(U32 uiClientIP);


#endif	/* _INET_UTIL_H_ */
