#ifndef _MSRPM_FUNC_H_
#define _MSRPM_FUNC_H_

#include "typedef.h"
#include "common_stg.h"	/* MSRP_TID_SIZE */
#include "mems.h"
#include "hasho.h"

/* st_MSRPM_MSG_INFO의 dStatus */
#define MSRPM_STATUS_WAIT		0
#define MSRPM_STATUS_START		1

/* st_MSRPM_MSG_INFO의 dEndStatus */
#define MSRPM_ENDSTATUS_EMPTY		0
#define MSRPM_ENDSTATUS_T1			1
#define MSRPM_ENDSTATUS_T2			2
#define MSRPM_ENDSTATUS_T3			3
#define MSRPM_ENDSTATUS_T4			4
#define MSRPM_ENDSTATUS_T5			5
#define MSRPM_ENDSTATUS_T6			6
#define MSRPM_ENDSTATUS_T7			7
#define MSRPM_ENDSTATUS_T8			8
#define MSRPM_ENDSTATUS_T9			9
#define MSRPM_ENDSTATUS_T10			10
#define MSRPM_ENDSTATUS_T11			11
#define MSRPM_ENDSTATUS_T12			12
#define MSRPM_ENDSTATUS_T13			13
#define MSRPM_ENDSTATUS_T14			14
#define MSRPM_ENDSTATUS_T15			15
#define MSRPM_ENDSTATUS_T16			16
#define MSRPM_ENDSTATUS_T17			17
#define MSRPM_ENDSTATUS_T18			18
#define MSRPM_ENDSTATUS_T19			19
#define MSRPM_ENDSTATUS_T20			20
#define MSRPM_ENDSTATUS_T21			21
#define MSRPM_ENDSTATUS_T22			22
#define MSRPM_ENDSTATUS_T23			23
#define MSRPM_ENDSTATUS_T24			24
#define MSRPM_ENDSTATUS_T25			25
#define MSRPM_ENDSTATUS_T26			26
#define MSRPM_ENDSTATUS_T27			27
#define MSRPM_ENDSTATUS_T28			28
#define MSRPM_ENDSTATUS_T29			29
#define MSRPM_ENDSTATUS_T30			30
#define MSRPM_ENDSTATUS_T31			31
#define MSRPM_ENDSTATUS_T			40
#define MSRPM_ENDSTATUS_H1			101
#define MSRPM_ENDSTATUS_H2			102
#define MSRPM_ENDSTATUS_H3			103
#define MSRPM_ENDSTATUS_H4			104
#define MSRPM_ENDSTATUS_H5			105
#define MSRPM_ENDSTATUS_H6			106
#define MSRPM_ENDSTATUS_H7			107
#define MSRPM_ENDSTATUS_F			200
#define MSRPM_ENDSTATUS_0D			300
#define MSRPM_ENDSTATUS_END			301

typedef struct _st_MSRPM_MSG_INFO {
	U32			uiLastSeq;
	U32			uiNextSeq;			/**< 다음에 받을 것으로 예상되는 Seq No. */
	STIME		uiStartTime;
	MTIME		uiStartMTime;
	STIME		uiLastUpdateTime;
	MTIME		uiLastUpdateMTime;
	STIME		uiAckTime;
	MTIME		uiAckMTime;
	U32			uiRawFileIndex;
	U32			uiRawPacketIndex;
	S32			dStatus;
	S32			dEndStatus;
	U32			uiLen;
	U32			uiIPDataSize;
	OFFSET		offset_DATA;
	S32			dTIDLen;
	U8			szTID[MSRP_TID_SIZE];
} st_MSRPM_MSG_INFO, *pst_MSRPM_MSG_INFO;
#define DEF_MSRPMMSGINFO_SIZE		sizeof(st_MSRPM_MSG_INFO)

#define MSRPM_MSG_INFO_CNT		2

/**
 * @brief st_MSRPM_TSESS_KEY : A_MSRPM에서 TCP Session 관리를 Hash Key 구조체
 *
 *
 * @see msrpm_api.h
 *
 * @note   Nothing
 *
 * @todo   Nothing
 */
typedef struct _st_MSRPM_TSESS_KEY {
	U32				uiCliIP;				/**< 단말 IP Address */
	U32				uiSrvIP;				/**< 서버 IP Address */
	U16				usCliPort;				/**< 단말 Port */
	U16				usSrvPort;				/**< 서버 Port */
	S32				dReserved;
} st_MSRPM_TSESS_KEY, *pst_MSRPM_TSESS_KEY;
#define DEF_MSRPMTSESSKEY_SIZE	sizeof(st_MSRPM_TSESS_KEY)

