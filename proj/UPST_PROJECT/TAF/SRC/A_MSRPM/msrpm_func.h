#ifndef _MSRPM_FUNC_H_
#define _MSRPM_FUNC_H_

#include "typedef.h"
#include "common_stg.h"	/* MSRP_TID_SIZE */
#include "mems.h"
#include "hasho.h"

/* st_MSRPM_MSG_INFO�� dStatus */
#define MSRPM_STATUS_WAIT		0
#define MSRPM_STATUS_START		1

/* st_MSRPM_MSG_INFO�� dEndStatus */
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
	U32			uiNextSeq;			/**< ������ ���� ������ ����Ǵ� Seq No. */
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
 * @brief st_MSRPM_TSESS_KEY : A_MSRPM���� TCP Session ������ Hash Key ����ü
 *
 *
 * @see msrpm_api.h
 *
 * @note   Nothing
 *
 * @todo   Nothing
 */
typedef struct _st_MSRPM_TSESS_KEY {
	U32				uiCliIP;				/**< �ܸ� IP Address */
	U32				uiSrvIP;				/**< ���� IP Address */
	U16				usCliPort;				/**< �ܸ� Port */
	U16				usSrvPort;				/**< ���� Port */
	S32				dReserved;
} st_MSRPM_TSESS_KEY, *pst_MSRPM_TSESS_KEY;
#define DEF_MSRPMTSESSKEY_SIZE	sizeof(st_MSRPM_TSESS_KEY)

/**
 * @brief st_MSRPM_TSESS : A_MSRPM���� TCP Session ������ Hash Data ����ü
 *
 *
 * @see msrpm_api.h
 *
 * @note   Nothing
 *
 * @todo   Nothing
 */
typedef struct _st_MSRPM_TSESS {
	STIME		uiTcpSynTime;			/**< TCP Session ���� �ð� */
	MTIME		uiTcpSynMTime;		/**< TCP Session ���� �ð� */
	U8			ucSynRtx;				/**< SYN UP/Down ���� 1: UP, 2: Down */

	DEF			dL4FailCode;
	U16			usAppCode;
	U16			usL4Code;
	U16			usL7Code;

	U8			ucTcpClientStatus;
	U8			ucTcpServerStatus;

	S32			dRange;
	S32			dNetwork;

	st_MSRPM_MSG_INFO	stMSRPMMSGINFO[MSRPM_MSG_INFO_CNT];

	U32			uiIPDataUpPktCnt;		/**< MN => Server, ������ ���� �ܰ迡�� ��Ŷ ���� (������ Reset) */
	U32			uiIPDataDnPktCnt;		/**< Server => MN, ������ ���� �ܰ迡�� ��Ŷ ���� (������ Reset) */
	U32			uiIPTotUpPktCnt;		/**< MN => Server, ���������� ��Ŷ ���� (������ Reset) */
	U32			uiIPTotDnPktCnt;		/**< Server => MN, ���������� ��Ŷ ���� (������ Reset) */
	U32			uiIPDataUpRetransCnt;	/**< MN => Server, ������ ���� �ܰ迡�� ������ ��Ŷ ���� (������ Reset) */
	U32			uiIPDataDnRetransCnt;	/**< Server => MN, ������ ���� �ܰ迡�� ������ ��Ŷ ���� (������ Reset) */
	U32			uiIPTotUpRetransCnt;	/**< MN => Server, ���������� ������ ��Ŷ ���� (������ Reset) */
	U32			uiIPTotDnRetransCnt;	/**< Server => MN, ���������� ������ ��Ŷ ���� (������ Reset) */
	U32			uiIPDataUpPktSize;		/**< MN => Server, ������ ���� �ܰ迡�� ������ ������ (������ Reset) */
	U32			uiIPDataDnPktSize;		/**< Server => MN, ������ ���� �ܰ迡�� ������ ������ (������ Reset) */
	U32			uiIPTotUpPktSize;		/**< MN => Server, ���������� ������ ������ (������ Reset) */
	U32			uiIPTotDnPktSize;		/**< Server => MN, ���������� ������ ������ (������ Reset) */
	U32			uiIPDataUpRetransSize;	/**< MN => Server, ������ ���� �ܰ迡�� ������ ������ ������ (������ Reset) */
	U32			uiIPDataDnRetransSize;	/**< Server => MN, ������ ���� �ܰ迡�� ������ ������ ������ (������ Reset) */
	U32			uiIPTotUpRetransSize;	/**< MN => Server, ���������� ������ ������ ������ (������ Reset) */
	U32			uiIPTotDnRetransSize;	/**< Server => MN, ���������� ������ ������ ������ (������ Reset) */
} st_MSRPM_TSESS, *pst_MSRPM_TSESS;
#define DEF_MSRPMTSESS_SIZE				sizeof(st_MSRPM_TSESS)


extern S32 dCloseMSRPM(stMEMSINFO *pstMEMSINFO, st_MSRPM_TSESS_KEY *pstMSRPMTSESSKEY, st_MSRPM_TSESS *pstMSRPTSESS, st_MSRPM_MSG_INFO *pstMSGINFO, U8 ucRtx, U16 usFailCode);
extern S32 dCloseMSRPMTSESS(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstMSRPMHASH, st_MSRPM_TSESS_KEY *pstMSRPMTSESSKEY, st_MSRPM_TSESS *pstMSRPMTSESS);
extern S32 dFlowMSRPM(stMEMSINFO *pstMEMSINFO, st_MSRPM_MSG_INFO *pstMSGINFO, U8 *szInData, S32 dDataSize, S32 *pdLen);
extern S32 dProcMSRPM(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstMSRPMHASH, TCP_INFO *pstTCPINFO, U8 *pDATA);
#endif /* _MSRPM_FUNC_H_ */
