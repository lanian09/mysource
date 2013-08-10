#ifndef __DB_STRUCT_H__
#define __DB_STRUCT_H__

#include "typedef.h"
#include "define.h"
#include "common_stg.h"
#include "config.h"				/* MAX_SW_COUNT, only */
#include "filter.h"
#include "db_define.h"			/* DEF_SVC_MMC */

#define DEF_DB_NAME  16
#define DEF_DESC_SIZE 100
#define MAX_SYS_COUNT             512

#define MAX_STATFAIL_CNT	(MAX_SVC_CNT * 100) /* 에러코드의 수 최대 50개 */
#define	MAX_SYSALARM_CNT	250
#define	MAX_SYSLOAD_CNT		250
#define MAX_STATPAGE_CNT    300

#define DEF_MAX_TS_NAME         10
#define DEF_MAXLEN_TS_NAME      48
#define DEF_MAXLEN_SQLSTAT      1024
#define DEF_MAX_TABLE_NAME      30
#define DEF_MAXLEN_TABLE_NAME   48



#define MAX_FLT_DB 20
#define MAX_VALUE 20
#define MAX_NAME 20
#define DEF_MAX_NAMELEN		20

#define DEF_MAX_TS		32
#define DEF_DESC_LEN	50

#define DEF_ALMNAME_LEN		16
#define DEF_MAXALMTYPE_CNT	16

#define DEF_MAXERRMSG_LEN	300
#define DEF_REASON_LEN		65

#define MAX_ALMMSG_SIZE 80
#define		MAX_SESSTOTAL		600

#define DEF_MAX_DEL_TB      100

#define MAX_IMSI_SIZE	17
#define SIP_CALLID_SIZE 65
#define SIP_METHOD_SIZE 17
#define SIP_FROM_SIZE 65
#define SIP_TO_SIZE 65
#define SIP_URI_SIZE 257
#define SIP_NONCE_SIZE 33
#define SIP_USERNAME_SIZE 33
#define SIP_PROTO_SIZE 33
#define MSRP_MSGID_SIZE 33
#define MSRP_METHOD_SIZE 17
#define MSRP_PATH_SIZE 257
#define MSRP_CONTENTTYPE_SIZE 33
#define MAX_TMSI_SIZE 17

#pragma pack(1)

typedef struct _st_Dest_Flt_db
{
	USHORT	usSvcCode[MAX_DEST_FLT];
	USHORT 	usDestPort[MAX_DEST_FLT];
	UINT 			uDestIP[MAX_DEST_FLT];
	USHORT	usSvcType[MAX_DEST_FLT];/* LJS 20050511 */
	char			szDESC[MAX_DEST_FLT][DEF_SVCNAME_LEN];
} st_Dest_Flt_db, *pst_Dest_Flt_db;



typedef struct _st_AlmLevel_db
{
	UINT	sCriticalLevel[MAX_ALMTYPE];
	UINT	sMajorLevel[MAX_ALMTYPE];
	UINT	sMinorLevel[MAX_ALMTYPE];
} st_AlmLevel_db, *pst_AlmLevel_db;

typedef struct _st_MNIP_db
{
	UINT	uMNIP_Start[MAX_MNIP_COUNT];
	UINT	uMNIP_End[MAX_MNIP_COUNT];
	UINT	uNASIP[MAX_MNIP_COUNT];
	char	szNASNAME[MAX_MNIP_COUNT][PDSNNAME];
	UINT	uLOCALNO[MAX_MNIP_COUNT];
} st_MNIP_db, *pst_MNIP_db;


/**** Defined By KIW in 2004.4.17 *****/
typedef struct _st_Flt_ContentType_db
{
	UINT uCode[MAX_FLT_DB];
	UINT uType[MAX_FLT_DB];
} st_Flt_ContentType_db, *pst_Flt_ContentType_db;

typedef struct _st_Flt_SvcOpt_db
{
	UINT SvcOpt[MAX_FLT_DB];
} st_Flt_SvcOpt_db, *pst_Flt_SvcOpt_db;

typedef struct _st_Http_Method_Info_db
{
	int MethodValue[MAX_VALUE];
	char MethodName[MAX_VALUE][MAX_NAME];
} st_Http_Method_Info_db, *pst_Http_Method_Info_db;


typedef struct _st_Log
{
	USHORT	usLogLevel[MAX_SW_COUNT];
	UCHAR	ucSwName[MAX_SW_COUNT][DEF_MAX_NAMELEN];
} st_Log, *pst_Log;



typedef struct _st_Sts
{
	UINT	dStartTime;
	USHORT	usSystemType;
	USHORT	usSystemNo;
	USHORT	usLocType;
	USHORT	usLocIdx;
	USHORT	usMax;
	USHORT	usAvg;
} st_Sts, *pst_Sts;


typedef struct _st_SvcIP_db
{
	USHORT	usSvcCode[MAX_SVC_CNT * 10];
	TCHAR	ucSvcName[MAX_SVC_CNT * 10][DEF_SVCNAME_LEN];
} st_SvcIP_db, *pst_SvcIP_db;


typedef struct _st_Svc_db
{
    TCHAR   ucSvcName[MAX_SVC_CNT * 10][DEF_SVCNAME_LEN];
	UINT    uSvcIP[MAX_SVC_CNT * 10];
	USHORT	usPort[MAX_SVC_CNT * 10];
	TCHAR	ucDesc[MAX_SVC_CNT * 10][DEF_DESC_LEN];
} st_Svc_db, *pst_Svc_db;


typedef struct _st_SvcName_db
{
	TCHAR   ucSvcName[MAX_SVC_CNT][DEF_SVCNAME_LEN];
} st_SvcName_db, *pst_SvcName_db;


typedef struct _st_Svc_Add_db
{
	UINT	uSvcIP;
	USHORT	usPort;
	USHORT	usSvcCode;
	USHORT	usSvcType;
	TCHAR	ucDesc[DEF_DESC_LEN];
	USHORT	usAct;
} st_Svc_Add_db, *pst_Svc_Add_db;

typedef struct _st_AlmLvl_db
{
	TCHAR	szName[DEF_MAXALMTYPE_CNT][DEF_ALMNAME_LEN];
	UINT	usCri[DEF_MAXALMTYPE_CNT];
	UINT	usWarn[DEF_MAXALMTYPE_CNT];
} st_AlmLvl_db, *pst_AlmLvl_db;



typedef struct _st_ErrMsg
{
	USHORT	usFailType[DEF_MAXERRMSG_LEN];
	USHORT	usFailCode[DEF_MAXERRMSG_LEN];
	TCHAR	szReason[DEF_MAXERRMSG_LEN][DEF_REASON_LEN];
} st_ErrMsg, *pst_ErrMsg;

typedef struct _st_Log_Tcp
{
    USHORT  usSvcCode[MAX_SVC_CNT];
    UINT    uTrialCnt[MAX_SVC_CNT];
    UINT    uSuccessCnt[MAX_SVC_CNT];
    UINT	uDuration[MAX_SVC_CNT];
	UINT	uDuration2[MAX_SVC_CNT];
    UINT	uConnDuration[MAX_SVC_CNT];
	UINT	uConnDuration2[MAX_SVC_CNT];
    UINT	uFinDuration[MAX_SVC_CNT];
	UINT	uFinDuration2[MAX_SVC_CNT];
    UINT    uDataSize[MAX_SVC_CNT];
	UINT	uHttpSessCnt[MAX_SVC_CNT];
	UINT	uPacket1Cnt[MAX_SVC_CNT];
	UINT	uPacket2Cnt[MAX_SVC_CNT];
	UINT	uRetrans1Cnt[MAX_SVC_CNT];
	UINT	uRetrans2Cnt[MAX_SVC_CNT];
	UINT	uTrafficSize[MAX_SVC_CNT];
	UINT	uPageCnt[MAX_SVC_CNT];
	USHORT  usSynCount[MAX_SVC_CNT];
	USHORT  usSynAckCount[MAX_SVC_CNT];
} st_Log_Tcp, *pst_Log_Tcp;

typedef struct _st_Log_Hcnt
{
    USHORT  usSvcCode[MAX_SVC_CNT];
    UINT    uHttpSessCnt[MAX_SVC_CNT];
} st_Log_Hcnt, *pst_Log_Hcnt;

