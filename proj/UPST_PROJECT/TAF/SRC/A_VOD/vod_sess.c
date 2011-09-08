/*******************************************************************************
 *      @file   vod_sess.c
 *      - HTTP Transaction을 관리 하는 프로세스
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: vod_sess.c,v 1.2 2011/09/06 12:46:40 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 12:46:40 $
 *      @warning    .
 *      @ref        http_main.c l4.h http_init.c http_func.c
 *      @todo       library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 *      @section    Intro(소개)
 *      - HTTP Transaction을 관리 하는 프로세스
 *
 *      @section    Requirement
 *       @li library 생성 이후 함수 대치
 *
*******************************************************************************/

/**
 *	Include headers
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// TOP
#include "procid.h"

// LIB
#include "loglib.h"
#include "Analyze_Ext_Abs.h"

// TAF
#include "rtsp.h"

// .
#include "vod_sess.h"


/**
 *	Declare var.
 */
S32                 giFinishSignal;     /**< Finish Signal */
S32                 giStopFlag;         /**< main loop Flag 0: Stop, 1: Loop */

extern stMEMSINFO   *pMEMSINFO;         /**< new interface 관리 구조체 */
extern stHASHOINFO  *pstVODSESSHASH;    /* 2007.08.23 LDH : VOD STREAMING SESSION */
extern stHASHOINFO  *pstRTCPSESSHASH;

/**
 *	Declare func.
 */
extern S32 dSend_VOD_Data(stMEMSINFO *pMEMSINFO, S32 dSndMsgQ, U8 *pNode);

/**
 *	Implement func.
 */

/*******************************************************************************

*******************************************************************************/
int dStart_Session( TCP_INFO *pstTCPINFO )
{
    st_VODSESSKEY   stSessKey;
    st_VODSESS      stSessData;
    pst_VODSESS     pstSessData;
    stHASHONODE     *pstVODSess;
	struct timeval	stNowTime;

	
	log_print( LOGN_INFO, "[VOD_SESSION] NEW START MSG IP:%u:%u", pstTCPINFO->uiCliIP, pstTCPINFO->usCliPort );

    stSessKey.uiSrcIP       = pstTCPINFO->uiCliIP;
    stSessKey.usSrcPort     = pstTCPINFO->usCliPort;

    /* ADD SESSION */
	memset( &stSessData, 0x00, DEF_VODSESS_SIZE );
	pstVODSess = hasho_add( pstVODSESSHASH, (UCHAR *)&stSessKey, (UCHAR *)&stSessData );
    if( pstVODSess == NULL ) {
        log_print( LOGN_CRI, LH"[FAIL] hasho_add IP:%u:%u", LT, stSessKey.uiSrcIP, stSessKey.usSrcPort );
        return -1;
    }
	else
		log_print( LOGN_DEBUG, "NEW VOD SESSION CREATED IP:%u:%u CNT:%u",
                                stSessKey.uiSrcIP, stSessKey.usSrcPort, hasho_get_occupied_node_count( pstVODSESSHASH) );

    /* SET SESSION INFORMATION */
    pstSessData = (pst_VODSESS)nifo_ptr( pstVODSESSHASH, pstVODSess->offset_Data);

	memcpy( &pstSessData->stVODSESSKEY, &stSessKey, DEF_VODSESSKEY_SIZE );

	gettimeofday( &stNowTime, NULL );

    pstSessData->stVODSESS.uiOpStartTime    = stNowTime.tv_sec;
    pstSessData->stVODSESS.uiOpStartMTime   = stNowTime.tv_usec;

	/* SET DATA TRAFFIC SIZE */
    pstSessData->stVODSESS.uiIPDataUpPktCnt         += pstTCPINFO->uiIPDataUpPktCnt;
    pstSessData->stVODSESS.uiIPDataDnPktCnt         += pstTCPINFO->uiIPDataDnPktCnt;
    pstSessData->stVODSESS.uiIPTotUpPktCnt          += pstTCPINFO->uiIPTotUpPktCnt;
    pstSessData->stVODSESS.uiIPTotDnPktCnt          += pstTCPINFO->uiIPTotDnPktCnt;
    pstSessData->stVODSESS.uiIPDataUpRetransCnt     += pstTCPINFO->uiIPDataUpRetransCnt;
    pstSessData->stVODSESS.uiIPDataDnRetransCnt     += pstTCPINFO->uiIPDataDnRetransCnt;
    pstSessData->stVODSESS.uiIPTotUpRetransCnt      += pstTCPINFO->uiIPTotUpRetransCnt;
    pstSessData->stVODSESS.uiIPTotDnRetransCnt      += pstTCPINFO->uiIPTotDnRetransCnt;
    pstSessData->stVODSESS.uiIPDataUpPktSize        += pstTCPINFO->uiIPDataUpPktSize;
    pstSessData->stVODSESS.uiIPDataDnPktSize        += pstTCPINFO->uiIPDataDnPktSize;
    pstSessData->stVODSESS.uiIPTotUpPktSize         += pstTCPINFO->uiIPTotUpPktSize;
    pstSessData->stVODSESS.uiIPTotDnPktSize         += pstTCPINFO->uiIPTotDnPktSize;
    pstSessData->stVODSESS.uiIPDataUpRetransSize    += pstTCPINFO->uiIPDataUpRetransSize;
    pstSessData->stVODSESS.uiIPDataDnRetransSize    += pstTCPINFO->uiIPDataDnRetransSize;
    pstSessData->stVODSESS.uiIPTotUpRetransSize     += pstTCPINFO->uiIPTotUpRetransSize;
    pstSessData->stVODSESS.uiIPTotDnRetransSize     += pstTCPINFO->uiIPTotDnRetransSize;

    return 1;
}


