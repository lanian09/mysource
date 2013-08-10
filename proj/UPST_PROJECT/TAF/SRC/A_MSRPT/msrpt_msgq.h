#ifndef __MSRPT_MSGQ_H__
#define __MSRPT_MSGQ_H__

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"

/**
 *	Declare func.
 */
extern S32 dSend_MSRPT_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode);

#endif	/* __MSRPT_MSGQ_H__ */
