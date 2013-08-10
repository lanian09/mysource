/** A. FILE INCLUSION *********************************************************/

// LIB
#include "typedef.h"
#include "loglib.h"
#include "mems.h"
#include "nifo.h"
#include "cifo.h"
#include "gifo.h"
#include "hasho.h"
#include "timerN.h"
#include "utillib.h"

#include "Analyze_Ext_Abs.h"

// PROJECT
#include "procid.h"
#include "common_stg.h"
#include "capdef.h"

#include "adns_serv.h"
#include "adns_init.h"

/** B. DEFINITION OF NEW CONSTANTS ********************************************/

/** C. DEFINITION OF NEW TYPES ************************************************/

/** D. DECLARATION OF VARIABLES ***********************************************/
int						gACALLCnt;

extern stMEMSINFO		*pMEMSINFO;
extern stCIFO			*gpCIFO;
extern stHASHOINFO		*pDNSSESSHASH;
extern stTIMERNINFO		*pTIMERNINFO;

extern int				guiTimerValue;

/** E.1 DEFINITION OF FUNCTIONS ***********************************************/


/** E.2 DEFINITION OF FUNCTIONS ***********************************************/

/*******************************************************************************

*******************************************************************************/
int dGetCALLProcID(unsigned int uiClientIP)
{
	return SEQ_PROC_A_CALL + ( uiClientIP % gACALLCnt );
}

/*******************************************************************************

*******************************************************************************/
int dDNSProcess( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pETH, TCP_INFO *pTCP, U8 *pDATA, OFFSET offset )
{
	int						dRet;

	DNS_SESS_KEY			DNSSESS_KEY;
	DNS_SESS				*pDNSSESS;

	stHASHONODE			 *pHASHNODE;


	/* CHECK FOR TCP OR UDP */
	if( pTCP == NULL ) {
		/* UDP */
		dRet = dUDPDNSProcess( pCAPHEAD, pETH, pTCP, pDATA );
		if( dRet < 0 ) {
			log_print( LOGN_INFO, "[%s.%d] ERROR IN dUDPDNSProcess dRet:%d", __FUNCTION__, __LINE__, dRet );
			return -1;
		}
	}
	else {
		/* TCP */
		switch( pTCP->cTcpFlag ) {
			case DEF_TCP_START:
				/* NO ACTION */
				break;

			case DEF_TCP_END:
				DNSSESS_KEY.SIP	= pTCP->uiCliIP;
				DNSSESS_KEY.DIP	= pTCP->uiSrvIP;
				DNSSESS_KEY.usSPort	= pTCP->usCliPort;
				DNSSESS_KEY.usDPort	= pTCP->usSrvPort;

				if( (pHASHNODE = hasho_find( pDNSSESSHASH, (U8 *)&DNSSESS_KEY )) == NULL ) {
					log_print( LOGN_DEBUG, "NO DNS SESSION SIP:%u DIP:%u SPT:%u DPT:%u",
										DNSSESS_KEY.SIP, DNSSESS_KEY.DIP,
										DNSSESS_KEY.usSPort, DNSSESS_KEY.usDPort );
					return -2;
				}

				pDNSSESS = (DNS_SESS *)nifo_ptr( pDNSSESSHASH, pHASHNODE->offset_Data );

				if( (dRet = dSend_DNSLog( &DNSSESS_KEY, pDNSSESS )) < 0 ) {
					log_print( LOGN_INFO, "[%s.%d] ERROR IN dSend_DNSLog dRet:%d", __FUNCTION__, __LINE__, dRet );
					return -3;
				}

				/* DEL SESS */
				timerN_del( pTIMERNINFO, pDNSSESS->timerNID);
				hasho_del( pDNSSESSHASH, (U8 *)&DNSSESS_KEY );

				break;

			case DEF_TCP_DATA:
				dRet = dTCPDNSProcess( pCAPHEAD, pETH, pTCP, pDATA, offset );
				if( dRet < 0 ) {
					log_print( LOGN_INFO, "[%s.%d] ERROR IN dTCPDNSProcess dRet:%d", __FUNCTION__, __LINE__, dRet );
					return -4;
				}

				break;

			default:
				log_print( LOGN_CRI, "[%s.%d] NOT TCP/UDP:%d", __FUNCTION__, __LINE__, pETH->stUDPTCP.dUDPTCP );
				break;
		}
	}

	return 1;
}


