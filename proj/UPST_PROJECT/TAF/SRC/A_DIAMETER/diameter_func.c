/**<  
 $Id: diameter_func.c,v 1.3 2011/09/07 05:05:23 dcham Exp $
 $Author: dcham $
 $Revision: 1.3 $
 $Date: 2011/09/07 05:05:23 $
 **/

/**
 * Include headers
 */
#include <ctype.h>
#include <sys/time.h>

// DQMS headers
#include "commdef.h"
#include "procid.h"
#include "common_stg.h"

// LIB headers
#include "common_stg.h"
#include "typedef.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"

// TAF headers
#include "capdef.h"
#include "filter.h"
#include "sip.h"
#include "Analyze_Ext_Abs.h"

// .
#include "diameter_comm.h"
#include "diameter_func.h"

/**
 * Declare variables
 */
st_Flt_Info		*flt_info;

extern stMEMSINFO       *pstMEMSINFO;
extern stCIFO			*gpCIFO;
extern stHASHOINFO      *pstHASHOINFO;
extern stTIMERNINFO     *pstTIMERNINFO;
extern st_TraceList		*pstTRACE;		/* TRACE */
extern S32				gTIMER_TRANS;
extern S32				gACALLCnt;

/**
 *	Declare extern func.
 */
extern int sip_min(char *sp, int slen, char *min, int *vendor);

/**
 *	Implement func.
 */

/** GetGapTime function.
 *
 *  @return         S64
 **/

S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime)
{
    S64     gapTime;

    gapTime = (((S64)endtime * 1000000 + (S64)endmtime) - ((S64)starttime * 1000000 + (S64)startmtime));

    if(gapTime < 0)
        gapTime = 0;

    return gapTime;
}

#if 0
char *PrintCommandCode(U32 type)
{   
	switch(type)
	{
		case 280:
			return "DEVICE-WATCH";
		case 300:		
			return "USER-AUTHORIZATION";
		case 301:		
			return "SERVER-ASSIGNMENT";
		case 302:		
			return "LOCATION-INFO";
		case 303:		
			return "MULTIMEDIA-AUTH";
		default:	
			return "UNKNOWN";
	}
}


char *PrintMsgString(U32 type, int dReqFlag)
{
	switch(type)
	{
		case 280:
			if (dReqFlag == REQUEST)
				return "Device-Watchdog-Request";
			else
				return "Device-Watchdog-Response";
		case 300:		
			if (dReqFlag == REQUEST)
				return "User-Authorization-Request";
			else
				return "User-Authorization-Response";
		case 301:		
			if (dReqFlag == REQUEST)
				return "Server-Assignment-Request";
			else
				return "Server-Assignment-Response";
		case 302:		
			if (dReqFlag == REQUEST)
				return "Location-Info-Request";
			else
				return "Location-Info-Response";
		case 303:		
			if (dReqFlag == REQUEST)
				return "Multimedia-Auth-Request";
			else
				return "Multimedia-Auth-Response";
		default:	
			return "UNKNOWN";
	}
}
#endif

int dump(char *s,int len)
{       
	char buf[BUFSIZ],lbuf[BUFSIZ],rbuf[BUFSIZ];
	unsigned char *p;
	int line,i;

	p =(unsigned char *) s;
	for(line = 1; len > 0; len -= WIDTH16,line++) {
		memset(lbuf,0,BUFSIZ);
		memset(rbuf,0,BUFSIZ);

		for(i = 0; i < WIDTH16 && len > i; i++,p++) {
			sprintf(buf,"%02x ",(unsigned char) *p);
			strcat(lbuf,buf);
			sprintf(buf,"%c",(!iscntrl(*p) && *p <= 0x7f) ? *p : '.');
			strcat(rbuf,buf);
		}
		printf("%04x: %-*s    %s\n",line - 1,WIDTH16 * 3,lbuf,rbuf);
	}           
	/*if(!(len%16)) log_print(LOGN_INFO, "\n"); */
	return line;
}

