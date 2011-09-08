#ifndef _CAPD_TEST_H_
#define _CAPD_TEST_H_

/**
 * Include headers
 */
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// LIB
#include "common_stg.h"
#include "mems.h"
#include "loglib.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"

// OAM
#include "path.h"			/* LOG_PATH */

// DQMS
#include "comm_def.h"
#include "proc_id.h"

// TAF
#include "Analyze_Ext_Abs.h"

#include "capd_api.h"
#include "log.h"

/**
 * Declare variables
 */
int				gdSockfd;
int				gdMaxfd;
//int				myqid;
unsigned int	guiMyIP;
unsigned int	guiSeqProcID;
char			gszMyProc[32];
stMEMSINFO		*pmem;
stCIFO			*pcifo;

extern U64      nifo_create;
extern U64      nifo_del;

/**
 * Declare functions
 */
int dUdpSockInit(unsigned short usPort, int *pdSock);
int dIsRcvedPacket();
void sigtest(int sig);
void test_func(char *ip, unsigned short usPort);


#endif /* _CAPD_TEST_H_ */