/*******************************************************************************

*******************************************************************************/
int dSend_DNSLog( DNS_SESS_KEY *pDNSSESSKEY, DNS_SESS *pDNSSESS )
{
	int						dRet;
	UCHAR					*pstNode;
	LOG_DNS				 *pLOGDNS;

	if( (pstNode = nifo_node_alloc(pMEMSINFO)) == NULL ) {
		log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
		return -1;
	}

	if( (pLOGDNS = (LOG_DNS *)nifo_tlv_alloc(pMEMSINFO, pstNode, LOG_DNS_DEF_NUM, LOG_DNS_SIZE, DEF_MEMSET_OFF)) == NULL ) {
		log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, LOG_DNS_DEF_NUM );
		return -2;
	}

	memset( pLOGDNS, 0x00, LOG_DNS_SIZE );

	pLOGDNS->uiClientIP		 = pDNSSESSKEY->SIP;
	pLOGDNS->usPlatformType	 = pDNSSESS->usPlatformType;
	pLOGDNS->usSvcL4Type		= pDNSSESS->usSvcL4Type;
	pLOGDNS->usSvcL7Type		= pDNSSESS->usSvcL7Type;
	pLOGDNS->usIdentification	= pDNSSESS->usIdentification;
	pLOGDNS->ucErrorCode		= pDNSSESS->ucErrorCode;
	pLOGDNS->usSPort			= pDNSSESSKEY->usSPort;
	pLOGDNS->DIP				= pDNSSESSKEY->DIP;
	pLOGDNS->uiCallTime		 = pDNSSESS->dRequestTime;
	pLOGDNS->uiCallMTime		= pDNSSESS->dRequestMTime;
	pLOGDNS->dRequestTime		= pDNSSESS->dRequestTime;
	pLOGDNS->dRequestMTime		= pDNSSESS->dRequestMTime;
	pLOGDNS->dResponseTime		= pDNSSESS->dResponseTime;
	pLOGDNS->dResponseMTime	 = pDNSSESS->dResponseMTime;
	pLOGDNS->usRequestCnt		= pDNSSESS->usRequestCnt;
	pLOGDNS->usResponseCnt		= pDNSSESS->usResponseCnt;
	pLOGDNS->uiRequestSize		= pDNSSESS->uiRequestSize;
	pLOGDNS->uiResponseSize	 = pDNSSESS->uiResponseSize;

	dRet = gifo_write( pMEMSINFO, gpCIFO, SEQ_PROC_A_DNS, dGetCALLProcID(pLOGDNS->uiClientIP), nifo_offset(pMEMSINFO, pstNode));
	if( dRet	< 0 ) {
		log_print(LOGN_CRI, "[%s][%s.%d] gifo_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
		return -3;
	}

	return 1;
}


