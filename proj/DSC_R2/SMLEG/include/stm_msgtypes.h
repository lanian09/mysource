#ifndef __STM_MSGTYPES_H__
#define __STM_MSGTYPES_H__

#include "stmconf.h"
#define MAXMSC	256

#pragma pack(1)
#if 0 //jean delete

// System 부하 통계(Memory average,max ; CPU average,max)
typedef struct {
	unsigned int	average_mem;
	unsigned int	max_mem;
	unsigned int	cpu_cnt;
	unsigned int	average_cpu[STM_MAX_CPU_CNT];
	unsigned int	max_cpu[STM_MAX_CPU_CNT];
} STM_LoadStatisticMsgType;

#else 

///////////////////
typedef struct {
    unsigned int    cpu_cnt;
    unsigned int    average_cpu[STM_MAX_CPU_CNT];
    unsigned int    max_cpu[STM_MAX_CPU_CNT];
    unsigned int    avg_mem;
    unsigned int    max_mem;
} STM_LoadCommStatMsgType;

typedef struct {
	STM_LoadCommStatMsgType comminfo;
    unsigned int    avg_disk;
    unsigned int    max_disk;
    unsigned int    avg_msgQ;
    unsigned int    max_msgQ;
    unsigned int    avg_sess;
    unsigned int    max_sess;
} STM_LoadMPStatMsgType;

typedef struct {
	STM_LoadMPStatMsgType ompInfo;
	STM_LoadCommStatMsgType pdInfo[2];
} STM_LoadOMPStatMsgType;
///////////////////

#endif 

typedef struct {
	STM_CommStatisticInfo 		comm_info;
	unsigned int	average_mem;
	unsigned int	max_mem;
	unsigned int	average_cpu0;
	unsigned int	max_cpu0;
	unsigned int	average_cpu1;
	unsigned int	max_cpu1;
	unsigned int	average_cpu2;
	unsigned int	max_cpu2;
	unsigned int	average_cpu3;
	unsigned int	max_cpu3;
	unsigned int	stat_cnt;
} STM_LoadStatisticInfo;


#if 0 //jean 

//
// 장애 통계
// - 시스템단위의 장애 통계와 그룹단위의 장애통계로 나누어진다.
// - 시스템단위 장애통계에는 공통부분(common)과 type별(specific) 특성부분으로
//   구성된다.
//
typedef struct { // 모든 시스템에서 공통적으로 발생하는 장애 정보
	unsigned int	cpu[SFM_ALM_CRITICAL];  // CPU 과부하 장애
	unsigned int	mem[SFM_ALM_CRITICAL];  // Memory 과부하 장애
	unsigned int	disk[SFM_ALM_CRITICAL]; // disk usage 장애
	unsigned int	lan[SFM_ALM_CRITICAL];  // LAN 장애
	unsigned int	proc[SFM_ALM_CRITICAL]; // S/W Process 장애
	unsigned int	dup[SFM_ALM_CRITICAL];  // Duplication 장애
	unsigned int	succ[SFM_ALM_CRITICAL]; // SuccessRate 장애
	unsigned int	pdcpu[SFM_ALM_CRITICAL];
	unsigned int	pdmem[SFM_ALM_CRITICAL];
	unsigned int    pdfan[4][SFM_ALM_CRITICAL];
	unsigned int    pdgiga[52][SFM_ALM_CRITICAL];
	unsigned int    rsrc[15][SFM_ALM_CRITICAL];
	unsigned int	hwntp[2][SFM_ALM_CRITICAL];
} STM_CommAlmStat;

typedef struct {
	unsigned int	hwCom[SFM_ALM_CRITICAL];	// MP H/W
	unsigned int    stat[SFM_ALM_CRITICAL];     // stat
} STM_SpecAlmStat_SMS;

typedef struct { // 시스템 type별로 종류가 다른 장애 정보
	union {
		STM_SpecAlmStat_SMS		sms;    // SMS
	} u;
} STM_SpecAlmStat;

typedef struct {
	char				sysType[COMM_MAX_NAME_LEN];
	char				sysGroup[COMM_MAX_NAME_LEN];
	char				sysName[COMM_MAX_NAME_LEN];
	STM_CommAlmStat		comm; // 모든 시스템에서 공통적으로 발생하는 장애 정보
	STM_SpecAlmStat		spec; // 시스템 type별로 종류가 다른 장애 정보
} STM_SysAlmStat;

typedef struct {
	unsigned char	eqSysCnt; // 실장된 시스템 갯수
	char			dummy[3]; // for alignment
	STM_SysAlmStat	sys[SYSCONF_MAX_ASSO_SYS_NUM]; // eqSysCnt 갯수만큼만 데이터가 들어간다.
} STM_AlarmStatisticMsgType;

#else

//////////////////////////////////

