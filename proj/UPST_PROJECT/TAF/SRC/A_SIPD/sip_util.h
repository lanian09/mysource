#ifndef _SIP_UTIL_H_
#define _SIP_UTIL_H_

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"

// LIB
#include "typedef.h"

/**
 * Declare functions
 */
S32 dGetSIP(U8 *data, U32 len, LOG_SIP_TRANS *pLOG);
U8 *PrintTYPE(S32 type);
U8 *dGetQStr(USHORT usPlatformType);


#endif	/* _SIP_UTIL_H_ */
