#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "utillib.h"

// PROJECT
#include "procid.h"
#include "sshmid.h"
#include "common_stg.h"

// TAF
#include "tools.h"

// .
#include "pciv_func.h"
#include "pciv_msgq.h"
#include "pciv_lex.h"


extern S32		gACALLCnt;

extern S64		curSessCnt;

extern struct timeval      stNowTime;

const int endian = 1;
#define is_bigendian() ( (*(char*)&endian) == 0 )

S32 dGetCALLSeqID(U32 uiClientIP)
{
	return SEQ_PROC_A_CALL + ( uiClientIP % gACALLCnt );
}

/** dSvcProcess.
 * TCP로 부터 받은 데이터를 Control하고 TCP Session, JNET Transaction 관리
 *
 * @param	*pMEMSINFO	: New Interface 관리를 위한 구조체
 * @param	*pHASH	:  Session을 관리하는 HASH
 * @param	*pTCPINFO	: TCP로 부터 받은 데이터의 정보를 담고 있는 구조체
 * @param	*pNode		: TCP로 부터 New Interface Node
 *
 * @return	S32
 * @see		pciv_func.c pciv_main.c pciv_init.c pciv_api.c
 *
 * @exception	nothing
 * @note		nothing
 **/

S32 dSvcProcess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHASH, TCP_INFO *pTCPINFO, U8 *pNode, U8* pDATA)
{
	S32	dRet;

	/*******/
	U8	szSrcIP[INET_ADDRSTRLEN];
	U8	szDestIP[INET_ADDRSTRLEN];
	unsigned short usSrcPort;
	unsigned short usDestPort;

	util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP);
	util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP);
	usSrcPort = pTCPINFO->usCliPort;
	usDestPort = pTCPINFO->usSrvPort;
	/*******/

	IV_SESS_KEY		SESSKEY;
	IV_SESS_KEY*	pSESSKEY;
	pSESSKEY 		= &SESSKEY;
	IV_SESS_DATA*	pSESS;
	stHASHONODE		*pHASHNODE;
	
	/* TCP 상태에 따른 처리*/
#if DEBUG_MODE
	dRet = RcvPacketInfo(pTCPINFO);
#endif

	MakeHashKey(pTCPINFO, pSESSKEY);

	switch(pTCPINFO->cTcpFlag)
	{
		case DEF_TCP_START:
			{
				/**
				 * A_TCP에서 TCP Session이 생성될 경우
				 * Session NODE 생성
				 **/
				if((pSESS = pCreateSession(pMEMSINFO, pHASH, pSESSKEY, pTCPINFO)) == NULL){
					log_print(LOGN_CRI, "** [%s][%s.%d]TCP_START. SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] pCreateSession NULL", 
						__FILE__, __FUNCTION__,__LINE__, szSrcIP, usSrcPort, szDestIP, usDestPort );

					return -1;
				}
				/* *************************************************************************** */

				if(dSend_Data(pMEMSINFO, dGetCALLSeqID(pSESSKEY->uiCliIP), pNode) < 0) {
					log_print(LOGN_CRI, "** [%s][%s.%d]TCP_START. SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] Fail SendSignal.",
					    __FILE__,__FUNCTION__, __LINE__, szSrcIP, usSrcPort, szDestIP, usDestPort );

					return -11;
				}

				log_print(LOGN_DEBUG, "[%s][%s.%d]TCP_START. SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]",
						__FILE__,__FUNCTION__, __LINE__, szSrcIP, usSrcPort, szDestIP, usDestPort );
			}break;

		case DEF_TCP_END:
			{
				/**
				 * A_TCP에서 TCP Session이 종료될 경우
				 * Session NODE 삭제
				 **/
				if((pHASHNODE = hasho_find(pHASH, (U8*)pSESSKEY))==NULL)
				{
					log_print(LOGN_CRI, "** [%s][%s.%d]TCP_END. SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] hasho_find NULL",
						__FILE__, __FUNCTION__,__LINE__, szSrcIP, usSrcPort, szDestIP, usDestPort );

					return -2;
				}
				pSESS = (IV_SESS_DATA*)nifo_ptr(pHASH, pHASHNODE->offset_Data);
				/* *************************************************************************** */

				if((dRet = dCloseSession(pMEMSINFO, pHASH, pSESSKEY, pSESS)) < 0){
					log_print(LOGN_CRI, "** [%s][%s.%d]TCP_END. SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] dCloseSession Fail dRet[%d]",
						__FILE__,__FUNCTION__, __LINE__, szSrcIP, usSrcPort, szDestIP, usDestPort, dRet);
					return -3;
				}
				/* *************************************************************************** */

				if(dSend_Data(pMEMSINFO, dGetCALLSeqID(pSESSKEY->uiCliIP), pNode) < 0) {
					log_print(LOGN_CRI, "** [%s][%s.%d]TCP_END. SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] Fail SendSignal",
					    __FILE__,__FUNCTION__, __LINE__, szSrcIP, usSrcPort, szDestIP, usDestPort );

					return -11;
				}

				log_print(LOGN_DEBUG, "[%s][%s.%d]TCP_END. SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]",
						__FILE__,__FUNCTION__, __LINE__, szSrcIP, usSrcPort, szDestIP, usDestPort );

			}break;

		case DEF_TCP_DATA:
			{
				/**
				 * A_TCP에서 TCP Data 
				 **/
				/*****/
				if((pHASHNODE = hasho_find(pHASH, (U8*)pSESSKEY))==NULL)
				{
					 /* Session Hash를 찾지 못함 */
					log_print(LOGN_DEBUG, "** [%s][%s.%d]TCP_DATA. SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] hasho_find NULL",
						__FILE__, __FUNCTION__,__LINE__, szSrcIP, usSrcPort, szDestIP, usDestPort );
					return -4;
				}
				pSESS = (IV_SESS_DATA*)nifo_ptr(pHASH, pHASHNODE->offset_Data);
				/* *************************************************************************** */

				if((dRet = dProcMessage(pMEMSINFO, pHASH, pSESSKEY, pSESS, pTCPINFO, pNode, pDATA)) < 0){
					log_print(LOGN_CRI, "** [%s][%s.%d]TCP_DATA. SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] dProcMessage dRet[%d]",
						__FILE__,__FUNCTION__, __LINE__,  szSrcIP, usSrcPort, szDestIP, usDestPort, dRet);
//	 				 return -6;
				}

				nifo_node_delete(pMEMSINFO, pNode);

			}break;
							
		default:
			return -8;
	}

	return 0;
}

void MakeHashKey(TCP_INFO *pTCPINFO, IV_SESS_KEY *pSESSKEY)
{
	pSESSKEY->uiCliIP = pTCPINFO->uiCliIP;
	pSESSKEY->usCliPort = pTCPINFO->usCliPort;
	pSESSKEY->usReserved = 0;
}

