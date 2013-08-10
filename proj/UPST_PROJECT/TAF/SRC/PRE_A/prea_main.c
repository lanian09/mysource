
/**
 *	Include headers
 */
#include <unistd.h>				/* GETPID(2), USLEEP(3) */
#include <arpa/inet.h>			/* inet_ntop() */		

// TOP
#include "common_stg.h"
#include "commdef.h"
#include "path.h"
#include "procid.h"
#include "msgdef.h"				/* MID_XX */
#include "sshmid.h"

// LIB
#include "typedef.h"
#include "loglib.h"
#include "verlib.h"
#include "Analyze_Ext_Abs.h"	/* INFO_ETH */
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "utillib.h"

// OAM
#include "almstat.h"			/* st_GEN_INFO */

// TAF
#include "ippool_bitarray.h"
#include "debug.h"				/* DEBUG_INFO */

// .
#include "prea_flt.h"			/* getSvcOnOffFlag(), FilterPkt*() */
#include "prea_init.h"			/* dInitPREAProc(), Read_XX()'s */
#include "prea_frag.h"
#include "prea_msgq.h"

/**
 *	Define cons.
 */
#define STAT_THRU		1	
#define STAT_TOTSTAT   	2 
#define STAT_IPSTAT		3
#define STAT_UDPSTAT   	4 
#define STAT_TCPSTAT   	5 
#define STAT_SCTPSTAT	6
#define STAT_ETCSTAT	7	
#define STAT_IPERROR   	8 
#define STAT_UTCPERROR 	9 
#define STAT_FAILERROR	10	
#define STAT_FILTEROUT  11 

/**
 *	Define structures
 */
typedef struct _st_UDP_INFO
{
	S32			cnt;
	U8			*pUDP;
} st_UDP_INFO, *pst_UDP_INFO;

/**
 *	Declare var.
 */
S32					JiSTOPFlag;
S32					FinishFlag;

stMEMSINFO			*pMEMSINFO;
stCIFO				*gpCIFO;
stHASHOINFO			*pLPREAHASH;
stHASHOINFO			*pLPREASCTP;
stHASHOINFO			*pIPFRAGHASH;
stHASHOINFO			*pROAMHASH;
stTIMERNINFO		*pIPFRAGTIMER;

S32  				dSysTypeInfo = 0;

char   				 vERSION[7] = "R3.0.0";

extern int          gATCPCnt;
extern int          gARPCnt;
extern int          gAINETCnt;

DEBUG_INFO			*pDEBUGINFO;
PREA_SUBINFO		PREASUBINFO;
PREA_SUBINFO		*pPREASUBINFO = &PREASUBINFO;

st_UDP_INFO			stUDPINFO;
st_UDP_INFO			*pUDPINFO = &stUDPINFO;

st_SYSCFG_INFO		stSYSCFGINFO;
st_SYSCFG_INFO		*pSYSCFG = &stSYSCFGINFO;

//T_GENINFO    		*gen_info;
st_GEN_INFO    		*gen_info;
pst_IPPOOLLIST   	pstIPPOOLBIT;

int                 g_StatIndex = 0;

/**
 *	Declare func.
 */
char *PrintRTX(S32 rtx);
char *PrintPROTOCOL(S32 protocol);
S32 dGetTCPProcID(UINT uiClientIP);
S32 dGetRPProcID( UINT uiClientIP );
S32 dGetINETProcID(UINT uiClientIP);
S32 dGetINETProcID(UINT uiClientIP);
void invoke_del_ipfrag(void *p);
void Set_Stat( int iFlag, unsigned int uiBytes );
void print_traffic_stat(int level, int index);
void Check_StatTimer(time_t now);

/**
 *	Implement func.
 */
char *PrintRTX(S32 rtx)
{
	switch(rtx)
	{
		case DEF_FROM_CLIENT:		return "FROM CLIENT";
		case DEF_FROM_SERVER:		return "FROM SERVER";
		case DEF_FROM_NONE:			return "NONE";
		default:					return "UNKNOWN";
	}
}

char *PrintPROTOCOL(S32 protocol)
{
	switch(protocol)
	{
		case DEF_PROTOCOL_UDP:		return "UDP";
		case DEF_PROTOCOL_TCP:		return "TCP";
		default:					return "UNKNOWN";
	}
}

S32 dGetTCPProcID(UINT uiClientIP)
{
	return SEQ_PROC_A_TCP0 + ( uiClientIP % gATCPCnt ); 
}

S32 dGetRPProcID( UINT uiClientIP )
{
	return SEQ_PROC_A_RP0 + ( uiClientIP % gARPCnt );
} 

S32 dGetINETProcID(UINT uiClientIP)
{
	return SEQ_PROC_A_INET0 +  ( uiClientIP % gAINETCnt ); 
}