/*******************************************************************************

*******************************************************************************/
int dEnd_Session( TCP_INFO *pstTCPINFO )
{
	int				dRet;
    st_VODSESSKEY   stSessKey;
    pst_VODSESS     pstSessData;
    stHASHONODE     *pstVODSessNode;
	struct timeval  stNowTime;

	OFFSET			offset_tmp;

	UCHAR			*pucSessLog, *pucBuffer;


	log_print( LOGN_INFO, "[VOD_SESSION] NEW END   MSG IP:%u:%u", pstTCPINFO->uiCliIP, pstTCPINFO->usCliPort ); 

    stSessKey.uiSrcIP       = pstTCPINFO->uiCliIP;
    stSessKey.usSrcPort     = pstTCPINFO->usCliPort;

    /* FIND SESSION */
    pstVODSessNode = hasho_find( pstVODSESSHASH, (UCHAR *)&stSessKey );
    if( pstVODSessNode == NULL ) {
        log_print( LOGN_WARN, "NO VOD SESSION IP:%u:%u IN End_Session",
                           stSessKey.uiSrcIP, stSessKey.usSrcPort );
        return -1;
    }

    pstSessData = (pst_VODSESS)nifo_ptr( pstVODSESSHASH, pstVODSessNode->offset_Data);

	/* CHECK NO RTSP MESSAGE */
	if( pstSessData->stVODSESS.uiCallTime == 0 ) {
		log_print( LOGN_DEBUG, "NO RTSP MESSAGE!!" );

		ClearSession( pstSessData );

		return -2;
	}

	/* SET DATA TRAFFIC SIZE */
    pstSessData->stVODSESS.uiIPDataUpPktCnt         += pstTCPINFO->uiIPDataUpPktCnt;
    pstSessData->stVODSESS.uiIPDataDnPktCnt         += pstTCPINFO->uiIPDataDnPktCnt;
    pstSessData->stVODSESS.uiIPTotUpPktCnt          += pstTCPINFO->uiIPTotUpPktCnt;
    pstSessData->stVODSESS.uiIPTotDnPktCnt          += pstTCPINFO->uiIPTotDnPktCnt;
    pstSessData->stVODSESS.uiIPDataUpRetransCnt     += pstTCPINFO->uiIPDataUpRetransCnt;
    pstSessData->stVODSESS.uiIPDataDnRetransCnt     += pstTCPINFO->uiIPDataDnRetransCnt;
    pstSessData->stVODSESS.uiIPTotUpRetransCnt      += pstTCPINFO->uiIPTotUpRetransCnt;
    pstSessData->stVODSESS.uiIPTotDnRetransCnt      += pstTCPINFO->uiIPTotDnRetransCnt;
    pstSessData->stVODSESS.uiIPDataUpPktSize        += pstTCPINFO->uiIPDataUpPktSize;
    pstSessData->stVODSESS.uiIPDataDnPktSize        += pstTCPINFO->uiIPDataDnPktSize;
    pstSessData->stVODSESS.uiIPTotUpPktSize         += pstTCPINFO->uiIPTotUpPktSize;
    pstSessData->stVODSESS.uiIPTotDnPktSize         += pstTCPINFO->uiIPTotDnPktSize;
    pstSessData->stVODSESS.uiIPDataUpRetransSize    += pstTCPINFO->uiIPDataUpRetransSize;
    pstSessData->stVODSESS.uiIPDataDnRetransSize    += pstTCPINFO->uiIPDataDnRetransSize;
    pstSessData->stVODSESS.uiIPTotUpRetransSize     += pstTCPINFO->uiIPTotUpRetransSize;
    pstSessData->stVODSESS.uiIPTotDnRetransSize     += pstTCPINFO->uiIPTotDnRetransSize;


    /* CREATE SESSION LOG FOR END */
	gettimeofday( &stNowTime, NULL );
    pstSessData->stVODSESS.uiOpEndTime  = stNowTime.tv_sec;
    pstSessData->stVODSESS.uiOpEndMTime = stNowTime.tv_usec;

	pstSessData->stVODSESS.uiTrafficSize = pstSessData->stVODSESS.uiAudioDataSize + 
										   pstSessData->stVODSESS.uiVideoDataSize;
	if( pstSessData->dATrackID > 0 )
		pstSessData->stVODSESS.usMediaCnt++;
	
	if( pstSessData->dVTrackID > 0 )
		pstSessData->stVODSESS.usMediaCnt++;


	if( (pstSessData->stVODSESS.ucStatus & DEF_SETUP) == 0 && (pstSessData->stVODSESS.ucStatus & DEF_SETUP_2) == 0 )
		pstSessData->stVODSESS.usUserErrorCode = DEF_USERERR_NOSETUP;
	else if( (pstSessData->stVODSESS.ucStatus & DEF_PLAY) == 0 )
        pstSessData->stVODSESS.usUserErrorCode = DEF_USERERR_NOPLAY;
	else if( (pstSessData->stVODSESS.ucStatus & DEF_TEARDOWN) == 0 )
		pstSessData->stVODSESS.usUserErrorCode = DEF_USERERR_NOTEARDOWN;

	STG_DiffTIME64( pstSessData->stVODSESS.uiLastPktTime, pstSessData->stVODSESS.uiLastPktMTime, 
					pstSessData->stVODSESS.uiSetupStartTime, pstSessData->stVODSESS.uiSetupStartMTime,
					&pstSessData->stVODSESS.llSessGapTime );


	PrintVODSessLog( &stSessKey, &pstSessData->stVODSESS );

    /* SEND SESSION LOG */
	pucSessLog = nifo_node_alloc( pMEMSINFO );
	if( pucSessLog == NULL ) {
		log_print( LOGN_CRI, LH"FAIl IN nifo_alloc", LT );
		ClearSession( pstSessData );

		return -3;
	}

	offset_tmp = nifo_get_offset_node( pMEMSINFO, pucSessLog );
    log_print( LOGN_INFO, "NIFO_MEM_TEST NEW:%ld", offset_tmp );


	pucBuffer = nifo_tlv_alloc( pMEMSINFO, pucSessLog, LOG_VOD_SESS_DEF_NUM, LOG_VOD_SESS_SIZE, DEF_MEMSET_OFF );
	if( pucBuffer == NULL ) {
		log_print( LOGN_CRI, LH"FAIl IN nifo_tvl_alloc", LT);
		ClearSession( pstSessData );

		offset_tmp = nifo_get_offset_node( pMEMSINFO, pucSessLog );
        log_print( LOGN_INFO, "NIFO_MEM_TEST DEL :%ld", offset_tmp );

		nifo_node_delete( pMEMSINFO, pucSessLog );

		return -4;
	}

	memcpy( pucBuffer, &pstSessData->stVODSESS, LOG_VOD_SESS_SIZE );

	dRet = dSend_VOD_Data(pMEMSINFO, SEQ_PROC_A_CALL, pucSessLog);
	if( dRet < 0 ) {
		log_print( LOGN_CRI, LH"FAIL IN dSend_VOD_Data", LT );
		ClearSession( pstSessData );

		offset_tmp = nifo_get_offset_node( pMEMSINFO, pucSessLog );
        log_print( LOGN_INFO, "NIFO_MEM_TEST DEL :%ld", offset_tmp );

		nifo_node_delete( pMEMSINFO, pucSessLog );

		return -5;
	}

    /* DELETE RTCP SESSION & VOD SESSION */
	ClearSession( pstSessData );

    return 0;
}

/*******************************************************************************

*******************************************************************************/
void ClearSession( pst_VODSESS pstVODSess )
{
	if( pstVODSess->dATrackID > 0 )
		hasho_del( pstRTCPSESSHASH, (UCHAR *)&pstVODSess->stAudioKEY );

    if( pstVODSess->dVTrackID > 0 )
        hasho_del( pstRTCPSESSHASH, (UCHAR *)&pstVODSess->stVideoKEY );

    hasho_del( pstVODSESSHASH, (UCHAR *)&pstVODSess->stVODSESSKEY );
}
	 


/*******************************************************************************

*******************************************************************************/
stHASHONODE *pCheck_Session( LOG_HTTP_TRANS *pstHTTPLOG )
{
    pst_VODSESS     pstSessData;
    st_VODSESSKEY   stSessKey;
	stHASHONODE 	*pstVODSess;

    stSessKey.uiSrcIP       = pstHTTPLOG->uiClientIP;
    stSessKey.usSrcPort     = pstHTTPLOG->usClientPort;

    /* FIND SESSION */
    pstVODSess = hasho_find( pstVODSESSHASH, (UCHAR *)&stSessKey );
    if( pstVODSess == NULL ) {
        log_print( LOGN_WARN, "NO VOD SESSION IP:%u:%u METHOD:%lu",
                           stSessKey.uiSrcIP, stSessKey.usSrcPort, pstHTTPLOG->ucMethod );
        return NULL;
    }

    pstSessData = (pst_VODSESS)nifo_ptr( pstVODSESSHASH, pstVODSess->offset_Data);

    /* UPDATE SESSION INFO */
    pstSessData->stVODSESS.uiLastPktTime    = pstHTTPLOG->uiLastPktTime;
    pstSessData->stVODSESS.uiLastPktMTime   = pstHTTPLOG->uiLastPktMTime;

    return pstVODSess;
}


