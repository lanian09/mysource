/**<	
 $Id: l2tp_func.c,v 1.2 2011/09/05 07:33:22 dhkim Exp $
 $Author: dhkim $
 $Revision: 1.2 $
 $Date: 2011/09/05 07:33:22 $
 **/

#include <ctype.h>
#include <sys/time.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "utillib.h"
#include "mems.h"
#include "memg.h"
#include "nifo.h"
#include "cifo.h"
#include "gifo.h"
#include "timerN.h"

#include "PPP_header.h"

// PROJECT
#include "procid.h"
#include "sshmid.h"
#include "filter.h"
#include "common_stg.h"

// TAF
#include "arp_head.h"

// .
#include "l2tp_func.h"

/**
 * Declare variables
 */
stHASHOINFO			*pstCHASHOINFO;		/* Control Session */
stHASHOINFO			*pstHASHOINFO;		/* Call Session */
stHASHOINFO			*pstTHASHOINFO;		/* Tunnel ID */
stHASHOINFO			*pstSHASHOINFO;		/* Session ID */

stTIMERNINFO		*pstTIMERNINFO;
stTIMERNINFO		*pstCTIMERNINFO;
stTIMERNINFO		*pstTTIMERNINFO;
stTIMERNINFO		*pstSTIMERNINFO;

st_Flt_Info		 *flt_info;
st_TraceList		*pstTRACE;			/* TRACE */

S32				 gTIMER_TRANS;

extern stMEMSINFO	*pstMEMSINFO;
extern stCIFO		*gpCIFO;

extern int dump_DebugString(char *debug_str, char *s, int len);
extern int Analyze_L2TP_PPP(PUCHAR pBuf, WORD wSize, INFO_PPP *pInfo, struct slcompress *pComp);

/** GetGapTime function.
 *
 *	@return		 S64
 **/

S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime)
{
	S64	 gapTime;

	gapTime = (((S64)endtime * 1000000 + (S64)endmtime) - ((S64)starttime * 1000000 + (S64)startmtime));

	if(gapTime < 0)
		gapTime = 0;

	return gapTime;
}

int dump(char *s,int len)
{		
	char buf[BUFSIZ],lbuf[BUFSIZ],rbuf[BUFSIZ];
	unsigned char *p;
	int line,i;

	p =(unsigned char *) s;
	for(line = 1; len > 0; len -= WIDTH,line++) {
		memset(lbuf,0,BUFSIZ);
		memset(rbuf,0,BUFSIZ);

		for(i = 0; i < WIDTH && len > i; i++,p++) {
			sprintf(buf,"%02x ",(unsigned char) *p);
			strcat(lbuf,buf);
			sprintf(buf,"%c",(!iscntrl(*p) && *p <= 0x7f) ? *p : '.');
			strcat(rbuf,buf);
		}
		printf("%04x: %-*s	%s\n",line - 1,WIDTH * 3,lbuf,rbuf);
	}			
	/*if(!(len%16)) log_print(LOGN_INFO, "\n"); */
	return line;
}


/*
 * cb_timeout_Control
 *
 */
int cb_timeout_Control(st_CONTROL_KEY *pstTransKey)
{
	int 				dRet;
	st_CONTROL_DATA		*pstTransData;
	stHASHONODE			*pstTransHashNode;

	pstTransHashNode = hasho_find (pstCHASHOINFO, (U8*)pstTransKey);
	pstTransData = (st_CONTROL_DATA *) nifo_ptr (pstCHASHOINFO, pstTransHashNode->offset_Data);
	pstTransData->uiLastUserErrCode = L2TP_USER_TIMEOUT;
	
	log_print (LOGN_DEBUG, "	### TIMEOUT	CRONTROL	REQTID: %d RESTID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d", 
			pstTransKey->usReqTunnelID, pstTransKey->usResTunnelID, HIPADDR(pstTransKey->uiSrcIP), HIPADDR(pstTransKey->uiDestIP) );

	/* TODO: dSendTransSignal */
	dRet = dSendStopCCNSignal(NULL, pstTransKey, pstTransData, NULL);

	return SUCC;
}

/*
 * cb_timeout_CallSess
 * Invoke session from Call Session Timer
 */
int cb_timeout_CallSess(st_CALLSESS_KEY *pstTransKey)
{
	int 				dRet;
	st_CALLSESS_DATA	*pstTransData;
	stHASHONODE			*pstTransHashNode;

	pstTransHashNode = hasho_find (pstHASHOINFO, (U8*)pstTransKey);
	pstTransData = (st_CALLSESS_DATA *) nifo_ptr (pstHASHOINFO, pstTransHashNode->offset_Data);
	pstTransData->uiLastUserErrCode = L2TP_USER_TIMEOUT;
	
	log_print (LOGN_DEBUG, "	### TIMEOUT	CALLSESS	REQSID: %d RESSID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d", 
			pstTransKey->usReqSessID, pstTransKey->usResSessID, HIPADDR(pstTransKey->uiSrcIP), HIPADDR(pstTransKey->uiDestIP) );

	// dSendTransSignal 
	dRet = dSendCDNSignal(NULL, pstTransKey, pstTransData, NULL);

	return SUCC;
}

/*
 * cb_timeout_Sessid
 *
 */
int cb_timeout_Sessid(st_SESSID_KEY *pstTransKey)
{
	int 				dRet;
	st_SESSID_DATA		*pstTransData;
	stHASHONODE			*pstTransHashNode;

	log_print (LOGN_DEBUG, "	### TIMEOUT	SESSIONID	SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d PORT: %d, %d", 
			pstTransKey->usSessionID, HIPADDR(pstTransKey->uiSrcIP), HIPADDR(pstTransKey->uiDestIP), pstTransKey->usSrcPort, pstTransKey->usDestPort );

	pstTransHashNode = hasho_find (pstSHASHOINFO, (U8*)pstTransKey);
	pstTransData = (st_SESSID_DATA *) nifo_ptr (pstSHASHOINFO, pstTransHashNode->offset_Data);
	pstTransData->uiLastUserErrCode = L2TP_USER_TIMEOUT;
	
	dRet = dSendSessIDSignal(NULL, pstTransKey, pstTransData);

	return SUCC;
}

/*
 * cb_timeout_tunnelid
 *
 */
int cb_timeout_tunnelid(st_TUNNELID_KEY *pstTransKey)
{
	int 				dRet;
	st_TUNNELID_DATA	*pstTransData;
	stHASHONODE			*pstTransHashNode;

	log_print (LOGN_DEBUG, "	### TIMEOUT	TUNNELID	SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d PORT: %d, %d", 
			pstTransKey->usTunnelID, HIPADDR(pstTransKey->uiSrcIP), HIPADDR(pstTransKey->uiDestIP), pstTransKey->usSrcPort, pstTransKey->usDestPort );

	pstTransHashNode = hasho_find (pstTHASHOINFO, (U8*)pstTransKey);
	pstTransData = (st_TUNNELID_DATA *) nifo_ptr (pstTHASHOINFO, pstTransHashNode->offset_Data);
	pstTransData->uiLastUserErrCode = L2TP_USER_TIMEOUT;
	
	dRet = dSendTunnelIDSignal(NULL, pstTransKey, pstTransData);

	return SUCC;
}

/*
 * dSendTUNNELIDSignal 
 * SCCRQ -> SCCRP -> SCCCN 까지 진행되지 않고 타이아웃 되었을 경우 
 */
int dSendTunnelIDSignal(Capture_Header_Msg *pCAPHEAD, st_TUNNELID_KEY *pstTunnelIDKey, st_TUNNELID_DATA *stTunnelIDData)
{
	int				 dRet;
	struct timeval		now;

	UCHAR				*pstStartNode;
	LOG_SIGNAL			*pstTransLog;

	st_TUNNELID_KEY 	stTunnelIDKey;
	st_TUNNELID_DATA	*pstTransData;
	stHASHONODE			*pstTransHashNode;


	gettimeofday(&now, 0);

	/*	ADD SIGNAL NIFO NODE */	
	if( (pstStartNode = nifo_node_alloc(pstMEMSINFO)) == NULL ) {
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	if( (pstTransLog = (LOG_SIGNAL *) 
				nifo_tlv_alloc(pstMEMSINFO, pstStartNode, LOG_SIGNAL_DEF_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_ON)) == NULL )
	{
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_delete(pstMEMSINFO, pstStartNode);
		return -2;
	}

	gettimeofday(&now, 0);

	/* COMMON */
	if ( pCAPHEAD==NULL ) {
		pstTransLog->uiLastUserErrCode = L2TP_USER_TIMEOUT;
	} else {
		pstTransLog->uiSessEndTime = pCAPHEAD->curtime;
		pstTransLog->uiSessEndMTime = pCAPHEAD->ucurtime;
		pstTransLog->uiSessDuration = GetGapTime(pstTransLog->uiSessEndTime, pstTransLog->uiSessEndMTime, 
				pstTransLog->uiSessStartTime, pstTransLog->uiSessStartMTime);
	}

	/* COMMON SIGNAL INFO */
	pstTransLog->uiCallTime = stTunnelIDData->ReqTunnelIDTime;
	pstTransLog->uiCallMTime = stTunnelIDData->ReqTunnelIDMTime;
	pstTransLog->uiSessStartTime = stTunnelIDData->ReqTunnelIDTime;
	pstTransLog->uiSessStartMTime = stTunnelIDData->ReqTunnelIDMTime;
	pstTransLog->uiSessStartTime = stTunnelIDData->ReqTunnelIDTime;
	pstTransLog->uiSessStartMTime = stTunnelIDData->ReqTunnelIDMTime;

	pstTransLog->uiProtoType = DEF_PROTOCOL_L2TP;
	pstTransLog->uiMsgType = MSG_L2TP_TUNNEL_START;

	pstTransLog->uiSrcIP = pstTunnelIDKey->uiSrcIP;
	pstTransLog->uiDestIP = pstTunnelIDKey->uiDestIP;
	pstTransLog->uiL2TPReqTime = stTunnelIDData->ReqTunnelIDTime; 		/* SCCRQ, ICRQ Time */
	pstTransLog->uiL2TPReqMTime = stTunnelIDData->ReqTunnelIDMTime;
	pstTransLog->uiL2TPRepTime = stTunnelIDData->ResTunnelIDTime; 		/* SCCRP, ICRP Time */
	pstTransLog->uiL2TPRepMTime = stTunnelIDData->ResTunnelIDMTime;
	pstTransLog->uiL2TPConTime = 0; 									/* SCCCN, ICCN Time */
	pstTransLog->uiL2TPConMTime = 0;

	pstTransLog->usReqTunnelID = stTunnelIDData->stCONTROLKey.usReqTunnelID; 				/* LAC Tunnel ID */
	pstTransLog->usRepTunnelID = stTunnelIDData->stCONTROLKey.usResTunnelID; 				/* LNS Tunnel ID */

	pstTransLog->uiUpL2TPPkts = stTunnelIDData->uiUpPktCnt;
	pstTransLog->uiDnL2TPPkts = stTunnelIDData->uiDnPktCnt;
	pstTransLog->uiUpL2TPBytes = stTunnelIDData->uiUpPktBytes;
	pstTransLog->uiDnL2TPBytes = stTunnelIDData->uiDnPktBytes;



	LOG_SIGNAL_Prt("PRINT LOG_SIGNAL", pstTransLog);

	/* SEND LOG */
	dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_L2TP, SEQ_PROC_A_CALL, nifo_offset( pstMEMSINFO, pstStartNode ));
	if( dRet < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
		nifo_node_delete(pstMEMSINFO, pstStartNode);
		return -1;
	}

	/*	DELETE TRANS INFO */
	if (stTunnelIDData->ulTimerNID != (long long)0) {
		timerN_del (pstTTIMERNINFO, stTunnelIDData->ulTimerNID);
	}
	hasho_del (pstTHASHOINFO, (U8 *)pstTunnelIDKey);

	log_print (LOGN_DEBUG, "	--> DEL TUNNELID HASH		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d TID: %lld",
			pstTunnelIDKey->usTunnelID, HIPADDR(pstTunnelIDKey->uiSrcIP), HIPADDR(pstTunnelIDKey->uiDestIP), stTunnelIDData->ulTimerNID);

	if(stTunnelIDData->usLastMessageType > L2TP_MSG_SCCRQ) {

		/* DELETE TUNNELID INFO */
		memcpy(&stTunnelIDKey, pstTunnelIDKey, DEF_TUNNELID_KEY_SIZE);
		if (pstTunnelIDKey->usTunnelID == stTunnelIDData->stCONTROLKey.usReqTunnelID) {
			stTunnelIDKey.usTunnelID = stTunnelIDData->stCONTROLKey.usResTunnelID;
		} else {
			stTunnelIDKey.usTunnelID = stTunnelIDData->stCONTROLKey.usReqTunnelID;
		}

		if((pstTransHashNode = hasho_find (pstTHASHOINFO, (U8*)&stTunnelIDKey)) != NULL) {
			pstTransData = (st_TUNNELID_DATA *) nifo_ptr (pstTHASHOINFO, pstTransHashNode->offset_Data);

			timerN_del (pstTTIMERNINFO, pstTransData->ulTimerNID);
			hasho_del (pstTHASHOINFO, (U8 *)&stTunnelIDKey);
			log_print (LOGN_DEBUG, "	--> DEL TUNNELID HASH		SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d SID: %d TID: %lld",
					HIPADDR(stTunnelIDKey.uiSrcIP), HIPADDR(stTunnelIDKey.uiDestIP), 
					stTunnelIDKey.usTunnelID, pstTransData->ulTimerNID);
		}
		else {
			log_print (LOGN_CRI, "	--> DEL TUNNELID HASH NOT FIND		SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d SID: %d TID: %lld",
					HIPADDR(stTunnelIDKey.uiSrcIP), HIPADDR(stTunnelIDKey.uiDestIP), 
					stTunnelIDKey.usTunnelID, pstTransData->ulTimerNID);
		}
	}

	return 0;
}


/*
 * dSendSESSIDSignal 
 * ICRQ -> ICRP -> ICCN 까지 진행되지 않고 타이아웃 되었을 경우 
 */
int dSendSessIDSignal(Capture_Header_Msg *pCAPHEAD, st_SESSID_KEY *pstSessIDKey, st_SESSID_DATA *pstSessIDData)
{
	int				 dRet;
	struct timeval		now;

	UCHAR				*pstStartNode;
	LOG_SIGNAL			*pstTransLog;

	st_SESSID_KEY 		stSessIDKey;

	stHASHONODE			*pstTransHashNode;
	st_SESSID_DATA		*pstTransData;

	gettimeofday(&now, 0);

	/*	ADD SIGNAL NIFO NODE */	
	if( (pstStartNode = nifo_node_alloc(pstMEMSINFO)) == NULL ) {
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	if( (pstTransLog = (LOG_SIGNAL *) 
				nifo_tlv_alloc(pstMEMSINFO, pstStartNode, LOG_SIGNAL_DEF_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_ON)) == NULL )
	{
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_delete(pstMEMSINFO, pstStartNode);
		return -2;
	}

	gettimeofday(&now, 0);

	/* COMMON */
	if ( pCAPHEAD==NULL ) {
		pstTransLog->uiLastUserErrCode = L2TP_USER_TIMEOUT;
	} else {
		pstTransLog->uiSessEndTime = pCAPHEAD->curtime;
		pstTransLog->uiSessEndMTime = pCAPHEAD->ucurtime;
		pstTransLog->uiSessDuration = GetGapTime(pstTransLog->uiSessEndTime, pstTransLog->uiSessEndMTime, 
				pstTransLog->uiSessStartTime, pstTransLog->uiSessStartMTime);
	}

	/* COMMON SIGNAL INFO */
	pstTransLog->uiCallTime = pstSessIDData->ReqSessionIDTime;
	pstTransLog->uiCallMTime = pstSessIDData->ReqSessionIDMTime;
	pstTransLog->uiSessStartTime = pstSessIDData->ReqSessionIDTime;
	pstTransLog->uiSessStartMTime = pstSessIDData->ReqSessionIDMTime;
	pstTransLog->uiSessStartTime = pstSessIDData->ReqSessionIDTime;
	pstTransLog->uiSessStartMTime = pstSessIDData->ReqSessionIDMTime;

	memcpy(pstTransLog->szIMSI, pstSessIDData->stCALLSESSKey.szIMSI, MAX_MIN_LEN);
	pstTransLog->szIMSI[MAX_MIN_LEN] = 0x00;

	pstTransLog->ucBranchID = pstSessIDData->ucBranchID;
	pstTransLog->uiProtoType = DEF_PROTOCOL_L2TP;
	pstTransLog->uiMsgType = MSG_L2TP_CALL_START;

	pstTransLog->uiSrcIP = pstSessIDKey->uiSrcIP;
	pstTransLog->uiDestIP = pstSessIDKey->uiDestIP;
	pstTransLog->uiL2TPReqTime = pstSessIDData->ReqSessionIDTime; 		/* SCCRQ, ICRQ Time */
	pstTransLog->uiL2TPReqMTime = pstSessIDData->ReqSessionIDMTime;
	pstTransLog->uiL2TPRepTime = pstSessIDData->ResSessionIDTime; 		/* SCCRP, ICRP Time */
	pstTransLog->uiL2TPRepMTime = pstSessIDData->ResSessionIDMTime;
	pstTransLog->uiL2TPConTime = 0; 				/* SCCCN, ICCN Time */
	pstTransLog->uiL2TPConMTime = 0;

	pstTransLog->usReqSessID = pstSessIDData->stCALLSESSKey.usReqSessID; 				/* LAC Tunnel ID */
	pstTransLog->usRepSessID = pstSessIDData->stCALLSESSKey.usResSessID; 				/* LNS Tunnel ID */

	pstTransLog->uiUpL2TPPkts = pstSessIDData->uiUpPktCnt;
	pstTransLog->uiDnL2TPPkts = pstSessIDData->uiDnPktCnt;
	pstTransLog->uiUpL2TPBytes = pstSessIDData->uiUpPktBytes;
	pstTransLog->uiDnL2TPBytes = pstSessIDData->uiDnPktBytes;



	LOG_SIGNAL_Prt("PRINT LOG_SIGNAL", pstTransLog);

	/* SEND LOG */
	dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_L2TP, SEQ_PROC_A_CALL, nifo_offset( pstMEMSINFO, pstStartNode ));
	if( dRet < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
		nifo_node_delete(pstMEMSINFO, pstStartNode);
		return -1;
	}

	/*	DELETE TRANS INFO */
	if (pstSessIDData->ulTimerNID != (long long)0) {
		timerN_del (pstSTIMERNINFO, pstSessIDData->ulTimerNID);
	}
	hasho_del (pstSHASHOINFO, (U8 *)pstSessIDKey);

	log_print (LOGN_DEBUG, "	--> DEL SESSID HASH		SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d SID: %d TID: %lld",
			HIPADDR(pstSessIDKey->uiSrcIP), HIPADDR(pstSessIDKey->uiDestIP), pstSessIDKey->usSessionID, pstSessIDData->ulTimerNID);

	if(pstSessIDData->usLastMessageType > L2TP_MSG_ICRQ) {

		/* DELETE SESSID INFO */
		memcpy(&stSessIDKey, pstSessIDKey, DEF_SESSID_KEY_SIZE);
		if (pstSessIDKey->usSessionID == pstSessIDData->stCALLSESSKey.usReqSessID) {
			stSessIDKey.usSessionID = pstSessIDData->stCALLSESSKey.usResSessID;
		} else {
			stSessIDKey.usSessionID = pstSessIDData->stCALLSESSKey.usReqSessID;
		}

		if((pstTransHashNode = hasho_find (pstSHASHOINFO, (U8*)&stSessIDKey)) != NULL) {
			pstTransData = (st_SESSID_DATA *) nifo_ptr (pstSHASHOINFO, pstTransHashNode->offset_Data);

			timerN_del (pstSTIMERNINFO, pstTransData->ulTimerNID);
			hasho_del (pstSHASHOINFO, (U8 *)&stSessIDKey);
			log_print (LOGN_DEBUG, "	--> DEL SESSID HASH		SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d SID: %d TID: %lld",
					HIPADDR(stSessIDKey.uiSrcIP), HIPADDR(stSessIDKey.uiDestIP), 
					stSessIDKey.usSessionID, pstTransData->ulTimerNID);
		}
		else {
			log_print (LOGN_CRI, "	--> DEL SESSID HASH NOT FIND		SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d SID: %d TID: %lld",
					HIPADDR(stSessIDKey.uiSrcIP), HIPADDR(stSessIDKey.uiDestIP), 
					stSessIDKey.usSessionID, pstTransData->ulTimerNID);
		}
	}

	return 0;
}



/* 
 * dSendStopCCNSignal
 *
 */