int main()
{
	int					dRet;
	OFFSET				offset, sub_offset;
	time_t				oldTime = 0, nowTime = 0;

	S32					dSndProcID;
	S32 				index;

	U8					szSIP[INET_ADDRSTRLEN];
	U8					szDIP[INET_ADDRSTRLEN];

	U8					*pNode, *pNextNode, *pIPFrag, *pCurrNode;
	Capture_Header_Msg	*pCAPHEAD;
	U8					*pDATA;
	INFO_ETH			*pINFOETH;
	NOTIFY_SIG			*pNOTISIG;

	stHASHONODE			*pHASHNODE;
	st_ROAMHash_Key		stROAMHashKey;
	st_ROAMHash_Key		*pKey = &stROAMHashKey;
	st_ROAMHash_Data	*pData;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_PRE_A, LOG_PATH"/PRE_A", "PRE_A");

	memset(pSYSCFG, 0x00, sizeof(st_SYSCFG_INFO));

	dRet = dInitPREAProc(&pMEMSINFO, &pIPFRAGHASH, &pIPFRAGTIMER, &pLPREAHASH, &pLPREASCTP, &pROAMHASH, pSYSCFG);
	if(dRet < 0)
	{
		log_print(LOGN_CRI, "[ERROR] FAIL dInitProc dRet[%d]", dRet);
		exit(1);
	} 

	if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_PRE_A, vERSION)) < 0 ) {
		log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_PRE_A, vERSION);
	}

	log_print(LOGN_CRI, "[PROCESS INIT SUCCESS, PROCESS START VER=%s]", vERSION);

	while(JiSTOPFlag)
	{
		nowTime = time(NULL);
		if(nowTime >= oldTime + 60) {
			oldTime = nowTime;

			memcpy(&pDEBUGINFO->PREAINFO.PREASUBINFO, pPREASUBINFO, sizeof(PREA_SUBINFO));
			pDEBUGINFO->PREAINFO.curtime = nowTime;
			memset(pPREASUBINFO, 0x00, sizeof(PREA_SUBINFO));
		}

		Check_StatTimer(nowTime);

		timerN_invoke(pIPFRAGTIMER);

		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_PRE_A)) <= 0) {
			usleep(0);
			continue;
		}

		pNextNode = nifo_ptr(pMEMSINFO, offset);
		pCurrNode = NULL;

		log_print(LOGN_INFO, "RCV FIRST MSG OFFSET[%ld]", offset);

		while(pCurrNode != pNextNode) 
		{
			pNode = pNextNode;
			pCurrNode = pNextNode;
			sub_offset = nifo_offset(pMEMSINFO, pNode);

			pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);
			pCAPHEAD = (Capture_Header_Msg *)nifo_get_value(pMEMSINFO, CAP_HEADER_NUM, sub_offset);
			pDATA = nifo_get_value(pMEMSINFO, ETH_DATA_NUM, sub_offset);

			/* CHECK FILTER RELOAD */
			if( pCAPHEAD == NULL && pDATA == NULL ) {
				pNOTISIG = (NOTIFY_SIG *)nifo_get_value( pMEMSINFO, NOTIFY_SIG_DEF_NUM, sub_offset );
				if( pNOTISIG != NULL ) {
					switch( pNOTISIG->uiType ) {
						case MID_FLT_MNIP:
							Read_MNData(pROAMHASH);
							break;
						case MID_FLT_SVC:
							Read_SVRData(pLPREAHASH);
							break;
						case MID_FLT_SCTP:
							Read_SCTPData(pLPREASCTP);
							break;
						case MID_FLT_ONOFF:
							Read_SVC_ONOFF();
							break;
						default:
							log_print( LOGN_CRI, "INVALID NOTI SIG TYPE:%d", pNOTISIG->uiType );	
							break;
					}
				}

				nifo_node_unlink_nont(pMEMSINFO, pNode);
                nifo_node_delete(pMEMSINFO, pNode);

				continue;
			}

			pPREASUBINFO->totCnt++;
			pPREASUBINFO->totSize += pCAPHEAD->datalen;	
			if((pINFOETH = (INFO_ETH *)nifo_tlv_alloc(pMEMSINFO, pNode, INFO_ETH_NUM, INFO_ETH_SIZE, DEF_MEMSET_ON)) == NULL) {
				log_print(LOGN_CRI, LH"nifo_tlv_alloc NULL", LT);
				nifo_node_unlink_nont(pMEMSINFO, pNode);
				nifo_node_delete(pMEMSINFO, pNode);
				continue;
			}

			/* THRU STAT */
			Set_Stat( STAT_THRU, pCAPHEAD->datalen );

			/* ANALYZE PROTOCOL */
			dRet = Analyze_Eth(pDATA, pCAPHEAD->datalen, pINFOETH);
			if(dRet < 0) {
				log_print(LOGN_CRI, "[ERROR] FAIL Analyze_Eth dRet[%d] datalen[%u] captime[%ld.%06ld]",
						dRet, pCAPHEAD->datalen, pCAPHEAD->curtime, pCAPHEAD->ucurtime);

				Set_Stat( STAT_FAILERROR, pCAPHEAD->datalen );

				nifo_node_unlink_nont(pMEMSINFO, pNode);
				nifo_node_delete(pMEMSINFO, pNode);

				continue;
			}

			/* TOTAL STAT */
			Set_Stat( STAT_TOTSTAT, pCAPHEAD->datalen );

			/* CHECK IP */
			if(pINFOETH->stIP.bIPHeader == 0) {
				log_print(LOGN_DEBUG, "Not IP frame. No need to handle.");
				//dump_DebugString("Not IP frame", pDATA, pCAPHEAD->datalen);
				nifo_node_unlink_nont(pMEMSINFO, pNode);
				nifo_node_delete(pMEMSINFO, pNode);
				continue;
			}

			Set_Stat( STAT_IPSTAT, pINFOETH->stIP.wTotalLength );

			/* CHECK IP CHECKSUM */
			if(pINFOETH->stIP.bChecksumErr == 1) {
				log_print(LOGN_DEBUG, "IP CHECKSUM ERROR RTX=%d:%s", pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType));
				nifo_node_unlink_nont(pMEMSINFO, pNode);
				nifo_node_delete(pMEMSINFO, pNode);

				Set_Stat( STAT_IPERROR, pINFOETH->stIP.wTotalLength );

				continue;
			}

			/* FILTER MOD ?? */
