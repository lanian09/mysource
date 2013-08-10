#ifndef __SCEM_H__
#define __SCEM_H__

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

/* SCEM에서 사용하는 파일들 */

#define SCEM_LOG_FILE       "LOG/OAM/scem_log"
#define SCEM_ERRLOG_FILE    "LOG/OAM/scem_err"

#define MAX_SCE_CNT			2

#define MAX_STR_LEN     32
#define MAX_FILE_NAME_LEN   128

/** T_loc_sadb에서 사용하는 status를 위한 값 종료 */

extern int report_SCE2FIMD (void);
extern int scem_initLog (void);
extern int scem_getSCE (int key);
extern int scem_get_snmp_sce_ipaddress(char *fname);
extern int scem_get_snmp_rdr_ipaddress(char *fname);
extern int	init_SCE_snmp();
extern int InitSys (void);
extern int report_SceFlow_Samd2Stmd (int flow_num, int sce_idx);
#endif //  __SCEM_H__
