
#ifndef __RDRHEADER_H_
#define __RDRHEADER_H_

#define DEF_VALUE_LIMIT 256
#define MAX_RDR_FIELD_NUM	32
#define SIZE_OF_RDR_HEADER	20
#define MAX_PKG_ID_NUMBER 65536

// RDR Filed Type ID tag.
enum TYPEID { T_INT8 = 11, T_INT16, T_INT32, T_UINT8, T_UINT16, T_UINT32, T_BOOLEAN, T_STRING=41 };

// RDR Tag ID
#define TAG_BASE	4042320000
#define BLOCK		1984
#define RPT_NUR		1920
#define RPT_SUR		1922
#define RPT_PUR		1924
#define RPT_LUR		1925
#define RPT_TR		1936
#define RPT_MALUR	2000
#define RPT_TUR		3000
#define RPT_RTSP	3008
#define RPT_HTTP	3004
#define RPT_VOIP	3050
#define RPT_RADIUS	1987

#define OK_RDR 1
#define SYNC 1
#define NOT_SYNC 0
#define BUFFERING_SIZE  2048


// Table
#define RPT_BLOCK_TBL 	"RPT_BLOCK"
#define RPT_TR_TBL 		"RPT_TR"
#define RPT_LUR_TBL 	"RPT_LUR"
#define RPT_HTTP_TBL	"RPT_HTTP"

#define MAX_STRING_LEN	128
#define MAX_SMS_STRING_LEN	80

#define SMS_TBL                     "sms_history"       /* sms history table name */ 
typedef struct {
#define SMS_DB_IP_MAX_SIZE         16
#define SMS_DB_NAME_MAX_SIZE       8
#define SMS_DB_USER_MAX_SIZE       16
#define SMS_DB_PASSWD_MAX_SIZE     16
#define SMS_DB_TABLE_MAX_SIZE      16
	char        ipaddr[SMS_DB_IP_MAX_SIZE];
	char        name[SMS_DB_NAME_MAX_SIZE];
	char        user[SMS_DB_USER_MAX_SIZE];
	char        passwd[SMS_DB_PASSWD_MAX_SIZE];
	char        table[SMS_DB_TABLE_MAX_SIZE];
	char		sms_msg[MAX_SMS_STRING_LEN];
} SMS_DB_INFO;

SMS_DB_INFO sms_db;

#define MAX_SMS_TEXT_LEN	160
#define MAX_SUBS_ID_LEN		16
#define MAX_ID_LEN			8
#define MAX_SESSCNT_LEN		8
#define MAX_IP_LEN			16
#define MAX_PORT_LEN		8
#define MAX_ACCS_STR_LEN	256
#define MAX_TIME_LEN		11
#define MAX_SHORT_TIME_LEN	6
#define MAX_VOLUME_LEN		12
#define MAX_SIGNATURE_LEN	12

// CSV 파일에서 토큰(,)으로 구분된 문자열을 차례대로 필드에 넣어둔다. 
typedef struct _BLK_CSV_
{
	int		fieldCnt;			// field 개수 
	char	szTimeStamp[14]; 	// 1247632583118 : ( time_t 1247632583 + msec 118 )
	char	szTag[11]; 			// 4042321936 : Transaction, 4042321984 : Block 
	char	szSceIp[16];		// 192.168.100.3 string type
	
	char	szSubsID[MAX_SUBS_ID_LEN]; 	// IMSI : 450061024447290
	char	szPkgID[MAX_ID_LEN];	 	// 0 ~ 4999	
	char	szSvcID[MAX_ID_LEN];	 	// 0 ~ 1000	
	char	szProtoID[MAX_ID_LEN];		// 0 ~ 1000	
	char	szCliIP[MAX_IP_LEN];		// Client IP : 178322974 inet_ntoa() 로 변환해야함. 
	char	szCliPort[MAX_PORT_LEN];	// Client Port
	char	szSrvIP[MAX_IP_LEN];		// Server IP : 178322974 inet_ntoa() 로 변환해야함. 
	char	szSrvPort[MAX_PORT_LEN];	// Server Port
	char	szInitSide[2];					// Initiating Side : 0 or 1
	char	szAccStr[MAX_ACCS_STR_LEN];
	char	szInfoStr[MAX_ACCS_STR_LEN];
	char	szBlkReason[2]; 			// 128 : bit별로 의미를 가진다. 
	char	szBlkCnt[MAX_SESSCNT_LEN];
	char	szBlkTime[MAX_TIME_LEN];					// time_t : 1247493297
} BLK_CSV;