//
// 장애 통계
// - 시스템단위의 장애 통계와 그룹단위의 장애통계로 나누어진다.
// - 시스템단위 장애통계에는 공통부분(common)과 type별(specific) 특성부분으로
//   구성된다.
//
typedef struct { // 모든 시스템에서 공통적으로 발생하는 장애 정보
    unsigned int    cpu[SFM_ALM_CRITICAL];  // CPU 과부하 장애
    unsigned int    mem[SFM_ALM_CRITICAL];  // Memory 과부하 장애
    unsigned int    disk[SFM_ALM_CRITICAL]; // disk usage 장애
    unsigned int    lan[SFM_ALM_CRITICAL];  // LAN 장애
    unsigned int    proc[SFM_ALM_CRITICAL]; // S/W Process 장애
    unsigned int    net_nms[SFM_ALM_CRITICAL];
	unsigned int    sess_ntp[SFM_ALM_CRITICAL];
    unsigned int    sess_nms[SFM_ALM_CRITICAL];
	unsigned int    pdcpu[2][SFM_ALM_CRITICAL];
    unsigned int    pdmem[2][SFM_ALM_CRITICAL];
	unsigned int    pdfan[2][SFM_ALM_CRITICAL];
	unsigned int    pdgiga[2][SFM_ALM_CRITICAL];
} STM_CommFltStat;

typedef struct {
    unsigned int    dup_hb[SFM_ALM_CRITICAL];  // Duplication 장애
    unsigned int    dup_oos[SFM_ALM_CRITICAL];  // Duplication 장애
    unsigned int    succ_wap1ana[SFM_ALM_CRITICAL];
    unsigned int    succ_wap2ana[SFM_ALM_CRITICAL];
    unsigned int    succ_httpana[SFM_ALM_CRITICAL];
    unsigned int    succ_uawap[SFM_ALM_CRITICAL];
    unsigned int    succ_aaa[SFM_ALM_CRITICAL];
    unsigned int    succ_vods[SFM_ALM_CRITICAL];    
    unsigned int    net_pdsn[SFM_ALM_CRITICAL];
    unsigned int    net_uawap[SFM_ALM_CRITICAL];
    unsigned int    net_aaa[SFM_ALM_CRITICAL];
    unsigned int    sess_ntp[SFM_ALM_CRITICAL];
    unsigned int    sess_uawap[SFM_ALM_CRITICAL];
	//jean
	unsigned int    rsrc[SFM_ALM_CRITICAL];
	unsigned int	hwntp[2][SFM_ALM_CRITICAL];
} STM_SpecFltStat_BSD;

typedef struct { // 시스템 type별로 종류가 다른 장애 정보
    union {
        STM_SpecFltStat_BSD     bsd;
    } u;
} STM_SpecFltStat;

typedef struct {
    char                sysType[COMM_MAX_NAME_LEN];
    char                sysGroup[COMM_MAX_NAME_LEN];
    char                sysName[COMM_MAX_NAME_LEN];
    STM_CommFltStat     comm; // 모든 시스템에서 공통적으로 발생하는 장애 정보
    STM_SpecFltStat     spec; // 시스템 type별로 종류가 다른 장애 정보
} STM_SysFltStat;

typedef struct {
    unsigned char   eqSysCnt; // 실장된 시스템 갯수
    char            dummy[3]; // for alignment
    STM_SysFltStat  sys[SYSCONF_MAX_ASSO_SYS_NUM]; // eqSysCnt 갯수만큼만 데이터가 들어간다.
} STM_AlarmStatisticMsgType;
//////////////////////////////////

#endif

#if 0
typedef struct {
	STM_CommStatisticInfo 		comm_info;
	STM_CommAlmStat				comm_flt;
	unsigned int				stat_cnt;
} STM_CommonFltStatisticInfo;

typedef struct {
	STM_CommStatisticInfo 		comm_info;
//	STM_SpecAlmStat_SMS			sms_flt;
	unsigned int				stat_cnt;
} STM_SMSFltStatisticInfo;
#else
typedef struct {
	STM_CommStatisticInfo 		comm_info;
	STM_CommFltStat				comm_flt;
	unsigned int				stat_cnt;
} STM_CommonFltStatisticInfo;

typedef struct {
	STM_CommStatisticInfo 		comm_info;
//	STM_SpecAlmStat_SMS			sms_flt;
	unsigned int				stat_cnt;
} STM_SMSFltStatisticInfo;
#endif

/* ///////////////////////////////////////////////////////////////////// */

