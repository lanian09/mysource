/**		@file	platform_type.c
 * 		- Util 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: platform_type.c,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 *
 * 		@Author		$Author: dcham $
 * 		@version	$Revision: 1.1.1.1 $
 * 		@date		$Date: 2011/08/29 05:56:42 $
 * 		@ref		
 * 		@todo	
 *
 * 		@section	Intro(소개)
 * 		- Util 함수들
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <stdio.h>
#include <common_stg.h>

#include "tools.h"
#include "loglib.h"

S32 dGetPlatformType(S32 L4Code, S32 L7Code)
{
	S32		platformType = 0;

	switch(L4Code)
	{
		case L4_CORP:
			platformType = DEF_PLATFORM_CORP;
			break;
		case L4_WIPI:
			switch(L7Code)
			{
				case APP_MENU:
					platformType = DEF_PLATFORM_MENU;
					break;
				case APP_DOWN:
					platformType = DEF_PLATFORM_DN;
					break;
				default:
					log_print(LOGN_CRI, "[%s][%s.%d] UNKNOWN L7CODE[%d]", __FILE__, __FUNCTION__, __LINE__, L7Code);
					platformType = 0;
			}
			break;
		case L4_FB:
		case L4_IV:
		case L4_WAP20:
			platformType = DEF_PLATFORM_MENU;
			break;
		case L4_DN_2G:
		case L4_DN_2G_NODN:
		case L4_DN_JAVA:
		case L4_DN_VOD:
		case L4_DN_VOD_NODN:
		case L4_OMA_DN:
		case L4_OMA_DN_2G:
		case L4_OMA_DN_VOD:
		case L4_OMA_DN_WIPI:
			platformType = DEF_PLATFORM_DN;
			break;
		case L4_VOD_STREAM:
		case L4_RTS_FB:
		case L4_RTS_WB:
		case L4_MBOX:
			platformType = DEF_PLATFORM_STREAM;
			break;
		case L4_MMS_UP:
		case L4_MMS_UP_NODN:
		case L4_MMS_DN:
		case L4_MMS_DN_NODN:
		case L4_MMS_NEW:
			platformType = DEF_PLATFORM_MMS;
			break;
		case L4_TODAY:
		case L4_WIDGET:
			platformType = DEF_PLATFORM_WIDGET;
			break;
		case L4_EMS:
		case L4_P_EMS:
		case L4_EMS_NO:
			platformType = DEF_PLATFORM_EMS;
			break;
		case L4_FV_FB:
		case L4_FV_IV:
		case L4_FV_EMS:
			platformType = DEF_PLATFORM_FV;
			break;
		case L4_SIP_MS:
		case L4_SIP_VENDOR:
		case L4_SIP_CSCF:
		case L4_MSRP_MS:
		case L4_MSRP_VENDOR:
		case L4_XCAP:
			platformType = DEF_PLATFORM_IM;
			break;
		case L4_VT:
			platformType = DEF_PLATFORM_VT;
			break;
		case L4_JNC:
		case L4_FTP:
		case L4_DNS:
		case L4_WIPI_ONLINE:
			platformType = DEF_PLATFORM_ETC;
			break;
		case L4_BANKON:
		case L4_VMBANK:
			platformType = DEF_PLATFORM_BANK;
			break;
		case L4_INET_TCP:
		case L4_INET_TCP_RECV:
		case L4_INET_HTTP:
		case L4_INET_HTTP_RECV:
			platformType = DEF_PLATFORM_INET;
			break;
		default:
			log_print(LOGN_CRI, "[%s][%s.%d] UNKNOWN L4CODE[%d]", __FILE__, __FUNCTION__, __LINE__, L4Code);
			platformType = 0;
			break;
	}
	return platformType;
}

#if 0
S32 dGetPlatformTypeOld(S32 l4Code)
{
	S32		platformType = 0;

	switch(l4Code)
	{
	case L4_WAP20:
	case L4_TODAY:
		platformType = DEF_PLATFORM_WAP20;
		break;
	case L4_WIPI:
		platformType = DEF_PLATFORM_WIPI;
		break;
	case L4_DN_2G:
	case L4_DN_JAVA:
	case L4_DN_VOD:
		platformType = DEF_PLATFORM_DN;
		break;
	case L4_VOD_STREAM:
		platformType = DEF_PLATFORM_VOD;
		break;
	case L4_MMS_UP:
	case L4_MMS_DN:
		platformType = DEF_PLATFORM_MMS;
		break;
	case L4_JNC:
		platformType = DEF_PLATFORM_JNC;
		break;
	case L4_FV_FB:
	case L4_FV_EMS:
	case L4_FV_IV:
		platformType = DEF_PLATFORM_FV;
		break;
	case L4_EMS:
		platformType = DEF_PLATFORM_EMS;
		break;
	case L4_FB:
		platformType = DEF_PLATFORM_FB;
		break;
	case L4_XCAP:
		platformType = DEF_PLATFORM_IMS;
		break;
	case L4_WIDGET:
		platformType = DEF_PLATFORM_WIDGET;
		break;
	case L4_MBOX:
		platformType = DEF_PLATFORM_INET;
		break;
	default:
		log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN L4CODE[%d]", __FILE__, __FUNCTION__, __LINE__, l4Code);
		platformType = 0;
		break;
	}
	return platformType;
}
#endif

/*
 * $Log: platform_type.c,v $
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.2  2011/08/05 09:04:49  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.9  2011/04/18 07:15:26  dark264sh
 * L4_INET 변경에 따른 수정
 *
 * Revision 1.8  2011/04/16 09:36:06  dark264sh
 * UTILLIB: L4_INET, L4_INET_RECV 처리 추가
 *
 * Revision 1.7  2011/01/11 04:09:05  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:04  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.6  2009/09/07 08:28:24  jsyoon
 * DEF_PLATFORM_CORP 추가
 *
 * Revision 1.5  2009/08/17 15:20:10  jsyoon
 * ADD L4_DN_2G_NODN L4_DN_VOD_NODN IN DN PLATFORM
 *
 * Revision 1.4  2009/07/22 06:25:21  dqms
 * *** empty log message ***
 *
 * Revision 1.3  2009/07/15 16:13:01  dqms
 * ADD EMS
 *
 * Revision 1.2  2009/07/11 16:13:31  dqms
 * ADD L4_FTP
 *
 * Revision 1.1  2009/07/11 10:45:54  dqms
 * Add dGetPlatformType Function
 *
 */
