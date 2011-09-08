#ifndef __NMSIF_PROTO_H__
#define	__NMSIF_PROTO_H__

#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <mysql/mysql.h>

#include <commlib.h>
#include <comm_msgtypes.h>
#include <comm_almsts_msgcode.h>
#include <sfmconf.h>
#include <proc_version.h>
#include <mysql_db_tables.h>

enum {
	HW_FAULT = 0,
	CPU_FAULT,
	MEM_FAULT,
	PROC_FAULT,
	CPU_LOAD,
	MEM_LOAD,
	DISK_LOAD,
	MSGQ_LOAD,
	TOTAL_TRAFFIC,
	LEG_PROCESS,
	LOGON_PROCESS,	/*	10	*/
	RULESET_TRAFFIC,
	RULESETENTRY_TRAFFIC,
	SMSC_INTERLOCK,
	DELAY_PACKET,
	SCE_FLOW
};

enum {
	EQUIP_TAPA = 0,	/*	TAPA	*/
	EQUIP_TAPB,		/*	TAPB	*/
	EQUIP_SCEA,		/*	SCEA	*/
	EQUIP_SCEB,		/*	SCEB	*/
	EQUIP_SCMA,		/*	SCMA	*/
	EQUIP_SCMB,		/*	SCMB	*/
	EQUIP_DSCM,		/*	DSCM	*/
	EQUIP_MAX_COUNT
};

enum {
	LEVEL_CRITICAL = 0,
	LEVEL_MAJOR,
	LEVEL_MINOR,
	LEVEL_MAX_COUNT
};

enum {
	LOAD_AVERAGE = 0,
	LOAD_MAXIMUM,
	LOAD_MAX_COUNT
};

#endif
