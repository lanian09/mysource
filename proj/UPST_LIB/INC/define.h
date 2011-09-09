/*******************************************************************************
			DQMS Project

	Author	: LEE SANG HO
	Section	: DQMS Project
	SCCS ID	: @(#)define.h	1.1
	Date	 : 01/15/03
	Revision History :
		'03.	1.	15 Initial

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __DEFINE_H__
#define __DEFINE_H__

#include <string.h>

#define RET_SUCC			1
#define RET_FAIL           -1
//#define NONE		    	0
//#define ECHO		    	2

#define DEF_FLAG_OFF	0x00
#define DEF_FLAG_ON		0x01

#define WIDTH			16
#define DEF_IRM_LEN		10


#define DEF_INIT_FILE				START_PATH"/NEW/DATA/INIT_NTAF.dat"
#define DEF_VER_FILE				START_PATH"/NEW/DATA/SW_VER.dat"

#define DEF_MACS_TIMEOUT_FILE		START_PATH"/NEW/DATA/MACS_TIMEOUT.dat"
#define DEF_WICGS_TIMEOUT_FILE		START_PATH"/NEW/DATA/WICGS_TIMEOUT.dat"

#define MAX_MIN_SIZE				16		/* Calling Station ID */

#define MAX_MODEL_SIZE				17
#define MAX_MODEL_LEN				(MAX_MODEL_SIZE - 1)

#define MAX_NTAF_MULTI				2		/* NTAF Muliple Count : Example IWF -> 2, PDSN -> 1 */
#define MAX_NTAF_PAIR				2		/* NTAF Pair Count	: Allways 2 */

#define MAX_CATESVC_COUNT			21		/* Max Category Service Count */
#define MAX_WATCH_COUNT				10		/* Max Watch Count */

#define MAX_IMSI_SIZE				17		/* MAX DSCP IMSI Size */
#define MAX_WINSVCNAME_SIZE			6		/* MAX DSCP WIN Service Name Size */
#define MAX_CELLINFO_SIZE			17		/* MAX DSCP CELLINFO Size */

#define MAX_FILENAME_SIZE			256	 /* File Name Size */

#define MAX_CON_SIZE				64		/* MAX Category content size */
#define MAX_CON_COUNT				100		/* MAX Category content count */

#define MAX_HOSTNAME_SIZE			41
#define MAX_HOSTNAME_LEN			(MAX_HOSTNAME_SIZE - 1)

/* DIAMETER COMMAND CODE */
#define ACCOUNTING								271
#define CREDIT_CONTROL							272
#define RE_AUTH									258

/* DIAMETER AVP CODE */
#define SESSION_ID								263
#define ORIGIN_HOST								264
#define	ORIGIN_REALM							296
#define DESTINATION_HOST						293
#define DESTINATION_REALM						283
#define	AUTH_APPLICATION_ID						259
#define CC_REQUEST_NUMBER						415
#define CC_REQUEST_TYPE							416
#define SUBSCRIPTION_ID							443
#define SUBSCRIPTION_ID_TYPE					450
#define SUBSCRIPTION_ID_DATA					444
#define ORIGIN_STATE_ID							278
#define PROXY_INFO								284
#define PROXY_HOST								280
#define PROXY_STATE								33
#define BEARER_OPERATION						1021
#define GCID									502
#define ICID									505
#define INSERVICE								10001
#define	BEARER_OPERATION						1021
#define RESULT_CODE								268
#define EXPERIMENTAL_RESULT						297
#define EXPERIMENTAL_RESULT_CODE				298
#define PCC_RULE_STATUS							1019
#define	CHARGING_SERVER							618
#define CAUSE_CODE								861
#define ACCOUNTING_RECORD_TYPE					480
#define CALLED_STATION_ID						40
#define ACCESS_NETWORK_CHARGING_IDENTIFIER_GX	1022
#define CHARGING_RULE_INSTALL					1001
#define IMS_INFORMATION							876
#define CHARGING_RULE_REPORT					1018
#define CHARGING_RULE_DEFINITION				1003
#define RE_AUTH_REQUEST_TYPE					285
#define SERVICE_INFORMATION						873