U32 dMakeLOGInfo(stMEMSINFO *pMEMSINFO,IV_SESS_DATA *pSESS)
{
	U8 	*pLOGNODE;
	LOG_IV	*pLOG;

	if((pLOGNODE = nifo_node_alloc(pMEMSINFO)) ==NULL) {
		log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
		return -1;
	}
	if((pLOG = (LOG_IV *)nifo_tlv_alloc(pMEMSINFO, pLOGNODE, LOG_IV_DEF_NUM, LOG_IV_SIZE, DEF_MEMSET_ON))== NULL) {
		log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc NULL", __FUNCTION__, __LINE__);
		return -2;
	}

	if(pSESS->szLOG.uiPageEndTime == 0 ){
		if(pSESS->szLOG.uiPageFirstFailActionCode != eNOT_EXACT_SEQNUM ){
			//pSESS->szLOG.uiPageFirstFailActionCode = eNOT_PAGE_ENDTIME;
			if( pSESS->uiLastActionCode == 0 ){
				pSESS->szLOG.uiPageFirstFailActionCode = eNOT_REQUEST;
			}else{
				pSESS->szLOG.uiPageFirstFailActionCode = pSESS->uiLastActionCode;
			}
			pSESS->szLOG.uiPageActionFailCnt ++;
		}
	}
						
	memcpy(pLOG, &pSESS->szLOG, LOG_IV_SIZE);
//	LOG_IV_Prt("LOG_IV", pLOG);
//	PrintLOGINFO(pLOG);

	if(dSend_Data(pMEMSINFO, dGetCALLSeqID(pLOG->uiClientIP), pLOGNODE)<0) {
		log_print(LOGN_CRI, "[%s,%d] Send Fail!!",__FUNCTION__,__LINE__);
		return -3;
	}	
	return 0;
}


/** dCreateSession  function.
 * Internet Viewer 세션을 생성
 *	관련 데이터를 전송 및 정리
 * @param	*pMEMSINFO	: New Interface 관리를 위한 구조체
 * @param	*pHASH	: Internet Viewer Session을 관리를 위한 HASH
 * @param	*pTCPINFO	: TCP로 부터 받은 데이터의 정보를 담고 있는 구조체
 * @param	*pNode		: TCP로 부터 New Interface Node
 *
 * @return	S32
 * @see		iv_func.c iv_msgq.c iv_main.c iv_init.c iv_api.c
 *
 * @exception	nothing
 * @note		nothing
 **/
IV_SESS_DATA* pCreateSession(stMEMSINFO* pMEMSINFO, stHASHOINFO* pHASH, IV_SESS_KEY* pSESSKEY, TCP_INFO* pTCPINFO)
{
	IV_SESS_DATA	SESS;
	IV_SESS_DATA*	pSESS = &SESS;
	stHASHONODE*	pHASHNODE;

	memset(&pSESS->szLOG, 0x00, LOG_IV_SIZE);

	/* general session info */
 	pSESS->szLOG.uiCallTime = pTCPINFO->uiCapTime;  
 	pSESS->szLOG.uiCallMTime = pTCPINFO->uiCapMTime;

	pSESS->szLOG.uiClientIP = pTCPINFO->uiCliIP;
	pSESS->szLOG.usClientPort = pTCPINFO->usCliPort;
	pSESS->szLOG.uiServerIP = pTCPINFO->uiSrvIP;
	pSESS->szLOG.usServerPort = pTCPINFO->usSrvPort;

	pSESS->szLOG.usSvcL4Type = pTCPINFO->usL4Code;				
	pSESS->szLOG.usSvcL7Type = pTCPINFO->usL7Code ;
	pSESS->szLOG.usPlatformType = dGetPlatformType(pTCPINFO->usL4Code, pTCPINFO->usL7Code);;

//	pSESS->szLOG.uiNASName = pSESS->szLOG.uiClientIP;
	pSESS->szLOG.uiNASName = 0;
	memset(pSESS->szLOG.szBrowserInfo, 0x00, MAX_BROWSERINFO_LEN);
	memset(pSESS->szLOG.szModel, 0x00, MAX_MODEL_LEN);
	memset(pSESS->szLOG.szNetOption, 0x00, MAX_SVCOPTION_LEN);
	memset(pSESS->szLOG.szHostName, 0x00, MAX_HOSTNAME_LEN);
	pSESS->szLOG.usServiceType = 0;  /*FIXME. nowhere use this field. */

	/* pciv session info **********/
	memset(pSESS->szLOG.szMIN, 0x00, MAX_MIN_SIZE);
	memset(pSESS->szLOG.szPcode, 0x00, MAX_PCODE_SIZE);

	/* pciv page info ************/
	pSESS->bSequenceNumDiff = 0;
	pSESS->bGotFirstRequest = 0;
	pSESS->szLOG.uiPageID = 0;
	PageInit(pSESS);
	pSESS->szLOG.usPageFlag = 0; 

	pSESS->uiNextSeqDn = 0;
	pSESS->uiLastSeqDn = 0;
	pSESS->uiNextSeqUp = 0;
	pSESS->uiLastSeqUp = 0;


	//DEBUG
	//pSESS->uiPacketCntforTest = 0;

	/******************************/
	log_print(LOGN_DEBUG, "[%s][%s.%d] Call CreateSession", __FILE__, __FUNCTION__, __LINE__);

	if((pHASHNODE = hasho_add(pHASH, (U8*)pSESSKEY, (U8*)pSESS)) == NULL)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] hasho_add NULL", __FILE__, __FUNCTION__, __LINE__);
		return  NULL;
	}
	else{
		pSESS = (IV_SESS_DATA*)nifo_ptr(pHASH, pHASHNODE->offset_Data);
	}

	curSessCnt++;
	return pSESS;
}

/** dCloseSession  function.
 * TCP HASH를 삭제하고 Internet Viewer 세션을 정리
 *	관련 데이터를 전송 및 정리
 * @param	*pMEMSINFO	: New Interface 관리를 위한 구조체
 * @param	*pHASH		: Internet Viewer Session을 관리를 위한 HASH
 * @param	*pTCPINFO	: TCP로 부터 받은 데이터의 정보를 담고 있는 구조체
 * @param	*pNode		: TCP로 부터 New Interface Node
 *
 * @return	S32
 * @see		pciv_func.c pciv_main.c pciv_init.c pciv_api.c
 *
 * @exception	nothing
 * @note		nothing
 **/