typedef struct _st_Stat_Tcp_5Min_Select
{
    USHORT  usSvcCode[MAX_SVC_CNT];
    UINT    uTrialCnt[MAX_SVC_CNT];
    UINT    uSuccessCnt[MAX_SVC_CNT];
    UINT	uDuration[MAX_SVC_CNT];
    UINT	uDuration2[MAX_SVC_CNT];
    UINT	uConnDuration[MAX_SVC_CNT];
    UINT	uConnDuration2[MAX_SVC_CNT];
    UINT	uFinDuration[MAX_SVC_CNT];
    UINT	uFinDuration2[MAX_SVC_CNT];
    UINT    uDataSize[MAX_SVC_CNT];
    UINT    uHttpSessCnt[MAX_SVC_CNT];
	UINT	uPacket1Cnt[MAX_SVC_CNT];
	UINT	uPacket2Cnt[MAX_SVC_CNT];
	UINT	uRetrans1Cnt[MAX_SVC_CNT];
	UINT	uRetrans2Cnt[MAX_SVC_CNT];
	UINT	uTrafficSize[MAX_SVC_CNT];
	UINT	uPageCnt[MAX_SVC_CNT];
	USHORT	usSynCount[MAX_SVC_CNT];
	USHORT	usSynAckCount[MAX_SVC_CNT];
    int     dHourFlag[MAX_SVC_CNT];
    int     dDayFlag[MAX_SVC_CNT];
    int     dWeekFlag[MAX_SVC_CNT];
    int     dMonFlag[MAX_SVC_CNT];
} st_Stat_Tcp_5Min_Select, *pst_Stat_Tcp_5Min_Select;


typedef struct _st_Stat_Tcp_5Min_Insert
{
    int		dStatTime[MAX_SVC_CNT];
    USHORT  usSvcCode[MAX_SVC_CNT];
    UINT    uTrialCnt[MAX_SVC_CNT];
    UINT    uSuccessCnt[MAX_SVC_CNT];
    UINT	uDuration[MAX_SVC_CNT];
	UINT	uDuration2[MAX_SVC_CNT];
    UINT	uConnDuration[MAX_SVC_CNT];
	UINT	uConnDuration2[MAX_SVC_CNT];
    UINT 	uFinDuration[MAX_SVC_CNT];
	UINT	uFinDuration2[MAX_SVC_CNT];
    UINT    uDataSize[MAX_SVC_CNT];
    UINT    uHttpSessCnt[MAX_SVC_CNT];
	UINT	uPacket1Cnt[MAX_SVC_CNT];
	UINT	uPacket2Cnt[MAX_SVC_CNT];
	UINT	uRetrans1Cnt[MAX_SVC_CNT];
	UINT	uRetrans2Cnt[MAX_SVC_CNT];
	UINT	uTrafficSize[MAX_SVC_CNT];
	UINT	uPageCnt[MAX_SVC_CNT];
	USHORT	usSynCount[MAX_SVC_CNT];
	USHORT	usSynAckCount[MAX_SVC_CNT];
    int     dHourFlag[MAX_SVC_CNT];
    int     dDayFlag[MAX_SVC_CNT];
    int     dWeekFlag[MAX_SVC_CNT];
    int     dMonFlag[MAX_SVC_CNT];
} st_Stat_Tcp_5Min_Insert, *pst_Stat_Tcp_5Min_Insert;


typedef struct _st_Stat_Tcp_Hour
{
    int		dStatTime[MAX_SVC_CNT];
    USHORT  usSvcCode[MAX_SVC_CNT];
    UINT    uTrialCnt[MAX_SVC_CNT];
    UINT    uSuccessCnt[MAX_SVC_CNT];
    UINT	uDuration[MAX_SVC_CNT];
    UINT	uDuration2[MAX_SVC_CNT];
    UINT	uConnDuration[MAX_SVC_CNT];
    UINT	uConnDuration2[MAX_SVC_CNT];
    UINT	uFinDuration[MAX_SVC_CNT];
    UINT	uFinDuration2[MAX_SVC_CNT];
    UINT    uDataSize[MAX_SVC_CNT];
    UINT    uHttpSessCnt[MAX_SVC_CNT];
	UINT	uPacket1Cnt[MAX_SVC_CNT];
	UINT	uPacket2Cnt[MAX_SVC_CNT];
	UINT	uRetrans1Cnt[MAX_SVC_CNT];
	UINT	uRetrans2Cnt[MAX_SVC_CNT];
	UINT	uTrafficSize[MAX_SVC_CNT];
	UINT	uPageCnt[MAX_SVC_CNT];
	USHORT	usSynCount[MAX_SVC_CNT];
	USHORT	usSynAckCount[MAX_SVC_CNT];
	int		dDayFlag[MAX_SVC_CNT];
	int		dWeekFlag[MAX_SVC_CNT];
	int		dMonFlag[MAX_SVC_CNT];
} st_Stat_Tcp_Hour, *pst_Stat_Tcp_Hour;


typedef struct _st_Log_Http
{
    USHORT  usSvcCode[MAX_SVC_CNT];
    UINT    uTrialCnt[MAX_SVC_CNT];
    UINT    uSuccessCnt[MAX_SVC_CNT];
    UINT	uDuration[MAX_SVC_CNT];
    UINT	uDuration2[MAX_SVC_CNT];
    UINT    uDataSize[MAX_SVC_CNT];
    UINT	uDataDuration[MAX_SVC_CNT];
    UINT	uDataDuration2[MAX_SVC_CNT];
	UINT	uPacket1Cnt[MAX_SVC_CNT];
	UINT	uPacket2Cnt[MAX_SVC_CNT];
	UINT	uRetrans1Cnt[MAX_SVC_CNT];
	UINT	uRetrans2Cnt[MAX_SVC_CNT];
	UINT	uURLSize[MAX_SVC_CNT];
	UINT	uHDRSize[MAX_SVC_CNT];
} st_Log_Http, *pst_Log_Http;


typedef struct _st_Stat_Http_5Min_Insert
{
    int		dStatTime[MAX_SVC_CNT];
    USHORT  usSvcCode[MAX_SVC_CNT];
    UINT    uTrialCnt[MAX_SVC_CNT];
    UINT    uSuccessCnt[MAX_SVC_CNT];
    UINT	uDuration[MAX_SVC_CNT];
    UINT	uDuration2[MAX_SVC_CNT];
    UINT    uDataSize[MAX_SVC_CNT];
    UINT	uDataDuration[MAX_SVC_CNT];
    UINT	uDataDuration2[MAX_SVC_CNT];
	UINT	uPacket1Cnt[MAX_SVC_CNT];
	UINT	uPacket2Cnt[MAX_SVC_CNT];
	UINT	uRetrans1Cnt[MAX_SVC_CNT];
	UINT	uRetrans2Cnt[MAX_SVC_CNT];
	UINT	uURLSize[MAX_SVC_CNT];
	UINT	uHDRSize[MAX_SVC_CNT];
    int     dHourFlag[MAX_SVC_CNT];
    int     dDayFlag[MAX_SVC_CNT];
    int     dWeekFlag[MAX_SVC_CNT];
    int     dMonFlag[MAX_SVC_CNT];
} st_Stat_Http_5Min_Insert, *pst_Stat_Http_5Min_Insert;

typedef struct _st_Stat_Http_5Min_Select
{
    USHORT  usSvcCode[MAX_SVC_CNT];
    UINT    uTrialCnt[MAX_SVC_CNT];
    UINT    uSuccessCnt[MAX_SVC_CNT];
    UINT    uDuration[MAX_SVC_CNT];
    UINT    uDuration2[MAX_SVC_CNT];
    UINT    uDataSize[MAX_SVC_CNT];
   	UINT    uDataDuration[MAX_SVC_CNT];
   	UINT    uDataDuration2[MAX_SVC_CNT];
	UINT	uPacket1Cnt[MAX_SVC_CNT];
	UINT	uPacket2Cnt[MAX_SVC_CNT];
	UINT	uRetrans1Cnt[MAX_SVC_CNT];
	UINT	uRetrans2Cnt[MAX_SVC_CNT];
	UINT	uURLSize[MAX_SVC_CNT];
	UINT	uHDRSize[MAX_SVC_CNT];
    int     dHourFlag[MAX_SVC_CNT];
    int     dDayFlag[MAX_SVC_CNT];
    int     dWeekFlag[MAX_SVC_CNT];
    int     dMonFlag[MAX_SVC_CNT];
} st_Stat_Http_5Min_Select, *pst_Stat_Http_5Min_Select;

