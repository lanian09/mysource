#ifndef _RADIUS_FUNC_H_
#define _RADIUS_FUNC_H_

/**
 * Include headers
 */
#include "typedef.h"
#include "define.h"
#include "capdef.h"
#include "Analyze_Ext_Abs.h"
#include "common_stg.h"


/**
 * Define constants
 */

#define MAX_SUBNET_SIZE			 	37
#define MAX_AUTH_SIZE				16
#define MAX_BSID_SIZE				12
#define MAX_PORT_SIZE				24

#define MAX_USERPWD_SIZE			17
#define MAX_CHAPPWD_SIZE			18
#define MAX_CNTS_SIZE				13
#define MAX_APPID_SIZE				11

#define RADIUS_ACCESS_REQUEST		1
#define RADIUS_ACCESS_ACCEPT 		2
#define RADIUS_ACCESS_REJECT 		3
#define RADIUS_ACCOUNT_REQUEST 		4
#define RADIUS_ACCOUNT_RESPONSE		5

#define	RADIUS_USER_SAMEKEY			0
#define	RADIUS_USER_RETRANS			1	
#define	RADIUS_USER_FAILED			2	
#define	RADIUS_USER_UNKNOWN			3	
#define RADIUS_USER_TIMEOUT			4

#define DEF_ACC_SUCCESS				0
#define DEF_ACCESS_REJECT			1
#define DEF_ACCESS_TIMEOUT			2
#define DEF_ACCOUNT_TIMEOUT			1

#define RADIUS_TRANS_CNT			20011

/**
 * Define structures
 */
typedef struct _st_IPPOOLInfo_
{
	unsigned int	uiFIP;
	unsigned short	usNetMask;
} st_IPPOOLInfo, *pst_IPPOOLInfo;

typedef struct _st_ippool_
{	
	unsigned int	uiCount;
#ifdef _IPPOOL_TYPE_
	unsigned int	dFlag;
#endif
	st_IPPOOLInfo	stIPPool[100];
} st_ippool, *pst_ippool;

/* STRUCTURE FOR ACCESS-REQUEST/ACCEPT/REJECT */
typedef struct _st_AccessInfo_
{
	UCHAR		ucUserPasswdF;
	UCHAR		ucCHAPPassedF;
	UCHAR		ucFramedProtoF;
	UCHAR		ucFramedRoutF;
	UCHAR		ucFramedMTUF;
	UCHAR		ucFramedCompF;
	UCHAR		ucSessTimeoutF;
	UCHAR		ucIdleTimeoutF;

	UINT		uiFramedProto;
	UINT		uiFramedRout;
	UINT		uiFramedMTU;
	UINT		uiFramedComp;
	UINT		uiSessTimeout;
	UINT		uiIdleTimeout;
	UCHAR		szUserPasswd[MAX_USERPWD_SIZE];
	UCHAR		szCHAPPassed[MAX_CHAPPWD_SIZE];
	UCHAR		szReserved[5];
} st_AccessInfo, *pst_AccessInfo;

