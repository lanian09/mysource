/**********************************************************
                 ABLEX Main-Memory DBMS

   Author   : LEE SANG HO
   Section  : IPAS Project
   SCCS ID  : %W%
   Date     : %G%
   Revision History :
        '01.  9. 19     Initial

   Description:
        Recovery

   Copyright (c) ABLEX 2001
***********************************************************/

#ifndef __SESSSVC__DEFINE___
#define __SESSSVC__DEFINE___  

/**A.1*  File Inclusion ***********************************/

#include <ipaf_svc.h>
#include <sys/types.h>

/* Define Call Flow Service Status */
#define SVC_ACCGS_STAT		1		/* General Accounting Flow : Start */
#define SVC_ACCGE_STAT		2		/* General Accounting Flow : Stop */
#define SVC_ACCPS_STAT		3		/* PPS Accounting Start Flow From PDSN */
#define SVC_ACCIS_STAT		4		/* PPS Accounting Start Flow From IWF */
#define SVC_ACCPE_STAT		5		/* PPS Accounting Stop Flow From PDSN */
#define SVC_ACCIE_STAT		5		/* PPS Accounting Stop Flow Form IWF */
#define SVC_QUD_STAT		6		/* QuD Flow */
#define SVC_ACCOFF_STAT		7		/* ACCOUNTING OFF FLOW */
#define SVC_WININFO_STAT	8		/* WIN INFO REQUEST */
#define SVC_CDR_STAT		9		/* CDR INFO To AAA */

/* Define Call Flow Message Status */
#define MSG_REQ_IPAF			1		/* Request To IPAFUIF */
#define MSG_RES_IPAF			2		/* Response To IPAFUIF */
#define MSG_REQ_DSCP			3		/* Request To DSCPIF */ 
#define MSG_RES_DSCP			4		/* Response To DSCPIF */ 
#define MSG_REQ_QUD				5		/* Request To QUDIF */
#define MSG_RES_QUD				6		/* Response To QUDIF */
#define MSG_REQ_AAA				7		/* Request To AAAIF */
#define MSG_RES_AAA				8		/* Response To AAAIF */


#define MAX_SESSINTER_COUNT	5
//#define MAX_TRACE_NUM       5
#define MAX_TRACE_NUM       10 
#define TYPE_IMSI			1
#define IP_TYPE				2

typedef struct _st_TrcInfo
{
#define MAX_IMSI_LEN 16
    time_t      tRegTime;
    int         dType; /* 1:IMSI, 2:IP */
////    INT64       llInfo;
    int         dDura;
	char		szImsi[MAX_IMSI_LEN];
////    int         dTrcLevel;
} st_TrcInfo, *pst_TrcInfo;

/* SESSSVC Local Memory Information */
typedef struct _st_SESSInfo
{
////	INT64		pllRLastNID[MAX_SESSINTER_COUNT];		/* Interface Block Received Last NID */ 
////	INT64		pllSLastNID[MAX_SESSINTER_COUNT];		/* Interface Block Send Last NID */ 
	INT			dTCount;								/* MIN Watch Count */ 
	st_TrcInfo	stTrc[MAX_TRACE_NUM];
////	char		szLastLogFile[MAX_FILENAME_SIZE];
} st_SESSInfo, *pst_SESSInfo;


/******************* LINKED LIST SESSMSG DEFINITION ****************/
#define MAX_SESSMSG_LIST	10000
#define SESSMSG_CID_TIMEOUT	5

#endif
