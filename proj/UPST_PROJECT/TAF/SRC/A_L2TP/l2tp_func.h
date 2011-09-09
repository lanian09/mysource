#ifndef _L2TP_FUNC_H_
#define _L2TP_FUNC_H_

/**
 * Include headers
 */

// LIB
#include "typedef.h"

#include "Analyze_Ext_Abs.h" 

// PROJECT
#include "capdef.h"

// TAF
#include "mmdb_psess.h"


#define	L2TP_CONTROL_CNT		10007
#define	L2TP_CALLSESS_CNT		20011
#define	L2TP_TRANS_CNT			1021

#define L2TP_MSG_ZLB 			0
#define L2TP_MSG_SCCRQ 			1
#define L2TP_MSG_SCCRP			2
#define L2TP_MSG_SCCCN			3
#define L2TP_MSG_StopCCN		4
#define L2TP_MSG_Reserved		5
#define L2TP_MSG_HELLO			6
#define L2TP_MSG_OCRQ			7
#define L2TP_MSG_OCRP			8
#define L2TP_MSG_ORCRP			9
#define L2TP_MSG_ICRQ 			10
#define L2TP_MSG_ICRP 			11
#define L2TP_MSG_ICCN 			12
#define L2TP_MSG_Reserved1 		13
#define L2TP_MSG_CDN 			14

/* LAST USER ERROR CODE STATUS */
#define L2TP_USER_SUCCESS     0
#define L2TP_USER_RETRANS     1   
#define L2TP_USER_FAILED      2   
#define L2TP_USER_TIMEOUT     3

/* TUNNEL SESSION */
typedef struct _st_L2TP_CONTROL_KEY {
	UINT 		uiSrcIP;
	UINT 		uiDestIP;
	USHORT 		usReqTunnelID;
	USHORT 		usResTunnelID;
	CHAR		szReserved[2];
} st_CONTROL_KEY;
#define DEF_CONTROL_KEY_SIZE   sizeof (st_CONTROL_KEY) 

typedef struct _st_L2TP_CONTROL_DATA {
	st_CONTROL_KEY 		stControlKey; 					/* CONTROL SESSION KEY */

	U64                 ulTimerNID;
	OFFSET 				dOffset;

	int					dReqFlag;
	UINT                uiCmdCode;
	UINT				uiLastMessage;

	USHORT 				usFirstCallsessID;
	USHORT 				usLastCallsessID;
	USHORT 				usNextCallsessID;
	USHORT 				usCurCallsessCnt;
	USHORT 				usTotCallsessCnt;

	UCHAR				szIMSI[MAX_MIN_SIZE];
	UCHAR				szTraceMIN[MAX_MIN_SIZE];

	STIME				uiReqTime;
	MTIME				uiReqMTime;
	STIME				uiResTime;
	MTIME				uiResMTime;
	STIME				uiConTime;
	MTIME				uiConMTime;
	STIME				uiStopTime;
	MTIME				uiStopMTime;
	UINT                uiLastUserErrCode;
	UINT				uiRetransReqCnt;
	UINT                uiUpPktCnt;
	UINT                uiDnPktCnt;
	UINT                uiUpPktBytes;
	UINT                uiDnPktBytes;
} st_CONTROL_DATA;                                                                  
#define DEF_CONTROL_DATA_SIZE  sizeof (st_CONTROL_DATA)

/* CALL SESSION */
typedef struct _st_L2TP_CALLSESS_KEY {
	UINT 		uiSrcIP;
	UINT 		uiDestIP;
	UCHAR 		szIMSI[MAX_MIN_SIZE];
	USHORT 		usReqSessID;
	USHORT 		usResSessID;
	CHAR		szReserved[4];
} st_CALLSESS_KEY;
#define DEF_CALLSESS_KEY_SIZE   sizeof (st_CALLSESS_KEY) 

