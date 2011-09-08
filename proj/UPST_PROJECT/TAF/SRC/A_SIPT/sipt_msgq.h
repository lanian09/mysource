#ifndef _SIPT_MSGQ_H_
#define _SIPT_MSGQ_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"

/**
 * Declare functions
 */
S32 dSend_SIPT_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode);

#endif	/* _SIPT_MSGQ_H_ */
