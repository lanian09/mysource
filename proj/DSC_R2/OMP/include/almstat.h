/*********************************************************
                 ABLEX IPAS Project (IPAM BLOCK)

   Author   : LEE SANG HO
   Section  : IPAS(IPAM) Project
   SCCS ID  : %W%
   Date     : %G%
   Revision History :
        '03.    01. 15. initial

   Description:

   Copyright (c) ABLEX 2003
*********************************************************/


#ifndef __ALARM_STATUS_HEADER_FILE__
#define __ALARM_STATUS_HEADER_FILE__


/* LocType */
#define LOCTYPE_PHSC        0x01
#define LOCTYPE_LOAD        0x02
#define LOCTYPE_PROCESS     0x03
#define LOCTYPE_CHNL        0x04
#define LOCTYPE_NETLINK     0x05
#define LOCTYPE_NTP         0x06
#define LOCTYPE_SESSLOAD    0x07
#define LOCTYPE_SUCCRATE    0x08
#define LOCTYPE_SUCCRATE2   0x09	/* 20040603 sunny : add SUCCRATE2, delete TIMESYNC */
//#define LOCTYPE_TIMESYNC    0x09

/* SysType : LOCTYPE[PHSC, LOAD, PROCESS, CHNL, NETLINK, NTP, SESSLOAD] */
#define SYSTYPE_IPAM    0x01
#define SYSTYPE_IPAF    0x02
#define SYSTYPE_BDF     0x03

/* SysType : LOCTYPE[SUCC_R] */
#if 0	/* 20041207,poopee */
#define SYSTYPE_SUCCPDSN    0x01
#define SYSTYPE_SUCCIWF     0x02
#define SYSTYPE_SUCCIPAF    0x03
#define SYSTYPE_SUCCAAA     0x04
#define SYSTYPE_SUCCDSCP    0x05
#define SYSTYPE_SUCCPDSNQ   0x06
#define SYSTYPE_SUCCIWFQ    0x07
#define SYSTYPE_SUCCGGSN    0x08	/* 20040506, sunny : GGSN */
#define SYSTYPE_SUCCGGSNQ   0x09	/* 20040506, sunny : GGSNQ */

#define SYSTYPE_SUCCIDR	    0x01	/* 20040506, sunny : IDR 1, in case of SUCCRATE2 */
#else
#define SYSTYPE_SUCCPDSN	11
#define SYSTYPE_SUCCIWF		12
#define SYSTYPE_SUCCIPAF	13
#define SYSTYPE_SUCCAAA		14
#define SYSTYPE_SUCCDSCP	15
#define SYSTYPE_SUCCPDSNQ	16
#define SYSTYPE_SUCCIWFQ	17
#define SYSTYPE_SUCCGGSN	18
#define SYSTYPE_SUCCGGSNQ	19
#define SYSTYPE_SUCCIDR		20
#define	SYSTYPE_SUCCWAG		21
#define	SYSTYPE_SUCCWAGQ	22
#endif

/* SysNo */
/* System Typeº° System no */

/* InvokeType PHSC */
#define INVTYPE_POWER    0x01
#define INVTYPE_ETH_INF  0x02
#define INVTYPE_FAN      0x03
#define INVTYPE_DISKARRY 0x04
#define INVTYPE_MC       0x05
#define INVTYPE_SC       0x06
#define INVTYPE_UPS      0x07
#define INVTYPE_CLU      0x08
#define INVTYPE_BDF_MIRROR  0x09    /* 20040408, sunny : check ipaf frame when 0 */

/* InvokeType : Load */
#define INVTYPE_CPU      0x01
#define INVTYPE_MEMORY   0x02
#define INVTYPE_DISK     0x03
#define INVTYPE_QUEUE    0x04
#define INVTYPE_SESSION	 0x05	/* 20040319,poopee */

/* InvokeType : Process */
#define INVTYPE_USERPROC 0x01
#define INVTYPE_USERTRCE 0x02

/* InvokeType chnl */
#define INVTYPE_IPAF    0x01
#define INVTYPE_DHUB    0x02
#define INVTYPE_DSCP    0x03
#define INVTYPE_NMS     0x04
#define INVTYPE_IDR     0x05	/* 20040506, sunny */
#define INVTYPE_NOIDR 	0x06	/* 20040907,poopee */

/* InvokeType NETLINK */
#define INVTYPE_PINGDSCP  0x01
#define INVTYPE_PINGPDSN  0x02
#define INVTYPE_PINGIWF   0x03
#define INVTYPE_PINGAAA   0x04
#define INVTYPE_PINGGGSN  0x05
#define INVTYPE_PINGIPC   0x06	/* 20040407,poopee */
#define INVTYPE_PINGWAG   0x07	/* 20041117,poopee */

/* InvokeType NTP */
#define INVTYPE_NTPSVR      0x01
#define INVTYPE_NTPCHNL     0x02
#define INVTYPE_TIMESYNC    0x03	/* 20040603, sunny : add TIMESYNC */

/* InvokeType SESSION MEM LOAD */
#define INVTYPE_SESSLOAD 0x01

/** InvokeType TIMESYNC ***/
//#define INVTYPE_TIMESYNC      0x01 /* 20040603, sunny : delete TIMESYNC */



/* InvokeType  SUCCRATE */
#define INVTYPE_SUCCMSG     0x01