typedef struct _st_Stat_Http_Hour
{
    int		dStatTime[MAX_SVC_CNT];
    USHORT  usSvcCode[MAX_SVC_CNT];
    UINT    uTrialCnt[MAX_SVC_CNT];
    UINT    uSuccessCnt[MAX_SVC_CNT];
    UINT	uDuration[MAX_SVC_CNT];
    UINT	uDuration2[MAX_SVC_CNT];
    UINT    uDataSize[MAX_SVC_CNT];
    UINT	uDataDuration[MAX_SVC_CNT];
    UINT	uDataDuration2[MAX_SVC_CNT];
	UINT	uPacket1Cnt[MAX_SVC_CNT];
	UINT	uPacket2Cnt[MAX_SVC_CNT];
	UINT	uRetrans1Cnt[MAX_SVC_CNT];
	UINT	uRetrans2Cnt[MAX_SVC_CNT];
	UINT	uURLSize[MAX_SVC_CNT];
	UINT	uHDRSize[MAX_SVC_CNT];
    int     dDayFlag[MAX_SVC_CNT];
    int     dWeekFlag[MAX_SVC_CNT];
    int     dMonFlag[MAX_SVC_CNT];
} st_Stat_Http_Hour, *pst_Stat_Http_Hour;


typedef struct _st_Log_Checksum
{
	USHORT	usNtafNo[MAX_NTAF_NUM];
	int		dCount[MAX_NTAF_NUM];
} st_Log_Checksum, *pst_Log_Checksum;

typedef struct _st_Stat_Checksum_5Min_Insert
{
	int		dStatTime[MAX_NTAF_NUM];
	USHORT	usNtafNo[MAX_NTAF_NUM];
	int		dCount[MAX_NTAF_NUM];
    int     dHourFlag[MAX_NTAF_NUM];
    int     dDayFlag[MAX_NTAF_NUM];
    int     dWeekFlag[MAX_NTAF_NUM];
    int     dMonFlag[MAX_NTAF_NUM];
} st_Stat_Checksum_5Min_Insert, *pst_Stat_Checksum_5Min_Insert;

typedef struct _st_Stat_Checksum_5Min_Select
{
    USHORT  usNtafNo[MAX_NTAF_NUM];
    int     dCount[MAX_NTAF_NUM];
    int     dHourFlag[MAX_NTAF_NUM];
    int     dDayFlag[MAX_NTAF_NUM];
    int     dWeekFlag[MAX_NTAF_NUM];
    int     dMonFlag[MAX_NTAF_NUM];
} st_Stat_Checksum_5Min_Select, *pst_Stat_Checksum_5Min_Select;

typedef struct _st_Stat_Checksum_Hour
{
    int     dStatTime[MAX_NTAF_NUM];
    USHORT  usNtafNo[MAX_NTAF_NUM];
    int     dCount[MAX_NTAF_NUM];
    int     dDayFlag[MAX_NTAF_NUM];
    int     dWeekFlag[MAX_NTAF_NUM];
    int     dMonFlag[MAX_NTAF_NUM];
} st_Stat_Checksum_Hour, *pst_Stat_Checksum_Hour;


typedef struct _st_Log_Fail
{
    USHORT  usSvcCode[MAX_STATFAIL_CNT];
    USHORT  usFailType[MAX_STATFAIL_CNT];
    USHORT  usFailCode[MAX_STATFAIL_CNT];
    int     dCount[MAX_STATFAIL_CNT];
} st_Log_Fail, *pst_Log_Fail;

typedef struct _st_Stat_Fail_5Min_Insert
{
    int     dStatTime[MAX_STATFAIL_CNT];
    USHORT  usSvcCode[MAX_STATFAIL_CNT];
    USHORT  usFailType[MAX_STATFAIL_CNT];
    USHORT  usFailCode[MAX_STATFAIL_CNT];
    int     dCount[MAX_STATFAIL_CNT];
    int     dHourFlag[MAX_STATFAIL_CNT];
    int     dDayFlag[MAX_STATFAIL_CNT];
    int     dWeekFlag[MAX_STATFAIL_CNT];
    int     dMonFlag[MAX_STATFAIL_CNT];
} st_Stat_Fail_5Min_Insert, *pst_Stat_Fail_5Min_Insert;

typedef struct _st_Stat_Fail_5Min_Select
{
    USHORT  usSvcCode[MAX_STATFAIL_CNT];
    USHORT  usFailType[MAX_STATFAIL_CNT];
    USHORT  usFailCode[MAX_STATFAIL_CNT];
    int     dCount[MAX_STATFAIL_CNT];
    int     dHourFlag[MAX_STATFAIL_CNT];
    int     dDayFlag[MAX_STATFAIL_CNT];
    int     dWeekFlag[MAX_STATFAIL_CNT];
    int     dMonFlag[MAX_STATFAIL_CNT];
} st_Stat_Fail_5Min_Select, *pst_Stat_Fail_5Min_Select;

typedef struct _st_Stat_Fail_Hour
{
    int     dStatTime[MAX_STATFAIL_CNT];
    USHORT  usSvcCode[MAX_STATFAIL_CNT];
    USHORT  usFailType[MAX_STATFAIL_CNT];
    USHORT  usFailCode[MAX_STATFAIL_CNT];
    int     dCount[MAX_STATFAIL_CNT];
    int     dDayFlag[MAX_STATFAIL_CNT];
    int     dWeekFlag[MAX_STATFAIL_CNT];
    int     dMonFlag[MAX_STATFAIL_CNT];
} st_Stat_Fail_Hour, *pst_Stat_Fail_Hour;


typedef struct _st_Stat_Page_Sess {
	int		dStartTime[MAX_STATPAGE_CNT];
	int 	dSvcCode[MAX_STATPAGE_CNT];
	int 	dSvcOption[MAX_STATPAGE_CNT];
	int 	dNID[MAX_STATPAGE_CNT];
	int 	dSID[MAX_STATPAGE_CNT];
	long	lServingPDSN[MAX_STATPAGE_CNT];
	char	szModel[MAX_STATPAGE_CNT][MAX_MIN_SIZE];
	char 	szMS_Man[MAX_STATPAGE_CNT][MAX_MS_MAN_LEN];
	char 	szBrowserInfo[MAX_STATPAGE_CNT][MAX_BROWSERINFO_LEN];
	char	szMenuUrl[MAX_STATPAGE_CNT][MAX_URL_LEN];
	char	ucMethod[MAX_STATPAGE_CNT];
	int		dPageSessFlag[MAX_STATPAGE_CNT];
	long	lTrialCnt[MAX_STATPAGE_CNT];
	long	lSuccessCnt[MAX_STATPAGE_CNT];
	long	lInHttpSessCount[MAX_STATPAGE_CNT];
	long    llTot_PageLoadTime[MAX_STATPAGE_CNT];
	long	lTotPackUpCnt[MAX_STATPAGE_CNT];
	long  	lTotPackDownCnt[MAX_STATPAGE_CNT];
	long	lTotReTransUpPackCnt[MAX_STATPAGE_CNT];
	long	lTotReTransDownPackCnt[MAX_STATPAGE_CNT];
	long 	lTotHdrSizei[MAX_STATPAGE_CNT];
	long	lTotDataSize[MAX_STATPAGE_CNT];
	long	lTotURLSizei[MAX_STATPAGE_CNT];
	int		dMaxURLSize[MAX_STATPAGE_CNT];
	long 	TotalT1Time[MAX_STATPAGE_CNT];
	long 	TotalT2Time[MAX_STATPAGE_CNT];
	long  	TotalT3Time[MAX_STATPAGE_CNT];
	long 	TotalT4Time[MAX_STATPAGE_CNT];
	long 	TotalT5Time[MAX_STATPAGE_CNT];
	long  	TotalT5Text[MAX_STATPAGE_CNT];
	long  	TotalT5Image[MAX_STATPAGE_CNT];
	long  	TotalT5Sound[MAX_STATPAGE_CNT];
	long		lContTextCnt[MAX_STATPAGE_CNT];
	long		lContImageCnt[MAX_STATPAGE_CNT];
	long		lContSoundCnt[MAX_STATPAGE_CNT];
	long		lRedirectCnt[MAX_STATPAGE_CNT];
	USHORT		usMaxContRedirectCnt[MAX_STATPAGE_CNT];
	long 	lTotRedirectDelayTime[MAX_STATPAGE_CNT];
	long		lMaxRedirectDelayTime[MAX_STATPAGE_CNT];
	USHORT		lLastTextFlag[MAX_STATPAGE_CNT];
	USHORT		lLastImageFlag[MAX_STATPAGE_CNT];
	USHORT		lLastSoundFlag[MAX_STATPAGE_CNT];
} st_Stat_Page_Sess, *pst_Stat_Page_Sess;

