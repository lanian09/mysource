#ifndef __TRCLIB_H__
#define __TRCLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>

#include <comm_msgtypes.h>
#include <loglib.h>


#define TRCLIB_MAX_TRC_TIMER	600
#define TRCBUF_LEN	8192	
#define TRCTMP_LEN	1024

typedef struct {
	long				mtype;
	char				trcDeviceName[32];
	SingleOctetType     trcFlag;
	SingleOctetType     trcLogFlag;
} TrcLibSetPrintMsgType;

extern int errno;

extern int trclib_printOut (char*,int,char*);
extern int trclib_writeLog (char*,int,char*);
extern int trclib_writeErr (char*,int,char*);
extern int trclib_writeLogErr (char*,int,char*);
extern int trclib_exeSetPrintMsg (TrcLibSetPrintMsgType*);


#endif /*__TRCLIB_H__*/