typedef struct _st_ACCInfo
{
	UCHAR	ucCode;					 /* RADIUS-CODE								*/
	UCHAR	ucID;						/* RADIUS IDENTIFIER						 */ 
	UCHAR	ucUserLen;					/* USER-NAME LENGTH							*/
	UCHAR	ucUDRSeqF;					/* UDR-Sequence					 (26/201) */	
	UCHAR	ucTimeStampF;				
	UCHAR	ucCallStatIDF;				/* Call-Stat-ID						 (31) */
	UCHAR	ucESNF;					 /* ESN								(26/52) */
	UCHAR	ucFramedIPF;				/* Framed-IP-Address					 (8) */

	UCHAR	ucAcctSessIDF;				/* Accounting Session ID				(44) */
	UCHAR	ucCorrelationIDF;			/* Correlation ID					(26/44) */
	UCHAR	ucSessContinueF;			/* Session Continue					(26/48) */
	UCHAR	ucUserNameF;				/* USER-NAME(NAI)						(1) */
	UCHAR	ucSvcOptF;					/* SVC-OPTION						(26/16) */ 
	UCHAR	ucUserIDF;					/* 3GPP2-User-ID (User-Zone)		 (26/11) */
	UCHAR	ucNASIPF;					/* PDSN-IP-ADDRESS						(4) */
	UCHAR	ucHAIPF;					/* HOME-AGENT						 (26/7) */

	UCHAR	ucPCFIPF;					/* PCF IP							 (26/9) */
	UCHAR	ucBSIDF;					/* 3GPP2-BSID						(26/10) */
	UCHAR	ucFwdFCHMuxF;				/* 3GPP2-Foward-FCH-Mux-Option		(26/12) */
	UCHAR	ucRevFCHMuxF;				/* 3GPP2-Reverse-FCH-Mux-Option		(26/13) */
	UCHAR	ucFwdTrafTypeF;			 /* 3GPP2-Forward-Traffic-Type		(26/17) */
	UCHAR	ucRevTrafTypeF;			 /* 3GPP2-Reverse-Traffic-Type		(26/18) */
	UCHAR	ucFCHSizeF;				 /* 3GPP2-FCH-Frame-Size				(26/19) */
	UCHAR	ucFwdFCHRCF;				/* 3GPP2-Forward-FCH-RC				(26/20) */

	UCHAR	ucRevFCHRCF;				/* 3GPP2-Reverse-FCH-RC				(26/21) */
	UCHAR	ucIPTechF;					/* IP Technology Flag				(26/22) */
	UCHAR	ucDCCHSizeF;				/* 3GPP2-DCCH-Frame-Size:0/5/20ms	(26/50) */
	UCHAR	ucReleaseIndF;				/* RELEASE-INDICATER				 (26/24) */	
	UCHAR	ucNASPortF;				 /* NAS-Port								(5) */
	UCHAR	ucNASPortTypeF;			 /* NAS-Port-Type						(61) */
	UCHAR	ucNASPortIDF;				/* NAS-Port-ID							(87) */	
	UCHAR	ucNASPortIDLen;			 /* NAS_Port-ID LEN							*/ 

	UCHAR	ucSvcTypeF;				 /* Service-Type							(6) */
	UCHAR	ucAcctStatTypeF;			/* ACCT-STATUS-TYPE					 (40) */
	UCHAR	ucNumActF;					/* 3GPP2-Number-Active-Transitions	(26/30) */	
	UCHAR	ucAcctInOctF;				/* Acct-Input-Octets 					(42) */
	UCHAR	ucAcctOutOctF;				/* Acct-Ouput-Octets						(43) */
	UCHAR	ucAlwaysOnF;				/* Always-On						 (26/78) */
	UCHAR	ucAcctInPktF;				/* Acct-Input-Packets 					(47) */
	UCHAR	ucAcctOutPktF;				/* Acct-Output-Packets 					(48) */

	UCHAR	ucBadPPPFrameCntF;			/* 3GPP2-Bad-PPP-Frame-Count 		 (26/25) */ 
	UCHAR	ucEventTimeF;				/* Event Time							(55) */
	UCHAR	ucActTimeF;				 /* 3GPP2-Active-Time				 (26/49) */
	UCHAR	ucTermSDBOctCntF;			/* 3GPP2-Terminating-SDB-Octet-Count (26/31) */
	UCHAR	ucOrgSDBOctCntF;			/* 3GPP2-Originating-SDB-Octet-Count (26/32) */
	UCHAR	ucTermNumSDBF;				/* 3GPP2-Terminating-Number-SDBs	 (26/33) */
	UCHAR	ucOrgNumSDBF;				/* 3GPP2-Originating-Number-SDBs	 (26/34) */
	UCHAR	ucRcvHDLCOctF;				/* 3GPP2-Received-HDLC-Octets		(26/43) */

	UCHAR	ucIPQoSF;					/* IP QoS							(26/36) */
	UCHAR	ucAcctSessTimeF;			/* Acct-Session-Time					(46) */	
	UCHAR	ucCompTunnelIndF;			/* 3GPP2-Compulsory-Tunnel-Indicator (26/23) */
	UCHAR	ucAcctAuthF;				/* Acct-Authentic						(45) */
	UCHAR	ucAcctTermCauseF;			/* Acct-Terminate-Cause				 (49) */
	UCHAR	ucAcctDelayTimeF;			/* Acct-Delay-Time						(41) */
	UCHAR	ucAirQoSF;					/* 3GPP2-Airlink-Priority			(26/39) */
	UCHAR	ucRPConnectIDF;			 /* 3GPP2-RP-Connect-ID				(26/41) */

	UCHAR	ucInMIPSigCntF;			 /* 3GPP2_Mobile_IP_Signaling_Inbound_Count	(26/46) */
	UCHAR	ucOutMIPSigCntF;			/* 3GPP2_Mobile_IP_Signaling_Outbound_Count (26/47) */
	UCHAR	ucMDNF;					 /* MDN(MSISDN)						(26/202) */
	UCHAR	ucAAAIPF;					/* AAAIP									 */
	UCHAR	ucRetryF;					/* Retry-Flag						(26/250) */
	UCHAR	ucAcctInterimF;				/* Acct-Interim-Interval					 */
	UCHAR	ucBeginningSessF;			/* Beginning-Session:True[1]Faulse[0](26/51) */ 
	UCHAR	szReservd[1];				/* Use for Acct-Res: FAIL_CODE				*/

	UCHAR	ucC23BITF;
	UCHAR	ucHBITF;					/* 080220, poopee, HBIT */
	UCHAR	ucSubnetF;					/* 071203, poopee, SUBNET */
	UCHAR	ucPBITF;					/* PBIT flag */
	UCHAR	ucCBITF;					/* CBIT flag */
	UCHAR 	ucStopFlag; 				/* Stop flag */
	char	reserved1[2];
	
	/**** VALUE ****/
	UINT	uiUDRSeq;							/* UDR-Sequence					 */
	UINT	uiTimeStamp;						
	UINT	uiAAAIP;							/* AAAIP address					*/
	UINT	uiKey;								/* UDRGEN <-> AAAIF DUPLICATION Key */

	UINT	uiFramedIP;				 /* Framed-IP-Address					 (8) */
	UINT	uiNASIP;					/* PDSN-IP-ADDRESS						(4) */ 
	UINT	uiPCFIP;					/* PCF IP							 (26/9) */ 
	UINT	uiHAIP;					 /* HOME-AGENT						 (26/7) */ 

	UINT	uiRADIUSLen;				/* RADIUS PACKET LEN						*/
	UINT	uiSessContinue;			 /* SESSION-CONTINUE						 */
	UINT	uiBeginnigSess;			 /* BEGINNING-SESSION				(26/51) */
	INT	 dSvcOpt;					/* Service-Option							*/

	INT	 dAcctStatType;				/* Acct-Status-Type						 */
	INT	 dCompTunnelInd;			 /* 3GPP2-Compulsory-Tunnel-Indicator		*/
	INT	 dNumAct;					/* 3GPP2-Number-Active						*/
	INT	 dSvcType;					/* Service-Type							 */	

	INT	 dFwdFCHMux;				 /* 3GPP2-Foward-FCH-Mux-Option		(26/12) */
	INT	 dRevFCHMux;				 /* 3GPP2-Reverse-FCH-Mux-Option	 (26/13) */
	INT	 dFwdTrafType;				/* 3GPP2-Forward-Traffic-Type				*/
	INT	 dRevTrafType;				/* 3GPP2-Reverse-Traffic-Type				*/

	INT	 dFCHSize;					/* 3GPP2-FCH-Frame-Size					 */
	INT	 dFwdFCHRC;					/* 3GPP2-Forward-FCH-RC					 */
	INT	 dRevFCHRC;					/* 3GPP2-Reverse-FCH-RC					 */
	INT	 dIPTech;					/* IP Technology							*/

	INT	 dDCCHSize;					/* 3GPP2-DCCH-Frame-Size					*/
	INT	 dNASPort;					/* NAS-Port(PDSN IP Addr)					*/
	INT	 dNASPortType;				/* NAS-Port-Type (18=wireless other,22=1x, 24=EVDO) */
	INT	 dReleaseInd;				/* Release Indicator						*/

	INT	 dAcctInOct;				 /* Acct-Input-Octets						*/
	INT	 dAcctOutOct;				/* Acct-Output-Octets						*/
	INT	 dAcctInPkt;				 /* Acct-Input-Packets						*/
	INT	 dAcctOutPkt;				/* Acct-Output-Packets						*/

	UINT	uiEventTime;				/* Event Time								*/
	UINT	uiActTime;					/* 3GPP2-Active-Time						*/
	UINT	uiAcctSessTime;			 /* Accounting Sesssion Time				 */
	UINT	uiAcctDelayTime;			/* Acct-Delay-Time							*/

	INT	 dTermSDBOctCnt;			 /* 3GPP2-Terminating-SDB-Octet-Count		*/
	INT	 dOrgSDBOctCnt;				/* 3GPP2-Originating-SDB-Octet-Count		*/
	INT	 dTermNumSDB;				/* 3GPP2-Terminating-Number-SDBs			*/
	INT	 dOrgNumSDB;				 /* 3GPP2-Originating-Number-SDBs			*/

	INT	 dRcvHDLCOct;				/* 3GPP2-Received-HDLC-Octets				*/
	INT	 dIPQoS;					 /* IP QoS									*/
	INT	 dAirQoS;					/* Air QoS									*/
	INT	 dRPConnectID;				/* 3GPP2-RP-Connect-ID (26/41,integer)		*/

	INT	 dBadPPPFrameCnt;			/* 3GPP2-Bad-PPP-Frame-Count				*/
	INT	 dAcctAuth;					/* Acct-Authentic (0=RADIUS, 1=LOCAL)		*/
	INT	 dAcctTermCause;			 /* Acct-Terminate-Cause					 */
	INT	 dAlwaysOn;					/* Always-On								*/

	INT	 dUserID;					/* 3GPP2-User-ID							*/
	INT	 dInMIPSigCnt;				/* 3GPP2_Mobile_IP_Signaling_Inbound_Count	*/
	INT	 dOutMIPSigCnt;				/* 3GPP2_Mobile_IP_Signaling_Inbound_Count	*/ 
	INT	 dAcctInterim;				/* Acct-Interim-Interval */

	INT64	llAcctSessID;				/* Accounting Session ID			*/
	INT64	llCorrelID;				 /* Correlation ID					*/
	UINT	uiRetryFlg;				 
	INT	 dReserved;					/* Use for Acct-Res: FAIL_CODE		*/

	INT		uiC23BIT;					/*1:Network Mode 0:relay mode */
	INT		uiHBIT;						/* 080220, poopee, HBIT */	

	INT		dPBIT;						/* PBIT value */
	INT		dCBIT;						/* CBIT value */

	UCHAR	szSubnet	[MAX_SUBNET_SIZE+1];		/* 071203, poopee, SUBNET */
	UCHAR	szReserved3	 [2];	

	UCHAR	szAuthen	 [MAX_AUTH_SIZE];		/* RADIUS Authenticator : 16		*/
	UCHAR	szMDN		[MAX_MIN_SIZE];		/* MDN : 15+1						*/
	UCHAR	szESN		[MAX_ESN_SIZE];		/* ESN : 15+1						*/ 
	UCHAR	szUserName	[MAX_USERNAME_SIZE];	/* User Name : 72					*/
	UCHAR	szBSID		[MAX_BSID_SIZE];		/* 3GPP2-BSID : 12					*/
	UCHAR	szNASPortID	[MAX_PORT_SIZE];		
	UCHAR	szMIN		[MAX_MIN_SIZE];		/* Calling-Station-ID : 15+1		*/

	st_AccessInfo	stAccessInfo;

} st_ACCInfo, *pst_ACCInfo;
#define DEF_ACCINFO_SIZE		sizeof(st_ACCInfo)

