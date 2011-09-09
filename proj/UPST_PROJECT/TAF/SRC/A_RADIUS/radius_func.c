/**<	
 $Id: radius_func.c,v 1.4 2011/09/07 06:30:38 dcham Exp $
 $Author: dcham $
 $Revision: 1.4 $
 $Date: 2011/09/07 06:30:38 $
 **/

#include <ctype.h>
#include <sys/time.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "ipclib.h"
#include "filelib.h"
#include "mems.h"
#include "memg.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

// PROJECT
#include "procid.h"
#include "filter.h"
#include "common_stg.h"

// .
#include "radius_func.h"

/**
 * Declare variables
 */
st_Flt_Info	 *flt_info;

extern stMEMSINFO		*pstMEMSINFO;
extern stCIFO			*gpCIFO;
extern stHASHOINFO		*pstHASHOINFO;
extern stTIMERNINFO		*pstTIMERNINFO;

extern st_TraceList		*pstTRACE;		/* TRACE */

extern st_ippool		stIPPool;

extern S32				gTIMER_TRANS;
extern S32				gACALLCnt;


#include "radius_func.h"

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

char *szPrintType(U32 type)
{	
	switch(type)
	{
		case 1:
			return "START";
		case 2:		
			return "STOP";
		case 3:		
			return "INTERIM";
		default:	
			return "UNKNOWN";
	}
}

char *szPrintCode(U8 type)
{	
	switch(type)
	{
		case RADIUS_ACCESS_REQUEST:
			return "ACCESS-REQUEST";
		case RADIUS_ACCESS_ACCEPT:		
			return "ACCESS-ACCEPT";
		case RADIUS_ACCESS_REJECT:		
			return "ACCESS-REJECT";
		case RADIUS_ACCOUNT_REQUEST:		
			return "ACCOUNTING-REQUEST";
		case RADIUS_ACCOUNT_RESPONSE:		
			return "ACCOUNTING-RESPONSE";
		default:	
			return "UNKNOWN";
	}
}