S32 dGetCALLProcID(U32 uiClientIP)
{
	return SEQ_PROC_A_CALL + ( uiClientIP % gACALLCnt );
}

int dSendTransLog(Capture_Header_Msg *pCAPHEAD, HKey_Trans *pstTransKey, HData_Trans *pstTransData)
{
	int					dRet;
	S32					dSeqProcID;
	struct timeval		now;

	UCHAR				*pNode;
	LOG_SIGNAL			*pstTransLog;

	pstTransLog = (LOG_SIGNAL *) nifo_ptr (pstMEMSINFO, pstTransData->dOffset);
	pNode = nifo_ptr (pstMEMSINFO, nifo_get_offset_node (pstMEMSINFO, (U8 *)pstTransLog));

	gettimeofday(&now, 0);

	/**<  COMMON **/
	if ( pCAPHEAD==NULL ) {
		pstTransLog->uiLastUserErrCode 	= pstTransData->uiLastUserErrCode;
		pstTransLog->uiSessEndTime 		= now.tv_sec;      
		pstTransLog->uiSessEndMTime 	= now.tv_usec;
	} else {
		pstTransLog->uiSessEndTime		= pCAPHEAD->curtime;
		pstTransLog->uiSessEndMTime		= pCAPHEAD->ucurtime;
		pstTransLog->uiSessDuration		= GetGapTime(pstTransLog->uiSessEndTime, pstTransLog->uiSessEndMTime, pstTransLog->uiSessStartTime, pstTransLog->uiSessStartMTime);
	}

//	LOG_SIGNAL_Prt("DIAMETER_LOG_SIGNAL", pstTransLog);
	log_print( LOGN_DEBUG, "DIA_MSG:%3u I:%15s CT:%10u.%06u SIP:%10u DIP:%10u RC:%4u LUEC:%u",
                         pstTransLog->uiMsgType, pstTransLog->szIMSI, pstTransLog->uiCallTime, pstTransLog->uiCallMTime,
                         pstTransLog->uiSrcIP, pstTransLog->uiDestIP, pstTransLog->uiResultCode, pstTransLog->uiLastUserErrCode );

	/**<  SEND LOG **/	
	dSeqProcID = dGetCALLProcID(pstTransLog->uiClientIP);
	dRet = gifo_write (pstMEMSINFO, gpCIFO, SEQ_PROC_A_DIAMETER, dSeqProcID, nifo_offset(pstMEMSINFO, pNode) );
	if (dRet < 0) {
		log_print (LOGN_CRI, "[%s][%s.%d] FAILED gifo_write dRet:[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		nifo_node_delete(pstMEMSINFO, pNode);
		return RET_FAIL;
	}

	/**<  DELETE TRANS INFO **/	
	if (pstTransData->ulTimerNID != (long long)0) {
		timerN_del (pstTIMERNINFO, pstTransData->ulTimerNID);
	}
	hasho_del (pstHASHOINFO, (U8 *)pstTransKey);

	log_print (LOGN_DEBUG, 	"	--> DEL HASH		HHID: 0x%02x SPORT: %d SID: %u TID: %lld", 
			pstTransKey->uiHHID, pstTransKey->usSrcPort, pstTransKey->uiSystemId, pstTransData->ulTimerNID);

	return RET_SUCC;
}

int cb_timeout_transaction(HKey_Trans *pstTransKey)
{
	int 			dRet;
	HData_Trans		*pstTransData;
	stHASHONODE		*pstTransHashNode;

	pstTransHashNode = hasho_find (pstHASHOINFO, (U8*)pstTransKey);
	pstTransData = (HData_Trans *) nifo_ptr (pstHASHOINFO, pstTransHashNode->offset_Data);
	pstTransData->uiLastUserErrCode = DIAMETER_USER_TIMEOUT;
	
	log_print (LOGN_DEBUG, 	"	--> TIME OUT		HHID: 0x%02x SPORT: %d SID: %u", 
			pstTransKey->uiHHID, pstTransKey->usSrcPort, pstTransKey->uiSystemId);

	if ( (dRet=dSendTransLog(NULL, pstTransKey, pstTransData)) < 0 )
	{
		log_print (LOGN_CRI, "dSendTransLog() MINUS");
		return RET_FAIL;
	}

	return RET_SUCC;
}

HData_Trans *pCreateTransaction(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, HKey_Trans *pstTransKey)
{
	HData_Trans		*pstTransData;
	HData_Trans		stTransData;

	UCHAR			*pstLogNode;
	LOG_SIGNAL		*pstTransLog;

	stHASHONODE		*pstTransHashNode;

	struct timeval  now;

	pstTransData = &stTransData;

	/**<  INIT HDATA_TRANS **/	
	pstTransData->ulTimerNID		= 0;
	pstTransData->dOffset			= 0;
	pstTransData->dReqFlag			= 0;
	pstTransData->uiLastUserErrCode	= 0;
	pstTransData->uiRetransReqCnt	= 0;

	gettimeofday(&now, 0);

	/**<  ADD LOG_SIGNAL NIFO NODE **/	
	if( (pstLogNode = nifo_node_alloc(pstMEMSINFO)) == NULL )
	{
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

	if( (pstTransLog = (LOG_SIGNAL *) nifo_tlv_alloc(pstMEMSINFO, pstLogNode, LOG_PISIGNAL_DEF_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_ON)) == NULL )
	{
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_delete(pstMEMSINFO, pstLogNode);
		return NULL;
	}

	pstTransData->dOffset = nifo_offset (pstMEMSINFO, pstTransLog);

	/**<  COMMON **/	
	pstTransLog->uiCallTime					= pCAPHEAD->curtime;
	pstTransLog->uiCallMTime				= pCAPHEAD->ucurtime;

	pstTransLog->uiAccStartTime				= pCAPHEAD->curtime;
	pstTransLog->uiAccStartMTime			= pCAPHEAD->ucurtime;

	pstTransLog->uiSessStartTime			= pCAPHEAD->curtime;
	pstTransLog->uiSessStartMTime			= pCAPHEAD->ucurtime;

	/**<  SAVE DATA  **/	
	pstTransData->CallTime					= pCAPHEAD->curtime;
	pstTransData->CallMTime					= pCAPHEAD->ucurtime;

	/**<  ADD HASHO, TIMERN NODE **/	
	if ( (pstTransHashNode = hasho_add(pstHASHOINFO, (U8*)pstTransKey, (U8*)pstTransData)) == NULL )
	{
		log_print (LOGN_CRI, "[%s][%s.%d] hasho_add NULL ", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	} else {
		gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_DIA_TIMEOUT];
		pstTransData = (HData_Trans*) nifo_ptr (pstHASHOINFO, pstTransHashNode->offset_Data);
		pstTransData->ulTimerNID = timerN_add (pstTIMERNINFO, (void *)&cb_timeout_transaction, (U8*)pstTransKey, DEF_TDATA_TRANS_TIMER_SIZE, time(NULL) + gTIMER_TRANS);

		log_print (LOGN_DEBUG, "	--> ADD HASH		HHID: 0x%02x SPORT: %d SID: %u TID: %lld OFFSET: %ld", 
								pstTransKey->uiHHID, pstTransKey->usSrcPort, pstTransKey->uiSystemId, pstTransData->ulTimerNID, pstTransData->dOffset);
	}

	return pstTransData;
}

int dCheck_TraceInfo( HData_Trans *pstSessData, UCHAR *pData, Capture_Header_Msg *pstCAP )
{
	int     i, dRet;        
	UCHAR   *pstNode;
	UCHAR   *pBuffer;

	st_TraceMsgHdr  *pstTrcMsg;

	/*  
	 * pstSessData->szIMSI		: 4500010~
	 * pstSessData->szTraceMIN	: 010~
	 */

	for( i=0; i< pstTRACE->count; i++ ) {
		if( pstTRACE->stTraceInfo[i].dType == TRC_TYPE_IMSI ) {

			if( atoll((char*)pstSessData->szIMSI) == pstTRACE->stTraceInfo[i].stTraceID.llIMSI ) {

				log_print( LOGN_INFO, "TRACE IMSI[%d] %s, %s", i, pstSessData->szIMSI, pstTRACE->stTraceInfo[i].stTraceID.szMIN );
				/* SEND TRACE PACKET */
				if( (pstNode = nifo_node_alloc( pstMEMSINFO )) == NULL ) {
					log_print( LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
					return -1;
				}

				if( (pstTrcMsg = (st_TraceMsgHdr *)nifo_tlv_alloc( pstMEMSINFO, pstNode, st_TraceMsgHdr_DEF_NUM, st_TraceMsgHdr_SIZE, DEF_MEMSET_OFF)) == NULL ) {
					log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, st_TraceMsgHdr_DEF_NUM );
					return -2;
				}

				if( (pBuffer = nifo_tlv_alloc( pstMEMSINFO, pstNode, ETH_DATA_NUM, pstCAP->datalen, DEF_MEMSET_OFF)) == NULL ) {
					log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, ETH_DATA_NUM);
					return -3;
				}

				pstTrcMsg->time         = pstCAP->curtime;
				pstTrcMsg->mtime        = pstCAP->ucurtime;
				pstTrcMsg->dType        = pstTRACE->stTraceInfo[i].dType;
				pstTrcMsg->usDataLen    = pstCAP->datalen;
				memcpy( &pstTrcMsg->stTraceID, &pstTRACE->stTraceInfo[i].stTraceID, st_TraceID_SIZE );
				memcpy( pBuffer, pData, pstCAP->datalen );

				if( (dRet = gifo_write (pstMEMSINFO, gpCIFO, SEQ_PROC_A_DIAMETER, SEQ_PROC_CI_LOG, nifo_offset(pstMEMSINFO, pstNode))) < 0 ) {
					log_print(LOGN_CRI, "[%s][%s.%d] FAILED IN gifo_write dRet[%d][%s]", 
						__FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
					return -4;
				}

				return 1;
			}
		}
		else if( pstTRACE->stTraceInfo[i].dType == TRC_TYPE_MDN ) {

			if( atoll((char*)pstSessData->szTraceMIN) == pstTRACE->stTraceInfo[i].stTraceID.llIMSI) {

				log_print( LOGN_INFO, "TRACE MIN[%d] %s, %s", i, pstSessData->szTraceMIN, pstTRACE->stTraceInfo[i].stTraceID.szMIN );
				/* SEND TRACE PACKET */
				if( (pstNode = nifo_node_alloc( pstMEMSINFO )) == NULL ) {
					log_print( LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
					return -1;
				}

				if( (pstTrcMsg = (st_TraceMsgHdr *)nifo_tlv_alloc( pstMEMSINFO, pstNode, st_TraceMsgHdr_DEF_NUM, st_TraceMsgHdr_SIZE, DEF_MEMSET_OFF)) == NULL ) {
					log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, st_TraceMsgHdr_DEF_NUM );
					return -2;
				}

				if( (pBuffer = nifo_tlv_alloc( pstMEMSINFO, pstNode, ETH_DATA_NUM, pstCAP->datalen, DEF_MEMSET_OFF)) == NULL ) {
					log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, ETH_DATA_NUM);
					return -3;
				}

				pstTrcMsg->time         = pstCAP->curtime;
				pstTrcMsg->mtime        = pstCAP->ucurtime;
				pstTrcMsg->dType        = pstTRACE->stTraceInfo[i].dType;
				pstTrcMsg->usDataLen    = pstCAP->datalen;
				memcpy( &pstTrcMsg->stTraceID, &pstTRACE->stTraceInfo[i].stTraceID, st_TraceID_SIZE );
				memcpy( pBuffer, pData, pstCAP->datalen );

				if( (dRet = gifo_write (pstMEMSINFO, gpCIFO, SEQ_PROC_A_DIAMETER, SEQ_PROC_CI_LOG, nifo_offset(pstMEMSINFO, pstNode))) < 0 ) {
					log_print(LOGN_CRI, "[%s][%s.%d] FAILED IN gifo_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
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

int dProcDIAMETER_Trans( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, UCHAR *pETHDATA, st_DiameterInfo *pstDIAMETERINFO)
{
	HKey_Trans		stTransKey;
	HData_Trans		*pstTransData = NULL;
//	LOG_DIAMETER	*pstTransLog;
	LOG_SIGNAL		*pstTransLog;

	stHASHONODE		*pstTransHashNode;

	int				dSendLogFlag = 0;
	int				dRet;
	int				dReqFlag;
	U32				uiEEID;
	U8				szTempMIN[32];
	int				dTempVendor;

	memset(&stTransKey, 0x00, DEF_HKEY_TRANS_SIZE );	

	/**<  SET TRANS KEY **/	
	if (pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
		stTransKey.usSrcPort 	= pINFOETH->stUDPTCP.wSrcPort;
		stTransKey.uiSystemId	= pINFOETH->stUDPTCP.seq;
		stTransKey.uiHHID		= pstDIAMETERINFO->stDiameterHdr.uiHopByHopID;
	} else {
		stTransKey.usSrcPort 	= pINFOETH->stUDPTCP.wDestPort;
		stTransKey.uiSystemId	= pINFOETH->stUDPTCP.ack;
		stTransKey.uiHHID		= pstDIAMETERINFO->stDiameterHdr.uiHopByHopID;
	}
	uiEEID = pstDIAMETERINFO->stDiameterHdr.uiEndToEndID;

	/**<  CHECK REQUEST FLAG **/	
	dReqFlag = pstDIAMETERINFO->stDiameterHdr.flags.bRequest;

	/**<  DOES NOT EXIST **/	
	if( (pstTransHashNode = hasho_find (pstHASHOINFO, (U8 *)&stTransKey))  == NULL ) {
		/**<  NEW TRANSACTION **/	
		if ( dReqFlag == REQUEST )
		{
			log_print (LOGN_DEBUG, 	"	--> NEW PACKET		HHID: 0x%02x SPORT: %d SID: %u", 
					stTransKey.uiHHID, stTransKey.usSrcPort, stTransKey.uiSystemId);

			if ( (pstTransData = pCreateTransaction(pCAPHEAD, pINFOETH, &stTransKey)) == NULL )
			{
				log_print (LOGN_CRI, "[%s][%s.%d]	pCreateTransaction() RETURN IS NULL ", __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
			pstTransLog = (LOG_SIGNAL *) nifo_ptr (pstMEMSINFO, pstTransData->dOffset);
	
			/*  COMMON SIGNAL INFO */
			pstTransLog->uiSessStartTime    = pCAPHEAD->curtime;
			pstTransLog->uiSessStartMTime   = pCAPHEAD->ucurtime;
			pstTransLog->uiSessEndTime      = pCAPHEAD->curtime;
			pstTransLog->uiSessEndMTime     = pCAPHEAD->ucurtime;
			pstTransLog->uiSessDuration     = 0;                        
	
			if (pINFOETH->usSysType == TYPE_CSCF) {
				pstTransLog->uiSrcIP            = pINFOETH->stIP.dwSrcIP;
				pstTransLog->uiDestIP           = pINFOETH->stIP.dwDestIP;
			} else {
				pstTransLog->uiSrcIP            = pINFOETH->stIP.dwDestIP;
				pstTransLog->uiDestIP           = pINFOETH->stIP.dwSrcIP;
			}

			/**<  DIAMETER HEADER INFO **/	
			pstTransLog->uiProtoType        = DEF_PROTOCOL_DIAMETER;      /* TODO: define protocal type */
			pstTransLog->uiClientIP         = pINFOETH->stIP.dwSrcIP;
			pstTransLog->uiMsgType			= pstDIAMETERINFO->stDiameterHdr.uiCmdCode;

			pstTransLog->uiHopByHopID		= stTransKey.uiHHID;
			pstTransLog->uiEndToEndID		= uiEEID;
			pstTransLog->uiApplicationId 	= pstDIAMETERINFO->stDiameterHdr.uiAppID;
			pstTransLog->uiCommandCode 		= pstDIAMETERINFO->stDiameterHdr.uiCmdCode;
#if TRACE
			memcpy(pstTransData->szIMSI, pstACCINFO->szMIN, MAX_MIN_LEN);
			pstTransData->szIMSI[MAX_MIN_LEN] = 0x00;
#endif

			/**<  DIAMETER AVP IE INFO **/	

			/**<  TRANSACTION DATA **/
			pstTransData->dReqFlag		= dReqFlag;

		} else {	/**<  RESPONSE **/	
			log_print (LOGN_DEBUG, "	--> DROP PACKET		HHID: 0x%02x SPORT: %d SID: %u", 
					stTransKey.uiHHID, stTransKey.usSrcPort, stTransKey.uiSystemId);
			return -2;
		}
	}
	/**<  EXIST **/	
	else {
		pstTransData = (HData_Trans *) nifo_ptr (pstHASHOINFO, pstTransHashNode->offset_Data);
		pstTransLog = (LOG_SIGNAL *) nifo_ptr (pstMEMSINFO, pstTransData->dOffset);
			
		/**<  RETRANSMISSION **/	
		if ( dReqFlag == REQUEST ) {
			log_print (LOGN_DEBUG, 	"	--> RETRANS PACKET	HHID: 0x%02x SPORT: %d SID: %u", 
					stTransKey.uiHHID, stTransKey.usSrcPort, stTransKey.uiSystemId);

			gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_DIA_TIMEOUT];
			pstTransData->ulTimerNID = timerN_update (pstTIMERNINFO, pstTransData->ulTimerNID, time(0) + gTIMER_TRANS);

			pstTransLog->uiLastUserErrCode 	= DIAMETER_USER_RETRANS;
			pstTransLog->uiLastEndToEndID 	= pstDIAMETERINFO->stDiameterHdr.uiEndToEndID;
			pstTransLog->uiSessDuration		= GetGapTime(pstTransLog->uiSessEndTime, pstTransLog->uiSessEndMTime, pstTransLog->uiSessStartTime, pstTransLog->uiSessStartMTime);

		} /**<  RESPONSE **/	
		else {
			log_print (LOGN_DEBUG, 	"	--> ANSWER PACKET	HHID: 0x%02x SPORT: %d SID: %u", 
					stTransKey.uiHHID, stTransKey.usSrcPort, stTransKey.uiSystemId);

			pstTransLog->uiLastEndToEndID = pstDIAMETERINFO->stDiameterHdr.uiEndToEndID;

			dSendLogFlag = 1;
		}
	}

	switch(pstTransLog->uiCommandCode)
	{
		case USER_AUTHORIZATION_TRANS:			/* 300 */	
		case SERVER_ASSIGNMENT_TRANS:			/* 301 */			
		case LOCATION_INFO_TRANS:				/* 302 */	
		case MULTIMEDIA_AUTH_TRANS:				/* 303 */	
		case REGISTRATION_TERMINATION_TRANS:	/* 304 */
		case PUSH_PROFILE_TRANS:				/* 305 */
			log_print (LOGN_DEBUG, "Cx INTERFACE MESSAGE TYPE: [%d]", pstTransLog->uiCommandCode);

			if ( dReqFlag == REQUEST ) {
				memcpy ( pstTransLog->szSessionID, pstDIAMETERINFO->szSessionID, MAX_SESSIONID_LEN);
				pstTransLog->szSessionID[MAX_SESSIONID_LEN] = 0x00;
				memcpy ( pstTransLog->szOrgHost, pstDIAMETERINFO->szOrgHost, MAX_HOST_REALM_LEN);
				pstTransLog->szOrgHost[MAX_HOST_REALM_LEN] = 0x00;
				memcpy ( pstTransLog->szOrgRealm, pstDIAMETERINFO->szOrgRealm, MAX_HOST_REALM_LEN);
				pstTransLog->szOrgRealm[MAX_HOST_REALM_LEN] = 0x00;
				memcpy ( pstTransLog->szDestHost, pstDIAMETERINFO->szDestHost, MAX_HOST_REALM_LEN);
				pstTransLog->szDestHost[MAX_HOST_REALM_LEN] = 0x00;
				memcpy ( pstTransLog->szDestRealm, pstDIAMETERINFO->szDestRealm, MAX_HOST_REALM_LEN);
				pstTransLog->szDestRealm[MAX_HOST_REALM_LEN] = 0x00;
				memcpy ( pstTransLog->szPublicID, pstDIAMETERINFO->szPublicID, MAX_SIPURI_LEN);
				pstTransLog->szPublicID[MAX_SIPURI_LEN] = 0x00;
			}
			else {
				if ( pstDIAMETERINFO->uiResultCode == 0 ) {
					pstTransLog->uiResultCode = pstDIAMETERINFO->uiExpResultCode;
				} else {
					pstTransLog->uiResultCode = pstDIAMETERINFO->uiResultCode;
				}
				if ( pstDIAMETERINFO->uiResultCode > 3000 ) {
					pstTransLog->uiLastUserErrCode = DIAMETER_USER_FAILED;
				}
			}
			break;
		case USER_DATA_TRANS:					/* 306 */
		case PROFILE_UPDATE_TRANS:				/* 307 */
		case SUBSCRIBE_NOTIFICATIONS_TRANS:		/* 308 */
		case PUSH_NOTIFICATION_TRANS:			/* 309 */
		case BOOSTRAPPING_INFO_TRANS:			/* 310 */
		case MESSAGE_PROCES_TRANS:				/* 311 */
			log_print (LOGN_DEBUG, "Sh INTERFACE MESSAGE TYPE: [%d]", pstTransLog->uiCommandCode);

			if ( dReqFlag == REQUEST ) {
				memcpy ( pstTransLog->szOrgRealm, pstDIAMETERINFO->szOrgRealm, MAX_HOST_REALM_LEN);
				pstTransLog->szOrgRealm[MAX_HOST_REALM_LEN] = 0x00;
				memcpy ( pstTransLog->szDestHost, pstDIAMETERINFO->szDestHost, MAX_HOST_REALM_LEN);
				pstTransLog->szOrgHost[MAX_HOST_REALM_LEN] = 0x00;
				memcpy ( pstTransLog->szPublicID, pstDIAMETERINFO->szPublicID, MAX_SIPURI_LEN);
				pstTransLog->szPublicID[MAX_SIPURI_LEN] = 0x00;

				pstTransLog->uiAuthSessionState = pstDIAMETERINFO->uiAuthSessionState;
			}
			else {
				if ( pstDIAMETERINFO->uiResultCode == 0 ) {
					pstTransLog->uiResultCode = pstDIAMETERINFO->uiExpResultCode;
				} else {
					pstTransLog->uiResultCode = pstDIAMETERINFO->uiResultCode;
				}
				if ( pstDIAMETERINFO->uiResultCode > 3000 ) {
					pstTransLog->uiLastUserErrCode = DIAMETER_USER_FAILED;
				}
			}
			break;
		case ACCOUNTING_REQUEST_TRANS:			/*  271 */	
		case DEVICE_WATCHDOG_TRANS:				/*  280 */	
			log_print (LOGN_DEBUG, "Rf INTERFACE MESSAGE TYPE: [%d]", pstTransLog->uiCommandCode);
			if ( dReqFlag == REQUEST )
			{
				memcpy ( pstTransLog->szOrgHost, pstDIAMETERINFO->szOrgHost, MAX_HOST_REALM_LEN);
				pstTransLog->szOrgHost[MAX_HOST_REALM_LEN] = 0x00;
				memcpy ( pstTransLog->szOrgRealm, pstDIAMETERINFO->szOrgRealm, MAX_HOST_REALM_LEN);
				pstTransLog->szOrgRealm[MAX_HOST_REALM_LEN] = 0x00;
			}
			else /**<  RESPONSE **/
			{
				memcpy ( pstTransLog->szOrgHost, pstDIAMETERINFO->szOrgHost, MAX_HOST_REALM_LEN);
				pstTransLog->szOrgHost[MAX_HOST_REALM_LEN] = 0x00;
				memcpy ( pstTransLog->szOrgRealm, pstDIAMETERINFO->szOrgRealm, MAX_HOST_REALM_LEN);
				pstTransLog->szOrgRealm[MAX_HOST_REALM_LEN] = 0x00;

				if ( pstDIAMETERINFO->uiResultCode == 0 ) {
					pstTransLog->uiResultCode = pstDIAMETERINFO->uiExpResultCode;
				} else {
					pstTransLog->uiResultCode = pstDIAMETERINFO->uiResultCode;
				}
				if ( pstDIAMETERINFO->uiResultCode > 3000 ) {
					pstTransLog->uiLastUserErrCode = DIAMETER_USER_FAILED;
				}
			}
			break;
		default:
			log_print (LOGN_CRI, "[%s][%s.%d] UNKNOWN MESSAGE TYPE: [%d]", __FILE__, __FUNCTION__, __LINE__, pstTransLog->uiCommandCode);
			break;
	}

	/*  LASTUSERERRCODE */	
	pstTransData->uiLastUserErrCode = pstTransLog->uiLastUserErrCode;

	/* IMSI */
	szTempMIN[0] = 0x00;
	dRet = sip_min( (char*)pstTransLog->szPublicID, strlen((char*)pstTransLog->szPublicID), (char*)szTempMIN, &dTempVendor );

	if( strlen((char*)szTempMIN) == 11 ) {
		/* XXX-YYYYY-ZZZZ */
		if( szTempMIN[2] == '9' )
			sprintf( (char*)pstTransLog->szIMSI, "45000%s", &szTempMIN[1] );
		else
			sprintf( (char*)pstTransLog->szIMSI, "45006%s", &szTempMIN[1] );
	}
	else if( strlen((char*)szTempMIN) == 10 ) {
		if( szTempMIN[2] == '9' )
			sprintf( (char*)pstTransLog->szIMSI, "45000%s", &szTempMIN[0] );
		else
			sprintf( (char*)pstTransLog->szIMSI, "45006%s", &szTempMIN[0] );
	}

	sprintf( (char*)pstTransData->szIMSI, "%s", pstTransLog->szIMSI );
	sprintf( (char*)pstTransData->szTraceMIN, "%s", szTempMIN );

	/* SEND TRACE INFO */
	if ( (dRet = dCheck_TraceInfo(pstTransData, pETHDATA, pCAPHEAD)) < 0 )
		log_print (LOGN_CRI, "FAILED dCheck_TraceInfo dRet:[%d]", dRet);

	/*  SEND TRANS LOG */	
	if ( dSendLogFlag ) {
		if ( (dRet=dSendTransLog(pCAPHEAD, &stTransKey, pstTransData)) < 0 )
		{
			log_print (LOGN_CRI, "[%s][%s.%d] dSendTransLog dRet:[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
			return -3;
		}
	}

	return 1;
}