typedef struct _RDR_BLOCK_
{
	int				dTimeStamp; 	// 1247632583118 : ( time_t 1247632583 + msec 118 )
	char			szTag[11]; 			// 4042321936 : Transaction, 4042321984 : Block 
	char			szSceIp[MAX_IP_LEN];

	unsigned char	ucSubscriberID[MAX_SUBS_ID_LEN];
	unsigned short	usPackageID;
	int				dServiceID;
	short			dProtocolID;
	unsigned int	uiClientIP;
	char			szCliIP[MAX_IP_LEN];
	unsigned short	usClientPort;
	unsigned int	uiSrcIP;
	char			szSrcIP[MAX_IP_LEN];
	unsigned short	usSrcPort;
	unsigned char	ucInitSide;
	unsigned char	ucAccString[MAX_ACCS_STR_LEN];
	unsigned char	ucInfoString[MAX_ACCS_STR_LEN];
	unsigned char	ucBlkReason;
	unsigned int	uiBlkRdrCnt;
	unsigned char	ucRedirect;
	unsigned int	uiTime;
} RDR_BLOCK;


// CSV 파일에서 토큰(,)으로 구분된 문자열을 차례대로 필드에 넣어둔다. 
typedef struct _TRAN_CSV_
{
	int		fieldCnt;			// field 개수 
	char	szTimeStamp[14]; 				// 1247632583118 : ( time_t 1247632583 + msec 118 )
	char	szTag[11]; 						// 4042321936 : Transaction, 4042321984 : Block 
	char	szSceIp[16];					// 192.168.100.3 string type
	
	char	szSubsID[MAX_SUBS_ID_LEN]; 		// IMSI : 450061024447290
	char	szPkgID[MAX_ID_LEN];	 		// 0 ~ 4999	
	char	szSvcID[MAX_ID_LEN];	 	
	char	szProtoID[MAX_ID_LEN];		
	char	szSessCnt[MAX_SESSCNT_LEN];		
	char	szSvrIP[MAX_IP_LEN];			// Server IP : 178322974 inet_ntoa() 로 변환해야함. 
	char	szSvrPort[MAX_PORT_LEN];		// Server Port
	char	szAccStr[MAX_ACCS_STR_LEN];
	char	szInfoStr[MAX_ACCS_STR_LEN];
	char	szCliIP[MAX_IP_LEN];			// Client IP : 178322974 inet_ntoa() 로 변환해야함. 
	char	szCliPort[MAX_PORT_LEN];		// Client Port
	char	szInitSide[2];					// Initiating Side : 0 or 1
	char	szRptTime[MAX_TIME_LEN];		// time_t : 1247493297
	char	szMilDura[MAX_SHORT_TIME_LEN]; 	// miliseconds
	char	szTimeFrame[MAX_TIME_LEN];
	char 	szUpVol[MAX_VOLUME_LEN];
	char 	szDnVol[MAX_VOLUME_LEN];
	char	szSubCntId[MAX_ID_LEN];
	char	szGblCntId[MAX_ID_LEN];
	char 	szPkgCntId[MAX_ID_LEN];
	char	szIpProto[MAX_ID_LEN];
	char	szProtoSig[MAX_SIGNATURE_LEN];
	char 	szZoneID[MAX_ID_LEN];
	char	szFlvID[MAX_ID_LEN];
	char	szClose[2];
} TRAN_CSV;

typedef struct _RDR_TR_
{
	int				dTimeStamp; 	// 1247632583118 : ( time_t 1247632583 + msec 118 )
	char			szTag[11]; 			// 4042321936 : Transaction, 4042321984 : Block 
	char			szSceIp[MAX_IP_LEN];

	unsigned char	ucSubscriberID[MAX_SUBS_ID_LEN];
	unsigned short	usPackageID;
	int				dServiceID;
	short			usProtocolID;
	int				dSampleSize;
	unsigned int	uiServerIP;
	char			szSvrIP[MAX_IP_LEN];		
	unsigned short	usServerPort;
	unsigned char	ucAccString[MAX_ACCS_STR_LEN];
	unsigned char	ucInfoString[MAX_ACCS_STR_LEN];
	unsigned int	uiClientIP;
	char			szCliIP[MAX_IP_LEN];			
	unsigned short	usClientPort;
	char			cInitSide;
	unsigned int	uiTime;
	unsigned int	uiMilDura;
	char			cTimeFrame;
	unsigned int	uiUpVol;
	unsigned int	uiDnVol;
	unsigned short	usSubCntId;
	unsigned short	usGblCntId;
	unsigned short	usPkgCntId;
	unsigned char	ucIpProto;
	unsigned int	uiProtoSig;
	unsigned int	uiZoneID;
	unsigned int	uiFlvID;
	unsigned char	ucClose;
} RDR_TR;

typedef union _PARA_VAL_
{
	unsigned char 	uchar_val;
	char 			char_val;
	short			short_val;
	unsigned short	ushort_val;
	int				int_val;
	unsigned int	uint_val;
	unsigned char	ucVal[128];
} PARA_VAL;


#endif
