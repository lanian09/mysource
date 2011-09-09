/**
 * Include headers
 */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

// TOP
#include "path.h"
#include "procid.h"
#include "func_time_check.h"
#include "filter.h"
#include "capdef.h"
#include "sshmid.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"
#include "verlib.h"
#include "Analyze_Ext_Abs.h"

// TAF
#include "debug.h"
#include "arp_head.h"
#include "mmdb_psess.h"

// .
#include "arp_init.h"
#include "arp_func.h"

/**
 * Declare variables
 */
S32						giStopFlag = 1;			/**< main loop Flag 0: Stop, 1: Loop */
S32						FinishFlag;

char    				vERSION[7] = "R3.0.0";

stMEMSINFO      		*pMEMSINFO;
stCIFO					*gpCIFO;
DEBUG_INFO				*pDEBUGINFO;
st_TraceList			*pstTRACE;
st_FuncTimeCheckList    stFuncTimeCheckList;
st_FuncTimeCheckList    *pFUNC = &stFuncTimeCheckList;
PSESS_DATA  			g_FirstPSessData, g_LastPSessData;
st_Flt_Info      		*flt_info;

void *hGREEntrySess = NULL;
void *hFlowIDSess = NULL;

extern PSESS_TABLE     	psess_tbl;
extern stHASHOINFO      *pCALLHASH;

/* MULTI PROCESS INFO */
int              		gAGRECnt = 0;
char            		gszMyProc[32];
int						PROCNO;
UINT					guiSeqProcID;

time_t					g_tCheckTime;

S32 					gSemID;

/**
 * Declare functions
 */
void RP_CheckTimer( time_t tCheckTime );

/**
 *	Implement funnc.
 */