/*******************************************************************************

*******************************************************************************/
int dProc_VODSESS( LOG_HTTP_TRANS *pstHTTPLOG, stHASHONODE *pstVODSessNode, 
				   char *pReqHdr, char *pRespHdr, char *pRespBody, int dReqHdrLen, int dRespHdrLen, int dRespBodyLen )
{
	int					dRet;
	int                 dTrackID;
    int                 dMinRange1, dMinRange2, dMaxRange1, dMaxRange2;
    int                 dVTrackID, dATrackID;
    UINT                uiSessionID;
    USHORT              usRTI, usPort1, usPort2, usRTCPPort;
	UCHAR				szMsMAN[MAX_MSMAN_SIZE];
	INT64				llRange;

    pst_VODSESS     	pstSessData;
	
	st_RTCPSESSKEY		stRTCPKey;
	st_RTCPSESS			stRTCPSess;
	pst_RTCPSESS		pstRTCPSess;
	stHASHONODE     	*pstRTCPSessNode;


	log_print( LOGN_INFO, "[VOD_SESSION] ANALYSE RTSP INFO IP:%u:%u", pstHTTPLOG->uiClientIP, pstHTTPLOG->usClientPort );

	if((pstHTTPLOG->usSvcL4Type != L4_VOD_STREAM) && (pstHTTPLOG->usSvcL4Type != L4_MBOX)) {
		log_print( LOGN_INFO, "VOD DOWNLOAD!!" );
		return 0;
	}

    pstSessData = (pst_VODSESS)nifo_ptr( pstVODSESSHASH, pstVODSessNode->offset_Data);

	/* ADD COMMON */
	pstSessData->stVODSESS.uiCallTime	= pstHTTPLOG->uiCallTime;
	pstSessData->stVODSESS.uiCallMTime	= pstHTTPLOG->uiCallMTime;
	pstSessData->stVODSESS.uiClientIP	= pstHTTPLOG->uiClientIP;
//	pstSessData->stVODSESS.uiNASName	= pstHTTPLOG->uiNASName;
	pstSessData->stVODSESS.uiNASName	= 0;
	pstSessData->stVODSESS.usServiceType	= pstHTTPLOG->usServiceType;
	memcpy( pstSessData->stVODSESS.szHostName, pstHTTPLOG->szHostName, MAX_HOSTNAME_SIZE );
	memcpy( pstSessData->stVODSESS.szBrowserInfo, pstHTTPLOG->szBrowserInfo, MAX_BROWSERINFO_SIZE );
	memcpy( pstSessData->stVODSESS.szModel, pstHTTPLOG->szModel, MAX_MODEL_SIZE );
	memcpy( pstSessData->stVODSESS.szNetOption, pstHTTPLOG->szNetOption, MAX_SVCOPTION_SIZE );
	memcpy( pstSessData->stVODSESS.szMIN, pstHTTPLOG->szMIN, MAX_MIN_SIZE );

    /* LAST METHOD UPDATE */
    pstSessData->stVODSESS.ucLastMethod = pstHTTPLOG->ucMethod;

	/* UPDATE INFO */
	pstSessData->stVODSESS.usClientPort				= pstHTTPLOG->usClientPort;
	pstSessData->stVODSESS.uiServerIP				= pstHTTPLOG->uiServerIP;
	pstSessData->stVODSESS.usServerPort				= pstHTTPLOG->usServerPort;
	pstSessData->stVODSESS.uiTcpSynTime				= pstHTTPLOG->uiTcpSynTime;
	pstSessData->stVODSESS.uiTcpSynMTime			= pstHTTPLOG->uiTcpSynMTime;
	pstSessData->stVODSESS.usPlatformType			= pstHTTPLOG->usPlatformType;
	pstSessData->stVODSESS.usSvcL4Type				= pstHTTPLOG->usSvcL4Type;
	pstSessData->stVODSESS.usSvcL7Type				= pstHTTPLOG->usSvcL7Type;
	pstSessData->stVODSESS.ucSubSysNo				= pstHTTPLOG->ucSubSysNo;

	pstSessData->stVODSESS.ucTcpClientStatus		= pstHTTPLOG->ucTcpClientStatus;
	pstSessData->stVODSESS.ucTcpServerStatus		= pstHTTPLOG->ucTcpServerStatus;

	pstSessData->stVODSESS.usL4FailCode				= pstHTTPLOG->usL4FailCode;
	pstSessData->stVODSESS.usL7FailCode				= pstHTTPLOG->usL7FailCode;

	/* UPDATE TRAFFIC SIZE */
	pstSessData->stVODSESS.uiIPDataUpPktCnt			+= pstHTTPLOG->uiIPDataUpPktCnt;
	pstSessData->stVODSESS.uiIPDataDnPktCnt			+= pstHTTPLOG->uiIPDataDnPktCnt;
	pstSessData->stVODSESS.uiIPTotUpPktCnt			+= pstHTTPLOG->uiIPTotUpPktCnt;
	pstSessData->stVODSESS.uiIPTotDnPktCnt			+= pstHTTPLOG->uiIPTotDnPktCnt;
	pstSessData->stVODSESS.uiIPDataUpRetransCnt		+= pstHTTPLOG->uiIPDataUpRetransCnt;
	pstSessData->stVODSESS.uiIPDataDnRetransCnt		+= pstHTTPLOG->uiIPDataDnRetransCnt;
	pstSessData->stVODSESS.uiIPTotUpRetransCnt		+= pstHTTPLOG->uiIPTotUpRetransCnt;
	pstSessData->stVODSESS.uiIPTotDnRetransCnt		+= pstHTTPLOG->uiIPTotDnRetransCnt;
	pstSessData->stVODSESS.uiIPDataUpPktSize		+= pstHTTPLOG->uiIPDataUpPktSize;
	pstSessData->stVODSESS.uiIPDataDnPktSize		+= pstHTTPLOG->uiIPDataDnPktSize;
    pstSessData->stVODSESS.uiIPTotUpPktSize			+= pstHTTPLOG->uiIPTotUpPktSize;
    pstSessData->stVODSESS.uiIPTotDnPktSize			+= pstHTTPLOG->uiIPTotDnPktSize;
    pstSessData->stVODSESS.uiIPDataUpRetransSize	+= pstHTTPLOG->uiIPDataUpRetransSize;
    pstSessData->stVODSESS.uiIPDataDnRetransSize	+= pstHTTPLOG->uiIPDataDnRetransSize;
    pstSessData->stVODSESS.uiIPTotUpRetransSize		+= pstHTTPLOG->uiIPTotUpRetransSize;
    pstSessData->stVODSESS.uiIPTotDnRetransSize		+= pstHTTPLOG->uiIPTotDnRetransSize;
    pstSessData->stVODSESS.uiTcpUpBodySize			+= pstHTTPLOG->uiTcpUpBodySize;
    pstSessData->stVODSESS.uiTcpDnBodySize			+= pstHTTPLOG->uiTcpDnBodySize;

	/* COUNT RTSP UP/DN */
#if 0
	if( pstCAPHEAD->bRtxType == DEF_FROM_SERVER )
        pstSessData->stVODSESS.usRtspDnCnt++;
    else if( pstCAPHEAD->bRtxType == DEF_FROM_CLIENT )
        pstSessData->stVODSESS.usRtspUpCnt++;
#endif

	/* CHECK FOR FB, WV 2008.06.18 BY LDH */
	if( pstHTTPLOG->usURLSize > 0 ) {
		if( strstr( (char*)pstHTTPLOG->szURL, "_fb" ) != NULL ) {
			/* CHANGE pstSessData->stVODSESS.usSvcL4Type 4200 */
			pstSessData->stVODSESS.usSvcL4Type = L4_RTS_FB;
		}
		else if( strstr( (char*)pstHTTPLOG->szURL, "_wv" ) != NULL ) {
			/* CHANGE pstSessData->stVODSESS.usSvcL4Type 4300 */
			pstSessData->stVODSESS.usSvcL4Type = L4_RTS_WB;
		}
	}

    switch( pstHTTPLOG->ucMethod )
    {
    case METHOD_DESCRIBE:
        /* INFORMATION 
        - FLOW SATTUS
        - RTI, MEDIA COUNT, TIME RANGE
        - URL
        - ETC TIME INFO
        */
		/* SET STATUS */
		pstSessData->stVODSESS.ucStatus = (pstSessData->stVODSESS.ucStatus | DEF_DESCRIBE);

		log_print( LOGN_INFO, "[VOD_SESSION] DESCRIBE STATUS %02lu", pstSessData->stVODSESS.ucStatus );

		szMsMAN[0] = 0x00;
		dRet = rtsp_reqhdr( pReqHdr, dReqHdrLen, (char*)szMsMAN ); 
		if( dRet < 0 ) 
			log_print( LOGN_WARN, "FAIL IN rtsp_reqhdr dRet:%d IN DESCRIBE", dRet );

		/* SET MS MAN */
		memcpy( pstSessData->stVODSESS.szMsMan, szMsMAN, MAX_MSMAN_SIZE );

		dVTrackID = dATrackID = -1;
		dMinRange1 = dMinRange2 = dMaxRange1 = dMaxRange2 = 0;

		if( pRespBody != NULL ) {
			dRet = rtsp_resbody( pRespBody, dRespBodyLen, &dMinRange1, &dMinRange2, &dMaxRange1, &dMaxRange2, &dVTrackID, &dATrackID );
        	if( dRet < 0 ) {
            	log_print( LOGN_WARN, "FAIL IN rtsp_resbody dRet:%d IN DESCRIBE", dRet );
			} else {
				log_print( LOGN_INFO, "[VOD_SESSION] DESCRIBE RANGE:%10u.%06u %10u.%06u VTID:%d ATID:%d",
							   dMinRange1, dMinRange2, dMaxRange1, dMaxRange2, dVTrackID, dATrackID );
			}
		}

		/* SET RANGE */
		STG_DiffTIME64( dMaxRange1, dMaxRange2, dMinRange1, dMinRange2, &llRange );
		pstSessData->stVODSESS.uiTimeRange 	= (UINT)(llRange/1000000);
		pstSessData->stVODSESS.uiTimeMRange	= (llRange - pstSessData->stVODSESS.uiTimeRange*1000000);

		/* SET TRACK ID INFO */
//		if( dVTrackID >= 0 )
			pstSessData->dVTrackID = dVTrackID;

//		if( dATrackID >= 0 )
			pstSessData->dATrackID = dATrackID;

		/* SET RANGE */

        if( pRespHdr != NULL ) {
			/* FOR RTI */

			uiSessionID = 0;
			usRTI = 0;
			usPort1 = usPort2 = 0;

        	dRet = rtsp_reshdr( pRespHdr, dRespHdrLen, &uiSessionID, &usRTI, &usPort1, &usPort2 );
            if( dRet < 0 ) {
                log_print( LOGN_WARN, "FAIL IN rtsp_reshdr dRet:%d IN DESCRIBE", dRet );
			} else {
				log_print( LOGN_INFO, "[VOD_SESSION] DESCRIBE SESSID:%u RTI:%u PORT:%u %u", 
								   uiSessionID, usRTI, usPort1, usPort2 );

				/* SET RTI */
				pstSessData->stVODSESS.usRTI = usRTI;
			}
		}

		/* SETUP START TIME */
		pstSessData->stVODSESS.uiSetupStartTime = pstHTTPLOG->uiReqStartTime;
        pstSessData->stVODSESS.uiSetupStartMTime    = pstHTTPLOG->uiReqStartMTime;

        pstSessData->stVODSESS.usLOGURLSize = pstHTTPLOG->usURLSize;
        memcpy( pstSessData->stVODSESS.szLOGURL, pstHTTPLOG->szURL, pstSessData->stVODSESS.usLOGURLSize );
        pstSessData->stVODSESS.szLOGURL[pstSessData->stVODSESS.usLOGURLSize] = 0x00;

        break;
    case METHOD_SETUP:
        /* INFORMATION 
        - FLOW SATTUS
        - TRACK ID INFO VIDEO & AUDIO
        - ETC TIME INFO
        */
		dTrackID = 0;
		uiSessionID = 0;
		usRTI = 0;
		usPort1 = usPort2 = 0;
        dRet = rtsp_requrl( (char*)pstHTTPLOG->szLOGURL, pstHTTPLOG->usLOGURLSize, &dTrackID );
        if( dRet < 0 ) {
            log_print( LOGN_WARN, "FAIL IN rtsp_reqhdr dRet:%d", dRet );
		} else {
			log_print( LOGN_INFO, "[VOD_SESSION] SETUP TRACKID:%u", dTrackID );
		}

        if( pRespHdr != NULL ) {
            dRet = rtsp_reshdr( pRespHdr, dRespHdrLen, &uiSessionID, &usRTI, &usPort1, &usPort2 );
            if( dRet < 0 ) {
                log_print( LOGN_WARN, "FAIL IN rtsp_reshdr dRet:%d IN SETUP", dRet );
			} else {
				log_print( LOGN_INFO, "[VOD_SESSION] SETUP SESSID:%u RTI:%u PORT:%u %u",
                                   uiSessionID, usRTI, usPort1, usPort2 ); 

				/* CREATE RTCP SESSION FOR TRACKID & PORT */
				if( usPort2%2 == 1 )
					usRTCPPort = usPort2;
				else if( usPort1%2 == 1 )
					usRTCPPort = usPort1;
				else {
					log_print(LOGN_CRI, "INVALID PORT1=%u PORT2=%u", usPort1, usPort2);
				}

				stRTCPKey.uiSrcIP		= pstHTTPLOG->uiClientIP;
				stRTCPKey.usSrcPort		= usRTCPPort;

				memset( &stRTCPSess, 0x00, DEF_RTCPSESS_SIZE ); 

				pstRTCPSessNode = hasho_add( pstRTCPSESSHASH, (UCHAR *)&stRTCPKey, (UCHAR *)&stRTCPSess );
				if( pstRTCPSessNode == NULL ) {
					log_print( LOGN_CRI, LH"FAIL IN hasho_add RTCP FUN", LT );
					return -1;
				}
				else {
					log_print( LOGN_INFO, "[VOD_SESSION_RTCP] NEW SESSION START IP:%u:%u",
									   stRTCPKey.uiSrcIP, stRTCPKey.usSrcPort );	

					pstRTCPSess	= (pst_RTCPSESS)nifo_ptr( pstRTCPSESSHASH, pstRTCPSessNode->offset_Data);
					memcpy( &pstRTCPSess->stRTCPSESSKEY, &stRTCPKey, DEF_RTCPSESSKEY_SIZE );
				}

				/* SET VOD SESS KEY IN RTCP SESSION */
				memcpy( &pstRTCPSess->stVODSESSKEY, &pstSessData->stVODSESSKEY, DEF_VODSESSKEY_SIZE );

				/* SET TRACK ID & MEDIA TYPE IN RTCP SESSION */
				log_print( LOGN_INFO, "[VOD_SESSION] BEFORE ATID:%d VTID:%d TID:%d", 
						pstSessData->dATrackID, pstSessData->dVTrackID, dTrackID);

				if( pstSessData->dATrackID == dTrackID ) {
					pstRTCPSess->uiMediaType = DEF_MEDIA_AUDIO;
					memcpy( &pstSessData->stAudioKEY, &stRTCPKey, DEF_RTCPSESSKEY_SIZE );
				}
				else if( pstSessData->dVTrackID == dTrackID ) {
					pstRTCPSess->uiMediaType = DEF_MEDIA_VIDEO;
					memcpy( &pstSessData->stVideoKEY, &stRTCPKey, DEF_RTCPSESSKEY_SIZE );
				}
				else {
					if(pstSessData->dATrackID == -1) {
						pstRTCPSess->uiMediaType = DEF_MEDIA_AUDIO;
						pstSessData->dATrackID = dTrackID;
						memcpy( &pstSessData->stAudioKEY, &stRTCPKey, DEF_RTCPSESSKEY_SIZE );
					} 
					else if(pstSessData->dVTrackID == -1) {
						pstRTCPSess->uiMediaType = DEF_MEDIA_VIDEO;
						pstSessData->dVTrackID = dTrackID;
						memcpy( &pstSessData->stVideoKEY, &stRTCPKey, DEF_RTCPSESSKEY_SIZE );
					} 
					/* DESCRIBE의 Audio/Video TrackID 와 SETUP의 TrackID가 다를 경우 세션을 삭제한다. */
					else {
						hasho_del( pstRTCPSESSHASH, (UCHAR *)&stRTCPKey );
						log_print( LOGN_INFO, "[VOD_SESSION_RTCP] DEL SESSION IP:%u:%u ",
								stRTCPKey.uiSrcIP, stRTCPKey.usSrcPort );
					}
				}
				log_print( LOGN_INFO, "[VOD_SESSION] AFTER ATID:%d VTID:%d TID:%d", 
						pstSessData->dATrackID, pstSessData->dVTrackID, dTrackID);

				pstRTCPSess->uiTrackID = dTrackID;

				/* SESSION ID 확인 */
				sprintf( (char*)pstSessData->stVODSESS.szVodSessID, "%u", uiSessionID );

			}
		}

        if( (pstSessData->stVODSESS.ucStatus & DEF_SETUP) == DEF_SETUP ) {
			/* SECOND SETUP */
            pstSessData->stVODSESS.ucStatus += DEF_SETUP;
        }
        else {
			/* FIRST SETUP */
            pstSessData->stVODSESS.ucStatus = (pstSessData->stVODSESS.ucStatus | DEF_SETUP);
        }

		log_print( LOGN_INFO, "[VOD_SESSION] SETUP STATUS:%02lu", pstSessData->stVODSESS.ucStatus );

        break;
	case METHOD_PLAY:
        /* INFORMATION 
        - FLOW SATTUS
        - ETC TIME INFO
        */
        if( (pstSessData->stVODSESS.ucStatus & DEF_PAUSE) == DEF_PAUSE )
            pstSessData->stVODSESS.ucStatus = (pstSessData->stVODSESS.ucStatus | DEF_PAUSE_PLAY);
        else
            pstSessData->stVODSESS.ucStatus = (pstSessData->stVODSESS.ucStatus | DEF_PLAY);

		log_print( LOGN_INFO, "[VOD_SESSION] PLAY STATUS:%02lu", pstSessData->stVODSESS.ucStatus );

		/* SETUP END TIME */
		pstSessData->stVODSESS.uiSetupEndTime       = pstHTTPLOG->uiResEndTime;
        pstSessData->stVODSESS.uiSetupEndMTime  = pstHTTPLOG->uiResEndMTime;

        break;
    case METHOD_PAUSE:
        /* INFORMATION 
        - FLOW SATTUS
        - ETC TIME INFO
        */
		pstSessData->stVODSESS.ucStatus = (pstSessData->stVODSESS.ucStatus | DEF_PAUSE);
		log_print( LOGN_INFO, "[VOD_SESSION] PAUSE STATUS:%02lu", pstSessData->stVODSESS.ucStatus );

        break;
    case METHOD_ANNOUNCE:
        /* INFORMATION 
        - ETC TIME INFO
        */
		log_print( LOGN_INFO, "[VOD_SESSION] ANNOUNCE" );

        break;
    case METHOD_TEARDOWN:
        /* INFORMATION 
        - FLOW SATTUS
        - ETC TIME INFO
        */
		pstSessData->stVODSESS.ucStatus = (pstSessData->stVODSESS.ucStatus | DEF_TEARDOWN);
		log_print( LOGN_INFO, "[VOD_SESSION] TEARDOWN STATUS:%02lu", pstSessData->stVODSESS.ucStatus );

        pstSessData->stVODSESS.uiTeardownTime       = pstHTTPLOG->uiReqStartTime;
        pstSessData->stVODSESS.uiTeardownMTime  = pstHTTPLOG->uiReqStartMTime;

        break;
    default:

        break;
    }

    return 0;
}


