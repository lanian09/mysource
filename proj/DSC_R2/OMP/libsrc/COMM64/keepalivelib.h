#ifndef __KEEPALIVELIB_H__
#define __KEEPALIVELIB_H__

#include "sysconf.h"

typedef struct {
    int     cnt[SYSCONF_MAX_APPL_NUM];
    int     oldcnt[SYSCONF_MAX_APPL_NUM];
    int     retry[SYSCONF_MAX_APPL_NUM];
} T_keepalive;

#endif //__KEEPALIVELIB_H__