#if 0
			if(FilterMOD(pSYSCFG, pINFOETH, pCAPHEAD->bRtxType) < 0) {
				log_print(LOGN_INFO, "MOD SIP=%s DIP=%s RTX=%d:%s MOD=%d",
						util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
						pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
						pSYSCFG->mod);
				nifo_node_unlink_nont(pMEMSINFO, pNode);
				nifo_node_delete(pMEMSINFO, pNode);
				continue;
			}
#endif

			/* IP Fragmentation 처리 */
			if((pINFOETH->stIP.usIPFrag & IP_FRAG_MORE) || (pINFOETH->stIP.usIPFrag & IP_FRAG_OFFSET)) {
				log_print(LOGN_INFO, "FRAGMENTATION RCV SIP=%s DIP=%s ID=%X:%d RTX=%d:%s PROTOCOL=%d:%s",
						util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
						pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent,
						pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
						pINFOETH->stIP.ucProtocol, PrintPROTOCOL(pINFOETH->stIP.ucProtocol));

				nifo_node_unlink_nont(pMEMSINFO, pNode);
				if((pIPFrag = ip_frag(pMEMSINFO, pIPFRAGHASH, pIPFRAGTIMER, pINFOETH, pNode)) == NULL) {
					log_print(LOGN_INFO, "FRAGMENTATION START SIP=%s DIP=%s ID=%X:%d RTX=%d:%s PROTOCOL=%d:%s",
							util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
							pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent,
							pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
							pINFOETH->stIP.ucProtocol, PrintPROTOCOL(pINFOETH->stIP.ucProtocol));
					continue;
				} else {
					log_print(LOGN_INFO, "FRAGMENTATION END SIP=%s DIP=%s ID=%X:%d RTX=%d:%s PROTOCOL=%d:%s",
							util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
							pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent,
							pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
							pINFOETH->stIP.ucProtocol, PrintPROTOCOL(pINFOETH->stIP.ucProtocol));
					pNode = pIPFrag;
					sub_offset = nifo_offset(pMEMSINFO, pNode);
					pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, sub_offset);
				}
			}

			if(pINFOETH->stIP.ucProtocol == DEF_PROTOCOL_TCP) {
				/* TCP PROTOCOL */
				pPREASUBINFO->totTcpCnt++;

				Set_Stat( STAT_TCPSTAT, pINFOETH->stIP.wTotalLength );

				/* CEHCK TCP CHECKSUM */
				if(pINFOETH->stUDPTCP.bChecksumErr == 1) {
					log_print(LOGN_DEBUG, "TCP/UDP CHECKSUM ERROR RTX=%d:%s PROTOCOL=%d:%s RTX=%d:%s SIP=%s DIP=%s", 
						pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
						pINFOETH->stIP.ucProtocol, PrintPROTOCOL(pINFOETH->stIP.ucProtocol),
						pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
						util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP));

					/* TCP CHECKSUM ERROR STAT */ 
					Set_Stat( STAT_UTCPERROR, pINFOETH->stIP.wTotalLength ); 

					nifo_node_unlink_nont(pMEMSINFO, pNode);
					nifo_node_delete(pMEMSINFO, pNode);
					pPREASUBINFO->chksumCnt++;
					continue;
				}

				/* CHECK FILTER & DIRECTION */
				if( _IPPOOL_ISSET( pINFOETH->stIP.dwSrcIP, pstIPPOOLBIT ) ) {
					pCAPHEAD->bRtxType = DEF_FROM_CLIENT;
				}
				else if( _IPPOOL_ISSET( pINFOETH->stIP.dwDestIP, pstIPPOOLBIT ) ) {
					pCAPHEAD->bRtxType = DEF_FROM_SERVER;
				}
				else {
					log_print( LOGN_DEBUG, "OUT OF IP SIP=%s:%d DIP=%s:%d",
									   util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
									   util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort );

					/* FILTER OUT STAT */
					Set_Stat(STAT_FILTEROUT, pINFOETH->stIP.wTotalLength );

					nifo_node_unlink_nont(pMEMSINFO, pNode);
					nifo_node_delete(pMEMSINFO, pNode);
					pPREASUBINFO->notSvcCnt++;
					continue;
				}

				if((dRet = FilterPkt(pLPREAHASH, pINFOETH, pCAPHEAD->bRtxType)) < 0) {

					pINFOETH->usL4Code = L4_INET_TCP;
					pINFOETH->usL7Code = APP_UNKNOWN;
					pINFOETH->usAppCode = SEQ_PROC_A_INET0;

					if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT)
						dSndProcID = dGetINETProcID(pINFOETH->stIP.dwSrcIP);
					else
						dSndProcID = dGetINETProcID(pINFOETH->stIP.dwDestIP);

					/* 등록된 서비스가 아님 */
					log_print(LOGN_DEBUG, "TCP NO SVC RTX=%d:%s SIP=%s:%d DIP=%s:%d RANGE=%d dRet=%d",
							pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
							util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
							util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort,
							pSYSCFG->range, dRet);

					/* FILTER OUT STAT */
					Set_Stat(STAT_FILTEROUT, pINFOETH->stIP.wTotalLength );

					/* 인터넷 서비스인데 분석설정이 OFF 인 경우 */
					if( !(dRet = getSvcOnOffFlag((pINFOETH->usL4Code/1000)*1000)) ) {

						log_print(LOGN_DEBUG, "OFF SVC[%u] ONOFF[%d] INET TCP SVC RTX=%d:%s SIP=%s:%d DIP=%s:%d RANGE=%d",
								pINFOETH->usL4Code, dRet, pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
								util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
								util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort, pSYSCFG->range);

						nifo_node_unlink_nont(pMEMSINFO, pNode);
						nifo_node_delete(pMEMSINFO, pNode);
						pPREASUBINFO->notSvcCnt++;
						continue; // No match to filter info.
					} 
				} else {

					/* 등록된 서비스인데 분석설정이 OFF 인 경우 */
					if( pINFOETH->usRpPiFlag==PI_FLAG && pINFOETH->usSysType==TYPE_SVC ) {
						if( !(dRet = getSvcOnOffFlag((pINFOETH->usL4Code/1000)*1000)) ) {

							log_print(LOGN_DEBUG, "OFF SVC[%u] ONOFF[%d] TCP SVC RTX=%d:%s SIP=%s:%d DIP=%s:%d RANGE=%d dRet=%d",
									pINFOETH->usL4Code, dRet, pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
									util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
									util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort, pSYSCFG->range, dRet);

							nifo_node_unlink_nont(pMEMSINFO, pNode);
							nifo_node_delete(pMEMSINFO, pNode);
							pPREASUBINFO->notSvcCnt++;
							continue; // No match to filter info.
						}
					}

					pPREASUBINFO->sndTcpCnt++;

					if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT)
						dSndProcID = dGetTCPProcID(pINFOETH->stIP.dwSrcIP);
					else
						dSndProcID = dGetTCPProcID(pINFOETH->stIP.dwDestIP);
				}
			}
			else if(pINFOETH->stIP.ucProtocol == DEF_PROTOCOL_UDP) {
				/* UDP PROTOCOL */
				pPREASUBINFO->totUdpCnt++;

				Set_Stat( STAT_UDPSTAT, pINFOETH->stIP.wTotalLength );

				/* CHECK FILTER & DIRECTION */
                if( _IPPOOL_ISSET( pINFOETH->stIP.dwSrcIP, pstIPPOOLBIT ) ) {
                    pCAPHEAD->bRtxType = DEF_FROM_CLIENT;
                }
                else if( _IPPOOL_ISSET( pINFOETH->stIP.dwDestIP, pstIPPOOLBIT ) ) {
                    pCAPHEAD->bRtxType = DEF_FROM_SERVER;
                }
                else {
					/* CHECK PDSN IP */
					if( dSysTypeInfo == RP_FLAG ) {
						pCAPHEAD->bRtxType = 0;
					}
					else {
                    	log_print( LOGN_DEBUG, "OUT OF IP SIP=%s:%d DIP=%s:%d",
                                       util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
                                       util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort );

                    	/* FILTER OUT STAT */
                    	Set_Stat(STAT_FILTEROUT, pINFOETH->stIP.wTotalLength );

                    	nifo_node_unlink_nont(pMEMSINFO, pNode);
                    	nifo_node_delete(pMEMSINFO, pNode);
                    	pPREASUBINFO->notSvcCnt++;
                    	continue;
					}
                }

				/* CHECK FILTER */
				if((dRet = FilterPkt(pLPREAHASH, pINFOETH, pCAPHEAD->bRtxType)) < 0) {

					log_print(LOGN_DEBUG, "UDP NO SVC RTX=%d:%s SIP=%s:%d DIP=%s:%d RANGE=%d dRet=%d",
							pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
							util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
							util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort,
							pSYSCFG->range, dRet);
#ifdef CHK_RADIUS
                    if( dSysTypeInfo == PI_FLAG && (
                                pINFOETH->stUDPTCP.wSrcPort == 1812 || pINFOETH->stUDPTCP.wSrcPort == 1813 ||
                                pINFOETH->stUDPTCP.wDestPort == 1812 || pINFOETH->stUDPTCP.wDestPort == 1813) ) {

                        log_print(LOGN_CRI, "#### RADIUS NO SVC RTX=%d:%s SIP=%s:%d DIP=%s:%d RANGE=%d ",
                                pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
                                util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
                                util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort,
                                pSYSCFG->range);
                    }
#endif
					/* VT TRAFFIC */
					if( dSysTypeInfo == PI_FLAG && (
								pINFOETH->stUDPTCP.wSrcPort == 1234 || pINFOETH->stUDPTCP.wDestPort == 1234 || 
								pINFOETH->stUDPTCP.wSrcPort == 1254 || pINFOETH->stUDPTCP.wDestPort == 1254 || 
								pINFOETH->stUDPTCP.wSrcPort == 1235 || pINFOETH->stUDPTCP.wDestPort == 1235 || 
								pINFOETH->stUDPTCP.wSrcPort == 1255 || pINFOETH->stUDPTCP.wDestPort == 1255) ) {

						pINFOETH->usAppCode = SEQ_PROC_A_VT;
					} else if (dSysTypeInfo == PI_FLAG) {
						pINFOETH->usL4Code = L4_INET_TCP;
						pINFOETH->usL7Code = APP_UNKNOWN;
						pINFOETH->usAppCode = SEQ_PROC_A_INET0;

						if( !(dRet = getSvcOnOffFlag((pINFOETH->usL4Code/1000)*1000)) ) {
							log_print(LOGN_DEBUG, "OFF SVC[%u] ONOFF[%d] INET UDP SVC RTX=%d:%s SIP=%s:%d DIP=%s:%d RANGE=%d",
									pINFOETH->usL4Code, dRet, pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
									util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
									util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort, pSYSCFG->range);

							nifo_node_unlink_nont(pMEMSINFO, pNode);
							nifo_node_delete(pMEMSINFO, pNode);
							continue; // No match to filter info.
						}
					} else {

						/* FILTER OUT STAT */
						Set_Stat(STAT_FILTEROUT, pINFOETH->stIP.wTotalLength );

						nifo_node_unlink_nont(pMEMSINFO, pNode);
						nifo_node_delete(pMEMSINFO, pNode);
						continue; // No match to filter info.
					}
				}
				else {
					/* SET pCAPHEAD->bRtxType */
					pCAPHEAD->bRtxType = dRet;
	
					/* 등록된 서비스인데 분석설정이 OFF 인 경우 */
					if( pINFOETH->usRpPiFlag==PI_FLAG && pINFOETH->usSysType==TYPE_SVC ) {
						if( !(dRet = getSvcOnOffFlag((pINFOETH->usL4Code/1000)*1000)) ) {

							log_print(LOGN_DEBUG, "OFF SVC[%u] ONOFF[%d] UDP SVC RTX=%d:%s SIP=%s:%d DIP=%s:%d RANGE=%d",
									pINFOETH->usL4Code, dRet, pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
									util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
									util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort, pSYSCFG->range);

							nifo_node_unlink_nont(pMEMSINFO, pNode);
							nifo_node_delete(pMEMSINFO, pNode);
							continue; // No match to filter info.
						}
					} 		
				}

				pPREASUBINFO->sndUdpCnt++;
		
				if(pINFOETH->usAppCode == SEQ_PROC_A_RP0) {
					if( pCAPHEAD->bRtxType == DEF_FROM_CLIENT)
                        dSndProcID = dGetRPProcID(pINFOETH->stIP.dwSrcIP);
                    else
                        dSndProcID = dGetRPProcID(pINFOETH->stIP.dwDestIP);
				}
				else if(pINFOETH->usAppCode == SEQ_PROC_A_RADIUS) {
					dSndProcID = SEQ_PROC_A_RADIUS;
				}
#if 0			/* 
				   RTP 트래픽일 경우 아래의 경로로 패스가 결정된다.
				   PRE_A -> A_TCP -> A_SIPM -> A_SIPT -> A_VT -> A_CALL 
				 */
				else if(pINFOETH->usAppCode == SEQ_PROC_A_VT)
					dSndProcID = SEQ_PROC_A_VT;
#endif
				else if(pINFOETH->usAppCode == SEQ_PROC_A_DNS) {
					dSndProcID = SEQ_PROC_A_DNS;
				}
				else if(pINFOETH->usAppCode == SEQ_PROC_A_L2TP) {
					/* 2010.03.04 ADD BY LDH */
					dSndProcID = SEQ_PROC_A_L2TP;
				}
				else if(pINFOETH->usAppCode == SEQ_PROC_A_INET0) {
					if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT)
						dSndProcID = dGetINETProcID(pINFOETH->stIP.dwSrcIP);
					else
						dSndProcID = dGetINETProcID(pINFOETH->stIP.dwDestIP);
				}
				else {
					if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT)
						dSndProcID = dGetTCPProcID(pINFOETH->stIP.dwSrcIP);
					else
						dSndProcID = dGetTCPProcID(pINFOETH->stIP.dwDestIP);
				}
			}
			else if( pINFOETH->stIP.ucProtocol == DEF_PROTOCOL_SCTP ) {
				Set_Stat( STAT_SCTPSTAT, pINFOETH->stIP.wTotalLength );

				/* SCTP PROTOCOL */
				if((dRet = FilterPktSCTP(pLPREASCTP, pINFOETH, pCAPHEAD)) < 0) {
					log_print( LOGN_DEBUG, "SCTP NO SVC RTX=%d:%s SIP=%s:%d DIP=%s:%d RANGE=%d dRet=%d",
                        pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
                        util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
                        util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort,
                        pSYSCFG->range, dRet);

					/* FILTER OUT STAT */
                    Set_Stat(STAT_FILTEROUT, pINFOETH->stIP.wTotalLength );

                    nifo_node_unlink_nont(pMEMSINFO, pNode);
                    nifo_node_delete(pMEMSINFO, pNode);
                    continue; // No match to filter info.
                }

				dSndProcID = SEQ_PROC_A_SCTP;
			}
			else if( pINFOETH->stIP.ucProtocol == DEF_PROTOCOL_GRE && dSysTypeInfo == RP_FLAG ) {
				Set_Stat( STAT_ETCSTAT, pINFOETH->stIP.wTotalLength );

				/* CHECK FILTER & DIRECTION */
                if( _IPPOOL_ISSET( pINFOETH->stIP.dwSrcIP, pstIPPOOLBIT ) ) {
                    pCAPHEAD->bRtxType = DEF_FROM_CLIENT;

					dSndProcID = dGetRPProcID(pINFOETH->stIP.dwSrcIP);
                }
                else if( _IPPOOL_ISSET( pINFOETH->stIP.dwDestIP, pstIPPOOLBIT ) ) {
                    pCAPHEAD->bRtxType = DEF_FROM_SERVER;

					dSndProcID = dGetRPProcID(pINFOETH->stIP.dwDestIP);
                }
                else {
					pCAPHEAD->bRtxType = 0;

					/* CHECK FILTER */
                	if((dRet = FilterPkt(pLPREAHASH, pINFOETH, pCAPHEAD->bRtxType)) < 0) {

                    	log_print(LOGN_DEBUG, "GRE NO PDSN RTX=%d:%s SIP=%s:%d DIP=%s:%d RANGE=%d dRet=%d",
                            pCAPHEAD->bRtxType, PrintRTX(pCAPHEAD->bRtxType),
                            util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
                            util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort,
                            pSYSCFG->range, dRet);

                    	/* FILTER OUT STAT */
                    	Set_Stat(STAT_FILTEROUT, pINFOETH->stIP.wTotalLength );
                
                    	nifo_node_unlink_nont(pMEMSINFO, pNode);
                    	nifo_node_delete(pMEMSINFO, pNode);
                    	continue; // No match to filter info.
                	}
					else {
						/* SET pCAPHEAD->bRtxType & SET dSndProcID */
						pCAPHEAD->bRtxType = dRet;
						dSndProcID = ( pCAPHEAD->bRtxType == DEF_FROM_CLIENT ) ? 
							dGetRPProcID(pINFOETH->stIP.dwSrcIP) : dGetRPProcID(pINFOETH->stIP.dwDestIP);
					}	
                }
			}
			else {
				Set_Stat( STAT_ETCSTAT, pINFOETH->stIP.wTotalLength );

				// Protocol not to analyze.
				nifo_node_unlink_nont(pMEMSINFO, pNode);
				nifo_node_delete(pMEMSINFO, pNode);
				continue;
			}

			/* roam systype */
			if(dSndProcID == SEQ_PROC_A_RADIUS || dSndProcID == SEQ_PROC_A_L2TP) {
				pKey->ip = (pCAPHEAD->bRtxType == DEF_FROM_CLIENT) ? pINFOETH->stIP.dwSrcIP : pINFOETH->stIP.dwDestIP;
				pKey->reserved = 0;
				
				if((pHASHNODE = hasho_find(pROAMHASH, (U8 *)pKey)) != NULL) {
					pData = (st_ROAMHash_Data *)nifo_ptr(pROAMHASH, pHASHNODE->offset_Data);
					pINFOETH->usSysType = pData->systype;
				}
			}
	
			nifo_node_unlink_nont(pMEMSINFO, pNode);
			if(pINFOETH->usAppCode == SEQ_PROC_A_INET0) {

				if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT)
					index = pINFOETH->stIP.dwSrcIP % gAINETCnt;
				else
					index = pINFOETH->stIP.dwDestIP % gAINETCnt;

				if((dRet = dSend_PREABUFF_Data(pMEMSINFO, dSndProcID, pNode, pCAPHEAD->curtime, index)) < 0) {
					log_print(LOGN_CRI, LH"dSend_PREABUFF_Data dRet[%d][%s]", LT, dRet, strerror(-dRet));

					Set_Stat( STAT_FAILERROR, pCAPHEAD->datalen );

					nifo_node_delete(pMEMSINFO, pNode);
				}
			} else if((dRet = dSend_PREA_Data(pMEMSINFO, dSndProcID, pNode)) < 0) {
				log_print(LOGN_CRI, LH"dSend_PREA_Data dRet[%d][%s]", LT, dRet, strerror(-dRet));

				Set_Stat( STAT_FAILERROR, pCAPHEAD->datalen );

				nifo_node_delete(pMEMSINFO, pNode);
			}
			/* 
			else {
				log_print(LOGN_INFO, "SEND MSG OFFSET[%lld]", nifo_offset(pMEMSINFO, pNode));
			}
			*/
		}

	}

	if(pUDPINFO->pUDP != NULL)
		nifo_node_delete(pMEMSINFO, pUDPINFO->pUDP);

	FinishProgram();

	return 0;
}