/* scp 통계 항목 */
typedef struct {
    unsigned int   ip;
    unsigned int   sys;				/* 통계 에서만 쓰는 놈 */
    /* connection */
	unsigned int   connTotal;
    unsigned int   connSucc;
    unsigned int   connFail;
    unsigned int   connClose;
    /* SYS INFO */
	unsigned int   txSIReq;
	unsigned int   rxSIRsp;
    unsigned int   txSIRsp;
    unsigned int   rxSIReq;
    /* OPERATION */
	unsigned int   total;
    unsigned int   succ;
    unsigned int   fail;
    /* Add */
    unsigned int   addTotal;
    unsigned int   addSucc;
    unsigned int   addFail;
    unsigned int   addeFormat;
    unsigned int   addeDbErr;
    unsigned int   addeDrop;
    unsigned int   addeTimeout;
    unsigned int   addeManMiss;
    unsigned int   addeNexist;
    /* delete */
    unsigned int   delTotal;
    unsigned int   delSucc;
    unsigned int   delFail;
    unsigned int   deleFormat;
    unsigned int   deleDbErr;
    unsigned int   deleDrop;
    unsigned int   deleTimeout;
    unsigned int   deleManMiss;
    unsigned int   deleNexist;
    /* update */
    unsigned int   upTotal;
    unsigned int   upSucc;
    unsigned int   upFail;
    unsigned int   upeFormat;
    unsigned int   upeDbErr;
    unsigned int   upeDrop;
    unsigned int   upeTimeout;
    unsigned int   upeManMiss;
    unsigned int   upeNexist;
    /* SVC_QRY */
    unsigned int   sqTotal;
    unsigned int   sqSucc;
    unsigned int   sqFail;
    unsigned int   sqeFormat;
    unsigned int   sqeDbErr;
    unsigned int   sqeDrop;
    unsigned int   sqeTimeout;
    unsigned int   sqeManMiss;
    unsigned int   sqeNexist;
} STM_ScibStatData;

#define MAX_SCIB_NUM		50	/* MAXCONNECT와 같다. */
typedef struct {
	STM_CommStatisticInfo   comm_info;
	int						num;
    STM_ScibStatData 		scib[MAX_SCIB_NUM];
} STM_ScibStatMsgtype;


typedef struct {
    unsigned long   ip;
    unsigned long   sys;
    /* connection */
    unsigned long   connTotal;
    unsigned long   connSucc;
    unsigned long   connFail;
    unsigned long   connClose;
    /* SYS INFO */
    unsigned long   txSIReq;
    unsigned long   rxSIRsp;
    unsigned long   txSIRsp;
    unsigned long   rxSIReq;
    /* OPERATION */
    unsigned long   total;
    unsigned long   succ;
    unsigned long   fail;
    /* RSR */
    unsigned long   rsrTotal;
    unsigned long   rsrSucc;
    unsigned long   rsrFail;
    unsigned long   rsreFormat;

    unsigned long   rsreDiff;			/* rsrePara -> */
    unsigned long   rsreDrop;
    unsigned long   rsreUnknown;		/* rsreTimeout -> */
    unsigned long   rsreManMiss;
    unsigned long   rsreWinErr;			/* rsreEtc -> */

    /* BR */
    unsigned long   brTotal;
    unsigned long   brSucc;
    unsigned long   brFail;
    unsigned long   breFormat;
    unsigned long   breDiff;			/* brePara -> */

    unsigned long   breDrop;
    unsigned long   breUnknown;			/* breTimeout -> */
    unsigned long   breManMiss;
    unsigned long   breWinErr;				/* breEtc -> */

    /* SVC_QRY */
    unsigned long    sqTotal;

    unsigned long   sqSucc;
    unsigned long   sqFail;
    unsigned long   sqeFormat;
    unsigned long   sqeDiff;			/* sqePara -> */
    unsigned long   sqeDrop;
    unsigned long   sqeUnknown;			/* sqeTimeout -> */
    unsigned long   sqeManMiss;
    unsigned long   sqeWinErr;			/* sqeEtc -> */
} STM_RcifStatData;

#define MAX_RCIF_NUM        200 /* MAXCONNECT와 같다. */
typedef struct {
	STM_CommStatisticInfo   comm_info;
    long             		num;
    STM_RcifStatData   		rcif[MAX_RCIF_NUM];
} STM_RcifStatMsgType;

