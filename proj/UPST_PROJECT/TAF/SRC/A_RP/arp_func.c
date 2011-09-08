/**
 * Include headers
 */
#include <unistd.h>

// TOP
#include "procid.h"
#include "common_stg.h"
#include "commdef.h"
#include "capdef.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
#include "utillib.h"
#include "Analyze_Ext_Abs.h"

// TAF headers
#include "mmdb_psess.h"
#include "arp_head.h"

// .
#include "arp_func.h"

/**
 * Declare variables
 */
stMEMSINFO      *pMEMSINFO;
stCIFO			*gpCIFO;
st_TraceList    *pstTRACE;

extern UINT		guiSeqProcID;
extern int		gAGRECnt;
extern S32 		gSemID;

/**
 *	Implement func.
 */

/*
	SEMA HASH
*/
stHASHONODE *hashs_find(stHASHOINFO *pstHASHOINFO, U8 *pKey)
{
	stHASHONODE		*pstHASHONODE;

	P(gSemID, MEMS_SEMA_ON);
	pstHASHONODE = hasho_find(pstHASHOINFO, pKey);
	V(gSemID, MEMS_SEMA_ON);

	return pstHASHONODE;
}

stHASHONODE *hashs_add(stHASHOINFO *pstHASHOINFO, U8 *pKey, U8 *pData)
{
	stHASHONODE		*pstHASHONODE;

	P(gSemID, MEMS_SEMA_ON);
	pstHASHONODE = hasho_add(pstHASHOINFO, pKey, pData);
	V(gSemID, MEMS_SEMA_ON);

	return pstHASHONODE;
}

void hashs_del(stHASHOINFO *pstHASHOINFO, U8 *pKey)
{
	P(gSemID, MEMS_SEMA_ON);
	hasho_del(pstHASHOINFO, pKey);
	V(gSemID, MEMS_SEMA_ON);
}