S32 dCloseSession(stMEMSINFO* pMEMSINFO, stHASHOINFO* pHASH, IV_SESS_KEY* pSESSKEY, IV_SESS_DATA* pSESS)
{
	S32 dRet;
	if( pSESS->szLOG.usPageFlag != 3){
		pSESS->szLOG.usPageFlag = 3;
		pSESS->szLOG.uiPageID++;

		SetTime(pSESS, eEND_TIME);
		if((dRet=dMakeLOGInfo(pMEMSINFO,pSESS))<0){
			log_print(LOGN_CRI,"[%s][%s.%d] FAIL MAKE IV_LOG [%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		}
	}

	hasho_del(pHASH, (U8 *)pSESSKEY);

	curSessCnt--;
	return 0;
}

void PageInit(IV_SESS_DATA *pSESS)
{
	/* page *************/
	pSESS->szLOG.uiPageID++;
	memset(pSESS->szLOG.szPageURL, 0x00, MAX_URL_SIZE);
	memset(pSESS->szLOG.szBrowserInfo, 0x00, MAX_BROWSERINFO_LEN);
	pSESS->szLOG.uiPageActionCnt = 0 ;
	
	pSESS->szLOG.usPageFlag = 2; /* 0 : init,  1 : Start Page, 2 : Middle, 3 : End */

	pSESS->szLOG.uiPageActionFailCnt = 0 ;
	pSESS->szLOG.uiPageFirstFailActionCode = 0;

	pSESS->szLOG.uiPageDataUpPktSize = 0 ;
	pSESS->szLOG.uiPageDataDnPktSize = 0 ;

	pSESS->szLOG.uiPageStartTime = 0; 
	pSESS->szLOG.uiPageStartMTime = 0;
	pSESS->szLOG.uiPageEndTime = 0;
	pSESS->szLOG.uiPageEndMTime = 0;
	pSESS->szLOG.uiPageDataUpPktSize = 0;
	pSESS->szLOG.uiPageDataDnPktSize = 0;

	/*********************/
	pSESS->szLOG.uiClickTime  = 0; 
	pSESS->szLOG.uiClickMTime = 0; 

	pSESS->szLOG.uiNavigateTime  = 0; 
	pSESS->szLOG.uiNavigateMTime = 0; 

	pSESS->szLOG.uiIsCompletedTime  = 0; 
	pSESS->szLOG.uiIsCompletedMTime = 0; 

	pSESS->szLOG.uiGetAllheadersTime  = 0; 
	pSESS->szLOG.uiGetAllheadersMTime = 0; 
	
	pSESS->szLOG.uiGetChunkReqStartTime  = 0; 
	pSESS->szLOG.uiGetChunkReqStartMTime = 0; 

	pSESS->szLOG.uiGetChunkReqEndTime  = 0; 
	pSESS->szLOG.uiGetChunkReqEndMTime = 0; 

	pSESS->szLOG.uiGetChunkRespStartTime  = 0; 
	pSESS->szLOG.uiGetChunkRespStartMTime = 0; 

	pSESS->szLOG.uiGetChunkRespEndTime  = 0; 
	pSESS->szLOG.uiGetChunkRespEndMTime = 0; 

	pSESS->szLOG.uiLastAPITime  = 0; 
	pSESS->szLOG.uiLastAPIMTime = 0; 
	pSESS->szLOG.llTransDurationTime = 0;
	pSESS->llRequestTime = 0;

	pSESS->bFirstGetChunkTrain =0;
	pSESS->bIsCompleted = 0;
	pSESS->uiLastActionCode = 0;
	/*********************/
	
	gettimeofday(&stNowTime, NULL);
	pSESS->szLOG.uiOpStartTime = stNowTime.tv_sec;  /* record creation time of session */
	pSESS->szLOG.uiOpStartMTime = stNowTime.tv_usec;
	/********************/
	pSESS->bRequest = 1; 

	pSESS->uiCmdRemindSize = 0;
	pSESS->bNeedGetURLfromResponse = 0;
	pSESS->uiURLIdx = URL_START_IDX; 

	pSESS->uiBeforeLastAPITime = 0;
	pSESS->uiBeforeLastAPIMTime = 0;
	pSESS->uiBeforeClickTime = 0;
	pSESS->uiBeforeClickMTime = 0;
	pSESS->uiBeforeNavigateTime = 0;
	pSESS->uiBeforeNavigateMTime = 0;

	pSESS->uiLastResponseResult = 0; 
	/********************/
}

S32 dCheckSeqNum( IV_SESS_DATA* pSESS, TCP_INFO *pTCPINFO, unsigned int* puiNextSeq, unsigned int* puiLastSeq )
{
	U32 uiLastSeq;
	U32 uiNextSeq; 
	S32 dRet = 0;

	if( pTCPINFO->ucRtx == DEF_FROM_SERVER){
		uiNextSeq = pSESS->uiNextSeqDn;
		uiLastSeq = pSESS->uiLastSeqDn;
	}else{
		uiNextSeq = pSESS->uiNextSeqUp;
		uiLastSeq = pSESS->uiLastSeqUp;
	}

	if((uiNextSeq != 0) && (uiNextSeq != pTCPINFO->uiSeqNum)) {
		*puiNextSeq = uiNextSeq;
		*puiLastSeq = uiLastSeq;
	
		pSESS->szLOG.uiPageFirstFailActionCode = eNOT_EXACT_SEQNUM;
		pSESS->szLOG.uiPageActionFailCnt ++;
		
		dRet = -1;
	}

	if( pTCPINFO->ucRtx == DEF_FROM_SERVER){
		pSESS->uiNextSeqDn = pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize;
		pSESS->uiLastSeqDn = pTCPINFO->uiSeqNum;
	}else{
		pSESS->uiNextSeqUp = pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize;
		pSESS->uiLastSeqUp = pTCPINFO->uiSeqNum;
	}

	return dRet;
}

void AddTransDuration( TCP_INFO* pTCPINFO, IV_SESS_DATA* pSESS )
{
	if( pSESS->llRequestTime != 0 ){
		S64 tempDuration = (S64)pTCPINFO->uiAckTime*1000000 + (S64)pTCPINFO->uiAckMTime  - pSESS->llRequestTime ;

		if(tempDuration < 0 ){
			tempDuration = 0;	
		}
		pSESS->szLOG.llTransDurationTime += tempDuration; 
	}
	pSESS->llRequestTime = 0;
}

U32 dProcMessage(stMEMSINFO* pMEMSINFO, stHASHOINFO* pHASH, IV_SESS_KEY* pSESSKEY, IV_SESS_DATA* pSESS, TCP_INFO* pTCPINFO, U8* pNode, U8* pINPUTDATA)
{
	S32 dRet;
	U32 uiLastSeq;
	U32 uiNextSeq; 

	int	version;

	/*******/
	U8	szSrcIP[INET_ADDRSTRLEN];
	U8	szDestIP[INET_ADDRSTRLEN];
	unsigned short usSrcPort;
	unsigned short usDestPort;

	util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP);
	util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP);
	usSrcPort = pTCPINFO->usCliPort;
	usDestPort = pTCPINFO->usSrvPort;
	/*******/

	U8*	pDATA;
	pDATA = &pINPUTDATA[pTCPINFO->uiSOffset];
	/* PC Internet Viewer structure */
	PCIV_Packet_t Packet;
	PCIV_Header_t* pHeader = &(Packet.header);

	/* *************************************************************************** */
	/*pSESS->uiPacketCntforTest++;
	if( pSESS->uiPacketCntforTest == 80 ){
		printf("PacketCnt %d skipped\n", pSESS->uiPacketCntforTest); 
		return 0;
	}*/
	/* *************************************************************************** */

	if((dRet = dCheckSeqNum( pSESS, pTCPINFO, &uiNextSeq, &uiLastSeq )) < 0 ){
		log_print(LOGN_CRI, "** [%s][%s.%d]TCP_DATA. SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] DIFF SEQ NSEQ[%u]RSEQ[%u]LastSEQ[%u] STIME[%u.%d]RTX[%s]", 
			__FILE__,__FUNCTION__, __LINE__, szSrcIP, usSrcPort, szDestIP, usDestPort,
			uiNextSeq, pTCPINFO->uiSeqNum, uiLastSeq, pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime, PrintRtx(pTCPINFO->ucRtx));

		MakePageLOG(pSESS, pTCPINFO, pMEMSINFO, 0);
		pSESS->bSequenceNumDiff = 1;

		return 0; //FIXME. 에러임에도 '0'을 반환하는것이 맞을까?
	}
	/* *************************************************************************** */
	
	U32 uiDataSize = 0;
	if( pTCPINFO->ucRtx == DEF_FROM_CLIENT ){
		uiDataSize = pTCPINFO->uiDataSize;
		pSESS->szLOG.uiPageDataUpPktSize += pTCPINFO->uiDataSize;
	}else if( pTCPINFO->ucRtx == DEF_FROM_SERVER ){
		uiDataSize = pTCPINFO->uiDataSize;
		pSESS->szLOG.uiPageDataDnPktSize += pTCPINFO->uiDataSize;
	}

	version = 0;
	pciv_header((char*)pDATA, 8, &version);

//	if( strncmp("ZACO0101", pDATA, 8) != 0 ){
	if( version == 0 ){
		if( pSESS->bSequenceNumDiff == 1 ){
			if( pTCPINFO->ucRtx == DEF_FROM_CLIENT ){
				pSESS->szLOG.uiPageDataUpPktSize -= pTCPINFO->uiDataSize;
			}else if( pTCPINFO->ucRtx == DEF_FROM_SERVER ){
				pSESS->szLOG.uiPageDataDnPktSize -= pTCPINFO->uiDataSize;
			}
			return 0;
		}

		if( pSESS->bGotFirstRequest == 0){
			log_print(LOGN_DEBUG,"[%s][%s.%d]  not yet get Request", __FILE__, __FUNCTION__,__LINE__);
			return 0;
		}

		SetCaptureTime(pSESS, eLAST_API_TIME, pTCPINFO); 

		pSESS->uiCmdRemindSize -= uiDataSize;
		if( pSESS->uiCmdRemindSize <= 0 ){
			/*FIXME decide state */

			if( pTCPINFO->ucRtx == DEF_FROM_CLIENT){
				pSESS->bRequest = 0;
			}else{
				AddTransDuration(pTCPINFO, pSESS );
				pSESS->bRequest = 1; /* 다음번에는 request를 받을 차례임을 표시. */
			}

			pSESS->bFirstGetChunkTrain = 0;
		}
		
 		if( pTCPINFO->ucRtx == DEF_FROM_SERVER ){

			if( pSESS->bFirstGetChunkTrain == 1 ){
				SetCaptureTime(pSESS, eGETCHUNK_RESP_END_TIME, pTCPINFO);
			}

			if( pSESS->bNeedGetURLfromResponse == 1){
				// Get URL 
				if(uiDataSize >= pSESS->uiURLIdx ){
					int iTitilSize = 0;
					int iMaxLength  = pSESS->uiURLIdx + 4; // 8byte skip, 4byte : title size
					for( ; pSESS->uiURLIdx < iMaxLength ; pSESS->uiURLIdx++){
						iTitilSize = iTitilSize << 8;
						iTitilSize = iTitilSize | (0x000000ff & pDATA[pSESS->uiURLIdx]);
					}
					int iURLSize = 0;
					pSESS->uiURLIdx += iTitilSize + 8; // title skip,  8byte skip
					iMaxLength = pSESS->uiURLIdx + 4;  // 4byte : url size
					if( uiDataSize >= iMaxLength ){
						for( ; pSESS->uiURLIdx < iMaxLength ; pSESS->uiURLIdx++){
							iURLSize = iURLSize << 8;
							iURLSize = iURLSize | (0x000000ff & pDATA[pSESS->uiURLIdx]);
						}
						if( uiDataSize >= pSESS->uiURLIdx + iURLSize ){
							if(iURLSize >= MAX_URL_LEN ){
								memcpy(pSESS->szLOG.szPageURL, pDATA + pSESS->uiURLIdx,  MAX_URL_LEN);
							}else{
								memcpy(pSESS->szLOG.szPageURL, pDATA + pSESS->uiURLIdx,  iURLSize);
							}
						}
					}
				}
				pSESS->bNeedGetURLfromResponse = 0;
			}
		}else{
			if( pSESS->bFirstGetChunkTrain == 1 ){
				SetCaptureTime(pSESS, eGETCHUNK_REQ_END_TIME, pTCPINFO);
			}
		}
		
		return 0;

	}else{

		switch(version)
		{
		case 100:
log_print(LOGN_CRI, "dark264sh GET VER 0100");
return 0;
			if(dGetHeaderFromVersion0100(pSESS, &Packet, uiDataSize, pDATA) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d VER0101", __FILE__, __FUNCTION__, __LINE__);
				return -14;
			}
			break;
		case 101:
			if(dGetHeaderFromVersion0101(pSESS, &Packet, uiDataSize, pDATA) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d VER0101", __FILE__, __FUNCTION__, __LINE__);
				return -12;
			}
			break;
		case 102:
log_print(LOGN_CRI, "dark264sh GET VER 0102 SIZE=%d", uiDataSize);
return 0;
			if(dGetHeaderFromVersion0102(pSESS, &Packet, uiDataSize, pDATA) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d VER0101", __FILE__, __FUNCTION__, __LINE__);
				return -13;
			}
			break;
		default:
			log_print(LOGN_CRI, "F=%s:%s.%d NOT SUPPORT VERSION=%04d CMD=%.*s",
				__FILE__, __FUNCTION__, __LINE__, version, 8, pDATA);
			return -11;
		}

		log_print(LOGN_DEBUG, "HEADER VER=%04d RESULT=%d CMDSIZE=%d CONTENTSIZE=%ld", 
			version, pHeader->result, pHeader->cmdSize, pHeader->contentSize);

		if((pHeader->cmdSize > 50000) || (pHeader->contentSize > 1000000)) {
			log_print(LOGN_CRI, "over size cmdsize=%d contentsize=%ld", pHeader->cmdSize, pHeader->contentSize);
			return -15;
		}

		pSESS->bGotFirstRequest = 1;

		SetCaptureTime(pSESS, eLAST_API_TIME, pTCPINFO); 
		sprintf((char*)pSESS->szLOG.szBrowserInfo, "%04d", version);

	}

	if( pTCPINFO->ucRtx == DEF_FROM_CLIENT ) {
		/* request */
//		Packet.command=(unsigned char*)(pDATA + PCIV_HDR_SIZE);
//		Packet.content=pDATA + PCIV_HDR_SIZE + pHeader->cmdSize;

		log_print(LOGN_DEBUG,"[%s][%s.%d] command[%.*s] ", __FILE__, __FUNCTION__,__LINE__,
				   pHeader->cmdSize, Packet.command);

		/* parse command */
		CmdEnum_t dCmd = 0;
		int iLastIdx = 0;
		iLastIdx = parseCommand( &Packet, &dCmd );

		int iArgLength = 0;
		switch(dCmd){
		/******************************************/
			case eClickEx:
//log_print(LOGN_CRI, "dark264sh GET ClickEx");
			case eClick :
				{
					SetCaptureTime(pSESS, eCLICK_TIME, pTCPINFO);
				}break;
			case eNavigate :
				{
					SetCaptureTime(pSESS, eNAVIGATE_TIME, pTCPINFO);
				}break;
			case eClose:
			case eExit:
				{
					pSESS->szLOG.usPageFlag = 3;
				}
			case eWaitCompleted:
//log_print(LOGN_CRI, "dark264sh GET WaitCompleted");
			case eIsCompleted:
				{
					if( pSESS->bSequenceNumDiff == 1){
						pSESS->bSequenceNumDiff = 0;
					}
					MakePageLOG(pSESS, pTCPINFO, pMEMSINFO, dCmd);
				}break;
			case eGetAllheaders :
				{
					if( pSESS->bSequenceNumDiff == 0){
						SetCaptureTime(pSESS, eGETALLHEADERS_TIME, pTCPINFO);

						pSESS->uiURLIdx = URL_START_IDX; 
						pSESS->bNeedGetURLfromResponse = 1;
					}else{
						if( pTCPINFO->ucRtx == DEF_FROM_CLIENT ){
							pSESS->szLOG.uiPageDataUpPktSize -= pTCPINFO->uiDataSize;
						}else if( pTCPINFO->ucRtx == DEF_FROM_SERVER ){
							pSESS->szLOG.uiPageDataDnPktSize -= pTCPINFO->uiDataSize;
						}
						return 0;
					}

				}break;
			case eGetChunkTrain:
				{
					if( pSESS->bSequenceNumDiff == 0 ){
						if( pSESS->szLOG.uiGetChunkReqStartTime == 0 ){
							SetCaptureTime(pSESS, eGETCHUNK_REQ_START_TIME, pTCPINFO);
							SetCaptureTime(pSESS, eGETCHUNK_REQ_END_TIME, pTCPINFO);
							pSESS->bFirstGetChunkTrain = 1;
						}
					}else{
						if( pTCPINFO->ucRtx == DEF_FROM_CLIENT ){
							pSESS->szLOG.uiPageDataUpPktSize -= pTCPINFO->uiDataSize;
						}else if( pTCPINFO->ucRtx == DEF_FROM_SERVER ){
							pSESS->szLOG.uiPageDataDnPktSize -= pTCPINFO->uiDataSize;
						}
						return 0;
					}

				}break;
			case eCheckAndVersion:
				{
					/*TODO szMIN 값을 구할 수 있다. */
					getArgument( &Packet, iLastIdx, &iArgLength ); 
					if( iArgLength > MAX_MIN_SIZE ){
							log_print(LOGN_CRI,"[%s][%s.%d] too large argument size(%d) to parse MIN code",
								   	__FILE__, __FUNCTION__,__LINE__, iArgLength );
					}else{
						memcpy(pSESS->szLOG.szMIN, Packet.command + iLastIdx , iArgLength);
					}
				}break;
			case eOpen :
			case eIsOpened :
			case eObjectClick :

			case eMouseOver :
			case eGoForward:
			case eGoBack:
			case eStop:

			case eCheckRectImage:
			case eGetRectImage:
			case eGetFullImage:
			case eGetObjectImage:

			case eGetObjectList:
			case eSetChunk:
			case eGetThumbnailImage:

			case eIsChangedDocument:

			case eGetAnchorList:
			case eGetCookies:
			case eGetTitle:
			case eGetTicket:
			case eInitTicket:

			case eGetInputData:
			case eSetInputData:
			case eSetSelectData:

			case eGetLoginItems:
			case eSetLoginItems:

			case eGetFontSize:
			case eSetFontSize:

			case eRefresh:
			case eGetFullSize:
			case eGetActiveWindow:
			case eSetActiveWindow:
			case eSetPopupWindow:
			case eExecuteSnapshot:
			case eThisIsJustPing:
			case eSetLCDSize:

			case eGetCurrentUrl:
			case eGetExViewURL:
			case eGetFlvUrl:
			case eGetServerList:
			case eGetServerInfo:
			case eGetBrokerMessage:
			case eGetText:
			case eMouseDrag:
			case eRegionCheck:
			case eMouseOverEx:
			case eOpenEx:
			case eSetTicketDuration:
			case eSendXCData:
			case eGetChunkTrainForce:
			case eEvAppWithURL:
			case eEvChangeSize:
			case eEvChangeURL:
			case eEvAlert:
			case eEvPopupWindow:
			case eEvXCExecute:
			case eCheckRectImageEx:
				{
					if(pSESS->bSequenceNumDiff == 1){
						if( pTCPINFO->ucRtx == DEF_FROM_CLIENT ){
							pSESS->szLOG.uiPageDataUpPktSize -= pTCPINFO->uiDataSize;
						}else if( pTCPINFO->ucRtx == DEF_FROM_SERVER ){
							pSESS->szLOG.uiPageDataDnPktSize -= pTCPINFO->uiDataSize;
						}
						return 0;
					}
				}break;	
			default :
				{
					log_print(LOGN_CRI,"[%s][%s.%d] fail to parse dCmd [%d], command[%.*s]",
							__FILE__, __FUNCTION__, __LINE__, dRet, pHeader->cmdSize, Packet.command);

					dCmd = pSESS->uiLastActionCode; // 엉뚱한 값이 들어갈 수 있으므로, 이전 ActionCode를 넣는다.

					if( pTCPINFO->ucRtx == DEF_FROM_CLIENT ){
						pSESS->szLOG.uiPageDataUpPktSize -= pTCPINFO->uiDataSize;
					}else if( pTCPINFO->ucRtx == DEF_FROM_SERVER ){
						pSESS->szLOG.uiPageDataDnPktSize -= pTCPINFO->uiDataSize;
					}
				}break;
		}

		pSESS->llRequestTime = (S64)pTCPINFO->uiCapTime*1000000 + (S64)pTCPINFO->uiCapMTime;

		pSESS->uiLastActionCode = dCmd;
		pSESS->szLOG.uiPageActionCnt ++;
		if( pSESS->bRequest != 1 ){
			if( pSESS->szLOG.uiPageFirstFailActionCode == 0 ){
				if( pSESS->uiLastActionCode == 0 ){
					pSESS->szLOG.uiPageFirstFailActionCode = eNOT_REQUEST;
				}else{
					pSESS->szLOG.uiPageFirstFailActionCode = pSESS->uiLastActionCode;
				}
			}
			pSESS->szLOG.uiPageActionFailCnt ++;
			log_print(LOGN_CRI,"[%s][%s.%d] PageActionFail. request command[%d]",__FILE__, __FUNCTION__, __LINE__, dCmd);
		}else{
			/* 더 받을 Data가 없으므로, 다음번엔 response를 받아야 한다. */
			if( pSESS->uiCmdRemindSize == 0 ){
				pSESS->bRequest = 0;
			}
		}

	}else if(pTCPINFO->ucRtx == DEF_FROM_SERVER ){
		/* response */
		if( pSESS->bSequenceNumDiff == 1){
			if( pTCPINFO->ucRtx == DEF_FROM_CLIENT ){
				pSESS->szLOG.uiPageDataUpPktSize -= pTCPINFO->uiDataSize;
			}else if( pTCPINFO->ucRtx == DEF_FROM_SERVER ){
				pSESS->szLOG.uiPageDataDnPktSize -= pTCPINFO->uiDataSize;
			}
			return 0;
		}

		pSESS->uiLastResponseResult = pHeader->result;

		if((pSESS->uiLastActionCode == eIsCompleted || pSESS->uiLastActionCode == eWaitCompleted) && pSESS->uiLastResponseResult != 0){
			pSESS->bIsCompleted = 1;	
		}
			
		if( pSESS->bFirstGetChunkTrain == 1 && pSESS->szLOG.uiGetChunkRespStartTime == 0){
			SetCaptureTime(pSESS, eGETCHUNK_RESP_START_TIME, pTCPINFO);
			SetCaptureTime(pSESS, eGETCHUNK_RESP_END_TIME, pTCPINFO);
		}

		if( pSESS->bRequest != 0 ){
			if( pSESS->szLOG.uiPageFirstFailActionCode == 0 ){
				pSESS->szLOG.uiPageFirstFailActionCode = eNOT_REQUEST;
			}
			pSESS->szLOG.uiPageActionFailCnt ++;
			log_print(LOGN_CRI,"[%s][%s.%d] PageActionFail. response",__FILE__, __FUNCTION__, __LINE__);

		}else{
			/* 더 받을 Data가 없으므로, 다음번엔 request를 받아야 한다. */
			if( pSESS->uiCmdRemindSize == 0 ){
				AddTransDuration(pTCPINFO, pSESS );
				pSESS->bRequest = 1;
			}
		}

		if( pSESS->bNeedGetURLfromResponse == 1){
				pSESS->bRequest = 1;
		}

	}else{
		log_print(LOGN_CRI, "[%s][%s,%d] INVALID RTX FLAG[%d]", __FILE__, __FUNCTION__,__LINE__,pTCPINFO->ucRtx);
		return -2;
	}
	return 0;
}

