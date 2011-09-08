#ifndef __DB_DEFINE_H__
#define __DB_DEFINE_H__

#include "define.h"

/*	START: definitions	*/
#define DEF_INIT_MYSQL			DATA_PATH"/DQMS_MySQL.env"
#define MAX_SQLQUERY_SIZE		20480

#define MAX_IPADDR_SIZE			16
#define MAX_PASS_SIZE			21
#define MAX_DBNAME_SIZE			21

#define MAX_SIGINFO_COUNT		200
#define MAX_CLIIP_COUNT			100
#define MAX_CLI_COUNT			20
#define MAX_SVR_COUNT			10
#define DEF_ALIAS_SIZE			2

#define MAX_CATMODE_COUNT		300

#define MAX_SVC_HEADER_SIZE		10000
#define MAX_SVC_BODY_SIZE		30000
#define DEF_SVCMSG_CNT			5000
#define DEF_SVCTR_CNT			200

#define DB_TABLE_NOT_EXIST		1146
#define DB_NOT_CONNECT			2002
#define DB_DUPLICATED_ENTRY		1062

enum {
	CPU_LOC = 0,
	MEM_LOC,
	DISK_LOC,
	QUE_LOC,
	NIFO_LOC,
	TAF_TRAFFIC_LOC,
	TRAFFIC_LOC = TAF_TRAFFIC_LOC,
	SWCPU_LOC,
	SWMEM_LOC,
	ALM_CNT
	
};

#define MAX_THRES			20
#define DEF_SVC_MMC         15
#define DEF_USER_MMC 		10

/*	END: definitions	*/
#endif	/*	__DB_DEFINE_H__	*/

