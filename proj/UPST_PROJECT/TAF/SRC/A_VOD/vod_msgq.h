#ifndef _VOD_MSGQ_H_
#define _VOD_MSGQ_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"

/**
 *	Declare func.
 */
extern S32 dSend_VOD_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode);

#endif /* _VOD_MSGQ_H_ */