int dSendStopCCNSignal(Capture_Header_Msg *pCAPHEAD, st_CONTROL_KEY *pstControlKey, st_CONTROL_DATA *pstControlData, st_TUNNELID_KEY *pstTunnelIDKey)
{
	int				 dRet;
	struct timeval		now;

	UCHAR				*pNode;
	LOG_SIGNAL			*pstTransLog;


	pstTransLog = (LOG_SIGNAL *) nifo_ptr (pstMEMSINFO, pstControlData->dOffset);
	pNode = nifo_ptr (pstMEMSINFO, nifo_get_offset_node (pstMEMSINFO, (U8 *)pstTransLog));

	gettimeofday(&now, 0);

	/* COMMON */
	if ( pCAPHEAD==NULL ) { 	// Received TIMEOUT Signal
		pstTransLog->uiLastUserErrCode = L2TP_USER_TIMEOUT;

		LOG_SIGNAL_Prt("PRINT LOG_SIGNAL", pstTransLog);

		/*	SEND LOG */
		dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_L2TP, SEQ_PROC_A_CALL, nifo_offset( pstMEMSINFO, pNode ));
		if( dRet < 0 ){
			log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
			nifo_node_delete(pstMEMSINFO, pNode);
			return -1;
		}

		/*	DELETE TRANS INFO */
		dRet = dDeleteControlSession( pstControlKey );


	} else {	/* Received StopCCN Signal */
		pstTransLog->uiSessEndTime = pCAPHEAD->curtime;
		pstTransLog->uiSessEndMTime = pCAPHEAD->ucurtime;
		pstTransLog->uiSessDuration = GetGapTime(pstTransLog->uiSessEndTime, pstTransLog->uiSessEndMTime, 
				pstTransLog->uiSessStartTime, pstTransLog->uiSessStartMTime);

		/* Update control session timer */
#ifdef SIM 
		gTIMER_TRANS = 20;
#else
		gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_WAIT_TIMEOUT];
#endif
		pstControlData->ulTimerNID = timerN_update (pstCTIMERNINFO, pstControlData->ulTimerNID, time(NULL) + gTIMER_TRANS);
	}

	return 0;
}

/* 
 * dSendStopCCNSignal
 *
 */
int dSendStopCCNSignalOrg(Capture_Header_Msg *pCAPHEAD, st_CONTROL_KEY *pstControlKey, st_CONTROL_DATA *pstControlData, st_TUNNELID_KEY *pstTunnelIDKey)
{
	int				 dRet;
	struct timeval		now;

	UCHAR				*pNode;
	LOG_SIGNAL			*pstTransLog;


	pstTransLog = (LOG_SIGNAL *) nifo_ptr (pstMEMSINFO, pstControlData->dOffset);
	pNode = nifo_ptr (pstMEMSINFO, nifo_get_offset_node (pstMEMSINFO, (U8 *)pstTransLog));

	gettimeofday(&now, 0);

	/* COMMON */
	if ( pCAPHEAD==NULL ) {
//		pstTransLog->uiSessEndTime = now.tv_sec;
//		pstTransLog->uiSessEndMTime = now.tv_usec;
		pstTransLog->uiLastUserErrCode = L2TP_USER_TIMEOUT;

		nifo_node_delete(pstMEMSINFO, pNode);

	} else {
		pstTransLog->uiSessEndTime = pCAPHEAD->curtime;
		pstTransLog->uiSessEndMTime = pCAPHEAD->ucurtime;
		pstTransLog->uiSessDuration = GetGapTime(pstTransLog->uiSessEndTime, pstTransLog->uiSessEndMTime, 
				pstTransLog->uiSessStartTime, pstTransLog->uiSessStartMTime);

		LOG_SIGNAL_Prt("PRINT LOG_SIGNAL", pstTransLog);

		/*	SEND LOG */
		dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_L2TP, SEQ_PROC_A_CALL, nifo_offset( pstMEMSINFO, pNode ));
		if( dRet < 0 ){
			log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
			nifo_node_delete(pstMEMSINFO, pNode);
			return -1;
		}
	}

	/*	DELETE TRANS INFO */
	dRet = dManageControlSession( pstControlKey );

	return 0;
}


/*
 * dSendCDNSignal 
 * Send Call Stop Signal
 */
int dSendCDNSignal(Capture_Header_Msg *pCAPHEAD, st_CALLSESS_KEY *pstCallSessKey, st_CALLSESS_DATA *pstCallSessData, st_SESSID_KEY *pstSessionIDKey)
{
	int				 dRet;
	struct timeval		now;

	UCHAR				*pNode;
	LOG_SIGNAL			*pstTransLog;

	st_SESSID_KEY 		stReqSessIDKey;
	st_SESSID_KEY 		stResSessIDKey;

	stHASHONODE 		*pstControlHashNode;
	st_CONTROL_KEY 		*pstControlKey;
	st_CONTROL_DATA 	*pstControlData;
	
//	stHASHONODE			*pstTransHashNode;
//	st_SESSID_DATA		*pstTransData;

	pstTransLog = (LOG_SIGNAL *) nifo_ptr (pstMEMSINFO, pstCallSessData->dOffset);
	pNode = nifo_ptr (pstMEMSINFO, nifo_get_offset_node (pstMEMSINFO, (U8 *)pstTransLog));

	gettimeofday(&now, 0);

	/* COMMON */
	if ( pCAPHEAD==NULL ) {
//		pstTransLog->uiSessEndTime = now.tv_sec;
//		pstTransLog->uiSessEndMTime = now.tv_usec;
		pstTransLog->uiLastUserErrCode = L2TP_USER_TIMEOUT;

		nifo_node_delete(pstMEMSINFO, pNode);
	} else {
		pstTransLog->uiSessEndTime = pCAPHEAD->curtime;
		pstTransLog->uiSessEndMTime = pCAPHEAD->ucurtime;
		pstTransLog->uiSessDuration = GetGapTime(pstTransLog->uiSessEndTime, pstTransLog->uiSessEndMTime, 
				pstTransLog->uiSessStartTime, pstTransLog->uiSessStartMTime);

		LOG_SIGNAL_Prt("CDN LOG_SIGNAL", pstTransLog);

		/* SEND LOG */
		dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_L2TP, SEQ_PROC_A_CALL, nifo_offset( pstMEMSINFO, pNode ));
		if( dRet < 0 ){
			log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
			nifo_node_delete(pstMEMSINFO, pNode);
			return -1;
		}
	}

	/*	DELETE TRANS INFO */
	if (pstCallSessData->ulTimerNID != (long long)0) {
		timerN_del (pstTIMERNINFO, pstCallSessData->ulTimerNID);
	}

	log_print (LOGN_DEBUG, "	--> DEL CALLSESS HASH		SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d REQSID: %d RESSID: %d TID: %lld",
			HIPADDR(pstCallSessKey->uiSrcIP), HIPADDR(pstCallSessKey->uiDestIP), 
			pstCallSessKey->usReqSessID, pstCallSessKey->usResSessID, pstCallSessData->ulTimerNID);

	hasho_del (pstHASHOINFO, (U8 *)pstCallSessKey);

	/* DELETE SESSID HASH */
	memset(&stReqSessIDKey, 0x00, DEF_SESSID_KEY_SIZE);
	stReqSessIDKey.uiSrcIP = pstCallSessKey->uiSrcIP;
	stReqSessIDKey.uiDestIP = pstCallSessKey->uiDestIP;
	stReqSessIDKey.usSrcPort = 0;
	stReqSessIDKey.usDestPort = 0;
	stReqSessIDKey.usSessionID = pstCallSessKey->usReqSessID;
	memcpy(&stResSessIDKey, &stReqSessIDKey, DEF_SESSID_KEY_SIZE);
	stResSessIDKey.usSessionID = pstCallSessKey->usResSessID;

	log_print (LOGN_DEBUG, "	--> DEL SESSID HASH		SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d", 
			stReqSessIDKey.usSessionID, HIPADDR(stResSessIDKey.uiSrcIP), HIPADDR(stResSessIDKey.uiDestIP));
	hasho_del (pstSHASHOINFO, (U8 *)&stReqSessIDKey);
	log_print (LOGN_DEBUG, "	--> DEL SESSID HASH		SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d", 
			stResSessIDKey.usSessionID, HIPADDR(stResSessIDKey.uiSrcIP), HIPADDR(stResSessIDKey.uiDestIP));
	hasho_del (pstSHASHOINFO, (U8 *)&stResSessIDKey);


	pstControlKey = &pstCallSessData->stControlKey;
	log_print (LOGN_DEBUG, "	### CONTROL KEY INFO		TID: %d, %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d", 
			pstControlKey->usReqTunnelID, pstControlKey->usResTunnelID, HIPADDR(pstControlKey->uiSrcIP), HIPADDR(pstControlKey->uiDestIP));

	if( (pstControlHashNode = hasho_find (pstCHASHOINFO, (U8 *)pstControlKey)) == NULL ) {
		log_print(LOGN_CRI, "[%s][%s.%d] CONTROL hasho_find is NULL", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	} else {
		pstControlData = (st_CONTROL_DATA *) nifo_ptr (pstCHASHOINFO, pstControlHashNode->offset_Data);
		pstControlData->usTotCallsessCnt--;
	}

#if 0
	if((pstTransHashNode = hasho_find (pstSHASHOINFO, (U8*)&stReqSessIDKey)) != NULL) {
		pstTransData = (st_SESSID_DATA *) nifo_ptr (pstSHASHOINFO, pstTransHashNode->offset_Data);

		log_print (LOGN_DEBUG, "	--> DEL SESSID HASH		SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d TID: %lld",
				stReqSessIDKey.usSessionID, HIPADDR(stReqSessIDKey.uiSrcIP), HIPADDR(stReqSessIDKey.uiDestIP), pstTransData->ulTimerNID);
		timerN_del (pstSTIMERNINFO, pstTransData->ulTimerNID);
		hasho_del (pstSHASHOINFO, (U8 *)&stReqSessIDKey);
	}
	else {
		log_print (LOGN_CRI, "	--> DEL SESSID HASH NOT FIND		SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d TID: %lld",
				stReqSessIDKey.usSessionID, HIPADDR(stReqSessIDKey.uiSrcIP), HIPADDR(stReqSessIDKey.uiDestIP), pstTransData->ulTimerNID);
	}

	if((pstTransHashNode = hasho_find (pstSHASHOINFO, (U8*)&stResSessIDKey)) != NULL) {
		pstTransData = (st_SESSID_DATA *) nifo_ptr (pstSHASHOINFO, pstTransHashNode->offset_Data);

		log_print (LOGN_DEBUG, "	--> DEL SESSID HASH		SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d TID: %lld",
				stResSessIDKey.usSessionID, HIPADDR(stResSessIDKey.uiSrcIP), HIPADDR(stResSessIDKey.uiDestIP), pstTransData->ulTimerNID);
		timerN_del (pstSTIMERNINFO, pstTransData->ulTimerNID);
		hasho_del (pstSHASHOINFO, (U8 *)&stResSessIDKey);
	}
	else {
		log_print (LOGN_CRI, "	--> DEL SESSID HASH NOT FIND		SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d TID: %lld",
				stResSessIDKey.usSessionID, HIPADDR(stResSessIDKey.uiSrcIP), HIPADDR(stResSessIDKey.uiDestIP), pstTransData->ulTimerNID);
	}
#endif 

	return 0;
}


/* 
 * dSendStartSignal
 * START, INTERIM
 */
int dSendStartSignal(LOG_SIGNAL *pstTransLog)
{
	int 		dRet;
	UCHAR		*pstStartNode;
	LOG_SIGNAL	*pstStartCall;

	struct timeval	now;

	gettimeofday(&now, 0);

	/* ADD SIGNAL NIFO NODE */	
	if( (pstStartNode = nifo_node_alloc(pstMEMSINFO)) == NULL ) {
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	if( (pstStartCall = (LOG_SIGNAL *) 
				nifo_tlv_alloc(pstMEMSINFO, pstStartNode, LOG_SIGNAL_DEF_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_ON)) == NULL )
	{
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_delete(pstMEMSINFO, pstStartNode);
		return -2;
	}

	memcpy(pstStartCall, pstTransLog, LOG_SIGNAL_SIZE);

	LOG_SIGNAL_Prt("PRINT SIGNAL", pstStartCall);

	/* SEND Start CAll */
	dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_L2TP, SEQ_PROC_A_CALL, nifo_offset( pstMEMSINFO, pstStartNode ));
	if( dRet < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
		nifo_node_delete(pstMEMSINFO, pstStartNode);
		return -3;
	}

	log_print (LOGN_DEBUG, "	### SEND SIGNAL CLTIP: %d.%d.%d.%d MSI: %s REQTIME: %d RESTIME: %d CONTIME: %d", 
			HIPADDR(pstStartCall->uiClientIP), pstStartCall->szIMSI, 
			pstStartCall->uiL2TPReqTime, pstStartCall->uiL2TPRepTime, pstStartCall->uiL2TPConTime);

	return 0;
}

/*
 * szPrintMsgType
 *
 */
char *szPrintMsgType(USHORT MsgType)
{
	switch(MsgType)
	{
		case L2TP_MSG_ZLB:
			return "ZLB";
		case L2TP_MSG_SCCRQ:
			return "SCCRQ";
		case L2TP_MSG_SCCRP:
			return "SCCRP";
		case L2TP_MSG_SCCCN:
			return "SCCCN";
		case L2TP_MSG_StopCCN:
			return "StopCCN";
		case L2TP_MSG_Reserved:
			return "RESERVED";
		case L2TP_MSG_HELLO:
			return "HELLO";
		case L2TP_MSG_OCRQ:
			return "OCRQ";
		case L2TP_MSG_OCRP:
			return "OCRP";
		case L2TP_MSG_ORCRP:
			return "ORCRP";
		case L2TP_MSG_ICRQ:
			return "ICRQ";
		case L2TP_MSG_ICRP:
			return "ICRP";
		case L2TP_MSG_ICCN:
			return "ICCN";
		case L2TP_MSG_Reserved1:
			return "RESERVED1";
		case L2TP_MSG_CDN:
			return "CDN";
		default:
			return "UNDEFINED";
	}
}

/* 
 * pGetControlSessionData
 * 
 */
st_CONTROL_DATA *pGetControlSessionData(st_TUNNELID_KEY *pstTunnelIDKey)
{
	st_TUNNELID_DATA	stTunnelIDData;
	st_TUNNELID_DATA	*pstTunnelIDData = &stTunnelIDData;

	stHASHONODE			*pstTunnelIDHashNode;
	stHASHONODE			*pstControlHashNode;

	st_CONTROL_KEY 		*pstControlKey;
	st_CONTROL_DATA 	*pstControlData = NULL;


	if( (pstTunnelIDHashNode = hasho_find (pstTHASHOINFO, (U8 *)pstTunnelIDKey)) == NULL ) {

		log_print (LOGN_WARN, "NOT FOUND TUNNEL ID 	SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d TUNNELID: %d", 
				HIPADDR(pstTunnelIDKey->uiSrcIP), HIPADDR(pstTunnelIDKey->uiDestIP), pstTunnelIDKey->usTunnelID);

		return NULL;

	} else {
		pstTunnelIDData = (st_TUNNELID_DATA *) nifo_ptr (pstTHASHOINFO, pstTunnelIDHashNode->offset_Data);
		pstControlKey = &pstTunnelIDData->stCONTROLKey;

		if( (pstControlHashNode = hasho_find (pstCHASHOINFO, (U8 *)pstControlKey)) == NULL ) {

			log_print (LOGN_CRI, "CONTROL SESSION NOT FOUND SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d REQTID: %d RESTID: %d", 
					HIPADDR(pstControlKey->uiSrcIP), HIPADDR(pstControlKey->uiDestIP), 
					pstControlKey->usReqTunnelID, pstControlKey->usResTunnelID);

			return NULL;

		} else {
			pstControlData = (st_CONTROL_DATA *) nifo_ptr (pstCHASHOINFO, pstControlHashNode->offset_Data);
		}
	}

	return pstControlData;
}


/* 
 * dProcessTunnelMessages 
 *
 */
