#ifndef _SI_NMS_SOCK_H_
#define _SI_NMS_SOCK_H_

/**
 *	Define constants
 */
#define LISTEN_PORT_NUM				20
#define MAX_NTAF_RP_NO				4
#define MAX_NTAF_PI_NO				8
#define	MAX_MTIME_LEN				40
#define	TRUE						1

// TODO lgt_nms.h 에 정의되어 있음, 다른곳에서는 사용하지 않음.
// TODO lgt_nms.h 에 정의되어 있음, 다른곳에서는 사용하지 않음.
// TODO lgt_nms.h 에 정의되어 있음, 다른곳에서는 사용하지 않음.
#define SELECT_TIMEOUT				10
#define MAX_RECEIVING_COUNT			100
#define MAX_STATISTICS_MSG_LEN		128

#define	CONTROL_PACKET				0x01
#define SIMPLE_PACKET				0x02
#define COMPLEX_PACKET				0x03

/* Message Type Field (type) */
#define INITIAL						0x00000200

#define HW_INFO						0x00001000
#define SW_INFO						0x00002000

/* Message Type Field (flow) */
#define REQUEST						0x00000002
#define CONFIRM						0x00000003
#define DATA_END					0x00000005
#define DATA_RECEIVED_OK			0x00000006

/* Message Type Field (item) */
#define ALARM_FAULT					0x00010000
#define LGT_MMC						0x00040000

/* Message Type Primitive */
#define InitialHwAlarmFaultRequest				(ALARM_FAULT | INITIAL | HW_INFO | REQUEST)
#define InitialHwAlarmFaultConfirm				(ALARM_FAULT | INITIAL | HW_INFO | CONFIRM)
#define InitialHwAlarmFaultData					(ALARM_FAULT | INITIAL | HW_INFO | POMD_DATA)
#define InitialHwAlarmFaultDataEnd				(ALARM_FAULT | INITIAL | HW_INFO | DATA_END)
#define InitialHwAlarmFaultDataReceivedOk		(ALARM_FAULT | INITIAL | HW_INFO | DATA_RECEIVED_OK)
#define InitialSwAlarmFaultRequest				(ALARM_FAULT | INITIAL | SW_INFO | REQUEST)
#define InitialSwAlarmFaultConfirm				(ALARM_FAULT | INITIAL | SW_INFO | CONFIRM)
#define InitialSwAlarmFaultData					(ALARM_FAULT | INITIAL | SW_INFO | POMD_DATA)
#define InitialSwAlarmFaultDataEnd				(ALARM_FAULT | INITIAL | SW_INFO | DATA_END)
#define InitialSwAlarmFaultDataReceivedOk		(ALARM_FAULT | INITIAL | SW_INFO | DATA_RECEIVED_OK)

#define MmcRequest									(LGT_MMC | REQUEST)
#define MmcConfirm									(LGT_MMC | CONFIRM)
#define MmcData										(LGT_MMC | POMD_DATA)

/* Message Type Field (flow) */
#define POMD_DATA					0x00000001

/**
 *	Declare functions
 */
extern void mtime2(char *sMtime);
extern int dSendMessage(int dSfd, int dMsgLen, char *sSdMsg);
extern int dInitSockFd(st_SelectInfo *stFD, int dListenIdx);
extern int Check_ClientEvent(st_NMSSFdInfo *stSock, st_SelectInfo *stFD);
extern int dSendPacket(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD, char *str, int slen);
extern int dSendBlockPacket(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD);
extern int dRecvPacket(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD);
extern int dHandleStatisticMsg(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD);
extern int dHandleSocketMsg(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD, char cFlag);
extern int dAcceptSockFd(st_NMSSFdInfo *stSock, st_SelectInfo *stFD, int dListenIdx, int *pdPos);
extern int dAddSockInTable(st_NMSSFdInfo *stSock, int dSfd, unsigned int uiIP, int dPort, st_SelectInfo *stFD);
extern int dDisConnSock(st_NMSSFdInfo *stSock, int dIdx, st_SelectInfo *stFD);
extern int dSendHeartBeat(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD);
extern int dHandleFSTATMsgQ(st_NMSSFdInfo *stSock, st_atQueryInfo *pstAtQueryInfo, st_SelectInfo *stFD);
extern int dSendPrimitive(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD, int dMsgType);
extern int dHandleCONDMsgQ(st_NMSSFdInfo *stSock, st_SelectInfo *stFD, char *psData, size_t szStrLen);
extern int dHandleALMDMsgQ(st_NMSSFdInfo *stSock, st_SelectInfo *stFD, st_almsts *pstAlmStatus);
extern int dDealMMCMsg(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD, char *sCommand, int dCmdLen);
extern int dRecvMMCResponse(st_NMSSFdInfo *stSock, int dCommunicatedIdx, st_SelectInfo *stFD);

#endif	/* _SI_NMS_SOCK_H_ */