/*******************************************************************************

*******************************************************************************/
int dCheck_TraceInfo( PSESS_DATA *pstSessData, unsigned char *pData, Capture_Header_Msg *pstCAP )
{
	int		i, dRet;
	UCHAR	*pstNode;
	UCHAR	*pBuffer;

	st_TraceMsgHdr	*pstTrcMsg;

	/*
	* pstSessData->szMIN 		: 4500010~
	* pstSessData->szTraceMIN 	: 010~
	*/

	for( i=0; i< pstTRACE->count; i++ ) {
		if( pstTRACE->stTraceInfo[i].dType == TRC_TYPE_IMSI ) {

			if( atoll(pstSessData->szMIN) == pstTRACE->stTraceInfo[i].stTraceID.llIMSI ) {

				log_print( LOGN_INFO, "TRACE IMSI[%d] %s, %s", i, pstSessData->szMIN, pstTRACE->stTraceInfo[i].stTraceID.szMIN );
				/* SEND TRACE PACKET */
				if( (pstNode = nifo_node_alloc( pMEMSINFO )) == NULL ) {
					log_print( LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
					return -1;
				}

				if( (pstTrcMsg = (st_TraceMsgHdr *)nifo_tlv_alloc( pMEMSINFO, pstNode, st_TraceMsgHdr_DEF_NUM, st_TraceMsgHdr_SIZE, DEF_MEMSET_OFF)) == NULL ) {
					log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, st_TraceMsgHdr_DEF_NUM );
					return -2;
				}

				if( (pBuffer = nifo_tlv_alloc( pMEMSINFO, pstNode, ETH_DATA_NUM, pstCAP->datalen, DEF_MEMSET_OFF)) == NULL ) {
					log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, ETH_DATA_NUM);
					return -3;
				}

				pstTrcMsg->time 		= pstCAP->curtime;
				pstTrcMsg->mtime		= pstCAP->ucurtime;
				pstTrcMsg->dType		= pstTRACE->stTraceInfo[i].dType;
				pstTrcMsg->usDataLen	= pstCAP->datalen;
				memcpy( &pstTrcMsg->stTraceID, &pstTRACE->stTraceInfo[i].stTraceID, st_TraceID_SIZE );
				memcpy( pBuffer, pData, pstCAP->datalen );

				dRet = gifo_write( pMEMSINFO, gpCIFO, guiSeqProcID, SEQ_PROC_CI_LOG, nifo_offset(pMEMSINFO, pstNode) );
				if( dRet < 0 ) {
					log_print(LOGN_CRI, "[%s][%s.%d] gifo_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
					return -4;
				}

				return 1;
			}
		}
		else if( pstTRACE->stTraceInfo[i].dType == TRC_TYPE_MDN ) {

			if( atoll(pstSessData->szTraceMIN) == pstTRACE->stTraceInfo[i].stTraceID.llIMSI ) {

				log_print( LOGN_INFO, "TRACE MIN[%d] %s, %s", i, pstSessData->szTraceMIN, pstTRACE->stTraceInfo[i].stTraceID.szMIN );
				/* SEND TRACE PACKET */
				if( (pstNode = nifo_node_alloc( pMEMSINFO )) == NULL ) {
                    log_print( LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
                    return -1;
                }               

                if( (pstTrcMsg = (st_TraceMsgHdr *)nifo_tlv_alloc( pMEMSINFO, pstNode, st_TraceMsgHdr_DEF_NUM, st_TraceMsgHdr_SIZE, DEF_MEMSET_OFF)) == NULL ) {
                    log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, st_TraceMsgHdr_DEF_NUM );
                    return -2;
                }           
                            
                if( (pBuffer = nifo_tlv_alloc( pMEMSINFO, pstNode, ETH_DATA_NUM, pstCAP->datalen, DEF_MEMSET_OFF)) == NULL ) {
                    log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, ETH_DATA_NUM);
                    return -3;
                }

				pstTrcMsg->time         = pstCAP->curtime;
                pstTrcMsg->mtime        = pstCAP->ucurtime;
                pstTrcMsg->dType        = pstTRACE->stTraceInfo[i].dType;
                pstTrcMsg->usDataLen    = pstCAP->datalen;
                memcpy( &pstTrcMsg->stTraceID, &pstTRACE->stTraceInfo[i].stTraceID, st_TraceID_SIZE );
                memcpy( pBuffer, pData, pstCAP->datalen );

				dRet = gifo_write( pMEMSINFO, gpCIFO, guiSeqProcID, SEQ_PROC_CI_LOG, nifo_offset(pMEMSINFO, pstNode) );
				if( dRet < 0 ){
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


/*******************************************************************************

*******************************************************************************/
unsigned int GetUpTime( time_t StartTime, int MStartTime, time_t EndTime, int MEndTime )
{
    unsigned int uiUpTime = 0;

    if( StartTime == 0 || EndTime == 0 || StartTime > EndTime || (StartTime == EndTime && MStartTime > MEndTime) )
        return 0;

    if( MStartTime > MEndTime ) {
        uiUpTime = (EndTime - StartTime - 1) * 1000000;
        uiUpTime += (MEndTime + 1000000 - MStartTime);
    }
    else {
        uiUpTime = (EndTime - StartTime ) * 1000000;
        uiUpTime += (MEndTime - MStartTime);
    }

    return uiUpTime;
}


/*******************************************************************************

*******************************************************************************/
int Report_SIGLog( UCHAR ucProto, UCHAR ucMsgType, PSESS_DATA *pPSessData )
{
	int				dRet;
	UCHAR			*pstNode;
	UCHAR			ucProtoType;
	LOG_SIGNAL		*pstSIGLog;

    UINT    		uiDuration;
    //st_SIGNAL_Log   *pstSIGLog;

	if( ucProto == TIMER_STOP_CALL_NUM ) {
		if(pPSessData->uiCallType >= DEF_CALL_RECALL)
			ucProtoType = STOP_RP_RECALL_NUM;
		else
			ucProtoType = STOP_CALL_NUM;
	}
	else
		ucProtoType = ucProto;

	if( (pstNode = nifo_node_alloc(pMEMSINFO)) == NULL ) {
		log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
		return -1;
	}


#if 1 /* INYOUNG */

	switch(ucProtoType) {
		case START_CALL_NUM:

			if( (pstSIGLog = 
				(LOG_SIGNAL *)nifo_tlv_alloc(pMEMSINFO, pstNode, START_CALL_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_OFF)) == NULL )
			{
				log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, START_CALL_NUM );
				nifo_node_delete(pMEMSINFO, pstNode);
				return -2;
			}
			break;

		case STOP_CALL_NUM:

			if( (pstSIGLog = 
				(LOG_SIGNAL *)nifo_tlv_alloc(pMEMSINFO, pstNode, STOP_CALL_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_OFF)) == NULL )
			{
				log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, STOP_CALL_NUM);
				nifo_node_delete(pMEMSINFO, pstNode);
				return -3; 
        	} 
			break;

		case START_RP_SIG_RECALL_NUM:

			if( (pstSIGLog = (LOG_SIGNAL *)nifo_tlv_alloc(pMEMSINFO, pstNode, START_RP_SIG_RECALL_NUM
															, LOG_SIGNAL_SIZE, DEF_MEMSET_OFF)) == NULL)
			{
				log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, START_RP_SIG_RECALL_NUM);
				nifo_node_delete(pMEMSINFO, pstNode);
				return -3; 
        	} 
			break;

		case STOP_RP_RECALL_NUM:

			if( (pstSIGLog = (LOG_SIGNAL *)nifo_tlv_alloc(pMEMSINFO, pstNode, STOP_RP_RECALL_NUM 
															, LOG_SIGNAL_SIZE, DEF_MEMSET_OFF)) == NULL)
			{
				log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, STOP_RP_RECALL_NUM);
				nifo_node_delete(pMEMSINFO, pstNode);
				return -3; 
        	} 
			break;

		default:

			if( (pstSIGLog = (LOG_SIGNAL *)nifo_tlv_alloc(pMEMSINFO, pstNode, LOG_SIGNAL_DEF_NUM
															, LOG_SIGNAL_SIZE, DEF_MEMSET_OFF)) == NULL )
			{
				log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, LOG_SIGNAL_DEF_NUM);
				nifo_node_delete(pMEMSINFO, pstNode);
				return -4;
			}

			break;
	}