int dProcessTunnelMessages(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, st_TUNNELID_KEY *pstTunnelIDKey, st_L2TP_INFO *pstL2TPInfo)
{
	int 				dRet;

	st_TUNNELID_DATA	stTunnelIDData;
	st_TUNNELID_DATA	*pstTunnelIDData = &stTunnelIDData;

	st_TUNNELID_KEY 	stTunnelNewIDKey;
	st_TUNNELID_KEY 	*pstTunnelNewIDKey = &stTunnelNewIDKey;
	st_TUNNELID_DATA	stTunnelNewIDData;
	st_TUNNELID_DATA	*pstTunnelNewIDData = &stTunnelNewIDData;

	stHASHONODE			*pstTunnelIDHashNode;
	stHASHONODE			*pstControlHashNode;

	st_CONTROL_KEY 		*pstControlKey;
	st_CONTROL_DATA 	stControlData;
	st_CONTROL_DATA 	*pstControlData = &stControlData;

	UCHAR 				*pstSignalNode;
	LOG_SIGNAL 			*pstSignalLog;

	S32 				dIDTimer = 2;

	log_print (LOGN_DEBUG, "	--> MSG %s		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d PORT: %d, %d", 
			szPrintMsgType(pstL2TPInfo->usMessageType), pstTunnelIDKey->usTunnelID, 
			HIPADDR(pstTunnelIDKey->uiSrcIP), HIPADDR(pstTunnelIDKey->uiDestIP),
			pstTunnelIDKey->usSrcPort, pstTunnelIDKey->usDestPort);

	if( (pstTunnelIDHashNode = hasho_find (pstTHASHOINFO, (U8 *)pstTunnelIDKey)) == NULL ) {

		if(pstL2TPInfo->usMessageType == L2TP_MSG_SCCRQ) {
			memset(pstTunnelIDData, 0x00, DEF_TUNNELID_DATA_SIZE);

			pstTunnelIDData->stCONTROLKey.uiSrcIP = pstTunnelIDKey->uiSrcIP;
			pstTunnelIDData->stCONTROLKey.uiDestIP = pstTunnelIDKey->uiDestIP;
			pstTunnelIDData->stCONTROLKey.usReqTunnelID = pstTunnelIDKey->usTunnelID;
			pstTunnelIDData->ReqTunnelIDTime = pCAPHEAD->curtime;
			pstTunnelIDData->ReqTunnelIDMTime = pCAPHEAD->ucurtime;

			if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
				pstTunnelIDData->uiUpPktCnt++;
				pstTunnelIDData->uiUpPktBytes += pINFOETH->stIP.wTotalLength;
			} else {
				pstTunnelIDData->uiDnPktCnt++;
				pstTunnelIDData->uiDnPktBytes += pINFOETH->stIP.wTotalLength;
			}

			/*	ADD HASHO, TIMERN NODE */
			if ( (pstTunnelIDHashNode = hasho_add(pstTHASHOINFO, (U8*)pstTunnelIDKey, (U8*)pstTunnelIDData)) == NULL ) {
				log_print (LOGN_CRI, "[%s][%s.%d] hasho_add NULL ", __FILE__, __FUNCTION__, __LINE__);
				return -1;
			} else {
				pstTunnelIDData = (st_TUNNELID_DATA *) nifo_ptr (pstTHASHOINFO, pstTunnelIDHashNode->offset_Data);
				pstTunnelIDData->ulTimerNID = 
					timerN_add (pstTTIMERNINFO, (void *)&cb_timeout_tunnelid, (U8*)pstTunnelIDKey, DEF_TDATA_TUNNELID_TIMER_SIZE, time(NULL) + dIDTimer);

				pstTunnelIDData->usLastMessageType = pstL2TPInfo->usMessageType;

				log_print (LOGN_DEBUG, "	### ADD TUNNELID HASH		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d PORT: %d, %d OFFSET: %ld TID: %lld", 
						pstTunnelIDKey->usTunnelID, HIPADDR(pstTunnelIDKey->uiSrcIP), HIPADDR(pstTunnelIDKey->uiDestIP),
						pstTunnelIDKey->usSrcPort, pstTunnelIDKey->usDestPort,	pstTunnelIDData->dOffset, pstTunnelIDData->ulTimerNID);
			}
		} else {
			log_print (LOGN_CRI, "[%s][%s.%d] NOT EXIST HASH		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
					__FILE__, __FUNCTION__, __LINE__, 
					pstTunnelIDKey->usTunnelID, HIPADDR(pstTunnelIDKey->uiSrcIP), HIPADDR(pstTunnelIDKey->uiDestIP));

			return -2;
		}

	} else {
		/* EXIST TUNNELID HASH */
		pstTunnelIDData = (st_TUNNELID_DATA *) nifo_ptr (pstTHASHOINFO, pstTunnelIDHashNode->offset_Data);
		pstTunnelIDData->usLastMessageType = pstL2TPInfo->usMessageType;

		if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
			pstTunnelIDData->uiUpPktCnt++;
			pstTunnelIDData->uiUpPktBytes += pINFOETH->stIP.wTotalLength;
		} else {
			pstTunnelIDData->uiDnPktCnt++;
			pstTunnelIDData->uiDnPktBytes += pINFOETH->stIP.wTotalLength;
		}

		if (pstL2TPInfo->usMessageType == L2TP_MSG_SCCRQ) {
			log_print (LOGN_DEBUG, "	--> DUP %s		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
					szPrintMsgType(pstL2TPInfo->usMessageType), pstTunnelIDKey->usTunnelID, 
					HIPADDR(pstTunnelIDKey->uiSrcIP), HIPADDR(pstTunnelIDKey->uiDestIP));

			/* UPDATE DUP TUNNEL ID HASH */
			pstTunnelIDData->ReqTunnelIDTime = pCAPHEAD->curtime;
			pstTunnelIDData->ReqTunnelIDMTime = pCAPHEAD->ucurtime;

			pstTunnelIDData->ulTimerNID = timerN_update (pstTTIMERNINFO, pstTunnelIDData->ulTimerNID, time(NULL) + dIDTimer);

		} else if (pstL2TPInfo->usMessageType == L2TP_MSG_SCCRP) {

			pstTunnelIDData->stCONTROLKey.usResTunnelID = pstL2TPInfo->usAssignedTunnelID;
			pstTunnelIDData->ResTunnelIDTime = pCAPHEAD->curtime;
			pstTunnelIDData->ResTunnelIDMTime = pCAPHEAD->ucurtime;

			memcpy(pstTunnelNewIDData, pstTunnelIDData, DEF_TUNNELID_DATA_SIZE);
			memcpy(pstTunnelNewIDKey, pstTunnelIDKey, DEF_TUNNELID_KEY_SIZE);
			pstTunnelNewIDKey->usTunnelID = pstL2TPInfo->usAssignedTunnelID;

			if ( (pstTunnelIDHashNode = hasho_add(pstTHASHOINFO, (U8*)pstTunnelNewIDKey, (U8*)pstTunnelNewIDData)) == NULL ) {
				log_print (LOGN_CRI, "[%s][%s.%d] hasho_add NULL ", __FILE__, __FUNCTION__, __LINE__);
				return -3;
			} else {
				pstTunnelNewIDData = (st_TUNNELID_DATA *) nifo_ptr (pstTHASHOINFO, pstTunnelIDHashNode->offset_Data);
				pstTunnelNewIDData->ulTimerNID = 
					timerN_add (pstTTIMERNINFO, (void *)&cb_timeout_tunnelid, (U8*)pstTunnelNewIDKey, DEF_TDATA_TUNNELID_TIMER_SIZE, time(NULL) + dIDTimer);

				log_print (LOGN_DEBUG, "	### ADD TUNNELID HASH		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d PORT: %d, %d OFFSET: %ld", 
						pstTunnelNewIDKey->usTunnelID, HIPADDR(pstTunnelNewIDKey->uiSrcIP), HIPADDR(pstTunnelNewIDKey->uiDestIP), 
						pstTunnelNewIDKey->usSrcPort, pstTunnelNewIDKey->usDestPort, pstTunnelNewIDData->dOffset);
			}
		} else if (pstL2TPInfo->usMessageType == L2TP_MSG_SCCCN) {

			pstControlKey = &pstTunnelIDData->stCONTROLKey;
			if (pstControlKey->usReqTunnelID && pstControlKey->usResTunnelID) {

				/* ADD SIGNAL LOG */
				if( (pstSignalNode = nifo_node_alloc(pstMEMSINFO)) == NULL )
				{
					log_print (LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
					return -4;
				}
				if( (pstSignalLog = (LOG_SIGNAL *) nifo_tlv_alloc(
								pstMEMSINFO, pstSignalNode, LOG_SIGNAL_DEF_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_ON)) == NULL )
				{
					log_print (LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
					nifo_node_delete(pstMEMSINFO, pstSignalNode);
					return -5;
				}
				memset(pstControlData, 0x00, DEF_CONTROL_DATA_SIZE);
				pstControlData->dOffset = nifo_offset (pstMEMSINFO, pstSignalLog);
				memcpy(&pstControlData->stControlKey, pstControlKey, DEF_CONTROL_KEY_SIZE);

				pstControlData->uiReqTime = pstTunnelIDData->ReqTunnelIDTime;
				pstControlData->uiReqMTime = pstTunnelIDData->ReqTunnelIDMTime;
				pstControlData->uiResTime = pstTunnelIDData->ResTunnelIDTime;
				pstControlData->uiResMTime = pstTunnelIDData->ResTunnelIDMTime;
				pstControlData->uiConTime = pCAPHEAD->curtime;
				pstControlData->uiConMTime = pCAPHEAD->ucurtime;

				pstControlData->uiUpPktCnt = pstTunnelIDData->uiUpPktCnt;
				pstControlData->uiDnPktCnt	= pstTunnelIDData->uiDnPktCnt;
				pstControlData->uiUpPktBytes = pstTunnelIDData->uiUpPktBytes;
				pstControlData->uiDnPktBytes = pstTunnelIDData->uiDnPktBytes;

				/* ADD CONTROL HASH */
				if ( (pstControlHashNode = hasho_add(pstCHASHOINFO, (U8*)pstControlKey, (U8*)pstControlData)) == NULL ) {
					log_print (LOGN_CRI, "[%s][%s.%d] hasho_add NULL ", __FILE__, __FUNCTION__, __LINE__);
					nifo_node_delete(pstMEMSINFO, pstSignalNode);
					return -6;
				} else {
					/* Control Session Timer */
#ifdef SIM 
					gTIMER_TRANS = 20;
#else				 
					gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT]; 
#endif
					pstControlData = (st_CONTROL_DATA *) nifo_ptr (pstCHASHOINFO, pstControlHashNode->offset_Data);
					pstControlData->ulTimerNID = 
						timerN_add (pstCTIMERNINFO, (void *)&cb_timeout_Control, (U8*)pstControlKey, DEF_TDATA_CONTROL_TIMER_SIZE, time(NULL) + gTIMER_TRANS);

					log_print (LOGN_DEBUG, "	### ADD CONTROL HASH		REQTID: %d RESTID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d OFFSET: %ld", 
							pstControlKey->usReqTunnelID, pstControlKey->usResTunnelID,
							HIPADDR(pstControlKey->uiSrcIP), HIPADDR(pstControlKey->uiDestIP), pstControlData->dOffset);
				}

				/* COMMON SIGNAL INFO */
				pstSignalLog->uiCallTime 		= pstControlData->uiReqTime;
				pstSignalLog->uiCallMTime 		= pstControlData->uiReqMTime;
				pstSignalLog->uiAccStartTime 	= pstControlData->uiReqTime;
				pstSignalLog->uiAccStartMTime 	= pstControlData->uiReqMTime;
				pstSignalLog->uiSessStartTime 	= pstControlData->uiReqTime;
				pstSignalLog->uiSessStartMTime 	= pstControlData->uiReqMTime;
				pstSignalLog->uiSessEndTime 	= pstControlData->uiConTime;
				pstSignalLog->uiSessEndMTime 	= pstControlData->uiConMTime;

				pstSignalLog->ucBranchID 		= pINFOETH->usSysType;
				pstSignalLog->uiProtoType 		= DEF_PROTOCOL_L2TP;
				pstSignalLog->uiMsgType 		= MSG_L2TP_TUNNEL_START;
				pstSignalLog->szIMSI[0] 		= 0x30;
				pstSignalLog->szIMSI[1] 		= 0x00;

				pstSignalLog->uiSrcIP 			= pstControlKey->uiSrcIP;
				pstSignalLog->uiDestIP 			= pstControlKey->uiDestIP;
				pstSignalLog->uiL2TPReqTime 	= pstControlData->uiReqTime; 				/* SCCRQ, ICRQ Time */
				pstSignalLog->uiL2TPReqMTime 	= pstControlData->uiReqMTime;
				pstSignalLog->uiL2TPRepTime 	= pstControlData->uiResTime; 				/* SCCRP, ICRP Time */
				pstSignalLog->uiL2TPRepMTime 	= pstControlData->uiResMTime;
				pstSignalLog->uiL2TPConTime		= pstControlData->uiConTime; 				/* SCCCN, ICCN Time */
				pstSignalLog->uiL2TPConMTime	= pstControlData->uiConMTime;
				pstSignalLog->uiSessDuration 	= GetGapTime(pstSignalLog->uiL2TPConTime, pstSignalLog->uiL2TPConMTime, 
						pstSignalLog->uiL2TPReqTime, pstSignalLog->uiL2TPReqMTime);

				pstSignalLog->usReqTunnelID 	= pstControlKey->usReqTunnelID; 			/* LAC Tunnel ID */
				pstSignalLog->usRepTunnelID 	= pstControlKey->usResTunnelID; 			/* LNS Tunnel ID */
				pstSignalLog->usReqSessID 		= 0; 										/* LAC Session ID */
				pstSignalLog->usRepSessID 		= 0; 										/* LNS Session ID */
				pstSignalLog->usResultCode 		= 0; 										/* CDN, StopCCN Result Code */
				pstSignalLog->usErrorCode 		= 0;

				pstSignalLog->uiUpL2TPPkts 		= pstControlData->uiUpPktCnt;
				pstSignalLog->uiDnL2TPPkts		= pstControlData->uiDnPktCnt;
				pstSignalLog->uiUpL2TPBytes		= pstControlData->uiUpPktBytes;
				pstSignalLog->uiDnL2TPBytes		= pstControlData->uiDnPktBytes;

				dRet = dSendStartSignal(pstSignalLog);
					
				/* TUNNELID TIMER DELETE */
				log_print (LOGN_DEBUG, "	### DELETE TUNNELID TIMER 	SID: %d TID: %lld", pstTunnelIDKey->usTunnelID, pstTunnelIDData->ulTimerNID);
				timerN_del (pstTTIMERNINFO, pstTunnelIDData->ulTimerNID);
				pstTunnelIDData->ulTimerNID = 0;

				memcpy(pstTunnelNewIDKey, pstTunnelIDKey, DEF_TUNNELID_KEY_SIZE);
				if (pstControlKey->usReqTunnelID == pstTunnelIDKey->usTunnelID) {
					pstTunnelNewIDKey->usTunnelID = pstControlKey->usResTunnelID;
				} else {
					pstTunnelNewIDKey->usTunnelID = pstControlKey->usReqTunnelID;
				}
				if( (pstTunnelIDHashNode = hasho_find(pstTHASHOINFO, (U8 *)pstTunnelNewIDKey)) == NULL) {
					log_print (LOGN_CRI, "TUNNELID KEY NOT FOUND 	TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d", 
							pstTunnelNewIDKey->usTunnelID, HIPADDR(pstTunnelNewIDKey->uiSrcIP), HIPADDR(pstTunnelNewIDKey->uiDestIP));
				} else {
					pstTunnelNewIDData = (st_TUNNELID_DATA *) nifo_ptr(pstTHASHOINFO, pstTunnelIDHashNode->offset_Data);

					log_print (LOGN_DEBUG, "	### DELETE TUNNELID TIMER 	SID: %d TID: %lld", pstTunnelNewIDKey->usTunnelID, pstTunnelNewIDData->ulTimerNID);
					timerN_del (pstTTIMERNINFO, pstTunnelNewIDData->ulTimerNID);
				}
			} else {
				log_print (LOGN_CRI, "CONTROL SESS KEY NOT COMPLETE SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d REQTID: %d RESTID: %d", 
						HIPADDR(pstControlKey->uiSrcIP), HIPADDR(pstControlKey->uiDestIP), 
						pstControlKey->usReqTunnelID, pstControlKey->usResTunnelID);

				return -7;
			}

		} else if (pstL2TPInfo->usMessageType == L2TP_MSG_StopCCN) {
			/* CONTROL SESSION CLEAR */
			log_print (LOGN_DEBUG, "	--> MSG %s		SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
					szPrintMsgType(pstL2TPInfo->usMessageType), HIPADDR(pstTunnelIDKey->uiSrcIP), HIPADDR(pstTunnelIDKey->uiDestIP));

			pstControlKey = &pstTunnelIDData->stCONTROLKey;
			if( (pstControlHashNode = hasho_find (pstCHASHOINFO, (U8 *)pstControlKey)) == NULL ) {

				log_print (LOGN_CRI, "CONTROL SESS NOT FOUND SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d REQTID: %d RESTID: %d", 
						HIPADDR(pstControlKey->uiSrcIP), HIPADDR(pstControlKey->uiDestIP), 
						pstControlKey->usReqTunnelID, pstControlKey->usResTunnelID);

				return -8;

			} else {
				pstControlData = (st_CONTROL_DATA *) nifo_ptr (pstCHASHOINFO, pstControlHashNode->offset_Data);
				pstSignalLog = (LOG_SIGNAL *) nifo_ptr (pstMEMSINFO, pstControlData->dOffset);

				pstControlData->uiStopTime = pCAPHEAD->curtime;
				pstControlData->uiStopMTime = pCAPHEAD->ucurtime;

				if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
					pstControlData->uiUpPktCnt++;
					pstControlData->uiUpPktBytes += pINFOETH->stIP.wTotalLength;
				} else {
					pstControlData->uiDnPktCnt++;
					pstControlData->uiDnPktBytes += pINFOETH->stIP.wTotalLength;
				}

				/* COMMON SIGNAL INFO */
				pstSignalLog->uiMsgType = MSG_L2TP_TUNNEL_STOP;
				pstSignalLog->szIMSI[0] = 0x30;
				pstSignalLog->szIMSI[1] = 0x00;

				pstSignalLog->uiSessStartTime 	= pstControlData->uiStopTime;	
				pstSignalLog->uiSessStartMTime 	= pstControlData->uiStopMTime;
				pstSignalLog->uiSessEndTime 	= pstControlData->uiStopTime;	
				pstSignalLog->uiSessEndMTime 	= pstControlData->uiStopMTime;
				pstSignalLog->usResultCode 		= pstL2TPInfo->usResultCode;
				pstSignalLog->usErrorCode 		= pstL2TPInfo->usErrorCode;
				pstSignalLog->uiSessDuration 	= GetGapTime(pstSignalLog->uiSessEndTime, pstSignalLog->uiSessEndMTime, 
						pstSignalLog->uiL2TPReqTime, pstSignalLog->uiL2TPReqMTime);
				pstSignalLog->uiLastUserErrCode	= 0;

				pstSignalLog->uiUpL2TPPkts = pstControlData->uiUpPktCnt;
				pstSignalLog->uiDnL2TPPkts = pstControlData->uiDnPktCnt;
				pstSignalLog->uiUpL2TPBytes = pstControlData->uiUpPktBytes;
				pstSignalLog->uiDnL2TPBytes = pstControlData->uiDnPktBytes;

				dRet = dSendStopCCNSignal(pCAPHEAD, pstControlKey, pstControlData, pstTunnelIDKey);

			}

		} else if (pstL2TPInfo->usMessageType == L2TP_MSG_HELLO) {
			/* CONTROL SESSION INTERIM */
			log_print (LOGN_DEBUG, "	--> MSG %s		SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
					szPrintMsgType(pstL2TPInfo->usMessageType), HIPADDR(pstTunnelIDKey->uiSrcIP), HIPADDR(pstTunnelIDKey->uiDestIP));

			pstControlKey = &pstTunnelIDData->stCONTROLKey;
			if( (pstControlHashNode = hasho_find (pstCHASHOINFO, (U8 *)pstControlKey)) == NULL ) {

				log_print (LOGN_CRI, "CONTROL SESS NOT FOUND SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d REQTID: %d RESTID: %d", 
						HIPADDR(pstControlKey->uiSrcIP), HIPADDR(pstControlKey->uiDestIP), 
						pstControlKey->usReqTunnelID, pstControlKey->usResTunnelID);

				return -9;

			} else {
				pstControlData = (st_CONTROL_DATA *) nifo_ptr (pstCHASHOINFO, pstControlHashNode->offset_Data);
				pstSignalLog = (LOG_SIGNAL *) nifo_ptr (pstMEMSINFO, pstControlData->dOffset);
#ifdef SIM 
				gTIMER_TRANS = 20;
#else				 
				gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
#endif
				pstControlData->ulTimerNID = timerN_update (pstCTIMERNINFO, pstControlData->ulTimerNID, time(NULL) + gTIMER_TRANS);

				if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
					pstControlData->uiUpPktCnt++;
					pstControlData->uiUpPktBytes += pINFOETH->stIP.wTotalLength;
				} else {
					pstControlData->uiDnPktCnt++;
					pstControlData->uiDnPktBytes += pINFOETH->stIP.wTotalLength;
				}

				/* COMMON SIGNAL INFO */
				pstSignalLog->uiMsgType = MSG_L2TP_TUNNEL_INTERIM;
				pstSignalLog->szIMSI[0] = 0x30;
				pstSignalLog->szIMSI[1] = 0x00;

				pstSignalLog->uiUpL2TPPkts = pstControlData->uiUpPktCnt;
				pstSignalLog->uiDnL2TPPkts = pstControlData->uiDnPktCnt;
				pstSignalLog->uiUpL2TPBytes = pstControlData->uiUpPktBytes;
				pstSignalLog->uiDnL2TPBytes = pstControlData->uiDnPktBytes;

				dRet = dSendStartSignal(pstSignalLog);
			}

		} else if (pstL2TPInfo->usMessageType == L2TP_MSG_ZLB) {
			if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
				pstControlData->uiUpPktCnt++;
				pstControlData->uiUpPktBytes += pINFOETH->stIP.wTotalLength;
			} else {
				pstControlData->uiDnPktCnt++;
				pstControlData->uiDnPktBytes += pINFOETH->stIP.wTotalLength;
			}
		} else {
			log_print (LOGN_CRI, "UNDEFINED L2TP MESSAGE TYPE: %d", pstL2TPInfo->usMessageType);
		}

	}

	return 1;
}

/*
 * dCheck_TraceInfo 
 *
 */
int dCheck_TraceInfo( st_SESSID_DATA *pstSessData, UCHAR *pData, Capture_Header_Msg *pstCAP )
{
	int	 i, dRet;		
	UCHAR	*pstNode;
	UCHAR	*pBuffer;

	int 	offset;

	st_TraceMsgHdr	*pstTrcMsg;

	/*	
	 * pstSessData->szIMSI		: 4500010~
	 * pstSessData->szTraceMIN	: 010~
	 */

	for( i=0; i< pstTRACE->count; i++ ) {
		if( pstTRACE->stTraceInfo[i].dType == TRC_TYPE_IMSI ) {

			if( atoll((char*)pstSessData->stCALLSESSKey.szIMSI) == pstTRACE->stTraceInfo[i].stTraceID.llIMSI ) {

				log_print( LOGN_INFO, "	@@@ TRACE IMSI[%d] %s, %s", i, pstSessData->stCALLSESSKey.szIMSI, pstTRACE->stTraceInfo[i].stTraceID.szMIN );
				/* SEND TRACE PACKET */
				if( (pstNode = nifo_node_alloc( pstMEMSINFO )) == NULL ) {
					log_print( LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
					return -1;
				}

				if( (pstTrcMsg = (st_TraceMsgHdr *)nifo_tlv_alloc( pstMEMSINFO, pstNode, st_TraceMsgHdr_DEF_NUM, st_TraceMsgHdr_SIZE, DEF_MEMSET_OFF)) == NULL ) {
					log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, st_TraceMsgHdr_DEF_NUM );
					nifo_node_delete(pstMEMSINFO, pstNode);
					return -2;
				}

				if( (pBuffer = nifo_tlv_alloc( pstMEMSINFO, pstNode, ETH_DATA_NUM, pstCAP->datalen, DEF_MEMSET_OFF)) == NULL ) {
					log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, ETH_DATA_NUM);
					nifo_node_delete(pstMEMSINFO, pstNode);
					return -3;
				}

				pstTrcMsg->time		 = pstCAP->curtime;
				pstTrcMsg->mtime		= pstCAP->ucurtime;
				pstTrcMsg->dType		= pstTRACE->stTraceInfo[i].dType;
				pstTrcMsg->usDataLen	= pstCAP->datalen;
				memcpy( &pstTrcMsg->stTraceID, &pstTRACE->stTraceInfo[i].stTraceID, st_TraceID_SIZE );
				memcpy( pBuffer, pData, pstCAP->datalen );

				dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_L2TP, SEQ_PROC_CI_LOG, nifo_offset( pstMEMSINFO, pstNode ));
				if( dRet < 0 ){
					log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
					nifo_node_delete(pstMEMSINFO, pstNode);
					return -4;
				}

				return 1;
			}
		}
		else if( pstTRACE->stTraceInfo[i].dType == TRC_TYPE_ROAM_IMSI || pstTRACE->stTraceInfo[i].dType == TRC_TYPE_ROAM_MDN) {
			// IRM 타입일 경우 IMSI의 뒷자리 10자리를 atoll 로 변환하여 TRACE정보의 llIMSI 값과 비교

			if(strlen((char*)pstSessData->stCALLSESSKey.szIMSI) < DEF_IRM_LEN) {
				log_print(LOGN_CRI, "[%s][%s.%d] MDN SIZE IS LESS THAN %d [%s", __FILE__, __FUNCTION__, __LINE__, DEF_IRM_LEN, pstSessData->stCALLSESSKey.szIMSI);
				return 1;
			}
			offset = strlen((char*)pstSessData->stCALLSESSKey.szIMSI) - DEF_IRM_LEN;
			log_print(LOGN_DEBUG, "	@@@ TRACE IRM: %lld OFFSET: %d", atoll((char*)&pstSessData->stCALLSESSKey.szIMSI[offset]), offset);
			if( atoll((char*)&pstSessData->stCALLSESSKey.szIMSI[offset]) == pstTRACE->stTraceInfo[i].stTraceID.llIMSI) {

				// SEND TRACE PACKET 
				if( (pstNode = nifo_node_alloc( pstMEMSINFO )) == NULL ) {
					log_print( LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
					return -1;
				}

				if( (pstTrcMsg = (st_TraceMsgHdr *)nifo_tlv_alloc( pstMEMSINFO, pstNode, st_TraceMsgHdr_DEF_NUM, st_TraceMsgHdr_SIZE, DEF_MEMSET_OFF)) == NULL ) {
					log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, st_TraceMsgHdr_DEF_NUM );
					nifo_node_delete(pstMEMSINFO, pstNode);
					return -2;
				}

				if( (pBuffer = nifo_tlv_alloc( pstMEMSINFO, pstNode, ETH_DATA_NUM, pstCAP->datalen, DEF_MEMSET_OFF)) == NULL ) {
					log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, ETH_DATA_NUM);
					nifo_node_delete(pstMEMSINFO, pstNode);
					return -3;
				}

				pstTrcMsg->time		 = pstCAP->curtime;
				pstTrcMsg->mtime		= pstCAP->ucurtime;
				pstTrcMsg->dType		= pstTRACE->stTraceInfo[i].dType;
				pstTrcMsg->usDataLen	= pstCAP->datalen;
				memcpy( &pstTrcMsg->stTraceID, &pstTRACE->stTraceInfo[i].stTraceID, st_TraceID_SIZE );
				memcpy( pBuffer, pData, pstCAP->datalen );

				dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_L2TP, SEQ_PROC_CI_LOG, nifo_offset( pstMEMSINFO, pstNode ));
				if( dRet < 0 ){
					log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
					nifo_node_delete(pstMEMSINFO, pstNode);
					return -4;
				}

				return 1;
			}
		}
		else
			continue;
	}

	return 0;
}