void invoke_del_ipfrag(void *p)
{
	U8					*pHEAD;
	IP_FRAG				*pIPFRAG;
	IP_FRAG_KEY			*pIPFRAGKEY;
	stHASHONODE			*pHASHNODE;

	pIPFRAGKEY = &(((IP_FRAG_COMMON *)p)->IPFRAGKEY);

	if((pHASHNODE = hasho_find(pIPFRAGHASH, (U8 *)pIPFRAGKEY)) != NULL)
	{
		log_print(LOGN_INFO, "TIMEOUT INVOKE");
		pIPFRAG = (IP_FRAG *)nifo_ptr(pIPFRAGHASH, pHASHNODE->offset_Data);
		pHEAD = nifo_ptr(pMEMSINFO, pIPFRAG->offset_NODE);
		nifo_node_delete(pMEMSINFO, pHEAD);
		hasho_del(pIPFRAGHASH, (U8 *)pIPFRAGKEY);
	}
	else
	{
		log_print(LOGN_CRI, "TIMEOUT INVOKE BUT HASH NULL");
	}
}

void Set_Stat( int iFlag, unsigned int uiBytes )
{
    switch( iFlag )
    {
		case STAT_THRU:
			gen_info->ThruStat[g_StatIndex].uiFrames ++;
            gen_info->ThruStat[g_StatIndex].ulBytes += uiBytes;
			break;

        case STAT_TOTSTAT:
            gen_info->TotStat[g_StatIndex].uiFrames ++;
            gen_info->TotStat[g_StatIndex].ulBytes += uiBytes;
            break;

		case STAT_IPSTAT:
			gen_info->IPStat[g_StatIndex].uiFrames ++;
            gen_info->IPStat[g_StatIndex].ulBytes += uiBytes;
			break;

        case STAT_UDPSTAT:
            gen_info->UDPStat[g_StatIndex].uiFrames ++;
            gen_info->UDPStat[g_StatIndex].ulBytes += uiBytes;
            break;

        case STAT_TCPSTAT:
            gen_info->TCPStat[g_StatIndex].uiFrames ++;
            gen_info->TCPStat[g_StatIndex].ulBytes += uiBytes;
            break;

		case STAT_SCTPSTAT:
			gen_info->SCTPStat[g_StatIndex].uiFrames ++;
            gen_info->SCTPStat[g_StatIndex].ulBytes += uiBytes;
            break;

		case STAT_ETCSTAT:
			gen_info->ETCStat[g_StatIndex].uiFrames ++;
			gen_info->ETCStat[g_StatIndex].ulBytes += uiBytes;
			break;

		case STAT_IPERROR:
            gen_info->IPError[g_StatIndex].uiFrames ++;
            gen_info->IPError[g_StatIndex].ulBytes += uiBytes;
            break;

        case STAT_UTCPERROR:
            gen_info->UTCPError[g_StatIndex].uiFrames ++;
            gen_info->UTCPError[g_StatIndex].ulBytes += uiBytes;
            break;

		case STAT_FAILERROR:
            gen_info->FailData[g_StatIndex].uiFrames ++;
            gen_info->FailData[g_StatIndex].ulBytes += uiBytes;
            break;

        case STAT_FILTEROUT:
            gen_info->FilterOut[g_StatIndex].uiFrames ++;
            gen_info->FilterOut[g_StatIndex].ulBytes += uiBytes;
            break;

        default:
            break;
    }
}

