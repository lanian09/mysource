#ifndef _INET_MSGQ_H_
#define _INET_MSGQ_H_

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"

// LIB
#include "mems.h"
#include "typedef.h"

/**
 * Define constants
 */
#define COLLECTION_MAX          100
#define COLLECTION_TIME         5
#define COLLECTION_MULTIPLY     2

/**
 * Declare functions
 */
extern S32 dSendINETLog(stMEMSINFO *pMEMSINFO, LOG_INET *pLOG, S32 dSeqProcID);
extern S32 dSend_INET_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pData);
extern int dSend_INET2_Data(stMEMSINFO *pstMEMSINFO, S32 dSeqProcID, U8 *pNode, U32 sec, U32 index);

#endif	/* _INET_MSGQ_H_ */