typedef struct _st_L2TP_CALLSESS_DATA {
	st_CALLSESS_KEY 	stCallSessKey;
	st_CONTROL_KEY 		stControlKey; 					/* CONTROL SESSION KEY */

	U64                 ulTimerNID;
	U64                 ulReqSIDTimerNID;
	U64                 ulResSIDTimerNID;
	OFFSET 				dOffset;

	int					dReqFlag;
	UINT                uiCmdCode;
	UINT				uiLastEEID;
	UINT 				uiIPAddr;

	UCHAR				szIMSI[MAX_MIN_SIZE];
	UCHAR				szTraceMIN[MAX_MIN_SIZE];
	UCHAR 				ucBranchID;

	STIME				uiReqTime;
	MTIME				uiReqMTime;
	STIME				uiResTime;
	MTIME				uiResMTime;
	STIME				uiConTime;
	MTIME				uiConMTime;
	STIME				uiStopTime;
	MTIME				uiStopMTime;
	UINT                uiLastUserErrCode;
	UINT				uiRetransReqCnt;
	UINT                uiUpPktCnt;
	UINT                uiDnPktCnt;
	UINT                uiUpPktBytes;
	UINT                uiDnPktBytes;

	USHORT 				usPPPFlag;
	USHORT 				ucAuthResultCode;

	T_SESS 				UpLCP;  
	T_SESS 				DownLCP;
	T_SESS 				UpLCPEcho;
	T_SESS 				DownLCPEcho;
	T_SESS 				CHAPAP; 
	T_SESS 				UpIPCP; 
	T_SESS 				DownIPCP;
	T_SESS 				LCPTerm;    

	UCHAR 				ucRegiReqCount;
	UCHAR 				ucRegiSuccCount;
	UCHAR 				ucUpdateReqCount;
	UCHAR 				ucUpdateAckCount;
	UCHAR 				ucUpLCPReqCount;
	UCHAR 				ucDownLCPReqCount;
	UCHAR 				ucUpIPCPReqCount;
	UCHAR 				ucDownIPCPReqCount;

	UCHAR 				ucCHAPFailCount;
	UCHAR 				ucPPPSetupCount;

	STIME  				UpLCPStartTime;
	STIME 				DownLCPStartTime;
	STIME 				UpLCPStartMTime;
	STIME 				DownLCPStartMTime;

	STIME 				AuthReqTime;
	STIME 				CHAPResTime;
	STIME 				AuthReqMTime;
	STIME 				CHAPResMTime;

	STIME 				AuthEndTime;
	STIME 				IPCPStartTime;
	STIME 				AuthEndMTime;
	STIME 				IPCPStartMTime;

	STIME 				PPPSetupTime;
	STIME 				PPPTermTime;
	STIME 				PPPSetupMTime;
	STIME 				PPPTermMTime;

	STIME 				uiLastUpdateTime;
	MTIME 				uiLastUpdateMTime;

	UCHAR 				szAuthUserName[32];
	UCHAR 				szCHAPFailMsg[64];

	UINT 				uiLCPDuration;
	UINT 				uiIPCPDuration; 
	UINT 				uiAuthDuration;

	UINT 				uiUpPPPFrames;
	UINT 				uiDownPPPFrames;
	UINT 				uiUpPPPBytes;
	UINT 				uiDownPPPBytes;

	UINT 				uiUpAnaErrFrames;
	UINT 				uiDownAnaErrFrames;

	UINT 				uiUpFCSErrFrames;
	UINT 				uiDownFCSErrFrames;
	UINT 				uiUpFCSErrBytes;
	UINT 				uiDownFCSErrBytes;

} st_CALLSESS_DATA;                                                                  
#define DEF_CALLSESS_DATA_SIZE  sizeof (st_CALLSESS_DATA)


/* TUNNEL ID HASH TABLE */
typedef struct _st_L2TP_TUNNELID_KEY {
	UINT 		uiSrcIP;
	UINT 		uiDestIP;
	USHORT 		usSrcPort;
	USHORT 		usDestPort;
	USHORT 		usTunnelID;
	CHAR		szReserved[2];
} st_TUNNELID_KEY;
#define DEF_TUNNELID_KEY_SIZE   sizeof (st_TUNNELID_KEY) 

typedef struct _st_L2TP_TUNNEL_DATA {
	st_CONTROL_KEY 	stCONTROLKey;
	
	U64 			ulTimerNID;
	OFFSET 			dOffset;
	USHORT 			usLastMessageType;
	STIME 			ReqTunnelIDTime;
	MTIME 			ReqTunnelIDMTime;
	STIME 			ResTunnelIDTime;
	MTIME 			ResTunnelIDMTime;

	STIME			uiReqTime;
	MTIME			uiReqMTime;
	STIME			uiResTime;
	MTIME			uiResMTime;
	STIME			uiConTime;
	MTIME			uiConMTime;
	STIME			uiStopTime;
	MTIME			uiStopMTime;
	UINT            uiLastUserErrCode;

	UINT			uiUpPktCnt;
	UINT			uiDnPktCnt;
	UINT			uiUpPktBytes;
	UINT			uiDnPktBytes;
} st_TUNNELID_DATA;
#define DEF_TUNNELID_DATA_SIZE   sizeof (st_TUNNELID_DATA) 

/* SESSION ID HASH TABLE */
typedef struct _st_L2TP_SESSID_KEY {
	UINT 		uiSrcIP;
	UINT 		uiDestIP;
	USHORT 		usSrcPort;
	USHORT 		usDestPort;
	USHORT 		usSessionID;
	CHAR		szReserved[2];
} st_SESSID_KEY;
#define DEF_SESSID_KEY_SIZE   sizeof (st_SESSID_KEY) 