typedef struct _T_Alm_Status {
    unsigned char   ucLocType;
    unsigned char   ucSysType;
    unsigned char   ucSysNo;
    unsigned char   ucInvType;
    unsigned char   ucInvNo;
    unsigned char   ucAlmLevel;
    unsigned char   ucOldAlmLevel;
    unsigned char   ucReserv;
    time_t          tWhen;
    unsigned int    uiIPAddr;
} T_Alm_Status, *pT_Alm_Status;




typedef struct _st_Flt {
    unsigned int   uiCri;
    unsigned int   uiMaj;
    unsigned int   uiMin;
    unsigned int   uiReserv;
} st_Flt, *pst_Flt;

typedef struct _st_FltIPAM {
    st_Flt      stHW;
    st_Flt      stSW;
} st_FltIPAM, *pst_FltIPAM;

typedef struct _st_FltIPAF {
    st_Flt      stHW;
    st_Flt      stSW;
} st_FltIPAF, *pst_FltIPAF;

typedef struct _st_FltBDF {
    st_Flt      stHW;
    st_Flt      stSW;
} st_FltBDF, *pst_FltBDF;

typedef struct _st_IPAS_FLT {
    time_t     tWhen;
    int        dReserv;
    st_FltIPAM stIPAM[2];    /* 0 : active , 1 : standby */
    st_FltIPAF stIPAF[32];
    st_FltBDF  stBDF[16];
} st_IPAS_FLT, *pst_IPAS_FLT;

#define MAX_STAT_LIST  12

typedef struct _st_IPAS_FLTSTAT {
    unsigned short  usStartIdx;
    unsigned short  usCurIdx;
    unsigned int    uiReserv;
    st_IPAS_FLT  stIPASFLT[MAX_STAT_LIST];
} st_IPAS_FLTSTAT, *pst_IPAS_FLTSTAT;


typedef struct _st_Flt2 {
	unsigned int	uiCri;
	unsigned int 	uiMaj;
	unsigned int    uiMin;   
	time_t		    tWhen;
} st_Flt2, *pst_Flt2;



typedef struct _st_MMLStatFlt {
    time_t  tWhen;
    time_t  tFrom;
    int     mprd;
    short   sSysType;
    short   sSysNo;
    st_Flt2 stHWFlt;
    st_Flt2 stSWFlt;
} st_MMLStatFlt, *pst_MMLStatFlt;

typedef struct _st_MMLStatHisFlt {
    time_t  tWhen;
	time_t  tFrom;
    short   sSysType;
    short   sSysNo;
	int		dReserv;
    int     dTotCnt;
    int     dCurCnt;
    st_Flt2 stHWFlt[20];
    st_Flt2 stSWFlt[20];
} st_MMLStatHisFlt, *pst_MMLStatHisFlt;


typedef struct _st_Load {
	int    dCnt;
	float  fMin;
	float  fMax;
	float  fAvg;
} st_Load, *pst_Load;

typedef struct _st_StatLoad {
	st_Load stCpu;
	st_Load stMem;
	st_Load stDisk;
	st_Load stQueue;
#if 1	/* 20040319,poopee */
	st_Load stSess;
#endif
} st_StatLoad, *pst_StatLoad ;

typedef struct _st_MMLStatLoad {
	time_t	tWhen;
	time_t  tFrom;
	int		mprd;
	short	sSysType;
	short	sSysNo;
	st_StatLoad stLoad;
} st_MMLStatLoad, *pst_MMLStatLoad;

typedef struct _st_StatLoad2 {
	time_t  tWhen;
	int		dReserv;
	st_Load stCpu;
	st_Load stMem;
	st_Load stDisk;
	st_Load stQueue;
#if 1 /*20040324, sunny : srch-stat-load */
	st_Load stSess;
#endif
} st_StatLoad2, *pst_StatLoad2 ;

typedef struct _st_MMLStatHisLoad {
	time_t	tWhen;
	time_t  tFrom;
	short	sSysType;
	short	sSysNo;
	int		dreserv;
	int		dTotCnt;
	int		dCurCnt;
	st_StatLoad2 stLoad[10];
} st_MMLStatHisLoad, *pst_MMLStatHisLoad;

typedef struct _st_IPAS_LOAD {
	time_t  	tWhen;
	time_t 		tReserv;
	st_StatLoad stIPAM;
	st_StatLoad stIPAF[32];
} st_IPAS_LOAD, *pst_IPAS_LOAD;

typedef struct _st_IPAS_LOADSTAT {
	unsigned short  usStartIdx;
	unsigned short  usCurIdx;
	unsigned int	uiReserv;
	st_IPAS_LOAD stIPASLOAD[MAX_STAT_LIST];
} st_IPAS_LOADSTAT, *pst_IPAS_LOADSTAT ;


typedef struct _st_LOAD_LEVEL
{
	st_Flt		stCPU;
	st_Flt		stMEM;
	st_Flt		stDISK;
	st_Flt		stQUE;
#if 1	/* 20040319,poopee */
	st_Flt		stSESS;
#endif
} st_LOAD_LEVEL, *pst_LOAD_LEVEL;	


typedef struct _st_NTP {
    unsigned char   ucSymF;
    unsigned char   ucTF;
    unsigned char   ucResev[6];
    unsigned short  usWhen;
    unsigned short  usPoll;
    unsigned short  usReach;
    unsigned short  usST;
    float           fDelay;
    float           fOffset;
    float           fJitter;
    float           fResefv;
    char            szIP[32];
    char            szRefIP[32];
} st_NTP, *pst_NTP;



typedef struct _st_NTP_STS {
    int     dNtpCnt;
    int     dReserv;
    st_NTP  stNTP[32];
} st_NTP_STS, *pst_NTP_STS;


#endif


