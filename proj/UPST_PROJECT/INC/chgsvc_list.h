/*******************************************************************************
			DQMS Project

	Author   : 
	Section  : DQMS Project
	SCCS ID  : @(#)chgsvc_list.h	1.1
	Date     : 
	Revision History :


	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __CHGSVC_LIST_H__
#define __CHGSVC_LIST_H__

#include <define.h>			/* MAX_HOSTNAME_SIZE */

#define MAX_IPLIST_CNT			128
#define MAX_HTTPLOG_LIST_CNT	200003
#define MAX_TCPLOG_LIST_CNT		200003	
#define DEF_HOSTNAME_SIZE		64

typedef struct _st_IPLIST_
{
	char    szHostname[MAX_HOSTNAME_SIZE];
	UINT    uiCnt;
	UINT    uiReserved;
	UINT    uiIPList[MAX_IPLIST_CNT];
} st_IPLIST, *pst_IPLIST;
#define DEF_IPLIST_SIZE         sizeof(st_IPLIST)

typedef struct _st_HTTPLOG_LIST_
{
	long long	llPktCnt;
	UINT        uiArrayIndex;       /* IPList ¿Œµ¶Ω∫ ¡§∫∏ */
	UINT		uiReserved;
} st_HTTPLOG_LIST, *pst_HTTPLOG_LIST;
#define DEF_HTTPLOG_LIST_SIZE       sizeof(st_HTTPLOG_LIST)

typedef struct _st_TCPLOG_LIST_
{
	long long	llPktCnt;
	UINT        uiIP;
	UINT		uiReserved;
} st_TCPLOG_LIST, *pst_TCPLOG_LIST;
#define DEF_TCPLOG_LIST_SIZE        sizeof(st_TCPLOG_LIST)

typedef struct _st_TOTLOG_LIST_
{
	UINT                uiHttpLogCnt;
	UINT                uiTcpLogCnt;
	st_HTTPLOG_LIST		stHttpLogList[MAX_HTTPLOG_LIST_CNT];
	st_IPLIST           stIPList[MAX_HTTPLOG_LIST_CNT];
	st_TCPLOG_LIST      stTcpLogList[MAX_TCPLOG_LIST_CNT];
} st_TOTLOG_LIST, *pst_TOTLOG_LIST;
#define DEF_TOTLOG_LIST_SIZE        sizeof(st_TOTLOG_LIST)


#endif	/*	__CHGSVC_LIST_H__ */