void MakePageLOG( IV_SESS_DATA* pSESS, TCP_INFO* pTCPINFO, stMEMSINFO* pMEMSINFO, int cmd)
{
	//FIXME. IsCompleted의 response->result 에 상관없이, 처음 불려진 IsCompleted 를 사용한다.
	U8    szSIP[INET_ADDRSTRLEN];
	S32	dRet;
	unsigned short usAutoRefresh = 0;	

	STIME tempTime = 0;
	MTIME tempMTime = 0;

	if( pSESS->uiLastActionCode == eClick ){
		SetCaptureTime(pSESS, eBEFORE_LAST_API_TIME, pTCPINFO);
		
		tempTime = pSESS->szLOG.uiClickTime;
		tempMTime = pSESS->szLOG.uiClickMTime;

		if( pSESS->uiBeforeClickTime != 0 ){
			pSESS->szLOG.uiClickTime = pSESS->uiBeforeClickTime; 
			pSESS->szLOG.uiClickMTime = pSESS->uiBeforeClickMTime;

			pSESS->szLOG.uiNavigateTime = 0;
			pSESS->szLOG.uiNavigateMTime = 0;
		}
	}else if(pSESS->uiLastActionCode == eNavigate ){
		SetCaptureTime(pSESS, eBEFORE_LAST_API_TIME, pTCPINFO);

		tempTime = pSESS->szLOG.uiNavigateTime;
		tempMTime = pSESS->szLOG.uiNavigateMTime;
		
		if(pSESS->uiBeforeNavigateTime != 0 ){
			pSESS->szLOG.uiNavigateTime = pSESS->uiBeforeNavigateTime; 
			pSESS->szLOG.uiNavigateMTime = pSESS->uiBeforeNavigateMTime;

			pSESS->szLOG.uiClickTime = 0;
			pSESS->szLOG.uiClickMTime = 0;
		}
	}else if(pSESS->uiLastActionCode == eIsChangedDocument && pSESS->uiLastResponseResult == 1 ){
		usAutoRefresh = 1;
	}

	if( pSESS->szLOG.usPageFlag == 0 ){
		pSESS->szLOG.usPageFlag = 1;

		log_print(LOGN_CRI,"SIP[%s] SPORT[%u] First Page Start",
			 util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort );
	}else{
		if( pSESS->bIsCompleted == 1){
			if( pSESS->szLOG.usPageFlag == 3){
				pSESS->szLOG.uiPageID++;
				if( pSESS->szLOG.uiPageStartTime == pSESS->uiBeforeClickTime 
					&& pSESS->szLOG.uiPageStartMTime == pSESS->uiBeforeClickMTime ){

					pSESS->szLOG.uiClickTime = pSESS->uiBeforeClickTime; 
					pSESS->szLOG.uiClickMTime = pSESS->uiBeforeClickMTime;
				}else if(pSESS->szLOG.uiPageStartTime == pSESS->uiBeforeNavigateTime 
					&& pSESS->szLOG.uiPageStartMTime == pSESS->uiBeforeNavigateMTime ){

					pSESS->szLOG.uiNavigateTime = pSESS->uiBeforeNavigateTime; 
					pSESS->szLOG.uiNavigateMTime = pSESS->uiBeforeNavigateMTime;
				}
			}

			SetTime(pSESS, eEND_TIME);
			if(( dRet = dMakeLOGInfo(pMEMSINFO,pSESS) )<0){
				log_print(LOGN_CRI,"[%s][%s.%d] FAIL MAKE IV_LOG [%d]",
						__FILE__, __FUNCTION__, __LINE__, dRet);
			}

			if( pSESS->szLOG.usPageFlag == 3){
				return; //break;
			}
			/***************************/
			PageInit(pSESS); 
			if( usAutoRefresh == 1 ){
				pSESS->szLOG.usPageFlag = 4;

				log_print(LOGN_CRI,"SIP[%s] SPORT[%u] Refresh Page[%d] Start",
				 util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,pSESS->szLOG.uiPageID);
			}else{
				log_print(LOGN_CRI,"SIP[%s] SPORT[%u] Middle Page[%d] Start",
				 util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,pSESS->szLOG.uiPageID);
			}
		}
	}

	SetTime(pSESS, eSTART_TIME);
	SetCaptureTime(pSESS, eISCOMPLETED_TIME, pTCPINFO);
	if(cmd == eWaitCompleted) {
		pSESS->szLOG.uiLastAPITime = pTCPINFO->uiCapTime;
		pSESS->szLOG.uiLastAPIMTime = pTCPINFO->uiCapMTime;
	}

	if( tempTime != 0  ){
		pSESS->szLOG.uiPageStartTime = tempTime;
		pSESS->szLOG.uiPageStartMTime = tempMTime;

		if( pSESS->uiLastActionCode == eClick ){
			pSESS->uiBeforeClickTime = tempTime;
			pSESS->uiBeforeClickMTime = tempMTime;
		}else if( pSESS->uiLastActionCode == eNavigate ){
			pSESS->uiBeforeNavigateTime = tempTime;
			pSESS->uiBeforeNavigateMTime = tempMTime;
		}
	}else if(pSESS->szLOG.uiPageStartTime == 0){
		SetCaptureTime(pSESS, eSTART_TIME, pTCPINFO );
	}
}