typedef struct st_Stat_Page_5Min_Insert {
	int		dStartTime[MAX_STATPAGE_CNT];
	USHORT	usSvcCode[MAX_STATPAGE_CNT];
	USHORT	usSvcOption[MAX_STATPAGE_CNT];
	USHORT 	usNID[MAX_STATPAGE_CNT];
	USHORT 	usSID[MAX_STATPAGE_CNT];
	USHORT	usServingPDSN[MAX_STATPAGE_CNT];
	char	szModel[MAX_STATPAGE_CNT][MAX_MIN_SIZE];
	char 	szMS_Man[MAX_STATPAGE_CNT][MAX_MS_MAN_LEN];
	char 	szBrowserInfo[MAX_STATPAGE_CNT][MAX_BROWSERINFO_LEN];
	char	szMenuUrl[MAX_STATPAGE_CNT][MAX_URL_LEN];
	short	ucMethod[MAX_STATPAGE_CNT];
	USHORT	usPageSessFlag[MAX_STATPAGE_CNT];

	UINT	uTrialCnt[MAX_STATPAGE_CNT];
	UINT	uSuccessCnt[MAX_STATPAGE_CNT];
	UINT	uInHttpSessCnt[MAX_STATPAGE_CNT];

	UINT	uTotPageLoadTime[MAX_STATPAGE_CNT];
	UINT	uTotPageLoadTime2[MAX_STATPAGE_CNT];
	UINT	uTotPackUpCnt[MAX_STATPAGE_CNT];
	UINT  	uTotPackDownCnt[MAX_STATPAGE_CNT];
	UINT	uTotReTransUpPackCnt[MAX_STATPAGE_CNT];
	UINT	uTotReTransDownPackCnt[MAX_STATPAGE_CNT];
	UINT 	uTotHdrSize[MAX_STATPAGE_CNT];
	UINT	uTotDataSize[MAX_STATPAGE_CNT];
	UINT	uTotURLSize[MAX_STATPAGE_CNT];
	USHORT		usMaxURLSize[MAX_STATPAGE_CNT];
	UINT	llTotT1Time[MAX_STATPAGE_CNT];
	UINT	llTotT1Time2[MAX_STATPAGE_CNT];
	UINT	llTotT2Time[MAX_STATPAGE_CNT];
	UINT	llTotT2Time2[MAX_STATPAGE_CNT];
	UINT	llTotT3Time[MAX_STATPAGE_CNT];
	UINT	llTotT3Time2[MAX_STATPAGE_CNT];
	UINT	llTotT4Time[MAX_STATPAGE_CNT];
	UINT	llTotT4Time2[MAX_STATPAGE_CNT];
	UINT	llTotT5Time[MAX_STATPAGE_CNT];
	UINT	llTotT5Time2[MAX_STATPAGE_CNT];
	UINT	llTotT5Text[MAX_STATPAGE_CNT];
	UINT	llTotT5Text2[MAX_STATPAGE_CNT];
	UINT	llTotT5Image[MAX_STATPAGE_CNT];
	UINT	llTotT5Image2[MAX_STATPAGE_CNT];
	UINT	llTotT5Sound[MAX_STATPAGE_CNT];
	UINT	llTotT5Sound2[MAX_STATPAGE_CNT];
	UINT	uContTextCnt[MAX_STATPAGE_CNT];
	UINT	uContImageCnt[MAX_STATPAGE_CNT];
	UINT	uContSoundCnt[MAX_STATPAGE_CNT];
	UINT	uRedirectCnt[MAX_STATPAGE_CNT];
	USHORT	usMaxContRedirectCnt[MAX_STATPAGE_CNT];
	UINT	llTotRedirectDelayTime[MAX_STATPAGE_CNT];
	UINT	llTotRedirectDelayTime2[MAX_STATPAGE_CNT];
	UINT	llMaxRedirectDelayTime[MAX_STATPAGE_CNT];
	UINT	llMaxRedirectDelayTime2[MAX_STATPAGE_CNT];
	USHORT		uLastTextFlag[MAX_STATPAGE_CNT];
	USHORT		uLastImageFlag[MAX_STATPAGE_CNT];
	USHORT		uLastSoundFlag[MAX_STATPAGE_CNT];
	int			dHourFlag[MAX_STATPAGE_CNT];
	int			dDayFlag[MAX_STATPAGE_CNT];
	int			dWeekFlag[MAX_STATPAGE_CNT];
	int			dMonFlag[MAX_STATPAGE_CNT];
} st_Stat_Page_5Min_Insert, *pst_Stat_Page_5Min_Insert;


typedef struct st_Stat_Page_5Min_Select {
	USHORT	usSvcCode[MAX_STATPAGE_CNT];
	USHORT	usSvcOption[MAX_STATPAGE_CNT];
	USHORT 	usNID[MAX_STATPAGE_CNT];
	USHORT 	usSID[MAX_STATPAGE_CNT];
	USHORT	usServingPDSN[MAX_STATPAGE_CNT];

	char 	szModel[MAX_STATPAGE_CNT][MAX_MIN_SIZE];
	char 	szMS_Man[MAX_STATPAGE_CNT][MAX_MS_MAN_LEN];
	char 	szBrowserInfo[MAX_STATPAGE_CNT][MAX_BROWSERINFO_LEN];
	char	szMenuUrl[MAX_STATPAGE_CNT][MAX_URL_LEN];
	USHORT	ucMethod[MAX_STATPAGE_CNT];

	USHORT	usPageSessFlag[MAX_STATPAGE_CNT];
	UINT	uTrialCnt[MAX_STATPAGE_CNT];
	UINT	uSuccessCnt[MAX_STATPAGE_CNT];
	UINT	uInHttpSessCnt[MAX_STATPAGE_CNT];
	UINT    uTotPageLoadTime[MAX_STATPAGE_CNT];
	UINT    uTotPageLoadTime2[MAX_STATPAGE_CNT];
	UINT	uTotPackUpCnt[MAX_STATPAGE_CNT];
	UINT  	uTotPackDownCnt[MAX_STATPAGE_CNT];
	UINT	uTotReTransUpPackCnt[MAX_STATPAGE_CNT];
	UINT	uTotReTransDownPackCnt[MAX_STATPAGE_CNT];
	UINT 	uTotHdrSize[MAX_STATPAGE_CNT];
	UINT	uTotDataSize[MAX_STATPAGE_CNT];
	UINT	uTotURLSize[MAX_STATPAGE_CNT];
	USHORT		usMaxURLSize[MAX_STATPAGE_CNT];
	UINT	llTotT1Time[MAX_STATPAGE_CNT];
	UINT	llTotT1Time2[MAX_STATPAGE_CNT];
	UINT	llTotT2Time[MAX_STATPAGE_CNT];
	UINT	llTotT2Time2[MAX_STATPAGE_CNT];
	UINT	llTotT3Time[MAX_STATPAGE_CNT];
	UINT	llTotT3Time2[MAX_STATPAGE_CNT];
	UINT	llTotT4Time[MAX_STATPAGE_CNT];
	UINT	llTotT4Time2[MAX_STATPAGE_CNT];
	UINT	llTotT5Time[MAX_STATPAGE_CNT];
	UINT	llTotT5Time2[MAX_STATPAGE_CNT];
	UINT	llTotT5Text[MAX_STATPAGE_CNT];
	UINT	llTotT5Text2[MAX_STATPAGE_CNT];
	UINT	llTotT5Image[MAX_STATPAGE_CNT];
	UINT	llTotT5Image2[MAX_STATPAGE_CNT];
	UINT	llTotT5Sound[MAX_STATPAGE_CNT];
	UINT	llTotT5Sound2[MAX_STATPAGE_CNT];
	UINT		uContTextCnt[MAX_STATPAGE_CNT];
	UINT		uContImageCnt[MAX_STATPAGE_CNT];
	UINT		uContSoundCnt[MAX_STATPAGE_CNT];
	UINT		uRedirectCnt[MAX_STATPAGE_CNT];
	USHORT		usMaxContRedirectCnt[MAX_STATPAGE_CNT];
	UINT	llTotRedirectDelayTime[MAX_STATPAGE_CNT];
	UINT	llTotRedirectDelayTime2[MAX_STATPAGE_CNT];
	UINT	llMaxRedirectDelayTime[MAX_STATPAGE_CNT];
	UINT	llMaxRedirectDelayTime2[MAX_STATPAGE_CNT];
	USHORT		uLastTextFlag[MAX_STATPAGE_CNT];
	USHORT		uLastImageFlag[MAX_STATPAGE_CNT];
	USHORT		uLastSoundFlag[MAX_STATPAGE_CNT];
	int			dHourFlag[MAX_STATPAGE_CNT];
	int			dDayFlag[MAX_STATPAGE_CNT];
	int			dWeekFlag[MAX_STATPAGE_CNT];
	int			dMonFlag[MAX_STATPAGE_CNT];
} st_Stat_Page_5Min_Select, *pst_Stat_Page_5Min_Select;


