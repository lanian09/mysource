/**
	@file		m_msgq.h
	@author
	@version
	@date		2011-07-26
	@brief		m_msgq.c 헤더파일
*/

#ifndef __M_MSGQ_H__
#define __M_MSGQ_H__

/**
	Include headers
*/
#include <time.h>
#include <mysql/mysql.h>

// DQMS
#include "msgdef.h"

/**
	Declare functions
*/
extern int dIsReceivedMessage(pst_MsgQ *pstMsgQ);
extern void SendMsg_SI_NMS(void *p);
extern int dSend_NMS_HOUR(time_t tCurrent);

extern int dCheckStatistics(MYSQL *pstMySQL, time_t tStatTime);
extern int dCheckHourStat(MYSQL *pstMySQL, time_t tStart, time_t tEnd);


#endif	/* __M_MSGQ_H__ */
