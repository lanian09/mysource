#ifndef __FILTER_H__
#define __FILTER_H__

#include <time.h>

#include "typedef.h"
#include "commdef.h"
#include "define.h"
#include "loglib.h"

#define DEF_NTAF_SUCCESS	5
#define DEF_NTAF_FAIL		6


#define PDSNNAME		24
//#define USERINFO		24
//#define USERLN			20
#define DEF_MAX_USER	20
#define DEF_MAX_THRES	30
#define	DEF_SVC_CNT		25

#define DEF_TYPENAME		20
#define	MAX_SCTP_CNT	48


#define HEADER_LOG_CAPTURE		1

enum {
	RP_FLAG_INDEX = 0,
	PI_FLAG_INDEX,
	MAX_FLAG_INDEX
};

enum {
    PCF_SYSTYPE = 5,
    PDSN_SYSTYPE,
    AAA_SYSTYPE,
    HSS_SYSTYPE,
    LNS_SYSTYPE,
    SERVICE_SYSTYPE, /* 10  */
    CSCF_SYSTYPE,
    MNIP_SYSTYPE,
    LAC_SYSTYPE,
    CRX_SYSTYPE,
	PDIF_SYSTYPE,
    BSC_SYSTYPE = 20, /* added by uamyd 20101005 */
    BTS_SYSTYPE,
    ROAM_JAPAN  = 30,
    ROAM_CHINA
};

typedef struct _st_SvcInfo
{
	int				dIdx;
	char			cReserved[6];

	unsigned int	uSvcIP;
	unsigned short	huPort;
	unsigned char	cFlag;					/*	RP_FLAG_INDEX,	PI_FLAG_INDEX	*/
	unsigned char	cSysType;				/*	PCF_SYSTYPE, PDSN_SYSTYPE, AAA_SYSTYPE, HSS_SYSTYPE, BSD_SYSTYPE, SERVICE_SYSTYPE. CSCF_SYSTYPE	*/
											/*	This is defined in INC/watch_mon.h.															*/
	unsigned short	huL4Code;				/*	This is defined in INC/common_stg.h[LPREA_CONF)	*/
	unsigned short	huL7Code;				/*	This is defined in INC/common_stg.h[LPREA_CONF)	*/
	unsigned short	huAppCode;				/*	This is defined in INC/common_stg.h[LPREA_CONF)	*/

	char			szDesc[MAX_SDESC];		/*	MAX_SDESC[64]: This MAX_SDESC definition is in INC/define.h									*/
} st_SvcInfo, *pst_SvcInfo;

typedef struct _st_SvcInfo_List
{
	int			dCount;
	char		reserved[4];
	st_SvcInfo	stSvcInfo[DEF_SVC_CNT];
} st_SvcInfo_List, *pst_SvcInfo_List;


typedef struct _st_SvcInfo_Shm	/* 20050712 추가 */
{
	int			dCount;
	char		reserved[4];
	st_SvcInfo	stSvcInfo[MAX_SVC_CNT];	/*	MAX_SVC_CNT 120 */
} st_SvcInfo_Shm, *pst_SvcInfo_Shm;

typedef struct _st_NAS
{
	int				dIdx;

	unsigned int	uMNIP;
	unsigned short	usNetMask;
	unsigned char	cFlag;					/*	RP_FLAG_INDEX,	PI_FLAG_INDEX	*/

	unsigned char	cSysType;				/*	SYSTEM_TYPE_PCF, SYSTEM_TYPE_PDSN, SYSTEM_TYPE_AAA, SYSTEM_TYPE_HSS, SYSTEM_TYPE_SERVICE	*/
											/*	This is defined in INC/watch_mon.h.															*/
	char			szDesc[MAX_SDESC];		/*	MAX_SDESC[65]: This MAX_SDESC definition is in INC/define.h									*/
} st_NAS, *pst_NAS;

typedef struct _st_NAS_List
{
	int		 dCount;
	int		 reserved;
	st_NAS		stNAS[DEF_MNIP_COUNT];
} st_NAS_List, *pst_NAS_List;