int main( int argc, char *argv[] )
{
	S32				i, dRet;
	OFFSET			offset;

	Capture_Header_Msg	*pCAPHEAD;
	INFO_ETH			*pINFOETH;
	U8					*pNode;
	U8					*pDATA;

	int					dLen;
	char                szLOGPATH[128];
	char				ucCheck = 0;

	int					dSeqProcID;
	time_t				tChkTime;

	
	memcpy(&gszMyProc[0], argv[0], strlen(argv[0]));

	dLen = strlen(gszMyProc);
    PROCNO = atoi(&gszMyProc[dLen-1]);
    guiSeqProcID = SEQ_PROC_A_RP0 + PROCNO;


	/* log_print 초기화 */
	sprintf(szLOGPATH, LOG_PATH"/%s", gszMyProc);
	log_init(S_SSHM_LOG_LEVEL, getpid(), guiSeqProcID, szLOGPATH, gszMyProc);

	bcon_init( szLOGPATH );

	SetUpSignal();

	memset( &g_FirstPSessData, 0x00, sizeof(PSESS_DATA) );
    memset( &g_LastPSessData, 0xFF, sizeof(PSESS_DATA) );

	if((dRet = dInit_Proc()) < 0 ) {
		log_print( LOGN_CRI, "[%s.%d] ERROR IN dInit_Proc dRet:%d", __FUNCTION__, __LINE__, dRet );
		exit(0);
	}

	if((dRet = set_version(S_SSHM_VERSION, guiSeqProcID, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, guiSeqProcID, vERSION);
    }

	log_print(LOGN_CRI, "[A_RP PROCESS INIT SUCCESS, PROCESS START VER=%s]", vERSION);

	time( &g_tCheckTime );

	tChkTime = g_tCheckTime;	

	/* MAIN LOOP */
	while(giStopFlag)
	{
		time( &tChkTime);
		if( tChkTime != g_tCheckTime ) {
			RP_CheckTimer( tChkTime );

			if( (tChkTime % 60) == 0 ) {
				if( ucCheck == 0 ) {
        			log_print( LOGN_CRI, "A_RP SESSION_COUNT:%d", psess_tbl->used );
					ucCheck = 1;
				}
			}
			else
				ucCheck = 0;
		}
		 
		for(i = 0; i < 300; i++) {
			if((offset = gifo_read(pMEMSINFO, gpCIFO, guiSeqProcID)) > 0) {
				
//				log_print( LOGN_DEBUG, "#### %d, %d", no++, time(NULL));

				pCAPHEAD = (Capture_Header_Msg *)nifo_get_value(pMEMSINFO, CAP_HEADER_NUM, offset);
				pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, offset);
				pNode = nifo_ptr(pMEMSINFO, offset);
				pDATA = (U8 *)nifo_get_value(pMEMSINFO, ETH_DATA_NUM, offset);

				if((pCAPHEAD == NULL) || (pINFOETH == NULL)) {
					nifo_node_delete(pMEMSINFO, pNode);
				} else {
					switch( pINFOETH->stIP.ucProtocol ) {
						case DEF_PROTOCOL_UDP:	/* UDP */
							if( pINFOETH->stUDPTCP.wSrcPort == DEF_A11_PORT && pINFOETH->stUDPTCP.wDestPort == DEF_A11_PORT ) {
								/* A11 MESSAGE */
								dRet = ProcA11( pDATA, pINFOETH, pCAPHEAD );
								if( dRet < 0 )
									log_print( LOGN_INFO, "FAIL IN ProcA11 dRet:%d", dRet );

								nifo_node_delete(pMEMSINFO, pNode);
							}
							else {
								log_print( LOGN_INFO, "NOT A11 PORT:%u,%u", pINFOETH->stUDPTCP.wSrcPort, pINFOETH->stUDPTCP.wDestPort );
								nifo_node_delete(pMEMSINFO, pNode);
							}

							break;
						case DEF_PROTOCOL_GRE:	/* GRE */
							if( pCAPHEAD->bRtxType == DEF_FROM_CLIENT )
								dSeqProcID = dGetGREProcID(pINFOETH->stIP.dwSrcIP);
							else
								dSeqProcID = dGetGREProcID(pINFOETH->stIP.dwDestIP);

							dRet = gifo_write( pMEMSINFO, gpCIFO, guiSeqProcID, dSeqProcID, nifo_offset(pMEMSINFO,pNode) );
							if( dRet < 0 ) {
								log_print( LOGN_CRI, "[%s.%d] ERROR IN gifo_write dRet:%d", __FUNCTION__, __LINE__, dRet );
								nifo_node_delete(pMEMSINFO, pNode);
							}

							break;
						default:
							log_print( LOGN_INFO, "DISCARD PROTOCOL:%d", pINFOETH->stIP.ucProtocol );
							nifo_node_delete(pMEMSINFO, pNode);
							break;
					}
				}
			} else {
				usleep(0);
				break;
			}
		}
	} /* while-loop end (main) */

	FinishProgram();

	return 1;
}