/* Cx */
#define EXCEPTIONAL_CASE							0
#define VISITED_NETWORK_IDENTIFIER					600
#define PUBLIC_IDENTITY								601
#define SERVER_NAME									602
#define SERVER_CAPABILITIES 						603
#define MANDATORY_CAPABILITY 						604
#define OPTIONAL_CAPABILITY 						605
#define USER_DATA 									606
#define SIP_NUMBER_AUTH_ITEMS 						607
#define SIP_AUTHENTICATION_SCHEME 					608
#define SIP_AUTHENTICATE							609
#define SIP_AUTHORIZATION							610
#define SIP_AUTHENTICATION_CONTEXT					611
#define SIP_AUTH_DATA_ITEM							612
#define SIP_ITEM_NUMBER								613
#define SERVER_ASSIGNMENT_TYPE 						614
#define DEREGISTRATION_REASON 						615
#define REASON_CODE									616
#define REASON_INFO									617
#define CHARGING_INFORMATION 						618
#define PRIMARY_EVENT_CHARGING_FUNCTION_NAME 		619
#define SECONDARY_EVENT_CHARGING_FUNCTION_NAME 		620
#define PRIMARY_CHARGING_COLLECTION_FUNCTION_NAME 	621
#define SECONDARY_CHARGING_COLLECTION_FUNCTION_NAME 622
#define USER_AUTHORIZATION_TYPE 					623
#define USER_DATA_ALREADY_AVAILABLE 				624
#define CONFIDENTIALITY_KEY 						625
#define INTEGRITY_KEY 								626
#define SUPPORTED_FEATURES							628
#define FEATURE_LIST_ID								629
#define FEATURE_LIST 								630
#define SUPPORTED_APPLICATIONS 						631
#define USER_IDENTITY 		 						700


/* FOR DIAMETER */
#define MAX_HOST_REALM_SIZE		 	64
#define MAX_HOST_REALM_LEN			(MAX_HOST_REALM_SIZE - 1)
#define MAX_SESSIONID_SIZE			128
#define MAX_SESSIONID_LEN			(MAX_SESSIONID_SIZE - 1)
#define MAX_SUBSCRIPTION_SIZE		512
#define MAX_SUBSCRIPTION_LEN		(MAX_SUBSCRIPTION_SIZE - 1)
#define MAX_PROXYHOST_SIZE			512
#define MAX_PROXYHOST_LEN			(MAX_PROXYHOST_SIZE - 1)
#define MAX_PROXYSTATE_SIZE			128
#define MAX_PROXYSTATE_LEN			(MAX_PROXYSTATE_SIZE - 1)
#define MAX_CALLED_STATION_ID_SIZE 	32
#define MAX_CALLED_STATION_ID_LEN 	(MAX_CALLED_STATION_ID_SIZE - 1)
#define MAX_GCID_SIZE				64
#define MAX_GCID_LEN				(MAX_GCID_SIZE - 1)
#define MAX_ICID_SIZE				64
#define MAX_ICID_LEN				(MAX_ICID_SIZE - 1)
#define MAX_ERR_MSG_SIZE			32
#define MAX_ERR_MSG_LEN				(MAX_ERR_MSG_SIZE -1)
#define MAX_INSERVICE_SIZE			8
#define MAX_INSERVICE_LEN			(MAX_INSERVICE_SIZE -1)

#define MAX_SHRTURL_LEN				64
#define MAX_HOST_LEN				40
#define MAX_USERAGENT_LEN			101
#define MAX_BUFFER_LEN				3072


#define MAX_DATE_LEN				32
#define MAX_CONTYPE_LEN				64
#define MAX_HASHCODE_LEN			17
#define MAX_BILLADD_LEN				101

