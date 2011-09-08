/**
 * Include headers
 */
// DQMS headers
#include "commdef.h"		/* Capture_Header_Msg */
#include "procid.h"
#include "sshmid.h"

// LIB headers
#include "common_stg.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"
#include "ipclib.h"

// TAF headers
#include "mmdb_psess.h"		/* PSESS_DATA */
#include "mmdb_greentry.h"	/* GREENTRY_TABLE */
#include "arp_head.h"		/* defines about LCP */

// .
#include "agre_func.h"

/**
 * Declare variables
 */
extern int			PROCNO;

extern long			glcurtime;
extern long			glucurtime; 
extern UINT			guiSeqProcID;

st_TraceList		*pstTRACE;
stMEMSINFO          *pMEMSINFO;
stCIFO				*gpCIFO;

/**
 *	Implement func.
 */
int Init_A11_PSESS( )
{
	UINT    uiShmSESSKey;

	uiShmSESSKey = S_SSHM_A11_PSESS0 + PROCNO;

	log_print( LOGN_CRI, "INIT A11 SHM KEY:%u PROCNO:%u", uiShmSESSKey, PROCNO );

	if( shm_init(uiShmSESSKey, sizeof(SESS_TABLE), (void**)&psess_tbl) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(A11 SHM Key=%d)", uiShmSESSKey);
        return -1;
    }

    return 1;
}

int Init_TraceShm( )
{

	if( shm_init(S_SSHM_TRACE_INFO, st_TraceList_SIZE, (void**)&pstTRACE) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(TRACE_INFO=%d)", S_SSHM_TRACE_INFO);
        return -1;
    }

    return 0;
}

int Init_GREEntry_Shm(void)
{
	UINT    uiShmSESSKey;

    uiShmSESSKey = S_SSHM_A11_GREENTRY0 + PROCNO;

    log_print( LOGN_CRI, "INIT A11_GREENTRY SHM KEY:%u PROCNO:%u", uiShmSESSKey, PROCNO );

    if( shm_init(uiShmSESSKey, sizeof(GREENTRY_TABLE), (void**)&greentry_tbl) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(A11 SHM Key=%d)", uiShmSESSKey);
        return -1;
    }
    return 1;
}