#endif /* INYOUNG */

	memset( pstSIGLog, 0x00, LOG_SIGNAL_SIZE );

	/* SET COMMON INFO */
    pstSIGLog->uiPCFIP      = util_cvtuint(pPSessData->key.uiServingPCF);
    pstSIGLog->uiGREKey     = pPSessData->key.uiKey;
    pstSIGLog->uiClientIP   = pPSessData->uiIPAddr;
	pstSIGLog->uiNASName	= util_cvtuint(pPSessData->uiNASIP);


    pstSIGLog->ucSYSID      = (pPSessData->stBSMSC.SYS_ID);
    pstSIGLog->ucBSCID      = (pPSessData->stBSMSC.BSC_ID);
    pstSIGLog->ucBTSID      = util_cvtushort(pPSessData->stBSMSC.BTS_ID);
    pstSIGLog->ucFA_ID      = (pPSessData->stBSMSC.FA_ID);
    pstSIGLog->ucSECTOR     = (pPSessData->stBSMSC.SEC_ID);

	memcpy(&pstSIGLog->szBSMSC[0], &pPSessData->ucBSMSC[4], 8 );
    memcpy(&pstSIGLog->szBSMSC[8], &pPSessData->ucBSMSC[0], 4 );
	pstSIGLog->szBSMSC[DEF_BSMSD_LENGTH-1] = 0x00;

    pstSIGLog->uiSvcOption  = (pPSessData->uiSvcOption);
    memcpy( pstSIGLog->szNetOption, pPSessData->szNetOption, MAX_SVCOPTION_SIZE );

	switch( ucProtoType )
	{
		case START_CALL_NUM:
		case STOP_CALL_NUM:

#if 1 /* INYOUNG */
		case START_RP_SIG_RECALL_NUM:
		case STOP_RP_RECALL_NUM:
#endif
			pstSIGLog->uiCallTime       = pPSessData->CreateTime;
            pstSIGLog->uiCallMTime      = pPSessData->CreateMTime;
            memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
            memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );

			pstSIGLog->uiProtoType      = ucProtoType;
            pstSIGLog->uiMsgType        = pPSessData->RegA11.ucReqMsgCode;

			pstSIGLog->uiSessStartTime  = (pPSessData->RegA11.StartTime);
            pstSIGLog->uiSessStartMTime = (pPSessData->RegA11.StartMTime);

			/* INYOUNG
			 * STOP_RP_RECALL_NUM 에 대한 조건 추가
			 */
			if( (ucProto == TIMER_STOP_CALL_NUM ) || (ucProtoType == STOP_CALL_NUM) || (ucProtoType == STOP_RP_RECALL_NUM))
			{
                if( pPSessData->LastStopTime == 0 && pPSessData->LastStopMTime == 0 ) {
                    pstSIGLog->uiSessEndTime  = pPSessData->LastUpdateTime;
                    pstSIGLog->uiSessEndMTime = pPSessData->LastUpdateMTime;
                }
                else {
                    pstSIGLog->uiSessEndTime  = pPSessData->LastStopTime;
                    pstSIGLog->uiSessEndMTime = pPSessData->LastStopMTime;
                }
            }
            else {
                pstSIGLog->uiSessEndTime    = (pPSessData->RegA11.EndTime);
                pstSIGLog->uiSessEndMTime   = (pPSessData->RegA11.EndMTime);
            }

			pstSIGLog->uiSrcIP 	= util_cvtuint(pPSessData->RegA11.uiSrcIP);
            pstSIGLog->uiDestIP = util_cvtuint(pPSessData->RegA11.uiDestIP);

			if( (pPSessData->uiStopFlag == 0) && (ucProto == TIMER_STOP_CALL_NUM) )
                pstSIGLog->uiRespCode   = TIMER_STOP_CALL_NUM;
			else {
				/* ##### CHECK ###### */
                pstSIGLog->uiRespCode   = pPSessData->RegA11.ucRepMsgCode;
			}

			pstSIGLog->uiUpGREPkts  = pPSessData->uiUpGREFrames;
            pstSIGLog->uiDnGREPkts  = pPSessData->uiDownGREFrames;
            pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
            pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;

			break;

		case NUM_UDP_A11:
			pstSIGLog->uiCallTime		= pPSessData->CreateTime;
			pstSIGLog->uiCallMTime		= pPSessData->CreateMTime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
			memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );

			pstSIGLog->uiProtoType		= ucProtoType;

			switch( ucMsgType ) {
				case DEF_A11_REG_REQ:
					pstSIGLog->uiMsgType		= pPSessData->RegA11.ucReqMsgCode;
					pstSIGLog->ucErrorCode		= pPSessData->RegA11.ucRegiReply;

					pstSIGLog->uiSessStartTime  = (pPSessData->RegA11.StartTime);
            		pstSIGLog->uiSessStartMTime = (pPSessData->RegA11.StartMTime);

					pstSIGLog->uiSessEndTime    = (pPSessData->RegA11.EndTime);
            		pstSIGLog->uiSessEndMTime   = (pPSessData->RegA11.EndMTime);

					pstSIGLog->uiRespCode       = pPSessData->RegA11.ucRepMsgCode;

            		uiDuration = GetUpTime( pPSessData->RegA11.StartTime, pPSessData->RegA11.StartMTime,
                                        	pPSessData->RegA11.EndTime, pPSessData->RegA11.EndMTime );
					pstSIGLog->uiSrcIP = util_cvtuint(pPSessData->RegA11.uiSrcIP);
            		pstSIGLog->uiDestIP = util_cvtuint(pPSessData->RegA11.uiDestIP);
					break;

				case DEF_A11_REG_UP:
					pstSIGLog->uiMsgType        = pPSessData->UpA11.ucReqMsgCode;
                    pstSIGLog->ucErrorCode      = pPSessData->UpA11.ucRegiReply;

                    pstSIGLog->uiSessStartTime  = (pPSessData->UpA11.StartTime);
                    pstSIGLog->uiSessStartMTime = (pPSessData->UpA11.StartMTime);

                    pstSIGLog->uiSessEndTime    = (pPSessData->UpA11.EndTime);
                    pstSIGLog->uiSessEndMTime   = (pPSessData->UpA11.EndMTime);

					pstSIGLog->uiRespCode       = pPSessData->UpA11.ucRepMsgCode;

                    uiDuration = GetUpTime( pPSessData->UpA11.StartTime, pPSessData->UpA11.StartMTime,
                                            pPSessData->UpA11.EndTime, pPSessData->UpA11.EndMTime );
                    pstSIGLog->uiSrcIP = util_cvtuint(pPSessData->UpA11.uiSrcIP);
                    pstSIGLog->uiDestIP = util_cvtuint(pPSessData->UpA11.uiDestIP);
					break;
				case DEF_A11_SESS_UP:
					pstSIGLog->uiMsgType        = pPSessData->SessA11.ucReqMsgCode;
                    pstSIGLog->ucErrorCode      = pPSessData->SessA11.ucRegiReply;

                    pstSIGLog->uiSessStartTime  = (pPSessData->SessA11.StartTime);
                    pstSIGLog->uiSessStartMTime = (pPSessData->SessA11.StartMTime);

                    pstSIGLog->uiSessEndTime    = (pPSessData->SessA11.EndTime);
                    pstSIGLog->uiSessEndMTime   = (pPSessData->SessA11.EndMTime);

                    pstSIGLog->uiRespCode       = pPSessData->SessA11.ucRepMsgCode;

                    uiDuration = GetUpTime( pPSessData->SessA11.StartTime, pPSessData->SessA11.StartMTime,
                                            pPSessData->SessA11.EndTime, pPSessData->SessA11.EndMTime );
                    pstSIGLog->uiSrcIP = util_cvtuint(pPSessData->SessA11.uiSrcIP);
                    pstSIGLog->uiDestIP = util_cvtuint(pPSessData->SessA11.uiDestIP);
					break;
			
				default:
					log_print( LOGN_CRI, "[%s.%d] INVALID MSGTYPE:%u", __FUNCTION__, __LINE__, ucMsgType ); 
					break;
			}


			pstSIGLog->uiSessDuration 	= (uiDuration);


			pstSIGLog->uiUpGREPkts	= pPSessData->uiUpGREFrames;
			pstSIGLog->uiDnGREPkts	= pPSessData->uiDownGREFrames;
			pstSIGLog->uiUpGREBytes	= pPSessData->uiUpGREBytes;
			pstSIGLog->uiDnGREBytes	= pPSessData->uiDownGREBytes;

			if( ucMsgType == 0x01 ) /* regi-request */
			{
				pstSIGLog->usLiftTime	= (pPSessData->RegA11.usLifetime);
				pstSIGLog->ucAppType	= pPSessData->ucAppType;
				if( pPSessData->ucAppType != 0 )
				{
					pstSIGLog->uiFMux 		= (pPSessData->uiFMux);
					pstSIGLog->uiRMux 		= (pPSessData->uiRMux);
					pstSIGLog->ucAirLink 	= pPSessData->RegA11.ucAirlink;

					if( pPSessData->RegA11.ucAirlink == 0x04 ) {
						pPSessData->LastStopTime 	= pPSessData->RegA11.EndTime;
						pPSessData->LastStopMTime 	= pPSessData->RegA11.EndMTime;
					}
					else {
						pPSessData->LastStopTime    = 0;
						pPSessData->LastStopMTime   = 0;
					}
				}
			}
			else if( ucMsgType == 0x14 ) /* regi-update */
			{
				pstSIGLog->usUpdateReason	= (pPSessData->UpA11.uiUpdateReason);
			}

			break;

		case NUM_UP_LCP :
			pstSIGLog->uiCallTime       = pPSessData->CreateTime;
			pstSIGLog->uiCallMTime      = pPSessData->CreateMTime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
			memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );

			pstSIGLog->uiUpGREPkts  = pPSessData->uiUpGREFrames;
            pstSIGLog->uiDnGREPkts  = pPSessData->uiDownGREFrames;
            pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
            pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;

			pstSIGLog->uiProtoType      = ucProtoType;
			if( ucMsgType == DEF_LCP_CONF_REQ ) /* configure-req */
			{
				pstSIGLog->uiMsgType		= pPSessData->UpLCP.ucReqMsgCode;
				pstSIGLog->uiSessStartTime  = (pPSessData->UpLCP.StartTime);
				pstSIGLog->uiSessStartMTime = (pPSessData->UpLCP.StartMTime);
				pstSIGLog->uiSessEndTime    = (pPSessData->UpLCP.EndTime);
				pstSIGLog->uiSessEndMTime   = (pPSessData->UpLCP.EndMTime);
				uiDuration = GetUpTime( pPSessData->UpLCP.StartTime, pPSessData->UpLCP.StartMTime,
										pPSessData->UpLCP.EndTime, pPSessData->UpLCP.EndMTime );

				pstSIGLog->uiRespCode		= pPSessData->UpLCP.ucRepMsgCode;
				pstSIGLog->ucPPPReqCnt 		= (pPSessData->UpLCP.ucReqCount);
				pstSIGLog->ucPPPNakCnt 		= (pPSessData->UpLCP.ucNakCount);
				pstSIGLog->ucPPPRejCnt 		= (pPSessData->UpLCP.ucRejCount);
			}
			else if( ucMsgType == DEF_LCP_TERM_REQ ) /* term */
			{
				pstSIGLog->uiMsgType 		= pPSessData->LCPTerm.ucReqMsgCode;
				pstSIGLog->uiSessStartTime 	= (pPSessData->LCPTerm.StartTime);
				pstSIGLog->uiSessStartMTime = (pPSessData->LCPTerm.StartMTime);
				pstSIGLog->uiSessEndTime 	= (pPSessData->LCPTerm.EndTime);
				pstSIGLog->uiSessEndMTime 	= (pPSessData->LCPTerm.EndMTime);

				uiDuration = GetUpTime( pPSessData->LCPTerm.StartTime, pPSessData->LCPTerm.StartMTime,
										pPSessData->LCPTerm.EndTime, pPSessData->LCPTerm.EndMTime );
				pstSIGLog->uiRespCode		= pPSessData->LCPTerm.ucRepMsgCode;
				pstSIGLog->ucPPPReqCnt 		= (pPSessData->LCPTerm.ucReqCount);
				pstSIGLog->ucPPPNakCnt 		= (pPSessData->LCPTerm.ucNakCount);
				pstSIGLog->ucPPPRejCnt 		= (pPSessData->LCPTerm.ucRejCount);
			}
			else if( ucMsgType == DEF_LCP_ECHO_REQ )
			{
				pstSIGLog->uiMsgType        = pPSessData->UpLCPEcho.ucReqMsgCode;
                pstSIGLog->uiSessStartTime  = (pPSessData->UpLCPEcho.StartTime);
                pstSIGLog->uiSessStartMTime = (pPSessData->UpLCPEcho.StartMTime);
                pstSIGLog->uiSessEndTime    = (pPSessData->UpLCPEcho.EndTime);
                pstSIGLog->uiSessEndMTime   = (pPSessData->UpLCPEcho.EndMTime);
                uiDuration = GetUpTime( pPSessData->UpLCPEcho.StartTime, pPSessData->UpLCPEcho.StartMTime,
                                        pPSessData->UpLCPEcho.EndTime, pPSessData->UpLCPEcho.EndMTime );

				/*
                pstSIGLog->uiRespCode       = pPSessData->UpLCPEcho.ucRepMsgCode;
                pstSIGLog->ucPPPReqCnt      = (pPSessData->UpLCPEcho.ucReqCount);
                pstSIGLog->ucPPPNakCnt      = (pPSessData->UpLCPEcho.ucNakCount);
                pstSIGLog->ucPPPRejCnt      = (pPSessData->UpLCPEcho.ucRejCount);
				*/
			}
			else
			{
				log_print( LOGN_WARN, "Unknown ProtoType[%u], MsgType[%u]", ucProtoType, ucMsgType );
				nifo_node_delete(pMEMSINFO, pstNode);
				return -1;
			}

			pstSIGLog->uiSessDuration 	= (uiDuration);

			pstSIGLog->uiSrcIP 			= util_cvtuint(pPSessData->key.uiServingPCF);
			pstSIGLog->uiDestIP 		= (pPSessData->uiHomeAgent);

			break;

		case NUM_DOWN_LCP :
			pstSIGLog->uiCallTime       = pPSessData->CreateTime;
			pstSIGLog->uiCallMTime      = pPSessData->CreateMTime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
			memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
			pstSIGLog->uiProtoType		= ucProtoType;

			pstSIGLog->uiUpGREPkts  = pPSessData->uiUpGREFrames;
            pstSIGLog->uiDnGREPkts  = pPSessData->uiDownGREFrames;
            pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
            pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;

			if( ucMsgType == DEF_LCP_CONF_REQ ) /* configure-request */
			{
				pstSIGLog->uiMsgType 		= pPSessData->DownLCP.ucReqMsgCode;
				pstSIGLog->uiSessStartTime 	= (pPSessData->DownLCP.StartTime);
				pstSIGLog->uiSessStartMTime = (pPSessData->DownLCP.StartMTime);
				pstSIGLog->uiSessEndTime 	= (pPSessData->DownLCP.EndTime);
				pstSIGLog->uiSessEndMTime 	= (pPSessData->DownLCP.EndMTime);
				uiDuration = GetUpTime( pPSessData->DownLCP.StartTime, pPSessData->DownLCP.StartMTime,
										pPSessData->DownLCP.EndTime, pPSessData->DownLCP.EndMTime );
				pstSIGLog->uiRespCode		= pPSessData->DownLCP.ucRepMsgCode;
				pstSIGLog->ucPPPReqCnt 		= (pPSessData->DownLCP.ucReqCount);
				pstSIGLog->ucPPPNakCnt 		= (pPSessData->DownLCP.ucNakCount);
				pstSIGLog->ucPPPRejCnt 		= (pPSessData->DownLCP.ucRejCount);
			}
			else if( ucMsgType == DEF_LCP_TERM_REQ ) /* term */
			{
				pstSIGLog->uiMsgType 		= pPSessData->LCPTerm.ucReqMsgCode;
				pstSIGLog->uiSessStartTime 	= (pPSessData->LCPTerm.StartTime);
				pstSIGLog->uiSessStartMTime = (pPSessData->LCPTerm.StartMTime);
				pstSIGLog->uiSessEndTime 	= (pPSessData->LCPTerm.EndTime);
				pstSIGLog->uiSessEndMTime 	= (pPSessData->LCPTerm.EndMTime);
				uiDuration = GetUpTime( pPSessData->LCPTerm.StartTime, pPSessData->LCPTerm.StartMTime,
										pPSessData->LCPTerm.EndTime, pPSessData->LCPTerm.EndMTime );
				pstSIGLog->uiRespCode		= pPSessData->LCPTerm.ucRepMsgCode;
				pstSIGLog->ucPPPReqCnt 		= (pPSessData->LCPTerm.ucReqCount);
				pstSIGLog->ucPPPNakCnt 		= (pPSessData->LCPTerm.ucNakCount);
				pstSIGLog->ucPPPRejCnt 		= (pPSessData->LCPTerm.ucRejCount);
			}
			else if( ucMsgType == DEF_LCP_ECHO_REQ )
            {
				pstSIGLog->uiMsgType        = pPSessData->DownLCPEcho.ucReqMsgCode;
                pstSIGLog->uiSessStartTime  = (pPSessData->DownLCPEcho.StartTime);
                pstSIGLog->uiSessStartMTime = (pPSessData->DownLCPEcho.StartMTime);
                pstSIGLog->uiSessEndTime    = (pPSessData->DownLCPEcho.EndTime);
                pstSIGLog->uiSessEndMTime   = (pPSessData->DownLCPEcho.EndMTime);
                uiDuration = GetUpTime( pPSessData->DownLCPEcho.StartTime, pPSessData->DownLCPEcho.StartMTime,
                                        pPSessData->DownLCPEcho.EndTime, pPSessData->DownLCPEcho.EndMTime );

				/*
                pstSIGLog->uiRespCode       = pPSessData->DownLCP.ucRepMsgCode;
                pstSIGLog->ucPPPReqCnt      = (pPSessData->DownLCP.ucReqCount);
                pstSIGLog->ucPPPNakCnt      = (pPSessData->DownLCP.ucNakCount);
                pstSIGLog->ucPPPRejCnt      = (pPSessData->DownLCP.ucRejCount);
				*/
            }
			else
			{
				log_print( LOGN_WARN, "Unknown ProtoType[%u], MsgType[%u]", ucProtoType, ucMsgType );
				nifo_node_delete(pMEMSINFO, pstNode);
				return -1;
			}
			pstSIGLog->uiSessDuration 	= (uiDuration);

			pstSIGLog->uiSrcIP 			= (pPSessData->uiHomeAgent);
			pstSIGLog->uiDestIP 		= util_cvtuint(pPSessData->key.uiServingPCF);

			break;

		case NUM_UP_IPCP :
			pstSIGLog->uiCallTime       = pPSessData->CreateTime;
			pstSIGLog->uiCallMTime      = pPSessData->CreateMTime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
            memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
			pstSIGLog->uiProtoType		= ucProtoType;
			pstSIGLog->uiMsgType 		= pPSessData->UpIPCP.ucReqMsgCode;

			pstSIGLog->uiUpGREPkts  = pPSessData->uiUpGREFrames;
            pstSIGLog->uiDnGREPkts  = pPSessData->uiDownGREFrames;
            pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
            pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;

			pstSIGLog->uiSessStartTime 	= (pPSessData->UpIPCP.StartTime);
			pstSIGLog->uiSessStartMTime = (pPSessData->UpIPCP.StartMTime);
			pstSIGLog->uiSessEndTime 	= (pPSessData->UpIPCP.EndTime);
			pstSIGLog->uiSessEndMTime 	= (pPSessData->UpIPCP.EndMTime);
			uiDuration = GetUpTime( pPSessData->UpIPCP.StartTime, pPSessData->UpIPCP.StartMTime,
										pPSessData->UpIPCP.EndTime, pPSessData->UpIPCP.EndMTime );
			pstSIGLog->uiSessDuration 	= (uiDuration);

			pstSIGLog->uiSrcIP = util_cvtuint(pPSessData->key.uiServingPCF);
			pstSIGLog->uiDestIP = (pPSessData->uiHomeAgent);

			pstSIGLog->uiRespCode		= pPSessData->UpIPCP.ucRepMsgCode;
			pstSIGLog->ucPPPReqCnt 		= (pPSessData->UpIPCP.ucReqCount);
			pstSIGLog->ucPPPNakCnt 		= (pPSessData->UpIPCP.ucNakCount);
			pstSIGLog->ucPPPRejCnt 		= (pPSessData->UpIPCP.ucRejCount);
			pstSIGLog->uiClientIP		= (pPSessData->uiIPAddr);

			break;

		case NUM_DOWN_IPCP :
			pstSIGLog->uiCallTime       = pPSessData->CreateTime;
			pstSIGLog->uiCallMTime      = pPSessData->CreateMTime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
			memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
			pstSIGLog->uiProtoType 		= ucProtoType;
			pstSIGLog->uiMsgType 		= pPSessData->DownIPCP.ucReqMsgCode;

			pstSIGLog->uiUpGREPkts  = pPSessData->uiUpGREFrames;
            pstSIGLog->uiDnGREPkts  = pPSessData->uiDownGREFrames;
            pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
            pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;

			pstSIGLog->uiSessStartTime 	= (pPSessData->DownIPCP.StartTime);
			pstSIGLog->uiSessStartMTime = (pPSessData->DownIPCP.StartMTime);
			pstSIGLog->uiSessEndTime 	= (pPSessData->DownIPCP.EndTime);
			pstSIGLog->uiSessEndMTime 	= (pPSessData->DownIPCP.EndMTime);
			uiDuration = GetUpTime( pPSessData->DownIPCP.StartTime, pPSessData->DownIPCP.StartMTime,
										pPSessData->DownIPCP.EndTime, pPSessData->DownIPCP.EndMTime );
			pstSIGLog->uiSessDuration 	= (uiDuration);

			pstSIGLog->uiSrcIP 			= (pPSessData->uiHomeAgent);
			pstSIGLog->uiDestIP 		= util_cvtuint(pPSessData->key.uiServingPCF);

			pstSIGLog->uiRespCode		= pPSessData->DownIPCP.ucRepMsgCode;
			pstSIGLog->ucPPPReqCnt 		= (pPSessData->DownIPCP.ucReqCount);
			pstSIGLog->ucPPPNakCnt 		= (pPSessData->DownIPCP.ucNakCount);
			pstSIGLog->ucPPPRejCnt 		= (pPSessData->DownIPCP.ucRejCount);
			pstSIGLog->uiClientIP		= (pPSessData->uiIPAddr);

			break;

		case NUM_CHAP :
		case NUM_PAP :
			pstSIGLog->uiCallTime       = pPSessData->CreateTime;
			pstSIGLog->uiCallMTime      = pPSessData->CreateMTime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
			memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
			pstSIGLog->uiProtoType 		= ucProtoType;
			pstSIGLog->uiMsgType 		= pPSessData->DownIPCP.ucReqMsgCode;

			pstSIGLog->uiUpGREPkts  = pPSessData->uiUpGREFrames;
            pstSIGLog->uiDnGREPkts  = pPSessData->uiDownGREFrames;
            pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
            pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;

			pstSIGLog->uiSessStartTime 	= (pPSessData->CHAPAP.StartTime);
			pstSIGLog->uiSessStartMTime = (pPSessData->CHAPAP.StartMTime);
			pstSIGLog->uiSessEndTime 	= (pPSessData->CHAPAP.EndTime);
			pstSIGLog->uiSessEndMTime 	= (pPSessData->CHAPAP.EndMTime);
			pstSIGLog->uiPPPResponseTime = (pPSessData->CHAPResTime);
			pstSIGLog->uiPPPResponseMTime = (pPSessData->CHAPResMTime);

			uiDuration = GetUpTime( pPSessData->CHAPAP.StartTime, pPSessData->CHAPAP.StartMTime,
										pPSessData->CHAPAP.EndTime, pPSessData->CHAPAP.EndMTime );
			pstSIGLog->uiSessDuration 	= (uiDuration);

			pstSIGLog->uiSrcIP 			= (pPSessData->uiHomeAgent);
			pstSIGLog->uiDestIP 		= util_cvtuint(pPSessData->key.uiServingPCF);

			pstSIGLog->uiRespCode		= pPSessData->CHAPAP.ucRepMsgCode;
			pstSIGLog->ucPPPReqCnt 		= (pPSessData->CHAPAP.ucReqCount);
			pstSIGLog->ucPPPNakCnt 		= (pPSessData->CHAPAP.ucNakCount);
			pstSIGLog->ucPPPRejCnt 		= (pPSessData->CHAPAP.ucRejCount);

			sprintf(pstSIGLog->szAuthUserName, "%s", pPSessData->szAuthUserName);
			//sprintf(pstSIGLog->szCHAPFailMsg, "%s", pPSessData->szCHAPFailMsg);

			pstSIGLog->uiPPPResponseTime	= (pPSessData->CHAPResTime);
			pstSIGLog->uiPPPResponseMTime	= (pPSessData->CHAPResMTime);

			break;

		default :
			log_print( LOGN_WARN, "Unknown ProtoType[%u]", ucProtoType );
			nifo_node_delete(pMEMSINFO, pstNode);
			return -1;
	}

	//LOG_SIGNAL_Prt( "Report_SIGLog", pstSIGLog ); 
	log_print( LOGN_DEBUG, "    LOG_SIG[%3u] I:%s CT:%10u.%06u CIP:%u PF:%u PD:%u M:%3u S:%10u.%06u E:%10u.%06u A:%u R:0X%02X PPP:0X%04X",
                        ucProtoType, pstSIGLog->szIMSI, pstSIGLog->uiCallTime, pstSIGLog->uiCallMTime, pstSIGLog->uiClientIP,
                        pstSIGLog->uiPCFIP, pstSIGLog->uiNASName, pstSIGLog->uiMsgType, pstSIGLog->uiSessStartTime, pstSIGLog->uiSessStartMTime,
                        pstSIGLog->uiSessEndTime, pstSIGLog->uiSessEndMTime, pstSIGLog->ucAirLink, pstSIGLog->ucErrorCode, pPSessData->usPPPFlag );

	if( (dRet = gifo_write( pMEMSINFO, gpCIFO, guiSeqProcID, SEQ_PROC_A_CALL, nifo_offset(pMEMSINFO, pstNode) )) < 0 ) {
		log_print(LOGN_CRI, "[%s][%s.%d] gifo_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
		nifo_node_delete(pMEMSINFO, pstNode);
		return -4;
	}
	else
		log_print( LOGN_INFO, "SUCCSESS SEND LOG:%d A_CALL_SeqProcID:%d", ucProtoType, SEQ_PROC_A_CALL ); 

    return 0;
}