/* SCPIF */
typedef struct {
    unsigned long  scpid;        /* KEY */
    unsigned long  sys;

    /* connection */
    unsigned long   connTotal;
    unsigned long   connSucc;
    unsigned long   connFail;
    unsigned long   connClose;
    /* SYS INFO */
    unsigned long   txSIReq;
    unsigned long   rxSIRsp;
    unsigned long   txSIRsp;
    unsigned long   rxSIReq;
    /* OPRATION */
    unsigned long   total;
    unsigned long   succ;
    unsigned long   fail;

    /* RSR */
    unsigned long   rsrTotal;
    unsigned long   rsrSucc;
    unsigned long   rsrFail;

	unsigned int   rsreWIN;
    unsigned int   rsreMisSvc;
    unsigned int   rsreUnkown;
    unsigned int   rsreDrop;
    unsigned int   rsreTimeout;
    unsigned int   rsreEtc;

    /* BR */
    unsigned long   brTotal;
    unsigned long   brSucc;
    unsigned long   brFail;

    unsigned int   breWIN;
    unsigned int   breMisSvc;
    unsigned int   breUnkown;
    unsigned int   breDrop;
    unsigned int   breTimeout;
    unsigned int   breEtc;


    /* SVC_QRY */
    unsigned long   sqTotal;
    unsigned long   sqSucc;
    unsigned long   sqFail;

    unsigned int   sqeWIN;
    unsigned int   sqeMisSvc;
    unsigned int   sqeUnkown;
    unsigned int   sqeDrop;
    unsigned int   sqeTimeout;
    unsigned int   sqeEtc;

} STM_ScpifStatData;

typedef struct {
	STM_CommStatisticInfo   comm_info;
    STM_ScpifStatData 		scpif;
} STM_ScpifStatMsgType;


#define	NUM_OF_SCP	11	/* scp1~10 */
typedef struct {
	unsigned int            num;
	STM_ScpifStatData       scpif[NUM_OF_SCP];
} STM_ScpifStat;


/* Wise 관련 */

typedef struct {
    unsigned int   attemp;
    unsigned int   success;
    unsigned int   fail;
    unsigned int   miss_para;
    unsigned int   alr_exist;
    unsigned int   not_exist;
    unsigned int   timeout;
    unsigned int   down_others;
} STM_DownStatData;

#define MAX_WISE_CMD_NUM		5	
#define  AREA_ADD       0
#define  AREA_DEL       1
#define  AREA_UPT     	2
#define  AREA_QRY      	3
#define  AREA_OTHER     4

typedef struct {
	STM_CommStatisticInfo           comm_info;
	unsigned int        cnt;
    STM_DownStatData 	data[MAX_WISE_CMD_NUM];
} STM_DownStatMsgType;


typedef struct {
    unsigned long   sys;
    
    unsigned long   addattemp;
    unsigned long   addsuccess;
    unsigned long   addfail;
    unsigned long   addmiss_para;
    unsigned long   addalr_exist;
    unsigned long   addnot_exist;
    unsigned long   addtimeout;
    unsigned long   adddown_others;

    unsigned long   delattemp;
    unsigned long   delsuccess;
    unsigned long   delfail;
    unsigned long   delmiss_para;
    unsigned long   delalr_exist;
    unsigned long   delnot_exist;
    unsigned long   deltimeout;
    unsigned long   deldown_others;

    unsigned long   upattemp;
    unsigned long   upsuccess;
    unsigned long   upfail;
    unsigned long   upmiss_para;
    unsigned long   upalr_exist;
    unsigned long   upnot_exist;
    unsigned long   uptimeout;
    unsigned long   updown_others;

    unsigned long   qryattemp;
    unsigned long   qrysuccess;
    unsigned long   qryfail;
    unsigned long   qrymiss_para;
    unsigned long   qryalr_exist;
    unsigned long   qrynot_exist;
    unsigned long   qrytimeout;
    unsigned long   qrydown_others;

    unsigned long   othattemp;
    unsigned long   othsuccess;
    unsigned long   othfail;
    unsigned long   othmiss_para;
    unsigned long   othalr_exist;
    unsigned long   othnot_exist;
    unsigned long   othtimeout;
    unsigned long   othdown_others;
} STM_WiseStatMsgType;

/* DB 관련 */

typedef struct {
    unsigned long    sys;
    unsigned long    select;
    unsigned long    insert;
    unsigned long    delete;
    unsigned long    update;
    unsigned long    notfound;
    unsigned long    alr_exist;
    unsigned long    acc_fail;
} STM_DbStatData;

typedef struct {
    STM_CommStatisticInfo       comm_info;
	STM_DbStatData				data;
} STM_DbStatMsgType;

/* Backup 관련 */
typedef struct {
    unsigned int    sys;        /* My SysID S_IBH1, S_IBH2 */
    unsigned int    srcSysid;   /* INBH1, 2, 3, 4, ... ex) 1,2,3,4 */
    unsigned int    conTry;
    unsigned int    conClose;
    unsigned int    insert;
    unsigned int    update;
    unsigned int    delete;
    unsigned int    fail;
    unsigned int    eTimeOut;
    unsigned int    eEtc;
} STM_ObStatData;			/* Online Backup Stat */

#define MAX_INBH_NUM    5
typedef struct {
	STM_CommStatisticInfo       comm_info;
    int num;        /* installed inbh backup client */
	STM_ObStatData				data[MAX_INBH_NUM];
} STM_ObStatMsgType;

#pragma pack()
#endif /*__STM_MSGTYPES_H__*/
