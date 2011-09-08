#ifndef __RPPI_DEF_H__
#define __RPPI_DEF_H__

#include "define.h"
#include "typedef.h"

/*  PROTOCOL TYPE  */
#define A11_PROTO			120
#define RADIUS_PROTO		121
#define DIAMETER_PROTO		122
#define PPP_PROTO			123
#define UPLCP_PROTO			31
#define DNLCP_PROTO			32
#define UPIPCP_PROTO		33
#define DNIPCP_PROTO		34
#define CHAP_PROTO			24
#define PAP_PROTO			25
#define OTHERPPP_PROTO		20

/*  MSG TYPE  */
#define A11_REGIREQ_MSG			0x01
#define A11_REGIUPDATE_MSG		0x14
#define A11_SESSUPDATE_MSG		0x16
#define RADIUS_ACCESS_MSG		0x1
#define RADIUS_ACCOUNT_MSG		0x4
#define	PPP_SETUP				0x1
#define PPP_TERM				0x5
#define LCP_ECHO				0x9
#define DIAMETER_UAR			300
#define DIAMETER_SAR			301
#define DIAMETER_LIR			302
#define DIAMETER_MAR			303
#define DIAMETER_RTR			304
#define DIAMETER_PPR			305


/* MSG REPCODE */
#define A11_REGI_REP		0x03
#define A11_UPDATE_ACK		0x15
#define A11_SESS_UPDATE_ACK	0x17

/* ACCOUNT TYPE */
#define ACCOUNTING_START	1
#define ACCOUNTING_STOP		2
#define ACCOUNTING_INTERIM	3

/* CALL TYPE */
#define INIT_CALLSTART			1
#define	REACT_CALLSTART			2
#define REACT_CALLSTART_PPP		3
#define REACT_CALLSTART_ECHO	4
#define INIT_ROAM_CALLSTART		5
#define INIT_ROAM_CALLSTART_LNS	6
#define INIT_RECALL_CALLSTART	7

/* AirLink */
#define CONNSETUP				0x01
#define	ACTIVE_START			0x02
#define CONNSETUP_ACTIVE_START	0x03
#define ACTIVE_STOP				0x04
#define ACTIVE_START_STOP		0x06

/* CALL STATE */
#define CALL_INIT_STATE		0
#define	A11_REGI_STATE		1
#define LCP_SETUP_STATE		2
#define	AUTH_SETUP_STATE	3
#define	IPCP_SETUP_STATE	4
#define	ON_SERVICE_STATE	5
#define	LCP_ECHO_STATE		6
#define	PPP_TERM_STATE		7
#define	RP_END_STATE		8
#define TIMEOUT_STATE		9

/* CALL STATE : RECALL */
#define RECALL_CALL_INIT_STATE		0
#define RECALL_PI_DATA_SETUP_STATE	1
#define RECALL_RP_DATA_SETUP_STATE	2
#define RECALL_RP_SIG_SETUP_STATE	3
#define RECALL_PI_SIG_SETUP_STATE	4
#define RECALL_ON_SERVICE_STATE		5
#define RECALL_RP_END_STATE			6
#define RECALL_PI_END_STATE			7 
#define RECALL_TIMEOUT_STATE		8	

/* CALL STATE : ROAMING */
#define ROAM_CALL_INIT_STATE	0
#define ROAM_ACCESS_STATE		1
#define ROAM_ACCOUNT_STATE		2
#define ROAM_L2TP_SETUP_STATE	3
#define ROAM_LNS_ACCESS_STATE	4
#define ROAM_AUTH_SETUP_STATE	5
#define ROAM_IPCP_SETUP_STATE	6
#define ROAM_LNS_ACCOUNT_STATE	7
#define ROAM_ON_SERVICE_STATE	8
#define ROAM_PPP_TERM_STATE		9
#define ROAM_RP_END_STATE		10
#define ROAM_TIMEOUT_STATE		11