U8 ucGetMsgType(U8 code)
{
	switch(code)
	{
		case RADIUS_ACCESS_REQUEST:
		case RADIUS_ACCESS_ACCEPT:
		case RADIUS_ACCESS_REJECT:
			return RADIUS_ACCESS_REQUEST;
		case RADIUS_ACCOUNT_REQUEST:
		case RADIUS_ACCOUNT_RESPONSE:
			return RADIUS_ACCOUNT_REQUEST;
		default:
			return 0;
	}
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

S32 dGetCALLProcID(U32 uiClientIP)
{
	return SEQ_PROC_A_CALL + ( uiClientIP % gACALLCnt );
}

int dSendTransLog(Capture_Header_Msg *pCAPHEAD, HKey_Trans *pstTransKey, HData_Trans *pstTransData)
{
	int					dRet;
	S32					dSndProcID;
	struct timeval		now;

	UCHAR				*pNode;
	LOG_SIGNAL			*pstTransLog;

	pstTransLog = (LOG_SIGNAL *) nifo_ptr (pstMEMSINFO, pstTransData->dOffset);
	pNode = nifo_ptr (pstMEMSINFO, nifo_get_offset_node (pstMEMSINFO, (U8 *)pstTransLog));

	gettimeofday(&now, 0);

	/*	COMMON */
	if ( pCAPHEAD==NULL ) {
/* TIMEOUT 일경우 Response를 그리지 않기 위해 삭제
		pstTransLog->uiSessEndTime 		= now.tv_sec;
		pstTransLog->uiSessEndMTime 	= now.tv_usec;
*/
	} else {
		pstTransLog->uiSessEndTime 		= pCAPHEAD->curtime;
		pstTransLog->uiSessEndMTime 	= pCAPHEAD->ucurtime;
		pstTransLog->uiSessDuration 	= GetGapTime(pstTransLog->uiSessEndTime, pstTransLog->uiSessEndMTime, 
				pstTransLog->uiSessStartTime, pstTransLog->uiSessStartMTime);
	}

	if( pstTransData->uiLastUserErrCode == RADIUS_USER_TIMEOUT ) {
		if( pstTransLog->uiMsgType == RADIUS_ACCESS_REQUEST )
			pstTransLog->uiLastUserErrCode = DEF_ACCESS_TIMEOUT;
		else
			pstTransLog->uiLastUserErrCode = DEF_ACCOUNT_TIMEOUT;
	}

//	LOG_SIGNAL_Prt("PRINT LOG_SIGNAL", pstTransLog);

	/*	SEND LOG */
	dSndProcID = dGetCALLProcID(pstTransLog->uiClientIP);
	dRet = gifo_write (pstMEMSINFO, gpCIFO, SEQ_PROC_A_RADIUS, dSndProcID, nifo_offset( pstMEMSINFO, pNode) );
	if (dRet < 0) 
	{
		log_print (LOGN_CRI, "[%s][%s.%d] gifo_wrtie dRet:[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		nifo_node_delete(pstMEMSINFO, pNode);
		return RET_FAIL;
	}

	/*	DELETE TRANS INFO */
	if (pstTransData->ulTimerNID != (long long)0) {
		timerN_del (pstTIMERNINFO, pstTransData->ulTimerNID);
	}
	hasho_del (pstHASHOINFO, (U8 *)pstTransKey);

	log_print (LOGN_DEBUG,	"	--> DEL HASH		SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d TID: %lld", 
							HIPADDR(pstTransKey->uiSrcIP), HIPADDR(pstTransKey->uiDestIP), pstTransData->ulTimerNID);

	return RET_SUCC;
}

int cb_timeout_transaction(HKey_Trans *pstTransKey)
{
	int 			dRet;
	HData_Trans		*pstTransData;
	stHASHONODE		*pstTransHashNode;

	pstTransHashNode = hasho_find (pstHASHOINFO, (U8*)pstTransKey);
	pstTransData = (HData_Trans *) nifo_ptr (pstHASHOINFO, pstTransHashNode->offset_Data);
	pstTransData->uiLastUserErrCode = RADIUS_USER_TIMEOUT;
	
	log_print (LOGN_DEBUG, "	--> TIME OUT		SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d ID: %d MSGTYPE: %d", HIPADDR(pstTransKey->uiSrcIP), HIPADDR(pstTransKey->uiDestIP), pstTransKey->ucID, pstTransKey->ucMsgType);

	if ( (dRet=dSendTransLog(NULL, pstTransKey, pstTransData)) < 0 )
	{
		log_print (LOGN_CRI, "dSendTransLog() MINUS");
		return RET_FAIL;
	}

	return RET_SUCC;
}

int pSendStartCall(LOG_SIGNAL *pstTransLog)
{
	int 		dRet;
	S32			dSndProcID;
	UCHAR		*pstStartNode;
	LOG_SIGNAL	*pstStartCall;

	struct timeval	now;

	gettimeofday(&now, 0);

	/**<	ADD RADIUS_START NIFO NODE **/	
	if( (pstStartNode = nifo_node_alloc(pstMEMSINFO)) == NULL ) {
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	if( (pstStartCall = (LOG_SIGNAL *) 
				nifo_tlv_alloc(pstMEMSINFO, pstStartNode, RADIUS_START_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_ON)) == NULL )
	{
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_delete(pstMEMSINFO, pstStartNode);
		return -2;
	}

	memcpy(pstStartCall, pstTransLog, LOG_SIGNAL_SIZE);

//	LOG_SIGNAL_Prt("PRINT START CALL", pstStartCall);

	/* SEND Start CAll */
	dSndProcID = dGetCALLProcID(pstTransLog->uiClientIP);
	dRet = gifo_write (pstMEMSINFO, gpCIFO, SEQ_PROC_A_RADIUS, dSndProcID, nifo_offset(pstMEMSINFO, pstStartNode) );
	if (dRet < 0) {
		log_print (LOGN_CRI, "[%s][%s.%d] gifo_write dRet:[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		nifo_node_delete(pstMEMSINFO, pstStartNode);
		return -3;
	}

	log_print (LOGN_DEBUG, "	--> SEND START CALL	CLTIP: %d.%d.%d.%d IMSI: %s", 
			HIPADDR(pstStartCall->uiClientIP), pstStartCall->szIMSI);

	return 0;
}

HData_Trans *pCreateTransaction(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, HKey_Trans *pstTransKey)
{
	HData_Trans			*pstTransData;
	HData_Trans			stTransData;

	UCHAR				*pstLogNode;
	LOG_SIGNAL			*pstTransLog;

	stHASHONODE			*pstTransHashNode;

	struct timeval	now;

	pstTransData = &stTransData;

	/**<	INIT HDATA_TRANS **/	
	pstTransData->ulTimerNID		= 0;
	pstTransData->dOffset			= 0;
	pstTransData->dReqFlag			= 0;
	pstTransData->uiLastUserErrCode	= 0;
	pstTransData->uiRetransReqCnt	= 0;

	gettimeofday(&now, 0);

	/**<	ADD st_RADIUSSIG_LOG NIFO NODE **/	
	if( (pstLogNode = nifo_node_alloc(pstMEMSINFO)) == NULL )
	{
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

	if( (pstTransLog = (LOG_SIGNAL *) nifo_tlv_alloc(
					pstMEMSINFO, pstLogNode, LOG_PISIGNAL_DEF_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_ON)) == NULL )
	{
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_delete(pstMEMSINFO, pstLogNode);
		return NULL;
	}
	pstTransData->dOffset = nifo_offset (pstMEMSINFO, pstTransLog);

	/**<	ADD HASHO, TIMERN NODE **/	
	if ( (pstTransHashNode = hasho_add(pstHASHOINFO, (U8*)pstTransKey, (U8*)pstTransData)) == NULL )
	{
		log_print (LOGN_CRI, "[%s][%s.%d] hasho_add NULL ", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_delete(pstMEMSINFO, pstLogNode);
		return NULL;
	} else {
		gTIMER_TRANS = flt_info->stTimerInfo.usTimerInfo[PI_RAD_TIMEOUT];
		pstTransData = (HData_Trans*) nifo_ptr (pstHASHOINFO, pstTransHashNode->offset_Data);
		pstTransData->ulTimerNID = timerN_add (pstTIMERNINFO, (void *)&cb_timeout_transaction, 
				(U8*)pstTransKey, DEF_TDATA_TRANS_TIMER_SIZE, time(NULL) + gTIMER_TRANS);

		log_print (LOGN_DEBUG, "	--> ADD HASH		SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d TID: %lld OFFSET: %ld", 
				HIPADDR(pstTransKey->uiSrcIP), HIPADDR(pstTransKey->uiDestIP), 
				pstTransData->ulTimerNID, pstTransData->dOffset);
	}

	return pstTransData;
}

int dCheck_TraceInfo( HData_Trans *pstSessData, UCHAR *pData, Capture_Header_Msg *pstCAP )
{
	int	 i, dRet, offset;		
	UCHAR	*pstNode;
	UCHAR	*pBuffer;

	st_TraceMsgHdr	*pstTrcMsg;

	/*	
	 * pstSessData->szIMSI		: 450061024441245
	 * pstSessData->szTraceMIN	: 010~
	 *
	 */

	for( i=0; i< pstTRACE->count; i++ ) {
		if( pstTRACE->stTraceInfo[i].dType == TRC_TYPE_IMSI ) {
			if( atoll((char*)pstSessData->szIMSI) == pstTRACE->stTraceInfo[i].stTraceID.llIMSI ) {
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

				pstTrcMsg->time		 = pstCAP->curtime;
				pstTrcMsg->mtime		= pstCAP->ucurtime;
				pstTrcMsg->dType		= pstTRACE->stTraceInfo[i].dType;
				pstTrcMsg->usDataLen	= pstCAP->datalen;
				memcpy( &pstTrcMsg->stTraceID, &pstTRACE->stTraceInfo[i].stTraceID, st_TraceID_SIZE );
				memcpy( pBuffer, pData, pstCAP->datalen );

				if( (dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_RADIUS, SEQ_PROC_CI_LOG, nifo_offset(pstMEMSINFO, pstNode) )) < 0 ) {
					log_print(LOGN_CRI, "[%s][%s.%d] gifo_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
					return -4;
				}

				return 1;
			}
		}
		else if( pstTRACE->stTraceInfo[i].dType == TRC_TYPE_MDN ) {
			if( atoll((char*)pstSessData->szTraceMIN) == pstTRACE->stTraceInfo[i].stTraceID.llIMSI) {
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

				pstTrcMsg->time		 = pstCAP->curtime;
				pstTrcMsg->mtime		= pstCAP->ucurtime;
				pstTrcMsg->dType		= pstTRACE->stTraceInfo[i].dType;
				pstTrcMsg->usDataLen	= pstCAP->datalen;
				memcpy( &pstTrcMsg->stTraceID, &pstTRACE->stTraceInfo[i].stTraceID, st_TraceID_SIZE );
				memcpy( pBuffer, pData, pstCAP->datalen );

				if( (dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_RADIUS, SEQ_PROC_CI_LOG, nifo_offset(pstMEMSINFO,pstNode) )) < 0 ) {
					log_print(LOGN_CRI, "[%s][%s.%d] gifo_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
					return -4;
				}

				return 1;
			}
		}
		else if( pstTRACE->stTraceInfo[i].dType == TRC_TYPE_ROAM_IMSI || pstTRACE->stTraceInfo[i].dType == TRC_TYPE_ROAM_MDN) {
			// IRM 타입일 경우 IMSI의 뒷자리 10자리를 atoll 로 변환하여 TRACE정보의 llIMSI 값과 비교
	
			if(strlen((char*)pstSessData->szIMSI) < DEF_IRM_LEN) {
				log_print(LOGN_CRI, "[%s][%s.%d] MDN SIZE LESS THAN %d [%s", __FILE__, __FUNCTION__, __LINE__, DEF_IRM_LEN, pstSessData->szIMSI);
				return 1;
			}
			offset = strlen((char*)pstSessData->szIMSI) - DEF_IRM_LEN;
			log_print(LOGN_DEBUG, "	@@@ TRACE IRM: %lld OFFSET: %d", atoll((char*)&pstSessData->szIMSI[offset]), offset);
			if( atoll((char*)&pstSessData->szIMSI[offset]) == pstTRACE->stTraceInfo[i].stTraceID.llIMSI) {

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

				pstTrcMsg->time		 = pstCAP->curtime;
				pstTrcMsg->mtime		= pstCAP->ucurtime;
				pstTrcMsg->dType		= pstTRACE->stTraceInfo[i].dType;
				pstTrcMsg->usDataLen	= pstCAP->datalen;
				memcpy( &pstTrcMsg->stTraceID, &pstTRACE->stTraceInfo[i].stTraceID, st_TraceID_SIZE );
				memcpy( pBuffer, pData, pstCAP->datalen );

				if( (dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_RADIUS, SEQ_PROC_CI_LOG, nifo_offset(pstMEMSINFO,pstNode) )) < 0 ) {
					log_print(LOGN_CRI, "[%s][%s.%d] gifo_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
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

/** 
 * IPPool에 uiIP가 해당되는 지 검사한다.
 * 성능 테스트시 성능이 나오지 않을경우 IPPOOL BITARRAY로 변경을 생각해야 겠다.
 * uiIP가 0일대의 처리가 들어가 지않아 변경 
 * 
 * @param uiIP 
 * 
 * @return Success = 1, 허용대역 아님 = -1, uiIP 0 = -2
 */

#ifdef _IPPOOL_TYPE_
int dCheck_IPPool(unsigned int uiIP)
{
	int				 i;
	unsigned int		uiFIP, netmask_filter;
	unsigned short		usNet;

	if( uiIP > 0 ) 
	{
		for( i=0; i < stIPPool.uiCount; i++ ) 
		{
			uiFIP = stIPPool.stIPPool[i].uiFIP;
			usNet = stIPPool.stIPPool[i].usNetMask;

			log_print(LOGN_INFO, "[%s][%s.%d]IP: %d.%d.%d.%d COMPARE IPPOOL IP: %d.%d.%d.%d:%u IP POOL TYPE %d", __FILE__, __FUNCTION__, __LINE__, 
					HIPADDR(uiIP), HIPADDR(stIPPool.stIPPool[i].uiFIP), stIPPool.stIPPool[i].usNetMask, stIPPool.dFlag);

			if( usNet == 32 )
				netmask_filter = 0x00000000U;
			else
				netmask_filter = 0xffffffffU >> usNet;

			log_print(LOGN_INFO, "[%s][%s.%d] IP: %d.%d.%d.%d COMPARE IPPOOL IP: %d.%d.%d.%d", __FILE__, __FUNCTION__, __LINE__, 
					 HIPADDR((uiIP | netmask_filter)), HIPADDR((uiFIP | netmask_filter)));

			if((uiIP | netmask_filter) == (uiFIP | netmask_filter)) 
			{
				if(stIPPool.dFlag == 0) { 	/* Filter 가 부정인 경우 필터와 같은 대역은 버린다. */
					return -1;
				} else {
					return 1;
				}
			}
		}
		if(stIPPool.dFlag == 0) { /* Filter가 부정인경우 필터의 대역이 아닌경우 통과	*/
			return 1;
		} else {
			return -1;
		}
	}
	else						/* uiIP가 0인경우 무조건 버린다. */
	{
		log_print(LOGN_CRI, "[%s][%s.%d] EXCEPTION FRAMED IP: %d.%d.%d.%d DROP", __FILE__, __FUNCTION__, __LINE__, HIPADDR(uiIP));
		return -2;
	}
	return 0;
}
#else
int dCheck_IPPool( unsigned int uiIP )
{
	int				 i;
	unsigned int		uiFIP, netmask_filter;
	unsigned short		usNet;

	if( uiIP > 0 ) {

		for( i=0; i<stIPPool.uiCount; i++ ) {
			uiFIP = stIPPool.stIPPool[i].uiFIP;
			usNet = stIPPool.stIPPool[i].usNetMask;

			log_print( LOGN_INFO, "%u %u %u", uiIP, stIPPool.stIPPool[i].uiFIP, stIPPool.stIPPool[i].usNetMask );

			if( usNet == 32 )
				netmask_filter = 0x00000000U;
			else
				netmask_filter = 0xffffffffU >> usNet;

			log_print( LOGN_INFO, "%u %u", (uiIP | netmask_filter), (uiFIP | netmask_filter) );

			if( (uiIP | netmask_filter) == (uiFIP | netmask_filter) )
				break;
		}

		if( i == stIPPool.uiCount )
			return -1;
	}

	return 1;
}
#endif

int dProcRADIUS_Trans( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, st_ACCInfo *pstACCINFO)
{
	int					dSendLogFlag = 0;
	int					dRet;

	HKey_Trans			stTransKey;
	HData_Trans			*pstTransData = NULL;
//	st_RADIUSSIG_LOG	*pstTransLog;
	LOG_SIGNAL			*pstTransLog;

	stHASHONODE			*pstTransHashNode;


	memset(&stTransKey, 0x00, DEF_HKEY_TRANS_SIZE);

	/*	SET TRANS KEY */	
	if (pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
		stTransKey.uiSrcIP 	= pINFOETH->stIP.dwSrcIP;
		stTransKey.uiDestIP	= pINFOETH->stIP.dwDestIP;
	} else {
		stTransKey.uiSrcIP 	= pINFOETH->stIP.dwDestIP;
		stTransKey.uiDestIP	= pINFOETH->stIP.dwSrcIP;
	}
	stTransKey.ucID = pstACCINFO->ucID;
	stTransKey.ucMsgType = ucGetMsgType(pstACCINFO->ucCode);


	switch( pstACCINFO->ucCode ) 
	{
		case RADIUS_ACCOUNT_REQUEST:
				/* 등록된 FramedIP가 아니면 분석하지 않는다. */
				if( (dRet = dCheck_IPPool( pstACCINFO->uiFramedIP )) < 0 ) {
					log_print (LOGN_DEBUG, "### UNDEFINED FRAMEDIP: %d.%d.%d.%d", HIPADDR(pstACCINFO->uiFramedIP));
///					 return -1;				 log 방지
					return 1;
				}


		case RADIUS_ACCESS_REQUEST:
			{
				if( (pstTransHashNode = hasho_find (pstHASHOINFO, (U8 *)&stTransKey)) != NULL ) {
					log_print (LOGN_WARN, "	--> NEW PACKET		IMSI: %s SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d ID: %d MSGTYPE: %d [%s %d:%s",
						pstACCINFO->szMIN, HIPADDR(stTransKey.uiSrcIP), HIPADDR(stTransKey.uiDestIP),
						stTransKey.ucID, stTransKey.ucMsgType, szPrintCode(pstACCINFO->ucCode), pstACCINFO->dAcctStatType,
						szPrintType(pstACCINFO->dAcctStatType));
					log_print (LOGN_WARN, "	--> RETRANS PACKET		SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d ID: %d MSGTYPE: %d",
							HIPADDR(stTransKey.uiSrcIP), HIPADDR(stTransKey.uiDestIP), stTransKey.ucID, stTransKey.ucMsgType);

					return RET_FAIL;
				}
				else {
					log_print (LOGN_DEBUG, "	--> NEW PACKET		IMSI: %s SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d ID: %d MSGTYPE: %d [%s %d:%s",
						pstACCINFO->szMIN, HIPADDR(stTransKey.uiSrcIP), HIPADDR(stTransKey.uiDestIP),
						stTransKey.ucID, stTransKey.ucMsgType, szPrintCode(pstACCINFO->ucCode), pstACCINFO->dAcctStatType,
						szPrintType(pstACCINFO->dAcctStatType));
				}

				if ( (pstTransData = pCreateTransaction(pCAPHEAD, pINFOETH, &stTransKey)) == NULL ) {
					log_print (LOGN_CRI, "[%s][%s.%d]	pCreateTransaction() NULL ", __FILE__, __FUNCTION__, __LINE__);
					return -1;
				}

				pstTransLog = (LOG_SIGNAL *) nifo_ptr (pstMEMSINFO, pstTransData->dOffset);

				/*	COMMON SIGNAL INFO */
				pstTransLog->uiCallTime			= pCAPHEAD->curtime;
				pstTransLog->uiCallMTime		= pCAPHEAD->ucurtime;
				pstTransLog->uiAccStartTime		= pCAPHEAD->curtime;
				pstTransLog->uiAccStartMTime	= pCAPHEAD->ucurtime;
				pstTransLog->uiSessStartTime	= pCAPHEAD->curtime;
				pstTransLog->uiSessStartMTime	= pCAPHEAD->ucurtime;
				pstTransLog->uiSessEndTime		= pCAPHEAD->curtime;
				pstTransLog->uiSessEndMTime		= pCAPHEAD->ucurtime;
				pstTransLog->uiSessDuration		= 0;
				if(pstACCINFO->ucPCFIPF == DEF_FLAG_ON) {
					pstTransLog->uiPCFIP		= pstACCINFO->uiPCFIP;
				}

				/* 2010.03.03 BY LDH FOR ROAMING */
				pstTransLog->ucBranchID = pINFOETH->usSysType;
				pstTransLog->ucEquipType = pINFOETH->usSysType;
				/* 2010.03.03 END */
	
//				pstTransLog->uiNASName 			= pINFOETH->stIP.dwSrcIP;
				pstTransLog->uiNASName 			= 0;
				pstTransLog->uiSrcIP			= pINFOETH->stIP.dwSrcIP;
				pstTransLog->uiDestIP			= pINFOETH->stIP.dwDestIP;

				memcpy(pstTransLog->szIMSI, pstACCINFO->szMIN, MAX_MIN_LEN);
				pstTransLog->szIMSI[MAX_MIN_LEN] = 0x00;
				memcpy(pstTransData->szIMSI, pstACCINFO->szMIN, MAX_MIN_LEN);
				pstTransData->szIMSI[MAX_MIN_LEN] = 0x00;

				/* RADIUS DATA INFO */
				pstTransLog->uiProtoType		= DEF_PROTOCOL_RADIUS; 		/* TODO: define protocal type */
				pstTransLog->uiClientIP			= pstACCINFO->uiFramedIP;
				pstTransLog->uiMsgType 			= pstACCINFO->ucCode;
				pstTransLog->ucAcctType 		= pstACCINFO->dAcctStatType;
				pstTransLog->ulAcctSessionID 	= pstACCINFO->llAcctSessID;
				pstTransLog->ulCorrelationID		= pstACCINFO->llCorrelID; 
				pstTransLog->usInterimTime		= pstACCINFO->dAcctInterim;
				pstTransLog->usTC					= pstACCINFO->dAcctTermCause;
				pstTransLog->uiEventTime			= pstACCINFO->uiEventTime;

				if(pstTransLog->ucAcctType == DEF_MSG_ACCREQ_STOP || pstTransLog->ucAcctType == DEF_MSG_ACCREQ_LINKSTOP) {
					pstTransLog->ucStopFlag		= pstACCINFO->uiSessContinue;
				} else if(pstTransLog->ucAcctType == DEF_MSG_ACCREQ_START || pstTransLog->ucAcctType == DEF_MSG_ACCREQ_LINKSTART) {
					pstTransLog->ucStopFlag		= pstACCINFO->uiBeginnigSess;
				} else {
					pstTransLog->ucStopFlag 	= 0;
				}

				/* 
					DIAL-UP ACCESS CHECK 
				 */
				if(pstACCINFO->ucC23BITF == DEF_FLAG_ON && pstACCINFO->uiC23BIT == 0) {
					log_print (LOGN_INFO, "DIAL-UP INTERNET ACCESS uiC23BIT: %d", pstACCINFO->uiC23BIT);
					pstTransLog->usServiceType = LOG_DIALUP_SESS_DEF_NUM;
				}

				/* 2010.03.03 BY LDH FOR ROAMING */
				memcpy( pstTransLog->szUserName, pstACCINFO->szUserName, MAX_USERNAME_LEN );
				pstTransLog->szUserName[MAX_USERNAME_LEN] = 0x00;
				pstTransLog->uiNasIP 			= pstACCINFO->uiNASIP;
				pstTransLog->usActiveTime 		= pstACCINFO->uiActTime;
				pstTransLog->uiAcctInOctets 	= pstACCINFO->dAcctInOct;
				pstTransLog->uiAcctOutOctets 	= pstACCINFO->dAcctOutOct;
				/* 2010.03.03 END */

				if(pstTransLog->ucAcctType == DEF_MSG_ACCREQ_START || pstTransLog->ucAcctType == DEF_MSG_ACCREQ_LINKSTART) {
					dRet = pSendStartCall(pstTransLog);
					if(dRet<0) {
						log_print (LOGN_CRI, "[%s][%s.%d] pSendStartCall()", __FILE__, __FUNCTION__, __LINE__);
					}
				}

				/* TODO: BranchID 가 TYPE_PDIF 인경우 바로 트랜잭션을 정리 */
			}
			break;
		case RADIUS_ACCESS_REJECT:
		case RADIUS_ACCESS_ACCEPT:
		case RADIUS_ACCOUNT_RESPONSE:

			if( (pstTransHashNode = hasho_find (pstHASHOINFO, (U8 *)&stTransKey)) == NULL ) {
				log_print (LOGN_DEBUG, "	--> DROP PACKET 	SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d ID: %d MSGTYPE: %d", 
						HIPADDR(stTransKey.uiSrcIP), HIPADDR(stTransKey.uiDestIP), stTransKey.ucID, stTransKey.ucMsgType);
/// 				return -2; log 방지
				return 2;
			} else {
				log_print (LOGN_DEBUG, "	--> ANSWER PACKET 	IMSI: %s SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d ID: %d MSGTYPE: %d", 
						pstACCINFO->szMIN, HIPADDR(stTransKey.uiSrcIP), HIPADDR(stTransKey.uiDestIP), stTransKey.ucID, stTransKey.ucMsgType);

				pstTransData = (HData_Trans*) nifo_ptr (pstHASHOINFO, pstTransHashNode->offset_Data);
				pstTransLog = (LOG_SIGNAL *) nifo_ptr (pstMEMSINFO, pstTransData->dOffset);

				pstTransLog->uiSessEndTime		= pCAPHEAD->curtime;
				pstTransLog->uiSessEndMTime		= pCAPHEAD->ucurtime;
				pstTransLog->uiSessDuration		= GetGapTime( pstTransLog->uiSessEndTime, pstTransLog->uiSessEndMTime, 
						pstTransLog->uiSessStartTime, pstTransLog->uiSessStartMTime);

				/* 2010.03.03 BY LDH FOR ROAMING */
				pstTransLog->ucBranchID = pINFOETH->usSysType;
				pstTransLog->ucEquipType = pINFOETH->usSysType;
				/* 2010.03.03 END */

				dSendLogFlag = 1;
			}

			if(pstACCINFO->ucCode == RADIUS_ACCESS_REJECT)
				pstTransLog->uiLastUserErrCode = DEF_ACCESS_REJECT;
			else
				pstTransLog->uiLastUserErrCode = DEF_ACC_SUCCESS;

			break;
		default:
			log_print (LOGN_DEBUG, "	--> UNKNOWN CODE: %d 	SRCIP: %d.%d.%d.%d DESTIP: %d.%d.%d.%d ID: %d MSGTYPE: %d", 
					pstACCINFO->ucCode, HIPADDR(stTransKey.uiSrcIP), HIPADDR(stTransKey.uiDestIP), stTransKey.ucID, stTransKey.ucMsgType);
			return RET_FAIL;
			break;
	}

	/* LASTUSERERRCODE */
	pstTransData->uiLastUserErrCode = pstTransLog->uiLastUserErrCode;

	/* SEND TRACE INFO */
	if (pstTransData) {
		if ( (dRet = dCheck_TraceInfo(pstTransData, pDATA, pCAPHEAD)) < 0 )
			log_print (LOGN_CRI, "FAILED dCheck_TraceInfo dRet:[%d]", dRet);
	}

	/* SEND TRANS LOG */
	if (dSendLogFlag) {
		if ( (dRet=dSendTransLog(pCAPHEAD, &stTransKey, pstTransData)) < 0 ) {
			log_print (LOGN_CRI, "[%s][%s.%d] dSendTransLog dRet:[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
			return RET_FAIL;
		}
	}

	return RET_SUCC;
}