typedef struct _st_NAS_Shm	/* 20050712 추가 */
{
	int		 dCount;
	int		 reserved;
	st_NAS		stNAS[MAX_MNIP_COUNT];
} st_NAS_Shm, *pst_NAS_Shm;

typedef struct _st_SCTP
{
	int				dIdx;
	char			cReserved[4];

	unsigned int	uSCTPIP;
	unsigned char	cSysType;				/*	SYSTEM_TYPE_PCF, SYSTEM_TYPE_PDSN, SYSTEM_TYPE_AAA, SYSTEM_TYPE_HSS, SYSTEM_TYPE_SERVICE	*/
											/*	This is defined in INC/watch_mon.h.															*/
	unsigned char	cDirection;
	unsigned short	huGroupID;
	char			szDesc[MAX_SDESC];		/*	MAX_SDESC[64]: This MAX_SDESC definition is in INC/define.h									*/
} st_SCTP, *pst_SCTP;

typedef struct _st_SCTP_Shm		/*	20090621 추가	*/
{
	int		 dCount;
	int		 dreserved;

	st_SCTP		stSCTP[MAX_SCTP_COUNT];
} st_SCTP_Shm, *pst_SCTP_Shm;

typedef struct _st_Conf
{
	short	sType;		/*	0: loglevel, 1: sysno, 2: dbno, 4: tcphdr	*/
	USHORT	usLogLevel;

	USHORT	usSysNo;
	USHORT	usDBNo;

	USHORT	usTcpHdr;
	char	reserved[6];
} st_Conf,	*pst_Conf;

typedef struct _st_HdrLog_Shm
{
	short		sHdrCapFlag; // HEADER_LOG_CAPTURE, HEADER_LOG_DROP
	char		reserved[6];
} st_HdrLog_Shm,	*pst_HdrLog_Shm;
/****************************************************************************************/

typedef struct _st_sna
{
	UINT	uSNAID;
	USHORT	uDepth;
	CHAR	szURL[MAX_SNAURL];
	CHAR	szMenuName[MAX_MENUNAME];
	CHAR	szSDESC[MAX_SDESC];
} st_sna, *pst_sna;

typedef struct _st_sna_List
{
	int		dCount;
	char	reserved[4];
	st_sna	stsna[MAX_SNA];
} st_sna_List, *pst_sna_List;

typedef struct _st_MS
{
	USHORT usMSID;
	CHAR	szMSNAME[MAX_MSNAME];
} st_MS, *pst_MS;

typedef struct _st_MS_List
{
	int dCount;
	char reserved[4];
	st_MS	stMS[MAX_MS];
} st_MS_List, *pst_MS_List;

typedef struct _st_Dest_Flt
{
	USHORT		usSvcCode;
	USHORT		usDestPort;
	UINT		uDestIP;

	USHORT		usSvcType;
	char		reserved[2];
	char		szDESC[DEF_SVCNAME_LEN];
} st_Dest_Flt, *pst_Dest_Flt;

typedef struct _st_Dest_Flt_List
{
	int		 dCount;
	int	 reserved;
	st_Dest_Flt	 stDestFlt[MAX_DEST_FLT];
} st_Dest_Flt_List, *pst_Dest_Flt_List;

typedef struct _st_MNIP
{
	UINT		uMNIP_Start;
	UINT		uMNIP_End;
	UINT		uNASIP;
	char		szNASNAME[PDSNNAME];
	UINT		uLOCALNO;

} st_MNIP, *pst_MNIP;

typedef struct _st_MNIP_List
{
	int		 dCount;
	int		 reserved;
	st_MNIP	 stMNIP[MAX_MNIP_COUNT];
} st_MNIP_List, *pst_MNIP_List;

typedef struct _st_AlmLevel
{
	char	szTypeName[DEF_TYPENAME];
	unsigned int	sCriticalLevel;
	unsigned int	sMajorLevel;
	unsigned int	sMinorLevel;
} st_AlmLevel, *pst_AlmLevel;

