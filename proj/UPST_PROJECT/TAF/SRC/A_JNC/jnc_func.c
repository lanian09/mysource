/**		@file	jnet_func.c
 * 		- java network content를 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: jnc_func.c,v 1.3 2011/09/07 06:30:47 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:47 $
 * 		@warning	.
 * 		@ref		jnet_main.c jnet_init.c jnet_func.c jnet_api.h
 * 		@todo		
 * 		@section	Intro(소개)
 * 		- java network content를 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>


#include "loglib.h"
#include "utillib.h"
#include "tools.h"

#include "procid.h"

#include "tools.h"

#include "jnc_util.h"
#include "jnc_msgq.h"
#include "jnc_func.h"

extern S64				curSessCnt;
extern struct timeval	stNowTime;

/** dProcJNCSvcProcess.
 * TCP로 부터 받은 데이터를 Control하고 TCP Session, JNET Transaction 관리
 *
 * @param	*pMEMSINFO	: New Interface 관리를 위한 구조체
 * @param	*pJNCHASH	: JNC Session을 관리하는 HASH
 * @param	*pTCPINFO	: TCP로 부터 받은 데이터의 정보를 담고 있는 구조체
 * @param	*pNode		: TCP로 부터 New Interface Node
 *
 * @return	S32
 * @see		jnc_func.c jnc_msgq.c jnc_main.c jnc_init.c jnc_util.c jnc_api.c
 *
 * @exception	nothing
 * @note		nothing
 **/

