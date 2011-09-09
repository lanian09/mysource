#ifndef _SI_DB_MSGQ_H_
#define _SI_DB_MSGQ_H_

/**
 *	Include headers
 */
// DQMS
#include "msgdef.h"

/**
 *	Declare functions
 */
extern int dIsReceivedMessage(st_MsgQ *pstMsgQ);
extern int dSendMsg(int dProcSeq, st_MsgQ *pstMsgQ);


#endif	/* _SI_DB_MSGQ_H_ */