void print_traffic_stat(int level, int index)
{
    log_print(level, "\n"
            "=========================== TRANSMISSION STAT ============================================\n"
			"THRU         [%10u][%20lu]\n"
            "TOTSTAT      [%10u][%20lu]\n"
			"IPSTAT       [%10u][%20lu]\n"
            "UDPSTAT      [%10u][%20lu]\n"
            "TCPSTAT      [%10u][%20lu]\n"
			"SCTPSTAT     [%10u][%20lu]\n"
			"ETCSTAT      [%10u][%20lu]\n"
            "IPERROR      [%10u][%20lu]\n"
            "UTCPERROR    [%10u][%20lu]\n"
            "FAIL         [%10u][%20lu]\n"
            "FILTEROUT    [%10u][%20lu]\n"
            "==========================================================================================",
			gen_info->ThruStat[index].uiFrames,		gen_info->ThruStat[index].ulBytes,
			gen_info->TotStat[index].uiFrames,		gen_info->TotStat[index].ulBytes,
            gen_info->IPStat[index].uiFrames, 		gen_info->IPStat[index].ulBytes,
            gen_info->UDPStat[index].uiFrames, 		gen_info->UDPStat[index].ulBytes,
            gen_info->TCPStat[index].uiFrames, 		gen_info->TCPStat[index].ulBytes,
			gen_info->SCTPStat[index].uiFrames,		gen_info->SCTPStat[index].ulBytes,
			gen_info->ETCStat[index].uiFrames,		gen_info->ETCStat[index].ulBytes,
            gen_info->IPError[index].uiFrames, 		gen_info->IPError[index].ulBytes,
            gen_info->UTCPError[index].uiFrames,	gen_info->UTCPError[index].ulBytes,
            gen_info->FailData[index].uiFrames, 	gen_info->FailData[index].ulBytes,
            gen_info->FilterOut[index].uiFrames, 	gen_info->FilterOut[index].ulBytes );
}

