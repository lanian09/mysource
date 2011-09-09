/**
 *	INCLUDE HEADER FILES
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

// DQMS
#include "procid.h"
#include "common_stg.h"
#include "path.h"

// LIB
#include "loglib.h"
#include "sshmid.h"
#include "verlib.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"

// TAM
#include "filter.h"
#include "watch_filter.h"

#include "rppi_def.h"

#include "roam_init.h"
#include "roam_func.h"
#include "roam_msgq.h"
#include "roam_util.h"


/**
 *	DECLARE VARIABLES
 */
char        vERSION[7] = "R4.0.0";
int			giFinishSignal;     
int			giStopFlag;         

st_Flt_Info		*flt_info;
st_WatchFilter	*gWatchFilter;

stMEMSINFO   	*pMEMSINFO;
stHASHOINFO  	*pHASHOINFO; 
stHASHOINFO  	*pIRMINFO; 
stHASHOINFO  	*pNASIPINFO;
stHASHOINFO  	*pDEFECTINFO;
stHASHOINFO  	*pMODELINFO;
stHASHOINFO  	*pMODELINFO1;
stHASHOINFO  	*pMODELINFO2;
stTIMERNINFO 	*pTIMERNINFO;
stCIFO			*pCIFO;

/**
 *	DECLARE FUNCTIONS
 */
void invoke_del_call(void *p);

/**
 *	IMPLEMENTS FUNCTIONS
 */
