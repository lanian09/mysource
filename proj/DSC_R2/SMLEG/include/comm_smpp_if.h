#ifndef __COM_SMPP_IF_H__
#define __COM_SMPP_IF_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


//////////////////////////////////////////////////////////	
// SMSC INTERFACE
//////////////////////////////////////////////////////////	
/* SMSC와 주고받는 Packet Format */
#define SMPP_HEAD_LEN   				8
#define SMPP_MSG_ID_LEN					16
#define SMPP_MSG_PWD_LEN				16
#define SMPP_MSG_PRIFIX_LEN				16
#define SMPP_MSG_ADDR_LEN				32
#define SMPP_MSG_TEXT_LEN				160
#define SMPP_MSG_DELIVER_TIME_LEN		20
#define SMPP_MSG_DEST_CODE_LEN			12


/* SMSC RESULT CODE */
/* BIND ACK RESULT CODE */
enum {BIND_E_OK=0
	, BIND_E_SYSFAIL
	, BIND_E_AUTH_FAIL
	, BIND_E_FORMAT_ERR };

/* DELIVER ACK RESULT CODE */
enum {DELIVER_E_OK=0
	, DELIVER_E_SYSFAIL
	, DELIVER_E_AUTH_FAIL
	, DELIVER_E_FORMAT_ERR
	, DELIVER_E_NOT_BOUND
	, DELIVER_E_NO_DESTIN
	, DELIVER_E_SENT
	, DELIVER_E_EXPIRED
	, DELIVER_E_INVALID_TERM=11
	, DELIVER_E_OVERFLOW=12 };

/* REPORT RESULT CODE */
enum {REPORT_E_SEND=6
	, REPORT_E_INVALIDDST
	, REPORT_E_POWEROFF
	, REPORT_E_HIDDEN
	, REPORT_E_TERMFUL
	, REPORT_E_ETC
	, REPORT_E_PORTED_OUT=13
	, REPORT_E_ROAMING_FAIL=25 };

/* REPORT ACK RESULT CODE */
enum {REPORT_E_OK=0
	, REPORT_E_SYSFAIL
	, REPORT_E_NOT_BOUND=4};


/* SMSC MSG TYPE */
enum {SMPP_BIND_MSG=0
	, SMPP_BIND_ACK_MSG
	, SMPP_DELIVER_MSG
	, SMPP_DELIVER_ACK_MSG
	, SMPP_REPORT_MSG
	, SMPP_REPORT_ACK_MSG };

typedef struct {
	char	sysid[16];
	int		tid;
	char	org_addr[SMPP_MSG_ADDR_LEN];
	char	callback[SMPP_MSG_ADDR_LEN];
} SMPP_MSG_INFO;


typedef struct {
	int		type;
	int		len;
} SMPP_MSG_H;

#define	SMS_MSG_H_LEN sizeof(SMPP_MSG_H)

typedef struct {
	SMPP_MSG_H	header;
	char 	id[SMPP_MSG_ID_LEN];
	char	pwd[SMPP_MSG_PWD_LEN];
} SMPP_BIND;

typedef struct {
	SMPP_MSG_H	header;
	int		result;
	char	prefix[SMPP_MSG_PRIFIX_LEN];
} SMPP_BIND_ACK;

typedef struct {
	SMPP_MSG_H	header;
	int		tid;
	char	org_addr[SMPP_MSG_ADDR_LEN];
	char	dst_addr[SMPP_MSG_ADDR_LEN];
	char	callback[SMPP_MSG_ADDR_LEN];
	char	text[SMPP_MSG_TEXT_LEN];
	int		sn;	/* serial number */
} SMPP_DELIVER;

typedef struct {
	SMPP_MSG_H	header;
	int		result;
	char	org_addr[SMPP_MSG_ADDR_LEN];
	char	dst_addr[SMPP_MSG_ADDR_LEN];
	int		sn;
} SMPP_DELIVER_ACK;

typedef struct {
	SMPP_MSG_H	header;
	int		result;
	char	org_addr[SMPP_MSG_ADDR_LEN];
	char	dst_addr[SMPP_MSG_ADDR_LEN];
	int		sn;
	char	deliver_t[SMPP_MSG_DELIVER_TIME_LEN];
	char	dest_code[SMPP_MSG_DEST_CODE_LEN];
} SMPP_REPORT;

typedef struct {
	SMPP_MSG_H	header;
	int		result;
} SMPP_REPORT_ACK;
//////////////////////////////////////////////////////////	


#if 1
////////////////////////////////////////////////////////////////////
// include/comm_msgtype.h 에 define.
// RDRANA -> SMPP 간 SMS INFORMATION STRUCTURE (MSG_Q BODY)
typedef struct __st_sms_info__ {
#define SMS_MAX_SUBSID_LEN          16			//64 -> 16
#define SMS_MAX_MSG_LEN             160
#define SMS_MAX_IP_LEN				16
	unsigned short  sendFlag;
	unsigned short  pkgID;
	unsigned short  sPBit;
	unsigned short  sHBit;
	unsigned int    blkTm;
	unsigned char   subsID[SMS_MAX_SUBSID_LEN];
	unsigned char	subsIP[SMS_MAX_IP_LEN];
	unsigned char   smsMsg[SMPP_MSG_TEXT_LEN];
} SMS_INFO;

#define SMS_MSG_LEN     sizeof(SMS_INFO)
////////////////////////////////////////////////////////////////////
// SMS HISTORY MANAGE STRUCTURE (DB TABLE SCHEMA)
typedef struct __st_sms_his__ {
	SMS_INFO        info;
	unsigned int    delivTm;
	unsigned short  delivSts;
	unsigned char	reportTm[SMPP_MSG_DELIVER_TIME_LEN];
	unsigned short  reportSts;
	unsigned int    cid;
} SMS_HIS;
#endif
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
// RDRANA -> SMPP 간 SMS INFORMATION STRUCTURE (MSG_Q BODY)

typedef struct __st_sms_msg__ {
	long        mtype;
	SMS_INFO    sms;
} SmsMsgType;

#endif /* __COM_SMPP_IF_H__ */

