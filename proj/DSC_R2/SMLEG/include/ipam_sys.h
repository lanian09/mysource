/* @(#)toto_sys.h	1.3 10/23/01 
	All Rights are reserved by ABLEX 2001
*/
#ifndef _IPAM_____________SYS_H
#define _IPAM_____________SYS_H

/*System Msgessage Definition*/


/*System Service ID Definition*/
#define SID_IPAF_MNG		11
#define SID_SERV_MNG		31

#define SID_STATUS			100
#define SID_CHG_INFO		101 /* 프로세스의 정보 변경을 위한 메시지 */
#define SID_DIS_INFO        102 /* mmcd 정보 조회를 위한 메시지 */
#define SID_MML             103
#define	SID_LINK			104
#define SID_TRACE			105

#define SID_IPAF_INFO		106
#define SID_IPAF_CHK		120

#define 	SID_VER             110

#define     MID_VER_STATUS      	1
#define     MID_VER_GET_PATCH   	2
#define     MID_VER_UPDATE_PATCH    3
#define     MID_VER_ROLLBACK_PATCH  4
#define     MID_VER_REBOOT      	5

/* DEFINITION : PARTIAL PKG */
#define 	MID_VER_PAR_DOWN		10
#define 	MID_VER_PAR_PATCH		11
#define		MID_VER_PAR_ROLLBACK	12

#define MID_IP_POOL			30
#define MID_SERV_GRUP		21

/*System Message ID Definition*/
#define MID_STATUS_ALMD		1
#define MID_STATUS_COND		2
#define MID_STATUS_STAT		3
#define MID_STATUS_LINK		4

/* 20040419, sunny : http ipaf sessana & svc block stat */
#define MID_STATUS_STAT_SESS 	 0x05
#define MID_STATUS_STAT_SVC	 	 0x06
#define MID_STATUS_STAT_NEWSVC   0x07

#define MID_MML_REQ			1
#define MID_MML_RST			2
#define MID_MML_ACK			3
#define MID_MML_CHK			4
#define MID_MML_CHNL		5

#define MID_CHG_IPPOOL		1
#define MID_CHG_SERGRP		2
#define MID_CHG_TIME		3
#define MID_CHG_PROC_PARM	4
#define MID_CHG_AN_IPPOOL   5  /*20070104 Add(AN_AAAIF.conf 변경 noti */

#define MID_CHG_PDSN   		6  /* jjinri */


#define MID_LINK_IPAF		1

#define MID_TRC				100

#define MID_IPAF_CHK_SVC	1
#define MID_IPAF_CHK_IPPOOL	2

/* 20040505,poopee */
#define	MID_CHG_IPC_RTE		40
#define	MID_CHG_IPC_INFO	41
#define	MID_CHG_IPC_TMR		42

/* 20040511, sunny */
#define	MID_ADD_IPC_INFO	43
#define	MID_DEL_IPC_INFO	44

#pragma pack(1)
typedef struct _st_SysMsgQ
{
	long int	llMType;
	INT64		llNID;
	USHORT		usBodyLen;
	USHORT		usRetCode;
	UINT		uiReserved;
	UCHAR		szBody[MAX_MSGBODY_SIZE];
}st_SysMsgQ, *pst_SysMsgQ;

#pragma pack()
#define DEF_SYSMSGQ_SIZE	sizeof(st_SysMsgQ)
#define DEF_SYSMSGHEAD_LEN	(sizeof(st_SysMsgQ) - MAX_MSGBODY_SIZE)

#endif
