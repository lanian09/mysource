#ifndef __KEEPALIVELIB_H__
#define __KEEPALIVELIB_H__

#include "sysconf.h"

typedef struct {
    int     cnt[SYSCONF_MAX_APPL_NUM];
    int     oldcnt[SYSCONF_MAX_APPL_NUM];
    int     retry[SYSCONF_MAX_APPL_NUM];
} T_keepalive;

int keepalivelib_init(char *processName);
void keepalivelib_increase();


extern int check_my_run_status (char *procname);
#endif //__KEEPALIVELIB_H__