S32 dJNCSvcProcess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pJNCHASH, TCP_INFO *pTCPINFO, U8 *pNode, U8 *pDATA)
{
	S32	dRet;

	U8	szSrcIP[INET_ADDRSTRLEN];
	U8	szDestIP[INET_ADDRSTRLEN];	
	JNC_SESS_KEY	JNCSESSKEY;
	JNC_SESS_KEY	*pJNCSESSKEY;
	JNC_SESS_DATA	*pJNCSESS;


	pJNCSESSKEY = &JNCSESSKEY;
	
	stHASHONODE		*pHASHNODE;
	
	/* TCP 상태에 따른 처리*/
#if DEBUG_MODE
	dRet = RcvPacketInfo(pTCPINFO);
#endif
	switch(pTCPINFO->cTcpFlag)
	{
	case DEF_TCP_START:
		/**
		 * A_TCP에서 TCP Session이 생성될 경우
		 * 새로운 TCP NODE 생성
		 * A_CALL에 TCP Session 생성 통보
		 **/
		 log_print(LOGN_DEBUG,"@@@ START SIP[%s] SPORT[%u]DIP[%s]DPORT[%u]",
	 			util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
				util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);

		MakeJNCHashKey(pTCPINFO, pJNCSESSKEY);
		
		if((pJNCSESS = pCreateJNCSess(pMEMSINFO,pJNCHASH, pJNCSESSKEY, pTCPINFO)) ==NULL){
			log_print(LOGN_CRI, "[%s][%s.%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] pCreateTcpSess NULL",
			__FILE__, __FUNCTION__,__LINE__,
			util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
			util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
			return -1;
		}
		else{
		 log_print(LOGN_DEBUG, "** [%s][%s.%d]START JNCCALL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] dCreateJNCSess dRet",
			 	__FILE__,__FUNCTION__, __LINE__,
				util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
			 	util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
		}

		UpdateJNCSess(pTCPINFO, pJNCSESS);

		if((dRet = dSend_JNC_Data(pMEMSINFO, SEQ_PROC_A_CALL, pNode)) < 0) {
			log_print(LOGN_CRI, "[%s][%s.%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] dSendSignal dRet[%d]",
				__FILE__, __FUNCTION__, __LINE__,
			util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
			util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, dRet);
			return -12;
		}

		break;


	case DEF_TCP_END:

		/**
		 * A_TCP에서 TCP Session이 생성될 경우
		 * 새로운 TCP NODE 생성
		 * A_CALL에 TCP Session 생성 통보
		 **/

		 log_print(LOGN_DEBUG, "** END SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]",
		 	util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
			util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);

			MakeJNCHashKey(pTCPINFO, pJNCSESSKEY);

			if((pHASHNODE = hasho_find(pJNCHASH, (U8*)pJNCSESSKEY))==NULL)
			{
			 /* Tcp Hash를 찾지 못함 */
				log_print(LOGN_CRI, "[%s][%s.%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]",
				__FILE__, __FUNCTION__,__LINE__,
				util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
				util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
				return -3;
			}
			pJNCSESS = (JNC_SESS_DATA*)nifo_ptr(pJNCHASH, pHASHNODE->offset_Data);
			UpdateJNCSess(pTCPINFO, pJNCSESS);
	
			gettimeofday(&stNowTime, NULL);
			pJNCSESS->szJNCLOG.uiOpEndTime = stNowTime.tv_sec;	
			pJNCSESS->szJNCLOG.uiOpEndMTime= stNowTime.tv_usec;

			if((dRet=dMakeJNCLOGInfo(pMEMSINFO,pJNCSESS))<0){
				log_print(LOGN_CRI,LH"FAIL MAKE JNCLOG [%d]",LT, dRet);
			}

			if((dRet = dCloseJNCSess(pMEMSINFO, pJNCHASH,pJNCSESSKEY,pJNCSESS))<0){

			 log_print(LOGN_CRI, "** [%s][%s.%d]END SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] dCloseJNCSess dRet",
			 	__FILE__,__FUNCTION__, __LINE__,
				util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
			 	util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
				return -4;
			}
			else{
			 log_print(LOGN_DEBUG, "** [%s][%s.%d]END JNCCALL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] dCloseJNCSess dRet",
			 	__FILE__,__FUNCTION__, __LINE__,
				util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
			 	util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
			}

			if((dRet = dSend_JNC_Data(pMEMSINFO, SEQ_PROC_A_CALL, pNode)) < 0) {
				log_print(LOGN_CRI, "[%s][%s.%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] dSendSignal dRet[%d]",
					__FILE__, __FUNCTION__, __LINE__,
				util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
				util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, dRet);
				return -22;
			}

			/* A_CALL에 Tcp Session 종료 통보 */

			break;

	case DEF_TCP_DATA:
		 log_print(LOGN_DEBUG, "** PACKINFO SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]",
		 	util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
			util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);

			MakeJNCHashKey(pTCPINFO, pJNCSESSKEY);
			
			if((pHASHNODE = hasho_find(pJNCHASH, (U8*)pJNCSESSKEY))==NULL)
			{
			 /* Tcp Hash를 찾지 못함 */
				log_print(LOGN_CRI, "[%s][%s.%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]",
				__FILE__, __FUNCTION__,__LINE__,
				util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
				util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
				return -6;
			}
			pJNCSESS = (JNC_SESS_DATA*)nifo_ptr(pJNCHASH, pHASHNODE->offset_Data);

			if((dRet = dJNCMessage(pMEMSINFO, pJNCHASH, pJNCSESSKEY, pJNCSESS, pTCPINFO, pNode, pDATA))<0){
			 log_print(LOGN_CRI, "** [%s][%s.%d]END SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] dJNCMessage dRet[%d]",
			 	__FILE__,__FUNCTION__, __LINE__,
				util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
			 	util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort,dRet);
		 		 return -7;
			}

			nifo_node_delete(pMEMSINFO, pNode);

			break;
						
	default:
		 log_print(LOGN_CRI, 
			 "[%s][%s.%d] STRANGE TCP SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s] FLAG[%d]",
			 	__FILE__,__FUNCTION__, __LINE__,
				util_cvtipaddr(szSrcIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
			 	util_cvtipaddr(szDestIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort,
				PrintRtx(pTCPINFO->ucRtx), pTCPINFO->cTcpFlag);
				return -8;
	}

	return 0;
}


/** dCreateJNCSess  function.
 * JNC 세션을 생성
 *	관련 데이터를 전송 및 정리
 * @param	*pMEMSINFO	: New Interface 관리를 위한 구조체
 * @param	*pJNCHASH	: JNC Session을 관리를 위한 HASH
 * @param	*pTCPINFO	: TCP로 부터 받은 데이터의 정보를 담고 있는 구조체
 * @param	*pNode		: TCP로 부터 New Interface Node
 *
 * @return	S32
 * @see		jnc_func.c jnc_msgq.c jnc_main.c jnc_init.c jnc_util.c jnc_api.c
 *
 * @exception	nothing
 * @note		nothing
 **/


JNC_SESS_DATA * pCreateJNCSess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pJNCHASH, JNC_SESS_KEY *pJNCSESSKEY, TCP_INFO *pTCPINFO)
{
	JNC_SESS_DATA	JNCSESS;
	JNC_SESS_DATA	*pJNCSESS = &JNCSESS;
	struct timeval      stNowTime;
	stHASHONODE 	*pHASHNODE;
	JNC_COMMON		JNCCOMMON;

	memcpy(&JNCCOMMON.JNCSESSKEY, pJNCSESSKEY, JNC_SESS_KEY_SIZE);


 	pJNCSESS->szJNCLOG.uiCallTime = pTCPINFO->uiCapTime;        
 	pJNCSESS->szJNCLOG.uiCallMTime = pTCPINFO->uiCapMTime;
	pJNCSESS->szJNCLOG.uiClientIP = pTCPINFO->uiCliIP;
	pJNCSESS->szJNCLOG.uiNASName = 0;


	pJNCSESS->szJNCLOG.usServiceType = 0;
	pJNCSESS->szJNCLOG.szHostName[0] = 0x00;
	pJNCSESS->szJNCLOG.szBrowserInfo[0] = 0x00;   	
	pJNCSESS->szJNCLOG.szModel[0]=0x00;   	
	pJNCSESS->szJNCLOG.szNetOption[0] =0x00;   	
	pJNCSESS->szJNCLOG.szMIN[0] =0x00;  				

	pJNCSESS->szJNCLOG.usClientPort = pTCPINFO->usCliPort;
	pJNCSESS->szJNCLOG.uiServerIP = pTCPINFO->uiSrvIP;
	pJNCSESS->szJNCLOG.usServerPort = pTCPINFO->usSrvPort;
	pJNCSESS->szJNCLOG.uiTcpSynTime = pTCPINFO->uiCapTime;	
	pJNCSESS->szJNCLOG.uiTcpSynMTime = pTCPINFO-> uiCapMTime;
	pJNCSESS->szJNCLOG.usTransID= 0;				

	pJNCSESS->szJNCLOG.usSvcL4Type = pTCPINFO->usL4Code;				
	pJNCSESS->szJNCLOG.usSvcL7Type = pTCPINFO->usL7Code ;		
	pJNCSESS->szJNCLOG.ucSubSysNo = 0;			
	pJNCSESS->szJNCLOG.usPlatformType = dGetPlatformType(pTCPINFO->usL4Code, pTCPINFO->usL7Code);
	pJNCSESS->szJNCLOG.ucMethod = 0;

	pJNCSESS->szJNCLOG.uiReqStartTime = 0;		
	pJNCSESS->szJNCLOG.uiReqStartMTime = 0;	
	pJNCSESS->szJNCLOG.uiReqEndTime = 0;		
	pJNCSESS->szJNCLOG.uiReqEndMTime= 0;			
	pJNCSESS->szJNCLOG.uiReqAckTime = 0 ;		
	pJNCSESS->szJNCLOG.uiReqAckMTime = 0;			
	pJNCSESS->szJNCLOG.uiResStartTime = 0;		
	pJNCSESS->szJNCLOG.uiResStartMTime = 0;		
	pJNCSESS->szJNCLOG.uiResEndTime = 0;		
	pJNCSESS->szJNCLOG.uiResEndMTime = 0;			
	pJNCSESS->szJNCLOG.uiMNAckTime = 0;
	pJNCSESS->szJNCLOG.uiMNAckMTime= 0;				
	pJNCSESS->szJNCLOG.uiLastPktTime= 0;		
	pJNCSESS->szJNCLOG.uiLastPktMTime = 0;			
	pJNCSESS->szJNCLOG.llTransGapTime = 0;		
	pJNCSESS->szJNCLOG.usResCode = 0;					

	pJNCSESS->szJNCLOG.ucTcpClientStatus = pTCPINFO->ucTcpClientStatus;
	pJNCSESS->szJNCLOG.ucTcpServerStatus = pTCPINFO->ucTcpServerStatus;
	pJNCSESS->szJNCLOG.ucStatus = 0;		
	pJNCSESS->szJNCLOG.usUserErrorCode = 0;		

	pJNCSESS->szJNCLOG.usL4FailCode =  pTCPINFO->usL4FailCode;		
	pJNCSESS->szJNCLOG.usL7FailCode = 0;			

	pJNCSESS->szJNCLOG.uiIPDataUpPktCnt =0;
	pJNCSESS->szJNCLOG.uiIPDataDnPktCnt = 0;
	pJNCSESS->szJNCLOG.uiIPTotUpPktCnt = 0;	
	pJNCSESS->szJNCLOG.uiIPTotDnPktCnt = 0;
	pJNCSESS->szJNCLOG.uiIPDataUpRetransCnt = 0;		
	pJNCSESS->szJNCLOG.uiIPDataDnRetransCnt = 0;	
	pJNCSESS->szJNCLOG.uiIPTotUpRetransCnt = 0;	
	pJNCSESS->szJNCLOG.uiIPTotDnRetransCnt = 0;	
	pJNCSESS->szJNCLOG.uiIPDataUpPktSize = 0;		
	pJNCSESS->szJNCLOG.uiIPDataDnPktSize = 0;		
	pJNCSESS->szJNCLOG.uiIPTotUpPktSize = 0;		
	pJNCSESS->szJNCLOG.uiIPTotDnPktSize = 0;		
	pJNCSESS->szJNCLOG.uiIPDataUpRetransSize = 0;	
	pJNCSESS->szJNCLOG.uiIPDataDnRetransSize = 0;	
	pJNCSESS->szJNCLOG.uiIPTotUpRetransSize = 0;	
	pJNCSESS->szJNCLOG.uiIPTotDnRetransSize = 0;	
	pJNCSESS->szJNCLOG.uiTcpUpBodySize=0;		
	pJNCSESS->szJNCLOG.uiTcpDnBodySize=0;	
	
	pJNCSESS->szJNCLOG.szContentCode[0]= 0x00;
	pJNCSESS->szJNCLOG.szCpCode[0]= 0x00;
	pJNCSESS->szJNCLOG.szMIN[0] = 0x00;

	pJNCSESS->szJNCLOG.uiOpStartTime =0;
	pJNCSESS->szJNCLOG.uiOpStartMTime = 0;
	pJNCSESS->szJNCLOG.uiOpEndTime = 0;
	pJNCSESS->szJNCLOG.uiOpEndMTime = 0;	

	pJNCSESS->usREQBuffLen = 0; 
	pJNCSESS->ucREQParaFlag = 0; 
	pJNCSESS->ucResStartFlag = 1;
	pJNCSESS->szREQBuffer[0] = 0x00;

	/*Tcp Session 생성 */
	if((pHASHNODE = hasho_add(pJNCHASH, (U8*)pJNCSESSKEY, (U8*)pJNCSESS))==NULL)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] hasho_add NULL", __FILE__, __FUNCTION__, __LINE__);
		return  NULL;
	}
	else{
		pJNCSESS = (JNC_SESS_DATA *)nifo_ptr(pJNCHASH, pHASHNODE->offset_Data);

		gettimeofday(&stNowTime, NULL);
		pJNCSESS->szJNCLOG.uiOpStartTime = stNowTime.tv_sec;
		pJNCSESS->szJNCLOG.uiOpStartMTime = stNowTime.tv_usec;
	}
	curSessCnt++;


	return pJNCSESS;
}




/** dCloseTcpSess  function.
 * TCP HASH를 삭제하고 JNC Session을 정리ㅑ
 *	관련 데이터를 전송 및 정리
 * @param	*pMEMSINFO	: New Interface 관리를 위한 구조체
 * @param	*pJNCHASH	: JNC Session을 관리를 위한 HASH
 * @param	*pTCPINFO	: TCP로 부터 받은 데이터의 정보를 담고 있는 구조체
 * @param	*pNode		: TCP로 부터 New Interface Node
 *
 * @return	S32
 * @see		jnc_func.c jnc_msgq.c jnc_main.c jnc_init.c jnc_util.c jnc_api.c
 *
 * @exception	nothing
 * @note		nothing
 **/

S32 dCloseJNCSess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pJNCHASH, JNC_SESS_KEY *pJNCSESSKEY, JNC_SESS_DATA *pJNCSESS)
{
	stHASHONODE	*pHASHNODE;
	if((pHASHNODE = hasho_find(pJNCHASH,(U8 *)pJNCSESSKEY))==NULL){
		log_print(LOGN_CRI, "[%s][%s.%d] hasho_find NULL", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	hasho_del(pJNCHASH, (U8 *)pJNCSESSKEY);

	curSessCnt--;
	
	return 0;
}

S32 RcvPacketInfo(TCP_INFO *pTCPINFO)
{
	int dLog = LOGN_INFO;
	U8	szSIP[INET_ADDRSTRLEN];
	U8	szDIP[INET_ADDRSTRLEN];

	log_print(dLog,"************RECIVE PACKET INFO****************");
    log_print(LOGN_DEBUG," SIP[%s] SPORT[%u]DIP[%s]DPORT[%u]",
	         util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
	         util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
	log_print(dLog,"Data Cnt UP[%u] Dn[%u] Tot Cnt UP[%u] DN[%u]",
			pTCPINFO->uiIPDataUpPktCnt,pTCPINFO->uiIPDataDnPktCnt,
			pTCPINFO->uiIPTotUpPktCnt,pTCPINFO->uiIPTotDnPktCnt);
	log_print(dLog,"Data Size Up[%u] Dn[%u] Tot Cnt Up[%u] Dn [%u]",
			pTCPINFO->uiIPDataUpPktSize,pTCPINFO->uiIPDataDnPktSize,
			pTCPINFO->uiIPTotUpPktSize,pTCPINFO->uiIPTotDnPktSize);
	log_print(dLog,"**********************************************");
	return 0;
}