/*******************************************************************************

*******************************************************************************/
int dRTCP_Session( OFFSET dOffset, INFO_ETH *pstINFOETH, Capture_Header_Msg *pstCAPHEAD )
{
	stHASHONODE         *pstVODSESSNode;
	pst_VODSESS         pstVODSessData;
	st_RTCPSESSKEY 		stRTCPSessKey;
	pst_RTCPSESS        pstRTCPSessData;
    stHASHONODE         *pstRTCPSessNode;

	U8                  *pucData;
    U8                  *pucRTCPData;

	pst_RTCP_COMM		pstRTCP_COMM;
	pst_RTCP_SR			pstRTCP_SR;
	pst_RTCP_RR			pstRTCP_RR;
	pst_RTCP_BILL		pstRTCP_BILL;
	RTP					*pRTP;

	UINT				uiLength = 0;
	int 				isRTPFlag = 0;


	/* CHECK RTCP SESSION */
	if( pstCAPHEAD->bRtxType == DEF_FROM_CLIENT ) {
    	stRTCPSessKey.uiSrcIP = pstINFOETH->stIP.dwSrcIP;
		if( (pstINFOETH->stUDPTCP.wSrcPort%2) == 0 ) {
			stRTCPSessKey.usSrcPort = pstINFOETH->stUDPTCP.wSrcPort + 1;
			isRTPFlag = 1;
		} else {
			stRTCPSessKey.usSrcPort = pstINFOETH->stUDPTCP.wSrcPort;
		}
	} else {
		stRTCPSessKey.uiSrcIP = pstINFOETH->stIP.dwDestIP;
		if( (pstINFOETH->stUDPTCP.wSrcPort%2) == 0 ) {
			isRTPFlag = 1;
			stRTCPSessKey.usSrcPort = pstINFOETH->stUDPTCP.wDestPort + 1;
		} else {
			stRTCPSessKey.usSrcPort = pstINFOETH->stUDPTCP.wDestPort;
		}
	}

	pstRTCPSessNode = hasho_find( pstRTCPSESSHASH, (UCHAR *)&stRTCPSessKey);
    if( pstRTCPSessNode == NULL ) {
        log_print( LOGN_DEBUG, "NO SESSSION RTCP PKT IP:%u:%u",
                            stRTCPSessKey.uiSrcIP, stRTCPSessKey.usSrcPort );
		return -2;
    }
	else
		log_print( LOGN_INFO, "[VOD_SESSION_RTCP] RTCP SESS KEY : %u:%u", stRTCPSessKey.uiSrcIP, stRTCPSessKey.usSrcPort );
	

    pstRTCPSessData = (pst_RTCPSESS)nifo_ptr( pstRTCPSESSHASH, pstRTCPSessNode->offset_Data);

	/* CHECK VOD SESSION */
    pstVODSESSNode = hasho_find( pstVODSESSHASH, (UCHAR *)&pstRTCPSessData->stVODSESSKEY);
    if( pstVODSESSNode == NULL ) {
        log_print( LOGN_CRI, "NO VOD SESSION IN RTCP SESSION VOD:%u:%u RTCP:%u:%u",
                          pstRTCPSessData->stVODSESSKEY.uiSrcIP, pstRTCPSessData->stVODSESSKEY.usSrcPort,
                          stRTCPSessKey.uiSrcIP, stRTCPSessKey.usSrcPort );

        /* DEL RTCP SESSION */
        hasho_del( pstRTCPSESSHASH, (UCHAR *)&stRTCPSessKey );
        return -3;
    }

    pstVODSessData = (pst_VODSESS)nifo_ptr( pstVODSESSHASH, pstVODSESSNode->offset_Data);

	pstVODSessData->stVODSESS.uiLastPktTime 	= pstCAPHEAD->curtime;
	pstVODSessData->stVODSESS.uiLastPktMTime 	= pstCAPHEAD->ucurtime;

	/* SET DATA TRAFFIC SIZE */
	if( pstCAPHEAD->bRtxType == DEF_FROM_CLIENT ) {
		pstVODSessData->stVODSESS.uiIPDataUpPktCnt++;
		pstVODSessData->stVODSESS.uiIPTotUpPktCnt++;
		pstVODSessData->stVODSESS.uiIPDataUpPktSize += pstINFOETH->stIP.wTotalLength;
		pstVODSessData->stVODSESS.uiIPTotUpPktSize  += pstINFOETH->stIP.wTotalLength;
	}
	else if( pstCAPHEAD->bRtxType == DEF_FROM_SERVER ) {
		pstVODSessData->stVODSESS.uiIPDataDnPktCnt++;
		pstVODSessData->stVODSESS.uiIPTotDnPktCnt++;
		pstVODSessData->stVODSESS.uiIPDataDnPktSize += pstINFOETH->stIP.wTotalLength;
		pstVODSessData->stVODSESS.uiIPTotDnPktSize  += pstINFOETH->stIP.wTotalLength;
	} 

	/* RTP PACKET COUNT */
	if( isRTPFlag == 1 ) {
		if( pstCAPHEAD->bRtxType == DEF_FROM_SERVER ) {
			pstVODSessData->stVODSESS.usRtpDnCnt++;
			pstVODSessData->stVODSESS.uiRtpDnDataSize += pstINFOETH->stIP.wTotalLength;
		}
		else if( pstCAPHEAD->bRtxType == DEF_FROM_CLIENT ) {
			pstVODSessData->stVODSESS.usRtpUpCnt++;
			pstVODSessData->stVODSESS.uiRtpUpDataSize += pstINFOETH->stIP.wTotalLength;
		}
		/* AUDIO/VIDEO 구분 */
		if( stRTCPSessKey.usSrcPort == pstVODSessData->stAudioKEY.usSrcPort) {
			pstVODSessData->stVODSESS.AudioCnt++;
			pstVODSessData->stVODSESS.uiAudioDataSize += pstINFOETH->stIP.wTotalLength;
		}
		if( stRTCPSessKey.usSrcPort == pstVODSessData->stVideoKEY.usSrcPort) {
			pstVODSessData->stVODSESS.VideoCnt++;
			pstVODSessData->stVODSESS.uiVideoDataSize += pstINFOETH->stIP.wTotalLength;
		}

		// 새로운 Jitter, Packet Loss 계산.
		pucData = nifo_get_value(pMEMSINFO, ETH_DATA_NUM, dOffset);
		pRTP = (PRTP)&pucData[pstINFOETH->offset];
		dRtpQosJitter(TOUINT(pRTP->Timestamp), pstRTCPSessData, pstVODSessData, pstCAPHEAD);
		dRtpQosLoss(TOUSHORT(pRTP->Sequence), pstRTCPSessData, pstVODSessData, pstCAPHEAD);

		return 0;
	}

    /* RTCP PARSING */
//	pucData = (U8 *)nifo_ptr( pMEMSINFO, dOffset);
	pucData = nifo_get_value(pMEMSINFO, ETH_DATA_NUM, dOffset);
    pucRTCPData = &pucData[pstINFOETH->offset];
	
	/* RTCP 카운트로 이름 변경 */
	if( pstCAPHEAD->bRtxType == DEF_FROM_SERVER )
        pstVODSessData->stVODSESS.usRtspDnCnt++;
    else if( pstCAPHEAD->bRtxType == DEF_FROM_CLIENT )
        pstVODSessData->stVODSESS.usRtspUpCnt++;

	while( uiLength < pstINFOETH->stUDPTCP.wDataLen ) {

		pstRTCP_COMM = (pst_RTCP_COMM)&pucRTCPData[uiLength];

		pstRTCP_COMM->usLength 	= htons(pstRTCP_COMM->usLength)*4;
		pstRTCP_COMM->uiSSRC	= htonl(pstRTCP_COMM->uiSSRC);

		log_print( LOGN_INFO, "[VOD_SESSION_RTCP] COMMON VERCNT:%u TYPE:%u LEN:%u SSRC:%u [%u:%u]",
						pstRTCP_COMM->ucVerCnt, pstRTCP_COMM->ucType, pstRTCP_COMM->usLength, pstRTCP_COMM->uiSSRC,
						uiLength, pstINFOETH->stUDPTCP.wDataLen );

		if( pstRTCP_COMM->ucType == DEF_SR ) {
			/* SENDER REPORT */
			log_print( LOGN_INFO, "[VOD_SESSION_RTCP] SENDER REPORT" );

			pstRTCP_SR	= (pst_RTCP_SR)&pucRTCPData[DEF_RTCP_COMM_SIZE + uiLength];

			pstRTCP_SR->uiRTPTime		= htonl(pstRTCP_SR->uiRTPTime);
			pstRTCP_SR->uiSendPktCnt	= htonl(pstRTCP_SR->uiSendPktCnt);
			pstRTCP_SR->uiSendOctCnt	= htonl(pstRTCP_SR->uiSendOctCnt);

			log_print( LOGN_INFO, "[VOD_SESSION_RTCP] SR NTP:%llu RTP:%u PKT:%u OCT:%u",
							   pstRTCP_SR->llNTPTime, pstRTCP_SR->uiRTPTime, pstRTCP_SR->uiSendPktCnt, pstRTCP_SR->uiSendOctCnt );
	
			uiLength += pstRTCP_COMM->usLength + 4;
			continue;
		}
		else if( pstRTCP_COMM->ucType == DEF_RR ) {
			/* RECEIVER REPORT */
			log_print( LOGN_INFO, "[VOD_SESSION_RTCP] RECEIVER REPORT" );

			pstRTCP_RR  = (pst_RTCP_RR)&pucRTCPData[DEF_RTCP_COMM_SIZE + uiLength];
#if 0

			pstRTCP_RR->uiSSRC		= htonl(pstRTCP_RR->uiSSRC);
			pstRTCP_RR->uiLostInfo	= htonl(pstRTCP_RR->uiLostInfo) & 0x00ffffff;
			pstRTCP_RR->uiSeqNum	= htonl(pstRTCP_RR->uiSeqNum);
			pstRTCP_RR->uiJitter	= htonl(pstRTCP_RR->uiJitter);
			pstRTCP_RR->uiLSR		= htonl(pstRTCP_RR->uiLSR);
			pstRTCP_RR->uiDLSR		= htonl(pstRTCP_RR->uiDLSR);

			log_print( LOGN_INFO, "[VOD_SESSION_RTCP] RR SSRC:%u LOST:%u SEQ:%u JIT:%u LSR:%u DLSR:%u",
							   pstRTCP_RR->uiSSRC, pstRTCP_RR->uiLostInfo, pstRTCP_RR->uiSeqNum, 
							   pstRTCP_RR->uiJitter, pstRTCP_RR->uiLSR, pstRTCP_RR->uiDLSR ); 

			/* CHECK MAX JITTER */
			if( pstVODSessData->stVODSESS.uiMaxJitter < pstRTCP_RR->uiJitter ) {
				pstVODSessData->stVODSESS.uiMaxJitter = pstRTCP_RR->uiJitter;
				pstVODSessData->stVODSESS.usMaxJitterTrackID = (USHORT)pstRTCPSessData->uiTrackID;
			}

			/* RTP CUMULATIVE LOSS CNT */
			if( (stRTCPSessKey.usSrcPort%2) == 0 ) {
				if( pstCAPHEAD->bRtxType == DEF_FROM_SERVER )
					pstVODSessData->stVODSESS.usRtpDnLossCnt = htonl(pstRTCP_RR->uiLostInfo);
				else if( pstCAPHEAD->bRtxType == DEF_FROM_CLIENT )
					pstVODSessData->stVODSESS.usRtpUpLossCnt = htonl(pstRTCP_RR->uiLostInfo);

				return 0;
			}
#endif
		}
		else if( pstRTCP_COMM->ucType == DEF_BILL ) {
			/* BILL */
			log_print( LOGN_INFO, "[VOD_SESSION_RTCP] USER SPECIFIC" );

			pstRTCP_BILL = (pst_RTCP_BILL)&pucRTCPData[DEF_RTCP_COMM_SIZE + uiLength];

			pstRTCP_BILL->uiSSRC		= htonl(pstRTCP_BILL->uiSSRC);
			pstRTCP_BILL->uiRcvDataSize	= htonl(pstRTCP_BILL->uiRcvDataSize);

			log_print( LOGN_INFO, "[VOD_SESSION_RTCP] UR SSRC:%u RCV:%u", 
							   pstRTCP_BILL->uiSSRC, pstRTCP_BILL->uiRcvDataSize );

			/* DATA SIZE FOR MEDIA */
#if 0
			if( pstRTCPSessData->uiMediaType == DEF_MEDIA_AUDIO )
				pstVODSessData->stVODSESS.uiAudioDataSize = pstRTCP_BILL->uiRcvDataSize;
			else if( pstRTCPSessData->uiMediaType == DEF_MEDIA_VIDEO )
				pstVODSessData->stVODSESS.uiVideoDataSize = pstRTCP_BILL->uiRcvDataSize;
#endif
		}
		else {
			/* ETC */
			log_print( LOGN_INFO, "IP:%u:%u RTCP:%u", 
						   	stRTCPSessKey.uiSrcIP, stRTCPSessKey.usSrcPort, pstRTCP_COMM->ucType );
			uiLength += pstRTCP_COMM->usLength + 4;
            continue;
		}

		uiLength += pstRTCP_COMM->usLength + 4;
	}

	return 0;
}


