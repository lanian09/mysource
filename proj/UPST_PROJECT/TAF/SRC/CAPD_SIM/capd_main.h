#ifndef _CAPD_MAIN_H_
#define _CAPD_MAIN_H_

/**
 * Include headers
 */
#include <unistd.h>
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
#include <sys/resource.h>

// LIB
#include "config.h"				/* MAX_SW_COUNT */
#include "common_stg.h"
#include "loglib.h"
#include "verlib.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"

// OAM
#include "filedb.h"

// DQMS
#include "comm_def.h"
#include "proc_id.h"

// TAF

// TOOLS
#include "dagapi.h"
#include "daginf.h"
#include "dagapi.h"
#include "dagutil.h"

#include "capd_head.h"

/**
 * Declare variables
 */
S32				gdStopFlag = 1;
//S32         	myqid;

int 			dag_devnum;
int 			dag_devnum1;
int 			dag_fd;
int 			dag_fd1;

stMEMSINFO      *pstMEMSINFO;
stCIFO			*pstCIFO;

UCHAR           *pstBuffer;
UCHAR           *pstNode;
UCHAR           *pstTLVNode;
st_PortStatus	stPortStatus[2];
st_NTAF			*fidb;

UINT 			guiSeqProcID;
char            gszMyProc[32];
char    		gszVersion[7] = "R3.0.0";

/**
 * Declare functions
 */
extern void test_func(char *ip, unsigned short usPort);
extern S32 dInitCapd(stMEMSINFO **pMEMSINFO, stCIFO **pCIFO);



#endif	/* _CAPD_MAIN_H_ */