/* SetupFailCode */
#define SETUP_SUCESS			0
#define ERR_A11_CALL_CUT		901
#define ERR_LCP_CALL_CUT		910		/* LCP 완료 후 종료 */
#define ERR_LCP_911				911		/* 단말 LCP 진행 단계, PDSN LCP 진행 없이 종료 */
#define ERR_LCP_912				912		/* 단말 LCP 완료, PDSN LCP 진행 단계에서 종료 */
#define ERR_LCP_913				913		/* 단말 LCP 진행, PDSN LCP 진행 단계에서 종료 */
#define ERR_LCP_914				914		/* PDSN LCP 진행 단계, 단말 LCP 진행 없이 종료 */
#define ERR_LCP_915				915		/* PDSN LCP 완료, 단말 LCP 진행 상태에서 종료 */
#define ERR_LCP_916				916		/* 단말 LCP 완료, PDSN LCP 완료에서 단말 LCP 재요청 후 종료 */
#define ERR_LCP_917				917		/* 단말 LCP 완료, PDSN LCP 완료에서 PDSN LCP 재요청 후 종료 */
#define ERR_LCP_918				918		/* PPP 없이 LCP ECHO 후 종료 */
#define ERR_CHAP_CALL_CUT		920		/* CHAP 완료 후 종료 */
#define ERR_CHAP_921			921		/* PDSN CHAP Challenge 전송 후, 단말 LCP 상태로 호 종료 */
#define ERR_CHAP_922			922		/* PDSN CHAP Challenge 전송 후, 단말 CHAP Response 후 종료 */
#define ERR_CHAP_923			923		/* PDSN CHAP Failure로 호 종료 */
#define ERR_CHAP_924			924		/* PDSN CHAP Success 후 호 종료 */
#define ERR_IPCP_CALL_CUT		940		/* IPCP 완료 후 종료 */
#define ERR_IPCP_941			941		/* PDSN IPCP진행, 단말 CHAP상태에서 호 종료 */
#define ERR_IPCP_942			942		/* 단말 IPCP 진행 단계, PDSN CHAP 상태에서 호 종료 */
#define ERR_IPCP_943			943		/* 단말 IPCP 진행, PDSN IPCP 진행 상태에서 호 종료 */
#define ERR_IPCP_944			944		/* 단말 IPCP 완료, PDSN IPCP 진행 상태에서 호 종료 */
#define ERR_IPCP_945			945		/* PDSN IPCP완료, 단말 IPCP 진행 상태에서 호 종료 */
#define ERR_IPCP_946			946		/* PDSN IPCP완료, 단말 IPCP 완료 후, 단말 IPCP 재요청 후 호 종료 */
#define ERR_IPCP_947			947		/* 단말 IPCP완료, PDSN IPCP 완료 후, PDSN IPCP 재요청 후 호 종료 */
#define ERR_CALL_DUPLICATE		960 	// 1Call에서 Before, after가 있는데 Call Start가 다시 와서 정리 될때
#define ERR_CALL_TIMEOUT		980
#define ERR_RADIUS_CALL_CUT		9001
#define ERR_LNS_RADIUS_CALL_CUT	9002
#define ERR_L2TP_1				1		/* Incoming-Call-Request 이후에 호 종료 */
#define ERR_L2TP_2				2		/* Incoming-Call-Reply(ICRP) 이후에 호 종료 */
#define ERR_L2TP_3				3		/* Incoming-Call-Connected(ICCN) 이후에 호 종료 */ 
#define ERR_RECALL_1 			1 		/* TCP DATA 발생 후에 GRE DATA 가 존재하지 않는 경우 */
#define ERR_RECALL_2 			2 		/* GRE DATA 발생 후에 A11-REGI 가 발생하지 않는 경우 */
#define ERR_RECALL_3 			3 		/* A11-REGI 후에 ACCOUNTING 이 발생하지 않는 경우 */
#define ERR_RECALL_4 			4 		/* ACCOUNTING 후에 서비스 사용 없이 종료되는 경우 */