typedef struct st_Stat_Page_Hour {
	int		dStartTime[MAX_STATPAGE_CNT];
	USHORT	usSvcCode[MAX_STATPAGE_CNT];
	USHORT	usSvcOption[MAX_STATPAGE_CNT];
	USHORT 	usNID[MAX_STATPAGE_CNT];
	USHORT 	usSID[MAX_STATPAGE_CNT];
	USHORT	usServingPDSN[MAX_STATPAGE_CNT];
	char	szModel[MAX_STATPAGE_CNT][MAX_MIN_SIZE];
	char 	szMS_Man[MAX_STATPAGE_CNT][MAX_MS_MAN_LEN];
	char 	szBrowserInfo[MAX_STATPAGE_CNT][MAX_BROWSERINFO_LEN];
	char	szMenuUrl[MAX_STATPAGE_CNT][MAX_URL_LEN];
	USHORT  ucMethod[MAX_STATPAGE_CNT];

	USHORT	usPageSessFlag[MAX_STATPAGE_CNT];
	UINT	uTrialCnt[MAX_STATPAGE_CNT];
	UINT	uSuccessCnt[MAX_STATPAGE_CNT];
	UINT	uInHttpSessCnt[MAX_STATPAGE_CNT];
	UINT    uTotPageLoadTime[MAX_STATPAGE_CNT];
	UINT    uTotPageLoadTime2[MAX_STATPAGE_CNT];
	UINT	uTotPackUpCnt[MAX_STATPAGE_CNT];
	UINT  	uTotPackDownCnt[MAX_STATPAGE_CNT];
	UINT	uTotReTransUpPackCnt[MAX_STATPAGE_CNT];
	UINT	uTotReTransDownPackCnt[MAX_STATPAGE_CNT];
	UINT 	uTotHdrSize[MAX_STATPAGE_CNT];
	UINT	uTotDataSize[MAX_STATPAGE_CNT];
	UINT	uTotURLSize[MAX_STATPAGE_CNT];
	USHORT		usMaxURLSize[MAX_STATPAGE_CNT];
	UINT 	llTotT1Time[MAX_STATPAGE_CNT];
	UINT	llTotT1Time2[MAX_STATPAGE_CNT];
	UINT	llTotT2Time[MAX_STATPAGE_CNT];
	UINT	llTotT2Time2[MAX_STATPAGE_CNT];
	UINT	llTotT3Time[MAX_STATPAGE_CNT];
	UINT	llTotT3Time2[MAX_STATPAGE_CNT];
	UINT	llTotT4Time[MAX_STATPAGE_CNT];
	UINT	llTotT4Time2[MAX_STATPAGE_CNT];
	UINT	llTotT5Time[MAX_STATPAGE_CNT];
	UINT	llTotT5Time2[MAX_STATPAGE_CNT];
	UINT	llTotT5Text[MAX_STATPAGE_CNT];
	UINT	llTotT5Text2[MAX_STATPAGE_CNT];
	UINT	llTotT5Image[MAX_STATPAGE_CNT];
	UINT	llTotT5Image2[MAX_STATPAGE_CNT];
	UINT	llTotT5Sound[MAX_STATPAGE_CNT];
	UINT	llTotT5Sound2[MAX_STATPAGE_CNT];
	UINT		uContTextCnt[MAX_STATPAGE_CNT];
	UINT		uContImageCnt[MAX_STATPAGE_CNT];
	UINT		uContSoundCnt[MAX_STATPAGE_CNT];
	UINT		uRedirectCnt[MAX_STATPAGE_CNT];
	USHORT		usMaxContRedirectCnt[MAX_STATPAGE_CNT];
	UINT	llTotRedirectDelayTime[MAX_STATPAGE_CNT];
	UINT	llTotRedirectDelayTime2[MAX_STATPAGE_CNT];
	UINT	llMaxRedirectDelayTime[MAX_STATPAGE_CNT];
	UINT	llMaxRedirectDelayTime2[MAX_STATPAGE_CNT];
	USHORT		uLastTextFlag[MAX_STATPAGE_CNT];
	USHORT		uLastImageFlag[MAX_STATPAGE_CNT];
	USHORT		uLastSoundFlag[MAX_STATPAGE_CNT];
	int			dDayFlag[MAX_STATPAGE_CNT];
	int			dWeekFlag[MAX_STATPAGE_CNT];
	int			dMonFlag[MAX_STATPAGE_CNT];
} st_Stat_Page_Hour, *pst_Stat_Page_Hour;

typedef struct _st_Log_SysAlarm
{
    USHORT  usSystemType[MAX_SYSALARM_CNT];
    USHORT  usSystemNo[MAX_SYSALARM_CNT];
    USHORT  usLocType[MAX_SYSALARM_CNT];
    USHORT  usAlmLevel[MAX_SYSALARM_CNT];
    int     dCount[MAX_SYSALARM_CNT];
} st_Log_SysAlarm, *pst_Log_SysAlarm;

typedef struct _st_Stat_SysAlarm_5Min_Insert
{
    int     dStatTime[MAX_SYSALARM_CNT];
    USHORT  usSystemType[MAX_SYSALARM_CNT];
    USHORT  usSystemNo[MAX_SYSALARM_CNT];
    USHORT  usLocType[MAX_SYSALARM_CNT];
    USHORT  usAlmLevel[MAX_SYSALARM_CNT];
    int     dCount[MAX_SYSALARM_CNT];
    int     dHourFlag[MAX_SYSALARM_CNT];
    int     dDayFlag[MAX_SYSALARM_CNT];
    int     dWeekFlag[MAX_SYSALARM_CNT];
    int     dMonFlag[MAX_SYSALARM_CNT];
} st_Stat_SysAlarm_5Min_Insert, *pst_Stat_SysAlarm_5Min_Insert;

typedef struct _st_Stat_SysAlarm_5Min_Select
{
    USHORT  usSystemType[MAX_SYSALARM_CNT];
    USHORT  usSystemNo[MAX_SYSALARM_CNT];
    USHORT  usLocType[MAX_SYSALARM_CNT];
    USHORT  usAlmLevel[MAX_SYSALARM_CNT];
    int     dCount[MAX_SYSALARM_CNT];
    int     dHourFlag[MAX_SYSALARM_CNT];
    int     dDayFlag[MAX_SYSALARM_CNT];
    int     dWeekFlag[MAX_SYSALARM_CNT];
    int     dMonFlag[MAX_SYSALARM_CNT];
} st_Stat_SysAlarm_5Min_Select, *pst_Stat_SysAlarm_5Min_Select;

typedef struct _st_Stat_SysAlarm_Hour
{
    int     dStatTime[MAX_SYSALARM_CNT];
    USHORT  usSystemType[MAX_SYSALARM_CNT];
    USHORT  usSystemNo[MAX_SYSALARM_CNT];
    USHORT  usLocType[MAX_SYSALARM_CNT];
    USHORT  usAlmLevel[MAX_SYSALARM_CNT];
    int     dCount[MAX_SYSALARM_CNT];
    int     dDayFlag[MAX_SYSALARM_CNT];
    int     dWeekFlag[MAX_SYSALARM_CNT];
    int     dMonFlag[MAX_SYSALARM_CNT];
} st_Stat_SysAlarm_Hour, *pst_Stat_SysAlarm_Hour;


typedef struct _st_Stat_SysLoad_5Min_Insert
{
    int     dStatTime[MAX_SYSLOAD_CNT];
    USHORT  usSystemType[MAX_SYSLOAD_CNT];
    USHORT  usSystemNo[MAX_SYSLOAD_CNT];
    USHORT  usLocType[MAX_SYSLOAD_CNT];
    USHORT  usLocIdx[MAX_SYSLOAD_CNT];
    int  	dMax[MAX_SYSLOAD_CNT];
    int  	dAvg[MAX_SYSLOAD_CNT];
    int     dHourFlag[MAX_SYSLOAD_CNT];
    int     dDayFlag[MAX_SYSLOAD_CNT];
    int     dWeekFlag[MAX_SYSLOAD_CNT];
    int     dMonFlag[MAX_SYSLOAD_CNT];
} st_Stat_SysLoad_5Min_Insert, *pst_Stat_SysLoad_5Min_Insert;

