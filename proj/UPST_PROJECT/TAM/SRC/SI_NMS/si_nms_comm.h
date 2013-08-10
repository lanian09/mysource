#ifndef _SI_NMS_COMM_H_
#define _SI_NMS_COMM_H_

/**
 *	Include headers
 */
// OAM
#include "mmcdef.h"

// TAM

/**
 *	Define constants & structures
 */
// lgt_nms.h 에 정의되어 있음, 다른곳에서는 사용하지 않음
// lgt_nms.h 에 정의되어 있음, 다른곳에서는 사용하지 않음
// lgt_nms.h 에 정의되어 있음, 다른곳에서는 사용하지 않음
#define MAX_BUF_SIZE				4096            /* socket send, receive max buffer */
#define MAX_CONN					30
#define NMS_STATISTICS_DIR			"NMS_DIR"

enum {
	PORT_IDX_ALM = 0,
	PORT_IDX_CONS,
	PORT_IDX_CONF,
	PORT_IDX_MMC,
	PORT_IDX_STAT,
	PORT_IDX_MAX
};

enum {
	FD_TYPE_LISTEN = 100,
	FD_TYPE_DATA
};

typedef struct _st_NMSIPInfo
{
	char			cType;			/*	STAT_PERIOD_5MIN, STAT_PERIOD_HOUR	*/
	unsigned int	uIP;
} st_NMSIPInfo;

#define MAX_STATISTICS_CONN			2
typedef struct _st_NMSIPList
{
	int				dCount;
	st_NMSIPInfo	stNMSIPInfo[MAX_STATISTICS_CONN];
} st_NMSIPList;

typedef struct _st_NMSPortInfo
{
	int		port[PORT_IDX_MAX];
	char	ipaddr[2][20];
} st_NMSPortInfo;
#define DEF_NMSPORTINFO_SIZE	sizeof(st_NMSPortInfo)

typedef struct _st_MMCInfo
{
	int				dMMCSfd;
	long long		gllTid;
	char			sAdminName[MAX_USER_NAME_LEN];	/*	24	*/
	unsigned char	cSerialNo;
} st_MMCInfo;

typedef struct _st_NMSSFdInfo
{
	time_t			tLastTime;
	char			cMask;
	char			cLevel;
	int				dSfd;
	int				dListenPort;
	unsigned int	uIPAddr;
	int				dType;							/*	FD_TYPE_LISTEN, FD_TYPE_DATA	*/
	ssize_t			dWriteStartPos;					/*	Write buffer START position		*/
	ssize_t			dWriteEndPos;					/*	Write buffer END position		*/
	char			sWriteBuf[MAX_BUF_SIZE];		/*	Write buffer: MAX_BUF_SIZE[4096]-This is defined in lgt_nms.h files.	*/
	short			dLastFlag;
	st_MMCInfo		stMMCInfo;
} st_NMSSFdInfo;

typedef struct _st_SelectInfo
{
	fd_set	Rfds;		/*	READ FD_SET		*/
	fd_set	Wfds;		/*	WRITE FD_SET	*/
	int		dMaxSfd;	/*	MAX FD VALUE	*/
} st_SelectInfo;

typedef struct _st_OidInfo
{
	char	sTableName[40];
	int		dSidFirstNum;
	int		dObjectID;
} st_OidInfo;

typedef struct _st_NTafName
{
	char	sNTAFName[64];
	int		dSysNo;
} st_NTafName;

#endif	/* _SI_NMS_COMM_H_ */