/*******************************************************************************

*******************************************************************************/
void PrintVODSessLog( pst_VODSESSKEY pstKey, LOG_VOD_SESS *pstVODSess )
{
	UCHAR	ucLog = LOGN_INFO;
	UCHAR	szIP1[4], szIP2[4];

	memcpy( szIP1, &pstKey->uiSrcIP, 4 );
	log_print( ucLog, "##### VOD SESSION LOG INFO : %3u.%3u.%3u.%3u:%6u", szIP1[3], szIP1[2], szIP1[1], szIP1[0], pstKey->usSrcPort );

	log_print( ucLog, "[CALL TIME:%10u.%06u]", pstVODSess->uiCallTime, pstVODSess->uiCallMTime );
	log_print( ucLog, "[NAS NAME :%u SVC TYPE:%u]", pstVODSess->uiNASName, pstVODSess->usServiceType );
	log_print( ucLog, "[HOST NAME:%s]", pstVODSess->szHostName );
	log_print( ucLog, "[BROWSER INFO:%s]", pstVODSess->szBrowserInfo );
	log_print( ucLog, "[MODEL    :%s]", pstVODSess->szModel );
	log_print( ucLog, "[SVC OPT  :%s]", pstVODSess->szNetOption );
	log_print( ucLog, "[MIN      :%s]", pstVODSess->szMIN );

	memcpy( szIP2, &pstVODSess->uiServerIP, 4 );
	log_print( ucLog, "[SRV IP :%3u.%3u.%3u.%3u:%6u]", szIP2[3], szIP2[2], szIP2[1], szIP2[0], pstVODSess->usServerPort );
	log_print( ucLog, "[TCP SYN:%10u.%06u]", pstVODSess->uiTcpSynTime, pstVODSess->uiTcpSynMTime );
	log_print( ucLog, "[TYPE PLAT:%ld SVCL4:%ld SVCL7:%ld SUB_SYSNO:%u]", pstVODSess->usPlatformType, pstVODSess->usSvcL4Type, pstVODSess->usSvcL7Type, pstVODSess->ucSubSysNo );
	log_print( ucLog, "[SETUP START:%10u.%06u]", pstVODSess->uiSetupStartTime, pstVODSess->uiSetupStartMTime );
	log_print( ucLog, "[SETUP END  :%10u.%06u]", pstVODSess->uiSetupEndTime, pstVODSess->uiSetupEndMTime );
	log_print( ucLog, "[TEARDOWN   :%10u.%06u]", pstVODSess->uiTeardownTime, pstVODSess->uiTeardownMTime );
	log_print( ucLog, "[LAST PKT   :%10u.%06u]", pstVODSess->uiLastPktTime, pstVODSess->uiLastPktMTime );
	log_print( ucLog, "[SESS GAP   :%lld]", pstVODSess->llSessGapTime );

	log_print( ucLog, "[TCP STS CLI:%lu SRV:%lu]", pstVODSess->ucTcpClientStatus, pstVODSess->ucTcpServerStatus );
	log_print( ucLog, "[STATUS:%lu USER_ERR:%d]", pstVODSess->ucStatus, pstVODSess->usUserErrorCode );
	log_print( ucLog, "[FAIL CODE L4:%lu L7:%d]", pstVODSess->usL4FailCode, pstVODSess->usL7FailCode );
	
	log_print( ucLog, "[IP DATA PKT CNT  UP/DN:%u/%u RETRAN UP/DN:%u/%u]", 
											   pstVODSess->uiIPDataUpPktCnt, pstVODSess->uiIPDataDnPktCnt,
											   pstVODSess->uiIPDataUpRetransCnt, pstVODSess->uiIPDataDnRetransCnt );
	log_print( ucLog, "[IP TOT  PKT CNT  UP/DN:%u/%u RETRAN UP/DN:%u/%u]", 
											   pstVODSess->uiIPTotUpPktCnt, pstVODSess->uiIPTotDnPktCnt,
											   pstVODSess->uiIPTotUpRetransCnt, pstVODSess->uiIPTotDnRetransCnt );
	log_print( ucLog, "[IP DATA PKT SIZE UP/DN:%u/%u RETRAN UP/DN:%u/%u]", 
                                               pstVODSess->uiIPDataUpPktSize, pstVODSess->uiIPDataDnPktSize,
                                               pstVODSess->uiIPDataUpRetransSize, pstVODSess->uiIPDataDnRetransSize );
    log_print( ucLog, "[IP TOT  PKT SIZE UP/DN:%u/%u RETRAN UP/DN:%u/%u]",
                                               pstVODSess->uiIPTotUpPktSize, pstVODSess->uiIPTotDnPktSize,
                                               pstVODSess->uiIPTotUpRetransSize, pstVODSess->uiIPTotDnRetransSize );
	log_print( ucLog, "[TCP BODY SIZE    IP/DN:%u/%u]", pstVODSess->uiTcpUpBodySize, pstVODSess->uiTcpDnBodySize );

	log_print( ucLog, "[SIZE TRAFFIC/AUDIO/VIDEO:%u/%u/%u]", pstVODSess->uiTrafficSize, pstVODSess->uiAudioDataSize, pstVODSess->uiVideoDataSize );
	log_print( ucLog, "[RTCP CNT UP/DN:%u/%u]", pstVODSess->usRtspUpCnt, pstVODSess->usRtspDnCnt );
	log_print( ucLog, "[JITTER MAX:%u ID:%u]", pstVODSess->uiMaxJitter, pstVODSess->usMaxJitterTrackID );
	log_print( ucLog, "[LOSS CNT      UP[%u] DOWN[%u]", pstVODSess->usRtpUpLossCnt, pstVODSess->usRtpDnLossCnt);
	log_print( ucLog, "[RTI:%u MEDIA_CNT:%u RANGE:%10u.%06u]", pstVODSess->usRTI, pstVODSess->usMediaCnt, pstVODSess->uiTimeRange, pstVODSess->uiTimeMRange );
	log_print( ucLog, "[URL_LEN:%u URL:%s", pstVODSess->usLOGURLSize, pstVODSess->szLOGURL );
	log_print( ucLog, "[MS MAN : %s]", pstVODSess->szMsMan );
	log_print( ucLog, "[SESS ID : %s]", pstVODSess->szVodSessID );
	log_print( ucLog, "[OP START END : %10u.%06u %10u.%06u]", pstVODSess->uiOpStartTime, pstVODSess->uiOpStartMTime, pstVODSess->uiOpEndTime, pstVODSess->uiOpEndMTime );
} 

