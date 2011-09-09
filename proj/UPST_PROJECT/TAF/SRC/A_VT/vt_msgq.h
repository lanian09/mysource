/*
 * vt_msgq.h
 *
 *  Created on: 2011. 8. 10.
 *      Author: hohyun
 */

#ifndef _VT_MSGQ_H_
#define _VT_MSGQ_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"

/**
 *	Declare func.
 */
extern S32 dSend_VT_Data(stMEMSINFO *pMEMSINFO, S32 dSndMsgQ, U8 *pNode);

#endif /* _VT_MSGQ_H_ */