S32 main()
{
	S32			dRet;           /**< 함수 Return 값 */
	OFFSET		offset;
	U8			*pNode;
	U8			*pNextNode;
	U8			*p, *data;
	S32			type, len, ismalloc;
	LOG_SIGNAL	*pstSIGNAL;


	/* Log 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_ROAM, LOG_PATH"/A_ROAM", "A_ROAM");

	/* A_RPPI 초기화 */
	if((dRet = dInitRPPI()) < 0)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] dInitCALL dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

	/* Hash 초기화 */	
	dMakeNASIPHash();
	dMakeDefectHash();
	dMakeModelHash();
	dMakeIRMHash();

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_ROAM, vERSION)) < 0)
	{
		log_print(LOGN_WARN,"MAIN : Failed in Initialize Version Info [%d]", dRet );
		exit(1);
	}

	log_print(LOGN_CRI, "START ROAM, VER=%s", vERSION);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		timerN_invoke(pTIMERNINFO);

		offset = gifo_read(pMEMSINFO, pCIFO, SEQ_PROC_A_ROAM);
		//if((offset = nifo_msg_read(pMEMSINFO, dMyQID, NULL)) > 0)
		if(offset > 0)
		{
			/* DB LOG 전송을 목적으로 하는 NODE (삭제 하지 않고 전송하기 위함 )*/
			pNode = nifo_ptr(pMEMSINFO, offset);
			pNextNode = pNode;

			do {
				p = pNextNode;

				while(p != NULL) {
					if((dRet = nifo_read_tlv_cont(pMEMSINFO, pNextNode, &type, &len, &data, &ismalloc, &p)) < 0)
						break;

					switch(type)
					{
						case LOG_SIGNAL_DEF_NUM:
							pstSIGNAL = (LOG_SIGNAL *)data;
// LOG_SIGNAL_Prt("PRINT LOG_SIGNAL", pstSIGNAL);
							switch(pstSIGNAL->uiProtoType)
							{
								case RADIUS_PROTO:
									switch(pstSIGNAL->uiMsgType)
									{       
										case RADIUS_ACCESS_MSG:
											dAccessInfo(data);
											break;  
										case RADIUS_ACCOUNT_MSG:
											dAccountInfo(data);
											break;  
										default:
											log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTO TYPE[%d]", 
													__FILE__, __FUNCTION__, __LINE__, pstSIGNAL->uiProtoType);
											break;  
									}       
									break;  
								case UPLCP_PROTO:
								case DNLCP_PROTO:
									dLcpInfo(data);
									break;  
								case UPIPCP_PROTO:
								case DNIPCP_PROTO:
									dIpcpInfo(data);
									break;  
								case CHAP_PROTO:
								case PAP_PROTO:
									dAuthInfo(data);
									break;  
								case OTHERPPP_PROTO:
									dOtherPPPInfo(data);
									break;  
								case DEF_PROTOCOL_L2TP:
									dL2TPInfo(data);
									break;
								case START_CALL_NUM:
								case A11_PROTO:
								case DIAMETER_PROTO:
								default:
									log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTO TYPE[%d]", 
											__FILE__, __FUNCTION__, __LINE__, pstSIGNAL->uiProtoType);
									break;
							}
							log_print(LOGN_INFO, "SIGNAL DATA MESSAGE END");
							break;
						case LOG_PAGE_TRANS_DEF_NUM:
							dPAGESessInfo(data);
							log_print(LOGN_INFO, "PAGE DATA MESSAGE END");
							break;
						case LOG_VOD_SESS_DEF_NUM:
							dVODSessInfo(data);
							log_print(LOGN_INFO, "VOD DATA MESSAGE END");
							break;
						case LOG_HTTP_TRANS_DEF_NUM:
							dHTTPSessInfo(data);
							log_print(LOGN_INFO, "HTTP DATA MESSAGE END");
							break;
						case LOG_TCP_SESS_DEF_NUM:
							dTCPSessInfo(data);
							log_print(LOGN_INFO, "TCP DATA MESSAGE END");
							break;
						case LOG_SIP_TRANS_DEF_NUM:
							dSIPSessInfo(data);
							log_print(LOGN_INFO, "SIP DATA MESSAGE END");
							break;
						case LOG_MSRP_TRANS_DEF_NUM:
							dMSRPSessInfo(data);
							log_print(LOGN_INFO, "MSRP DATA MESSAGE END");
							break;
						case LOG_VT_SESS_DEF_NUM:
							dVTSessInfo(data);
							log_print(LOGN_INFO, "VT DATA MESSAGE END");
							break;
						case LOG_IM_SESS_DEF_NUM:
							dIMSessInfo(data);
							log_print(LOGN_INFO, "IM DATA MESSAGE END");
							break;
						case LOG_FTP_DEF_NUM:
							dFTPSessInfo(data);
							log_print(LOGN_INFO, "FTP DATA MESSAGE END");
							break;
						case LOG_DIALUP_SESS_DEF_NUM:
							dDIALUPSessInfo(data);
							log_print(LOGN_INFO, "DIALUP DATA MESSAGE END");
							break;
						case NOTIFY_SIG_DEF_NUM:
							dNotiSigInfo(data);
							log_print(LOGN_INFO, "FILETER MESSAGE END");
							break;	
						case START_SERVICE_DEF_NUM:
							dStartServiceInfo(data);
							log_print(LOGN_INFO, "START SERVICE MESSAGE END");
							break;
						case LOG_DNS_DEF_NUM:
							dDNSSessInfo(data);
							log_print(LOGN_INFO, "DNS DATA MESSAGE END");
							break;	
						default:
							log_print(LOGN_WARN, "UNKNOWN TYPE[%d]", type);
							break;
					}

					if(ismalloc == DEF_READ_MALLOC){ free(data); }
				}

				pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);

			} while(pNode != pNextNode);

			switch(type)
			{
				case START_CALL_NUM:
				case NOTIFY_SIG_DEF_NUM:
				case START_SERVICE_DEF_NUM:
				case LOG_DIALUP_SESS_DEF_NUM:
				case STOP_CALL_NUM:
				case LOG_DNS_DEF_NUM:
				case LOG_FTP_DEF_NUM:
					log_print(LOGN_INFO, "DELETE LOGTYPE[%d]", type);
					nifo_node_delete(pMEMSINFO, pNode);
					break;
				case LOG_SIGNAL_DEF_NUM:
				case LOG_PAGE_TRANS_DEF_NUM:
				case LOG_VOD_SESS_DEF_NUM:
				case LOG_HTTP_TRANS_DEF_NUM:
				case LOG_TCP_SESS_DEF_NUM:
				case LOG_SIP_TRANS_DEF_NUM:
				case LOG_MSRP_TRANS_DEF_NUM:
				case LOG_VT_SESS_DEF_NUM:
				case LOG_IM_SESS_DEF_NUM:
					
					offset = nifo_offset(pMEMSINFO, pNode);
					dRet = gifo_write(pMEMSINFO, pCIFO, SEQ_PROC_A_ROAM, SEQ_PROC_M_LOG, offset);
					if(dRet < 0)
					{
						log_print(LOGN_CRI, LH"gifo_write dRet:[%d] LOGTYPE[%d]", LT, 
									dRet, type);
						nifo_node_delete(pMEMSINFO, pNode);
					}

#if 0		
					if((dRet = nifo_msg_write(pMEMSINFO, dLogQID, pNode)) < 0)
					{
						log_print(LOGN_CRI, "[%s][%s.%d] nifo_msg_write dRet:[%d] LOGTYPE[%d]",
								__FILE__, __FUNCTION__, __LINE__, dRet, type);
						nifo_node_delete(pMEMSINFO, pNode);
					}
#endif

					break;
				default:
					log_print(LOGN_CRI, "UNKNOWN LOGTYPE[%d]", type);
					nifo_node_delete(pMEMSINFO, pNode);
					break;
			}
		} 
		else 
		{
			usleep(0);
		}
	}

	FinishProgram();

	return 0;
}