#define MAX_MODELINFO_LEN			16
#define MAX_BASEID_LEN				24
#define MAX_MMSVER_LEN				8
#define MAX_ADDRESS_LEN				24
#define MAX_TITLE_LEN				64
#define MAX_SVCNAME_LEN				32
#define MAX_MENUCODE_LEN			4
#define MAX_NAME_SIZE				64
#define MAX_PROTO_LEN				16


//#define MAX_NTAF_NUM				16
//#define DEF_MSG_CNT					((MAX_NTAF_NUM)+1)

#define DEF_SVCNAME_LEN				32
#define MAX_SVC_CNT					2000	/* #define MAX_SVC_CNT	100 */

#define	MAX_SCTP_COUNT				200
#define DEF_MNIP_COUNT				32

#define MAX_MS						30
#define MAX_MSNAME					20

#define MAX_SNA						5
#define MAX_SNAURL					64
#define MAX_MENUNAME				64
#define MAX_SDESC					64

#define MAX_DEST_FLT				100
#define MAX_ALMTYPE					7


/* DEFINE LOG LEVEL */
#ifdef NEW_LOGDEFINE
	#define LOGN_NOPRINT			0
	#define LOGN_CRI				1
	#define LOGN_WARN				2
	#define LOGN_DEBUG				3
	#define LOGN_INFO				4
#else
	#define LOG_NOPRINT				0
	#define LOG_CRI					1
	#define LOG_WARN				2
	#define LOG_DEBUG				3
	#define LOG_INFO				4
#endif

#define LOG_TYPE_DEBUG				1
#define LOG_TYPE_WRITE				2

#define TCP_FIN						0x01
#define TCP_SYN						0x02
#define TCP_RST						0x04
#define TCP_PSH						0x08
#define TCP_ACK						0x10
#define TCP_SYNACK					0x12

/* ADD BY HWH */
#define DEF_FILLTEROUT_OFF			0x00
#define DEF_FILLTEROUT_ON			0x01
#define DEF_FILLTEROUT_RPASS		0x02
#define DEF_QUD_SND_TIME			5

#define DEF_CALL_CLEAR_NOREMAIN		1000

/* ADD BY JHL */
typedef enum
{
	DEF_HTTP_PARA	= 0,
	DEF_METHOD_GET,
	DEF_METHOD_POST,
	DEF_METHOD_CONNECT,
	DEF_METHOD_OPTIONS,
	DEF_METHOD_HEAD,
	DEF_METHOD_PUT,
	DEF_METHOD_DELETE,
	DEF_METHOD_TRACE,
	DEF_METHOD_RESULT,
	DEF_METHOD_ETC,			/* 10 */
	DEF_RET_HOST,
	DEF_RET_USERAGENT,
	DEF_RET_PHONE,
	DEF_RET_CONLEN,
	DEF_RET_ENCODE,
	DEF_RET_CHANNEL,
	DEF_RET_PHONEPARA,
	DEF_RET_MMSAPP,
	DEF_RET_MMSVERSION,
	DEF_RET_MMSTO,			/* 20 */
	DEF_RET_MMSFROM,
	DEF_RET_MMSMOBILETYPE,
	DEF_RET_MMSCALLBACK,
	DEF_RET_MMSTITLE,
	DEF_RET_CONTYPE,
	DEF_RET_COOKIE,
	DEF_RET_CPDATA,
	DEF_RET_LOCATION,
	DEF_RET_VERSION,
	DEF_RET_CDU				/* 30 */
} DEF_HTTP_REQ_PARA;

typedef enum
{
	TYPE_UNKNOWN = 0,
	TYPE_TEXT,
	TYPE_IMAGE,
	TYPE_SOUND,
	TYPE_HTML,
	TYPE_WML,
	TYPE_DESC,
	TYPE_STYLE,
	TYPE_LMSG,
	TYPE_MMSG,
	TYPE_VOD,
	TYPE_XML,
	TYPE_APPLICATION,	/* 12 */
	TYPE_BREWDOWN	= 20,
	TYPE_WIPIDOWN	= 21,
	TYPE_NDWN		= 601,
	TYPE_MA2		= 610,
	TYPE_MA3		= 611,
	TYPE_MA5		= 612,
	TYPE_SIS		= 650
} DEF_CONTENT_TYPE;