void Check_StatTimer(time_t now)
{
    if( ((now/300)%MAX_STAT_SIZE) != g_StatIndex )
    {
        print_traffic_stat( LOGN_CRI, g_StatIndex );

        g_StatIndex = (now/300) % MAX_STAT_SIZE;
        gen_info->ThruStat[g_StatIndex].KeyTime 	= now/300;
		gen_info->TotStat[g_StatIndex].KeyTime		= gen_info->ThruStat[g_StatIndex].KeyTime;
        gen_info->IPStat[g_StatIndex].KeyTime 		= gen_info->ThruStat[g_StatIndex].KeyTime;
        gen_info->UDPStat[g_StatIndex].KeyTime 		= gen_info->ThruStat[g_StatIndex].KeyTime;
        gen_info->TCPStat[g_StatIndex].KeyTime 		= gen_info->ThruStat[g_StatIndex].KeyTime;
		gen_info->SCTPStat[g_StatIndex].KeyTime		= gen_info->ThruStat[g_StatIndex].KeyTime;
		gen_info->ETCStat[g_StatIndex].KeyTime		= gen_info->ThruStat[g_StatIndex].KeyTime;
        gen_info->IPError[g_StatIndex].KeyTime 		= gen_info->ThruStat[g_StatIndex].KeyTime;
        gen_info->UTCPError[g_StatIndex].KeyTime 	= gen_info->ThruStat[g_StatIndex].KeyTime;
        gen_info->FailData[g_StatIndex].KeyTime 	= gen_info->ThruStat[g_StatIndex].KeyTime;
        gen_info->FilterOut[g_StatIndex].KeyTime 	= gen_info->ThruStat[g_StatIndex].KeyTime;
    }
}
