#ifndef _SIPM_FUNC_H_
#define _SIPM_FUNC_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"
#include "hasho.h"

/**
 *	Define constants
 */
#define SIP_UERR_9300				9300

/* st_SIPM_MSG_INFO�� dStatus */
#define SIPM_STATUS_HDRWAIT			0
#define SIPM_STATUS_HDRSTART		1
#define SIPM_STATUS_BODYWAIT		2
#define SIPM_STATUS_BODYSTART		3

/* st_SIPM_MSG_INFO�� dEndStatus */
#define SIPM_ENDSTATUS_EMPTY		0
#define SIPM_ENDSTATUS_0D			1
#define SIPM_ENDSTATUS_0D0A			2
#define SIPM_ENDSTATUS_0D0A0D		3
#define SIPM_ENDSTATUS_0D0A0D0A		4
#define SIPM_ENDSTATUS_BODY			5
#define SIPM_ENDSTATUS_END			6

#define SIPM_MSG_INFO_CNT			2

/**
 *	Define structures
 */

// A_SIPM���� TCP Session ������ Hash Key ����ü
typedef struct _st_SIPM_TSESS_KEY {
	U32				uiCliIP;				/**< �ܸ� IP Address */
	U32				uiSrvIP;				/**< ���� IP Address */
	U16				usCliPort;				/**< �ܸ� Port */
	U16				usSrvPort;				/**< ���� Port */
	S32				dReserved;
} st_SIPM_TSESS_KEY, *pst_SIPM_TSESS_KEY;
#define DEF_SIPMTSESSKEY_SIZE	sizeof(st_SIPM_TSESS_KEY)

typedef struct _st_SIPM_MSG_INFO {
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
	U32			uiTotalLen;
	U32			uiHdrLen;
	U32			uiBodyLen;
	U32			uiContentLen;
	U32			uiIPDataSize;
	OFFSET		offset_DATA;
	OFFSET		offset_CurDATA;
} st_SIPM_MSG_INFO, *pst_SIPM_MSG_INFO;
#define DEF_SIPMMSGINFO_SIZE		sizeof(st_SIPM_MSG_INFO)

// A_SIPM���� TCP Session ������ Hash Data ����ü
typedef struct _st_SIPM_TSESS {
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

	st_SIPM_MSG_INFO	stSIPMMSGINFO[SIPM_MSG_INFO_CNT];

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
} st_SIPM_TSESS, *pst_SIPM_TSESS;
#define DEF_SIPMTSESS_SIZE				sizeof(st_SIPM_TSESS)

/**
 *	Declare functions
 */
extern S32 dProcSIPM(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstSIPMHASH, TCP_INFO *pstTCPINFO, U8 *pDATA);
extern S32 dCloseSIPM(stMEMSINFO *pstMEMSINFO, st_SIPM_TSESS_KEY *pstSIPMTSESSKEY, st_SIPM_TSESS *pstSIPMTSESS, st_SIPM_MSG_INFO *pstMSGINFO, U8 ucRtx, U16 usFailCode);
extern S32 dCloseSIPMTSESS(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstSIPMHASH, st_SIPM_TSESS_KEY *pstSIPMTSESSKEY, st_SIPM_TSESS *pstSIPMTSESS);
extern S32 dGetHDR(stMEMSINFO *pstMEMSINFO, st_SIPM_MSG_INFO *pstMSGINFO, U8 *szInData, S32 dDataSize, S32 *pdLen);
extern S32 dGetBODY(stMEMSINFO *pstMEMSINFO, st_SIPM_MSG_INFO *pstMSGINFO, U8 *szInData, S32 dDataSize, S32 *pdLen);

#endif	/* _SIPM_FUNC_H_ */

