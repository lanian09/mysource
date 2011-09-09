#ifndef __STMCONF_H__
#define __STMCONF_H__

#include "sfmconf.h"

#define STM_MAX_CPU_CNT			SFM_MAX_CPU_CNT

typedef struct {
	char sysType[COMM_MAX_NAME_LEN];
	char sysGroup[COMM_MAX_NAME_LEN];
	char sysName[COMM_MAX_NAME_LEN];
} STM_CommStatisticInfo;

#endif /*__STMCONF_H__*/