/**
 * @brief st_MSRPM_TSESS : A_MSRPM에서 TCP Session 관리를 Hash Data 구조체
 *
 *
 * @see msrpm_api.h
 *
 * @note   Nothing
 *
 * @todo   Nothing
 */
typedef struct _st_MSRPM_TSESS {
	STIME		uiTcpSynTime;			/**< TCP Session 시작 시간 */
	MTIME		uiTcpSynMTime;		/**< TCP Session 시작 시간 */
	U8			ucSynRtx;				/**< SYN UP/Down 방향 1: UP, 2: Down */

	DEF			dL4FailCode;
	U16			usAppCode;
	U16			usL4Code;
	U16			usL7Code;

	U8			ucTcpClientStatus;
	U8			ucTcpServerStatus;

	S32			dRange;
	S32			dNetwork;

	st_MSRPM_MSG_INFO	stMSRPMMSGINFO[MSRPM_MSG_INFO_CNT];

	U32			uiIPDataUpPktCnt;		/**< MN => Server, 데이터 전송 단계에서 패킷 개수 (전송후 Reset) */
	U32			uiIPDataDnPktCnt;		/**< Server => MN, 데이터 전송 단계에서 패킷 개수 (전송후 Reset) */
	U32			uiIPTotUpPktCnt;		/**< MN => Server, 전구간에서 패킷 개수 (전송후 Reset) */
	U32			uiIPTotDnPktCnt;		/**< Server => MN, 전구간에서 패킷 개수 (전송후 Reset) */
	U32			uiIPDataUpRetransCnt;	/**< MN => Server, 데이터 전송 단계에서 재전송 패킷 개수 (전송후 Reset) */
	U32			uiIPDataDnRetransCnt;	/**< Server => MN, 데이터 전송 단계에서 재전송 패킷 개수 (전송후 Reset) */
	U32			uiIPTotUpRetransCnt;	/**< MN => Server, 전구간에서 재전송 패킷 개수 (전송후 Reset) */
	U32			uiIPTotDnRetransCnt;	/**< Server => MN, 전구간에서 재전송 패킷 개수 (전송후 Reset) */
	U32			uiIPDataUpPktSize;		/**< MN => Server, 데이터 전송 단계에서 데이터 사이즈 (전송후 Reset) */
	U32			uiIPDataDnPktSize;		/**< Server => MN, 데이터 전송 단계에서 데이터 사이즈 (전송후 Reset) */
	U32			uiIPTotUpPktSize;		/**< MN => Server, 전구간에서 데이터 사이즈 (전송후 Reset) */
	U32			uiIPTotDnPktSize;		/**< Server => MN, 전구간에서 데이터 사이즈 (전송후 Reset) */
	U32			uiIPDataUpRetransSize;	/**< MN => Server, 데이터 전송 단계에서 재전송 데이터 사이즈 (전송후 Reset) */
	U32			uiIPDataDnRetransSize;	/**< Server => MN, 데이터 전송 단계에서 재전송 데이터 사이즈 (전송후 Reset) */
	U32			uiIPTotUpRetransSize;	/**< MN => Server, 전구간에서 재전송 데이터 사이즈 (전송후 Reset) */
	U32			uiIPTotDnRetransSize;	/**< Server => MN, 전구간에서 재전송 데이터 사이즈 (전송후 Reset) */
} st_MSRPM_TSESS, *pst_MSRPM_TSESS;
#define DEF_MSRPMTSESS_SIZE				sizeof(st_MSRPM_TSESS)


extern S32 dCloseMSRPM(stMEMSINFO *pstMEMSINFO, st_MSRPM_TSESS_KEY *pstMSRPMTSESSKEY, st_MSRPM_TSESS *pstMSRPTSESS, st_MSRPM_MSG_INFO *pstMSGINFO, U8 ucRtx, U16 usFailCode);
extern S32 dCloseMSRPMTSESS(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstMSRPMHASH, st_MSRPM_TSESS_KEY *pstMSRPMTSESSKEY, st_MSRPM_TSESS *pstMSRPMTSESS);
extern S32 dFlowMSRPM(stMEMSINFO *pstMEMSINFO, st_MSRPM_MSG_INFO *pstMSGINFO, U8 *szInData, S32 dDataSize, S32 *pdLen);
extern S32 dProcMSRPM(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstMSRPMHASH, TCP_INFO *pstTCPINFO, U8 *pDATA);
#endif /* _MSRPM_FUNC_H_ */
