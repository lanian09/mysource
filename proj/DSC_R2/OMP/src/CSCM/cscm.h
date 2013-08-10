#ifndef __CSCM_H__
#define __CSCM_H__

#include <ctype.h>

#include "sysconf.h"
#include "commlib.h"
#include "sfm_msgtypes.h"
#include "stm_msgtypes.h"
#include "sfm_snmp.h"
#include <sys/ipc.h>
#include <sys/msg.h>
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

/* CSCM에서 사용하는 파일들 */

#define CSCM_LOG_FILE       "LOG/OAM/cscm_log"
#define CSCM_ERRLOG_FILE    "LOG/OAM/cscm_err"

#define MAX_SCE_CNT			2
/** T_loc_sadb에서 사용하기 위한 값 시작 */
#define	MAX_CPU_CNT			8
#define	MAX_DISK_CNT		10
/** T_loc_sadb에서 사용하기 위한 값 종료 */

#define KEEPALIVE_CHECK_TIME     60
#define MSGQ_CLEAR_TIME		3

#define MAX_STR_LEN     32
#define MAX_FILE_NAME_LEN   128

extern int report_L2SW2FIMD (void);
extern int cscm_initLog (void);
extern int cscm_get_snmp_rdr_ipaddress(char *fname);
extern int cscm_getL2SW (int key);
extern int cscm_get_snmp_L2_ipaddress(char *fname);
extern int InitSys (void);
extern int ping_test_with_ip (char *ipaddr);
#endif //  __CSCM_H__