typedef struct _st_L2TP_SESSID_DATA {
	st_CALLSESS_KEY 	stCALLSESSKey;

	U64 			ulTimerNID;
	OFFSET 			dOffset;

	UCHAR 			ucBranchID;
	UCHAR			szIMSI[MAX_MIN_SIZE];
	USHORT 			usLastMessageType;
	STIME 			ReqSessionIDTime;
	MTIME 			ReqSessionIDMTime;
	STIME 			ResSessionIDTime;
	MTIME 			ResSessionIDMTime;

	STIME			uiReqTime;
	MTIME			uiReqMTime;
	STIME			uiResTime;
	MTIME			uiResMTime;
	STIME			uiConTime;
	MTIME			uiConMTime;
	STIME			uiStopTime;
	MTIME			uiStopMTime;
	UINT            uiLastUserErrCode;

	UINT			uiUpPktCnt;
	UINT			uiDnPktCnt;
	UINT			uiUpPktBytes;
	UINT			uiDnPktBytes;
} st_SESSID_DATA;
#define DEF_SESSID_DATA_SIZE   sizeof (st_SESSID_DATA) 

/* TIMER INFO */
typedef struct _st_timer_Control_data {
	st_CONTROL_KEY 		key_trans;
} TCONTROL_DATA;
#define DEF_TDATA_CONTROL_TIMER_SIZE  sizeof (TCONTROL_DATA)

typedef struct _st_timer_CallSess_data {
	st_CALLSESS_KEY 	key_trans;
} TCALLSESS_DATA;
#define DEF_TDATA_CALLSESS_TIMER_SIZE  sizeof (TCALLSESS_DATA)

/* ID TIMER INFO */
typedef struct _st_timer_tunnelid_data {
	st_TUNNELID_KEY		key_trans;
} TTUNNELID_DATA;
#define DEF_TDATA_TUNNELID_TIMER_SIZE  sizeof (TTUNNELID_DATA)

typedef struct _st_timer_sessid_data {
	st_SESSID_KEY 	key_trans;
} TSESSID_DATA;
#define DEF_TDATA_SESSID_TIMER_SIZE  sizeof (TSESSID_DATA)
/**
 * Declare functions
 */

extern S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime);
extern int dump(char *s,int len);
extern int cb_timeout_Control(st_CONTROL_KEY *pstTransKey);
extern int cb_timeout_CallSess(st_CALLSESS_KEY *pstTransKey);
extern int cb_timeout_Sessid(st_SESSID_KEY *pstTransKey);
extern int cb_timeout_tunnelid(st_TUNNELID_KEY *pstTransKey);
extern int dSendTunnelIDSignal(Capture_Header_Msg *pCAPHEAD, st_TUNNELID_KEY *pstTunnelIDKey, st_TUNNELID_DATA *stTunnelIDData);
extern int dSendSessIDSignal(Capture_Header_Msg *pCAPHEAD, st_SESSID_KEY *pstSessIDKey, st_SESSID_DATA *pstSessIDData);
extern int dSendStopCCNSignal(Capture_Header_Msg *pCAPHEAD, st_CONTROL_KEY *pstControlKey, st_CONTROL_DATA *pstControlData, st_TUNNELID_KEY *pstTunnelIDKey);
extern int dSendStopCCNSignalOrg(Capture_Header_Msg *pCAPHEAD, st_CONTROL_KEY *pstControlKey, st_CONTROL_DATA *pstControlData, st_TUNNELID_KEY *pstTunnelIDKey);
extern int dSendCDNSignal(Capture_Header_Msg *pCAPHEAD, st_CALLSESS_KEY *pstCallSessKey, st_CALLSESS_DATA *pstCallSessData, st_SESSID_KEY *pstSessionIDKey);
extern int dSendStartSignal(LOG_SIGNAL *pstTransLog);
extern char *szPrintMsgType(USHORT MsgType);
extern st_CONTROL_DATA *pGetControlSessionData(st_TUNNELID_KEY *pstTunnelIDKey);
extern int dProcessTunnelMessages(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, st_TUNNELID_KEY *pstTunnelIDKey, st_L2TP_INFO *pstL2TPInfo);
extern int dCheck_TraceInfo( st_SESSID_DATA *pstSessData, UCHAR *pData, Capture_Header_Msg *pstCAP );
extern int dManageControlSession( st_CONTROL_KEY *pstControlKey );
extern int dDeleteControlSession( st_CONTROL_KEY *pstControlKey );
extern int dProcessSessionMessages(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, st_SESSID_KEY *pstSessionIDKey, st_TUNNELID_KEY *pstTunnelIDKey, st_L2TP_INFO *pstL2TPInfo);
extern int Report_SIGLog( UCHAR ucProtoType, UCHAR ucMsgType, st_CALLSESS_DATA *pPSessData );
extern int dProcPPP( st_CALLSESS_DATA *pPSessData, UCHAR *pBuf, USHORT siSize, INFO_ETH *pstEth, Capture_Header_Msg *pstCAP );
extern int dProcessDataMessages(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, st_SESSID_KEY *pstSessionIDKey, st_TUNNELID_KEY *pstTunnelIDKey, st_L2TP_INFO *pstL2TPInfo);
extern int dProcL2TP_Trans( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, st_L2TP_INFO *pstL2TPInfo);
extern void printL2TPInfo(st_L2TP_INFO *pstL2TPInfo);


#endif	/* _L2TP_FUNC_H_ */