/*
 * dManageControlSession
 *
 */
int dManageControlSession( st_CONTROL_KEY *pstControlKey )
{
	int 	dRet=0;

	st_CONTROL_DATA		*pstControlData;
	stHASHONODE			*pstControlHashNode;

	stHASHONODE 		*pstTransHashNode;
	st_TUNNELID_DATA 	*pstTransData;

	st_TUNNELID_KEY 	stReqTunnelIDKey;
	st_TUNNELID_KEY 	stResTunnelIDKey;


	if( (pstControlHashNode = hasho_find (pstCHASHOINFO, (U8 *)pstControlKey)) == NULL ) {
		log_print(LOGN_CRI, "[%s][%s.%d] CONTROL hasho_find is NULL", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	} else {
		pstControlData = (st_CONTROL_DATA *) nifo_ptr (pstCHASHOINFO, pstControlHashNode->offset_Data);

		if(pstControlData->usTotCallsessCnt==0) {
			// Delete Control and TunnelID HASH 
			log_print (LOGN_DEBUG, "	### DEL CONTROL HASH		REQTID: %d RESQID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d TID: %lld",
					pstControlKey->usReqTunnelID, pstControlKey->usResTunnelID,
					HIPADDR(pstControlKey->uiSrcIP), HIPADDR(pstControlKey->uiDestIP), pstControlData->ulTimerNID);

			if (pstControlData->ulTimerNID != (long long)0) {
				timerN_del (pstCTIMERNINFO, pstControlData->ulTimerNID);
				pstControlData->ulTimerNID = 0;
			}
			hasho_del (pstCHASHOINFO, (U8 *)pstControlKey);

			/* DELETE TUNNELID INFO */
			memset(&stReqTunnelIDKey, 0x00, DEF_TUNNELID_KEY_SIZE);
			stReqTunnelIDKey.uiSrcIP = pstControlKey->uiSrcIP;
			stReqTunnelIDKey.uiDestIP = pstControlKey->uiDestIP;
			stReqTunnelIDKey.usSrcPort = 0;
			stReqTunnelIDKey.usDestPort = 0;
			stReqTunnelIDKey.usTunnelID = pstControlKey->usReqTunnelID;
			memcpy(&stResTunnelIDKey, &stReqTunnelIDKey, DEF_TUNNELID_KEY_SIZE);
			stResTunnelIDKey.usTunnelID = pstControlKey->usResTunnelID;

			log_print (LOGN_DEBUG, "	### DELETE TUNNELID HASH		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
					stReqTunnelIDKey.usTunnelID, HIPADDR(stReqTunnelIDKey.uiSrcIP), HIPADDR(stReqTunnelIDKey.uiDestIP));
			if((pstTransHashNode = hasho_find (pstTHASHOINFO, (U8*)&stReqTunnelIDKey)) != NULL) {
				pstTransData = (st_TUNNELID_DATA *) nifo_ptr (pstTHASHOINFO, pstTransHashNode->offset_Data);
				if (pstTransData->ulTimerNID != (long long)0) {
					timerN_del (pstTTIMERNINFO, pstTransData->ulTimerNID);
					pstTransData->ulTimerNID = 0;
				}
				hasho_del (pstTHASHOINFO, (U8 *)&stReqTunnelIDKey);
			}
			else {
				log_print (LOGN_CRI, "	### TUNNELID HASH NOT FIND		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
						stReqTunnelIDKey.usTunnelID, HIPADDR(stReqTunnelIDKey.uiSrcIP), HIPADDR(stReqTunnelIDKey.uiDestIP));
			}

			log_print (LOGN_DEBUG, "	### DELETE TUNNELID HASH		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
					stResTunnelIDKey.usTunnelID, HIPADDR(stResTunnelIDKey.uiSrcIP), HIPADDR(stResTunnelIDKey.uiDestIP));
			if((pstTransHashNode = hasho_find (pstTHASHOINFO, (U8*)&stResTunnelIDKey)) != NULL) {
				pstTransData = (st_TUNNELID_DATA *) nifo_ptr (pstTHASHOINFO, pstTransHashNode->offset_Data);
				if (pstTransData->ulTimerNID != (long long)0) {
					timerN_del (pstTTIMERNINFO, pstTransData->ulTimerNID);
					pstTransData->ulTimerNID = 0;
				}
				hasho_del (pstTHASHOINFO, (U8 *)&stResTunnelIDKey);
			}
			else {
				log_print (LOGN_CRI, "	### TUNNELID HASH NOT FIND		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
						stResTunnelIDKey.usTunnelID, HIPADDR(stResTunnelIDKey.uiSrcIP), HIPADDR(stResTunnelIDKey.uiDestIP));
			}
		} else {
			log_print (LOGN_DEBUG, "	### KEEP CONTROL HASH		TOT: %d REQTID: %d RESQID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d TID: %lld",
					pstControlData->usTotCallsessCnt, pstControlKey->usReqTunnelID, pstControlKey->usResTunnelID,
					HIPADDR(pstControlKey->uiSrcIP), HIPADDR(pstControlKey->uiDestIP), pstControlData->ulTimerNID);
		}
	}


	return dRet;
}

/*
 * dDeleteControlSession
 * 터널제어세션이 TIMEOUT이 되면 터널제어세션을 삭제한다. 
 */
int dDeleteControlSession( st_CONTROL_KEY *pstControlKey )
{
	int 	dRet=0;

	st_CONTROL_DATA		*pstControlData;
	stHASHONODE			*pstControlHashNode;

	stHASHONODE 		*pstTransHashNode;
	st_TUNNELID_DATA 	*pstTransData;

	st_TUNNELID_KEY 	stReqTunnelIDKey;
	st_TUNNELID_KEY 	stResTunnelIDKey;


	if( (pstControlHashNode = hasho_find (pstCHASHOINFO, (U8 *)pstControlKey)) == NULL ) {
		log_print(LOGN_CRI, "[%s][%s.%d] CONTROL hasho_find is NULL", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	} else {
		pstControlData = (st_CONTROL_DATA *) nifo_ptr (pstCHASHOINFO, pstControlHashNode->offset_Data);

		// Delete Control and TunnelID HASH 
		log_print (LOGN_DEBUG, "	### DEL CONTROL HASH		REQTID: %d RESQID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d TID: %lld",
				pstControlKey->usReqTunnelID, pstControlKey->usResTunnelID,
				HIPADDR(pstControlKey->uiSrcIP), HIPADDR(pstControlKey->uiDestIP), pstControlData->ulTimerNID);

		if (pstControlData->ulTimerNID != (long long)0) {
			timerN_del (pstCTIMERNINFO, pstControlData->ulTimerNID);
			pstControlData->ulTimerNID = 0;
		}
		hasho_del (pstCHASHOINFO, (U8 *)pstControlKey);

		/* DELETE TUNNELID INFO */
		memset(&stReqTunnelIDKey, 0x00, DEF_TUNNELID_KEY_SIZE);
		stReqTunnelIDKey.uiSrcIP = pstControlKey->uiSrcIP;
		stReqTunnelIDKey.uiDestIP = pstControlKey->uiDestIP;
		stReqTunnelIDKey.usSrcPort = 0;
		stReqTunnelIDKey.usDestPort = 0;
		stReqTunnelIDKey.usTunnelID = pstControlKey->usReqTunnelID;
		memcpy(&stResTunnelIDKey, &stReqTunnelIDKey, DEF_TUNNELID_KEY_SIZE);
		stResTunnelIDKey.usTunnelID = pstControlKey->usResTunnelID;

		log_print (LOGN_DEBUG, "	### DELETE TUNNELID HASH		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
				stReqTunnelIDKey.usTunnelID, HIPADDR(stReqTunnelIDKey.uiSrcIP), HIPADDR(stReqTunnelIDKey.uiDestIP));
		if((pstTransHashNode = hasho_find (pstTHASHOINFO, (U8*)&stReqTunnelIDKey)) != NULL) {
			pstTransData = (st_TUNNELID_DATA *) nifo_ptr (pstTHASHOINFO, pstTransHashNode->offset_Data);
			if (pstTransData->ulTimerNID != (long long)0) {
				timerN_del (pstTTIMERNINFO, pstTransData->ulTimerNID);
				pstTransData->ulTimerNID = 0;
			}
			hasho_del (pstTHASHOINFO, (U8 *)&stReqTunnelIDKey);
		}
		else {
			log_print (LOGN_CRI, "	### TUNNELID HASH NOT FIND		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
					stReqTunnelIDKey.usTunnelID, HIPADDR(stReqTunnelIDKey.uiSrcIP), HIPADDR(stReqTunnelIDKey.uiDestIP));
		}

		log_print (LOGN_DEBUG, "	### DELETE TUNNELID HASH		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
				stResTunnelIDKey.usTunnelID, HIPADDR(stResTunnelIDKey.uiSrcIP), HIPADDR(stResTunnelIDKey.uiDestIP));
		if((pstTransHashNode = hasho_find (pstTHASHOINFO, (U8*)&stResTunnelIDKey)) != NULL) {
			pstTransData = (st_TUNNELID_DATA *) nifo_ptr (pstTHASHOINFO, pstTransHashNode->offset_Data);
			if (pstTransData->ulTimerNID != (long long)0) {
				timerN_del (pstTTIMERNINFO, pstTransData->ulTimerNID);
				pstTransData->ulTimerNID = 0;
			}
			hasho_del (pstTHASHOINFO, (U8 *)&stResTunnelIDKey);
		}
		else {
			log_print (LOGN_CRI, "	### TUNNELID HASH NOT FIND		TID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
					stResTunnelIDKey.usTunnelID, HIPADDR(stResTunnelIDKey.uiSrcIP), HIPADDR(stResTunnelIDKey.uiDestIP));
		}
	}

	return dRet;
}


/*
 * dProcessSessionMessages
 * ICRQ, ICRP, ICCN, CDN Message 
 *
 */
int dProcessSessionMessages(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, st_SESSID_KEY *pstSessionIDKey, st_TUNNELID_KEY *pstTunnelIDKey, st_L2TP_INFO *pstL2TPInfo)
{
	int 				dRet;

	st_SESSID_DATA		stSessionIDData;
	st_SESSID_DATA		*pstSessionIDData = &stSessionIDData;

	st_SESSID_KEY		stSessionNewIDKey;
	st_SESSID_KEY		*pstSessionNewIDKey = &stSessionNewIDKey;
	st_SESSID_DATA		stSessionNewIDData;
	st_SESSID_DATA		*pstSessionNewIDData = &stSessionNewIDData;

	stHASHONODE			*pstSessionIDHashNode;
	stHASHONODE			*pstCallSessHashNode;
	stHASHONODE 		*p;

	st_CALLSESS_KEY 	*pstCallSessKey;
	st_CONTROL_KEY 		*pstControlKey;
	st_CALLSESS_DATA 	stCallSessData;
	st_CALLSESS_DATA 	*pstCallSessData = &stCallSessData;

	st_CONTROL_DATA 	*pstControlData = NULL;

	UCHAR 				*pstSignalNode;
	LOG_SIGNAL 			*pstSignalLog;

	S32 				dIDTimer = 2;



	log_print (LOGN_DEBUG, "	--> MSG %s		SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
			szPrintMsgType(pstL2TPInfo->usMessageType), pstSessionIDKey->usSessionID,
			HIPADDR(pstSessionIDKey->uiSrcIP), HIPADDR(pstSessionIDKey->uiDestIP));

	if ( (pstControlData = pGetControlSessionData(pstTunnelIDKey)) == NULL ) {
		return -1;
	} else {
		// UPDATE CONTROL SESSION 
#ifdef SIM 
		gTIMER_TRANS = 20;
#else				 
		gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
#endif		 
		pstControlData->ulTimerNID = timerN_update (pstCTIMERNINFO, pstControlData->ulTimerNID, time(NULL) + gTIMER_TRANS);

		if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
			pstControlData->uiUpPktCnt++;
			pstControlData->uiUpPktBytes += pINFOETH->stIP.wTotalLength;
		} else {
			pstControlData->uiDnPktCnt++;
			pstControlData->uiDnPktBytes += pINFOETH->stIP.wTotalLength;
		}
	}

	if( (pstSessionIDHashNode = hasho_find (pstSHASHOINFO, (U8 *)pstSessionIDKey)) == NULL ) {

		if(pstL2TPInfo->usMessageType == L2TP_MSG_ICRQ) {
			memset(pstSessionIDData, 0x00, DEF_SESSID_DATA_SIZE);

			pstSessionIDData->stCALLSESSKey.uiSrcIP = pstSessionIDKey->uiSrcIP;
			pstSessionIDData->stCALLSESSKey.uiDestIP = pstSessionIDKey->uiDestIP;
			pstSessionIDData->stCALLSESSKey.usReqSessID = pstSessionIDKey->usSessionID;
			memcpy(pstSessionIDData->stCALLSESSKey.szIMSI, pstL2TPInfo->szCallingNumber, MAX_MIN_LEN);
			pstSessionIDData->szIMSI[MAX_MIN_LEN] = 0x00;
			pstSessionIDData->ucBranchID = pINFOETH->usSysType;

			pstSessionIDData->ReqSessionIDTime = pCAPHEAD->curtime;
			pstSessionIDData->ReqSessionIDMTime = pCAPHEAD->ucurtime;

			if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
				pstSessionIDData->uiUpPktCnt++;
				pstSessionIDData->uiUpPktBytes += pINFOETH->stIP.wTotalLength;
			} else {
				pstSessionIDData->uiDnPktCnt++;
				pstSessionIDData->uiDnPktBytes += pINFOETH->stIP.wTotalLength;
			}

			/*	ADD HASHO, TIMERN NODE */
			if ( (pstSessionIDHashNode = hasho_add(pstSHASHOINFO, (U8*)pstSessionIDKey, (U8*)pstSessionIDData)) == NULL ) {
				log_print (LOGN_CRI, "[%s][%s.%d] hasho_add NULL ", __FILE__, __FUNCTION__, __LINE__);
				return -2;
			} else {
				pstSessionIDData = (st_SESSID_DATA *) nifo_ptr (pstSHASHOINFO, pstSessionIDHashNode->offset_Data);
				pstSessionIDData->ulTimerNID = 
					timerN_add (pstSTIMERNINFO, (void *)&cb_timeout_Sessid, (U8*)pstSessionIDKey, DEF_TDATA_SESSID_TIMER_SIZE, time(NULL) + dIDTimer);

				pstSessionIDData->usLastMessageType = pstL2TPInfo->usMessageType;

				log_print (LOGN_DEBUG, "	### ADD SESSID HASH		SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d OFFSET: %ld TID: %lld", 
						pstSessionIDKey->usSessionID, HIPADDR(pstSessionIDKey->uiSrcIP), HIPADDR(pstSessionIDKey->uiDestIP), 
						pstSessionIDData->dOffset, pstSessionIDData->ulTimerNID);
			}
		} else {
			log_print (LOGN_CRI, "[%s][%s.%d] NOT EXIST SESSID HASH", __FILE__, __FUNCTION__, __LINE__);

			return -2;
		}
	} else {
		/* EXIST SESSIONID HASH */
		pstSessionIDData = (st_SESSID_DATA *) nifo_ptr (pstSHASHOINFO, pstSessionIDHashNode->offset_Data);
		pstSessionIDData->usLastMessageType = pstL2TPInfo->usMessageType;

		if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
			pstSessionIDData->uiUpPktCnt++;
			pstSessionIDData->uiUpPktBytes += pINFOETH->stIP.wTotalLength;
		} else {
			pstSessionIDData->uiDnPktCnt++;
			pstSessionIDData->uiDnPktBytes += pINFOETH->stIP.wTotalLength;
		}

		if (pstL2TPInfo->usMessageType == L2TP_MSG_ICRQ) {
			log_print (LOGN_WARN, "	--> DUP %s 		SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d",
					szPrintMsgType(pstL2TPInfo->usMessageType), pstSessionIDKey->usSessionID,
					HIPADDR(pstSessionIDKey->uiSrcIP), HIPADDR(pstSessionIDKey->uiDestIP));

			/* UPDATE DUP SESSION ID HASH */
			pstSessionIDData->ReqSessionIDTime = pCAPHEAD->curtime;
			pstSessionIDData->ReqSessionIDMTime = pCAPHEAD->ucurtime;
			pstSessionIDData->ucBranchID = pINFOETH->usSysType;

			pstSessionIDData->ulTimerNID = timerN_update (pstSTIMERNINFO, pstSessionIDData->ulTimerNID, time(NULL) + dIDTimer);

		} else if (pstL2TPInfo->usMessageType == L2TP_MSG_ICRP) {

			pstSessionIDData->stCALLSESSKey.usResSessID = pstL2TPInfo->usAssignedSessionID;
			pstSessionIDData->ResSessionIDTime = pCAPHEAD->curtime;
			pstSessionIDData->ResSessionIDMTime = pCAPHEAD->ucurtime;

			pstSessionIDData->ulTimerNID = timerN_update (pstSTIMERNINFO, pstSessionIDData->ulTimerNID, time(NULL) + dIDTimer);

			memcpy(pstSessionNewIDData, pstSessionIDData, DEF_SESSID_DATA_SIZE);
			memcpy(pstSessionNewIDKey, pstSessionIDKey, DEF_SESSID_KEY_SIZE);
			pstSessionNewIDKey->usSessionID = pstL2TPInfo->usAssignedSessionID;

			if ( (pstSessionIDHashNode = hasho_add(pstSHASHOINFO, (U8*)pstSessionNewIDKey, (U8*)&stSessionNewIDData)) == NULL ) {
				log_print (LOGN_CRI, "[%s][%s.%d] hasho_add NULL ", __FILE__, __FUNCTION__, __LINE__);
				return -4;
			} else {
				pstSessionNewIDData = (st_SESSID_DATA *) nifo_ptr (pstSHASHOINFO, pstSessionIDHashNode->offset_Data);
				pstSessionNewIDData->ulTimerNID = 
					timerN_add (pstSTIMERNINFO, (void *)&cb_timeout_Sessid, (U8*)pstSessionNewIDKey, DEF_TDATA_SESSID_TIMER_SIZE, time(NULL) + dIDTimer);

				log_print (LOGN_DEBUG, "	### ADD SESSID HASH		SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d OFFSET: %ld TID: %lld", 
						pstSessionNewIDKey->usSessionID, HIPADDR(pstSessionNewIDKey->uiSrcIP), HIPADDR(pstSessionNewIDKey->uiDestIP), 
						pstSessionNewIDData->dOffset, pstSessionNewIDData->ulTimerNID);
			}

		} else if (pstL2TPInfo->usMessageType == L2TP_MSG_ICCN) {
			log_print (LOGN_DEBUG, "	--> MSG %s		IMSI: %s SID: %d, %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d", 
					szPrintMsgType(pstL2TPInfo->usMessageType), pstSessionIDData->stCALLSESSKey.szIMSI,
					pstSessionIDData->stCALLSESSKey.usReqSessID, pstSessionIDData->stCALLSESSKey.usResSessID,
					HIPADDR(pstSessionIDKey->uiSrcIP), HIPADDR(pstSessionIDKey->uiDestIP));

			pstCallSessKey = &pstSessionIDData->stCALLSESSKey;
			pstControlKey = &pstControlData->stControlKey;
			if (pstCallSessKey->usReqSessID && pstCallSessKey->usResSessID) {

				/* ADD SIGNAL LOG */
				if( (pstSignalNode = nifo_node_alloc(pstMEMSINFO)) == NULL )
				{
					log_print (LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
					return -5;
				}
				if( (pstSignalLog = (LOG_SIGNAL *) nifo_tlv_alloc( 
								pstMEMSINFO, pstSignalNode, LOG_SIGNAL_DEF_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_ON)) == NULL )
				{
					log_print (LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
					nifo_node_delete(pstMEMSINFO, pstSignalNode);
					return -6;
				}
				
				memset(pstCallSessData, 0x00, DEF_CALLSESS_DATA_SIZE);
				pstCallSessData->dOffset = nifo_offset (pstMEMSINFO, pstSignalLog);
				memcpy(&pstCallSessData->stCallSessKey, pstCallSessKey, DEF_CALLSESS_KEY_SIZE);
				memcpy(&pstCallSessData->stControlKey, pstControlKey, DEF_CONTROL_KEY_SIZE);
				memcpy(pstCallSessData->szIMSI, pstCallSessKey->szIMSI, MAX_MIN_SIZE);

				pstCallSessData->uiReqTime = pstSessionIDData->ReqSessionIDTime;
				pstCallSessData->uiReqMTime = pstSessionIDData->ReqSessionIDMTime;
				pstCallSessData->uiResTime = pstSessionIDData->ResSessionIDTime;
				pstCallSessData->uiResMTime = pstSessionIDData->ResSessionIDMTime;
				pstCallSessData->uiConTime = pCAPHEAD->curtime;
				pstCallSessData->uiConMTime = pCAPHEAD->ucurtime;
				pstCallSessData->ucBranchID = pINFOETH->usSysType;

				pstCallSessData->uiUpPktCnt = pstSessionIDData->uiUpPktCnt;
				pstCallSessData->uiDnPktCnt = pstSessionIDData->uiDnPktCnt;
				pstCallSessData->uiUpPktBytes = pstSessionIDData->uiUpPktBytes;
				pstCallSessData->uiDnPktBytes = pstSessionIDData->uiDnPktBytes;

				/* ADD CALLSESS */
				if ( (pstCallSessHashNode = hasho_add(pstHASHOINFO, (U8*)pstCallSessKey, (U8*)pstCallSessData)) == NULL ) {
					log_print (LOGN_CRI, "[%s][%s.%d] hasho_add NULL ", __FILE__, __FUNCTION__, __LINE__);
					nifo_node_delete(pstMEMSINFO, pstSignalNode);
					return -7;
				} else {
					pstControlData->usTotCallsessCnt++;

					/* Call Session Timer */
#ifdef SIM 
					gTIMER_TRANS = 20;
#else				 
					gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
#endif
					pstCallSessData = (st_CALLSESS_DATA *) nifo_ptr (pstHASHOINFO, pstCallSessHashNode->offset_Data);
					pstCallSessData->ulTimerNID = 
						timerN_add (pstTIMERNINFO, (void *)&cb_timeout_CallSess, (U8*)pstCallSessKey, DEF_TDATA_CALLSESS_TIMER_SIZE, time(NULL) + gTIMER_TRANS);
					/* SESSID TIMER UPDATE */

					log_print (LOGN_DEBUG, "	### ADD CALLSESS HASH		IMSI: %s REQSID: %d RESSID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d OFFSET: %ld", 
							pstCallSessKey->szIMSI, pstCallSessKey->usReqSessID, pstCallSessKey->usResSessID,
							HIPADDR(pstCallSessKey->uiSrcIP), HIPADDR(pstCallSessKey->uiDestIP), pstCallSessData->dOffset);

					/* LCP Flag Setting */
					pstCallSessData->usPPPFlag = 0;
					if (pstL2TPInfo->ucSendLCPFlag) {
						pstCallSessData->usPPPFlag |= 0x0001; 		/* UP LCP */
						pstCallSessData->usPPPFlag |= 0x0008; 		/* DN ACK */
					} 
					if (pstL2TPInfo->ucSendLCPFlag) {
						pstCallSessData->usPPPFlag |= 0x0004; 		/* DN LCP */
						pstCallSessData->usPPPFlag |= 0x0002; 		/* UP ACK */
					} 

					/* CHAP Flag Setting */
					if (pstL2TPInfo->ucCHAPChalFlag) {
						pstCallSessData->usPPPFlag |= 0x0100; 		/* CHAP BASE */
						pstCallSessData->usPPPFlag |= 0x0200; 		/* CHAP CHAL */
						pstCallSessData->CHAPAP.uiID = pstL2TPInfo->ucAUTHID;
						pstCallSessData->CHAPAP.ucFlag = 0x01;

						pstCallSessData->CHAPAP.StartTime = pCAPHEAD->curtime;
						pstCallSessData->CHAPAP.StartMTime = pCAPHEAD->ucurtime;
					}
					if (pstL2TPInfo->ucCHAPRespFlag) {
						pstCallSessData->usPPPFlag |= 0x0400; 		/* CHAP RESP */
					}

					/* COMMON SIGNAL INFO */
					pstSignalLog->uiCallTime 		= pstCallSessData->uiReqTime;
					pstSignalLog->uiCallMTime 		= pstCallSessData->uiReqMTime;
					pstSignalLog->uiAccStartTime 	= pstCallSessData->uiReqTime;
					pstSignalLog->uiAccStartMTime 	= pstCallSessData->uiReqMTime;
					pstSignalLog->uiSessStartTime 	= pstCallSessData->uiReqTime;
					pstSignalLog->uiSessStartMTime 	= pstCallSessData->uiReqMTime;
					pstSignalLog->uiSessEndTime 	= pstCallSessData->uiConTime;
					pstSignalLog->uiSessEndMTime 	= pstCallSessData->uiConMTime;

					memcpy(pstSignalLog->szIMSI, pstCallSessData->szIMSI, MAX_MIN_LEN);
					pstSignalLog->szIMSI[MAX_MIN_LEN] = 0x00;

					pstSignalLog->ucBranchID		= pINFOETH->usSysType;
					pstSignalLog->uiProtoType 		= DEF_PROTOCOL_L2TP;
					pstSignalLog->uiMsgType 		= MSG_L2TP_CALL_START;

					pstSignalLog->uiSrcIP 			= pstCallSessKey->uiSrcIP;
					pstSignalLog->uiDestIP 			= pstCallSessKey->uiDestIP;
					pstSignalLog->uiL2TPReqTime 	= pstCallSessData->uiReqTime; 				/* SCCRQ, ICRQ Time */
					pstSignalLog->uiL2TPReqMTime 	= pstCallSessData->uiReqMTime;
					pstSignalLog->uiL2TPRepTime 	= pstCallSessData->uiResTime; 				/* SCCRP, ICRP Time */
					pstSignalLog->uiL2TPRepMTime 	= pstCallSessData->uiResMTime;
					pstSignalLog->uiL2TPConTime		= pstCallSessData->uiConTime; 				/* SCCCN, ICCN Time */
					pstSignalLog->uiL2TPConMTime	= pstCallSessData->uiConMTime;
					pstSignalLog->uiSessDuration 	= GetGapTime(pstSignalLog->uiL2TPConTime, pstSignalLog->uiL2TPConMTime, 
							pstSignalLog->uiL2TPReqTime, pstSignalLog->uiL2TPReqMTime);

					pstSignalLog->usReqSessID 		= pstCallSessKey->usReqSessID; 				/* LAC Tunnel ID */
					pstSignalLog->usRepSessID 		= pstCallSessKey->usResSessID; 				/* LNS Tunnel ID */
					pstSignalLog->usReqTunnelID		= pstControlKey->usReqTunnelID;				/* LAC Session ID */
					pstSignalLog->usRepTunnelID		= pstControlKey->usResTunnelID;				/* LNS Session ID */
					pstSignalLog->usResultCode 		= 0; 										/* CDN, StopCCN Result Code */
					pstSignalLog->usErrorCode 		= 0;										/* CDN, StopCCN Result Code */ 

					pstSignalLog->uiUpL2TPPkts = pstCallSessData->uiUpPktCnt;
					pstSignalLog->uiDnL2TPPkts = pstCallSessData->uiDnPktCnt;
					pstSignalLog->uiUpL2TPBytes = pstCallSessData->uiUpPktBytes;
					pstSignalLog->uiDnL2TPBytes = pstCallSessData->uiDnPktBytes;


					dRet = dSendStartSignal(pstSignalLog);

					/* SESSID TIMER DELETE */
					log_print (LOGN_DEBUG, "	### DELETE SESSIONID TIMER 	SID: %d TID: %lld", pstSessionIDKey->usSessionID, pstSessionIDData->ulTimerNID);
					timerN_del (pstSTIMERNINFO, pstSessionIDData->ulTimerNID);
					pstSessionIDData->ulTimerNID = 0;

//					gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
//					pstSessionIDData->ulTimerNID = timerN_update (pstSTIMERNINFO, pstSessionIDData->ulTimerNID, time(NULL) + gTIMER_TRANS);

					memcpy(pstSessionNewIDKey, pstSessionIDKey, DEF_SESSID_KEY_SIZE);
					if ( pstCallSessKey->usReqSessID == pstSessionIDKey->usSessionID ) {
						pstSessionNewIDKey->usSessionID = pstCallSessKey->usResSessID;
					} else {
						pstSessionNewIDKey->usSessionID = pstCallSessKey->usReqSessID;
					}
					if( (pstSessionIDHashNode = hasho_find (pstSHASHOINFO, (U8 *)pstSessionNewIDKey)) == NULL ) {
						log_print (LOGN_CRI, "SESSIONID KEY NOT FOUND 	SID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d", 
								pstSessionNewIDKey->usSessionID, HIPADDR(pstCallSessKey->uiSrcIP), HIPADDR(pstCallSessKey->uiDestIP));
					} else {
						pstSessionNewIDData = (st_SESSID_DATA *) nifo_ptr (pstSHASHOINFO, pstSessionIDHashNode->offset_Data);

						timerN_del (pstSTIMERNINFO, pstSessionNewIDData->ulTimerNID);
						log_print (LOGN_DEBUG, "	### DELETE SESSIONID TIMER 	SID: %d TID: %lld", pstSessionNewIDKey->usSessionID, pstSessionNewIDData->ulTimerNID);
//						pstSessionNewIDData->ulTimerNID = timerN_update (pstSTIMERNINFO, pstSessionNewIDData->ulTimerNID, time(NULL) + gTIMER_TRANS);
					}
				} /* END ADD CALLSESS */
			} else {
				log_print (LOGN_CRI, "SESSIONID KEY NOT COMPLETE 	REQSID: %d RESSID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d", 
						pstCallSessKey->usReqSessID, pstCallSessKey->usResSessID, 
						HIPADDR(pstCallSessKey->uiSrcIP), HIPADDR(pstCallSessKey->uiDestIP));
	
				/* DELETE EXIST SESSID HASH */
				log_print (LOGN_DEBUG, "	### DELETE SESSIONID HASH 	SID: %d", pstSessionIDKey->usSessionID);
				hasho_del (pstSHASHOINFO, (U8 *)pstSessionIDKey);
				if (pstCallSessKey->usReqSessID == pstSessionIDKey->usSessionID) {
					pstSessionIDKey->usSessionID = pstCallSessKey->usResSessID;
				} else {
					pstSessionIDKey->usSessionID = pstCallSessKey->usReqSessID;
				}
				if(!(p = hasho_find(pstSHASHOINFO, (UCHAR *)pstSessionIDKey))) {
					log_print (LOGN_WARN, "	### DELETE SESSIONID HASH 	SID: %d", pstSessionIDKey->usSessionID);
					hasho_del (pstSHASHOINFO, (U8 *)pstSessionIDKey);
				}	
				
				/* Checking Control Session */
				dRet = dManageControlSession( pstControlKey );

				return -8;
			}

		} else if (pstL2TPInfo->usMessageType == L2TP_MSG_CDN) {
			/* CALL SESSION CLEAR */

			pstCallSessKey = &pstSessionIDData->stCALLSESSKey;
			if( (pstCallSessHashNode = hasho_find (pstHASHOINFO, (U8 *)pstCallSessKey)) != NULL ) {
				pstCallSessData = (st_CALLSESS_DATA *) nifo_ptr (pstHASHOINFO, pstCallSessHashNode->offset_Data);
				pstSignalLog = (LOG_SIGNAL *) nifo_ptr (pstMEMSINFO, pstCallSessData->dOffset);

				pstCallSessData->uiStopTime = pCAPHEAD->curtime;
				pstCallSessData->uiStopMTime = pCAPHEAD->ucurtime;

				if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
					pstCallSessData->uiUpPktCnt++;
					pstCallSessData->uiUpPktBytes += pINFOETH->stIP.wTotalLength;
				} else {
					pstCallSessData->uiDnPktCnt++;
					pstCallSessData->uiDnPktBytes += pINFOETH->stIP.wTotalLength;
				}

				/* COMMON SIGNAL INFO */
				pstSignalLog->uiMsgType = MSG_L2TP_CALL_STOP;

				pstSignalLog->uiSessStartTime = pstCallSessData->uiStopTime;	
				pstSignalLog->uiSessStartMTime = pstCallSessData->uiStopMTime;
				pstSignalLog->uiSessEndTime = pstCallSessData->uiStopTime;	
				pstSignalLog->uiSessEndMTime = pstCallSessData->uiStopMTime;
				pstSignalLog->usResultCode = pstL2TPInfo->usResultCode;
				pstSignalLog->usErrorCode = pstL2TPInfo->usErrorCode;

				pstSignalLog->uiUpL2TPPkts = pstCallSessData->uiUpPktCnt;
				pstSignalLog->uiDnL2TPPkts = pstCallSessData->uiDnPktCnt;
				pstSignalLog->uiUpL2TPBytes = pstCallSessData->uiUpPktBytes;
				pstSignalLog->uiDnL2TPBytes = pstCallSessData->uiDnPktBytes;

				/* Send Stop Signal */
				dRet = dSendCDNSignal(pCAPHEAD, pstCallSessKey, pstCallSessData, pstSessionIDKey);

			} else {
				log_print (LOGN_CRI, "CALL SESS NOT FOUND		IMSI: %s REQTID: %d RESTID: %d SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d ", 
						pstCallSessKey->szIMSI, pstCallSessKey->usReqSessID, pstCallSessKey->usResSessID, 
						HIPADDR(pstCallSessKey->uiSrcIP), HIPADDR(pstCallSessKey->uiDestIP));

				return -9;
			}

		} else {
			log_print (LOGN_WARN, "UNDEFINED MESSAGE TYPE: %d", pstL2TPInfo->usMessageType);

			return -10;
		}
	}

	if ((dRet = dCheck_TraceInfo(pstSessionIDData, pDATA, pCAPHEAD)) < 0) {
			log_print (LOGN_CRI, "[%s.%d] FAILED dCheck_TraceInfo dRet:[%d]", __FUNCTION__, __LINE__, dRet);
	}

	return 1;
}

 
/*
 * Report_SIGLog 
 *
 * Send PPP CHAP Success 
 * Send PPP IPCP Transaction 
 * Send PPP LCP Termination Transaction
 */
