
#ifndef __COMM_SESSION_H__
#define __COMM_SESSION_H__

#include "memg.h"
#include "hasho.h"
#include "timerN.h"

#define RAD_HASHO_OFFSET(INFOPTR, ptr)  ((OFFSET) (((U8 *) (ptr)) - ((U8 *) INFOPTR)) )
#define RAD_HASHO_PTR(INFOPTR, offset)  ((U8 *) (((OFFSET) (offset)) + ((OFFSET) INFOPTR)) ) 

#define MAX_RAD_HASHO_SIZE		524287
//#define MAX_RAD_HASHO_SIZE		300000

/* MODIFY: by june, 2010-09-17
 * - HASH KEY ����: FramedIP --> Calling Station ID
 *                  Disconnect_Req �޽����� �߰��Ǿ����� �ش� �޽�������
 *                  FramedIP Field�� ���� KEY ����.
 * - HASH BODY����: SCE LOGON/OUT Operation�� �ʿ��� ����Ÿ �ʵ� �߰�.
 * - LINE		  : 22 --> 
 */
#if 0
typedef struct _rad_sess_key_ {
	U32		mobIP;
} rad_sess_key;
#else
#define MAX_CSID_SIZE			16
typedef struct _rad_sess_key_ {
	U8				szCSID[MAX_CSID_SIZE];		/* Calling Station ID: MAX Len 15 */
} rad_sess_key;
#endif
#define RAD_SESS_KEY_SIZE	sizeof(rad_sess_key)

#define MAX_SESS_IMSI_SIZE		16
#if 0
typedef struct _rad_sess_body_ {
	U8				IMSI[MAX_SESS_IMSI_SIZE];
	TIMERNID		timerID;
	SHORT			sPkgID;
	UINT            uiCBit;        
	UINT            uiPBit;        
	UINT            uiHBit;
	UINT			uiDoneLogOnF;				/* Log on �� call �� ���� ǥ�� */
} rad_sess_body;
#else
typedef struct _rad_sess_body_ {
	UINT    		uiFramedIP;
	UINT            uiCBit;        
	UINT            uiPBit;        
	UINT            uiHBit;
	SHORT			sPkgID;
	SHORT			reserve;
	UINT			uiDoneLogOnF;				/* Log on �� call �� ���� ǥ�� */
	TIMERNID		timerID;
	INT				dConnID;	// ROUTE RLEG Index
	INT				dTrcFlag;
	//U8				IMSI[MAX_SESS_IMSI_SIZE];
} rad_sess_body;
#endif
#define RAD_SESS_BODY_SIZE	sizeof(rad_sess_body)

#define MAX_RLEG_CNT	1
typedef struct _mem_count_ {
	UINT rad_sess;
	UINT sm_sess[MAX_RLEG_CNT];
} _mem_check;
#define MEM_CHECK_SIZE	sizeof(_mem_check)

#endif /* __COMM_SESSION_H__ */
