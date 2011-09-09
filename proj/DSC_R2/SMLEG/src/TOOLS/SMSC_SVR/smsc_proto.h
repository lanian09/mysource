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

#define LOGLIB_MAX_OPEN_FILE    16      /* 한 프로세스에서 최대로 open할 수 있는 log file 갯수 */
#define LOGLIB_MAX_FILE_SIZE    1024000 /* LOGLIB_MODE_LIMIT_SIZE의 경우 파일 크기 제한 (byte단위) */
#define LOGLIB_MAX_LOG_SUFFIX   10      /* LOGLIB_MODE_LIMIT_SIZE의 경우 로그 파일을 몇개까지 만들 것인지 정의 */
            
#define LOGLIB_MODE_LIMIT_SIZE      0x00000001  /* 화일 크기를 제한 하는 경우 */
#define LOGLIB_MODE_DAILY           0x00000002  /* 매일 새로운 화일을 생성하는 경우 */
#define LOGLIB_MODE_HOURLY          0x00000004  /* 매시 새로운 화일을 생성하는 경우 */
#define LOGLIB_MODE_ONE_DIR         0x00000008  /* YYYY.mm.dd 생성하는 경우 */
#define LOGLIB_MODE_7DAYS           0x00010000  /* 1주일이 지난 파일 자동 삭제  */
#define LOGLIB_FLUSH_IMMEDIATE      0x00020000  /* 매번 fflush 한다. */
#define LOGLIB_FNAME_LNUM           0x00040000  /* 소스 파일이름과 line_number 기록 */
#define LOGLIB_TIME_STAMP           0x00080000  /* 시각(time_stamp) 기록 */
            
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
