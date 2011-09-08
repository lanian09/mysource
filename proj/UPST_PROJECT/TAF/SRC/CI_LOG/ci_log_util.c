/**		@file   ci_log_main.c
 *		- TCP Session을 관리 하는 프로세스
 *
 *		Copyright (c) 2006~ by Upresto Inc. Korea
 *		All rights reserved
 *
 *		$Id: ci_log_util.c,v 1.2 2011/09/06 12:46:38 hhbaek Exp $
 *
 *		@Author		$Author: hhbaek $
 *		@version	$Revision: 1.2 $
 *		@date		$Date: 2011/09/06 12:46:38 $
 *		@ref		tcp_main.c
 *		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 *		@section    Intro(소개)
 *		- TCP Session을 관리 하는 프로세스
 *
 *		@section    Requirement
 *			@li library 생성 이후 함수 대치
 *
 **/

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"
#include "msgdef.h"
#include "filter.h"

// LIB
#include "utillib.h"

/**
 *	Implement func.
 */

/** PrintSID function. 
 *
 *	print sid
 *
 * 	@param sid: sid
 *
 *  @return     sid string value
 *  @see        ci_log_main.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
char *PrintSID(int sid)
{
	switch(sid)
	{
		case SID_STATUS:			return "SID_STATUS";
		case SID_SVC:				return "SID_SVC";
		case SID_SESS_INFO:			return "SID_SESS_INFO";
		case SID_TCP_INFO:			return "SID_TCP_INFO";
		case SID_DATA_INFO:			return "SID_DATA_INFO";
		case SID_CHECK_MSG:			return "SID_CHECK_MSG";
		case SID_LOG:				return "SID_LOG";
		case SID_FLT:				return "SID_FLT";
		case SID_GFLT:				return "SID_GFLT";
		case SID_CHKRES:			return "SID_CHKRES";
		case SID_CHKREQ:			return "SID_CHKREQ";
		case SID_MML:				return "SID_MML";
		case SID_PATCH:				return "SID_PATCH";
		case SID_COR:				return "SID_COR";
		default: 					return "UNKNOWN";
	}

	return "UNKNOWN";
}

/** PrintTcpLog function. 
 *
 *	print Message log 
 *
 *  @return     string
 *
 *  @exception  
 *  @note       Nothing
 **/
char *PrintMID(int type)
{
	switch(type)
	{
		case LOG_FTP_DEF_NUM: 			return "LOG_FTP_DEF_NUM";
		case START_CALL_NUM: 			return "START_CALL_NUM";
		case STOP_CALL_NUM: 			return "STOP_CALL_NUM";
		case LOG_SIGNAL_DEF_NUM: 		return "LOG_SIGNAL";
		case LOG_TCP_SESS_DEF_NUM:		return "LOG_TCP_SESS";
		case LOG_IM_SESS_DEF_NUM:		return "LOG_IM_SESS";
		case LOG_VT_SESS_DEF_NUM:		return "LOG_VT_SESS";
		case LOG_SIP_TRANS_DEF_NUM:		return "LOG_SIP_TRANS";
		case LOG_MSRP_TRANS_DEF_NUM:	return "LOG_MSRP_TRANS";
		case LOG_HTTP_TRANS_DEF_NUM:	return "LOG_HTTP_TRANS";
		case LOG_PAGE_TRANS_DEF_NUM:	return "LOG_PAGE_TRANS";
		case LOG_ONLINE_TRANS_DEF_NUM:	return "LOG_ONLINE_TRANS";
		case LOG_JNC_TRANS_DEF_NUM:		return "LOG_JNC_TRANS";
		case LOG_IV_DEF_NUM:			return "LOG_IV";
		case LOG_VOD_SESS_DEF_NUM:		return "LOG_VOD_SESS";
		case START_SERVICE_DEF_NUM:		return "START_SERVICE";
		case LOG_DNS_DEF_NUM: 			return "LOG_DNS";
		case LOG_DIALUP_SESS_DEF_NUM: 	return "LOG_DIALUP_SESS";
		default: 						return "UNKNOWN";
	}

	return "UNKNOWN";
}

char *PrintTYPE(int type)
{
	switch(type)
	{
		case START_CALL_NUM:
		case LOG_PISIGNAL_DEF_NUM:
		case START_SERVICE_DEF_NUM:
		case STOP_CALL_NUM:
		case START_PI_DATA_RECALL_NUM:
		case START_RP_DATA_RECALL_NUM:
		case START_PI_SIG_RECALL_NUM:
		case START_RP_SIG_RECALL_NUM:
		case STOP_PI_RECALL_NUM:
		case STOP_RP_RECALL_NUM:
			return PRINT_TAG_DEF_ALL_CALL_INPUT(type);
		default:
			return PRINT_DEF_NUM_table_log(type);
	}
}

/*
 * $Log: ci_log_util.c,v $
 * Revision 1.2  2011/09/06 12:46:38  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.7  2011/04/21 08:21:25  dark264sh
 * CI_LOG: PrintTYPE 함수 추가
 *
 * Revision 1.6  2011/01/11 04:09:12  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.5  2009/08/06 06:56:10  dqms
 * 로그레벨 공유메모리로 수정
 *
 * Revision 1.4  2009/08/01 09:28:28  dqms
 * *** empty log message ***
 *
 * Revision 1.3  2009/08/01 08:46:11  dqms
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/13 18:29:06  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1  2009/06/13 16:11:02  jsyoon
 * *** empty log message ***
 *
 **/
