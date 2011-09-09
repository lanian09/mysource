#ifndef __PRE_A_MSGQ_H__
#define __PRE_A_MSGQ_H__

/**
 *	Include headers
 */
#include "typedef.h"
#include "mems.h"

/**
 *	Define constants
 */
#define COLLECTION_MAX          100
#define COLLECTION_TIME         5
#define COLLECTION_MULTIPLY     2

/**
 *	Declare func.
 */
extern int dSend_PREABUFF_Data(stMEMSINFO *pstMEMSINFO, S32 dSndProcID, U8 *pNode, U32 sec, U32 index);
extern S32 dSend_PREA_Data(stMEMSINFO *pMEMSINFO, S32 dSndProcID, U8 *pNode);

#endif /* __PRE_A_MSGQ_H__ */