/**< RADIUS Msg Transaction Key **/
typedef struct _st_radius_hashkey {
	UINT 		uiSrcIP;
	UINT 		uiDestIP;
	UCHAR 		ucID;
	UCHAR 		ucMsgType;
	CHAR 		szReserved[6];
} HKey_Trans;
#define DEF_HKEY_TRANS_SIZE	sizeof (HKey_Trans)

typedef struct _st_radius_hashdata {
	U64				 ulTimerNID;
	OFFSET 				dOffset;

	int					dReqFlag;
	UINT				uiCmdCode;
	UINT				uiLastEEID;

	UCHAR				szIMSI[MAX_MIN_SIZE];
	UCHAR				szTraceMIN[MAX_MIN_SIZE];

	STIME				CallTime;
	MTIME				CallMTime;
	UINT				uiLastUserErrCode;
	UINT				uiRetransReqCnt;
	UINT				uiReqDataSize;
	UINT				uiResDataSize;
} HData_Trans;																	
#define DEF_HDATA_TRANS_SIZE	sizeof (HData_Trans)

typedef struct _st_timer_trans_data {
	HKey_Trans			key_trans;
} TData_Trans;
#define DEF_TDATA_TRANS_TIMER_SIZE	sizeof (TData_Trans)

/**
 * Declare functions
 */
extern S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime);
extern char *szPrintType(U32 type);
extern char *szPrintCode(U8 type);
extern U8 ucGetMsgType(U8 code);
extern int dump(char *s,int len);
extern S32 dGetCALLProcID(U32 uiClientIP);
extern int dSendTransLog(Capture_Header_Msg *pCAPHEAD, HKey_Trans *pstTransKey, HData_Trans *pstTransData);
extern int cb_timeout_transaction(HKey_Trans *pstTransKey);
extern int pSendStartCall(LOG_SIGNAL *pstTransLog);
extern HData_Trans *pCreateTransaction(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, HKey_Trans *pstTransKey);
extern int dCheck_TraceInfo( HData_Trans *pstSessData, UCHAR *pData, Capture_Header_Msg *pstCAP );
#ifdef _IPPOOL_TYPE_
extern int dCheck_IPPool(unsigned int uiIP);
#else
extern int dCheck_IPPool( unsigned int uiIP );
#endif
extern int dProcRADIUS_Trans( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, st_ACCInfo *pstACCINFO);

#endif	/* _RADIUS_FUNC_H_ */