typedef struct _st_AlmLevel_List
{
	int dCount;
	int dSysNo;
	st_AlmLevel stAlmLevel[MAX_ALMTYPE];
} st_AlmLevel_List, *pst_AlmLevel_List;

typedef struct _st_Tmf_Info
{
	USHORT usTmfID;
	char	szSysType[6];
} st_Tmf_Info, *pst_Tmf_Info;

typedef struct _st_Flt_Common
{
	UINT	usCheckInterval;
	UINT	usRepeatCnt;
	UINT	usTCPLongLast;
	UINT	bOnOffState;
} st_Flt_Common, *pst_Flt_Common;

#define MAX_FCT 20
typedef struct _st_Flt_ContentType
{
	int ContentType[MAX_FCT];
	int FCCount;
	int reserved;
} st_Flt_ContentType, *pst_Flt_ContentType;

#define MAX_FSO 20
typedef struct _st_Flt_SvcOpt
{
	int SvcOpt[MAX_FSO];
	int FSCount;
	int reserved;
} st_Flt_SvcOpt, *pst_Flt_SvcOpt;

#define MAX_METHOD 20
#define MAX_REQUEST 20
typedef struct _st_Http_Method_Info
{
	int	 MethodValue[MAX_METHOD];
	char	MethodName[MAX_METHOD][MAX_REQUEST];
	int	 HttpCount;
	char	reserved[4];
} st_Http_Method_Info, *pst_Http_Method_Info;


typedef struct _st_SvcCode
{
	USHORT		usAppCode;
	USHORT		usSvcCode;
	USHORT		usSvcGroup;
	USHORT		usSvcType;
} st_SvcCode, *pst_SvcCode;

typedef struct _st_SvcCode_list
{
	int	dCount;
	int	reserved;
	st_SvcCode	stSvcCode[MAX_SVC_CNT];
} st_SvcCode_list, *pst_SvcCode_list;

#define MAX_TRACE_CNT	50
/*
#define TRC_TYPE_IP		1
#define TRC_TYPE_IMSI	2
#define TRC_TYPE_SERV	3
#define TRC_TYPE_MDN	4
*/

#define MIN_SIZE		16

typedef union _TraceID
{
	char szMIN[MIN_SIZE];
	UINT uIP;
	INT64 llIMSI;
} un_TraceID, *pun_TraceID;

typedef struct _st_BMsgHeader
{
	time_t		time;
	time_t		mtime;

	USHORT		usDataLen;
	char		reserved[2];
	UINT		uiNSAPI;

	long long 	llIMSI;

	USHORT		usSrcPort;
	USHORT		usDestPort;
	UINT		uiReserved;
} st_BMsgHeader, *pst_BMsgHeader;

typedef struct _stRanapInfo_MEM
{
	int	 Max_SessCnt;	/*최대 수용가능한 세션수 */
	int	 Use_SessCnt;	/*사용 중인 세션수 - Call*/
	int	 Use_MsgSessCnt;	/*사용 중인 세션수 - Msg*/
	int	 Use_IMSISessCnt;	/*사용 중인 세션수 - IMSI*/
	int	 Use_PagingSessCnt;	/*사용 중인 세션수 - IMSI*/
	int	 Use_SccpSrcSessCnt;	 /*사용 중인 세션수 - SCCP Src Session */
	int	 Use_SccpDstSessCnt;	 /*사용 중인 세션수 - SCCP Dest Session */
} stRanapInfo_MEM;

#define MAX_VERSION_LEN			32
#define DEF_FULL_PATCH			1
#define DEF_PARTIAL_PATCH		2

typedef struct _st_Patch_Info
{
	char	szGtamIP[16];
	char	szVersion[MAX_VERSION_LEN];
	int	 dPatchType; /* 1, 2 */
	int	 dReserved;

} st_Patch_Info, *pst_Patch_Info;

typedef struct _st_SessCnt_Info
{
	int dTcpSessCnt;
	int dHttpSessCnt;
	int dPageSessCnt;
	int dVodSessCnt;
	int dVodMedSessCnt;
	int dGtpSessCnt;
} st_SessCnt_Info, *pst_SessCnt_Info;