int dCheck_TraceInfo( PSESS_DATA *pstSessData, unsigned char *pData, Capture_Header_Msg *pstCAP )
{
    int     i, dRet;
    UCHAR   *pstNode;
    UCHAR   *pBuffer;

    st_TraceMsgHdr  *pstTrcMsg;

    /*
    * pstSessData->szMIN        : 4500010~
    * pstSessData->szTraceMIN   : 010~
    */

    for( i=0; i< pstTRACE->count; i++ ) {
        if( pstTRACE->stTraceInfo[i].dType == TRC_TYPE_IMSI ) {

			if( atoll(pstSessData->szMIN) == pstTRACE->stTraceInfo[i].stTraceID.llIMSI ) {

				log_print( LOGN_DEBUG, "TRACE IMSI[%d] %s, %s", i, pstSessData->szMIN, pstTRACE->stTraceInfo[i].stTraceID.szMIN );
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

                dRet = gifo_write( pMEMSINFO, gpCIFO, SEQ_PROC_A_GRE, SEQ_PROC_CI_LOG, nifo_offset(pMEMSINFO, pstNode) );
				if( dRet < 0 ) {
                    log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
                    return -4;
                }

                return 1;
            }
        }
		else if( pstTRACE->stTraceInfo[i].dType == TRC_TYPE_MDN ) {

			if( atoll(pstSessData->szTraceMIN) == pstTRACE->stTraceInfo[i].stTraceID.llIMSI ) {

				log_print( LOGN_DEBUG, "TRACE MIN[%d] %s, %s", i, pstSessData->szTraceMIN, pstTRACE->stTraceInfo[i].stTraceID.szMIN );
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
				if( dRet < 0 ) {
                    log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
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

int Report_SIGLog( UCHAR ucProtoType, UCHAR ucMsgType, PSESS_DATA *pPSessData )
{   
    int             dRet; 
	U8				szIPAddr[32];
    UCHAR           *pstNode; 
    LOG_SIGNAL      *pstSIGLog;

    UINT            uiDuration; 
    //st_SIGNAL_Log   *pstSIGLog;

    if( (pstNode = nifo_node_alloc(pMEMSINFO)) == NULL ) {
        log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
        return -1;
    }

	if( ucProtoType == START_RP_DATA_RECALL_NUM) {
		if( (pstSIGLog = (LOG_SIGNAL *)nifo_tlv_alloc(pMEMSINFO, pstNode, START_RP_DATA_RECALL_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_OFF)) == NULL) {
			log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, START_RP_DATA_RECALL_NUM);
			nifo_node_delete(pMEMSINFO, pstNode);
			return -2;
		}
	} else {
		if( (pstSIGLog = (LOG_SIGNAL *)nifo_tlv_alloc(pMEMSINFO, pstNode, LOG_SIGNAL_DEF_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_OFF)) == NULL ) {
			log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, LOG_SIGNAL_DEF_NUM);
			nifo_node_delete(pMEMSINFO, pstNode);
			return -3;
		}
	}

    memset( pstSIGLog, 0x00, LOG_SIGNAL_SIZE );

	/* SET COMMON INFO */
    pstSIGLog->uiPCFIP      = util_cvtuint(pPSessData->key.uiServingPCF);
    pstSIGLog->uiGREKey     = pPSessData->key.uiKey;
    pstSIGLog->uiClientIP   = pPSessData->uiIPAddr;
	pstSIGLog->uiNASName    = util_cvtuint(pPSessData->uiNASIP);


	pstSIGLog->ucSYSID      = (pPSessData->stBSMSC.SYS_ID);
	pstSIGLog->ucBSCID      = (pPSessData->stBSMSC.BSC_ID);
	pstSIGLog->ucBTSID      = util_cvtushort(pPSessData->stBSMSC.BTS_ID);
	pstSIGLog->ucFA_ID      = (pPSessData->stBSMSC.FA_ID);
	pstSIGLog->ucSECTOR     = (pPSessData->stBSMSC.SEC_ID);

    memcpy(&pstSIGLog->szBSMSC[0], &pPSessData->ucBSMSC[4], 8 );
	memcpy(&pstSIGLog->szBSMSC[8], &pPSessData->ucBSMSC[0], 4 );

	log_print( LOGN_INFO, "SYSID:%u BSCID:%u BTSID:%u FAID:%u SEC:%u BSMSC:%s",
                       pstSIGLog->ucSYSID, pstSIGLog->ucBSCID, pstSIGLog->ucBTSID, 
                       pstSIGLog->ucFA_ID, pstSIGLog->ucSECTOR, pstSIGLog->szBSMSC );

	pstSIGLog->uiSvcOption  = (pPSessData->uiSvcOption);
	memcpy( pstSIGLog->szNetOption, pPSessData->szNetOption, MAX_SVCOPTION_SIZE );

	switch( ucProtoType )
    {
		/*
        case START_CALL_NUM:
        case NUM_UDP_A11 :
            pstSIGLog->uiCallTime       = pPSessData->CreateTime;
            pstSIGLog->uiCallMTime      = pPSessData->CreateMTime;
            memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
            memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );

            pstSIGLog->uiProtoType      = ucProtoType;
            pstSIGLog->uiMsgType        = pPSessData->A11.ucReqMsgCode;

            pstSIGLog->uiSessStartTime  = (pPSessData->A11.StartTime);
            pstSIGLog->uiSessStartMTime = (pPSessData->A11.StartMTime);

            pstSIGLog->uiSessEndTime    = (pPSessData->A11.EndTime);
            pstSIGLog->uiSessEndMTime   = (pPSessData->A11.EndMTime);
            uiDuration = GetUpTime( pPSessData->A11.StartTime, pPSessData->A11.StartMTime,
                                        pPSessData->A11.EndTime, pPSessData->A11.EndMTime );

            pstSIGLog->uiSrcIP = util_cvtuint(pPSessData->A11.uiSrcIP);
            pstSIGLog->uiDestIP = util_cvtuint(pPSessData->A11.uiDestIP);

            pstSIGLog->uiSessDuration   = (uiDuration);
            pstSIGLog->uiRespCode       = pPSessData->A11.ucRepMsgCode;

            //pstSIGLog->ucADR          = pPSessData->ucRegiReply;

            //pstSIGLog->usReqCount = (pPSessData->A11.ucReqCount);
            //pstSIGLog->usNakCount = (pPSessData->A11.ucNakCount);
            //pstSIGLog->usRejCount = (pPSessData->A11.ucRejCount);

            pstSIGLog->uiUpGREPkts  = pPSessData->uiUpGREFrames;
            pstSIGLog->uiDnGREPkts  = pPSessData->uiDownGREFrames;
            pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
            pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;

            if( ucMsgType == 0x01 )
            {
                pstSIGLog->usLiftTime   = (pPSessData->usLifetime);
                pstSIGLog->ucAppType    = pPSessData->ucAppType;
                if( pPSessData->ucAppType != 0 )
                {
                    pstSIGLog->uiFMux       = (pPSessData->uiFMux);
                    pstSIGLog->uiRMux       = (pPSessData->uiRMux);
                    pstSIGLog->ucAirLink    = pPSessData->ucAirlink;
                }
            }
            else if( ucMsgType == 0x14 )
            {
                pstSIGLog->usUpdateReason   = (pPSessData->uiUpdateReason);
            }

            break;
		*/

#if 1 /* INYOUNG */

		/*
		 * TODO :  START_CALL_NUM 와 같은 동일한 데이터 저장을 해야 하는지 확인할 필요가 있음.
		 */
		case START_RP_DATA_RECALL_NUM:

//			pstSIGLog->uiCallTime       = pPSessData->CreateTime;
//			pstSIGLog->uiCallMTime      = pPSessData->CreateMTime;
			pstSIGLog->uiCallTime       = glcurtime;
			pstSIGLog->uiCallMTime      = glucurtime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
			memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );

			pstSIGLog->uiProtoType      = ucProtoType;
			pstSIGLog->uiMsgType        = pPSessData->RegA11.ucReqMsgCode;

			pstSIGLog->uiSessStartTime  = (pPSessData->RegA11.StartTime);
			pstSIGLog->uiSessStartMTime = (pPSessData->RegA11.StartMTime);

			pstSIGLog->uiSessEndTime    = (pPSessData->RegA11.EndTime);
			pstSIGLog->uiSessEndMTime   = (pPSessData->RegA11.EndMTime);
			uiDuration = GetUpTime( pPSessData->RegA11.StartTime, pPSessData->RegA11.StartMTime,
					pPSessData->RegA11.EndTime, pPSessData->RegA11.EndMTime );

			pstSIGLog->uiSrcIP = util_cvtuint(pPSessData->RegA11.uiSrcIP);
			pstSIGLog->uiDestIP = util_cvtuint(pPSessData->RegA11.uiDestIP);

			pstSIGLog->uiSessDuration   = (uiDuration);
			pstSIGLog->uiRespCode       = pPSessData->RegA11.ucRepMsgCode;

			//pstSIGLog->ucADR          = pPSessData->ucRegiReply;
			//pstSIGLog->usReqCount = (pPSessData->RegA11.ucReqCount);
			//pstSIGLog->usNakCount = (pPSessData->RegA11.ucNakCount);
			//pstSIGLog->usRejCount = (pPSessData->RegA11.ucRejCount);

			pstSIGLog->uiUpGREPkts  = pPSessData->uiUpGREFrames;
			pstSIGLog->uiDnGREPkts  = pPSessData->uiDownGREFrames;
			pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
			pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;

			if( ucMsgType == 0x01 )
			{
				pstSIGLog->usLiftTime   = (pPSessData->RegA11.usLifetime);
				pstSIGLog->ucAppType    = pPSessData->ucAppType;
				if( pPSessData->ucAppType != 0 )
				{
					pstSIGLog->uiFMux       = (pPSessData->uiFMux);
					pstSIGLog->uiRMux       = (pPSessData->uiRMux);
					pstSIGLog->ucAirLink    = pPSessData->RegA11.ucAirlink;
				}
			}
			else if( ucMsgType == 0x14 )
			{
				pstSIGLog->usUpdateReason   = (pPSessData->RegA11.uiUpdateReason);
			}

			break;
#endif	/* INYOUNG */

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
                pstSIGLog->uiMsgType        = pPSessData->UpLCP.ucReqMsgCode;
                pstSIGLog->uiSessStartTime  = (pPSessData->UpLCP.StartTime);
                pstSIGLog->uiSessStartMTime = (pPSessData->UpLCP.StartMTime);
                pstSIGLog->uiSessEndTime    = (pPSessData->UpLCP.EndTime);
                pstSIGLog->uiSessEndMTime   = (pPSessData->UpLCP.EndMTime);
                uiDuration = GetUpTime( pPSessData->UpLCP.StartTime, pPSessData->UpLCP.StartMTime,
                                        pPSessData->UpLCP.EndTime, pPSessData->UpLCP.EndMTime );

                pstSIGLog->uiRespCode       = pPSessData->UpLCP.ucRepMsgCode;
                pstSIGLog->ucPPPReqCnt      = (pPSessData->UpLCP.ucReqCount);
                pstSIGLog->ucPPPNakCnt      = (pPSessData->UpLCP.ucNakCount);
                pstSIGLog->ucPPPRejCnt      = (pPSessData->UpLCP.ucRejCount);
            }
            else if( ucMsgType == DEF_LCP_TERM_REQ ) /* term */
            {
                pstSIGLog->uiMsgType        = pPSessData->LCPTerm.ucReqMsgCode;
                pstSIGLog->uiSessStartTime  = (pPSessData->LCPTerm.StartTime);
                pstSIGLog->uiSessStartMTime = (pPSessData->LCPTerm.StartMTime);
                pstSIGLog->uiSessEndTime    = (pPSessData->LCPTerm.EndTime);
                pstSIGLog->uiSessEndMTime   = (pPSessData->LCPTerm.EndMTime);

                uiDuration = GetUpTime( pPSessData->LCPTerm.StartTime, pPSessData->LCPTerm.StartMTime,
                                        pPSessData->LCPTerm.EndTime, pPSessData->LCPTerm.EndMTime );
                pstSIGLog->uiRespCode       = pPSessData->LCPTerm.ucRepMsgCode;
                pstSIGLog->ucPPPReqCnt      = (pPSessData->LCPTerm.ucReqCount);
                pstSIGLog->ucPPPNakCnt      = (pPSessData->LCPTerm.ucNakCount);
                pstSIGLog->ucPPPRejCnt      = (pPSessData->LCPTerm.ucRejCount);
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

            pstSIGLog->uiSessDuration   = (uiDuration);

            pstSIGLog->uiSrcIP          = util_cvtuint(pPSessData->key.uiServingPCF);
            pstSIGLog->uiDestIP         = (pPSessData->uiHomeAgent);

            break;

		case NUM_DOWN_LCP :
            pstSIGLog->uiCallTime       = pPSessData->CreateTime;
            pstSIGLog->uiCallMTime      = pPSessData->CreateMTime;
            memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
            memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
            pstSIGLog->uiProtoType      = ucProtoType;

            pstSIGLog->uiUpGREPkts  = pPSessData->uiUpGREFrames;
            pstSIGLog->uiDnGREPkts  = pPSessData->uiDownGREFrames;
            pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
            pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;

            if( ucMsgType == DEF_LCP_CONF_REQ ) /* configure-request */
            {
                pstSIGLog->uiMsgType        = pPSessData->DownLCP.ucReqMsgCode;
                pstSIGLog->uiSessStartTime  = (pPSessData->DownLCP.StartTime);
                pstSIGLog->uiSessStartMTime = (pPSessData->DownLCP.StartMTime);
                pstSIGLog->uiSessEndTime    = (pPSessData->DownLCP.EndTime);
                pstSIGLog->uiSessEndMTime   = (pPSessData->DownLCP.EndMTime);
                uiDuration = GetUpTime( pPSessData->DownLCP.StartTime, pPSessData->DownLCP.StartMTime,
                                        pPSessData->DownLCP.EndTime, pPSessData->DownLCP.EndMTime );
                pstSIGLog->uiRespCode       = pPSessData->DownLCP.ucRepMsgCode;
                pstSIGLog->ucPPPReqCnt      = (pPSessData->DownLCP.ucReqCount);
                pstSIGLog->ucPPPNakCnt      = (pPSessData->DownLCP.ucNakCount);
                pstSIGLog->ucPPPRejCnt      = (pPSessData->DownLCP.ucRejCount);
            }
            else if( ucMsgType == DEF_LCP_TERM_REQ ) /* term */
            {
                pstSIGLog->uiMsgType        = pPSessData->LCPTerm.ucReqMsgCode;
                pstSIGLog->uiSessStartTime  = (pPSessData->LCPTerm.StartTime);
                pstSIGLog->uiSessStartMTime = (pPSessData->LCPTerm.StartMTime);
                pstSIGLog->uiSessEndTime    = (pPSessData->LCPTerm.EndTime);
                pstSIGLog->uiSessEndMTime   = (pPSessData->LCPTerm.EndMTime);
                uiDuration = GetUpTime( pPSessData->LCPTerm.StartTime, pPSessData->LCPTerm.StartMTime,
                                        pPSessData->LCPTerm.EndTime, pPSessData->LCPTerm.EndMTime );
                pstSIGLog->uiRespCode       = pPSessData->LCPTerm.ucRepMsgCode;
                pstSIGLog->ucPPPReqCnt      = (pPSessData->LCPTerm.ucReqCount);
                pstSIGLog->ucPPPNakCnt      = (pPSessData->LCPTerm.ucNakCount);
                pstSIGLog->ucPPPRejCnt      = (pPSessData->LCPTerm.ucRejCount);
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
            pstSIGLog->uiSessDuration   = (uiDuration);

            pstSIGLog->uiSrcIP          = (pPSessData->uiHomeAgent);
            pstSIGLog->uiDestIP         = util_cvtuint(pPSessData->key.uiServingPCF);

            break;

		case NUM_UP_IPCP :
            pstSIGLog->uiCallTime       = pPSessData->CreateTime;
            pstSIGLog->uiCallMTime      = pPSessData->CreateMTime;
			memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
            memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
            pstSIGLog->uiProtoType      = ucProtoType;
            pstSIGLog->uiMsgType        = pPSessData->UpIPCP.ucReqMsgCode;

            pstSIGLog->uiUpGREPkts  = pPSessData->uiUpGREFrames;
            pstSIGLog->uiDnGREPkts  = pPSessData->uiDownGREFrames;
            pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
            pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;

            pstSIGLog->uiSessStartTime  = (pPSessData->UpIPCP.StartTime);
            pstSIGLog->uiSessStartMTime = (pPSessData->UpIPCP.StartMTime);
            pstSIGLog->uiSessEndTime    = (pPSessData->UpIPCP.EndTime);
            pstSIGLog->uiSessEndMTime   = (pPSessData->UpIPCP.EndMTime);
            uiDuration = GetUpTime( pPSessData->UpIPCP.StartTime, pPSessData->UpIPCP.StartMTime,
                                        pPSessData->UpIPCP.EndTime, pPSessData->UpIPCP.EndMTime );
            pstSIGLog->uiSessDuration   = (uiDuration);

            pstSIGLog->uiSrcIP = util_cvtuint(pPSessData->key.uiServingPCF);
            pstSIGLog->uiDestIP = (pPSessData->uiHomeAgent);

            pstSIGLog->uiRespCode       = pPSessData->UpIPCP.ucRepMsgCode;
            pstSIGLog->ucPPPReqCnt      = (pPSessData->UpIPCP.ucReqCount);
            pstSIGLog->ucPPPNakCnt      = (pPSessData->UpIPCP.ucNakCount);
            pstSIGLog->ucPPPRejCnt      = (pPSessData->UpIPCP.ucRejCount);
            pstSIGLog->uiClientIP       = (pPSessData->uiIPAddr);

            break;

		case NUM_DOWN_IPCP :
            pstSIGLog->uiCallTime       = pPSessData->CreateTime;
            pstSIGLog->uiCallMTime      = pPSessData->CreateMTime;
            memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
            memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
            pstSIGLog->uiProtoType      = ucProtoType;
            pstSIGLog->uiMsgType        = pPSessData->DownIPCP.ucReqMsgCode;

            pstSIGLog->uiUpGREPkts  = pPSessData->uiUpGREFrames;
            pstSIGLog->uiDnGREPkts  = pPSessData->uiDownGREFrames;
            pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
            pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;

            pstSIGLog->uiSessStartTime  = (pPSessData->DownIPCP.StartTime);
            pstSIGLog->uiSessStartMTime = (pPSessData->DownIPCP.StartMTime);
            pstSIGLog->uiSessEndTime    = (pPSessData->DownIPCP.EndTime);
            pstSIGLog->uiSessEndMTime   = (pPSessData->DownIPCP.EndMTime);
            uiDuration = GetUpTime( pPSessData->DownIPCP.StartTime, pPSessData->DownIPCP.StartMTime,
                                        pPSessData->DownIPCP.EndTime, pPSessData->DownIPCP.EndMTime );
            pstSIGLog->uiSessDuration   = (uiDuration);

            pstSIGLog->uiSrcIP          = (pPSessData->uiHomeAgent);
            pstSIGLog->uiDestIP         = util_cvtuint(pPSessData->key.uiServingPCF);

            pstSIGLog->uiRespCode       = pPSessData->DownIPCP.ucRepMsgCode;
            pstSIGLog->ucPPPReqCnt      = (pPSessData->DownIPCP.ucReqCount);
            pstSIGLog->ucPPPNakCnt      = (pPSessData->DownIPCP.ucNakCount);
            pstSIGLog->ucPPPRejCnt      = (pPSessData->DownIPCP.ucRejCount);
            pstSIGLog->uiClientIP       = (pPSessData->uiIPAddr);

            break;

		case NUM_CHAP :
        case NUM_PAP :
            pstSIGLog->uiCallTime       = pPSessData->CreateTime;
            pstSIGLog->uiCallMTime      = pPSessData->CreateMTime;
            memcpy( pstSIGLog->szIMSI, pPSessData->szMIN, MAX_MIN_SIZE );
            memcpy( pstSIGLog->szMIN, pPSessData->szTraceMIN, MAX_MIN_SIZE );
            pstSIGLog->uiProtoType      = ucProtoType;
            pstSIGLog->uiMsgType        = pPSessData->CHAPAP.ucReqMsgCode;

            pstSIGLog->uiUpGREPkts  = pPSessData->uiUpGREFrames;
            pstSIGLog->uiDnGREPkts  = pPSessData->uiDownGREFrames;
            pstSIGLog->uiUpGREBytes = pPSessData->uiUpGREBytes;
            pstSIGLog->uiDnGREBytes = pPSessData->uiDownGREBytes;

            pstSIGLog->uiSessStartTime  = (pPSessData->CHAPAP.StartTime);
            pstSIGLog->uiSessStartMTime = (pPSessData->CHAPAP.StartMTime);
            pstSIGLog->uiSessEndTime    = (pPSessData->CHAPAP.EndTime);
            pstSIGLog->uiSessEndMTime   = (pPSessData->CHAPAP.EndMTime);
            pstSIGLog->uiPPPResponseTime = (pPSessData->CHAPResTime);
            pstSIGLog->uiPPPResponseMTime = (pPSessData->CHAPResMTime);

            uiDuration = GetUpTime( pPSessData->CHAPAP.StartTime, pPSessData->CHAPAP.StartMTime,
                                        pPSessData->CHAPAP.EndTime, pPSessData->CHAPAP.EndMTime );
            pstSIGLog->uiSessDuration   = (uiDuration);

            pstSIGLog->uiSrcIP          = (pPSessData->uiHomeAgent);
            pstSIGLog->uiDestIP         = util_cvtuint(pPSessData->key.uiServingPCF);

            pstSIGLog->uiRespCode       = pPSessData->CHAPAP.ucRepMsgCode;
            pstSIGLog->ucPPPReqCnt      = (pPSessData->CHAPAP.ucReqCount);
            pstSIGLog->ucPPPNakCnt      = (pPSessData->CHAPAP.ucNakCount);
            pstSIGLog->ucPPPRejCnt      = (pPSessData->CHAPAP.ucRejCount);

            sprintf(pstSIGLog->szAuthUserName, "%s", pPSessData->szAuthUserName);

            pstSIGLog->uiPPPResponseTime    = (pPSessData->CHAPResTime);
            pstSIGLog->uiPPPResponseMTime   = (pPSessData->CHAPResMTime);

            break;

        default :
            log_print( LOGN_WARN, "Unknown ProtoType[%u]", ucProtoType );
            nifo_node_delete(pMEMSINFO, pstNode);
            return -1;
    }

    //LOG_SIGNAL_Prt( "Report_SIGLog", pstSIGLog );

	log_print( LOGN_DEBUG, "LOG_SIG[%3u] IMSI:%s CT:%10u.%06u CIP:%15s PCF:%u MSG:%3u SS:%10u.%06u SE:%10u.%06u AIR:%u PPP:0X%04X",
                        ucProtoType, pstSIGLog->szIMSI, pstSIGLog->uiCallTime, pstSIGLog->uiCallMTime, util_cvtipaddr(szIPAddr, pstSIGLog->uiClientIP),
                        pstSIGLog->uiPCFIP, pstSIGLog->uiMsgType, pstSIGLog->uiSessStartTime, pstSIGLog->uiSessStartMTime,
                        pstSIGLog->uiSessEndTime, pstSIGLog->uiSessEndMTime, pstSIGLog->ucAirLink, pPSessData->usPPPFlag );

    if( (dRet = gifo_write( pMEMSINFO, gpCIFO, guiSeqProcID, SEQ_PROC_A_CALL, nifo_offset( pMEMSINFO, pstNode ))) < 0 ) {
        log_print(LOGN_CRI, "[%s][%s.%d] gifo_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
		nifo_node_delete(pMEMSINFO, pstNode);
        return -4;
    }

    return 0;
}
