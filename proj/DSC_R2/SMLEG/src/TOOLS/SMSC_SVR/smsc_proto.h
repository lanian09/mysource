#ifndef __SMSC_PROTO_H__
#define __SMSC_PROTO_H__

#include <sys/errno.h>
#include <sys/msg.h>
#include <sys/sysinfo.h>
#include <sys/shm.h>

#include "socklib.h"
#include "com_smpp_if.h"


#define SYSCONF_MAX_ASSO_SYS_NUM        3
#define COMM_MAX_NAME_LEN   			16  
#define COMM_MAX_VALUE_LEN  			100

#define MY_SYS_NAME             "MY_SYS_NAME"
#define IV_HOME                 "IV_HOME"
#define SYSCONF_FILE            "NEW/DATA/smpp.conf"

#define SMSC_LOG_FILE			"APPLOG/OAM/smsc_log"
#define SMSC_ERRLOG_FILE		"APPLOG/OAM/smsc_err"

#define FL  __FILE__, __LINE__

#define LOGLIB_MAX_OPEN_FILE    16      /* �� ���μ������� �ִ�� open�� �� �ִ� log file ���� */
#define LOGLIB_MAX_FILE_SIZE    1024000 /* LOGLIB_MODE_LIMIT_SIZE�� ��� ���� ũ�� ���� (byte����) */
#define LOGLIB_MAX_LOG_SUFFIX   10      /* LOGLIB_MODE_LIMIT_SIZE�� ��� �α� ������ ����� ���� ������ ���� */
            
#define LOGLIB_MODE_LIMIT_SIZE      0x00000001  /* ȭ�� ũ�⸦ ���� �ϴ� ��� */
#define LOGLIB_MODE_DAILY           0x00000002  /* ���� ���ο� ȭ���� �����ϴ� ��� */
#define LOGLIB_MODE_HOURLY          0x00000004  /* �Ž� ���ο� ȭ���� �����ϴ� ��� */
#define LOGLIB_MODE_ONE_DIR         0x00000008  /* YYYY.mm.dd �����ϴ� ��� */
#define LOGLIB_MODE_7DAYS           0x00010000  /* 1������ ���� ���� �ڵ� ����  */
#define LOGLIB_FLUSH_IMMEDIATE      0x00020000  /* �Ź� fflush �Ѵ�. */
#define LOGLIB_FNAME_LNUM           0x00040000  /* �ҽ� �����̸��� line_number ��� */
#define LOGLIB_TIME_STAMP           0x00080000  /* �ð�(time_stamp) ��� */
            
#define TRCBUF_LEN  8192 
#define TRCTMP_LEN  1024


extern int	errno;

extern int ixpc_initial (void);
extern int ixpc_initLog (void);
extern int ixpc_newConnEvent (int);
extern int ixpc_recvEventRxPort (int, SockLibMsgType*);
extern int ixpc_recvEventTxPort (int);
extern int ixpc_disconnEventRxPort (int);
extern int ixpc_disconnEventTxPort (int);

int smsc_user_input(int msg_type, int *result);
int smsc_check_input(int type, int result);
char *smsc_get_msgtype(int type);
void smsc_printf_menu(int msg_type);


#endif /*__SMSC_PROTO_H__*/