typedef struct _st_MirrorTime_
{
	UINT	uiMirrorPortA;
	UINT	uiMirrorPortB;
	UINT	uiMirrorPortC;
	UINT	uiMirrorPortD;
} st_MirrorTime, *pst_MirrorTime;

#define DEF_TIMERINFO_CNT   30

typedef struct _st_TIMER_INFO {
    U32         usTimerInfo[DEF_TIMERINFO_CNT];
} TIMER_INFO;

typedef struct _st_User_Add
{
    int     dIdx;
    short   sSLevel;
    USHORT  usLogin;

    char    szUserName[USERINFO];
    char    szPassword[USERINFO];

    UINT    uLastLoginTime;
    UINT    uConnectIP;

    UINT    uCreateTime;

    char    szLocalName[USERLN];
    char    szContact[USERLN];
} st_User_Add, *pst_User_Add;

typedef struct _st_User_Add_Shm /* 20050712 추가 */
{
    int         dCount;
    int         reserved;

    st_User_Add stUserAdd[MAX_USER];
} st_User_Add_Shm, *pst_User_Add_Shm;

#define DEF_EQUIPNAME 20
typedef struct _st_Info_Equip
{
    int             dIdx;

    unsigned int    uEquipIP;
    unsigned int    uNetmask;
    unsigned char   cEquipType;                     /*  SYSTEM_TYPE_PCF, SYSTEM_TYPE_PDSN, SYSTEM_TYPE_AAA, SYSTEM_TYPE_HSS, SYSTEM_TYPE_SERVICE    */
    char            szEquipTypeName[DEF_EQUIPNAME]; /*  DEF_EQUIPNAME[20]   */
    char            szEquipName[DEF_EQUIPNAME];     /*  DEF_EQUIPNAME[20]   */
    char            szDesc[MAX_SDESC];              /*  MAX_SDESC[64]       */
    unsigned char   cMon1MinFlag;
    char            cReserved[6];
} st_Info_Equip, *pst_Info_Equip;

#define MAX_EQUIP 5000
typedef struct _st_Info_Equip_Shm   /* 20050712 추가 */
{
  int   dCount;
  int   reserved;
  st_Info_Equip stInfoEquip[MAX_EQUIP];
} st_Info_Equip_Shm, *pst_Info_Equip_Shm;

typedef struct _st_Thres
{
    int             dIdx;
    char            cReserved[3];

    unsigned char   cSvcType;
    unsigned int    uTCPSetupTime;
    unsigned int    uResponseTime;
    unsigned int    uUpThroughput;
    unsigned int    uDnThroughput;
    unsigned int    uUpRetransCount;
    unsigned int    uDnRetransCount;
    unsigned int    uUpJitter;
    unsigned int    uDnJitter;
    unsigned int    uUpPacketLoss;
    unsigned int    uDnPacketLoss;
    char            szDesc[MAX_SDESC];
} st_Thres, *pst_Thres;

typedef struct _st_Thres_Shm  /* 20050712 추가 */
{
     int   dCount;
     int   reserved;

     st_Thres stThres[MAX_DEFECT_THRES];
} st_Thres_Shm, *pst_Thres_Shm;

typedef struct _st_Flt_Info
{
	st_MirrorTime			stMirrorTime;
	st_AlmLevel_List		stAlmLevelList;
	st_Tmf_Info				stTmfInfo;
	st_Flt_Common			stFltCommon;
	st_LogLevel_List		stLogLevelList;
	st_NAS_Shm				stNASShm;
	st_SCTP_Shm				stSCTPShm;
	st_SvcInfo_Shm			stSvcInfoShm;
	st_HdrLog_Shm			stHdrLogShm;
	st_SessCnt_Info			stSessCntInfo;
	TIMER_INFO				stTimerInfo;
	st_User_Add_Shm         stUserAddShm;
	st_Info_Equip_Shm		stInfoEquipShm;
	st_Thres_Shm			stThresShm;
} st_Flt_Info, *pst_Flt_Info;

#define DEF_FLT_INFO_SIZE sizeof(st_Flt_Info)

#endif /* __FILTER_H__ */
