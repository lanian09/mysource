/**
 * Include headers
 */
#include <unistd.h>

// TOP
#include "commdef.h"
#include "procid.h"
#include "common_stg.h"
#include "path.h"
#include "sshmid.h"

// LIB headers
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"
#include "utillib.h"
#include "verlib.h"

// TAF headers
#include "mmdb_greentry.h"		/* GREENTRY_KEY, GREENTRY_DATA */
#include "arp_head.h"			/* DEF_CALL_RECALL */
#include "Analyze_Ext_Abs.h"	/* INFO_ETH */
#include "mmdb_psess.h"			/* PSESS_KEY */
#include "capdef.h"				/* Capture_Header_Msg */

/**
 * Declare variables
 */
stMEMSINFO	*pMEMSINFO;
stCIFO		*gpCIFO;
int			JiSTOPFlag;
int			FinishFlag;

char		vERSION[7] = "R3.0.0";
void		*hGREEntrySess = NULL;

long		glcurtime;
long		glucurtime; 

/* MULTI PROCESS INFO */
int			gAGRECnt;
char		gszMyProc[32];
int			PROCNO;
UINT		guiSeqProcID;

/**
 * Declare functions
 */
int Print_GREEntry(void);

/**
 *	Implement func.
 */
int main( int argc, char *argv[] )
{
	S32			i;
	S32			dRet;
	S32         dHeaderLen;
	OFFSET		offset;

	Capture_Header_Msg  *pCAPHEAD;
    INFO_ETH            *pINFOETH;
	LOG_SIGNAL			*pstSIGLog;
    U8                  *pNode;
    U8                  *pDATA;

	int                 dLen;
    char                szLOGPATH[128];
	char				szIPAddr[32];
	
	PSESS_KEY           PSessKey;
    PSESS_DATA          *pPSessData;
	GREENTRY_DATA	*pstData;
	GREENTRY_KEY	stKey;

	memcpy(&gszMyProc[0], argv[0], strlen(argv[0]));

    dLen = strlen(gszMyProc);
    PROCNO = atoi(&gszMyProc[dLen-1]);
    guiSeqProcID = SEQ_PROC_A_GRE0 + PROCNO;

	sprintf(szLOGPATH, LOG_PATH"/%s", gszMyProc);
	log_init(S_SSHM_LOG_LEVEL,getpid(), guiSeqProcID, szLOGPATH, gszMyProc);

	SetUpSignal();

	dRet = dInit_Proc();
	if( dRet < 0 ) {
		log_print( LOGN_DEBUG, "ERROR IN dInit_Proc dRet:%d", dRet );
		exit(0);
	}

	if((dRet = set_version(S_SSHM_VERSION, guiSeqProcID, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,SeqProcID=%d,VER=%s)", dRet, guiSeqProcID, vERSION);
    } 

	log_print(LOGN_CRI, "[A_GRE PROCESS INIT SUCCESS, PROCESS START VER=%s]", vERSION);
//	int no = 0;

	while( JiSTOPFlag ) {

		for( i=0; i<300; i++ ) {
			if((offset = gifo_read(pMEMSINFO, gpCIFO, guiSeqProcID)) > 0) {

//				log_print( LOGN_DEBUG, "#### %d, %d", no++, time(NULL));

				pCAPHEAD = (Capture_Header_Msg *)nifo_get_value(pMEMSINFO, CAP_HEADER_NUM, offset);
				pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, offset);
				pNode = nifo_ptr(pMEMSINFO, offset);
				pDATA = (U8 *)nifo_get_value(pMEMSINFO, ETH_DATA_NUM, offset);

				if((pCAPHEAD == NULL) || (pINFOETH == NULL)) {
					if( (pstSIGLog = (LOG_SIGNAL *)nifo_get_value(pMEMSINFO, LOG_SIGNAL_DEF_NUM, offset )) == NULL ) {
						if( (pstSIGLog = (LOG_SIGNAL *)nifo_get_value(pMEMSINFO, START_CALL_NUM, offset )) != NULL ) {
							if( (dRet = gifo_write( pMEMSINFO, gpCIFO, guiSeqProcID, SEQ_PROC_A_CALL, nifo_offset(pMEMSINFO,pNode) )) < 0 ){
								log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
								nifo_node_delete(pMEMSINFO, pNode);
							}
						} else {
							nifo_node_delete(pMEMSINFO, pNode);
						}

					} else {
						if( (dRet = gifo_write( pMEMSINFO, gpCIFO, guiSeqProcID, SEQ_PROC_A_CALL, nifo_offset(pMEMSINFO,pNode) )) < 0 ) {
							log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet=%d"EH, LT, dRet, ET);
							nifo_node_delete(pMEMSINFO, pNode);
						}

					}

				} else if( pINFOETH->stIP.ucProtocol == DEF_PROTOCOL_GRE ) {
					if( pCAPHEAD->bRtxType == DEF_FROM_CLIENT ) {
						PSessKey.uiServingPCF = htonl(pINFOETH->stIP.dwSrcIP);
					} else if( pCAPHEAD->bRtxType == DEF_FROM_SERVER ) {
						PSessKey.uiServingPCF = htonl(pINFOETH->stIP.dwDestIP);
					} else {
						log_print( LOGN_CRI, "%s.%d INVALID DIRECTION:%d", __FUNCTION__, __LINE__, pCAPHEAD->bRtxType );
						nifo_node_delete(pMEMSINFO, pNode);
						continue;
					}

					PSessKey.uiKey = pINFOETH->stGRE.dwGREKey;

					pPSessData = Search_PSESS( &PSessKey );
					if( pPSessData == NULL ) {
#if 0
						stKey.uiPCFIP = PSessKey.uiServingPCF;
						stKey.uiGREKey = PSessKey.uiKey;
						pstData = (st_GREEntry_Data *)Search_Data(hGREEntrySess, &stKey);
						if( pstData == NULL ) {
							log_print( LOGN_INFO, "PSESS NOT FOUND 1, SRVPCF:%u,%15s,%d KEY:%u", 
									stKey.uiPCFIP, util_cvtipaddr(szIPAddr, PSessKey.uiServingPCF), (PSessKey.uiServingPCF%gAGRECnt), stKey.uiGREKey );
							nifo_node_delete(pMEMSINFO, pNode);
							log_print(LOGN_INFO, "GRE KEY ENTRY COUNT. [%u]", Count_Data(hGREEntrySess));
							//Print_GREEntry();
							continue;
						}
						PSessKey.uiKey = pstData->uiMainGREKey;
						pPSessData = Search_PSESS( &PSessKey ); 
						if( pPSessData == NULL ) {
							log_print( LOGN_INFO, "PSESS NOT FOUND 2, SRVPCF:%u,%15s,%d KEY:%u", 
									PSessKey.uiServingPCF, util_cvtipaddr(szIPAddr, PSessKey.uiServingPCF), (PSessKey.uiServingPCF%gAGRECnt), PSessKey.uiKey );
							nifo_node_delete(pMEMSINFO, pNode);
							continue;
						}
#endif
						stKey.uiPCFIP = PSessKey.uiServingPCF;
						stKey.uiGREKey = PSessKey.uiKey;
						pstData = Search_GREENTRY(&stKey);
						if( pstData == NULL ) {
							log_print( LOGN_INFO, "PSESS NOT FOUND 1, SRVPCF:%u,%15s,%d KEY:%u", 
									stKey.uiPCFIP, util_cvtipaddr(szIPAddr, PSessKey.uiServingPCF), (PSessKey.uiServingPCF%gAGRECnt), stKey.uiGREKey );
							nifo_node_delete(pMEMSINFO, pNode);
							//Print_GREEntry();
							continue;
						}
						PSessKey.uiKey = pstData->uiMainGREKey;
						pPSessData = Search_PSESS( &PSessKey ); 
						if( pPSessData == NULL ) {
							log_print( LOGN_INFO, "PSESS NOT FOUND 2, SRVPCF:%u,%15s,%d KEY:%u", 
									PSessKey.uiServingPCF, util_cvtipaddr(szIPAddr, PSessKey.uiServingPCF), (PSessKey.uiServingPCF%gAGRECnt), PSessKey.uiKey );
							nifo_node_delete(pMEMSINFO, pNode);
							continue;
						}
						log_print( LOGN_INFO, "FIND PSESS SRVPCF:%15s KEY:%u MAINKEY:%u", util_cvtipaddr(szIPAddr, PSessKey.uiServingPCF), pINFOETH->stGRE.dwGREKey, PSessKey.uiKey );
					}
					else {
						log_print( LOGN_INFO, "FIND PSESS SRVPCF:%15s KEY:%u", util_cvtipaddr(szIPAddr, PSessKey.uiServingPCF), PSessKey.uiKey );

#if 1 /* INYOUNG */
						/*
						 * DORMANT 상태에서 DOWNLINK 데이터가 발생하는 경우 
						 */
//						if(pPSessData->uiStopFlag == 2) {
						if(pPSessData->uiStopFlag == 2 && pPSessData->uiCallType == 0) {

							log_print( LOGN_DEBUG, "@@@ RECALL PSESS SrvPCF[%u] Key[%u] CreateTime[%ld] IMSI[%s] A11Flag[0x%04x] STOP[%u] CALL[%u]", 
									pPSessData->key.uiServingPCF, pPSessData->key.uiKey, pPSessData->CreateTime, pPSessData->szMIN, 
									pPSessData->RegA11.usA11Flag, pPSessData->uiStopFlag, pPSessData->uiCallType );

							glcurtime = pCAPHEAD->curtime;
							glucurtime= pCAPHEAD->ucurtime; 

							dRet = Report_SIGLog( START_RP_DATA_RECALL_NUM, pPSessData->RegA11.ucReqMsgCode, pPSessData );
							if( dRet < 0 )
								log_print( LOGN_DEBUG, "[FAIL:%d] Report_SIGLog()", dRet );
							/* TODO: 세마포어 사용 여부 결정 */
							pPSessData->uiCallType = DEF_CALL_RECALL;
						}
#endif /* INYOUNG */
					}

					/* CHECK TRACE INFO */
					dRet = dCheck_TraceInfo( pPSessData, pDATA, pCAPHEAD );
					if( dRet < 0 )
						log_print( LOGN_INFO, "[%s.%d] ERROR IN dCheck_TraceInfo dRet:%d", __FUNCTION__, __LINE__, dRet );


					/* UPDATE GRE PACKET INFO */
					if( pCAPHEAD->bRtxType == DEF_FROM_CLIENT ) {
						pPSessData->uiUpGREFrames++;

						/* CHECK pINFOETH->stIP.wTotalLength, pINFOETH->stGRE.wDataSize */
						pPSessData->uiUpGREBytes += pINFOETH->stIP.wTotalLength;
					}
					else {
						pPSessData->uiDownGREFrames++;

						/* CHECK pINFOETH->stIP.wTotalLength, pINFOETH->stGRE.wDataSize */
						pPSessData->uiDownGREBytes += pINFOETH->stIP.wTotalLength;
					}

					dHeaderLen = 14 + pINFOETH->stIP.wIPHeaderLen + pINFOETH->stGRE.wGREHeaderSize;
					dRet = MergePPPData( pPSessData, pDATA + dHeaderLen, pINFOETH->stGRE.wDataSize, pINFOETH, pCAPHEAD );
					if( dRet < 0 )
						log_print( LOGN_INFO, "%s.%d ERROR IN MergePPPData dRet:%d", __FUNCTION__, __LINE__, dRet );

					nifo_node_delete(pMEMSINFO, pNode);
				}
				else {
					log_print( LOGN_INFO, "DISCARD PROTOCOL:%d", pINFOETH->stIP.ucProtocol );
					nifo_node_delete(pMEMSINFO, pNode);
				}
			}
			else {
				usleep(0);
				break;
			}
		}


	}

	return 0;
}

int Print_GREEntry(void)
{
#if 0
	st_GREEntry_Key stKey1, stKey2, *pKey;
	st_GREEntry_Data *pData;

	memset(&stKey1, 0, sizeof(stKey1));
	memset(&stKey2, 0xFF, sizeof(stKey2));

	while( (pKey = Select_Key(hGREEntrySess, &stKey1, &stKey2)) ) {
		pData = Search_Data(hGREEntrySess, pKey);
		if( pData ) {
			log_print(LOGN_CRI, "PCF[%u] GRE[%u] MAINGRE[%u]", pKey->uiPCFIP, pKey->uiGREKey, pData->uiMainGREKey);
		}
		else {
			log_print(LOGN_CRI, "ERROR!!!");
		}

		stKey1 = *pKey;
	}
#endif

	return 0;
}


