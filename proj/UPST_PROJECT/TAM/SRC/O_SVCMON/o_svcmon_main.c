/**		@file	o_svcmon_main.c
 * 		- O_SVCMON Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: o_svcmon_main.c,v 1.4 2011/09/05 12:52:57 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.4 $
 * 		@date		$Date: 2011/09/05 12:52:57 $
 * 		@warning	.
 * 		@ref		o_svcmon_main.c o_svcmon_init.c o_svcmon_func.c
 * 		@todo		.
 *
 * 		@section	Intro(소개)
 * 		- O_SVCMON Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <stdio.h>
#include <string.h>
#include <errno.h>

// LIB
#include "typedef.h"
#include "hasho.h"
#include "loglib.h"
#include "verlib.h"

// PROJECT
#include "path.h"
#include "procid.h"
#include "sshmid.h"
#include "msgdef.h"
#include "func_time_check.h"

// TAM
#include "watch_mon.h"
#include "watch_filter.h"

// .
#include "o_svcmon_init.h"
#include "o_svcmon_conf.h"
#include "o_svcmon_msgq.h"
#include "o_svcmon_func.h"
#include "o_svcmon_alarm.h"


S32						giFinishSignal;					/**< Finish Signal */
S32						giStopFlag;						/**< main loop Flag 0: Stop, 1: Loop */

st_MonTotal		*gMonTotal;
st_MonTotal_1Min	*gMonTotal1Min;
st_WatchFilter	*gWatchFilter;

char	vERSION[7] = "R4.0.0";

S32						dMyQID;
S32						dMONDQID;
S32						dALMDQID;
S32						dCONDQID;
S32						dMSVCMONQID;
S32						dSysNo = 0;
S32						gdIndex = 1;

stHASHOINFO				*pAMonHash;
stHASHOINFO				*pSMonHash;

stHASHOINFO				*pMonHash;
stHASHOINFO				*pNextMonHash;

stHASHOINFO				*pDefHash;
stHASHOINFO				*pThresHash;
stHASHOINFO				*pNasIPHash;

st_MonList				stBaseMonList;
st_MonList_1Min			stABaseMonList1Min;
st_MonList_1Min			stSBaseMonList1Min;

st_MonList_1Min			*pBaseMonList1Min;
st_MonList_1Min			*pNextBaseMonList1Min;

