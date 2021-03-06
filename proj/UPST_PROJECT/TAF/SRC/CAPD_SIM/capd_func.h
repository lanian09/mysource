#ifndef _CAPD_FUNC_H_
#define _CAPD_FUNC_H_

/**
 * Include headers
 */
#include <signal.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/poll.h>
#include <time.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/ethernet.h>     /* the L2 protocols */

// LIB
#include "common_stg.h"
#include "loglib.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"

// OAM
#include "filedb.h"			/* st_NTAF */
#include "almstat.h"		/* CRITICAL */

// TOOLS
#include "daginf.h"
#include "dagapi.h"
#include "dagutil.h"

// DQMS
#include "proc_id.h"

// TAF
#include "typedef.h"
#include "tmf_cap.h"

#include "capd_head.h"
#include "log.h"

/**
 * Define constants
 */
#define OPTBUFSIZE				ONE_KIBI
#define MAX_READ				5000

#ifdef BUFFERING
#define COLLECTION_MIN			50
#else
#define COLLECTION_MIN			0
#endif

#define COLLECTION_MAX			100
#define COLLECTION_TIME			5
#define COLLECTION_MULTIPLY		2

/**
 * Declare variables
 */
static char buffer[OPTBUFSIZE];
static char dagname[DAGNAME_BUFSIZE];

//extern S32	myqid;

int fcs_bits = 32; /* default Ethernet FCS bits size */
int loop_continue = 1;

unsigned char			*Send_Node_Head = NULL;
unsigned int			Diff_Node_Cnt=0;
unsigned int			Send_Node_Cnt=0;
unsigned int			Collection_Cnt = COLLECTION_MIN;
unsigned int			old_time = 0;

unsigned int			uiPortCurrentCount[2] = {0};
unsigned int			uiPortStatusCount[2] = {0};
unsigned int			uiPortStatusRetry[2] = {0};
time_t					curTime, oldTime=0, chkTime;
unsigned int			check_pkt=0;

extern int				dag_devnum;
extern int 				dag_fd;

extern stMEMSINFO		*pstMEMSINFO;
extern stCIFO			*pstCIFO;
extern UCHAR			*pstBuffer;
extern UCHAR			*pstNode;
extern UCHAR			*pstTLVNode;
extern st_NTAF     		*fidb;
extern st_PortStatus	stPortStatus[2];
extern UINT				guiSeqProcID;

/**
 * Declare functions
 */
int open_device(char *dagname_buf);
int close_device(void);
void do_packet_capture(void);
ULONG read_one_packet(void *buffer, ULONG bufferlen, int dDagNum );
int handle_ethernet(dag_record_t *cap_rec, int dDagNum );
void conv_ts2tv(uint64_t ts, struct timeval *tv);
int dump_DebugString(char *debug_str, char *s, int len);
int do_action(int port, int len, char *data, struct timeval *tmv);
int dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, U8 *pNode, U32 sec);

#endif	/* _CAPD_FUNC_H_ */
