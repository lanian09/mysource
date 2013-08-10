#ifndef _CI_LOG_FUNC_H_
#define _CI_LOG_FUNC_H_

/**
 *	Include headers
 */
// TOP
#include "msgdef.h"

/**
 *	Define cons.
 */
#define DEF_CONNECT			3
#define DEF_LOGOUT			0

// TODO dqms_ipclib.h 에 있었음
#define IPC_RETRY_CNT       2
/**
 *	Declare func.
 */
int dSndMsgProc(int type, int size, char *data);
int dSndTraceMsgProc(char *pTraceHdr, char *pPacket, int TraceHdrLen, int PacketLen);
int dProc_Msg();
int dCheckMQRcv(st_MsgQ *pstRcv);
int dCheckNProc_Event();
int dCheckSock();

#endif	/* _CI_LOG_FUNC_H_ */
