/**		@file	inet_main.c
 * 		- Internet 사용량을 계산 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: inet_main.c,v 1.3 2011/09/07 06:30:47 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:47 $
 * 		@ref		inet_main.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(소개)
 * 		- Internet 사용량을 계산 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

/**
 * Include headers
 */
#include <stdio.h>
#include <unistd.h>

// TOP
#include "commdef.h"
#include "procid.h"
#include "path.h"
#include "filter.h"
#include "capdef.h"
#include "sshmid.h"

// LIB
#include "common_stg.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "memg.h"
#include "Analyze_Ext_Abs.h"
#include "utillib.h"

// .
#include "inet_init.h"
#include "inet_func.h"

/**
 * Declare variables
 */
stMEMSINFO			*pMEMSINFO;
stCIFO				*gpCIFO;
stHASHOINFO			*pCALLHASH;
stHASHOINFO			*pINETHASH;
stTIMERNINFO		*pTIMER;

S32					giFinishSignal;				/**< Finish Signal */
S32					giStopFlag;					/**< main loop Flag 0: Stop, 1: Loop */
S32					guiSeqProcID;

S32					gATCPCnt = 0;
S32					gACALLCnt = 0;

char				gszMyProc[32];

long long			total_cnt, total_size;

extern st_Flt_Info	*flt_info;

/**
 * Declare functions
 */
void invoke_del(void *p);

/**
 *	Implement func.
 */

/** main function.
 *
 *	mani Function
 *
 *	@param	argc	:	파라미터 개수
 *	@param	*argv[]	:	파라미터
 *
 *	@return			S32
 *	@see			inet_main.c
 *
 *	@exception		Nothing
 *	@not			Nothing
 **/ 
int main(int argc, char *argv[])
{
	S32						i, dRet;
	OFFSET					offset, sub_offset;
	Capture_Header_Msg		*pCAPHEAD;
	INFO_ETH				*pINFOETH;
	CALL_KEY				*pCALLKey;	
	U8						*pNode, *pCurrNode, *pNextNode;
	S32						dLen, PROCNO;
	char					szLOGPATH[128];
	char					vERSION[7] = "R3.0.0";
	U32						uiClientIP;
	U8						ucProtocol;
	time_t					nowTime, oldTime;
	S32						index;

	/* process name */
	memcpy(&gszMyProc[0], argv[0], strlen(argv[0]));

	dLen = strlen(gszMyProc);
	PROCNO = atoi(&gszMyProc[dLen-1]);
	guiSeqProcID = SEQ_PROC_A_INET0 + PROCNO;	

	/* log_print 초기화 */
	sprintf(szLOGPATH, LOG_PATH"/%s", gszMyProc);
	log_init(S_SSHM_LOG_LEVEL, getpid(), guiSeqProcID, szLOGPATH, gszMyProc);

	/* A_TCP 초기화 */
	if((dRet = dInitINET(&pMEMSINFO, &pCALLHASH, &pINETHASH, &pTIMER, PROCNO)) < 0) 
	{
		log_print(LOGN_CRI, "[%s][%s.%d] dInitNET dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

	if((dRet = set_version(S_SSHM_VERSION, guiSeqProcID, vERSION)) < 0 ) 
	{
		log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, guiSeqProcID, vERSION);
	}

	log_print(LOGN_CRI, "START %s[%d]", gszMyProc, PROCNO);

	nowTime = time(NULL);
	oldTime = nowTime;
	
	total_cnt = 0;
	total_size = 0;

	while(giStopFlag)
	{
		timerN_invoke(pTIMER);

		nowTime = time(NULL);
		if(nowTime >= oldTime + 60)
		{
			log_print(LOGN_CRI, "TOT CALL=%u SESS=%u CNT=%lld SIZE=%lld TIMER=%u",
				((stMEMGINFO *)HASHO_PTR(pCALLHASH, pCALLHASH->offset_memginfo))->uiMemNodeAllocedCnt,
				((stMEMGINFO *)HASHO_PTR(pINETHASH, pINETHASH->offset_memginfo))->uiMemNodeAllocedCnt,
				total_cnt, total_size, flt_info->stTimerInfo.usTimerInfo[PI_INET_TIMEOUT]);
			total_cnt = 0;
			total_size = 0;
			oldTime = nowTime;
		}

		for(i = 0; i < 200000; i++)
		{
			if((offset = gifo_read(pMEMSINFO, gpCIFO, guiSeqProcID)) > 0)
			{
				pNextNode = nifo_ptr(pMEMSINFO, offset);
				pNode = pNextNode;
				pCurrNode = NULL;
				while(pCurrNode != pNextNode)
				{
					pCurrNode = pNextNode;
					sub_offset = nifo_offset(pMEMSINFO, pCurrNode);
					pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);

					pCAPHEAD = (Capture_Header_Msg *)nifo_get_value(pMEMSINFO, CAP_HEADER_NUM, sub_offset);
					pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, sub_offset);
					pCALLKey = (CALL_KEY *)nifo_get_value(pMEMSINFO, CLEAR_CALL_NUM, sub_offset);

					if(pCALLKey)
					{
						dProcINETCallStop(pMEMSINFO, pTIMER, pCALLHASH, pINETHASH, pCALLKey);
						nifo_node_unlink_nont(pMEMSINFO, pCurrNode);
						nifo_node_delete(pMEMSINFO, pCurrNode);
					}
					else if((pCAPHEAD == NULL) || (pINFOETH == NULL))
					{
						nifo_node_unlink_nont(pMEMSINFO, pCurrNode);
						nifo_node_delete(pMEMSINFO, pCurrNode);
					}
					else
					{
						total_cnt++;
						total_size += pINFOETH->stIP.wTotalLength;

						dProcINETData(pMEMSINFO, pTIMER, pCALLHASH, pINETHASH, pCAPHEAD, pINFOETH);

						if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT)
						{
							uiClientIP = pINFOETH->stIP.dwSrcIP;
						}
						else
						{
							uiClientIP = pINFOETH->stIP.dwDestIP;
						}
						ucProtocol = pINFOETH->stIP.ucProtocol;
						nifo_node_unlink_nont(pMEMSINFO, pCurrNode);
						index = uiClientIP % gATCPCnt;
						dSend_INET2_Data(pMEMSINFO, dGetProcID(uiClientIP, ucProtocol), pCurrNode, pCAPHEAD->curtime, index);
					}	
			
				}
			}
			else
			{
				usleep(0);
				break;
			}
		}
	}

	FinishProgram();
	exit(0);	

}