/* SVC TYPE */
#define NOSVCTYPE					0x00
#define SVCTYPE_MENU				0x01
#define SVCTYPE_DOWN				0x02
#define SVCTYPE_MMS					0x04
#define SVCTYPE_STREAM				0x08
#define SVCTYPE_SMPP				0x20
#define SVCTYPE_FTP					0x40
#define SVCTYPE_IMS					0x80
#define SVCTYPE_DEFAULT				0xFD

/* SVC CODE */
#define SVCCODE_IMS_SIP				45
#define SVCCODE_IMS_MSRP			46
#define SVCCODE_IMS_XCAP			47

/* Added by syhan */
#define KTFVODSAPPCODE				5
#define LGTVODSAPPCODE				55

#define MAX_ACCTID_LEN				19


/*****************************************************************/
/* used in 'dGetMyHostId()' in 'libLogUtil.a' in 'CHSMD' etc	 */
/*****************************************************************/
#define DEF_HOSTNAME	"localhost.localdomain"
#define DEF_HOSTNAME2	"localhost.localdomain"
/* <**************************************************************/

#define DEF_APN_LEN 		33

#define MAX_STMT_SIZE		1024

#define TIME_LEN			16
#define MAX_TOT_RECORD		2000
#define USER_NAME_LEN		16
#define USER_PASS_LEN		16
#define MAX_USER_CNT		200

#define MAX_MENUCODE_LEN	4

#define ST_MAX_FCT			20
#define USERINFO			21
#define USERLN				21

#define MAX_SDESC			64

#define MAX_HOST_LEN		40
#define MAX_HASHCODE_LEN	17

#define MAX_NTAF_NUM		8
#define DEF_MSG_CNT		 ((MAX_NTAF_NUM)+1)
/* #define MAX_WATCH_RECORD_IN_TIME	500 */
#define BSMSC_SIZE			13
#define MAX_ALMTYPE		 	7
#define MAX_DEST_FLT		100
#define DEF_SVCNAME_LEN	 	32
#define PDSNNAME			24
#define MAX_MNIP_COUNT		10000	/* 2000개와 비교 */
#define MAX_SCTP_COUNT		200

#define MAX_LOG_CNT		 	4500
#define MAX_TRCMSG_SIZE	 	2700	/* trace msg max length */
#define MAX_MIN_SIZE		16		/* Calling Station ID */
#define MAX_USERNAME_SIZE	65
#define MAX_ESN_SIZE		9
#define	MAX_BSMSC_SIZE		13
#define MAX_MSISDN_SIZE		17
#define MAX_WINSVC_SIZE		7
#define MAX_WINSVC_SIZE_NEW	6
#define MAX_AUTH_SIZE		16

#define MAX_NTAF_MULTI		2 		/* NTAF Muliple Count : Example IWF -> 2, PDSN -> 1 */
#define MAX_NTAF_PAIR		2 		/* NTAF Pair Count	: Allways 2 */

#define MAX_CATESVC_COUNT	21		/* Max Category Service Count */
#define MAX_WATCH_COUNT		10		/* Max Watch Count */


#define MAX_IMSI_SIZE		17		/* MAX DSCP IMSI Size */
#define MAX_WINSVCNAME_SIZE	6		/* MAX DSCP WIN Service Name Size */
#define MAX_CELLINFO_SIZE	17		/* MAX DSCP CELLINFO Size */

#define MAX_FILENAME_SIZE	256		/* File Name Size */

#define MAX_RELOAD_COUNT	30
#define MAX_RELOAD_MAP_PRC	10

/* GT INFO */
#define DEF_GT_LEN 			20

#define MAX_MENUNAME 		64

