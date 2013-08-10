#ifndef __RLEG_H__
#define __RLEG_H__

#include "SmApiNonBlocking_c.h"
#include "ipaf_svc.h"

#define MAX_CALLING_STATION_ID_SIZE		17
#define MAX_FRAMEDIP_SIZE		 		16
#define MAX_DOMAIN_SIZE		 			16
#define LEG_SCM_ADDR		"10.160.28.106"
#define LEG_SCM_PORT		14374	

#define MAX_PBIT_CNT	100
#define MAX_HBIT_CNT	100


#define		PROPERTY_PACKAGE_ID				"packageId"
#define		PROPERTY_MONITOR				"monitor"

#define		PROPERTY_MONITOR_MODE_OFF		0
#define		PROPERTY_MONITOR_MODE_ON		1
#define		PROPERTY_PACKAGE_ID_VAL			20

#define     HIPADDR(d)      ((d>>24)&0xff),((d>>16)&0xff),((d>>8)&0xff),(d&0xff)
#define     NIPADDR(d)      (d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)

typedef struct __subscribers_info {
	USHORT 			usPkgNo;
	unsigned char 	szMIN[MAX_CALLING_STATION_ID_SIZE];
	unsigned char 	szFramedIP[MAX_FRAMEDIP_SIZE];
	unsigned char 	szDomain[MAX_DOMAIN_SIZE];
	MappingType 	type;

} SUBS_INFO, *PSUBS_INFO;

/* PH Bit Table Information */
typedef struct _st_Pkginfo_ {
	UCHAR		ucUsedFlag;
	UCHAR		ucSMSFlag;
	UCHAR 		ucReserved[2];
	USHORT		usPkgNo;
	USHORT 		usRePkgNo;
} ST_PKG_INFO, *PST_PKG_INFO;

#define	DEF_ST_PKG_INFO_SIZE 	sizeof(ST_PKG_INFO)

typedef struct _st_PBTable_List_ {
	UINT 			dCount;
	ST_PKG_INFO		stPBTable[MAX_PBIT_CNT][MAX_HBIT_CNT];		/* PBit, HBit Table */
} ST_PBTABLE_LIST, *PST_PBTABLE_LIST;

#define	DEF_ST_PBTABLE_SIZE 	sizeof(ST_PBTABLE_LIST)

/* global variable definition */
extern SMNB_HANDLE     gSCE_nbapi;

/* leg_init.c */
extern int initProc(void);
extern void finProc(void);

/* leg_radius.c */
extern void makeSubsRecords (pst_ACCInfo pstACCInfo, PSUBS_INFO psi);
extern int branchMsg (pst_MsgQ pstMsgQ);

/* leg_msgq.c */
extern int msgQRead	(st_MsgQ *pstMsgQ);

/* leg_sce_comm.c */

extern void connectSCE (void);
extern void disconnSCE (void);
extern void loginSCE (SUBS_INFO *si);
extern void logoutSCE (SUBS_INFO *si);

#endif /* __RLEG_H__ */