/*******************************************************************************

*******************************************************************************/
S32 dGetGREProcID( UINT uiClientIP )
{
	return SEQ_PROC_A_GRE + ( uiClientIP % gAGRECnt );
}


/*******************************************************************************

*******************************************************************************/
void Print_INFO_A11( INFO_A11 *pINFOA11 )
{
	UCHAR	ucLog = LOGN_INFO;

	log_print( ucLog, "bA11:%u", pINFOA11->bA11 );
	log_print( ucLog, "ucMsg:%u", pINFOA11->ucMsg );
	log_print( ucLog, "ucCode:%u", pINFOA11->ucCode );
	log_print( ucLog, "usLifetime:%u", pINFOA11->usLifetime );
	log_print( ucLog, "dwKey:%u", pINFOA11->dwKey );
	log_print( ucLog, "dwServiceOption:%u", pINFOA11->dwServiceOption );
	log_print( ucLog, "szMDN:%s", pINFOA11->szMDN );
	log_print( ucLog, "szBsMscId:%s", pINFOA11->szBsMscId );
	log_print( ucLog, "ucESN:%s", pINFOA11->ucESN );
	log_print( ucLog, "dwForwardMux:%u", pINFOA11->dwForwardMux);
	log_print( ucLog, "dwReverseMux:%u", pINFOA11->dwReverseMux );
	log_print( ucLog, "ucHomeAgentAddr:%u.%u.%u.%u", pINFOA11->ucHomeAgentAddr[0], pINFOA11->ucHomeAgentAddr[1],
												   pINFOA11->ucHomeAgentAddr[2], pINFOA11->ucHomeAgentAddr[3]);
	log_print( ucLog, "dwUpdateReason:%u", pINFOA11->dwUpdateReason );
	log_print( ucLog, "ucAirlinkType:%u", pINFOA11->ucAirlinkType );
	log_print( ucLog, "ucApplicationType:%u", pINFOA11->ucApplicationType ); 
}
		

/*
 * $Log
 */