void invoke_del(void *p)
{
	INET_KEY		*pINETKEY, *pSUBDATA;
	INET_DATA		*pINETDATA;
	INET_CALL_KEY	INETCALLKEY;
	INET_CALL_KEY	*pCALLKEY = &INETCALLKEY;
	INET_CALL_DATA	*pCALLDATA;
	stHASHONODE		*pHASHNODE;
	LOG_INET		*pLOG;
	U8				*pNODE, *pNEXT;
	U8				szCIP[INET_ADDRSTRLEN];
	U8				szSIP[INET_ADDRSTRLEN];
	struct timeval	stTime;
	OFFSET			offset;

	gettimeofday(&stTime, NULL);

	pINETKEY = &(((TIMER_COMMON *)p)->INETKEY);

	if((pHASHNODE = hasho_find(pINETHASH, (U8 *)pINETKEY)) != NULL)
	{
		pINETDATA = (INET_DATA *)nifo_ptr(pINETHASH, pHASHNODE->offset_Data);
		pLOG = (LOG_INET *)nifo_ptr(pMEMSINFO, pINETDATA->offset_Log);
		log_print(LOGN_DEBUG, "@@@ DATA TIMER TIMEOUT CIP=%s SIP=%s SPORT=%d",
				util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);
		
		pLOG->uiOpEndTime = stTime.tv_sec;
		pLOG->uiOpEndMTime = stTime.tv_usec;

		dSendINETLog(pMEMSINFO, pLOG, dGetCALLProcID(pLOG->uiClientIP));

		pCALLKEY->uiIP = pINETKEY->uiCIP;
		pCALLKEY->uiReserved = 0;
		if((pHASHNODE = hasho_find(pCALLHASH, (U8 *)pCALLKEY)) != NULL)
		{
			pCALLDATA = (INET_CALL_DATA *)nifo_ptr(pCALLHASH, pHASHNODE->offset_Data);
			pNODE = nifo_ptr(pMEMSINFO, pCALLDATA->offset_Data);
			while(pNODE != NULL)
			{
				pNEXT = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNODE)->nont.offset_next), NIFO, nont);

				offset = nifo_offset(pMEMSINFO, pNODE);
				pSUBDATA = (INET_KEY *)nifo_get_value(pMEMSINFO, INET_KEY_DEF_NUM, offset);

				if(!memcmp(pSUBDATA, pINETKEY, INET_KEY_SIZE))
				{
					nifo_node_unlink_nont(pMEMSINFO, pNODE);
					nifo_node_delete(pMEMSINFO, pNODE);

					if(pNODE == pNEXT) pNEXT = NULL;
					pNODE = pNEXT;

					pCALLDATA->offset_Data = nifo_offset(pMEMSINFO, pNODE);
					break;
				}

				if(pNODE == pNEXT) pNEXT = NULL;
				pNODE = pNEXT;
			}

			if(pCALLDATA->offset_Data == 0)
			{
				hasho_del(pCALLHASH, (U8 *)pCALLKEY);
				log_print(LOGN_INFO, "@@@ TIMER DEL CALL IP=%s", util_cvtipaddr(szCIP, pCALLKEY->uiIP));
			}

			hasho_del(pINETHASH, (U8 *)pINETKEY);
			log_print(LOGN_INFO, "@@@ TIMER DEL INET CIP=%s SIP=%s SPORT=%d",
					util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);
		}
		else
		{
			log_print(LOGN_DEBUG, "@@@ DATA TIMER TIMEOUT BUT CALL NULL CIP=%s SIP=%s SPORT=%d",
					util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);
		}
		
	}
	else
	{
		log_print(LOGN_CRI, "@@@ DATA TIMER TIMEOUT BUT NODE NULL CIP=%s SIP=%s SPORT=%d",
				util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);
	}
}

