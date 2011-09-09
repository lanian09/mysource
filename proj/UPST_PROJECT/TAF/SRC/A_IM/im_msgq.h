/*
 * im_msgq.h
 *
 *  Created on: 2011. 8. 10.
 *      Author: hohyun
 */

#ifndef _IM_MSGQ_H_
#define _IM_MSGQ_H_

/**
 *	Include headers
 */
// LIB
#include "mems.h"

/**
 *	Declare func.
 */
extern S32 dSend_IM_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode);

#endif /* IM_MSGQ_H_ */
