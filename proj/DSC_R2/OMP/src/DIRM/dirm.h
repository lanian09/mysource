#ifndef __DIRM_H__
#define __DIRM_H__

#include <ctype.h>

#include "sysconf.h"
#include "commlib.h"
#include "sfm_msgtypes.h"
#include "stm_msgtypes.h"
#include "sfm_snmp.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include "comm_almsts_msgcode.h"
#include <proc_version.h>
#include "comm_msgtypes.h"
#include "socklib.h"

#include <sys/types.h>
#include <sys/shm.h>
#include <sock.h>

#include "netOrderL3pdChange.h"
#include "netOrderChange.h"
#include "netOrderSceInfoChange.h"

#ifdef TRU64
#include <sys/old_procfs.h>
#define	SHIFTPAGE		3
#define pagetok(n)		((n) << SHIFTPAGE)
#else
#include <sys/procfs.h>
#endif

#define NEXT(a,b)	((a+1)%b)

#define PING_CNT		2

#define NTP_DAEMON_DOWN_COUNT	3

#define DIRM_LOG_FILE       "LOG/OAM/dirm_log"
#define DIRM_ERRLOG_FILE    "LOG/OAM/dirm_err"

#define	MAX_CPU_CNT			8
#define	MAX_DISK_CNT		10
/** T_loc_sadb에서 사용하기 위한 값 종료 */

#define KEEPALIVE_CHECK_TIME     60
#define MSGQ_CLEAR_TIME		3

#define MAX_STR_LEN     32
#define MAX_FILE_NAME_LEN   128
#define MAX_SYS_CNT     3

extern int dirm_initLog (void);
extern int dirm_getL3pd (int key);
extern int dirm_get_snmp_rdr_ipaddress(char *fname);
extern int InitSys (void);
extern int ping_test_with_ip (char *ipaddr);
extern char l3pd_IPaddr[MAX_PROBE_DEV_NUM][20];
extern int dirm_get_TAP_ipaddress(char *fname);
extern int  get_TAP_Info_main (void);
extern int  get_TAP_Info_init (void);
#endif //  __DIRM_H__