int Report_SIGLog( UCHAR ucProtoType, UCHAR ucMsgType, st_CALLSESS_DATA *pPSessData )
{
	int			 dRet; 
	U8				szIPAddr[32];
	UCHAR			*pstNode; 
	LOG_SIGNAL		*pstSIGLog;

	UINT			uiDuration; 
	//st_SIGNAL_Log	*pstSIGLog;

	if( (pstNode = nifo_node_alloc(pstMEMSINFO)) == NULL ) {
		log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
		return -1;
	}

	if( (pstSIGLog = (LOG_SIGNAL *)nifo_tlv_alloc(pstMEMSINFO, pstNode, LOG_SIGNAL_DEF_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_OFF)) == NULL ) {
		log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, LOG_SIGNAL_DEF_NUM);
		nifo_node_delete(pstMEMSINFO, pstNode);
		return -2;
	}

	memset( pstSIGLog, 0x00, LOG_SIGNAL_SIZE );

	/* SET COMMON INFO */
	pstSIGLog->usReqSessID 		= pPSessData->stCallSessKey.usReqSessID;
	pstSIGLog->usRepSessID 		= pPSessData->stCallSessKey.usResSessID;
	pstSIGLog->uiClientIP 		= pPSessData->uiIPAddr;
	pstSIGLog->ucBranchID 		= pPSessData->ucBranchID;
//	pstSIGLog->uiNASName 		= CVT_UINT(pPSessData->uiNASIP);

/* 
	pstSIGLog->ucSYSID		= (pPSessData->stBSMSC.SYS_ID);
	pstSIGLog->ucBSCID		= (pPSessData->stBSMSC.BSC_ID);
	pstSIGLog->ucBTSID		= CVT_USHORT(pPSessData->stBSMSC.BTS_ID);
	pstSIGLog->ucFA_ID		= (pPSessData->stBSMSC.FA_ID);
	pstSIGLog->ucSECTOR	 = (pPSessData->stBSMSC.SEC_ID);
	
	memcpy(&pstSIGLog->szBSMSC[0], &pPSessData->ucBSMSC[4], 8 );
	memcpy(&pstSIGLog->szBSMSC[8], &pPSessData->ucBSMSC[0], 4 );
	
	log_print( LOGN_INFO, "SYSID:%u BSCID:%u BTSID:%u FAID:%u SEC:%u BSMSC:%s",
	pstSIGLog->ucSYSID, pstSIGLog->ucBSCID, pstSIGLog->ucBTSID, 
	pstSIGLog->ucFA_ID, pstSIGLog->ucSECTOR, pstSIGLog->szBSMSC );
	
	pstSIGLog->uiSvcOption	= (pPSessData->uiSvcOption);
	memcpy( pstSIGLog->szNetOption, pPSessData->szNetOption, MAX_SVCOPTION_SIZE );
*/
	switch( ucProtoType )
	{
/*
		case START_CALL_NUM:
		case NUM_UDP_A11 :
		pstSIGLog->uiCallTime		= pPSessData->uiReqTime;
		pstSIGLog->uiCallMTime		= pPSessData->uiReqMTime;
		memcpy( pstSIGLog->szIMSI, pPSessData->szIMSI, MAX_MIN_SIZE );
		memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
		
		pstSIGLog->uiProtoType		= ucProtoType;
		pstSIGLog->uiMsgType		= pPSessData->A11.ucReqMsgCode;
		
		pstSIGLog->uiSessStartTime	= (pPSessData->A11.StartTime);
		pstSIGLog->uiSessStartMTime = (pPSessData->A11.StartMTime);
		
		pstSIGLog->uiSessEndTime	= (pPSessData->A11.EndTime);
		pstSIGLog->uiSessEndMTime	= (pPSessData->A11.EndMTime);
		uiDuration = GetGapTime( pPSessData->A11.StartTime, pPSessData->A11.StartMTime,
		pPSessData->A11.EndTime, pPSessData->A11.EndMTime );
		
		pstSIGLog->uiSrcIP = CVT_UINT(pPSessData->A11.uiSrcIP);
		pstSIGLog->uiDestIP = CVT_UINT(pPSessData->A11.uiDestIP);
		
		pstSIGLog->uiSessDuration	= (uiDuration);
		pstSIGLog->uiRespCode		= pPSessData->A11.ucRepMsgCode;
		
		//pstSIGLog->ucADR			= pPSessData->ucRegiReply;
		
		//pstSIGLog->usReqCount = (pPSessData->A11.ucReqCount);
		//pstSIGLog->usNakCount = (pPSessData->A11.ucNakCount);
		//pstSIGLog->usRejCount = (pPSessData->A11.ucRejCount);
		
		pstSIGLog->uiUpGREPkts	= pPSessData->uiUpGREFrames;
		pstSIGLog->uiDnGREPkts	= pPSessData->uiDownGREFrames;
		pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
		pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;
		
		if( ucMsgType == 0x01 )
		{
		pstSIGLog->usLiftTime	= (pPSessData->usLifetime);
		pstSIGLog->ucAppType	= pPSessData->ucAppType;
		if( pPSessData->ucAppType != 0 )
		{
		pstSIGLog->uiFMux		= (pPSessData->uiFMux);
		pstSIGLog->uiRMux		= (pPSessData->uiRMux);
		pstSIGLog->ucAirLink	= pPSessData->ucAirlink;
		}
		}
		else if( ucMsgType == 0x14 )
		{
		pstSIGLog->usUpdateReason	= (pPSessData->uiUpdateReason);
		}
		
		break;
*/

		case NUM_UP_LCP :
			pstSIGLog->uiCallTime		= pPSessData->uiReqTime;
			pstSIGLog->uiCallMTime		= pPSessData->uiReqMTime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szIMSI, MAX_MIN_SIZE );
			memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );

