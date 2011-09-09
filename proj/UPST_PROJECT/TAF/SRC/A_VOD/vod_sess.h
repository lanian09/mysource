#ifndef _VOD_SESS_H_
#define _VOD_SESS_H_

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"
#include "capdef.h"

//LIB
#include "Analyze_Ext_Abs.h"

/**
 *	Define cons.
 */
#define DEF_VODSESS_CNT			20011
#define DEF_RTCPSESS_CNT		40011

#define DEF_SETUP				0x02
#define DEF_SETUP_2				0x04

#define DEF_DESCRIBE			0x01
#define DEF_PLAY				0x08
#define DEF_PAUSE				0x10
#define DEF_PAUSE_PLAY			0x20
#define DEF_OPT					0x40
#define DEF_TEARDOWN			0x80

#define DEF_MEDIA_VIDEO			1
#define DEF_MEDIA_AUDIO			2

#define DEF_SR					200
#define DEF_RR					201
#define DEF_BILL				204

#define DEF_USERERR_NOSETUP		1
#define DEF_USERERR_NOTEARDOWN	2
#define DEF_USERERR_NOPLAY		3

/**
 *	Define structures
 */
typedef struct _st_VODSESSKEY_
{
	UINT		uiSrcIP;
	USHORT		usSrcPort;
} st_VODSESSKEY, *pst_VODSESSKEY;
#define DEF_VODSESSKEY_SIZE     sizeof(st_VODSESSKEY)

typedef struct _st_RTCPSESSKEY_
{
    UINT        uiSrcIP;
    USHORT      usSrcPort;
} st_RTCPSESSKEY, *pst_RTCPSESSKEY;
#define DEF_RTCPSESSKEY_SIZE	sizeof(st_RTCPSESSKEY)

typedef struct _st_VODSESS_
{
	int				dATrackID;
	int				dVTrackID;
	st_RTCPSESSKEY	stAudioKEY;
	st_RTCPSESSKEY	stVideoKEY;

	st_VODSESSKEY	stVODSESSKEY;
	LOG_VOD_SESS	stVODSESS;
} st_VODSESS, *pst_VODSESS;
#define DEF_VODSESS_SIZE        sizeof(st_VODSESS)

typedef struct _st_RTCPSESS_
{
	UINT			uiMediaType;
	UINT			uiTrackID;
	UINT			uiOldTimestamp;
	UINT			uiIntervalJitter;
	UINT			uiOldSequence;
	UINT			uiUpLossCnt;
	UINT			uiDnLossCnt;
	struct timeval	tvOldTime;
	st_VODSESSKEY	stVODSESSKEY;
	st_RTCPSESSKEY	stRTCPSESSKEY;
} st_RTCPSESS, *pst_RTCPSESS;
#define DEF_RTCPSESS_SIZE		sizeof(st_RTCPSESS)

typedef struct _st_RTCP_COMM_
{
	UCHAR	ucVerCnt;
	UCHAR	ucType;
	USHORT	usLength;
	UINT	uiSSRC;
} st_RTCP_COMM, *pst_RTCP_COMM;
#define DEF_RTCP_COMM_SIZE		sizeof( st_RTCP_COMM )

typedef struct _st_RTCP_SR_
{
	INT64	llNTPTime;
	UINT	uiRTPTime;
	UINT	uiSendPktCnt;
	UINT	uiSendOctCnt;
} st_RTCP_SR, *pst_RTCP_SR;

typedef struct _st_RTCP_RR_
{
	UINT	uiSSRC;
	UINT	uiLostInfo;
	UINT	uiSeqNum;
	UINT	uiJitter;
	UINT	uiLSR;
	UINT	uiDLSR;
} st_RTCP_RR, *pst_RTCP_RR;

typedef struct _st_RTCP_BILL_
{
	UINT	uiSSRC;
	UINT	uiRcvDataSize;
	UCHAR	szBillStr[5];
} st_RTCP_BILL, *pst_RTCP_BILL;

/**
 *	Declare func.
 */
extern int dStart_Session( TCP_INFO *pstTCPINFO );
extern int dEnd_Session( TCP_INFO *pstTCPINFO );
extern void ClearSession( pst_VODSESS pstVODSess );
extern stHASHONODE *pCheck_Session( LOG_HTTP_TRANS *pstHTTPLOG );
extern int dProc_VODSESS( LOG_HTTP_TRANS *pstHTTPLOG, stHASHONODE *pstVODSessNode, char *pReqHdr, char *pRespHdr, char *pRespBody, int dReqHdrLen, int dRespHdrLen, int dRespBodyLen );
extern int dRTCP_Session( OFFSET dOffset, INFO_ETH *pstINFOETH, Capture_Header_Msg *pstCAPHEAD );
extern void PrintVODSessLog( pst_VODSESSKEY pstKey, LOG_VOD_SESS *pstVODSess );
extern int dRtpQosJitter(UINT uiTimestamp, pst_RTCPSESS pstRTCPSessData, pst_VODSESS pstVODSessData, Capture_Header_Msg *pstCAPHEAD);
extern int dRtpQosLoss(UINT uiSequence, pst_RTCPSESS pstRTCPSessData, pst_VODSESS pstVODSessData, Capture_Header_Msg *pstCAPHEAD);

#endif /* _VOD_SESS_H_ */
