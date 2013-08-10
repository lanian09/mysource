#ifndef _CAPD_INIT_H_
#define _CAPD_INIT_H_

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
#include "config.h"
#include "common_stg.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"
#include "loglib.h"
#include "verlib.h"

// OAM
#include "commdef.h"			/* LOG_PATH, FILE_NIFO_ZOND, FILE_CIFO_CONF, FILE_GIFO_CONF */
#include "sshmid.h"				/* S_SSHM_LOG_LEVEL */
#include "path.h"				/* LOG_PATH */
#include "filedb.h"				/* st_NTAF */
#include "almstat.h"			/* CRITICAL */

// TOOLS
//#include "dagapi.h"
//#include "daginf.h"
//#include "dagapi.h"
//#include "dagutil.h"

// DQMS
#include "procid.h"

// TAF
#include "capd_test.h"

/**
 * Declare variables
 */
int						gdFinishFlag;

int 				loop_continue=1;
char    		gszVersion[7] = "R3.0.0";
extern UINT				guiSeqProcID;
extern char				gszMyProc[32];
extern st_PortStatus	stPortStatus[2];
extern st_NTAF			*fidb;


/**
 * Declare functions
 */
S32 dInitCapd(stMEMSINFO **pMEMSINFO, stCIFO **pCIFO);

extern void SetUpSignal();


#endif	/* _CAPD_INIT_H_ */
