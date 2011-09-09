/**		@file	m_svcmon_log.c
 * 		- M_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: m_svcmon_util.c,v 1.4 2011/09/07 07:07:51 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.4 $
 * 		@date		$Date: 2011/09/07 07:07:51 $
 * 		@ref		m_svcmon_init.c m_svcmon_maic.c
 *
 * 		@section	Intro(소개)
 * 		- M_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		@section	Requirement
 *
 **/

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>

#include "m_svcmon_func.h"
#include "m_svcmon_util.h"

/*
 * $Log: m_svcmon_util.c,v $
 * Revision 1.4  2011/09/07 07:07:51  hhbaek
 * *** empty log message ***
 *
 * Revision 1.3  2011/09/07 04:31:59  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/01 07:49:50  dcham
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.1  2011/08/23 10:59:21  dcham
 * *** empty log message ***
 *
 * Revision 1.4  2011/01/11 04:09:17  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:10  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.3  2010/03/29 12:22:41  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2009/07/14 06:42:38  dark264sh
 * M_SVCMON 파일 저장 ASCII로 변경
 *
 * Revision 1.1  2009/06/16 08:05:51  dark264sh
 * M_SVCMON 기본 동작 처리
 *
 */
