/*******************************************************************************
                DQMS Project

   Author   : Lee Dong-Hwan
   Section  : CAPD 
   SCCS ID  : @(#)capd_main.c (V1.0)
   Date     : 07/02/09
   Revision History :
        '09.    07. 02. initial

   Description :

   Copyright (c) uPRESTO 2005
*******************************************************************************/

/** A. FILE INCLUSION *********************************************************/
#include <string.h>
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
#include "typedef.h"
#include "config.h"				/* MAX_SW_COUNT */
#include "common_stg.h"
#include "loglib.h"
#include "verlib.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"

// DQMS
#include "commdef.h"
#include "procid.h"

// TAF

// OAM
#include "filedb.h"				/* st_NTAF */

// TOOLS
#include "dagapi.h"
#include "daginf.h"
#include "dagapi.h"
#include "dagutil.h"
#include "capd_test.h"

/**
 * Declare variables
 */
S32				gdStopFlag = 1;
//gS32         	myqid;

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

/* FOR DEBUG */
S64				curSessCnt;		/* Transaction 개수 */
S64				sessCnt;
S64				rcvNodeCnt;		/* 받은 NODE 개수 */
S64				diffSeqCnt;		/* DIFF SEQ가 된 개수 */

/** B. DEFINITION OF NEW CONSTANTS ********************************************/
/** C. DEFINITION OF NEW TYPES ************************************************/
/** D. DECLARATION OF VARIABLES ***********************************************/
char				gszMyProc[32];
UINT				guiSeqProcID;
/** E.1 DEFINITION OF FUNCTIONS ***********************************************/
extern int open_device(char *dagname_buf);
extern void do_packet_capture(void);
extern int close_device(void);
extern S32 dInitCapd(stMEMSINFO **pMEMSINFO, stCIFO **pCIFO);
/** E.2 DEFINITION OF FUNCTIONS ***********************************************/
/*******************************************************************************

*******************************************************************************/
int main(int argc, char* argv[])
{
	// Assign process name
	memcpy(&gszMyProc[0], argv[0], strlen(argv[0]));

	// Assign process sequence
	guiSeqProcID = SEQ_PROC_CAPD;

	// Initiate log, gifo, fidb, port status ..
	dInitCapd(&pstMEMSINFO, &pstCIFO);

#ifdef SETPRIORITY
	setpriority(PRIO_PROCESS, getpid(), -20);
#endif

#if 0
	open_device("dag0");
//	open_device("dag1");

	do_packet_capture();
	close_device();

#endif

	if(argc > 2)
	{
		test_func2(argv[1], 0);
	}

	return 1;
}
