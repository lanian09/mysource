#ifndef __KEEPALIVELIB_H__
#define __KEEPALIVELIB_H__

#include "sysconf.h"

typedef struct {
    int     cnt[SYSCONF_MAX_APPL_NUM];
    int     oldcnt[SYSCONF_MAX_APPL_NUM];
    int     retry[SYSCONF_MAX_APPL_NUM];
} T_keepalive;

extern int keepalivelib_init (char*);
extern void keepalivelib_increase (void);

#endif //__KEEPALIVELIB_H__