typedef struct _st_Stat_SysLoad_5Min_Select
{
    USHORT  usSystemType[MAX_SYSLOAD_CNT];
    USHORT  usSystemNo[MAX_SYSLOAD_CNT];
    USHORT  usLocType[MAX_SYSLOAD_CNT];
    USHORT  usLocIdx[MAX_SYSLOAD_CNT];
    int     dMax[MAX_SYSLOAD_CNT];
    int     dAvg[MAX_SYSLOAD_CNT];
    int     dHourFlag[MAX_SYSLOAD_CNT];
    int     dDayFlag[MAX_SYSLOAD_CNT];
    int     dWeekFlag[MAX_SYSLOAD_CNT];
    int     dMonFlag[MAX_SYSLOAD_CNT];
} st_Stat_SysLoad_5Min_Select, *pst_Stat_SysLoad_5Min_Select;

typedef struct _st_Stat_SysLoad_Hour
{
    int     dStatTime[MAX_SYSLOAD_CNT];
    USHORT  usSystemType[MAX_SYSLOAD_CNT];
    USHORT  usSystemNo[MAX_SYSLOAD_CNT];
    USHORT  usLocType[MAX_SYSLOAD_CNT];
    USHORT  usLocIdx[MAX_SYSLOAD_CNT];
    int     dMax[MAX_SYSLOAD_CNT];
    int     dAvg[MAX_SYSLOAD_CNT];
    int     dDayFlag[MAX_SYSLOAD_CNT];
    int     dWeekFlag[MAX_SYSLOAD_CNT];
    int     dMonFlag[MAX_SYSLOAD_CNT];
} st_Stat_SysLoad_Hour, *pst_Stat_SysLoad_Hour;


typedef struct _st_Log_Url
{
	char	szUrl[MAX_URL_CNT][MAX_URL_LEN];
    UINT    uTrialCnt[MAX_URL_CNT];
    UINT    uSuccessCnt[MAX_URL_CNT];
    INT64    uDuration[MAX_URL_CNT];
    UINT    uDataSize[MAX_URL_CNT];
    INT64    uDataDuration[MAX_URL_CNT];
} st_Log_Url, *pst_Log_Url;

typedef struct _st_Stat_Url_5Min_Insert
{
    int     dStatTime[MAX_URL_CNT];
	char	szUrl[MAX_URL_CNT][MAX_URL_LEN];
    UINT    uTrialCnt[MAX_URL_CNT];
    UINT    uSuccessCnt[MAX_URL_CNT];
    INT64    uDuration[MAX_URL_CNT];
    UINT    uDataSize[MAX_URL_CNT];
    INT64    uDataDuration[MAX_URL_CNT];
    int     dHourFlag[MAX_URL_CNT];
    int     dDayFlag[MAX_URL_CNT];
    int     dWeekFlag[MAX_URL_CNT];
    int     dMonFlag[MAX_URL_CNT];
} st_Stat_Url_5Min_Insert, *pst_Stat_Url_5Min_Insert;

typedef struct _st_Stat_Url_5Min_Select
{
	char	szUrl[MAX_URL_CNT][MAX_URL_LEN];
    UINT    uTrialCnt[MAX_URL_CNT];
    UINT    uSuccessCnt[MAX_URL_CNT];
    INT64    uDuration[MAX_URL_CNT];
    UINT    uDataSize[MAX_URL_CNT];
    INT64    uDataDuration[MAX_URL_CNT];
    int     dHourFlag[MAX_URL_CNT];
    int     dDayFlag[MAX_URL_CNT];
    int     dWeekFlag[MAX_URL_CNT];
    int     dMonFlag[MAX_URL_CNT];
} st_Stat_Url_5Min_Select, *pst_Stat_Url_5Min_Select;

typedef struct _st_Stat_Url_Hour
{
    int     dStatTime[MAX_URL_CNT];
	char	szUrl[MAX_URL_CNT][MAX_URL_LEN];
    UINT    uTrialCnt[MAX_URL_CNT];
    UINT    uSuccessCnt[MAX_URL_CNT];
    INT64    uDuration[MAX_URL_CNT];
    UINT    uDataSize[MAX_URL_CNT];
    INT64    uDataDuration[MAX_URL_CNT];
    int     dDayFlag[MAX_URL_CNT];
    int     dWeekFlag[MAX_URL_CNT];
    int     dMonFlag[MAX_URL_CNT];
} st_Stat_Url_Hour, *pst_Stat_Url_Hour;

typedef struct _st_Log_SysAlarm_Insert
{
	int dCreateTime;
	USHORT usSystemType;
	USHORT usSystemNo;
	USHORT usLocType;
	USHORT usLocIdx;
	USHORT usColIdx;
	USHORT usLogLevel;
	char	szContents[MAX_ALMMSG_SIZE];
} st_Log_SysAlarm_Insert, *pst_Log_SysAlarm_Insert;


typedef struct _st_Log_Sess_Total
{
	UINT	uServerIP[MAX_SESSTOTAL];
	UINT	uClientIP[MAX_SESSTOTAL];
	USHORT	usServerPort[MAX_SESSTOTAL];
	USHORT	usClientPort[MAX_SESSTOTAL];
	UINT	uTcpSynTime[MAX_SESSTOTAL];
	UINT	uTcpSynMTime[MAX_SESSTOTAL];
	USHORT	usHttpSessID[MAX_SESSTOTAL];
	USHORT	usHttpSessCnt[MAX_SESSTOTAL];
	char	szUrl[MAX_SESSTOTAL][MAX_URL_LEN];
	char	szMin[MAX_SESSTOTAL][MAX_MIN_SIZE];
	UINT	uTcpSynackTime[MAX_SESSTOTAL];
	UINT	uTcpSynackMTime[MAX_SESSTOTAL];
	UINT	uGetpostReqTime[MAX_SESSTOTAL];
	UINT	uGetpostReqMTime[MAX_SESSTOTAL];
	UINT	uDataSendStartTime[MAX_SESSTOTAL];
	UINT	uDataSendStartMTime[MAX_SESSTOTAL];
	UINT	uDataSendEndTime[MAX_SESSTOTAL];
	UINT	uDataSendEndMTime[MAX_SESSTOTAL];
	UINT	uMNAckTime[MAX_SESSTOTAL];
	UINT	uMNAckMTime[MAX_SESSTOTAL];
	UINT	uTcpFinTime[MAX_SESSTOTAL];
	UINT	uTcpFinMTime[MAX_SESSTOTAL];
	UINT	uTcpFinackTime[MAX_SESSTOTAL];
	UINT	uTcpFinackMTime[MAX_SESSTOTAL];
	USHORT	usHttpResCode[MAX_SESSTOTAL];
	USHORT	usTcpFailCode[MAX_SESSTOTAL];
	USHORT	usUserErrorCode[MAX_SESSTOTAL];
	UINT	uHttpDataSize[MAX_SESSTOTAL];
	UINT	uPacketCount[MAX_SESSTOTAL];
	UINT	uRetransCount[MAX_SESSTOTAL];
} st_Log_Sess_Total, *pst_Log_Sess_Total;

/* used in CHSMD for check DB Table Space by uamyd0626 06.06.20 */
typedef struct _st_Ts_Info_
{
	char	szTsName[DEF_MAX_TS][DEF_MAXLEN_TS_NAME];
	long	lTotal[DEF_MAX_TS];
	long	lUsed[DEF_MAX_TS];
	long	lFree[DEF_MAX_TS];
	int		dPercent[DEF_MAX_TS];
} st_Ts_Info, *pst_Ts_Info;

typedef struct _st_FltSvcInfo_
{
	USHORT	usSvcCode;
	char	szSvcName[DEF_SVCNAME_LEN];
} st_FltSvcInfo, *pst_FltSvcInfo;

typedef struct _st_Tb_Info_
{
    char    szTbName[DEF_MAX_DEL_TB][DEF_MAXLEN_TABLE_NAME];
} st_Tb_Info, *pst_Tb_Info;



typedef struct _st_Svc_Condition_
{
	int			dCount;
	int			dSvcCode[MAX_SVC_CNT];
} st_Svc_Condition, *pst_Svc_Condition;


typedef struct _st_Dump_Log
{
	int dCount;
	int dStatTime;
	int dSendFlag[12];
} st_Dump_Log, *pst_Dump_Log;