void getArgument( PCIV_Packet_t* pPacket, int iLastIdx, int* iArgLength )
{
	unsigned char*	command = pPacket->command;
	int iMaxIdx = pPacket->header.cmdSize - iLastIdx;
	int idx = iLastIdx;
	for( ; idx < iMaxIdx ; idx++){
		if( command[idx] == ',' ){
			*iArgLength = idx - iLastIdx;
			break;
		}
	}	
}

void SetCaptureTime( IV_SESS_DATA* pSESS, U16 usTimeType, TCP_INFO* pTCPINFO )
{
	switch(usTimeType){
		case eSTART_TIME:
			{
				pSESS->szLOG.uiPageStartTime  = pTCPINFO->uiCapTime; 
				pSESS->szLOG.uiPageStartMTime = pTCPINFO->uiCapMTime; 
			}break;
//		case eEND_TIME:
//			{
//				pSESS->szLOG.uiPageEndTime  = pTCPINFO->uiAckTime; 
//				pSESS->szLOG.uiPageEndMTime = pTCPINFO->uiAckMTime; 
//			}break;
		case eCLICK_TIME:
			{
				pSESS->szLOG.uiClickTime  = pTCPINFO->uiCapTime; 
				pSESS->szLOG.uiClickMTime = pTCPINFO->uiCapMTime; 
			}break; 
		case eNAVIGATE_TIME:
			{
				pSESS->szLOG.uiNavigateTime  = pTCPINFO->uiCapTime; 
				pSESS->szLOG.uiNavigateMTime = pTCPINFO->uiCapMTime; 
			}break;
		case eISCOMPLETED_TIME:
			{
				pSESS->szLOG.uiIsCompletedTime  = pTCPINFO->uiCapTime; 
				pSESS->szLOG.uiIsCompletedMTime = pTCPINFO->uiCapMTime; 
			}break;
		case eGETALLHEADERS_TIME:
			{
				pSESS->szLOG.uiGetAllheadersTime  = pTCPINFO->uiCapTime; 
				pSESS->szLOG.uiGetAllheadersMTime = pTCPINFO->uiCapMTime; 
			}break;
		case eGETCHUNK_REQ_START_TIME:
			{
				pSESS->szLOG.uiGetChunkReqStartTime  = pTCPINFO->uiCapTime; 
				pSESS->szLOG.uiGetChunkReqStartMTime = pTCPINFO->uiCapMTime; 
			}break;
		case eGETCHUNK_REQ_END_TIME:
			{
				pSESS->szLOG.uiGetChunkReqEndTime  = pTCPINFO->uiAckTime; 
				pSESS->szLOG.uiGetChunkReqEndMTime = pTCPINFO->uiAckMTime; 
			}break;
		case eGETCHUNK_RESP_START_TIME:
			{
				pSESS->szLOG.uiGetChunkRespStartTime  = pTCPINFO->uiCapTime; 
				pSESS->szLOG.uiGetChunkRespStartMTime = pTCPINFO->uiCapMTime; 
			}break;
		case eGETCHUNK_RESP_END_TIME:
			{
				// eEND_TIME과 같은 값 
				pSESS->szLOG.uiGetChunkRespEndTime  = pTCPINFO->uiAckTime; 
				pSESS->szLOG.uiGetChunkRespEndMTime = pTCPINFO->uiAckMTime; 

				pSESS->szLOG.uiPageEndTime  = pTCPINFO->uiAckTime; 
				pSESS->szLOG.uiPageEndMTime = pTCPINFO->uiAckMTime; 
			}break;
#if 0
		/* 2009.03.02 dark264sh WaitCompleted 시간을 szLOG->uiLastAPITime 넣어주기 위함 */
		case eLAST_API_TIME:
			{
				pSESS->uiBeforeLastAPITime = pSESS->szLOG.uiLastAPITime; 
				pSESS->uiBeforeLastAPIMTime = pSESS->szLOG.uiLastAPIMTime; 

				pSESS->szLOG.uiLastAPITime  = pTCPINFO->uiAckTime; 
				pSESS->szLOG.uiLastAPIMTime = pTCPINFO->uiAckMTime; 
			}break;
		case eBEFORE_LAST_API_TIME:
			{
				pSESS->szLOG.uiLastAPITime = pSESS->uiBeforeLastAPITime; 
				pSESS->szLOG.uiLastAPIMTime = pSESS->uiBeforeLastAPIMTime; 
			}break;
#endif
		default:
			break;
	}
}	