void invoke_del_call(void *p)
{
	S32				isFirstServ;
    RPPISESS_KEY    *pstRPPIKey;
	STIME			uiCallTime;
	MTIME			uiCallMTime;
	S64				llBeforeGapTime, llAfterGapTime;
	
	stHASHONODE     *pHASHONODE;
    LOG_RPPI        *pstRPPILog;

    HData_RPPI      *pstRPPIHash;
    HData_RPPI      stRPPIHash;
	
	pstRPPIHash = &stRPPIHash;

    pstRPPIKey = &(((RPPI_TIMER *)p)->RPPIKEY);
    uiCallTime = ((RPPI_TIMER *)p)->uiCallTime;
    uiCallMTime = ((RPPI_TIMER *)p)->uiCallMTime;

	log_print(LOGN_DEBUG, "INVOKE TIMER IMSI[%s] CallTime[%d.%d]", pstRPPIKey->szIMSI, uiCallTime, uiCallMTime);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) )
    {
        pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		if (pstRPPIHash->after.dOffset == 0)
		{
			STG_DeltaTIME64(uiCallTime, uiCallMTime, pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime, &llBeforeGapTime);
			if (llBeforeGapTime == 0)
			{
				pstRPPILog = (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);
				isFirstServ = pstRPPIHash->before.uiFirstServFlag;
				log_print(LOGN_DEBUG, "Before RPPI LOG OFFSET[%ld] TIMER[%lld]", pstRPPIHash->before.dOffset, pstRPPIHash->before.timerNID);
			}
			else
			{
				log_print(LOGN_CRI,"[%s][%s.%d] Before[%d.%d]TIMER TIME[%d.%d]", __FILE__, __FUNCTION__, __LINE__,
                        pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime,
                        uiCallTime, uiCallMTime);
                return;	
			}	
		}
		else
		{
			STG_DeltaTIME64(uiCallTime, uiCallMTime, pstRPPIHash->after.uiCallTime, pstRPPIHash->after.uiCallMTime, &llAfterGapTime);
        	STG_DeltaTIME64(uiCallTime, uiCallMTime, pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime, &llBeforeGapTime);

        	if (llAfterGapTime == 0)
        	{
				log_print(LOGN_DEBUG, "After RPPI LOG OFFSET[%ld] TIMER[%lld]", pstRPPIHash->after.dOffset, pstRPPIHash->after.timerNID);
				
				/* AFTER 삭제 할때 BEFORE도 같이 삭제 */
				/* AFTER 먼저 삭제 */
            	pstRPPILog = (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->after.dOffset);
				isFirstServ = pstRPPIHash->after.uiFirstServFlag;
				if (pstRPPILog->uiReleaseTime == 0) {
//					pstRPPILog->usCallState = ROAM_TIMEOUT_STATE;
//					pstRPPILog->uiLastFailReason = A11_DEFECT + CALL_SETUP + ERR_CALL_TIMEOUT;	
//					pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					pstRPPILog->stopFlag = TIMER_STOP_CALL_NUM;
				}
				struct timeval  stNowTime;
				gettimeofday(&stNowTime, NULL);
				pstRPPILog->uiOpEndTime = stNowTime.tv_sec;
				pstRPPILog->uiOpEndMTime = stNowTime.tv_usec;

				dProcCallStop(pstRPPIHash, pstRPPILog, isFirstServ);
				dSendLogRPPI(pstRPPIHash, pstRPPILog);
            	
				pstRPPILog = (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);
				isFirstServ = pstRPPIHash->before.uiFirstServFlag;
				
				
        	}
			else if (llBeforeGapTime == 0 )
            {
                pstRPPILog = (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);
				isFirstServ = pstRPPIHash->before.uiFirstServFlag;
                log_print(LOGN_DEBUG, "Before RPPI LOG OFFSET[%ld] TIMER[%lld]",  pstRPPIHash->before.dOffset, pstRPPIHash->before.timerNID);
            }
        	else
        	{
             log_print(LOGN_CRI,"[%s][%s.%d] Before[%d.%d]After[%d.%d]TIMER TIME[%d.%d]", __FILE__, __FUNCTION__, __LINE__,
                        pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime,
                        pstRPPIHash->after.uiCallTime, pstRPPIHash->after.uiCallMTime,
                        uiCallTime, uiCallMTime);
                    return;
        	}	
		}

		if (pstRPPILog->uiReleaseTime == 0) {
//			pstRPPILog->usCallState = ROAM_TIMEOUT_STATE;
//			pstRPPILog->uiLastFailReason = A11_DEFECT + CALL_SETUP + ERR_CALL_TIMEOUT;
//			pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
			pstRPPILog->stopFlag = TIMER_STOP_CALL_NUM;
		}
		struct timeval  stNowTime;
    	gettimeofday(&stNowTime, NULL);
    	pstRPPILog->uiOpEndTime = stNowTime.tv_sec;
    	pstRPPILog->uiOpEndMTime = stNowTime.tv_usec;

		dProcCallStop(pstRPPIHash, pstRPPILog, isFirstServ);
		dSendLogRPPI(pstRPPIHash, pstRPPILog);
        
    }
	else
	{
		log_print(LOGN_CRI, "[%s][%s.%d] NOT FOUND HASH IMSI[%s] CreateTime[%d.%d]", __FILE__, __FUNCTION__, __LINE__,
										pstRPPIKey->szIMSI, uiCallTime, uiCallMTime); 
	}

}