typedef struct _st_User_Info_
{
	char			szUserName[MAX_USER_CNT][USER_NAME_LEN];
	char			szUserPass[MAX_USER_CNT][USER_PASS_LEN];
	short			sUserLevel[MAX_USER_CNT];
	time_t			tLastLogin[MAX_USER_CNT];
	unsigned int	uiLoginIP[MAX_USER_CNT];
} st_User_Info, *pst_User_Info;

typedef struct _st_Cmd_Info_
{
	char			szUserName[MAX_TOT_RECORD][USER_NAME_LEN];
	unsigned int	tTime[MAX_TOT_RECORD];
	short			sResult[MAX_TOT_RECORD];
	char			szCommand[MAX_TOT_RECORD][256];
} st_Cmd_Info, *pst_Cmd_Info;


typedef struct _st_SvcAlm_db
{
	short			sType[MAX_SVC_CNT * 2];
	short			sSvcCode[MAX_SVC_CNT * 2];
	unsigned int	uiCri[MAX_SVC_CNT * 2];
	unsigned int	uiWarn[MAX_SVC_CNT * 2];
	unsigned int	uiReserved[MAX_SVC_CNT * 2];
} st_SvcAlm_db, *pst_SvcAlm_db;

typedef struct _st_SvcAlmName_db
{
    short           sType[MAX_SVC_CNT * 2];
    char           	szSvcName[MAX_SVC_CNT * 2][DEF_SVCNAME_LEN];
    unsigned int    uiCri[MAX_SVC_CNT * 2];
    unsigned int    uiWarn[MAX_SVC_CNT * 2];
    unsigned int    uiReserved[MAX_SVC_CNT * 2];
} st_SvcAlmName_db, *pst_SvcAlmName_db;



typedef struct _st_Del_Cmd_
{
	char			szName[MAX_USER_CNT * 2][USER_NAME_LEN];
} st_Del_Cmd, *pst_Del_Cmd;


/***********************************************************************

*************************************************************************/


//#define MAX_MDN_LEN    16
#define DEF_IMSI_SIZE  16

/* DELETE CONDITION INFOMATION */
typedef struct _st_DelInfo_
{
    int         dCount;
    int         dDelCondition;
    int         dCondition;
    char        szTableName[DEF_MAX_TABLE_NAME][DEF_MAXLEN_TABLE_NAME];
} st_DelInfo, *pst_DelInfo;

/* CHECK INFOMATION */
typedef struct _st_ChkInfo_
{
    int         dCount;
    char        szTsName[DEF_MAX_TS_NAME][DEF_MAXLEN_TS_NAME];
    st_DelInfo  stDelInfo[DEF_MAX_TS_NAME];
} st_ChkInfo, *pst_ChkInfo;

typedef struct _st_MINTraceDB
{
	UINT                   uiCreateTime;
	UCHAR                szIMSI[16];
	UINT                   uiMSIP;
	UINT                   uiFTcpSynTime;
	UINT                   uiFTcpSynMTime;
	UINT                   uiLTcpSynTime;
	UINT                   uiLTcpSynMTime;
	UINT                   uiMenuAckCnt;
} st_MINTraceDB, *pst_MINTraceDB;


typedef struct _st_Test_Log
{
	UINT				test1[10];
	UINT				test2[10];
} st_Test_Log, *pst_Test_Log;

#define DEF_ACT_LEN 15
#define DEF_IDX_LEN 60
#define DEF_TYPE_LEN    20
#define DEF_DETAIL_LEN 128
#define DEF_STMT_LEN 128

typedef struct _st_Sys_AlarmLog_Insert
{
	unsigned int	uiTime;
	unsigned short	usSysType;
	unsigned short	usSysNo;
	unsigned char	ucLocType[DEF_TYPE_LEN + 1];
	unsigned char	ucLocIdx[DEF_IDX_LEN + 1];
	unsigned short	usAlarmLevel;
	unsigned char	szDetailCause[DEF_DETAIL_LEN + 1];
	unsigned char	szReleaseActor[DEF_ACT_LEN + 1];
	unsigned char	szStatement[DEF_STMT_LEN + 1];
	unsigned int	uiClearTime;
} st_Sys_AlarmLog_Insert, *pst_Sys_AlarmLog_Insert;

typedef struct _st_CDLLog_Insert
{
	unsigned int	uiCreateTime;
	unsigned int	uiCreateMTime;
	unsigned short	usVersion;
	unsigned char	ucMsMACAddr[17];
	unsigned short	usEventType;

	unsigned short	usHoInfoFlag;
	unsigned char	ucBsId[17];
	unsigned short	usBasicCid;
	unsigned short	usPrimaryCid;
	unsigned short	usRangingType;

	unsigned short	usArcId;
	unsigned short	usSmcId;
	unsigned short	usAscId;
	unsigned short	usAtcId;
	unsigned short	usApcId;

	unsigned short	usAccId;
	unsigned int	uiRasTrfIp;
	unsigned int	uiRasGreKey;
	unsigned int	uiAcrTrfIp;
	unsigned int	uiAcrGreKey;

	unsigned short	usHoState;
	unsigned short	usHoType;
	unsigned short	usNumActiveBs;
	unsigned char	ucSourceBsId[17];
	unsigned char	ucTargetBsId[17];

	unsigned int	uiTxThroughput;
	unsigned int	uiReTxThroughput;
	unsigned int	uiRxThroughput;
	unsigned short	usNumActiveUser;

	char	szDetailCause[65];

} st_CDLLog_Insert, *pst_CDLLog_Insert;

#define MAX_CONDMSG_MMC 6
#define MAX_CONDMSG_CNT 5000

typedef struct _st_sys_cond_msg
{
	unsigned int   uiTime;
	unsigned char  ucSysType;
	unsigned char  ucNTAMID;
	unsigned char  ucNTAFID;
	unsigned char  ucLocType;
	unsigned char  ucInvType;
	unsigned char  ucInvNo;
	unsigned int   uiIPAddr;
	char           szMessage[255];
}st_SysCONDMsg, *pst_SysCONDMsg;

typedef struct _st_sys_cond_msg_list
{
    int             dCount;
    int             dReserved;
    st_SysCONDMsg   stCONDMsg[MAX_CONDMSG_MMC];
} st_CondMsg_List, *pst_CondMsg_List;


/*****************************************************
for GTAM
***************************************************/
#define MAX_NTAM_NUM 32

typedef struct _st_NtamIP_db
{
    SHORT   sNo[MAX_NTAM_NUM];
    UINT    uiIP[MAX_NTAM_NUM];
} st_NtamIP_db, *pst_NtamIP_db;


#pragma pack(0)

#define DEF_LOGCALL_SIZE    sizeof(st_LogCall)
#define DEF_FTPLOG_SIZE sizeof(st_Ftp_Log_Insert)

typedef struct _st_EQUIP_SE
{
	int				dIdx;
    unsigned int    uiIP;
    int             dSysNo;

    unsigned char   chSysType;
    char            szName[MAX_PDSNNAME+1]; //24+1
    char            szDesc[MAX_SDESC+1];    //64+1
} st_EQUIP_SE;

#define DEF_EQUIP_SE_SIZE	sizeof(st_EQUIP_SE)

/***************************************************** for DB Access **************************************************/

/*
*	USER 정보 조회시 사용되는 구조체 
*/

#define DEF_USER 40
typedef struct _st_User_Add_List
{
    int         dCount;
    int         reserved;

    st_User_Add stUserAdd[DEF_USER];
} st_User_Add_List, *pst_User_Add_List;

/*
*	System Fault 통계를 조회할 때 사용되는 구조체 
*/

typedef struct _st_sys_stat {
    time_t  starttime;
    time_t  finishtime;

    unsigned short  usStatType; /*  FAULT[0x00], LOAD[0x01]         */
    unsigned short  usType;     /*  If usStatType is FAULT: HARDWARE[0x00], SOFTWARE[0x01]  */
    /*  If usStatType is LOAD: CPU[0x00], MEM[0x01], QUEUE[0x02], DISK1[0x03], DISK2[0x04], DISK3[0x05], DISK4[0x06], INVALID_VALUE[0x64]   */

    unsigned short  usSysType;  /*  TAMAPP[0x00], TAF[0x01]         */
    unsigned short  usSysID;

    unsigned int    uiAvg;
    unsigned int    uiCri;
    unsigned int    uiMaj;
    unsigned int    uiMax;
    unsigned int    uiMin;
    unsigned int    uiStop;
    unsigned int    uiCnt;
} st_SYS_STAT,*pst_SYS_STAT;

/*
*	System Threshold 조회시 사용되는 구조체 
*/