/*
			pstSIGLog->uiUpGREPkts	= pPSessData->uiUpGREFrames;
			pstSIGLog->uiDnGREPkts	= pPSessData->uiDownGREFrames;
			pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
			pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;
*/

			pstSIGLog->uiProtoType		= ucProtoType;
			if( ucMsgType == DEF_LCP_CONF_REQ ) /* configure-req */
			{
				pstSIGLog->uiMsgType		= pPSessData->UpLCP.ucReqMsgCode;
				pstSIGLog->uiSessStartTime	= (pPSessData->UpLCP.StartTime);
				pstSIGLog->uiSessStartMTime = (pPSessData->UpLCP.StartMTime);
				pstSIGLog->uiSessEndTime	= (pPSessData->UpLCP.EndTime);
				pstSIGLog->uiSessEndMTime	= (pPSessData->UpLCP.EndMTime);
				uiDuration = GetGapTime( pPSessData->UpLCP.StartTime, pPSessData->UpLCP.StartMTime,
						pPSessData->UpLCP.EndTime, pPSessData->UpLCP.EndMTime );

				pstSIGLog->uiRespCode		= pPSessData->UpLCP.ucRepMsgCode;
				pstSIGLog->ucPPPReqCnt		= (pPSessData->UpLCP.ucReqCount);
				pstSIGLog->ucPPPNakCnt		= (pPSessData->UpLCP.ucNakCount);
				pstSIGLog->ucPPPRejCnt		= (pPSessData->UpLCP.ucRejCount);
			}
			else if( ucMsgType == DEF_LCP_TERM_REQ ) /* term */
			{
				pstSIGLog->uiMsgType		= pPSessData->LCPTerm.ucReqMsgCode;
				pstSIGLog->uiSessStartTime	= (pPSessData->LCPTerm.StartTime);
				pstSIGLog->uiSessStartMTime = (pPSessData->LCPTerm.StartMTime);
				pstSIGLog->uiSessEndTime	= (pPSessData->LCPTerm.EndTime);
				pstSIGLog->uiSessEndMTime	= (pPSessData->LCPTerm.EndMTime);

				uiDuration = GetGapTime( pPSessData->LCPTerm.StartTime, pPSessData->LCPTerm.StartMTime,
						pPSessData->LCPTerm.EndTime, pPSessData->LCPTerm.EndMTime );
				pstSIGLog->uiRespCode		= pPSessData->LCPTerm.ucRepMsgCode;
				pstSIGLog->ucPPPReqCnt		= (pPSessData->LCPTerm.ucReqCount);
				pstSIGLog->ucPPPNakCnt		= (pPSessData->LCPTerm.ucNakCount);
				pstSIGLog->ucPPPRejCnt		= (pPSessData->LCPTerm.ucRejCount);
			}
			else if( ucMsgType == DEF_LCP_ECHO_REQ )
			{
				pstSIGLog->uiMsgType		= pPSessData->UpLCPEcho.ucReqMsgCode;
				pstSIGLog->uiSessStartTime	= (pPSessData->UpLCPEcho.StartTime);
				pstSIGLog->uiSessStartMTime = (pPSessData->UpLCPEcho.StartMTime);
				pstSIGLog->uiSessEndTime	= (pPSessData->UpLCPEcho.EndTime);
				pstSIGLog->uiSessEndMTime	= (pPSessData->UpLCPEcho.EndMTime);
				uiDuration = GetGapTime( pPSessData->UpLCPEcho.StartTime, pPSessData->UpLCPEcho.StartMTime,
						pPSessData->UpLCPEcho.EndTime, pPSessData->UpLCPEcho.EndMTime );

/*
				pstSIGLog->uiRespCode		= pPSessData->UpLCPEcho.ucRepMsgCode;
				pstSIGLog->ucPPPReqCnt		= (pPSessData->UpLCPEcho.ucReqCount);
				pstSIGLog->ucPPPNakCnt		= (pPSessData->UpLCPEcho.ucNakCount);
				pstSIGLog->ucPPPRejCnt		= (pPSessData->UpLCPEcho.ucRejCount);
*/
			}
			else
			{
				log_print( LOGN_WARN, "Unknown ProtoType[%u], MsgType[%u]", ucProtoType, ucMsgType );
				nifo_node_delete(pstMEMSINFO, pstNode);
				return -1;
			}

			pstSIGLog->uiSessDuration	= (uiDuration);

			pstSIGLog->uiSrcIP			= pPSessData->stCallSessKey.uiSrcIP;
			pstSIGLog->uiDestIP		 = pPSessData->stCallSessKey.uiDestIP;

			break;

		case NUM_DOWN_LCP :
			pstSIGLog->uiCallTime		= pPSessData->uiReqTime;
			pstSIGLog->uiCallMTime		= pPSessData->uiReqMTime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szIMSI, MAX_MIN_SIZE );
			memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
			pstSIGLog->uiProtoType		= ucProtoType;

/*
			pstSIGLog->uiUpGREPkts	= pPSessData->uiUpGREFrames;
			pstSIGLog->uiDnGREPkts	= pPSessData->uiDownGREFrames;
			pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
			pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;
*/

			if( ucMsgType == DEF_LCP_CONF_REQ ) /* configure-request */
			{
				pstSIGLog->uiMsgType		= pPSessData->DownLCP.ucReqMsgCode;
				pstSIGLog->uiSessStartTime	= (pPSessData->DownLCP.StartTime);
				pstSIGLog->uiSessStartMTime = (pPSessData->DownLCP.StartMTime);
				pstSIGLog->uiSessEndTime	= (pPSessData->DownLCP.EndTime);
				pstSIGLog->uiSessEndMTime	= (pPSessData->DownLCP.EndMTime);
				uiDuration = GetGapTime( pPSessData->DownLCP.StartTime, pPSessData->DownLCP.StartMTime,
						pPSessData->DownLCP.EndTime, pPSessData->DownLCP.EndMTime );
				pstSIGLog->uiRespCode		= pPSessData->DownLCP.ucRepMsgCode;
				pstSIGLog->ucPPPReqCnt		= (pPSessData->DownLCP.ucReqCount);
				pstSIGLog->ucPPPNakCnt		= (pPSessData->DownLCP.ucNakCount);
				pstSIGLog->ucPPPRejCnt		= (pPSessData->DownLCP.ucRejCount);
			}
			else if( ucMsgType == DEF_LCP_TERM_REQ ) /* term */
			{
				pstSIGLog->uiMsgType		= pPSessData->LCPTerm.ucReqMsgCode;
				pstSIGLog->uiSessStartTime	= (pPSessData->LCPTerm.StartTime);
				pstSIGLog->uiSessStartMTime = (pPSessData->LCPTerm.StartMTime);
				pstSIGLog->uiSessEndTime	= (pPSessData->LCPTerm.EndTime);
				pstSIGLog->uiSessEndMTime	= (pPSessData->LCPTerm.EndMTime);
				uiDuration = GetGapTime( pPSessData->LCPTerm.StartTime, pPSessData->LCPTerm.StartMTime,
						pPSessData->LCPTerm.EndTime, pPSessData->LCPTerm.EndMTime );
				pstSIGLog->uiRespCode		= pPSessData->LCPTerm.ucRepMsgCode;
				pstSIGLog->ucPPPReqCnt		= (pPSessData->LCPTerm.ucReqCount);
				pstSIGLog->ucPPPNakCnt		= (pPSessData->LCPTerm.ucNakCount);
				pstSIGLog->ucPPPRejCnt		= (pPSessData->LCPTerm.ucRejCount);
			}
			else if( ucMsgType == DEF_LCP_ECHO_REQ )
			{
				pstSIGLog->uiMsgType		= pPSessData->DownLCPEcho.ucReqMsgCode;
				pstSIGLog->uiSessStartTime	= (pPSessData->DownLCPEcho.StartTime);
				pstSIGLog->uiSessStartMTime = (pPSessData->DownLCPEcho.StartMTime);
				pstSIGLog->uiSessEndTime	= (pPSessData->DownLCPEcho.EndTime);
				pstSIGLog->uiSessEndMTime	= (pPSessData->DownLCPEcho.EndMTime);
				uiDuration = GetGapTime( pPSessData->DownLCPEcho.StartTime, pPSessData->DownLCPEcho.StartMTime,
						pPSessData->DownLCPEcho.EndTime, pPSessData->DownLCPEcho.EndMTime );

/*
			pstSIGLog->uiRespCode		= pPSessData->DownLCP.ucRepMsgCode;
			pstSIGLog->ucPPPReqCnt		= (pPSessData->DownLCP.ucReqCount);
			pstSIGLog->ucPPPNakCnt		= (pPSessData->DownLCP.ucNakCount);
			pstSIGLog->ucPPPRejCnt		= (pPSessData->DownLCP.ucRejCount);
*/
			}
			else
			{
				log_print( LOGN_WARN, "Unknown ProtoType[%u], MsgType[%u]", ucProtoType, ucMsgType );
				nifo_node_delete(pstMEMSINFO, pstNode);
				return -1;
			}
			pstSIGLog->uiSessDuration	= (uiDuration);

//			pstSIGLog->uiSrcIP			= (pPSessData->uiHomeAgent);
			pstSIGLog->uiSrcIP 			= pPSessData->stCallSessKey.uiDestIP;
			pstSIGLog->uiDestIP		 = pPSessData->stCallSessKey.uiSrcIP;

			break;

		case NUM_UP_IPCP :
			pstSIGLog->uiCallTime		= pPSessData->uiReqTime;
			pstSIGLog->uiCallMTime		= pPSessData->uiReqMTime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szIMSI, MAX_MIN_SIZE );
			memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
			pstSIGLog->uiProtoType		= ucProtoType;
			pstSIGLog->uiMsgType		= pPSessData->UpIPCP.ucReqMsgCode;
/*
			pstSIGLog->uiUpGREPkts	= pPSessData->uiUpGREFrames;
			pstSIGLog->uiDnGREPkts	= pPSessData->uiDownGREFrames;
			pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
			pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;
*/
			pstSIGLog->uiSessStartTime	= (pPSessData->UpIPCP.StartTime);
			pstSIGLog->uiSessStartMTime = (pPSessData->UpIPCP.StartMTime);
			pstSIGLog->uiSessEndTime	= (pPSessData->UpIPCP.EndTime);
			pstSIGLog->uiSessEndMTime	= (pPSessData->UpIPCP.EndMTime);
			uiDuration = GetGapTime( pPSessData->UpIPCP.StartTime, pPSessData->UpIPCP.StartMTime,
					pPSessData->UpIPCP.EndTime, pPSessData->UpIPCP.EndMTime );
			pstSIGLog->uiSessDuration	= (uiDuration);

			pstSIGLog->uiSrcIP = pPSessData->stCallSessKey.uiSrcIP;
			pstSIGLog->uiDestIP = pPSessData->stCallSessKey.uiDestIP;
//			pstSIGLog->uiDestIP = (pPSessData->uiHomeAgent);

			pstSIGLog->uiRespCode		= pPSessData->UpIPCP.ucRepMsgCode;
			pstSIGLog->ucPPPReqCnt		= (pPSessData->UpIPCP.ucReqCount);
			pstSIGLog->ucPPPNakCnt		= (pPSessData->UpIPCP.ucNakCount);
			pstSIGLog->ucPPPRejCnt		= (pPSessData->UpIPCP.ucRejCount);
			pstSIGLog->uiClientIP		= (pPSessData->uiIPAddr);

			break;

		case NUM_DOWN_IPCP :
			pstSIGLog->uiCallTime		= pPSessData->uiReqTime;
			pstSIGLog->uiCallMTime		= pPSessData->uiReqMTime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szIMSI, MAX_MIN_SIZE );
			memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
			pstSIGLog->uiProtoType		= ucProtoType;
			pstSIGLog->uiMsgType		= pPSessData->DownIPCP.ucReqMsgCode;
/*
			pstSIGLog->uiUpGREPkts	= pPSessData->uiUpGREFrames;
			pstSIGLog->uiDnGREPkts	= pPSessData->uiDownGREFrames;
			pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
			pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;
*/
			pstSIGLog->uiSessStartTime	= (pPSessData->DownIPCP.StartTime);
			pstSIGLog->uiSessStartMTime = (pPSessData->DownIPCP.StartMTime);
			pstSIGLog->uiSessEndTime	= (pPSessData->DownIPCP.EndTime);
			pstSIGLog->uiSessEndMTime	= (pPSessData->DownIPCP.EndMTime);
			uiDuration = GetGapTime( pPSessData->DownIPCP.StartTime, pPSessData->DownIPCP.StartMTime,
					pPSessData->DownIPCP.EndTime, pPSessData->DownIPCP.EndMTime );
			pstSIGLog->uiSessDuration	= (uiDuration);

//			pstSIGLog->uiSrcIP			= (pPSessData->uiHomeAgent);
			pstSIGLog->uiSrcIP 			= pPSessData->stCallSessKey.uiDestIP;
			pstSIGLog->uiDestIP		 = pPSessData->stCallSessKey.uiSrcIP;

			pstSIGLog->uiRespCode		= pPSessData->DownIPCP.ucRepMsgCode;
			pstSIGLog->ucPPPReqCnt		= (pPSessData->DownIPCP.ucReqCount);
			pstSIGLog->ucPPPNakCnt		= (pPSessData->DownIPCP.ucNakCount);
			pstSIGLog->ucPPPRejCnt		= (pPSessData->DownIPCP.ucRejCount);
			pstSIGLog->uiClientIP		= (pPSessData->uiIPAddr);

			break;

		case NUM_CHAP :
		case NUM_PAP :
			pstSIGLog->uiCallTime		= pPSessData->uiReqTime;
			pstSIGLog->uiCallMTime		= pPSessData->uiReqMTime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szIMSI, MAX_MIN_SIZE );
			memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
			pstSIGLog->uiProtoType		= ucProtoType;
			pstSIGLog->uiMsgType		= pPSessData->CHAPAP.ucReqMsgCode;
/*
			pstSIGLog->uiUpGREPkts	= pPSessData->uiUpGREFrames;
			pstSIGLog->uiDnGREPkts	= pPSessData->uiDownGREFrames;
			pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
			pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;
*/
			pstSIGLog->uiSessStartTime	= (pPSessData->CHAPAP.StartTime);
			pstSIGLog->uiSessStartMTime = (pPSessData->CHAPAP.StartMTime);
			pstSIGLog->uiSessEndTime	= (pPSessData->CHAPAP.EndTime);
			pstSIGLog->uiSessEndMTime	= (pPSessData->CHAPAP.EndMTime);
			pstSIGLog->uiPPPResponseTime = (pPSessData->CHAPResTime);
			pstSIGLog->uiPPPResponseMTime = (pPSessData->CHAPResMTime);

			uiDuration = GetGapTime( pPSessData->CHAPAP.StartTime, pPSessData->CHAPAP.StartMTime,
					pPSessData->CHAPAP.EndTime, pPSessData->CHAPAP.EndMTime );
			pstSIGLog->uiSessDuration	= (uiDuration);

//			pstSIGLog->uiSrcIP			= (pPSessData->uiHomeAgent);
			pstSIGLog->uiSrcIP		 = pPSessData->stCallSessKey.uiDestIP;
			pstSIGLog->uiDestIP		 = pPSessData->stCallSessKey.uiSrcIP;

			pstSIGLog->uiRespCode		= pPSessData->CHAPAP.ucRepMsgCode;
			pstSIGLog->ucPPPReqCnt		= (pPSessData->CHAPAP.ucReqCount);
			pstSIGLog->ucPPPNakCnt		= (pPSessData->CHAPAP.ucNakCount);
			pstSIGLog->ucPPPRejCnt		= (pPSessData->CHAPAP.ucRejCount);

			sprintf((char*)pstSIGLog->szAuthUserName, "%s", pPSessData->szAuthUserName);

			pstSIGLog->uiPPPResponseTime	= (pPSessData->CHAPResTime);
			pstSIGLog->uiPPPResponseMTime	= (pPSessData->CHAPResMTime);

			break;

		default :
			log_print( LOGN_WARN, "Unknown ProtoType[%u]", ucProtoType );
			nifo_node_delete(pstMEMSINFO, pstNode);
			return -1;
	}

	LOG_SIGNAL_Prt( "Report_SIGLog", pstSIGLog );

	log_print( LOGN_DEBUG, "LOG_SIG[%3u] IMSI:%s CT:%10u.%06u CIP:%15s PCF:%u MSG:%3u SS:%10u.%06u SE:%10u.%06u AIR:%u PPP:0X%04X",
			ucProtoType, pstSIGLog->szIMSI, pstSIGLog->uiCallTime, pstSIGLog->uiCallMTime, util_cvtipaddr(szIPAddr, pstSIGLog->uiClientIP),
			pstSIGLog->uiPCFIP, pstSIGLog->uiMsgType, pstSIGLog->uiSessStartTime, pstSIGLog->uiSessStartMTime,
			pstSIGLog->uiSessEndTime, pstSIGLog->uiSessEndMTime, pstSIGLog->ucAirLink, pPSessData->usPPPFlag );

	dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_L2TP, SEQ_PROC_A_CALL, nifo_offset( pstMEMSINFO, pstNode ));
	if( dRet < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
		nifo_node_delete(pstMEMSINFO, pstNode);
		return -4;
	}

	return 0;
}

/*
 * dProcPPP 
 *
 */
int dProcPPP( st_CALLSESS_DATA *pPSessData, UCHAR *pBuf, USHORT siSize, INFO_ETH *pstEth, Capture_Header_Msg *pstCAP )
{
	int 				ret;
	INFO_PPP 			g_PPP;
	struct slcompress 	*m_comp;

	pPSessData->uiLastUpdateTime = pstCAP->curtime;
	pPSessData->uiLastUpdateMTime = pstCAP->ucurtime;

	memset( &g_PPP, 0x00, DEF_INFOPPP_SIZE );

	ret = Analyze_L2TP_PPP( pBuf, siSize, &g_PPP, m_comp );
	if( ret < 0 ) {
		log_print( LOGN_CRI, "	>>>[ERROR:%d] Analyze_L2TP_PPP() IMSI[%s] KEY[%d, %d]UpDown[%d] Seq[%u]",
				ret, pPSessData->szIMSI, pPSessData->stCallSessKey.usReqSessID, pPSessData->stCallSessKey.usResSessID, 
				pstCAP->bRtxType, pstEth->stGRE.dwSeqNum );

		dump_DebugString( "ERR PPP", (char*)pBuf, siSize );

		if( pstCAP->bRtxType == DEF_FROM_CLIENT )
			pPSessData->uiUpAnaErrFrames++;
		else
			pPSessData->uiDownAnaErrFrames++;

		return -1;
	}
	else {
		log_print( LOGN_INFO, "	>>> IMSI[%s] Key[%d, %d] PPP Size[%d] Protocol[0x%04x] Code[0x%02x]",
				pPSessData->szIMSI, pPSessData->stCallSessKey.usReqSessID, pPSessData->stCallSessKey.usResSessID, 
				siSize, g_PPP.stPPP.wProtocol, g_PPP.stPPP.ucCode );
	}

	if( g_PPP.stPPP.bFCSError ) {
		log_print( LOGN_DEBUG, "	>>> FCS ERROR, IMSI[%s] Key[%d, %d] UpDown[%d] Seq[%u]",
				pPSessData->szIMSI, pPSessData->stCallSessKey.usReqSessID, pPSessData->stCallSessKey.usResSessID, 
				pstCAP->bRtxType, pstEth->stGRE.dwSeqNum );

		if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
			pPSessData->uiUpFCSErrFrames++;
			pPSessData->uiUpFCSErrBytes += siSize;
		}
		else {
			pPSessData->uiDownFCSErrFrames++;
			pPSessData->uiDownFCSErrBytes += siSize;
		}

		return -2;
	}

	if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
		pPSessData->uiUpPPPFrames += 1;
		pPSessData->uiUpPPPBytes += siSize;
	}
	else {
		pPSessData->uiDownPPPFrames += 1;
		pPSessData->uiDownPPPBytes += siSize;
	}