void SetTime( IV_SESS_DATA* pSESS, U16 usTimeType )
{
	gettimeofday(&stNowTime, NULL);

	if( usTimeType == eSTART_TIME ){
		pSESS->szLOG.uiOpStartTime = stNowTime.tv_sec;
		pSESS->szLOG.uiOpStartMTime = stNowTime.tv_usec;
	}else if(usTimeType == eEND_TIME){
		pSESS->szLOG.uiOpEndTime = stNowTime.tv_sec;
		pSESS->szLOG.uiOpEndMTime = stNowTime.tv_usec;
	}
}	

void PrintLOGINFO(LOG_IV* pLOG)
{
	U8              szSIP[INET_ADDRSTRLEN];
	U8              szDIP[INET_ADDRSTRLEN];
	log_print(LOGN_DEBUG,"******* IV LOGINFO ***********");
	log_print(LOGN_DEBUG, "SIP[%s] SPORT[%u] DIP[%s] DPORT[%u]",
			util_cvtipaddr(szSIP, pLOG->uiClientIP), pLOG->usClientPort, 
			util_cvtipaddr(szDIP, pLOG->uiServerIP), pLOG->usServerPort);

	log_print(LOGN_DEBUG,"CallTime---------[%u][%u]",pLOG->uiCallTime,pLOG->uiCallMTime);
	log_print(LOGN_DEBUG,"NASNAME----------[%u]",pLOG->uiNASName);
	log_print(LOGN_DEBUG,"BrowserInfo------[%s]",pLOG->szBrowserInfo);
	log_print(LOGN_DEBUG,"Model------------[%s]",pLOG->szModel);
	log_print(LOGN_DEBUG,"NetOption--------[%s]",pLOG->szNetOption);
	log_print(LOGN_DEBUG,"SvcL4Type--------[%ld]",pLOG->usSvcL4Type);
	log_print(LOGN_DEBUG,"SvcL7Type--------[%ld]",pLOG->usSvcL7Type);
	log_print(LOGN_DEBUG,"MIN--------------[%s]",pLOG->szMIN);
	log_print(LOGN_DEBUG,"PageID-----------[%d]",pLOG->uiPageID);
	log_print(LOGN_DEBUG,"PageURL----------[%s]",pLOG->szPageURL);
	log_print(LOGN_DEBUG,"PageFlag----------[%u]",pLOG->usPageFlag);
	log_print(LOGN_DEBUG,"PageFirstFailActionCode----------[%u]",pLOG->uiPageFirstFailActionCode);

	log_print(LOGN_DEBUG,"****** [TIME] *******");
	log_print(LOGN_DEBUG,"PageStartTime---------[%u][%u]",pLOG->uiPageStartTime,pLOG->uiPageStartMTime);
	log_print(LOGN_DEBUG,"ClickTime-------------[%u][%u]",pLOG->uiClickTime,pLOG->uiClickMTime);
	log_print(LOGN_DEBUG,"NavigateTime----------[%u][%u]",pLOG->uiNavigateTime,pLOG->uiNavigateMTime);
	log_print(LOGN_DEBUG,"IsCompletedTime-------[%u][%u]",pLOG->uiIsCompletedTime,pLOG->uiIsCompletedMTime);
	log_print(LOGN_DEBUG,"GetAllheadersTime-----[%u][%u]",pLOG->uiGetAllheadersTime,pLOG->uiGetAllheadersMTime);
	log_print(LOGN_DEBUG,"GetChunkReqStartTime--[%u][%u]",pLOG->uiGetChunkReqStartTime,pLOG->uiGetChunkReqStartMTime);
	log_print(LOGN_DEBUG,"GetChunkReqEndTime----[%u][%u]",pLOG->uiGetChunkReqEndTime,pLOG->uiGetChunkReqEndMTime);
	log_print(LOGN_DEBUG,"GetChunkRespStartTime-[%u][%u]",pLOG->uiGetChunkRespStartTime,pLOG->uiGetChunkRespStartMTime);
	log_print(LOGN_DEBUG,"GetChunkRespEndTime---[%u][%u]",pLOG->uiGetChunkRespEndTime,pLOG->uiGetChunkRespEndMTime);
	log_print(LOGN_DEBUG,"PageEndTime-----------[%u][%u]",pLOG->uiPageEndTime,pLOG->uiPageEndMTime);
	log_print(LOGN_DEBUG,"LastAPITime-----------[%u][%u]",pLOG->uiLastAPITime,pLOG->uiLastAPIMTime);
	log_print(LOGN_DEBUG,"llTransDurationTime---[%lld]",pLOG->llTransDurationTime);
	log_print(LOGN_DEBUG,"OpStartTime-----------[%u][%u]",pLOG->uiOpStartTime,pLOG->uiOpStartMTime);
	log_print(LOGN_DEBUG,"OpEndTime-------------[%u][%u]",pLOG->uiOpEndTime,pLOG->uiOpEndMTime);

	log_print(LOGN_DEBUG,"***** [COUNT & SIZE] *****");
	log_print(LOGN_DEBUG,"PageActionCnt-------[%u]",pLOG->uiPageActionCnt);
	log_print(LOGN_DEBUG,"PageActionFailCnt-------[%u]",pLOG->uiPageActionFailCnt);
	log_print(LOGN_DEBUG,"PageDataPktSize-------UP[%u] DOWN[%u]",pLOG->uiPageDataUpPktSize,pLOG->uiPageDataDnPktSize);
	log_print(LOGN_DEBUG,"***************************************************************************");
}