#define DEF_THRES_MMC 		10
#define DEF_EQUIP_MMC 		15
#define DEF_VIEW_MMC 		10
/*	START: GI NTAF에게만 SCTP filter 정보를 보내주기 위하여	*/
#define	MAX_SCTP_CNT		48
#define	DEF_SCTPID_MMC		MAX_SCTP_CNT
/*	END: Writer: Han-jin Park, Date: 2009.05.11	*/
#define DEF_WSTYPE_GEN		250

#define DEF_NASTYPE_PDSN	1		/* NASTYPE PDSN */
#define DEF_NASTYPE_IWF		2		/* NASTYPE IWF */
#define DEF_NASTYPE_AAA		3		/* NASTYPE AAA */

#define MAX_RADIUS_PKTSIZE	5000	/* Radius Packet Max Size */

#define MAX_PDSN			16		/* Max PDSN number */
#define MAX_IWF	 			64		/* Max IWF	number */
#define MAX_AAA	 			32		/* Max AAA number */
#define MAX_IDENTIFIER		256		/* Max Identifier count */

#define DEF_NTAF_ACT		1		/* NTAF ACTVIE MODE */
#define DEF_NTAF_STA		2		/* NTAF SECONDARY MODE */
#define DEF_NTAF_TIME		255		/* NTAF TIME OUT */

#define DEF_SYS_SESSIF		1
#define DEF_SYS_SESSSVC	 	2
#define DEF_SYS_NTAFUIF	 	3
#define DEF_SYS_DSCPIF		4
#define DEF_SYS_NTAFTIF	 	5
#define DEF_SYS_CHSMD		6
#define DEF_SYS_MMCD		7
#define DEF_SYS_FSTATD		8
#define DEF_SYS_SESSMNG		9

/* R2.0.0 Add 2004.0407 (lander) */
#define	DEF_SYS_IDRIF		10
#define DEF_SYS_MRDRIF		11

#define DEF_MMC_TIMEOUT		25

#define DEF_NTAF_PAIR		16

#define DEF_NAS_PDSN1X		1		/* PDSN 1X */
#define DEF_NAS_PDSNEV		2		/* PDSN 1X-EVDO */
#define DEF_NAS_PDSNAI		3		/* PDSN AIWU */
#define DEF_NAS_PDSND		4		/* PDSN Default */
#define DEF_NAS_IWF			5		/* IWF */
#define DEF_NAS_IWFA		6		/* AIWF */
#define DEF_NAS_IWFN		7		/* NIWF */
#define DEF_NAS_IWFD		8		/* IWF-DEFAULT */
#define DEF_NAS_GGSN		9		/* GGSN */
/* DPP_R1.0.0 Add 2004.1101 (lander) ---> */
#define DEF_NAS_WAG			10		/* WLAN-KT */
#define DEF_NAS_KTONE		11		/* KT-ONE-Phone */
/* <--- */

#define DEF_ACCT_REQ_START		1	// Accounting Start
#define DEF_ACCT_REQ_STOP		2	// Accounting Stop
#define DEF_ACCT_REQ_IN			5	// Intelligent Network
#define DEF_ACCT_REQ_ACCT_OFF	8	// Accounting-OFF
#define DEF_ACCT_INTERIM		3	// Interim-Update for GGSN

#define HASH_MODELINFO_CNT		10000019
//#define HASH_MODELINFO_CNT	10021


#define DEF_MSISDN_CALLSTATID_CALL		1	// CALLSTATID EXIST but MSISDN NOT EXIST
#define DEF_MSISDN_CALLSTATID_MSISDN	2	// CALLSTATID NOT EXIST but MSISDN EXIST

/* DEFINE LOG LEVEL */
#define LOG_NOPRINT			0
#define LOG_CRI				1
#define LOG_WARN			2
#ifndef SNMPIF
#define LOG_DEBUG			3
#define LOG_INFO			4
#endif

#define LOG_TYPE_DEBUG		1
#define LOG_TYPE_WRITE		2

/* DEFINE TRACE TYPE */
#define TRACE_IMSI			1		// IMSI
#define TRACE_MSISDN		2		// MSISDN
#define TRACE_STR_IMSI		"IMSI"
#define TRACE_STR_MSISDN	"MSISDN"