/* tvTime 구조체는 캡쳐 시간을 가지는 정보임 */
int dRtpQosJitter(UINT uiTimestamp, pst_RTCPSESS pstRTCPSessData, pst_VODSESS pstVODSessData, Capture_Header_Msg *pstCAPHEAD)
{
	struct timeval stDiffRecvTime, tvTime;
	int diff, jitter;
	unsigned int subTimestamp;

	tvTime.tv_sec = pstCAPHEAD->curtime;
	tvTime.tv_usec = pstCAPHEAD->ucurtime;

	if( pstRTCPSessData->tvOldTime.tv_sec == 0 && pstRTCPSessData->tvOldTime.tv_usec == 0 )
		pstRTCPSessData->tvOldTime = tvTime;
	timersub(&tvTime, &pstRTCPSessData->tvOldTime, &stDiffRecvTime);
	if( stDiffRecvTime.tv_sec < 0 ) {
		return -1;
	}

	/* 1ms = 8 sampling */
	if( pstRTCPSessData->uiOldTimestamp == 0 )
		pstRTCPSessData->uiOldTimestamp = uiTimestamp;
	if( (subTimestamp = uiTimestamp - pstRTCPSessData->uiOldTimestamp) < 0) {
		pstRTCPSessData->uiOldTimestamp = uiTimestamp;
		pstRTCPSessData->tvOldTime = tvTime;
		return -2;
	}

	diff = abs((stDiffRecvTime.tv_sec * 8) - subTimestamp);
	jitter = pstRTCPSessData->uiIntervalJitter;
	pstRTCPSessData->uiIntervalJitter = jitter + ((diff - jitter)/16);

	/* init */
	pstRTCPSessData->uiOldTimestamp = uiTimestamp;
	pstRTCPSessData->tvOldTime = tvTime;

	/* CHECK MAX JITTER */
	if( pstVODSessData->stVODSESS.uiMaxJitter < pstRTCPSessData->uiIntervalJitter ) {
		pstVODSessData->stVODSESS.uiMaxJitter = pstRTCPSessData->uiIntervalJitter;
		pstVODSessData->stVODSESS.usMaxJitterTrackID = (USHORT)pstRTCPSessData->uiTrackID;
	}
////log_print(LOGN_INFO, "JITTER[%u:%u] MAXJITTER[%u:%u]",
//pstRTCPSessData->uiTrackID, pstRTCPSessData->uiIntervalJitter,
//pstVODSessData->stVODSESS.uiMaxJitter, pstVODSessData->stVODSESS.usMaxJitterTrackID);

	return 0;
}

