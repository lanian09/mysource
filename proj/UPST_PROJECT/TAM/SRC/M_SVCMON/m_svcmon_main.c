/**		@file	m_svcmon_main.c
 * 		- M_SVCMON Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: m_svcmon_main.c,v 1.3 2011/09/05 12:52:56 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/05 12:52:56 $
 * 		@warning	.
 * 		@ref		m_svcmon_main.c m_svcmon_init.c m_svcmon_func.c
 * 		@todo		.
 *
 * 		@section	Intro(소개)
 * 		- M_SVCMON Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/
//System Header
#if 1
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#endif
#include <sys/time.h>
#include <sys/socket.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
//Common Header
#include "typedef.h"
#include "msgdef.h"
#include "procid.h"
#include "sshmid.h"
//LIB Header
#include "path.h"
#include "verlib.h"
#include "loglib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
//TAM Header
#include "watch_mon.h"
//Local Header
#include "m_svcmon_init.h"
#include "m_svcmon_func.h"
#include "m_svcmon_msgq.h"

S32			giFinishSignal;					/**< Finish Signal */
S32			giStopFlag;						/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO          *pMEMSINFO;
stCIFO              *gpCIFO;
extern st_MonTotal	*gMonTotal;
st_MonTotal_1Min	*gMonTotal1Min;

char	vERSION[7] = "R4.0.0";

S32			dMyQID;
S32			dSISVCMON;

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			m_svcmon_main.c m_svcmon_init.c m_svcmon_func.c
 *
 *  @exception		.
 *  @note			.
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32				dRet;
	st_SvcMonMsg	*pSvcMonMsg;
	st_MsgQ			stMsgQ;
	st_MsgQSub		*pstMsgQSub = (st_MsgQSub *)&stMsgQ.llMType;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_M_SVCMON, LOG_PATH"/M_SVCMON", "M_SVCMON");

	pMEMSINFO = nifo_init_zone((unsigned char *)"CHGSVCM", SEQ_PROC_CHGSVCM, FILE_NIFO_ZONE);
	if( pMEMSINFO == NULL ){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN nifo_init, NULL",  __FILE__, __FUNCTION__, __LINE__);
		exit(0);
	}

	gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
	if( gpCIFO == NULL ){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN gifo_init_group. cifo=%s, gifo=%s",
				__FILE__, __FUNCTION__, __LINE__, FILE_CIFO_CONF, FILE_GIFO_CONF);
		exit(0);
	}

	/* M_SVCMON 초기화 */
	if((dRet = dInitMSVCMON()) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dInitOSVCMON dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_M_SVCMON, vERSION)) != 0)
		log_print( LOGN_CRI, "set_version error(ret=%d,idx=%d,ver=%s)", dRet,SEQ_PROC_ALMD,vERSION);

	log_print(LOGN_CRI, "START M_SVCMON. VER=%s", vERSION);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		dRet = dIsReceivedMessage(&stMsgQ);
		if(dRet < 0) {
			usleep(0);
		} else {
			log_print(LOGN_INFO, "RCV MSG TYPE=%u SVCID=%u MSGID=%u", pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);

			switch(pstMsgQSub->usType)
			{
			case DEF_SYS:
				switch(pstMsgQSub->usSvcID)
				{
				case SID_STATUS:
					switch(pstMsgQSub->usMsgID)
					{
					case MID_SVC_MONITOR:
						pSvcMonMsg = (st_SvcMonMsg *)stMsgQ.szBody;
						dProcSvcMonMsg(gMonTotal, pSvcMonMsg);
						break;
					case MID_SVC_MONITOR_1MIN:
						pSvcMonMsg = (st_SvcMonMsg *)stMsgQ.szBody;
						dProcSvcMon1MinMsg(gMonTotal1Min, pSvcMonMsg);
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
	}

	FinishProgram();

	return 0;
}

/*
 *  $Log: m_svcmon_main.c,v $
 *  Revision 1.3  2011/09/05 12:52:56  uamyd
 *  modified
 *
 *  Revision 1.2  2011/09/01 07:49:50  dcham
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.1  2011/08/23 10:59:21  dcham
 *  *** empty log message ***
 *
 *  Revision 1.6  2011/06/30 08:41:49  innaei
 *  *** empty log message ***
 *
 *  Revision 1.5  2011/01/11 04:09:17  uamyd
 *  modified
 *
 *  Revision 1.2  2010/11/14 10:22:44  jwkim96
 *  STP 작업 내용 반영.
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:10  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.4  2009/06/27 11:44:56  dark264sh
 *  M_LOG, M_SVCMON O_SVCMON 버전 처리
 *
 *  Revision 1.3  2009/06/20 13:37:27  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2009/06/17 11:03:53  dark264sh
 *  O_SVCMON, M_SVCMON MsgQ 사용 변경
 *
 *  Revision 1.1  2009/06/16 08:05:51  dark264sh
 *  M_SVCMON 기본 동작 처리
 *
 */



