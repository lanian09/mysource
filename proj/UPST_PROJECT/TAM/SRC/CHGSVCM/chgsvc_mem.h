/*******************************************************************************
               DQMS Project

     Author   :
     Section  :
     SCCS ID  :
     Date     :
     Revision History :

     Description :

     Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __CHGSVCM_MEM_H__
#define __CHGSVCM_MEM_H__
/** A.1* FILE INCLUDE *************************************/

/* User Define */
#include <typedef.h>
#include <chgsvc_list.h>


/** B.1* DEFINITION OF NEW CONSTANTS **********************/
#define MAX_HTTPLOG_HASH_CNT	100003
#define MAX_TCPLOG_HASH_CNT		100003
#define MAX_IPLIST_HASH_CNT		100003
#define MAX_GOOGLE_PUSH_CNT		1024 


/** C.1* DEFINITION OF NEW TYPES **************************/


/** HASH ******/
typedef struct _st_HTTPLOG_KEY_
{
	char	szHostname[DEF_HOSTNAME_SIZE];
} st_HTTPLOG_KEY, *pst_HTTPLOG_KEY;
#define DEF_HTTPLOG_KEY_SIZE		sizeof(st_HTTPLOG_KEY)

typedef struct _st_HTTPLOG_DATA_
{
	UINT	uiArrayIndex;
} st_HTTPLOG_DATA, *pst_HTTPLOG_DATA;
#define DEF_HTTPLOG_DATA_SIZE		sizeof(st_HTTPLOG_DATA)

typedef struct _st_TCPLOG_KEY_
{
	UINT	uiIP;
} st_TCPLOG_KEY, *pst_TCPLOG_KEY;
#define DEF_TCPLOG_KEY_SIZE		sizeof(st_TCPLOG_KEY)

typedef struct _st_TCPLOG_DATA_
{
	UINT	uiArrayIndex;
} st_TCPLOG_DATA, *pst_TCPLOG_DATA;
#define DEF_TCPLOG_DATA_SIZE		sizeof(st_TCPLOG_DATA)

typedef struct _st_IPLIST_KEY_
{
	UINT	uiIP;
} st_IPLIST_KEY, *pst_IPLIST_KEY;
#define DEF_IPLIST_KEY_SIZE		sizeof(st_IPLIST_KEY)

typedef struct _st_IPLIST_DATA_
{
	UINT	uiIPIndex;
} st_IPLIST_DATA, *pst_IPLIST_DATA;
#define DEF_IPLIST_DATA_SIZE		sizeof(st_IPLIST_DATA)

typedef struct _st_GOOGLEPUSH_KEY_
{
	UINT	uiIP;
} st_GOOGLEPUSH_KEY, *pst_GOOGLEPUSH_KEY;
#define DEF_GOOGLEPUSH_KEY_SIZE     sizeof(st_GOOGLEPUSH_KEY)

typedef struct _st_GOOGLEPUSH_DATA_
{
	UINT	uiReserved;
} st_GOOGLEPUSH_DATA, *pst_GOOGLEPUSH_DATA;
#define DEF_GOOGLEPUSH_DATA_SIZE    sizeof(st_GOOGLEPUSH_DATA)

typedef struct _st_GOOGLEPUSH_MEM_
{
	int		dCnt;
	int		dReserved;
	UINT	IPList[MAX_GOOGLE_PUSH_CNT];
} st_GOOGLEPUSH_MEM, *pst_GOOGLEPUSH_MEM;
#define DEF_GOOGLEPUSH_MEM_SIZE     sizeof(st_GOOGLEPUSH_MEM)



/** D.1* DECLARATION OF VARIABLES *************************/
/** E.1* DEFINITION OF FUNCTIONS **************************/
extern int	dInitMem(void);
extern void	CleanHttpLogMem(void);
extern void	CleanTcpLogMem(void);
extern int	dSetHttpLogHash(char *hostname, UINT IP, long long pktcnt);
extern int	dSetTcpLogHash(UINT IP, long long pktcnt);
extern void	HttpLogQuickSort(st_HTTPLOG_LIST httplog[], int low, int high);
extern void	TcpLogQuickSort(st_TCPLOG_LIST tcplog[], int low, int high);
extern int	dAddGooglePushHash(UINT IP);

#endif /* __CHGSVCM_MEM_H__ */

