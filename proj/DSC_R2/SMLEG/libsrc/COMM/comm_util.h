#ifndef __COMM_UTIL_H__
#define __COMM_UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include <trclib.h>

extern int errno;

extern void commlib_convByteOrd (char*p, int);
extern void commlib_microSleep (int);
extern void commlib_setupSignals (int*);
extern void commlib_quitSignal (int);
extern void commlib_ignoreSignal (int);
extern char *commlib_printTStamp (void);
extern char *commlib_printDateTime (time_t);
extern char *commlib_printTime (void);
extern char *commlib_printTime_usec (void);

#endif /*__COMM_UTIL_H__*/