/*
 * 	$Log: inet_main.c,v $
 * 	Revision 1.3  2011/09/07 06:30:47  hhbaek
 * 	*** empty log message ***
 * 	
 * 	Revision 1.2  2011/09/04 12:16:51  hhbaek
 * 	*** empty log message ***
 * 	
 * 	Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * 	NEW OAM SYSTEM
 * 	
 * 	Revision 1.4  2011/08/18 04:38:28  hhbaek
 * 	*** empty log message ***
 * 	
 * 	Revision 1.3  2011/08/17 13:00:01  hhbaek
 * 	A_INET
 * 	
 * 	Revision 1.2  2011/08/10 09:57:43  uamyd
 * 	modified and block added
 * 	
 * 	Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * 	init DQMS2
 * 	
 * 	Revision 1.12  2011/05/11 07:44:08  jsyoon
 * 	*** empty log message ***
 * 	
 * 	Revision 1.11  2011/05/10 16:57:24  dark264sh
 * 	A_INET: buffering 처리
 * 	
 * 	Revision 1.10  2011/05/09 09:35:42  dark264sh
 * 	A_INET: A_CALL multi 처리
 * 	
 * 	Revision 1.9  2011/04/24 12:04:10  dark264sh
 * 	A_INET: Call Clear 메시지의 node를 삭제하지 않는 버그 수정
 * 	
 * 	Revision 1.8  2011/04/24 11:09:05  dark264sh
 * 	A_INET: Call, Session, 사용량 log_print 추가
 * 	
 * 	Revision 1.7  2011/04/19 05:03:24  dark264sh
 * 	A_INET: timeout 이후 inet hash를 삭제하지 않는 버그 수정
 * 	
 * 	Revision 1.6  2011/04/18 08:38:42  dark264sh
 * 	A_INET: LOG_INET없는 경우 call 삭제 하로독 변경
 * 	
 * 	Revision 1.5  2011/04/17 11:11:21  dark264sh
 * 	A_INET: timeout 처리 버그 수정
 * 	
 * 	Revision 1.4  2011/04/16 09:47:31  dark264sh
 * 	A_INET: TCP Protocol인 경우 A_ITCP로 전송
 * 	
 * 	Revision 1.3  2011/04/13 15:37:55  dark264sh
 * 	A_INET: dProcINETData 처리
 * 	
 * 	Revision 1.2  2011/04/13 14:15:36  dark264sh
 * 	A_INET: dSendINETLog 처리
 * 	
 * 	Revision 1.1  2011/04/13 13:14:38  dark264sh
 * 	A_INET 추가
 * 	
 */