#define NAS_ANRELINDI_OK	1
#define NAS_ANRELINDI_DROP	2

#define DSCP_OFF			6
#define DSCP_DROP			7

#define MAX_INDEX_VALUE		60000
#define DEF_INDEX_INIT		100

#define DEF_ENUM_IMSI		75
#define DEF_ENUM_MSISDN	 112
#define DEF_ENUM_SRCIP		113

/* DPP_R1.0.0 Change 2004.1102 (lander) AAA_SOCK 2->3 */
#define MAX_AAA_SOCK		3

#define MAX_TIME_LEN		48			/* added by LYH 20051105 for MMC */

#define DEF_TIMEOUT		 1
#define DEF_SUCCESS		 2
#define DEF_FAIL			3
#define DEF_NTAF_TIMEOUT	4
#define DEF_NTAF_SUCCESS	5
#define DEF_NTAF_FAIL		6
#define DEF_NTAM_NOT_LOGON	7
#define DEF_NTAF_NOT_LOGON	8

/*
*	ADDED BY UAMYD0626 06.06.07 FOR W-NTAS
*/
#define MAX_NTAF_PAIR		2
#define MAX_IMSI_SIZE		17		/* MAX DSCP IMSI Size */
#define MAX_WINSVCNAME_SIZE 6		/* MAX DSCP WIN Service Name Size */
#define MAX_CELLINFO_SIZE	17		/* MAX DSCP CELLINFO Size */
#define MAX_CATESVC_COUNT	21		/* Max Category Service Count */
#define MAX_CON_SIZE		64		/* MAX Category content size */
#define MAX_CON_COUNT		100	 /* MAX Category content count */


/* #define RP_NETWORK_MSG_NUM			6		// SGSN, DNS, RNC, AAA, IPAS, WIN */
/* #define MAX_WATCHABLE_SERVICE_NUM	4		// MAGICN, MULTIPACK, MMS, FIMM */
/* #define MAX_WATCHABLE_MSG_NUM		(RP_NETWORK_MSG_NUM)+(MAX_WATCHABLE_SERVICE_NUM) */


#define DEF_MAX_HEADERLEN		256
#define MAX_REQRES_HEADER		((DEF_MAX_HEADERLEN)*2+8)

#define NONSTOP	 			0x00
#define STOP					0x01

#define MAX_DISK_COUNT			4
#define MAX_OMP_COUNT			20
//#define MAX_SW_COUNT				32		/* 70 > 64 : BY UAMYD0626 06.06.07, for w-ntam */
#define MAX_DEST_FLT				100
#define MAX_TMFNO	 			16
#define DEF_MAX_IDX	 		16

#define MAX_ONOFF 				5
#define ST_MAX_FCT 				20
#define ST_MAX_FSO 				20
#define MAX_METHOD 				20
#define MAX_REQUEST 			20