/* DEFECT CODE */
#define	A11_DEFECT				10000000
#define AAA_DEFECT				20000000
#define DIAMETER_DEFECT			30000000
#define L2TP_DEFECT				40000000
#define RECALL_DEFECT			50000000 		/* 착신 호 */
#define SERVICE_DEFECT			90000000

#define A11_REGI_REPLY			100000
#define A11_REGI_UPDATE_INIT	200000
#define A11_REGI_UPDATE_REACT	210000
#define A11_REGI_UPDATE_ACK		300000
#define	A11_SESS_UPDATE			400000 
#define RECALL_SETUP			970000
#define CALL_SETUP				980000
#define CALL_PPP_TERM			990000
#define SERVICE_TCP_DEFECT		100000
#define SERVICE_HTTP_DEFECT		200000
#define SERVICE_MSRP_DEFECT		300000
#define SERVICE_PAGE_DEFECT		400000
#define SERVICE_VOD_DEFECT		500000
#define SERVICE_SIP_DEFECT		600000
#define	SERVICE_DELAY_DEFECT	700000
#define SERVICE_IM_DEFECT		800000
#define SERVICE_VT_DEFECT		900000

#define AAA_ACCESS_DEFECT		100000
#define AAA_ACCOUNTING_DEFECT	200000
#define DIAMETER_CMD_DEFECT		100000
#define L2TP_CCN_DEFECT			100000
#define L2TP_CDN_DEFECT			200000

#define RESPONSETIME				9000
#define THROUGHPUT					9001
#define	UPRETRANSCOUNT				9002
#define DNRETRANSCOUNT				9003
#define JITTER						9004
#define PACKETLOSS					9005

#define HASH_PCFINFO_CNT			8009
#define HASH_DEFECTINFO_CNT			101
#define HASH_NASIP_CNT				200003


/* DEFECT SERVICE TYPE */
typedef enum  {
    SERVICE_A11 = 1,
    SERVICE_RADIUS,
    SERVICE_DIAMETER,
    SERVICE_TCP,
    SERVICE_MENU_FB,
    SERVICE_MENU_PCIV,
    SERVICE_MENU_WAP20,
    SERVICE_MENU_WIPI,
    SERVICE_DOWNLOAD_2G,
    SERVICE_DOWNLOAD_VOD,
    SERVICE_DOWNLOAD_WIPI,
    SERVICE_DOWNLOAD_JAVA,
	SERVICE_DOWNLOAD_OMA_DN,
	SERVICE_DOWNLOAD_OMA_2G,
	SERVICE_DOWNLOAD_OMA_VOD,
	SERVICE_DOWNLOAD_OMA_WIPI,
    SERVICE_STREAMING_RTS,
    SERVICE_STREAMING_SVOD,
    SERVICE_STREAMING_MBOX,
	SERVICE_MMS,
	SERVICE_TODAY,
	SERVICE_WIDGET,
	SERVICE_PHONE,
	SERVICE_EMS,
	SERVICE_FV,
    SERVICE_IM,
    SERVICE_VT
}SERVICE_TYPE;

/* DEFECT ALARM TYPE */
typedef enum {
    ALARM_TCPSETUPTIME = 1,
    ALARM_RESPONSETIME,
    ALARM_UPTHROUGHPUT,
    ALARM_DNTHROUGHPUT,
    ALARM_UPRETRANSCNT,
    ALARM_DNRETRANSCNT,
    ALARM_UPJITTER,
    ALARM_DNJITTER,
    ALARM_UPPACKETLOSS,
    ALARM_DNPACKETLOSS
}ALARM_TYPE;



typedef struct _st_CallKey {
	U8		szIMSI[MAX_MIN_SIZE];
}st_CallKey, *pst_CallKey;

#define RPPISESS_KEY		st_CallKey
#define PRPPISESS_KEY		pst_CallKey

#define RPPISESS_KEY_SIZE	sizeof(RPPISESS_KEY)