int parseCommand(PCIV_Packet_t* pPacket, CmdEnum_t* dCmd )
{
	int cmd = 0, last = 0;

	if(pciv_parse((char*)pPacket->command, pPacket->header.cmdSize, &cmd, &last) != 0) {
		log_print(LOGN_CRI, "F=%s:%s.%d fail parse len=%d:%.*s", 
			__FILE__, __FUNCTION__, __LINE__, pPacket->header.cmdSize, pPacket->header.cmdSize, pPacket->command);	
		last = -5;
	}

	*dCmd = cmd;
	return last;
}

U8 *PrintRtx(U8 ucRtxType)
{
	return (U8*)((ucRtxType == DEF_FROM_SERVER) ? "FROM_SERVER" : "FROM_CLIENT");
}

#if DEBUG_MODE
S32 RcvPacketInfo(TCP_INFO *pTCPINFO)
{
	U8	szSIP[INET_ADDRSTRLEN];
	U8	szDIP[INET_ADDRSTRLEN];

	log_print(LOGN_INFO,"************RECIVE PACKET INFO****************");
	log_print(LOGN_DEBUG,"TCPFlag[%u]",pTCPINFO->cTcpFlag);
    log_print(LOGN_DEBUG," SIP[%s] SPORT[%u]DIP[%s]DPORT[%u]",
	         util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort, util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
	log_print(LOGN_INFO,"Data Cnt UP[%u] Dn[%u] Tot Cnt UP[%u] DN[%u]",
			pTCPINFO->uiIPDataUpPktCnt,pTCPINFO->uiIPDataDnPktCnt, pTCPINFO->uiIPTotUpPktCnt,pTCPINFO->uiIPTotDnPktCnt);
	log_print(LOGN_INFO,"Data Size Up[%u] Dn[%u] Tot Cnt Up[%u] Dn [%u]",
			pTCPINFO->uiIPDataUpPktSize,pTCPINFO->uiIPDataDnPktSize, pTCPINFO->uiIPTotUpPktSize,pTCPINFO->uiIPTotDnPktSize);
	log_print(LOGN_INFO,"**********************************************");
	return 0;
}
#endif


int dGetHeaderFromVersion0101(IV_SESS_DATA *pSESS, PCIV_Packet_t *pPCIV, U32 uiDataSize, U8 *pDATA)
{
	int			index = 0, size;
	int			cvttmp32;
	long long	cvttmp64;

	if(uiDataSize < PCIV_HDR_SIZE)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d too short data size=%d to parse header",
			__FILE__, __FUNCTION__, __LINE__, uiDataSize);
		return -2;
	}

	/* ver=0101 ResultCode index=8 size=4 */
	index = 8;
	size = 4;
	memcpy(&cvttmp32, &pDATA[index], size); 
	pPCIV->header.result = ntohl(cvttmp32);
	index += size;	

	/* ver=0101 CommandSize index=24 size=4 */
	index += 12;		/* Reserved1 size=4, Reserved2 size=4, Reserved3 size=4 */
	size = 4;
	memcpy(&cvttmp32, &pDATA[index], size); 
	pPCIV->header.cmdSize = ntohl(cvttmp32);
	index += size;	
		
	/* ver=0101 ContentSize index=28 size=8 */
	size = 8;
	memcpy(&cvttmp64, &pDATA[index], size); 
	pPCIV->header.contentSize = util_cvtint64(cvttmp64);	

	pSESS->uiCmdRemindSize = pPCIV->header.cmdSize + pPCIV->header.contentSize - (uiDataSize - PCIV_HDR_SIZE);

	pPCIV->command = pDATA + PCIV_HDR_SIZE;
	pPCIV->content = pDATA + PCIV_HDR_SIZE + pPCIV->header.cmdSize;

	return 0;
}