#define MAX_VERINFO_LEN	 		16
#define MAX_LOG_CNT				 4500
#define MAX_TCPLOG_CNT				1500
#define MAX_PAGELOG_CNT			 1500
#define MAX_VODLOG_CNT				1500
#define MAX_MEDIALOG_CNT			1500
#define MAX_RTSPLOG_CNT			 1500
#define MAX_SOAPLOG_CNT			 1500
#define MAX_SIPLOG_CNT				1500
#define MAX_HEADER_LENGTH			199
#define MAX_DOMAINNAME_LENGTH		50
#define MAX_LONGURL_SIZE			1001
#define MAX_COR_CNT				 1500
#define MAX_SQL_SIZE				1024
#define DELETE_TABLE_TIME			10
#define MAX_STMT_SIZE2				2048
#define MAX_PDSNNAME	24
#define MAX_BASEID_LEN				24
#define MAX_MODELINFO_LEN				16
#define MAX_MS_MAN_LEN 16
#define MAX_SVCNAME_LEN 			32
#define MAX_PAGENAME_LEN			32
#define MAX_TABLE_NAME_SIZE		 50
#define MAX_SESS_CNT				1500
#define MAX_DHUB_CHNL				4		/* Changed By Calm1004 20050908 */
#define MAX_BDF_NUM	 			8
#define DEF_SQLSTMT_LEN	 		1024
#define DEF_TABLENAME_LEN			256
#define MAX_HTTP_BUF 				256
#define DEF_MAX_STS_COM	 		32
#define MAX_HASH_SIZE					MAX_HASHCODE_LEN
#define MAX_NAME_SIZE					64
#define MAX_ADDR_SIZE					24
#define MAX_PROTO_LEN				16
#define MAX_TITLE_LEN				64
#define DEF_MNIP_COUNT				32
#define PDSNNAME					24
#define MAX_VER_SIZE				16
#define MAX_GUID_SIZE				16
#define MAX_CCID_SIZE				16
#define MAX_WVMODEL_SIZE			16
#define MAX_WVTRANS_SIZE			8
#define MAX_SOAPSVCTYPE_SIZE		8
#define MAX_URI_SIZE				32
#define MAX_LURI_SIZE				64
#define MAX_INFOEQUIP_ARRAY 		128

#define HEADER_LOG_DROP	 		0
#define HEADER_LOG_CAPTURE			1

#define DEF_INSERTCNT				300

/*
#define SEC_OF_MIN					60
#define SEC_OF_5MIN	 			300
#define SEC_OF_HOUR	 			3600
#define SEC_OF_DAY					86400
#define SEC_OF_WEEK	 			604800
#define SEC_OF_MON					2592000
#define SEC_OF_YEAR	 			31536000
*/

#define DEF_MAX_BUF_SIZE			8192*1000

#define MAX_MEDIATYPE_SIZE			16

#define MAX_USER_NAME				25
#define MAX_MMC_COMMAND_LEN			256

#define DIAMETER_HOST_REALM_SIZE			64
#define DIAMETER_HOST_REALM_LEN			(DIAMETER_HOST_REALM_SIZE - 1)
#define DIAMETER_SESSID_SIZE				128
#define DIAMETER_SESSID_LEN			 (DIAMETER_SESSID_SIZE - 1)
#define DIAMETER_CALLED_STATION_ID_SIZE 32
#define DIAMETER_CALLED_STATION_ID_LEN	(DIAMETER_CALLED_STATION_ID_SIZE - 1)
#define DIAMETER_GCID_SIZE				64
#define DIAMETER_GCID_LEN				(DIAMETER_GCID_SIZE - 1)
#define DIAMETER_ICID_SIZE				64
#define DIAMETER_ICID_LEN				(DIAMETER_ICID_SIZE - 1)
#define DIAMETER_ERR_MSG_SIZE			32
#define DIAMETER_ERR_MSG_LEN				(DIAMETER_ERR_MSG_SIZE - 1)
#define DIAMETER_INSERVICE_SIZE			8
#define DIAMETER_INSERVICE_LEN			(DIAMETER_INSERVICE_SIZE - 1)

/* MMCD 에서만 사용됨....
//#define	MAX_MMCD_MSG_SIZE			4000
//#define	MAX_MMCD_MSG_SIZE			(MSG_DATA_LEN+1000)
typedef struct _st_sys_mmcd_msg
{
	char			szUserName[MAX_USER_NAME];
	unsigned int	uiTime;
	unsigned int	uiUserBIP;
	unsigned short	usSeq;
	unsigned short	usResult;
	unsigned char	szCommand[MAX_MMC_COMMAND_LEN];
	unsigned char	szMessage[MAX_MMCD_MSG_SIZE];
}st_SysMMCDMsg, *pst_SysMMCDMsg;
*/

#define MAX_DEFECT_THRES 32	 /* filter.h, watch_filter.h 중복 */
#define MAX_USER		 500 /* mmcdef.h, filter.h 중복 */

#endif	/*	__DEFINE_H__	*/