/*
	DEF_LCP_CONF_REQ UP : pPSessData->usPPPFlag |= 0x0001 
	DEF_LCP_CONF_REQ DN : pPSessData->usPPPFlag |= 0x0004
	DEF_LCP_CONF_ACK DN : pPSessData->usPPPFlag |= 0x0008	
	DEF_LCP_CONF_ACK UP : pPSessData->usPPPFlag |= 0x0002
	
	DEF_LCP_TERM_REQ UP : pPSessData->usPPPFlag |= 0x0010
	DEF_LCP_TERM_REQ DN : pPSessData->usPPPFlag |= 0x0040 
	DEF_LCP_TERM_ACK DN : pPSessData->usPPPFlag |= 0x0080
	DEF_LCP_TERM_ACK UP : pPSessData->usPPPFlag |= 0x0020

	CHAP : pPSessData->usPPPFlag |= 0x0100
	DEF_CHAP_CHAL : pPSessData->usPPPFlag |= 0x0200
	DEF_CHAP_RESP : pPSessData->usPPPFlag |= 0x0400
	DEF_CHAP_SUCC : pPSessData->usPPPFlag |= 0x0800

	DEF_IPCP_CONF_REQ UP : pPSessData->usPPPFlag |= 0x1000
	DEF_IPCP_CONF_REQ DN : pPSessData->usPPPFlag |= 0x4000
	DEF_IPCP_CONF_ACK DN : pPSessData->usPPPFlag |= 0x8000
	DEF_IPCP_CONF_ACK UP :	pPSessData->usPPPFlag |= 0x2000
*/

	switch( g_PPP.stPPP.wProtocol )
	{
		case PPP_LCP:	/* LCP */

			if( g_PPP.stPPP.ucCode == DEF_LCP_CONF_REQ ) {			/* CONFIG-REQ */

				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					pPSessData->ucUpLCPReqCount++;

					if( pPSessData->UpLCP.StartTime == 0 ) {
						pPSessData->UpLCPStartTime		= pstCAP->curtime;
						pPSessData->UpLCPStartMTime 	= pstCAP->ucurtime;

						pPSessData->UpLCP.StartTime	 = pstCAP->curtime;
						pPSessData->UpLCP.StartMTime	= pstCAP->ucurtime;
					}

					pPSessData->usPPPFlag |= 0x0001;

					pPSessData->UpLCP.ucFlag		|= 0x01;
					pPSessData->UpLCP.uiID			= g_PPP.stPPP.ucID;
					pPSessData->UpLCP.ucReqMsgCode	= g_PPP.stPPP.ucCode;
					pPSessData->UpLCP.ucReqCount++;

					/* TO HANDLE WHEN PACKET IS INVERTED */
					/* 아래의 경우는 발생하지 않음. ID를 비교하기 때문에 PACKET이 역전되는 경우는 처리 못함 */
					if( pPSessData->UpLCP.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_UP_LCP, pPSessData->UpLCP.ucReqMsgCode, pPSessData);
						memset( &pPSessData->UpLCP, 0x00, DEF_TSESS_SIZE );
					}
				}
				else {	/* DOWN */
					pPSessData->ucDownLCPReqCount++;
					if( pPSessData->DownLCP.StartTime == 0 ) {
						pPSessData->DownLCPStartTime	= pstCAP->curtime;
						pPSessData->DownLCPStartMTime	= pstCAP->ucurtime;

						pPSessData->DownLCP.StartTime 	= pstCAP->curtime;
						pPSessData->DownLCP.StartMTime 	= pstCAP->ucurtime;
					}

					pPSessData->usPPPFlag |= 0x0004;

					pPSessData->DownLCP.ucFlag 			|= 0x01;
					pPSessData->DownLCP.uiID 			= g_PPP.stPPP.ucID;
					pPSessData->DownLCP.ucReqMsgCode 	= g_PPP.stPPP.ucCode;
					pPSessData->DownLCP.ucReqCount++;

					/* TO HANDLE WHEN PACKET IS INVERED */
					/* 아래의 경우는 발생하지 않음. ID를 비교하기 때문에 PACKET이 역전되는 경우는 처리 못함 */
					if( pPSessData->DownLCP.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_DOWN_LCP, pPSessData->DownLCP.ucReqMsgCode, pPSessData );
						memset( &pPSessData->DownLCP, 0x00, DEF_TSESS_SIZE );
					}
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_LCP_CONF_ACK ) { /* CONFIG-ACK */
				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->DownLCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_DEBUG, "NOT MATCHED F_CLI ID I:%s REQ:%u RES:%u", pPSessData->szIMSI, pPSessData->DownLCP.uiID, g_PPP.stPPP.ucID );
						break;
					}

					pPSessData->DownLCP.EndTime 		= pstCAP->curtime;
					pPSessData->DownLCP.EndMTime 		= pstCAP->ucurtime;

					pPSessData->DownLCP.ucFlag 			|= 0x02;
					pPSessData->DownLCP.uiID 			= g_PPP.stPPP.ucID;
					pPSessData->DownLCP.ucRepMsgCode 	= g_PPP.stPPP.ucCode;

					log_print(LOGN_INFO, "DN_LCP DownLCP.ucFlag:0x%02X UpLCP.ucFlag:0x%02X", pPSessData->DownLCP.ucFlag, pPSessData->UpLCP.ucFlag );

					pPSessData->usPPPFlag |= 0x0008;

					if( pPSessData->DownLCP.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_DOWN_LCP, pPSessData->DownLCP.ucReqMsgCode, pPSessData );
						memset( &pPSessData->DownLCP, 0x00, DEF_TSESS_SIZE );
					}
				}
				else { /* DOWN */
					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->UpLCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_DEBUG, "NOT MATCHED F_SVR ID I:%s REQ:%u RES:%u", pPSessData->szIMSI, pPSessData->UpLCP.uiID, g_PPP.stPPP.ucID );
						break;
					}

					pPSessData->UpLCP.EndTime 		= pstCAP->curtime;
					pPSessData->UpLCP.EndMTime 		= pstCAP->ucurtime;

					pPSessData->UpLCP.ucFlag 		|= 0x02;
					pPSessData->UpLCP.uiID 			= g_PPP.stPPP.ucID;
					pPSessData->UpLCP.ucRepMsgCode 	= g_PPP.stPPP.ucCode;

					log_print(LOGN_INFO, "UP_LCP DownLCP.ucFlag:0x%02X UpLCP.ucFlag:0x%02X", pPSessData->DownLCP.ucFlag, pPSessData->UpLCP.ucFlag );

					pPSessData->usPPPFlag |= 0x0002;

					if( pPSessData->UpLCP.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_UP_LCP, pPSessData->UpLCP.ucReqMsgCode, pPSessData );
						memset( &pPSessData->UpLCP, 0x00, DEF_TSESS_SIZE );
					}
				}

				if( pPSessData->PPPSetupTime == 0 && (pPSessData->usPPPFlag & 0x000f) == 0x000f) {
					if( pPSessData->UpLCPStartTime == pPSessData->DownLCPStartTime ) {
						if( pPSessData->UpLCPStartMTime < pPSessData->DownLCPStartMTime )
							pPSessData->uiLCPDuration = GetGapTime( pPSessData->UpLCPStartTime, pPSessData->UpLCPStartMTime,
									pstCAP->curtime, pstCAP->ucurtime);
						else
							pPSessData->uiLCPDuration = GetGapTime( pPSessData->DownLCPStartTime, pPSessData->DownLCPStartMTime,
									pstCAP->curtime, pstCAP->ucurtime);
					}
					else if( pPSessData->UpLCPStartTime < pPSessData->DownLCPStartTime )
						pPSessData->uiLCPDuration = GetGapTime( pPSessData->UpLCPStartTime, pPSessData->UpLCPStartMTime,
								pstCAP->curtime, pstCAP->ucurtime);
					else
						pPSessData->uiLCPDuration = GetGapTime( pPSessData->DownLCPStartTime, pPSessData->DownLCPStartMTime,
								pstCAP->curtime, pstCAP->ucurtime);
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_LCP_TERM_REQ ) { /* TERM-REQ */
				if( pPSessData->LCPTerm.StartTime == 0 ) {
					pPSessData->PPPTermTime 		= pstCAP->curtime;
					pPSessData->PPPTermMTime 		= pstCAP->ucurtime;

					pPSessData->LCPTerm.StartTime 	= pstCAP->curtime;
					pPSessData->LCPTerm.StartMTime 	=	pstCAP->ucurtime;
				}

				pPSessData->LCPTerm.ucFlag 			|= 0x01;
				pPSessData->LCPTerm.ucReqMsgCode 	= g_PPP.stPPP.ucCode;
				pPSessData->LCPTerm.uiID 			= g_PPP.stPPP.ucID;
				pPSessData->LCPTerm.ucReqCount++;

				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					pPSessData->usPPPFlag |= 0x0010;

					if( pPSessData->LCPTerm.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_UP_LCP, pPSessData->LCPTerm.ucReqMsgCode, pPSessData );
						memset( &pPSessData->LCPTerm, 0x00, DEF_TSESS_SIZE );
					}
				}
				else { /* DOWN */
					pPSessData->usPPPFlag |= 0x0040;

					if( pPSessData->LCPTerm.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_DOWN_LCP, pPSessData->LCPTerm.ucReqMsgCode, pPSessData );
						memset( &pPSessData->LCPTerm, 0x00, DEF_TSESS_SIZE );
					}
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_LCP_TERM_ACK ) { /* TERM-ACK */

				/* CHECK ID 2009.09.06 BY LDH */
				if( pPSessData->LCPTerm.uiID != g_PPP.stPPP.ucID ) {
					log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->LCPTerm.uiID, g_PPP.stPPP.ucID );
					break;
				}

				pPSessData->LCPTerm.EndTime 		= pstCAP->curtime;
				pPSessData->LCPTerm.EndMTime 		= pstCAP->ucurtime;

				pPSessData->LCPTerm.ucFlag 			|= 0x02;
				pPSessData->LCPTerm.ucRepMsgCode 	= g_PPP.stPPP.ucCode;
				pPSessData->LCPTerm.uiID 			= g_PPP.stPPP.ucID;

				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					pPSessData->usPPPFlag |= 0x0080;

					if( pPSessData->LCPTerm.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_DOWN_LCP, pPSessData->LCPTerm.ucReqMsgCode, pPSessData );
						memset( &pPSessData->LCPTerm, 0x00, DEF_TSESS_SIZE );
					}
				}
				else {
					pPSessData->usPPPFlag |= 0x0020;

					if( pPSessData->LCPTerm.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_UP_LCP, pPSessData->LCPTerm.ucReqMsgCode, pPSessData );
						memset( &pPSessData->LCPTerm, 0x00, DEF_TSESS_SIZE );
					}
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_LCP_CONF_NAK ) { /* NAK */
				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					pPSessData->DownLCP.ucNakCount++;

					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->DownLCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->DownLCP.uiID, g_PPP.stPPP.ucID );
						break;
					}
					pPSessData->DownLCP.ucRepMsgCode = g_PPP.stPPP.ucCode;
				}
				else {
					pPSessData->UpLCP.ucNakCount++;

					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->UpLCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->UpLCP.uiID, g_PPP.stPPP.ucID );
						break;
					}
					pPSessData->UpLCP.ucRepMsgCode = g_PPP.stPPP.ucCode;
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_LCP_CONF_REJ ) { /* REJECT */
				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					pPSessData->DownLCP.ucRejCount++;

					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->DownLCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->DownLCP.uiID, g_PPP.stPPP.ucID );
						break;
					}
					pPSessData->DownLCP.ucRepMsgCode = g_PPP.stPPP.ucCode;
				}
				else {
					pPSessData->UpLCP.ucRejCount++;

					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->UpLCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->UpLCP.uiID, g_PPP.stPPP.ucID );
						break;
					}
					pPSessData->UpLCP.ucRepMsgCode = g_PPP.stPPP.ucCode;
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_LCP_ECHO_REQ ) {
				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {

					pPSessData->ucUpLCPReqCount++;

					if( pPSessData->UpLCPEcho.StartTime == 0 ) {
						pPSessData->UpLCPEcho.StartTime	 = pstCAP->curtime;
						pPSessData->UpLCPEcho.StartMTime	= pstCAP->ucurtime;
					}

					pPSessData->UpLCPEcho.ucFlag		|= 0x01;
					pPSessData->UpLCPEcho.uiID			= g_PPP.stPPP.ucID;
					pPSessData->UpLCPEcho.ucReqMsgCode	= g_PPP.stPPP.ucCode;
					pPSessData->UpLCPEcho.ucReqCount++;

					/* TO HANDLE WHEN PACKET IS INVERTED */
					/* 아래의 경우는 발생하지 않음. ID를 비교하기 때문에 PACKET이 역전되는 경우는 처리 못함 */
					if( pPSessData->UpLCPEcho.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_UP_LCP, pPSessData->UpLCPEcho.ucReqMsgCode, pPSessData);
						memset( &pPSessData->UpLCPEcho, 0x00, DEF_TSESS_SIZE );
					}
				}
				else {
					pPSessData->ucDownLCPReqCount++;
					if( pPSessData->DownLCPEcho.StartTime == 0 ) {
						pPSessData->DownLCPEcho.StartTime	= pstCAP->curtime;
						pPSessData->DownLCPEcho.StartMTime	= pstCAP->ucurtime;
					}

					pPSessData->DownLCPEcho.ucFlag			|= 0x01;
					pPSessData->DownLCPEcho.uiID			= g_PPP.stPPP.ucID;
					pPSessData->DownLCPEcho.ucReqMsgCode	= g_PPP.stPPP.ucCode;
					pPSessData->DownLCPEcho.ucReqCount++;

					/* TO HANDLE WHEN PACKET IS INVERED */
					/* 아래의 경우는 발생하지 않음. ID를 비교하기 때문에 PACKET이 역전되는 경우는 처리 못함 */
					if( pPSessData->DownLCPEcho.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_DOWN_LCP, pPSessData->DownLCPEcho.ucReqMsgCode, pPSessData );
						memset( &pPSessData->DownLCPEcho, 0x00, DEF_TSESS_SIZE );
					}
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_LCP_ECHO_REP ) {
				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->DownLCPEcho.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_DEBUG, "NOT MATCHED F_CLI ID I:%s REQ:%u RES:%u", pPSessData->szIMSI, pPSessData->DownLCPEcho.uiID, g_PPP.stPPP.ucID );
						break;
					}

					pPSessData->DownLCPEcho.EndTime		 = pstCAP->curtime;
					pPSessData->DownLCPEcho.EndMTime		= pstCAP->ucurtime;

					pPSessData->DownLCPEcho.ucFlag			|= 0x02;
					pPSessData->DownLCPEcho.uiID			= g_PPP.stPPP.ucID;
					pPSessData->DownLCPEcho.ucRepMsgCode	= g_PPP.stPPP.ucCode;

					if( pPSessData->DownLCPEcho.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_DOWN_LCP, pPSessData->DownLCPEcho.ucReqMsgCode, pPSessData );
						memset( &pPSessData->DownLCPEcho, 0x00, DEF_TSESS_SIZE );
					}
				}
				else { /* DOWN */
					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->UpLCPEcho.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_DEBUG, "NOT MATCHED F_SVR ID I:%s REQ:%u RES:%u", pPSessData->szIMSI, pPSessData->UpLCPEcho.uiID, g_PPP.stPPP.ucID );
						break;
					}

					pPSessData->UpLCPEcho.EndTime		= pstCAP->curtime;
					pPSessData->UpLCPEcho.EndMTime		= pstCAP->ucurtime;

					pPSessData->UpLCPEcho.ucFlag		|= 0x02;
					pPSessData->UpLCPEcho.uiID			= g_PPP.stPPP.ucID;
					pPSessData->UpLCPEcho.ucRepMsgCode	= g_PPP.stPPP.ucCode;

					if( pPSessData->UpLCPEcho.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_UP_LCP, pPSessData->UpLCPEcho.ucReqMsgCode, pPSessData );
						memset( &pPSessData->UpLCPEcho, 0x00, DEF_TSESS_SIZE );
					}
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_LCP_PROT_REJ || g_PPP.stPPP.ucCode == DEF_LCP_DISC_REQ )
				/* DISCARD-REQUEST */ ;
			else {
				log_print( LOGN_INFO, "Unknown LCP Code[%d]", g_PPP.stPPP.ucCode );
			}

			break;

		case PPP_CHAP:	/* CHAP */

			pPSessData->usPPPFlag |= 0x0100;
			if( g_PPP.stPPP.ucCode == DEF_CHAP_CHAL ) { 		/* CHALLENGE */
				pPSessData->usPPPFlag |= 0x0200;

				if( pPSessData->CHAPAP.StartTime == 0 ) {
					pPSessData->CHAPAP.StartTime	= pstCAP->curtime;
					pPSessData->CHAPAP.StartMTime	= pstCAP->ucurtime;

					pPSessData->AuthReqTime	 = pstCAP->curtime;
					pPSessData->AuthReqMTime	= pstCAP->ucurtime;
				}

				pPSessData->CHAPAP.ucFlag		|= 0x01;
				pPSessData->CHAPAP.uiID		 = g_PPP.stPPP.ucID;
				pPSessData->CHAPAP.ucReqMsgCode = g_PPP.stPPP.ucCode;
				pPSessData->CHAPAP.ucReqCount++;

				if( pPSessData->CHAPAP.ucFlag == 0x03 ) {
					Report_SIGLog( NUM_CHAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
					memset( &pPSessData->CHAPAP, 0x00, DEF_TSESS_SIZE );
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_CHAP_RESP ) {	/* RESPONSE */
				/* CHECK ID 2009.09.06 BY LDH */
				if( pPSessData->CHAPAP.uiID != g_PPP.stPPP.ucID ) {
					log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->CHAPAP.uiID, g_PPP.stPPP.ucID );
					break;
				}

				pPSessData->usPPPFlag |= 0x0400;

				pPSessData->CHAPAP.ucRepMsgCode = g_PPP.stPPP.ucCode;
				pPSessData->CHAPAP.uiID		 = g_PPP.stPPP.ucID;

				pPSessData->ucAuthResultCode	= g_PPP.stPPP.ucCode;
				sprintf( (char*)pPSessData->szAuthUserName, "%s", g_PPP.stPPP.szUserName );

				pPSessData->CHAPResTime	 = pstCAP->curtime;
				pPSessData->CHAPResMTime	= pstCAP->ucurtime;
			}
			else if( g_PPP.stPPP.ucCode == DEF_CHAP_SUCC ) {	/* SUCCESS */
				/* CHECK ID 2009.09.06 BY LDH */
				if( pPSessData->CHAPAP.uiID != g_PPP.stPPP.ucID ) {
					log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->CHAPAP.uiID, g_PPP.stPPP.ucID );
					break;
				}

				pPSessData->usPPPFlag |= 0x0800;

				pPSessData->CHAPAP.EndTime		= pstCAP->curtime;
				pPSessData->CHAPAP.EndMTime 	= pstCAP->ucurtime;

				pPSessData->CHAPAP.ucFlag		|= 0x02;
				pPSessData->CHAPAP.uiID		 = g_PPP.stPPP.ucID;
				pPSessData->CHAPAP.ucRepMsgCode = g_PPP.stPPP.ucCode;

				if( pPSessData->CHAPAP.ucFlag == 0x03 ) {
					Report_SIGLog( NUM_CHAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
					memset( &pPSessData->CHAPAP, 0x00, DEF_TSESS_SIZE );
				}

				pPSessData->ucAuthResultCode	=	g_PPP.stPPP.ucCode;

				pPSessData->AuthEndTime	 	= pstCAP->curtime;
				pPSessData->AuthEndMTime		= pstCAP->ucurtime;
			}
			else if( g_PPP.stPPP.ucCode == DEF_CHAP_FAIL ) {	/* FAILURE */
				/* CHECK ID 2009.09.06 BY LDH */
				if( pPSessData->CHAPAP.uiID != g_PPP.stPPP.ucID ) {
					log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->CHAPAP.uiID, g_PPP.stPPP.ucID );
					break;
				}

				pPSessData->CHAPAP.EndTime 		= pstCAP->curtime;
				pPSessData->CHAPAP.EndMTime 	= pstCAP->ucurtime;

				pPSessData->CHAPAP.ucFlag 		|= 0x02;
				pPSessData->CHAPAP.uiID	 		= g_PPP.stPPP.ucID;
				pPSessData->CHAPAP.ucRepMsgCode = g_PPP.stPPP.ucCode;

				if( pPSessData->CHAPAP.ucFlag == 0x03 ) {
					Report_SIGLog( NUM_CHAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
					memset( &pPSessData->CHAPAP, 0x00, DEF_TSESS_SIZE );
				}

				pPSessData->ucAuthResultCode =	g_PPP.stPPP.ucCode;

				pPSessData->AuthEndTime = pstCAP->curtime;
				pPSessData->AuthEndMTime = pstCAP->ucurtime;
			}
			break;

		case PPP_PAP:	/* PAP */
			if( g_PPP.stPPP.ucCode == DEF_PAP_AUTH_REQ ) {	/* PAP REQUEST */
				pPSessData->usPPPFlag |= 0x0200;

				if( pPSessData->CHAPAP.StartTime == 0 ) {
					pPSessData->CHAPAP.StartTime = pstCAP->curtime;
					pPSessData->CHAPAP.StartMTime = pstCAP->ucurtime;

					pPSessData->AuthReqTime = pstCAP->curtime;
					pPSessData->AuthReqMTime = pstCAP->ucurtime;
				}

				sprintf( (char*)pPSessData->szAuthUserName, "%s", g_PPP.stPPP.szUserName );

				pPSessData->CHAPAP.ucFlag 		|= 0x01;
				pPSessData->CHAPAP.uiID 		= g_PPP.stPPP.ucID;
				pPSessData->CHAPAP.ucReqMsgCode = g_PPP.stPPP.ucCode;

				// TO HANDLE WHEN PACKET IS INVERTED
				if( pPSessData->CHAPAP.ucFlag == 0x03 ) {
					Report_SIGLog( NUM_PAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
					memset( &pPSessData->CHAPAP, 0x00, DEF_TSESS_SIZE );
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_PAP_AUTH_ACK ) {	/* PAP SUCCESS ACK */
				pPSessData->usPPPFlag |= 0x0800;

				pPSessData->CHAPAP.EndTime 		= pstCAP->curtime;
				pPSessData->CHAPAP.EndMTime 	= pstCAP->ucurtime;

				pPSessData->CHAPAP.ucFlag 		|= 0x02;
				pPSessData->CHAPAP.uiID 		= g_PPP.stPPP.ucID;
				pPSessData->CHAPAP.ucRepMsgCode = g_PPP.stPPP.ucCode;

				// TO HANDLE WHEN PACKET IS INVERTED
				if( pPSessData->CHAPAP.ucFlag == 0x03 ) {
					Report_SIGLog( NUM_PAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
					memset( &pPSessData->CHAPAP, 0x00, DEF_TSESS_SIZE );
				}

				pPSessData->ucAuthResultCode 	= g_PPP.stPPP.ucCode;

				pPSessData->AuthEndTime 		= pstCAP->curtime;
				pPSessData->AuthEndMTime 		= pstCAP->ucurtime;
			}
			else if( g_PPP.stPPP.ucCode == DEF_PAP_AUTH_NAK ) {	/* PAP FAIL ACK */
				pPSessData->CHAPAP.EndTime 		= pstCAP->curtime;
				pPSessData->CHAPAP.EndMTime 	= pstCAP->ucurtime;

				pPSessData->CHAPAP.ucFlag 		|= 0x02;
				pPSessData->CHAPAP.uiID 		= g_PPP.stPPP.ucID;
				pPSessData->CHAPAP.ucRepMsgCode = g_PPP.stPPP.ucCode;

				// TO HANDLE WHEN PACKET IS INVERTED
				if( pPSessData->CHAPAP.ucFlag == 0x03 ) {
					Report_SIGLog( NUM_PAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
					memset( &pPSessData->CHAPAP, 0x00, DEF_TSESS_SIZE );
				}

				pPSessData->ucAuthResultCode 	= g_PPP.stPPP.ucCode;

				pPSessData->AuthEndTime 		= pstCAP->curtime;
				pPSessData->AuthEndMTime 		= pstCAP->ucurtime;
			}

			break;

		case PPP_IPCP:	/* IPCP */
			if( g_PPP.stPPP.ucCode == DEF_IPCP_CONF_REQ ) {	 /* CONFIG-REQ */

				if( pPSessData->IPCPStartTime == 0 ) {
					pPSessData->IPCPStartTime		= pstCAP->curtime;
					pPSessData->IPCPStartMTime		= pstCAP->ucurtime;
				}

				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					log_print( LOGN_INFO, "	>>> DEF_IPCP_CONF_REQ UpIPCP.FLAG[%02x]", pPSessData->UpIPCP.ucFlag);

					pPSessData->ucUpIPCPReqCount++;
					pPSessData->usPPPFlag |= 0x1000;

					if( pPSessData->UpIPCP.StartTime == 0 ) {
						pPSessData->UpIPCP.StartTime	= pstCAP->curtime;
						pPSessData->UpIPCP.StartMTime	= pstCAP->ucurtime;
					}

					pPSessData->UpIPCP.ucFlag		|= 0x01;
					pPSessData->UpIPCP.uiID		 = g_PPP.stPPP.ucID;
					pPSessData->UpIPCP.ucReqMsgCode = g_PPP.stPPP.ucCode;
					pPSessData->UpIPCP.ucReqCount++;

					if( pPSessData->UpIPCP.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_UP_IPCP, pPSessData->UpIPCP.ucReqMsgCode, pPSessData );
						memset( &pPSessData->UpIPCP, 0x00, DEF_TSESS_SIZE );
					}
				}
				else { /* DOWN */
					log_print( LOGN_INFO, "	>>> DEF_IPCP_CONF_REQ DownIPCP.FLAG[%02x]", pPSessData->DownIPCP.ucFlag);

					pPSessData->ucDownIPCPReqCount++;
					pPSessData->usPPPFlag |= 0x4000;

					if( pPSessData->DownIPCP.StartTime == 0 ) {
						pPSessData->DownIPCP.StartTime 	= pstCAP->curtime;
						pPSessData->DownIPCP.StartMTime = pstCAP->ucurtime;
					}

					pPSessData->DownIPCP.ucFlag		 |= 0x01;
					pPSessData->DownIPCP.uiID			= g_PPP.stPPP.ucID;
					pPSessData->DownIPCP.ucReqMsgCode	= g_PPP.stPPP.ucCode;
					pPSessData->DownIPCP.ucReqCount++;

					if( pPSessData->DownIPCP.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_DOWN_IPCP, pPSessData->DownIPCP.ucReqMsgCode, pPSessData );
						memset( &pPSessData->DownIPCP, 0x00, DEF_TSESS_SIZE );
					}
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_IPCP_CONF_ACK ) {	/* CONFIG-ACK */
				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					log_print( LOGN_INFO, "	>>> DEF_IPCP_CONF_ACK DownIPCP.FLAG[%02x]", pPSessData->DownIPCP.ucFlag);

					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->DownIPCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->DownIPCP.uiID, g_PPP.stPPP.ucID );
						break;
					}

					pPSessData->DownIPCP.EndTime 	= pstCAP->curtime;
					pPSessData->DownIPCP.EndMTime 	= pstCAP->ucurtime;

					pPSessData->DownIPCP.ucFlag		 |= 0x02;
					pPSessData->DownIPCP.uiID			= g_PPP.stPPP.ucID;
					pPSessData->DownIPCP.ucRepMsgCode	= g_PPP.stPPP.ucCode;

					pPSessData->usPPPFlag |= 0x8000;

					if( pPSessData->DownIPCP.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_DOWN_IPCP, pPSessData->DownIPCP.ucReqMsgCode, pPSessData );
						memset( &pPSessData->DownIPCP, 0x00, DEF_TSESS_SIZE );
					}
				}
				else {
					log_print( LOGN_INFO, "	>>> DEF_IPCP_CONF_ACK UpIPCP.FLAG[%02x]", pPSessData->UpIPCP.ucFlag);

					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->UpIPCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->UpIPCP.uiID, g_PPP.stPPP.ucID );
						break;
					}

					pPSessData->uiIPAddr = TOULONG(g_PPP.stPPP.ucIPAddr);

					pPSessData->UpIPCP.EndTime	= pstCAP->curtime;
					pPSessData->UpIPCP.EndMTime = pstCAP->ucurtime;

					pPSessData->UpIPCP.ucFlag		|= 0x02;
					pPSessData->UpIPCP.uiID		 = g_PPP.stPPP.ucID;
					pPSessData->UpIPCP.ucRepMsgCode = g_PPP.stPPP.ucCode;

					pPSessData->usPPPFlag |= 0x2000;

					if( pPSessData->UpIPCP.ucFlag == 0x03 ) {
						Report_SIGLog( NUM_UP_IPCP, pPSessData->UpIPCP.ucReqMsgCode, pPSessData );
						memset( &pPSessData->UpIPCP, 0x00, DEF_TSESS_SIZE );
					}
				}

				if( (pPSessData->usPPPFlag & 0xf000) == 0xf000 ) {
					if( pPSessData->PPPSetupTime == 0 ) {
						pPSessData->PPPSetupTime	= pstCAP->curtime;
						pPSessData->PPPSetupMTime	= pstCAP->ucurtime;

						pPSessData->uiIPCPDuration = GetGapTime( pPSessData->IPCPStartTime, pPSessData->IPCPStartMTime,
								pstCAP->curtime, pstCAP->ucurtime);
					}
					else {
						log_print( LOGN_DEBUG, "IPCP_ACK dup, CreateTime[%d] MIN[%s]", pPSessData->uiReqTime, pPSessData->szIMSI );
					}

					pPSessData->ucPPPSetupCount ++;
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_IPCP_CONF_NAK ) {	/* CONFIGURE-NAK */
				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					pPSessData->DownIPCP.ucNakCount++;

					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->DownIPCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->DownIPCP.uiID, g_PPP.stPPP.ucID );
						break;
					}
					pPSessData->DownIPCP.ucRepMsgCode	= g_PPP.stPPP.ucCode;
				}
				else {
					pPSessData->UpIPCP.ucNakCount++;

					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->UpIPCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->UpIPCP.uiID, g_PPP.stPPP.ucID );
						break;
					}
					pPSessData->UpIPCP.ucRepMsgCode = g_PPP.stPPP.ucCode;
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_IPCP_CONF_REJ ) {	/* CONFIGURE-REJ*/
				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					pPSessData->DownIPCP.ucRejCount++;

					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->DownIPCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->DownIPCP.uiID, g_PPP.stPPP.ucID );
						break;
					}
					pPSessData->DownIPCP.ucRepMsgCode	= g_PPP.stPPP.ucCode;
				}
				else {
					pPSessData->UpIPCP.ucRejCount++;

					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->UpIPCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->UpIPCP.uiID, g_PPP.stPPP.ucID );
						break;
					}
					pPSessData->UpIPCP.ucRepMsgCode = g_PPP.stPPP.ucCode;
				}
			}
			else {
				log_print( LOGN_DEBUG, "Unknown IPCP Code[%d]", g_PPP.stPPP.ucCode );
			}

			break;

		case PPP_IP:
		case PPP_VJCTCPIP:
		case PPP_VJUCTCPIP:
		case PPP_CD:
		case PPP_CCP1:
		case PPP_CCP2:
		case PPP_LQR:
			log_print( LOGN_INFO, "Not Interesting PPP[0x%04x]", g_PPP.stPPP.wProtocol );
			break;

		default:
			log_print( LOGN_DEBUG, "Unknown PPP[0x%04x] MIN[%s] CreateT[%u.%d]",
					g_PPP.stPPP.wProtocol, pPSessData->szIMSI, pPSessData->uiReqTime, pPSessData->uiReqMTime );
			break;
	}

	return 0;
}