st_FuncTimeCheckList    stFuncTimeCheckList;
st_FuncTimeCheckList    *pFUNC = &stFuncTimeCheckList;
/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			o_svcmon_main.c o_svcmon_init.c o_svcmon_func.c
 *
 *  @exception		.
 *  @note			.
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32				isChanged = 0, isHashChg = 0;
	S32				dRet;
	time_t			curtime, oldtime, stattime, nowstat;
	time_t			stattime2, nowstat2;
	st_WatchMsg		*pWATCH;
	st_MonList		*pMonList = NULL, *pOldMonList = NULL;
	st_MonList_1Min	*pMonList1Min = NULL;
	st_SvcMonMsg	*pSvcMonMsg;
	st_MsgQ			stMsgQ;
	st_MsgQSub		*pstMsgQSub = (st_MsgQSub *)&stMsgQ.llMType;

	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_O_SVCMON, LOG_PATH"/O_SVCMON", "O_SVCMON");

	/* O_SVCMON 초기화 */
	if((dRet = dInitOSVCMON(&pDefHash, &pAMonHash, &pSMonHash, &pThresHash, &pNasIPHash)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dInitOSVCMON dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_O_SVCMON, vERSION)) != 0)
		log_print( LOGN_CRI, "set_version error(ret=%d,idx=%d,ver=%s)", dRet,SEQ_PROC_ALMD,vERSION);

	pMonHash = pAMonHash;
	pNextMonHash = pSMonHash;

	pBaseMonList1Min = &stABaseMonList1Min;
	pNextBaseMonList1Min = &stSBaseMonList1Min;

	log_print(LOGN_CRI, "START O_SVCMON");

	curtime = time(NULL);
	oldtime = time(NULL);
	stattime = curtime / DEF_MON_PERIOD * DEF_MON_PERIOD;

	dMakeThresHash(pThresHash, gWatchFilter);
	dMakeBaseMonList(pMonHash, pNasIPHash, gWatchFilter, &stBaseMonList, pBaseMonList1Min);
	pMonList = pGetNextMonList(gMonTotal, &stBaseMonList);
	pOldMonList = pMonList;
	pMonList1Min = pGetNextMon1MinList(gMonTotal1Min, pBaseMonList1Min);
	//PRINTLOG(LOG_DEBUG, 0, pMonList, pMonList1Min);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		dRet = dIsReceivedMessage(&stMsgQ);
		if(dRet < 0) {
			log_print(LOGN_CRI, "F=%s:%s.%d dIsReceivedMessage dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
			usleep(0);
		} else if(dRet == 100 || dRet == 0) {
			usleep(0);
		} else {

			log_print(LOGN_INFO, "RCV MSG TYPE=%u SVCID=%u MSGID=%u. TOTSIZE[%ld]"
						, pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, sizeof(st_MsgQSub));

			switch(pstMsgQSub->usType)
			{
			case DEF_SYS:
				switch(pstMsgQSub->usSvcID)
				{
				case SID_STATUS:
					switch(pstMsgQSub->usMsgID)
					{
					case MID_SVC_MONITOR:
						pWATCH = (st_WatchMsg *)stMsgQ.szBody;
						dProcMON(pDefHash, pMonHash, pNasIPHash, pWATCH, pMonList, &stBaseMonList, pMonList1Min, pBaseMonList1Min);
//PRINTLOG(LOGN_INFO, 1, pMonList, pMonList1Min);
						break;
					default:
						log_print(LOGN_INFO, "UNKNOWN MSGID TYPE=%u SVCID=%u MSGID=%u", 
								pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);
						break;
					}
					break;
				case SID_FLT:
					switch(pstMsgQSub->usMsgID)
					{
					case MID_FLT_SVC:
					case MID_FLT_SCTP:
					case MID_FLT_EQUIP:
					case MID_FLT_ACCESS:
						log_print(LOGN_CRI, "RCV FILTER UPDATE");
						dMakeBaseMonList(pNextMonHash, pNasIPHash, gWatchFilter, &stBaseMonList, pNextBaseMonList1Min);
						isChanged = 1;
						break;
					case MID_FLT_MON_THRES:
						log_print(LOGN_CRI, "RCV THRESHOLD UPDATE");
						dMakeThresHash(pThresHash, gWatchFilter);
						break;
					default:
						log_print(LOGN_INFO, "UNKNOWN MSGID TYPE=%u SVCID=%u MSGID=%u", 
								pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);
						break;
					}
					break;
				default:
					log_print(LOGN_INFO, "UNKNOWN SVCID TYPE=%u SVCID=%u MSGID=%u", 
							pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);
					break;
				}
				break;
			default:
				log_print(LOGN_INFO, "UNKNOWN TYPE TYPE=%u SVCID=%u MSGID=%u", 
						pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);
				break;
			}
		}

		curtime = time(NULL);
		if(curtime != oldtime) {
			nowstat = curtime / DEF_MON_PERIOD * DEF_MON_PERIOD;
			if(stattime != nowstat) {
				/* check alarm */
				dAlarmMON(pThresHash, pMonHash, pMonList, pOldMonList);

				pSvcMonMsg = (st_SvcMonMsg *)stMsgQ.szBody;
				pSvcMonMsg->lTime = pMonList->lTime;
				pSvcMonMsg->uiIdx = gMonTotal->dCurIdx;
log_print(LOGN_INFO, "MID_SVC_MONITOR: pSvcMonMsg->lTime[%ld] pSvcMonMsg->uiIdx[%u]", pSvcMonMsg->lTime, pSvcMonMsg->uiIdx);
				pstMsgQSub->usType = DEF_SYS;
				pstMsgQSub->usSvcID = SID_STATUS;
				pstMsgQSub->usMsgID = MID_SVC_MONITOR;
				stMsgQ.ucProID = SEQ_PROC_O_SVCMON;
				stMsgQ.usBodyLen = sizeof(st_SvcMonMsg);

				/* send signal to mond */
				if((dRet = dSendMsg(dMONDQID, &stMsgQ)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d dSendMsg dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
				}

#ifndef _DISABLE_INTERLOCK_DNMS_ /* DNMS interlock disabled by uamyd 20101101 */
				/* send signal to m_svcmon */
				if((dRet = dSendMsg(dMSVCMONQID, &stMsgQ)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d dSendMsg dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
				}
#endif /* _DISABLE_INTERLOCK_DNMS_ */

//PRINTLOG(LOG_DEBUG, 1, pMonList, pMonList1Min);
PRINT_FUNC_TIME_CHECK(pFUNC);

				pOldMonList = pMonList;

				/* get next st_MonList */
				pMonList = pGetNextMonList(gMonTotal, &stBaseMonList);
				if(isChanged) {
					void *p = pMonHash;
					pMonHash = pNextMonHash;
					pNextMonHash = p;

					p = pBaseMonList1Min;
					pBaseMonList1Min = pNextBaseMonList1Min;
					pNextBaseMonList1Min = p;

					isChanged = 0;
					isHashChg = 1;
				} 

				stattime = nowstat;
			}

			nowstat2 = curtime / DEF_MON_PERIOD_1MIN * DEF_MON_PERIOD_1MIN;
			if( stattime2 != nowstat2 ) {
				if(isHashChg) {
                	dAlarmMON1Min(pThresHash, pNextMonHash, pMonList1Min);
					isHashChg = 0;
				}
				else {
                	dAlarmMON1Min(pThresHash, pMonHash, pMonList1Min);
				}

                pSvcMonMsg = (st_SvcMonMsg *)stMsgQ.szBody;
                pSvcMonMsg->lTime = pMonList1Min->lTime;
                pSvcMonMsg->uiIdx = gMonTotal1Min->dCurIdx;
log_print(LOGN_INFO, "MID_SVC_MONITOR_1MIN: pSvcMonMsg->lTime[%ld] pSvcMonMsg->uiIdx[%u]", pSvcMonMsg->lTime, pSvcMonMsg->uiIdx);
                pstMsgQSub->usType = DEF_SYS;
                pstMsgQSub->usSvcID = SID_STATUS;
                pstMsgQSub->usMsgID = MID_SVC_MONITOR_1MIN;
                stMsgQ.ucProID = SEQ_PROC_O_SVCMON;
                stMsgQ.usBodyLen = sizeof(st_SvcMonMsg);

                /* send signal to mond */
                if((dRet = dSendMsg(dALMDQID, &stMsgQ)) < 0) {
                    log_print(LOGN_CRI, "F=%s:%s.%d dSendMsg dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
                }       

#ifndef _DISABLE_INTERLOCK_DNMS_ /* DNMS interlock disabled by uamyd 20101101 */
                /* send signal to m_svcmon */
                if((dRet = dSendMsg(dMSVCMONQID, &stMsgQ)) < 0) {
                    log_print(LOGN_CRI, "F=%s:%s.%d dSendMsg dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
                }       
#endif /* _DISABLE_INTERLOCK_DNMS_ */

				//PRINTLOG(LOG_DEBUG, 1, pMonList, pMonList1Min);
				PRINT_FUNC_TIME_CHECK(pFUNC);
                /* get next st_MonList */
                pMonList1Min = pGetNextMon1MinList(gMonTotal1Min, pBaseMonList1Min);
#if 0
                if(isChanged) {
                    void *p = pMonHash;
                    pMonHash = pNextMonHash;
                    pNextMonHash = p;
                    isChanged = 0;
                } 
#endif
				stattime2 = nowstat2;
			}

			oldtime = curtime;
		}
	}

	FinishProgram();

	return 0;
}

/*
 *  $Log: o_svcmon_main.c,v $
 *  Revision 1.4  2011/09/05 12:52:57  uamyd
 *  modified
 *
 *  Revision 1.3  2011/09/05 05:33:00  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/31 16:08:08  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.1  2011/08/23 08:58:48  dcham
 *  *** empty log message ***
 *
 *  Revision 1.25  2011/06/30 08:41:49  innaei
 *  *** empty log message ***
 *
 *  Revision 1.24  2011/06/28 18:51:17  innaei
 *  *** empty log message ***
 *
 *  Revision 1.23  2011/06/23 15:35:34  innaei
 *  *** empty log message ***
 *
 *  Revision 1.22  2011/06/23 00:58:18  innaei
 *  *** empty log message ***
 *
 *  Revision 1.21  2011/05/20 02:50:52  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.20  2011/05/10 14:57:42  innaei
 *  *** empty log message ***
 *
 *  Revision 1.19  2011/04/24 06:52:22  innaei
 *  *** empty log message ***
 *
 *  Revision 1.18  2011/04/20 14:01:08  uamyd
 *  log hexa delete
 *
 *  Revision 1.17  2011/04/18 03:06:11  innaei
 *  *** empty log message ***
 *
 *  Revision 1.16  2011/04/17 14:24:13  jhbaek
 *  *** empty log message ***
 *
 *  Revision 1.15  2011/04/17 12:36:52  innaei
 *  *** empty log message ***
 *
 *  Revision 1.14  2011/04/14 11:46:04  jhbaek
 *  *** empty log message ***
 *
 *  Revision 1.13  2011/04/12 07:42:45  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.12  2011/01/11 04:09:17  uamyd
 *  modified
 *
 *  Revision 1.2  2010/11/14 10:22:44  jwkim96
 *  STP 작업 내용 반영.
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:11  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.11  2010/03/29 12:23:34  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.10  2010/03/04 06:00:53  dark264sh
 *  ROAM NASIP NetMask 처리
 *
 *  Revision 1.9  2009/08/12 17:18:57  pkg
 *  *** empty log message ***
 *
 *  Revision 1.8  2009/06/27 11:56:56  dark264sh
 *  O_SVCMON MsgQ 전송시 llMType 세팅
 *
 *  Revision 1.7  2009/06/27 11:45:14  dark264sh
 *  M_LOG, M_SVCMON O_SVCMON 버전 처리
 *
 *  Revision 1.6  2009/06/24 07:51:03  dark264sh
 *  O_SVCMON MID_FLT 세분화 작업
 *
 *  Revision 1.5  2009/06/21 13:34:33  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.4  2009/06/20 15:51:23  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2009/06/18 17:01:48  dark264sh
 *  O_SVCMON Filter 처리
 *
 *  Revision 1.2  2009/06/17 11:05:35  dark264sh
 *  O_SVCMON, M_SVCMON MsgQ 사용 변경
 *
 *  Revision 1.1  2009/06/15 08:06:04  dark264sh
 *  O_SVCMON 기본 동작 처리
 *
 */

