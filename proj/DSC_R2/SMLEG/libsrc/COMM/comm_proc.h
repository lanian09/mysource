#ifndef COMM_PROC_H
#define COMM_PROC_H

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
//#include <asm/param.h>

#define SYS_STATUS_FILE             "/proc/stat"
#define PROC_STAT_STATTIME_POS      22


extern int get_system_uptime(time_t *sysuptime);
extern int get_proc_starttime(pid_t prcpid, time_t sysuptime, time_t *prcuptime, char *starttime);

#endif /* COMM_PROC_H */
