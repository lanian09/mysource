#ifndef _ONLINE_MSGQ_H_
#define _ONLINE_MSGQ_H_

/**
 * Include headers
 */

// LIB
#include "typedef.h"
#include "mems.h"

/**
 * Declare variables
 */
extern stCIFO *gpCIFO;

/**
 * Declare functions
 */
extern S32 dSend_ONLINE_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode);


#endif	/* _ONLINE_MSGQ_H_ */