/*******************************************************************************

*******************************************************************************/
int dUDPDNSProcess( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pETH, TCP_INFO *pTCP, U8 *pDATA )
{
	int						dRet;
	U16					 usIdent;
	U8						ucRCode;
	DNS_SESS_KEY			DNSSESS_KEY;
	DNS_SESS_KEY			*pDNSSESS_KEY = &DNSSESS_KEY;
	DNS_SESS				DNSSESS, *pDNSSESS;
	PDNS					pDNS;
	DNS_SESS_KEY			COMMON;

	stHASHONODE			 *pHASHNODE;

	
	if( pCAPHEAD->bRtxType == DEF_FROM_CLIENT ) {
		if( pETH->stUDPTCP.wDestPort != DEF_PROTOCOL_DNS_PORT ) {
			log_print( LOGN_CRI, "INVALID PORT INFO DEF_FROM_CLIENT DPORT:%d SPORT:%d",
								pETH->stUDPTCP.wDestPort, pETH->stUDPTCP.wSrcPort );
			return -1;
		}

		pDNSSESS_KEY->SIP 		= pETH->stIP.dwSrcIP;
		pDNSSESS_KEY->DIP		= pETH->stIP.dwDestIP;
		pDNSSESS_KEY->usSPort	= pETH->stUDPTCP.wSrcPort;
		pDNSSESS_KEY->usDPort	= pETH->stUDPTCP.wDestPort;
	}
	else if( pCAPHEAD->bRtxType == DEF_FROM_SERVER ) {
		if( pETH->stUDPTCP.wSrcPort != DEF_PROTOCOL_DNS_PORT ) {
			log_print( LOGN_CRI, "INVALID PORT INFO DEF_FROM_SERVER SPORT:%d DPORT:%d",
								pETH->stUDPTCP.wSrcPort, pETH->stUDPTCP.wDestPort );
			return -2;
		}

		pDNSSESS_KEY->SIP 		= pETH->stIP.dwDestIP;
		pDNSSESS_KEY->DIP		= pETH->stIP.dwSrcIP;
		pDNSSESS_KEY->usSPort	= pETH->stUDPTCP.wDestPort;
		pDNSSESS_KEY->usDPort	= pETH->stUDPTCP.wSrcPort;
	}
	else {
		log_print( LOGN_CRI, "INVALID DIRECTION RTX:%d", pCAPHEAD->bRtxType );
		return -3;
	}

	/* CHECK DNS SESS */
	if( (pHASHNODE = hasho_find( pDNSSESSHASH, (U8 *)pDNSSESS_KEY )) == NULL ) {
		if( pCAPHEAD->bRtxType == DEF_FROM_CLIENT ) {
			if( (pHASHNODE = hasho_add( pDNSSESSHASH, (U8 *)pDNSSESS_KEY, (U8 *)&DNSSESS )) == NULL ) {
				log_print( LOGN_CRI, "[%s.%d] ERROR IN hasho_add NULL", __FUNCTION__, __LINE__ );
				return -4;
			}

			pDNSSESS = (DNS_SESS *)nifo_ptr( pDNSSESSHASH, pHASHNODE->offset_Data );
			memcpy( &COMMON, pDNSSESS_KEY, DNS_SESS_KEY_SIZE );
			pDNSSESS->timerNID = timerN_add(pTIMERNINFO, invoke_del_DNS, (U8 *)&COMMON, DNS_SESS_KEY_SIZE, time(NULL) + guiTimerValue );

			InitDNSSESS( pDNSSESS );
		}
		else {
			log_print( LOGN_CRI, "CURRENT RESPONSE & NO DNS SESSION SIP:%u SPORT:%u DIP:%u DPORT:%u",
								pETH->stIP.dwSrcIP, pETH->stUDPTCP.wSrcPort, pETH->stIP.dwDestIP, pETH->stUDPTCP.wDestPort );
			return -5;
		}
	}
	else
		pDNSSESS = (DNS_SESS *)nifo_ptr( pDNSSESSHASH, pHASHNODE->offset_Data );

	/* ANALYZE DNS PROTOCOL & SET DNS SESS INFO */
	pDNS = (PDNS)(pDATA + 14 + pETH->stIP.wIPHeaderLen + pETH->stUDPTCP.wHeaderLen);

	/* CHECK QUERY & REPLY */
	if( (pDNS->Param[0] & DEF_CHECK_QR) == DEF_CHECK_QR) {
		/* REPLY & CHECK DIRECTION */
		if( pCAPHEAD->bRtxType != DEF_FROM_SERVER ) {
			log_print( LOGN_CRI, "DEF_FROM_SERVER & REPLY!!" );
			return -6;
		}

		pDNSSESS->timerNID = timerN_update(pTIMERNINFO, pDNSSESS->timerNID, time(NULL) + guiTimerValue );

		/* CHECK IDENTIFICATION */
		memcpy( &usIdent, &pDNS->Ident[0], 2 );
		if( usIdent != pDNSSESS->usIdentification ) {
			log_print( LOGN_CRI, "INVALID IDENT SESS SPORT:%u DIP:%u",
								pDNSSESS_KEY->usSPort, pDNSSESS_KEY->DIP );
			return -7;
		}

		if( pDNSSESS->dRequestTime == 0 && pDNSSESS->dRequestMTime == 0 ) {
			/* NO QUERY & REPLY */
			log_print( LOGN_CRI, "NO QUERY, NEW REPLY!!" );

			/* DEL SESS */
			timerN_del( pTIMERNINFO, pDNSSESS->timerNID);
			hasho_del( pDNSSESSHASH, (U8 *)pDNSSESS_KEY );
		}
		else {
			pDNSSESS->dResponseTime	 = pCAPHEAD->curtime;
			pDNSSESS->dResponseMTime	= pCAPHEAD->ucurtime;
		}

		pDNSSESS->usResponseCnt++;
		pDNSSESS->uiResponseSize		+= pETH->stIP.wTotalLength;

		/* CHECK RCODE */
		ucRCode = (pDNS->Param[1] & DEF_CHECK_RCODE);

		log_print( LOGN_INFO, "REPLY INFO TM:%u.%u CNT:%u SZ:%u RCode:%u",
							pDNSSESS->dResponseTime, pDNSSESS->dResponseMTime,
							pDNSSESS->usResponseCnt, pDNSSESS->uiResponseSize, ucRCode );
		if( ucRCode == 0 ) {
			/* SUCCESS, SEND LOG_DNS */
			if( (dRet = dSend_DNSLog( pDNSSESS_KEY, pDNSSESS )) < 0 ) {
				log_print( LOGN_INFO, "[%s.%d] ERROR IN dSend_DNSLog dRet:%d", __FUNCTION__, __LINE__, dRet );
				return -8;
			}
			else
				log_print( LOGN_INFO, "SUCCESS dSend_DNSLog" );

			/* DEL SESS */
			timerN_del( pTIMERNINFO, pDNSSESS->timerNID);
			hasho_del( pDNSSESSHASH, (U8 *)pDNSSESS_KEY );
		}
		else {
			/* FAIL, CONTINUE */
			pDNSSESS->ucErrorCode = ucRCode;
		}
	}
	else if( (pDNS->Param[0] & DEF_CHECK_QR) == 0 ) {
		/* QUERY & CHECK DIRECTION */
		if( pCAPHEAD->bRtxType != DEF_FROM_CLIENT ) {
			log_print( LOGN_CRI, "DEF_FROM_CLIENT & REPLY!!" );
			return -9;
		}

		if( pDNSSESS->dRequestTime != 0 && pDNSSESS->dRequestMTime != 0 ) {
			/* RETRY */
			pDNSSESS->timerNID = timerN_update(pTIMERNINFO, pDNSSESS->timerNID, time(NULL) + guiTimerValue );
		}
		else {
			pDNSSESS->dRequestTime		= pCAPHEAD->curtime;
			pDNSSESS->dRequestMTime	 = pCAPHEAD->ucurtime;

			pDNSSESS->usPlatformType	= DEF_PLATFORM_ETC;
			pDNSSESS->usSvcL4Type		= pETH->usL4Code;
			pDNSSESS->usSvcL7Type		= pETH->usL7Code;

			memcpy( &pDNSSESS->usIdentification, &pDNS->Ident[0], 2 );
			pDNSSESS->ucErrorCode		= 0;
		}

		pDNSSESS->usRequestCnt++;
		pDNSSESS->uiRequestSize	 += pETH->stIP.wTotalLength;

		log_print( LOGN_INFO, "QUERY INFO TM:%u.%u CNT:%u SZ:%u",
							pDNSSESS->dRequestTime, pDNSSESS->dRequestMTime, 
							pDNSSESS->usRequestCnt, pDNSSESS->uiRequestSize );
	}
	else {
		log_print( LOGN_CRI, "UNKNOWN QUERY OR REPLY:%u", pDNS->Param[0] );
		return -10;
	}

	return 1;
}