typedef struct _st_Rppi_Timer {
    RPPISESS_KEY    RPPIKEY;
	STIME			uiCallTime;
	MTIME			uiCallMTime;
} RPPI_TIMER;

typedef struct _st_Hash_data
{
	U64				timerNID;
	OFFSET			dOffset;
	STIME			uiCallTime;
	MTIME			uiCallMTime;
	U32				uiFirstServFlag;
	U32				uiTimerStop;
	U64				ulCorrelationID;
}HData;

typedef struct _st_RPPI_hashdata {
	HData	before;
	HData	after;
} HData_RPPI;

#define HDATA_RPPI_SIZE sizeof(HData_RPPI)
#define MAX_RPPISESS_CNT	250007


typedef struct _st_PCF_hashdata {
	U8 ucBranchID;
	U8 ucPCFType;
} HData_PCF;

#define HDATA_PCF_SIZE sizeof(HData_PCF)

typedef struct _st_DEFECT_hashdata {
    U32                 uiTCPSetupTime;
    U32                 uiResponseTime;
    U32                 uiUpThroughput;
    U32                 uiDnThroughput;
    U32                 uiUpRetransCnt;
    U32                 uiDnRetransCnt;
    U32                 UpJitter;
    U32                 DnJitter;
    U32                 UpPacketLoss;
    U32                 DnPacketLoss;
} HData_DEFECT;

#define HDATA_DEFECT_SIZE sizeof(HData_DEFECT)

typedef struct _st_Model_hasdata {
	U8				szModel[MAX_MODEL_SIZE];
	U8				szMIN[MAX_MIN_SIZE];
} HData_Model;

#define HDATA_MODEL_SIZE sizeof(HData_Model)

typedef struct _st_NASIP_hashdata {
	U8				ucNationID;
	U8				szReserved[7];
} HData_NASIP;
#define HDATA_NASIP_SIZE	sizeof(HData_NASIP)

#define DEF_IRM_PATTERN_LEN		4
#define DEF_IRM_PATTERN_SIZE	(DEF_IRM_PATTERN_LEN + 1)
#define DEF_IRM_PREFIX_LEN		5
#define DEF_IRM_PREFIX_SIZE		(DEF_IRM_PREFIX_LEN + 1)
#define DEF_IRM_MIN_LEN			10

typedef struct _st_IRMHash_Key {
	U8				szIRM[DEF_IRM_PATTERN_SIZE];
	U8				szReserved[3];
} st_IRMHash_Key, *pst_IRMHash_Key;
#define DEF_IRMHASH_KEY_SIZE		sizeof(st_IRMHash_Key)

typedef struct _st_IRMHash_Data {
	U8				szIMSI[DEF_IRM_PATTERN_SIZE];
	U8				szPrefix[DEF_IRM_PREFIX_SIZE];
	U8				szReserved[5];
} st_IRMHash_Data, *pst_IRMHash_Data;
#define DEF_IRMHASH_DATA_SIZE		sizeof(st_IRMHash_Data)

#define DEF_IRMHASH_CNT			1009

#define STG_DeltaTIME64(ENDT,ENDM,STARTT,STARTM,RESULT)  {                                               \
            *(RESULT) = (((S64)ENDT * 1000000 + (S64)ENDM) - ((S64)STARTT * 1000000 + (S64)STARTM));    \
}

#define ALLOW_PITIME	1000000
#define RPPI_GAP_TIME	2

#define MAX_PDSNIF_HASH_COUNT		30031
typedef struct _st_PDSNIF_KEY_
{
	UINT    uiIP;
} st_PDSNIF_KEY, *pst_PDSNIF_KEY;
#define DEF_PDSNIF_KEY_SIZE			sizeof(st_PDSNIF_KEY)

typedef struct _st_PDSNIF_DATA_
{
	UINT    uiIP;
	UINT    uiPDSNID;
} st_PDSNIF_DATA, *pst_PDSNIF_DATA;
#define DEF_PDSNIF_DATA_SIZE		sizeof(st_PDSNIF_DATA)






#endif
