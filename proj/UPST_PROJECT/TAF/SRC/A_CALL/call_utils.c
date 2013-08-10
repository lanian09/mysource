/**		@file	call_main.c
 * 		- CALL 정보를 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: call_utils.c,v 1.1 2011/09/04 08:04:51 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.1 $
 * 		@date		$Date: 2011/09/04 08:04:51 $
 * 		@warning	.
 * 		@ref		call_main.c call_init.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- CALL 정보를 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "typedef.h"
#include "common_stg.h"


U8 *PrintTYPE(S32 type)
{
	switch(type)
	{
	case LOG_HTTP_TRANS_DEF_NUM: 		return (U8 *)"LOG_HTTP_TRANS";
	case LOG_TCP_SESS_DEF_NUM: 			return (U8 *)"LOG_TCP_SESS";
	case TCP_INFO_DEF_NUM: 				return (U8 *)"TCP_INFO";
	case BODY_DEF_NUM: 					return (U8 *)"BODY";
	case LOG_RTSP_TRANS_DEF_NUM:		return (U8 *)"LOG_RTSP_TRANS";
	case LOG_PAGE_TRANS_DEF_NUM:		return (U8 *)"LOG_PAGE_TRANS";
	case LOG_CALL_TRANS_DEF_NUM:		return (U8 *)"LOG_CALL_TRANS";
	case LOG_JNC_TRANS_DEF_NUM:			return (U8 *)"LOG_JNC_TRANS";
	case LOG_VOD_SESS_DEF_NUM:			return (U8 *)"LOG_VOD_SESS";
	default: 							return (U8 *)"UNKNOWN";
	}
}

U8 *PrintTcpFlag(S32 tcpflag)
{
	switch(tcpflag)
	{
	case DEF_TCP_START: 				return (U8 *)"TCP_START";
	case DEF_TCP_END: 					return (U8 *)"TCP_END";
	default: 							return (U8 *)"UNKNOWN";
	}
}	

void for_debugging ()
{
	int i = 0;
	i++;
}


/*
 *  $Log: call_utils.c,v $
 *  Revision 1.1  2011/09/04 08:04:51  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *
 */