/*******************************************************************************

*******************************************************************************/
int dTCPDNSProcess( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pETH, TCP_INFO *pTCP, U8 *pDATA, OFFSET offset )
{
	int						dRet;
	U16					 usIdent;
	U8						ucRCode;
	U8						szIP[32];
	U16						usLength;
	DNS_SESS_KEY			DNSSESS_KEY;
	DNS_SESS_KEY			*pDNSSESS_KEY = &DNSSESS_KEY;
	DNS_SESS				DNSSESS, *pDNSSESS;
	PDNS					pDNS;
	DNS_SESS_KEY			COMMON;

	stHASHONODE			 *pHASHNODE;


	if( pTCP->usSrvPort != DEF_PROTOCOL_DNS_PORT ) {
		log_print( LOGN_CRI, "INVALID PORT INFO DEF_FROM_CLIENT DPORT:%d SPORT:%d",
							pTCP->usSrvPort, pTCP->usCliPort );
		return -1;
	}

	/* SET DNS SESS KEY */
	pDNSSESS_KEY->SIP		= pTCP->uiCliIP;
	pDNSSESS_KEY->DIP		= pTCP->uiSrvIP;
	pDNSSESS_KEY->usSPort	= pTCP->usCliPort;
	pDNSSESS_KEY->usDPort	= pTCP->usSrvPort;

	/* CHECK DNS SESS */
	if( (pHASHNODE = hasho_find( pDNSSESSHASH, (U8 *)pDNSSESS_KEY )) == NULL ) {
		if( pTCP->ucRtx == DEF_FROM_CLIENT ) {
			if( (pHASHNODE = hasho_add( pDNSSESSHASH, (U8 *)pDNSSESS_KEY, (U8 *)&DNSSESS )) == NULL ) {
				log_print( LOGN_CRI, "[%s.%d] ERROR IN hasho_add NULL", __FUNCTION__, __LINE__ );
				return -1;
			}

			pDNSSESS = (DNS_SESS *)nifo_ptr( pDNSSESSHASH, pHASHNODE->offset_Data );
			memcpy( &COMMON, pDNSSESS_KEY, DNS_SESS_KEY_SIZE );
			pDNSSESS->timerNID = timerN_add(pTIMERNINFO, invoke_del_DNS, (U8 *)&COMMON, DNS_SESS_KEY_SIZE, time(NULL) + guiTimerValue );

			InitDNSSESS( pDNSSESS );

			log_print( LOGN_DEBUG, "CREATE NEW DNS SESS SIP:%s SPT:%u DIP:%s DPT:%u", 
								util_cvtipaddr(szIP, pDNSSESS_KEY->SIP), pDNSSESS_KEY->usSPort,
								util_cvtipaddr(szIP, pDNSSESS_KEY->DIP), pDNSSESS_KEY->usDPort );
								
		}
		else {
			log_print( LOGN_CRI, "CURRENT RESPONSE & NO DNS SESSTION SIP:%u SPORT:%u DIP:%u DPORT:%u",
								pTCP->uiCliIP, pTCP->usCliPort, pTCP->uiSrvIP, pTCP->usSrvPort );
			return -2;
		}
	}
	else
		pDNSSESS = (DNS_SESS *)nifo_ptr( pDNSSESSHASH, pHASHNODE->offset_Data );

	log_print( LOGN_INFO, "uiTotLength:%u uiCurLength:%u uiDataSize:%u",
					pDNSSESS->uiTotLength, pDNSSESS->uiCurLength, pTCP->uiDataSize );

	/* CHECK DATA SIZE */
	if( pDNSSESS->uiTotLength == 0 ) {
		if( pTCP->uiDataSize == 2 ) {
			/* DATA LENGTH */
			memcpy( &usLength, &pDATA[pTCP->uiSOffset], 2 );
			usLength = util_cvtushort(usLength);

			pDNSSESS->uiTotLength = usLength;
			pDNSSESS->uiCurLength = 2;

			if( pTCP->ucRtx == DEF_FROM_CLIENT ) {
				pDNSSESS->dRequestTime		= pTCP->uiCapTime;
				pDNSSESS->dRequestMTime	 = pTCP->uiCapMTime;

				pDNSSESS->usRequestCnt++;
				pDNSSESS->uiRequestSize		 += pTCP->uiDataSize;
				pDNSSESS->usPlatformType	= DEF_PLATFORM_ETC;
				pDNSSESS->usSvcL4Type		= pTCP->usL4Code;
				pDNSSESS->usSvcL7Type		= pTCP->usL7Code;
			}
			else {
				pDNSSESS->dResponseTime	 = pTCP->uiCapTime;
				pDNSSESS->dResponseMTime	= pTCP->uiCapMTime;

				pDNSSESS->usResponseCnt++;
				pDNSSESS->uiResponseSize		+= pTCP->uiDataSize;

				pDNSSESS->usPlatformType	= DEF_PLATFORM_ETC;
				pDNSSESS->usSvcL4Type		= pTCP->usL4Code;
				pDNSSESS->usSvcL7Type		= pTCP->usL7Code;
			}

			return 0;
		}
		else {
			memcpy( &usLength, &pDATA[pTCP->uiSOffset], 2 );
			usLength = util_cvtushort(usLength);

			if( usLength != (pTCP->uiDataSize - 2) ) {
				log_print( LOGN_DEBUG, "INVALID LENGTH INFO LEN:%u DATASIZE:%u", usLength, (pTCP->uiDataSize - 2) );
				return -1;
			}

			pDNSSESS->uiTotLength = usLength;
			pDNSSESS->uiCurLength = pTCP->uiDataSize;

			pDNS = (PDNS)(&pDATA[pTCP->uiSOffset + 2]);

			/*
			if( pTCP->uiDataSize < pDNSSESS->uiTotLength ) {
				log_print( LOGN_CRI, "CHECK CASE IN TCP 1 !!!!!" );
				pDNSSESS->offset_Prev = offset;
				return 0;
			}
			else {
				pDNS = (PDNS)(&pDATA[pTCP->uiSOffset + 2]);	
			}
			*/
		}
	}
	else {
		if( pDNSSESS->uiCurLength == 2 ) {
			if( pDNSSESS->uiTotLength == pTCP->uiDataSize ) {
				pDNS = (PDNS)(&pDATA[pTCP->uiSOffset]);

				pDNSSESS->uiTotLength = 0;
				pDNSSESS->uiCurLength = 0;
			}
			else
				return 0;
		}
		else {
			pDNS = (PDNS)(&pDATA[pTCP->uiSOffset]);

			pDNSSESS->uiTotLength = 0;
			pDNSSESS->uiCurLength = 0;
		}

		/*
		else if( pDNSSESS->offset_Prev != 0 ) {
			log_print( LOGN_CRI, "CHECK CASE IN TCP 2 !!!!!");
			return 0;
		}
		*/
	}

	/* CHECK QUERY & REPLY */
	if( (pDNS->Param[0] & DEF_CHECK_QR) == DEF_CHECK_QR) {
		/* REPLY & CHECK DIRECTION */
		if( pTCP->ucRtx != DEF_FROM_SERVER ) {
			log_print( LOGN_CRI, "DEF_FROM_SERVER & QUERY PARAM:%u RTX:%d SIP:%u SPT:%u DIP:%u DPT:%u",
								pDNS->Param[0], pTCP->ucRtx, pTCP->uiCliIP, pTCP->usCliPort,	pTCP->uiSrvIP, pTCP->usSrvPort );
			return -3;
		}

		pDNSSESS->timerNID = timerN_update(pTIMERNINFO, pDNSSESS->timerNID, time(NULL) + guiTimerValue );

		/* CHECK IDENTIFICATION */
		memcpy( &usIdent, &pDNS->Ident[0], 2 );
		if( usIdent != pDNSSESS->usIdentification ) {
			log_print( LOGN_CRI, "INVALID IDENT SESS SPORT:%u DIP:%u %u:%u",
								pDNSSESS_KEY->usSPort, pDNSSESS_KEY->DIP, usIdent, pDNSSESS->usIdentification );
			return -4;
		}

		if( pDNSSESS->dRequestTime == 0 && pDNSSESS->dRequestMTime == 0 ) {
			/* NO QUERY & REPLY */
			log_print( LOGN_CRI, "NO QUERY, NEW REPLY!!" );

			/* DEL SESS */
			timerN_del( pTIMERNINFO, pDNSSESS->timerNID);
			hasho_del( pDNSSESSHASH, (U8 *)pDNSSESS_KEY );
		}
		else {
			pDNSSESS->dResponseTime	 = pTCP->uiCapTime;
			pDNSSESS->dResponseMTime	= pTCP->uiCapMTime;
		}

		pDNSSESS->usResponseCnt++;
		
		pDNSSESS->uiResponseSize		+= pTCP->uiIPTotDnPktSize;
		pDNSSESS->uiRequestSize			+= pTCP->uiIPTotUpPktSize;

		/* CHECK RCODE */
		ucRCode = (pDNS->Param[1] & DEF_CHECK_RCODE);

		log_print( LOGN_INFO, "REPLY INFO TM:%u.%u CNT:%u SZ:%u RCode:%u",
							pDNSSESS->dResponseTime, pDNSSESS->dResponseMTime,
							pDNSSESS->usResponseCnt, pDNSSESS->uiResponseSize, ucRCode );
		if( ucRCode == 0 ) {
			/* SUCCESS, SEND LOG_DNS */
			if( (dRet = dSend_DNSLog( pDNSSESS_KEY, pDNSSESS )) < 0 ) {
				log_print( LOGN_INFO, "[%s.%d] ERROR IN dSend_DNSLog dRet:%d", __FUNCTION__, __LINE__, dRet );
				return -5;
			}
			else
				log_print( LOGN_INFO, "SUCCESS dSend_DNSLog" );

			/* DEL SESS */
			timerN_del( pTIMERNINFO, pDNSSESS->timerNID);
			hasho_del( pDNSSESSHASH, (U8 *)pDNSSESS_KEY );
		}
		else {
			/* FAIL, CONTINUE */
			pDNSSESS->ucErrorCode = ucRCode;
		}
	}
	else if( (pDNS->Param[0] & DEF_CHECK_QR) == 0 ) {
		/* QUERY & CHECK DIRECTION */
		if( pTCP->ucRtx != DEF_FROM_CLIENT ) {
			log_print( LOGN_CRI, "DEF_FROM_CLIENT & REPLY PARAM:%u RTX:%d SIP:%u SPT:%u DIP:%u DPT:%u",
								pDNS->Param[0], pTCP->ucRtx, pTCP->uiCliIP, pTCP->usCliPort,	pTCP->uiSrvIP, pTCP->usSrvPort );
			return -6;
		}

		//if( pDNSSESS->dRequestTime != 0 && pDNSSESS->dRequestMTime != 0 ) {
		if( pDNSSESS->uiCurLength > 0 && pDNSSESS->uiCurLength > 2 ) {
			/* RETRY */
			pDNSSESS->timerNID = timerN_update(pTIMERNINFO, pDNSSESS->timerNID, time(NULL) + guiTimerValue );
		}
		else {
			pDNSSESS->dRequestTime		= pTCP->uiCapTime;
			pDNSSESS->dRequestMTime	 = pTCP->uiCapMTime;

			pDNSSESS->usPlatformType	= DEF_PLATFORM_ETC;
			pDNSSESS->usSvcL4Type		= pTCP->usL4Code;
			pDNSSESS->usSvcL7Type		= pTCP->usL7Code;

			memcpy( &pDNSSESS->usIdentification, &pDNS->Ident[0], 2 );
			pDNSSESS->ucErrorCode		= 0;
		}

		pDNSSESS->usRequestCnt++;

		pDNSSESS->uiResponseSize		+= pTCP->uiIPTotDnPktSize;
		pDNSSESS->uiRequestSize		 += pTCP->uiIPTotUpPktSize;

		log_print( LOGN_INFO, "QUERY INFO TM:%u.%u CNT:%u SZ:%u",
							pDNSSESS->dRequestTime, pDNSSESS->dRequestMTime,
							pDNSSESS->usRequestCnt, pDNSSESS->uiRequestSize );
	}
	else {
		log_print( LOGN_CRI, "UNKNOWN QUERY OR REPLY:%u", pDNS->Param[0] );
		return -7;
	}


	return 1;
}


/*******************************************************************************

*******************************************************************************/
void InitDNSSESS( DNS_SESS *pDNSSESS )
{
	pDNSSESS->usPlatformType 	= 0;
	pDNSSESS->usSvcL4Type		= 0;
	pDNSSESS->usSvcL7Type		= 0;
	pDNSSESS->usIdentification	= 0;
	pDNSSESS->ucErrorCode		= 0;
	pDNSSESS->dRequestTime		= 0;
	pDNSSESS->dRequestMTime		= 0;
	pDNSSESS->dResponseTime		= 0;
	pDNSSESS->dResponseMTime	= 0;
	pDNSSESS->usRequestCnt		= 0;
	pDNSSESS->usResponseCnt		= 0;
	pDNSSESS->uiRequestSize		= 0;
	pDNSSESS->uiResponseSize	= 0;
	pDNSSESS->uiTotLength		= 0;
	pDNSSESS->uiCurLength		= 0;
	pDNSSESS->offset_Prev		= 0;
}