int dRtpQosLoss(UINT uiSequence, pst_RTCPSESS pstRTCPSessData, pst_VODSESS pstVODSessData, Capture_Header_Msg *pstCAPHEAD)
{
	int     loss;
	stHASHONODE *pstAudioNode, *pstVideoNode;
	pst_RTCPSESS pstAudioData, pstVideoData;
	char ip[INET_ADDRSTRLEN];
	struct in_addr in;

	if( pstRTCPSessData->uiOldSequence == 0 )
		pstRTCPSessData->uiOldSequence = uiSequence;

	loss = uiSequence - pstRTCPSessData->uiOldSequence;
	if(loss == 1) {
		pstRTCPSessData->uiOldSequence = uiSequence;
	}
	else if(loss > 1) {
		if( pstCAPHEAD->bRtxType == DEF_FROM_SERVER )
			pstRTCPSessData->uiDnLossCnt += loss - 1;
		else if( pstCAPHEAD->bRtxType == DEF_FROM_CLIENT )
			pstRTCPSessData->uiUpLossCnt += loss - 1;
		pstRTCPSessData->uiOldSequence = uiSequence;

		if( pstVODSessData->dATrackID > 0 ) {
			pstAudioNode = hasho_find(pstRTCPSESSHASH, (UCHAR *)&pstVODSessData->stAudioKEY);
			if( pstAudioNode ) {
				pstAudioData = (pst_RTCPSESS)HASHO_PTR(pstRTCPSESSHASH, pstAudioNode->offset_Data);
				pstVODSessData->stVODSESS.usRtpUpLossCnt = pstAudioData->uiUpLossCnt;
				pstVODSessData->stVODSESS.usRtpDnLossCnt = pstAudioData->uiDnLossCnt;
			}
			else {
				ip[0] = 0;
				in.s_addr = htonl(pstVODSessData->stAudioKEY.uiSrcIP);
				inet_ntop(AF_INET, &in, ip, sizeof(ip));
				log_print(LOGN_WARN, "AUDIO NODE NOT FOUND. ID[%d] IP[%s] PORT[%u]",
						pstVODSessData->dATrackID, ip, pstVODSessData->stAudioKEY.usSrcPort);
			}
		}
		if( pstVODSessData->dVTrackID > 0 ) {
			pstVideoNode = hasho_find(pstRTCPSESSHASH, (UCHAR *)&pstVODSessData->stVideoKEY);
			if( pstVideoNode ) {
				pstVideoData = (pst_RTCPSESS)HASHO_PTR(pstRTCPSESSHASH, pstVideoNode->offset_Data);
				pstVODSessData->stVODSESS.usRtpUpLossCnt += pstVideoData->uiUpLossCnt;
				pstVODSessData->stVODSESS.usRtpDnLossCnt += pstVideoData->uiDnLossCnt;
			}
			else {
				ip[0] = 0;
				in.s_addr = htonl(pstVODSessData->stVideoKEY.uiSrcIP);
				inet_ntop(AF_INET, &in, ip, sizeof(ip));
				log_print(LOGN_WARN, "VIDEO NODE NOT FOUND. ID[%d] IP[%s] PORT[%u]",
						pstVODSessData->dVTrackID, ip, pstVODSessData->stVideoKEY.usSrcPort);
			}
		}
//log_print(LOGN_INFO, "LOSSCNT1: CAPTIME[%ld.%06ld] UP[%u] DOWN[%u]",
//pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
//pstVODSessData->stVODSESS.usRtpUpLossCnt, pstVODSessData->stVODSESS.usRtpDnLossCnt);
	}
	else if(loss < 0) {
		/* overflow process */
		if(loss < -100) {
			pstRTCPSessData->uiOldSequence = uiSequence;
		}
		else {
			if( pstCAPHEAD->bRtxType == DEF_FROM_SERVER ) {
				if( pstRTCPSessData->uiDnLossCnt > 0)
					pstRTCPSessData->uiDnLossCnt--;
			}
			else if( pstCAPHEAD->bRtxType == DEF_FROM_CLIENT ) {
				if( pstRTCPSessData->uiUpLossCnt > 0)
					pstRTCPSessData->uiUpLossCnt--;
			}
			if( pstVODSessData->dATrackID > 0 ) {
				pstAudioNode = hasho_find(pstRTCPSESSHASH, (UCHAR *)&pstVODSessData->stAudioKEY);
				if( pstAudioNode ) {
					pstAudioData = (pst_RTCPSESS)HASHO_PTR(pstRTCPSESSHASH, pstAudioNode->offset_Data);
					pstVODSessData->stVODSESS.usRtpUpLossCnt = pstAudioData->uiUpLossCnt;
					pstVODSessData->stVODSESS.usRtpDnLossCnt = pstAudioData->uiDnLossCnt;
				}
				else {
					ip[0] = 0;
					in.s_addr = htonl(pstVODSessData->stAudioKEY.uiSrcIP);
					inet_ntop(AF_INET, &in, ip, sizeof(ip));
					log_print(LOGN_WARN, "AUDIO NODE NOT FOUND. ID[%d] IP[%s] PORT[%u]",
							pstVODSessData->dATrackID, ip, pstVODSessData->stAudioKEY.usSrcPort);
				}
			}
			if( pstVODSessData->dVTrackID > 0 ) {
				pstVideoNode = hasho_find(pstRTCPSESSHASH, (UCHAR *)&pstVODSessData->stVideoKEY);
				if( pstVideoNode ) {
					pstVideoData = (pst_RTCPSESS)HASHO_PTR(pstRTCPSESSHASH, pstVideoNode->offset_Data);
					pstVODSessData->stVODSESS.usRtpUpLossCnt += pstVideoData->uiUpLossCnt;
					pstVODSessData->stVODSESS.usRtpDnLossCnt += pstVideoData->uiDnLossCnt;
				}
				else {
					ip[0] = 0;
					in.s_addr = htonl(pstVODSessData->stVideoKEY.uiSrcIP);
					inet_ntop(AF_INET, &in, ip, sizeof(ip));
					log_print(LOGN_WARN, "VIDEO NODE NOT FOUND. ID[%d] IP[%s] PORT[%u]",
							pstVODSessData->dVTrackID, ip, pstVODSessData->stVideoKEY.usSrcPort);
				}
			}
//log_print(LOGN_INFO, "LOSSCNT2: CAPTIME[%ld.%06ld] UP[%u] DOWN[%u]",
//pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
//pstVODSessData->stVODSESS.usRtpUpLossCnt, pstVODSessData->stVODSESS.usRtpDnLossCnt);
		}
	}

	return 0;
}