/*******************************************************************************

*******************************************************************************/
void RP_CheckTimer( time_t tCheckTime )
{
	int			i, dRet;
	UINT		uiTimerValue;
    PSESS_DATA *pPSessData;

	st_CallHashKey      stCallKey;
    pst_CallHashData    pstCallData;

    stHASHONODE         *pHASHNODE;


    for( i=0; i < 50; i++ ) {
    	pPSessData = Select_PSESS( &g_FirstPSessData.key, &g_LastPSessData.key );
    	if( pPSessData == NULL ) {

        	memset( &g_FirstPSessData.key, 0x00, sizeof(PSESS_KEY) );
        	g_tCheckTime = tCheckTime;
        	break;
    	}
    	else {

			/* INYOUNG
			 * DORMANT 상태 유지시간 타이어 추가
			 */
			if( pPSessData->uiStopFlag == 1 )
                uiTimerValue = flt_info->stTimerInfo.usTimerInfo[RP_DORM_TIMEOUT];
			else if( pPSessData->uiStopFlag == 2 )
                uiTimerValue = flt_info->stTimerInfo.usTimerInfo[RP_RCALL_TIMEOUT];
            else
                uiTimerValue = flt_info->stTimerInfo.usTimerInfo[RP_CALL_TIMEOUT];

			/* 착신데이타 후 시그널없는 경우 처리 */
			if ( pPSessData->uiCallType == DEF_CALL_RECALL )
                uiTimerValue = flt_info->stTimerInfo.usTimerInfo[RP_RCALL_SIGWAIT];

        	log_print( LOGN_INFO, "ProcTimer : IMSI[%s] CreateT[%u.%d] LastUpdateT[%u.%d] check_time[%u] TIMER[%u] STOP[%u] CALL[%u]",
            					pPSessData->szMIN, pPSessData->CreateTime, pPSessData->CreateMTime, 
								pPSessData->LastUpdateTime, pPSessData->LastUpdateMTime, tCheckTime, uiTimerValue, 
								pPSessData->uiStopFlag, pPSessData->uiCallType );

        	memcpy( &g_FirstPSessData.key, &pPSessData->key, sizeof(PSESS_KEY) );


			if( pPSessData->LastUpdateTime + uiTimerValue < tCheckTime ) {

				log_print( LOGN_DEBUG, "PROCA11:RP_STOP T_OUT I[%s] CT[%10u.%06d] PCF[%10u] PD[%10u] K[%10u] TOUT[%u] STOP[%u] CALL[%u]",
						pPSessData->szMIN, pPSessData->CreateTime, pPSessData->CreateMTime,
						pPSessData->key.uiServingPCF, pPSessData->uiNASIP, pPSessData->key.uiKey, uiTimerValue, 
						pPSessData->uiStopFlag, pPSessData->uiCallType );

				ProcPPPSess( pPSessData );

				stCallKey.llIMSI = atoll(pPSessData->szMIN);
				if( (pHASHNODE = hashs_find( pCALLHASH, (U8 *)&stCallKey )) != NULL ) {
					pstCallData = (pst_CallHashData)nifo_ptr(pCALLHASH, pHASHNODE->offset_Data);
	
					/* DORMANT 유지시간만큼 대기 시간 후 정리 할 때는 TIMER 시그널을 보내지 않는다. */
					if( pstCallData->uiPCFIP == pPSessData->key.uiServingPCF && pPSessData->uiStopFlag != 2 ) {

						dRet = Report_SIGLog( TIMER_STOP_CALL_NUM, pPSessData->RegA11.ucReqMsgCode, pPSessData );
						if( dRet < 0 )
							log_print( LOGN_DEBUG, "[FAIL:%d] Report_SIGLog()", dRet );
					} else if( pstCallData->uiPCFIP == pPSessData->key.uiServingPCF && pPSessData->uiCallType == DEF_CALL_RECALL) {
						dRet = Report_SIGLog( TIMER_STOP_CALL_NUM, pPSessData->RegA11.ucReqMsgCode, pPSessData );
						if( dRet < 0 )
							log_print( LOGN_DEBUG, "[FAIL:%d] Report_SIGLog()", dRet );

						pPSessData->uiCallType = DEF_CALL_NORMAL;
						return;
					}
				}

				pPSessData->uiCallType = DEF_CALL_NORMAL;
				/* YOON, 20110424
				 * 1. DORMANT 상태일 경우에 DORMANT 유지시간으로 타임아웃 시간을 설정하고 세션을 유지
				 * 2. 착신대기상태일 경우 타임아웃시에 세션 정리 
				 */
				if(pPSessData->uiStopFlag == 0 || pPSessData->uiStopFlag == 2) {
					dRet = RemoveA10( pPSessData );
					if( dRet < 0 )
						log_print( LOGN_DEBUG, "[FAIL:%d] RemoveA10()", dRet );
				} else if(pPSessData->uiStopFlag == 1) {
					pPSessData->uiStopFlag = 2;
					log_print( LOGN_DEBUG, "    CHANGE FLAG STOP[%u] CALL[%u]", pPSessData->uiStopFlag, pPSessData->uiCallType);
				}
			}
    	}
    }
}


/*
 * $Log
 */