/* 
 * dProcessDataMessages 
 *
 * 1. PPP 패킷 분석 
 * 2. CHAP Success 메세지 분석후 시그널 전송
 * 3. IPCP Transaction 시그널 전송
 * 4. LCP Termination 시그널 전송
 */
int dProcessDataMessages(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, st_SESSID_KEY *pstSessionIDKey, st_TUNNELID_KEY *pstTunnelIDKey, st_L2TP_INFO *pstL2TPInfo)
{
	int 				dRet;

	st_SESSID_DATA		stSessionIDData;
	st_SESSID_DATA		*pstSessionIDData = &stSessionIDData;

	st_CONTROL_DATA		*pstControlData = NULL;

	stHASHONODE			*pstSessionIDHashNode;
	stHASHONODE			*pstCallSessHashNode;

	st_CALLSESS_KEY 	*pstCallSessKey;
	st_CALLSESS_DATA 	stCallSessData;
	st_CALLSESS_DATA 	*pstCallSessData = &stCallSessData;

	int 				dHeaderLen;
	int 				siSize;


	/* CONTROL SESSION */
	if ( (pstControlData = pGetControlSessionData(pstTunnelIDKey)) == NULL ) {
		log_print (LOGN_CRI, LH" pGetControlSessionData() is NULL", LT);
		return -1;
	} else {

		// UPDATE CONTROL SESSION TIMER 
#ifdef SIM 
		gTIMER_TRANS = 20;
#else				 
		gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
#endif
		pstControlData->ulTimerNID = timerN_update (pstCTIMERNINFO, pstControlData->ulTimerNID, time(NULL) + gTIMER_TRANS);

		/* CALL SESSION */
		if( (pstSessionIDHashNode = hasho_find (pstSHASHOINFO, (U8 *)pstSessionIDKey)) == NULL ) {

			log_print (LOGN_CRI, "SESSION ID NOT FOUND 	SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d REQ-SID : %d RES-SID : %d",
					HIPADDR(pstSessionIDKey->uiSrcIP), HIPADDR(pstSessionIDKey->uiDestIP),
					pstSessionIDData->stCALLSESSKey.usReqSessID, pstSessionIDData->stCALLSESSKey.usResSessID);

			return -2;
		} else {
			pstSessionIDData = (st_SESSID_DATA *) nifo_ptr (pstSHASHOINFO, pstSessionIDHashNode->offset_Data);
			pstSessionIDData->usLastMessageType = pstL2TPInfo->usMessageType;

			// UPDATE SESSION ID TIMER 
//			pstSessionIDData->ulTimerNID = timerN_update (pstSTIMERNINFO, pstSessionIDData->ulTimerNID, time(NULL) + gTIMER_TRANS);

			/* FIND CALL SESSION */
			pstCallSessKey = &pstSessionIDData->stCALLSESSKey;
			if( (pstCallSessHashNode = hasho_find (pstHASHOINFO, (U8 *)pstCallSessKey)) != NULL ) {
				pstCallSessData = (st_CALLSESS_DATA *) nifo_ptr (pstHASHOINFO, pstCallSessHashNode->offset_Data);

				if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
					pstCallSessData->uiUpPktCnt++;
					pstCallSessData->uiUpPktBytes += pINFOETH->stIP.wTotalLength;
				} else {
					pstCallSessData->uiDnPktCnt++;
					pstCallSessData->uiDnPktBytes += pINFOETH->stIP.wTotalLength;
				}

				// UPDATE CALL SESSION TIMER
				pstCallSessData->ulTimerNID = timerN_update (pstTIMERNINFO, pstCallSessData->ulTimerNID, time(NULL) + gTIMER_TRANS);

				dHeaderLen = 14 + pINFOETH->stIP.wIPHeaderLen + pINFOETH->stUDPTCP.wHeaderLen + pstL2TPInfo->usHeaderSize;
				siSize = pINFOETH->stUDPTCP.wDataLen - pstL2TPInfo->usHeaderSize;

#ifdef __DEBUG__
				log_print (LOGN_DEBUG, "	>>> IPHLEN: %d UDPHLEN: %d L2TPHLEN: %d UDPDLEN: %d SIZE: %d",
						pINFOETH->stIP.wIPHeaderLen, pINFOETH->stUDPTCP.wHeaderLen, pstL2TPInfo->usHeaderSize, pINFOETH->stUDPTCP.wDataLen, siSize);
#endif
				dRet = dProcPPP( pstCallSessData, pDATA+dHeaderLen, siSize, pINFOETH, pCAPHEAD);

			} else {
				log_print (LOGN_CRI, "CALL SESS NOT FOUND SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d REQTID: %d RESTID: %d", 
						HIPADDR(pstCallSessKey->uiSrcIP), HIPADDR(pstCallSessKey->uiDestIP), 
						pstCallSessKey->usReqSessID, pstCallSessKey->usResSessID);

				return -3;
			}

			if ((dRet = dCheck_TraceInfo(pstSessionIDData, pDATA, pCAPHEAD)) < 0) {
				log_print (LOGN_CRI, "[%s.%d] FAILED dCheck_TraceInfo dRet:[%d]", __FUNCTION__, __LINE__, dRet);
			}
		}
	}

	return 1;
}


/*
 * dProcL2TP_Trans
 *
 */
int dProcL2TP_Trans( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, st_L2TP_INFO *pstL2TPInfo)
{
	int					dRet;

	st_TUNNELID_KEY 	stTunnelIDKey;
	st_TUNNELID_KEY 	*pstTunnelIDKey = &stTunnelIDKey;
	st_SESSID_KEY 		stSessIDKey;
	st_SESSID_KEY 		*pstSessIDKey = &stSessIDKey;


	memset(&stTunnelIDKey, 0x00, DEF_TUNNELID_KEY_SIZE);
	memset(&stSessIDKey, 0x00, DEF_SESSID_KEY_SIZE);

	/*	SET TRANS KEY */
	if (pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
		stTunnelIDKey.uiSrcIP 	= pINFOETH->stIP.dwSrcIP;
		stTunnelIDKey.uiDestIP	= pINFOETH->stIP.dwDestIP;
		stSessIDKey.uiSrcIP 	= pINFOETH->stIP.dwSrcIP;
		stSessIDKey.uiDestIP	= pINFOETH->stIP.dwDestIP;
	} else {
		stTunnelIDKey.uiSrcIP 	= pINFOETH->stIP.dwDestIP;
		stTunnelIDKey.uiDestIP	= pINFOETH->stIP.dwSrcIP;
		stSessIDKey.uiSrcIP 	= pINFOETH->stIP.dwDestIP;
		stSessIDKey.uiDestIP	= pINFOETH->stIP.dwSrcIP;
	}

	if (pstL2TPInfo->usPacketType) {
		/* Control Message */
		if (pstL2TPInfo->usLength==12) {
		}
		switch (pstL2TPInfo->usMessageType) {
			case L2TP_MSG_SCCRQ:
				pstTunnelIDKey->usTunnelID = pstL2TPInfo->usAssignedTunnelID;
				if ( (dRet = dProcessTunnelMessages(pCAPHEAD, pINFOETH, &stTunnelIDKey, pstL2TPInfo)) < 0 ) {
					log_print (LOGN_CRI, "[%s][%s.%d]	dProcessTunnelMessages() dRet: %d", __FILE__, __FUNCTION__, __LINE__, dRet);
					return -1;
				}
				break;
			case L2TP_MSG_SCCRP:
			case L2TP_MSG_SCCCN:
			case L2TP_MSG_StopCCN:
			case L2TP_MSG_HELLO:
			case L2TP_MSG_ZLB:
				pstTunnelIDKey->usTunnelID = pstL2TPInfo->usTunnelID;
				if ( (dRet = dProcessTunnelMessages(pCAPHEAD, pINFOETH, &stTunnelIDKey, pstL2TPInfo)) < 0 ) {
					log_print (LOGN_CRI, "[%s][%s.%d]	dProcessTunnelMessages() dRet: %d", __FILE__, __FUNCTION__, __LINE__, dRet);
					return -1;
				}
				break;
			case L2TP_MSG_ICRQ:
				pstSessIDKey->usSessionID = pstL2TPInfo->usAssignedSessionID;
				pstTunnelIDKey->usTunnelID = pstL2TPInfo->usTunnelID;
				if ( (dRet = dProcessSessionMessages(pCAPHEAD, pINFOETH, pDATA, pstSessIDKey, pstTunnelIDKey, pstL2TPInfo)) < 0 ) {
					log_print (LOGN_CRI, "[%s][%s.%d]	dProcessSessionMessages() dRet: %d", __FILE__, __FUNCTION__, __LINE__, dRet);
					return -1;
				}
				break;
			case L2TP_MSG_ICRP:
			case L2TP_MSG_ICCN:
			case L2TP_MSG_CDN:
				pstSessIDKey->usSessionID = pstL2TPInfo->usSessionID;
				pstTunnelIDKey->usTunnelID = pstL2TPInfo->usTunnelID;
				if ( (dRet = dProcessSessionMessages(pCAPHEAD, pINFOETH, pDATA, pstSessIDKey, pstTunnelIDKey, pstL2TPInfo)) < 0 ) {
					log_print (LOGN_CRI, "[%s][%s.%d]	dProcessSessionMessages() dRet: %d ", __FILE__, __FUNCTION__, __LINE__, dRet);
					return -1;
				}
				break;
			default:
				log_print(LOGN_CRI, "UNDEFINED L2TP MESSAGE TYPE: %d", pstL2TPInfo->usMessageType);
				break;
		}
	}
	else {
		/* Data Message */
		pstSessIDKey->usSessionID = pstL2TPInfo->usSessionID;
		pstTunnelIDKey->usTunnelID = pstL2TPInfo->usTunnelID;

		dRet = dProcessDataMessages(pCAPHEAD, pINFOETH, pDATA, pstSessIDKey, pstTunnelIDKey, pstL2TPInfo);
		if(dRet < 0) {
			log_print (LOGN_CRI, "[%s][%s.%d]	dProcessDataMessages() NULL ", __FILE__, __FUNCTION__, __LINE__);
			return -1;
		}
	}

	return SUCC;
}

void printL2TPInfo(st_L2TP_INFO *pstL2TPInfo)
{
	log_print(LOGN_DEBUG, " TYPE: %d HDRSIZE: %d LEN: %d TID: %d SID: %d NS: %d NR: %d OFFSETSIZE: %d",
			pstL2TPInfo->usPacketType, pstL2TPInfo->usHeaderSize, pstL2TPInfo->usLength,
			pstL2TPInfo->usTunnelID, pstL2TPInfo->usSessionID,
			pstL2TPInfo->usNs, pstL2TPInfo->usNr, pstL2TPInfo->usOffsetSize);

	log_print(LOGN_DEBUG, " HOST: %s VENDOR: %s TID: %d SID: %d WINSIZE: %d CHAL: %s CHALRES: %s",
			pstL2TPInfo->szHostName, pstL2TPInfo->szVendorName, 
			pstL2TPInfo->usAssignedTunnelID, pstL2TPInfo->usAssignedSessionID, pstL2TPInfo->usReceiveWindowSize,
			pstL2TPInfo->szChallenge, pstL2TPInfo->szChallengeResp);
}