int dGetHeaderFromVersion0102(IV_SESS_DATA *pSESS, PCIV_Packet_t *pPCIV, U32 uiDataSize, U8 *pDATA)
{
	int			index = 0, size;
	int			cvttmp32;

	if(uiDataSize < PCIV_HDR_SIZE)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d too short data size=%d to parse header",
			__FILE__, __FUNCTION__, __LINE__, uiDataSize);
		return -2;
	}

	/* ver=0102 ResultCode index=12 size=4 */
	index = 12;
	size = 4;
	memcpy(&cvttmp32, &pDATA[index], size); 
	pPCIV->header.result = ntohl(cvttmp32);
	index += size;	

	/* ver=0102 CommandSize index=28 size=4 */
	index += 12;		/* Reserved1 size=4, Reserved2 size=4, Reserved3 size=4 */
	size = 4;
	memcpy(&cvttmp32, &pDATA[index], size); 
	pPCIV->header.cmdSize = ntohl(cvttmp32);
	index += size;	
		
	/* ver=0102 ContentSize index=32 size=4 */
	size = 4;
	memcpy(&cvttmp32, &pDATA[index], size); 
	pPCIV->header.contentSize = ntohl(cvttmp32);
	index += size;	

	pSESS->uiCmdRemindSize = pPCIV->header.cmdSize + pPCIV->header.contentSize - (uiDataSize - PCIV_HDR_SIZE);

	pPCIV->command = pDATA + PCIV_HDR_SIZE;
	pPCIV->content = pDATA + PCIV_HDR_SIZE + pPCIV->header.cmdSize;

	return 0;
}

int dGetHeaderFromVersion0100(IV_SESS_DATA *pSESS, PCIV_Packet_t *pPCIV, U32 uiDataSize, U8 *pDATA)
{
	int			index = 0, size;
	int			cvttmp32;
	long long	cvttmp64;

	if(uiDataSize < PCIV_HDR_SIZE)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d too short data size=%d to parse header",
			__FILE__, __FUNCTION__, __LINE__, uiDataSize);
		return -2;
	}

	pPCIV->header.result = 0;

	/* ver=0100 CommandSize index=8 size=4 */
	index = 8;		/* Reserved1 size=4, Reserved2 size=4, Reserved3 size=4 */
	size = 4;
	memcpy(&cvttmp32, &pDATA[index], size); 
	pPCIV->header.cmdSize = ntohl(cvttmp32);
	index += size;	
	pPCIV->command = pDATA + index;

	index += pPCIV->header.cmdSize;

	if(uiDataSize >= index + 8) 
	{
		/* ver=0101 ContentSize index=28 size=8 */
		size = 8;
		memcpy(&cvttmp64, &pDATA[index], size); 
		pPCIV->header.contentSize = util_cvtint64(cvttmp64);	
		index += size;
		pPCIV->content = pDATA + index;
	}

	pSESS->uiCmdRemindSize = pPCIV->header.cmdSize + pPCIV->header.contentSize - (uiDataSize - PCIV_HDR_SIZE);


	return 0;
}