#define DEF_THRES_MMC 10
typedef struct _st_Thres_List
{
     int   dCount;
     int   reserved;

     st_Thres stThres[DEF_THRES_MMC];
} st_Thres_List, *pst_Thres_List;

typedef struct _st_Thres_MMC
{
     int   dCount;
     int   reserved;

     st_Thres stThres[DEF_THRES_MMC];
} st_Thres_MMC, *pst_Thres_MMC;

typedef struct _st_ThresMMC
{
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
    unsigned char   cReserved[7];
} st_ThresMMC, *pst_ThresMMC;


typedef struct _st_traffic_stat
{
    time_t          starttime;
    time_t          finishtime;

    unsigned int    uThruStatFrames;
    unsigned long   ulThruStatBytes;

    unsigned int    uTotStatFrames;
    unsigned long   ulTotStatBytes;

    unsigned int    uIPStatFrames;
    unsigned long   ulIPStatBytes;

    unsigned int    uUDPStatFrames;
    unsigned long   ulUDPStatBytes;

    unsigned int    uTCPStatFrames;
    unsigned long   ulTCPStatBytes;

    unsigned int    uSCTPStatFrames;
    unsigned long   ulSCTPStatBytes;

    unsigned int    uETCStatFrames;
    unsigned long   ulETCStatBytes;

    unsigned int    uIPErrorFrames;
    unsigned long   ulIPErrorBytes;

    unsigned int    uUTCPErrorFrames;
    unsigned long   ulUTCPErrorBytes;

    unsigned int    uFailDataFrames;
    unsigned long   ulFailDataBytes;

    unsigned int    uFilterOutFrames;
    unsigned long   ulFilterOutBytes;
} st_traffic_stat, *pst_traffic_stat;

#define DEF_EQUIP 40
typedef struct _st_Info_Equip_List
{
    int             dCount;
    int             reserved;
    st_Info_Equip   stInfoEquip[DEF_EQUIP];
} st_Info_Equip_List, *pst_Info_Equip_List;

typedef struct _st_Info_Equip_MMC
{
  int   dCount;
  int   reserved;
  st_Info_Equip     stInfoEquip[DEF_EQUIP_MMC];
} st_Info_Equip_MMC, *pst_Info_Equip_MMC;

typedef struct _st_InfoEquip_MMC
{
    unsigned int    uEquipIP;
    unsigned int    uNetmask;
    unsigned char   cEquipType;                     /*  SYSTEM_TYPE_PCF, SYSTEM_TYPE_PDSN, SYSTEM_TYPE_AAA, SYSTEM_TYPE_HSS, SYSTEM_TYPE_SERVICE    */
                                                    /*  This is defined in INC/watch_mon.h.                                                         */
    char            szEquipTypeName[DEF_EQUIPNAME]; /*  DEF_EQUIPNAME[20]   */
    char            szEquipName[DEF_EQUIPNAME];     /*  DEF_EQUIPNAME[20]   */
    char            szDesc[MAX_SDESC];              /*  MAX_SDESC[64]       */
    char            cMon1MinFlag;                   /*  1min monitoring flag 1min=1, 5min=0(default) */
    char            cReserved[2];
} st_InfoEquip_MMC, *pst_InfoEquip_MMC;

typedef struct _st_SvcMmc
{
    unsigned int    uSvcIP;
    unsigned short  huPort;
    unsigned char   cFlag;                  /*  RP_FLAG_INDEX,  PI_FLAG_INDEX, CSCF_FLAG_INDEX  */
    unsigned char   cSysType;               /*  PDSN_SYSTYPE, AAA_SYSTYPE, SERVICE_SYSTYPE      */
                                            /*  This is defined in INC/filter.h.                */
    unsigned short  huL4Code;               /*  This is defined in INC/common_stg.h[LPREA_CONF) */
    unsigned short  huL7Code;               /*  This is defined in INC/common_stg.h[LPREA_CONF) */
    unsigned short  huAppCode;              /*  This is defined in INC/common_stg.h[LPREA_CONF) */

    char            szDesc[MAX_SDESC];      /*  MAX_SDESC[64]: This MAX_SDESC definition is in INC/define.h     */
    char            cReserved[2];
} st_SvcMmc, *pst_SvcMmc;

typedef struct _st_SvcInfoList
{
    int         dCount;
    char        reserved[4];
    st_SvcMmc   stSvcMmc[DEF_SVC_MMC];  /*  DEF_SVC_MMC 15 */
} st_SvcInfoList, *pst_SvcInfoList;

/*
*	command history 조회를 위한 구조체 
*/
typedef struct _st_Cmd
{
    char            szUserName[24];
    unsigned int    uiTime;
    unsigned int    uiUserBIP;
    char            szCommand[68];
} st_Cmd, *pst_Cmd;

#define MAX_CMD_CNT 12
typedef struct _st_CmdList
{
    int     dCount;
    int     dReserved;
    st_Cmd  stCmd[MAX_CMD_CNT];
} st_CmdList, *pst_CmdList;

typedef struct _st_MON_ThresMMC
{
    unsigned short  huBranchID;
    unsigned char   cSvcType;
    unsigned char   cAlarmType;
    unsigned char   cStartHour;
    unsigned char   cDayRange;
    unsigned short  huDayRate;
    unsigned short  huNightRate;
    unsigned char   ucReserved1[2];
    unsigned int    uDayMinTrial;
    unsigned int    uNigthMinTrial;
    unsigned int    uPeakTrial;
    char            szDesc[MAX_SDESC];
    unsigned int    dSvcIP;          //added by dcham 20110616
    unsigned char   ucReserved2[4];
} st_MON_ThresMMC, *pst_MON_ThresMMC;

typedef struct _st_MON_Thres
{
    int             dIdx;
    unsigned short  huBranchID;
    unsigned char   cSvcType;
    unsigned char   cAlarmType;
    unsigned char   cStartHour;
    unsigned char   cDayRange;
    unsigned short  huDayRate;
    unsigned short  huNightRate;
    unsigned char   ucReserved1[2];
    unsigned int    uDayMinTrial;
    unsigned int    uNigthMinTrial;
    unsigned int    uPeakTrial;
    unsigned char   ucReserved2[4];
    char            szDesc[MAX_SDESC];
    unsigned int    dSvcIP;          //added by dcham 20110616
    unsigned char   ucReserved3[4];
} st_MON_Thres, *pst_MON_Thres;

#define DEF_MON_THRESHOLD_COUNT 15
typedef struct _st_MON_Thres_List
{
    int             dCount;
    char            cReserved[7];

    st_MON_Thres    stMonThreshold[DEF_MON_THRESHOLD_COUNT];    /*  15  */
} st_MON_Thres_List, *pst_MON_Thres_List;

typedef struct _st_NAS_db
{
    unsigned int    uMNIP;
    unsigned short  usNetMask;
    unsigned char   cFlag;                  /*  RP_FLAG_INDEX,  PI_FLAG_INDEX   */

    unsigned char   cSysType;               /*  PCF_SYSTYPE, PDSN_SYSTYPE, MNIP_SYSTYPE */
                                            /*  This is defined in INC/filter.h.                                                            */
    char            szDesc[MAX_SDESC];      /*  MAX_SDESC[65]: This MAX_SDESC definition is in INC/define.h                                 */
} st_NAS_db, *pst_NAS_db;


#define DEF_MNIP_MMC 15
typedef struct _st_NAS_MMC
{
    int             dCount;
    unsigned char   ucType;             /* SYS TYPE */
    unsigned char   ucReserved[7];
    st_NAS          stNAS[DEF_MNIP_MMC];
} st_NAS_MMC, *pst_NAS_MMC;

/*
*	SCTP 관련 구조체 
*/

typedef struct _st_SCTP_DB
{
    unsigned int    uSCTPIP;
    unsigned char   cSysType;               /*  HSS_SYSTYPE, CSCF_SYSTYPE   */
                                            /*  This is defined in INC/filter.h.                                                            */
    unsigned char   cDirection;             /*  UP, DOWN    */
    unsigned short  huGroupID;
    char            szDesc[MAX_SDESC];      /*  MAX_SDESC[64]: This MAX_SDESC definition is in INC/define.h                                 */
} st_SCTP_DB, *pst_SCTP_DB;

#define DEF_SCTP_MMC 15
typedef struct _st_SCTP_MMC     /*  20090621 추가   */
{
    int         dCount;
    int         dreserved;

    st_SCTP     stSCTP[DEF_SCTP_MMC];		/* in filter.h */
} st_SCTP_MMC, *pst_SCTP_MMC;


#endif	/*	__DB_STRUCT_H__	*/
