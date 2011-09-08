
/*********************************************************
                 ABLEX IPAS Project (IPAM BLOCK)

   Author   : LEE SANG HO
   Section  : IPAS(IPAM) Project
   SCCS ID  : @(#)ipam_svc.h	1.4
   Date     : 1/22/03
   Revision History :
        '03.    01. 15. initial

   Description:

   Copyright (c) ABLEX 2003
*********************************************************/

#ifndef _IPAF_SVC_________________H
#define _IPAF_SVC_________________H

#include <time.h>
#include <sys/time.h>
#include <define.h>
#include "comm_typedef.h"
#include "comm_msgtypes.h"

#define FATAL_ERROR     5
#define MAJOR_ERROR     4
#define MINOR_ERROR     3
#define WARNING         2
#define NOTIFICATION    1

/* IS-95A/IS-95B */
#define DEF_IS95A		15
#define DEF_IS95B		25

/* Definition of New Constants */
// user type
#define POST_PAID           1
#define PRE_PAID            2
/* 지능망 선불형 가입자 처리: ADD BY HWH 2004/05/03 */
#define NO_REMAIN   		4

// cdr type
#define CDR_UNKNOWN         0
#define CDR_END             1
#define CDR_INT             3

// return code
#define OK                  0
#define DECTED              1
#define NOT_OK              2
#define DUP                 3

// billing type
#define TIME                1
#define BYTE                2
#define TIME_PLUS_BYTE      3

// cdr end reason
#define NORMAL_END          1
#define ABNORMAL_END        2

/* Message Code Definition */
#define DEF_SVC			1000
#define	DEF_SYS			5000

#define MAX_EXTRA_SIZE		95	
#define MAX_MSGBODY_SIZE	4000		/* 3584 */
#define DEF_CONTENT_SIZE	53
#define DEF_SER_ALIAS_SIZE  24+6

#define BILLCOM_HEAD_SIZE	96
#define MACS_HEAD_SIZE		48
#define WICGS_HEAD_SIZE		56
#define WICGS_BILLCOM_SIZE	108

/* FIRST PACKET RDR SEND FLAG */
#define DEF_NOTFIRST_PACK   0
#define DEF_FIRST_PACK      1

#define DEF_PKTCNT_SIZE		100

/* Cond Page count setting, MsgQ Ext */
typedef struct _st_CondCount
{
        USHORT          usTotPage;
        USHORT          usCurPage;
        USHORT          usSerial;
        USHORT          usReserved;
}st_CondCount, *pst_CondCount;


/* Messsage Queue Definition */
typedef struct _st_MsgQ
{
    long int	llMType;					/* Message Type */
	int			uiReserved;
	INT64		llNID;						/* Message Unique ID */
	UCHAR		szIPAFID[MAX_IPAF_PAIR];	/* IPAF PAIR ID if ucNaType = 2, Used szIPAFID[1] */
	UCHAR		ucProID;					/* Process ID : Example = SEQ_PROC_SESSIF */
	char		szReserved[5];				/* Call Reference Number */
    INT64		llIndex;					/* Local DB Index Number */
	INT			dMsgQID;					/* Source Queue ID */
	UINT		uiNaIP;						/* IWF/PDSN IP ADDRESS  || RDR LAST TID*/
	UCHAR		ucNaType;					/* NA SYSTEM TYPE : 1=PDSN, 2=IWF */
	UCHAR		szReserved2;
	USHORT		usCatID;
	USHORT 		usBodyLen;					/* Receive Message Body Length */
	USHORT		usRetCode;					/* RetCode */
	UCHAR		szMIN[MAX_MIN_SIZE];		/* MIN Number */
	UCHAR		szExtra[MAX_EXTRA_SIZE];	/* Shared Information */	
    UCHAR		szBody[MAX_MSGBODY_SIZE]; 	/* Packet Message */
} st_MsgQ, *pst_MsgQ;

#define DEF_MSGQ_SIZE		sizeof(st_MsgQ)
#define DEF_MSGHEAD_LEN		(sizeof(st_MsgQ) - MAX_MSGBODY_SIZE)

/* SVC Message Queue llMType Sub Definition */
typedef struct _st_MsgQSub
{
    USHORT	usType;			/* 5000 : SYS, 1000 : SVC */
	UCHAR	usSvcID;		/* SERVICE ID */
	UCHAR	usMsgID;		/* MESSAGE ID */
} st_MsgQSub, *pst_MsgQSub;

/* SVC Message Queue llNID Sub Definition */
typedef struct _st_NID
{
    char       	ucReserved;
    UCHAR       ucSysType;		/* 1 : SESSIF, 2 : SESSSVC, 3 : IPAFUIF, 4 : DSCPIF, 5 : IPAFTIF, 6 :IPAFUIF */
    USHORT      usSerial;		/* Serial Number */
    time_t      stBuild;		/* Build Time */
} st_NID, *pst_NID;

typedef union _un_NID
{
	INT64	llNID;
	st_NID	stNID;
} un_NID, *pun_NID;

/* CRN Definition */
typedef	union _un_CRN
{ 
	INT64	llCRN;
	struct {
		INT64	ucReserved:8;	/* Reserved */
		INT64 	ucSide:1;		/* A Side : 0, B Side : 1, Always 0 */
		INT64	ucIPAFID:7;		/* IPAF ID */
		INT64	usSerial:16;	/* 10 ~ 60000 */
		INT64	dTime:32;		/* Currnet Year - 2000, Up to 127 Year */
	} stBody;
} un_CRN, *pun_CRN;

/* 서비스 블럭에서 쓰기 위한 EXTRA FIELD Structure */
/* 2003-03-24 add by hwh */
typedef struct _st_STOPMsgKey
{
    INT64           llAcctSessID;
    UINT            uiSrcIP;
    USHORT          usSrcPort;
	UCHAR			ucRDRSndType;	/* 2004/05/12 ADD BY HWH : 0->IPAM SEND, 1->LOG SAVE */
    UCHAR          	ucPayType;
    struct timeval  tSndTime;       /* SEND TIME SESSANA TO SVCBLOCK */
	UCHAR			szMDN[MAX_MIN_SIZE-1];
	USHORT			usCatID;
	UCHAR			ucURLChar;
	char			szReserved[5];
}st_STOPMsgKey, *pst_STOPMsgKey;



/* Service ID Defnition */
#define SID_ACC_S				11	/* Start Service */
#define SID_ACC_E				21  /* End Service */
#define SID_QUD					31	/* QuD Service */
#define SID_CDR					41	/* CDR Service */
#define SID_PPS					51  /* PPS Info Service */
#define SID_CALL				61	/* CALL Service */
#define SID_CLEAR				71	/* Sess Clear Service */

/* Session Clear MID */
#define MID_SESS_CLEAR          101
/* REQUEST MESSAGE ALLWAYS %10 == 1 */
#define MID_GEN_SREQ			11	
#define MID_GEN_SRES			12
#define MID_GEN_EREQ			21	
#define MID_GEN_ERES			22
#define MID_PPS_SREQ			31	
#define MID_PPS_SRES			32
#define MID_PPS_EREQ			41	
#define MID_PPS_ERES			42
#define MID_CDR_REQ				51
#define MID_CDR_RES				52
#define MID_PPS_REQ				61
#define MID_PPS_RES				62
#define MID_QUD_REQ				71
#define MID_QUD_RES				72

/* LGT BMT NEW SID & MID 2006.04.11 */
#define SID_ACC_REQ				91
#define MID_ACC_START			11
#define MID_ACC_RESPONSE		12
#define MID_ACC_END				13
#define MID_ACC_INTERIM			15
#define MID_ACC_REQ				17
#define MID_ACC_ACPT			18 
#define MID_ACC_REJ				19
/* 20060707(challa) add */
#define MID_ACC_UDR             21

#define MID_ACCT_HTTP           25
#define MID_ACCT_UAWAP          33
#define MID_ACCT_WAP1           27
#define MID_ACCT_WAP2           29

#define DEF_ACCESS_REQUEST		1
#define DEF_ACCESS_ACCEPT		2
#define DEF_ACCESS_REJECT		3
#define DEF_ACCOUNTING_REQ		4
#define DEF_ACCOUNTING_RES		5

/** BSDT1.0.0 20060630 add(chlla)  **/
#define SID_DUP                 101
#define MID_STB_REQ             101
#define MID_STB_RES             102
#define MID_ACT_REQ             111   /* 이중화 절체시 SDMD->AAAIF       */
#define MID_ACT_RES             112   /* UDRGEN->AAAIF 메시지큐 정상처리 */
/** add 2006.09.05 by chlla **/
#define MID_DUAL_ACT_STB_NOTI   113	  /* DUAL_ACTIVE */
#define MID_DUAL_ACT_ACT_NOTI   114   /* */
#define MID_DUP_STB_NOTI        115   /* DUAL-ACTIVE BSDA->STANDBY TRANSFORM */
#define MID_DUP_ACT_NOTI        116   /* DUAL-ACTIVE BSDA->ACTIVE TRANSFORM */

#define MID_SESS_UPDATE_ACC_REQ     120
#define MID_SESS_UPDATE_UDR_REQ     121
/* <--- */



/* SESSIF <-> SESSSVC Interface */
/*
	Accounting Request (Start) | SID_ACC_S | MID_GEN_SREQ & MID_GEN_SRES
	Accounting Request (PPS:Start) | SID_ACC_S | MID_PPS_SREQ & MID_PPS_SRES
	Accounting Request (Stop) | SID_ACC_E | MID_GEN_EREQ & MID_GEN_ERES
	Accounting Request (PPS:Stop) | SID_ACC_E | MID_PPS_EREQ & MID_PPS_ERES
	CDR Accounting Request | SID_CDR | MID_CDR_REQ & MID_CDR_RES
	QuD Request | SID_QUD | MID_QUD_REQ & MID_QUD_RES
*/

/* DSCP<->SESSSVC Interfice */
/*
	WinInfomationRequest SID : SID_ACC_S | MID : MID_PPS_REQ & MID_PPS_RES
	WinChargingRequest SID : SID_CDR | MID : MID_PPS_REQ & MID_PPS_RES
	WinStopRequest SID : SID_QUD | MID : MID_PPS_REQ & MID_PPS_RES
	CallStatus	SID : SID_CALL | MID : MID_PPS_REQ & MID_PPS_RES
*/

/* IPAFUIF <-> SESSSVC Interface */
/* 
	Accounting Request (Start) | SID_ACC_S | MID_GEN_SREQ & MID_GEN_SRES
	Accounting Request (End) | SID_ACC_E | MID_GEN_EREQ & MID_GEN_ERES
	PPS Request | SID_PPS | MID_PPS_REQ & MID_PPS_RES
	CDR Accounting Request | SID_CDR | MID_CDR_REQ & MID_CDR_RES
*/	

/* ACCOUNT START MSG DEF. */
typedef struct _st_AcctStartReq
{
    UCHAR       ucPayType;
    UCHAR       ucAmountType;
    UINT        uiPacketPeriod;
    UINT        uiTimePeriod;
}st_AcctStartReq, *pst_AcctStartReq;

/* ACCOUNT END MSG DEF. */
typedef struct _st_AcctEndReq
{
	UCHAR		ucReason;
}st_AcctEndReq, *pst_AcctEndReq;

/*************** DEFINITION CDR INFORMATION *************************/
typedef struct _st_CDRInfo
{
    time_t      dStartTime;         /* Start Packet Received Time */
    time_t      dLastTime;          /* Last Packet Received Time */
	INT64		llUTCPAmount;
	INT64		llDTCPAmount;
	INT64   	llUUDPAmount;
	INT64   	llDUDPAmount;
	INT64   	llUETCAmount;
	INT64     	llDETCAmount;
    INT64       llUAmount;          /* Upload Packet Byte Amount */
    INT64       llDAmount;          /* Download Packet Byte Amount */
    INT64       llRUpAmount;        /* Real Upload Packet Byte Amount */
    INT64       llRDownAmount;      /* Real Download Packet Byte Amount */
    UINT        uiUCount;           /* Upload Packet Count */
    UINT        uiDCount;           /* Down Packet Count */
    UINT        uiRUCount;          /* Upload Retransmission Packet Count */
    UINT        uiRDCount;          /* Down Restransmission Packet Count */
    USHORT      usSVCCategory;      /* Service Category ID */
	char		szReserved[6];		/* Reserved */
} st_CDRInfo, *pst_CDRInfo;
#define DEF_CDRINFO_LEN		sizeof(st_CDRInfo)

typedef struct _st_CDR
{
	UCHAR		ucPayType;
	UCHAR		ucReserved1;
	USHORT		usSerial;
	UINT		uiReserved1;

	UINT		uiUPacket;
	UINT		uiDPacket;

	UINT		dStartTime;
	UINT		dLastTime;

	UCHAR		ucCateCount;
	UCHAR		szReserved1[7];
	st_CDRInfo	stCDRInfo[MAX_CATESVC_COUNT];
} st_CDR, *pst_CDR;
#define DEF_CDR_LEN			32

typedef struct _st_Alive
{
	UINT		uiIdleTime;
} st_AliveReqMsg, *pst_AliveReqMsg;
#define DEF_ALIVEREQ_LEN	4

/***************** DEFINITION SESSIF & SESSSVC INTERFACE ************/
/* LGBSD 20060622(challa) ---> */ 
typedef struct _st_AAAFlgInfo
{
	UCHAR   ucTimeStampF;               /* BSD에서 UDR을 생성한 시간        (26/200) */
	UCHAR   ucUDRSeqF;                  /* UDR-Sequencea                    (26/201) */
	UCHAR   ucCallStatIDF;              /* Call-Stat-ID                         (31) */
	UCHAR   ucESNF;                     /* ESN                               (26/52) */
	UCHAR   ucFramedIPF;                /* Framed-IP-Address                     (8) */
    UCHAR   ucAcctSessIDF;              /* Accounting Session ID                (44) */
    UCHAR   ucCorrelationIDF;           /* Correlation ID                    (26/44) */
    UCHAR   ucSessContinueF;            /* Session Continue                  (26/48) */

	UCHAR   ucUserNameF;                /* USER-NAME(NAI)                        (1) */
	UCHAR   ucSvcOptF;                  /* SVC-OPTION                        (26/16) */ 
	UCHAR   ucNASIPF;                   /* PDSN-IP-ADDRESS                       (4) */
	UCHAR   ucPCFIPF;                   /* PCF IP                             (26/9) */
	UCHAR   ucHAIPF;                    /* HOME-AGENT                         (26/7) */
	UCHAR   ucBSIDF;                    /* 3GPP2-BSID                        (26/10) */
	UCHAR   ucFwdFCHMuxF;               /* 3GPP2-Foward-FCH-Mux-Option       (26/12) */
	UCHAR   ucRevFCHMuxF;               /* 3GPP2-Reverse-FCH-Mux-Option      (26/13) */

	UCHAR   ucFwdTrafTypeF;             /* 3GPP2-Forward-Traffic-Type        (26/17) */
	UCHAR   ucRevTrafTypeF;             /* 3GPP2-Reverse-Traffic-Type        (26/18) */
	UCHAR   ucFCHSizeF;                 /* 3GPP2-FCH-Frame-Size              (26/19) */
	UCHAR   ucFwdFCHRCF;                /* 3GPP2-Forward-FCH-RC              (26/20) */
	UCHAR   ucRevFCHRCF;                /* 3GPP2-Reverse-FCH-RC              (26/21) */
	UCHAR   ucIPTechF;                  /* IP Technology Flag                (26/22) */
	UCHAR   ucDCCHSizeF;                /* 3GPP2-DCCH-Frame-Size:0/5/20ms    (26/50) */
	UCHAR   ucReleaseIndF;              /* RELEASE-INDICATER                 (26/24) */  

	UCHAR   ucNASPortF;                 /* NAS-Port                              (5) */
	UCHAR   ucNASPortTypeF;             /* NAS-Port-Type                        (61) */
	UCHAR   ucNASPortIDF;               /* NAS-Port-ID                          (87) */   
	UCHAR   ucSvcTypeF;                 /* Service-Type                          (6) */
	UCHAR   ucAcctStatTypeF;            /* ACCT-STATUS-TYPE                     (40) */
	UCHAR   ucCompTunnelIndF;           /* 3GPP2-Compulsory-Tunnel-Indicator (26/23) */
	UCHAR   ucNumActF;                  /* 3GPP2-Number-Active-Transitions   (26/30) */  
    UCHAR   ucAcctInOctF;               /* Acct-Input-Octets 					(42) */

	UCHAR   ucAcctOutOctF;              /* Acct-Ouput-Octets  					(43) */
	UCHAR   ucAlwaysOnF;                /* Always-On                         (26/78) */
	UCHAR   ucAcctInPktF;               /* Acct-Input-Packets 					(47) */
	UCHAR   ucAcctOutPktF;              /* Acct-Output-Packets 					(48) */
	UCHAR   ucBadPPPFrameCntF;          /* 3GPP2-Bad-PPP-Frame-Count 		 (26/25) */ 
	UCHAR   ucActTimeF;                 /* 3GPP2-Active-Time                 (26/49) */
	UCHAR   ucTermSDBOctCntF;           /* 3GPP2-Terminating-SDB-Octet-Count (26/31) */
	UCHAR   ucOrgSDBOctCntF;            /* 3GPP2-Originating-SDB-Octet-Count (26/32) */

	UCHAR   ucTermNumSDBF;              /* 3GPP2-Terminating-Number-SDBs     (26/33) */
	UCHAR   ucOrgNumSDBF;               /* 3GPP2-Originating-Number-SDBs     (26/34) */
	UCHAR   ucEventTimeF;               /* Event Time                           (55) */
	UCHAR   ucRcvHDLCOctF;              /* 3GPP2-Received-HDLC-Octets        (26/43) */
	UCHAR   ucIPQoSF;                   /* IP QoS                            (26/36) */
	UCHAR   ucAcctSessTimeF;            /* Acct-Session-Time                    (46) */    
	UCHAR   ucAcctAuthF;                /* Acct-Authentic                       (45) */
	UCHAR   ucAcctTermCauseF;           /* Acct-Terminate-Cause                 (49) */

	UCHAR   ucAcctDelayTimeF;           /* Acct-Delay-Time                      (41) */
	UCHAR   ucAirQoSF;                  /* 3GPP2-Airlink-Priority            (26/39) */
	UCHAR   ucUserIDF;                  /* 3GPP2-User-ID (User-Zone)         (26/11) */
	UCHAR   ucMDNF;                     /* MDN(MSISDN)                       (26/202)*/
	UCHAR   ucRPConnectIDF;             /* 3GPP2-RP-Connect-ID               (26/41) */
	UCHAR   ucInMIPSigCntF;             /* 3GPP2_Mobile_IP_Signaling_Inbound_Count  (26/46) */
	UCHAR   ucOutMIPSigCntF;            /* 3GPP2_Mobile_IP_Signaling_Outbound_Count (26/47) */
    UCHAR   ucBeginningSessF;           /* Beginning-Session:True[1]Faulse[0](26/51) */

	/*** UDR Info ***/
	UCHAR   ucDataSvcTypeF;             /* Data-Service-Type                (26/203) */
	UCHAR   ucTransIDF;                 /* Transaction-ID                   (26/204) */
	UCHAR   ucReqTimeF;                 /* Requesst-Time                    (26/205) */
	UCHAR   ucResTimeF;                 /* Response-Time                    (26/206) */
	UCHAR   ucSessTimeF;                /* Session-Time 					(26/207) */
	UCHAR   ucDestIPF;                  /* Server-IP-Address                (26/208) */
	UCHAR   ucDestPortF;                /* Server-Port                      (26/209) */
	UCHAR   ucSrcPortF;                 /* Terminal-Port                    (26/210) */

	UCHAR   ucURLF;                     /* URL                              (26/211) */
	UCHAR	ucCTypeF;		            /* -> NEW ADD <-                    (26/212) */
	/** Chagend Info ***/
	UCHAR   ucAppIDF;                   /* Application-ID                   (26/213) */
	UCHAR   ucCntCodeF;                 /* Content-Code 					(26/214) */
	UCHAR   ucMethTypeF;                /* Method-Type 						(26/215) */
	UCHAR   ucResultCodeF;              /* Result-Code 						(26/216) */
	UCHAR   ucIPUpSizeF;                /* IP-Layer-Upload-Size 			(26/217) */
	UCHAR   ucIPDownSizeF;              /* IP-Layer-Download-Size           (26/218) */

	UCHAR   ucReInputSizeF;             /* TCP-Layer-Retrans-Input-Size     (26/219) */
	UCHAR   ucReOutputSizeF;            /* TCP-Layer-Retrans-Output-Size    (26/220) */
	UCHAR   ucCntLenF;                  /* Transaction-Content-Length       (26/221) */
	UCHAR   ucTransCompleteF;           /* Transaction-Completeness 		(26/222) */
	UCHAR   ucTransTermReasonF;         /* Transaction-Termination-Reason   (26/223) */
	UCHAR   ucUserAgentF;               /* User-Agent                       (26/224) */

	/* R1.3.0 20061127 add(challa) ---> */
	UCHAR   ucCPCodeF;                  /* CP Code                          (26/228) */
	UCHAR   ucPNumberF;                 /* Phone-Number                     (26/229) */

	UCHAR   ucUseCntF;                  /* Use-Count                        (26/230) */
	UCHAR   ucUseTimeF;                 /* Use-Time                         (26/231) */
	UCHAR   ucTotSizeF;                 /* Total-Size                       (26/232) */
	UCHAR   ucTotTimeF;                 /* Total-time                       (26/233) */
	UCHAR   ucBillcomF;                 /* Billcom-Header-Count             (26/234) */
	UCHAR   ucGatewayHeaderF;           /* Gateway-Header-Count             (26/235) */
	UCHAR   ucHModelF;                  /* Handset-Model                    (26/236) */
	UCHAR   ucPktCntF;                  /* Packet-Counter                   (26/237) */
	/* <--- */

	UCHAR   ucRetryF;                   /* Retry-Flag                       (26/250) */
	UCHAR	ucReserved[7];

} st_AAAFlgInfo, *pst_AAAFlgInfo;



/* LGBSD 06/06/23 (challa) ---> */
#define MAX_AUTH_SIZE        16
#define MAX_ESN_SIZE         16
#define MAX_BSID_SIZE        12
#define MAX_PORT_SIZE        24
#define MAX_USERNAME_SIZE    72
#define	MAX_SUBNET_SIZE      37		/* 071203, poopee, SUBNET */

/* #define MAX_URL_SIZE          151 */
#define MAX_URL_SIZE          721
#define MAX_USERAGENT_SIZE    61
#define MAX_HOST_LEN          61 

#define MAX_TRANSPORT_SIZE    201
#define MAX_SESSION_SIZE      51
#define MAX_BILLINFO_SIZE     101
/* <--- */

#define MAX_USERPWD_SIZE		17
#define MAX_CHAPPWD_SIZE		18
#define MAX_CNTS_SIZE		13
#define MAX_APPID_SIZE		11

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
	UCHAR   ucCode;                     /* RADIUS-CODE                               */
	UCHAR   ucID;                       /* RADIUS IDENTIFIER                         */ 
	UCHAR   ucUserLen;                  /* USER-NAME LENGTH                          */
    UCHAR   ucUDRSeqF;                  /* UDR-Sequence                     (26/201) */  
    UCHAR   ucTimeStampF;               /* BSD에서 UDR을 생성한 시간        (26/200) */
	UCHAR   ucCallStatIDF;              /* Call-Stat-ID                         (31) */
	UCHAR   ucESNF;                     /* ESN                               (26/52) */
	UCHAR   ucFramedIPF;                /* Framed-IP-Address                     (8) */

    UCHAR   ucAcctSessIDF;              /* Accounting Session ID                (44) */
    UCHAR   ucCorrelationIDF;           /* Correlation ID                    (26/44) */
    UCHAR   ucSessContinueF;            /* Session Continue                  (26/48) */
	UCHAR   ucUserNameF;                /* USER-NAME(NAI)                        (1) */
	UCHAR   ucSvcOptF;                  /* SVC-OPTION                        (26/16) */ 
	UCHAR   ucUserIDF;                  /* 3GPP2-User-ID (User-Zone)         (26/11) */
	UCHAR   ucNASIPF;                   /* PDSN-IP-ADDRESS                       (4) */
	UCHAR   ucHAIPF;                    /* HOME-AGENT                         (26/7) */

	UCHAR   ucPCFIPF;                   /* PCF IP                             (26/9) */
	UCHAR   ucBSIDF;                    /* 3GPP2-BSID                        (26/10) */
	UCHAR   ucFwdFCHMuxF;               /* 3GPP2-Foward-FCH-Mux-Option       (26/12) */
	UCHAR   ucRevFCHMuxF;               /* 3GPP2-Reverse-FCH-Mux-Option      (26/13) */
	UCHAR   ucFwdTrafTypeF;             /* 3GPP2-Forward-Traffic-Type        (26/17) */
	UCHAR   ucRevTrafTypeF;             /* 3GPP2-Reverse-Traffic-Type        (26/18) */
	UCHAR   ucFCHSizeF;                 /* 3GPP2-FCH-Frame-Size              (26/19) */
	UCHAR   ucFwdFCHRCF;                /* 3GPP2-Forward-FCH-RC              (26/20) */

	UCHAR   ucRevFCHRCF;                /* 3GPP2-Reverse-FCH-RC              (26/21) */
	UCHAR   ucIPTechF;                  /* IP Technology Flag                (26/22) */
	UCHAR   ucDCCHSizeF;                /* 3GPP2-DCCH-Frame-Size:0/5/20ms    (26/50) */
	UCHAR   ucReleaseIndF;              /* RELEASE-INDICATER                 (26/24) */  
	UCHAR   ucNASPortF;                 /* NAS-Port                              (5) */
	UCHAR   ucNASPortTypeF;             /* NAS-Port-Type                        (61) */
	UCHAR   ucNASPortIDF;               /* NAS-Port-ID                          (87) */   
	UCHAR   ucNASPortIDLen;             /* NAS_Port-ID LEN                           */ 

	UCHAR   ucSvcTypeF;                 /* Service-Type                          (6) */
	UCHAR   ucAcctStatTypeF;            /* ACCT-STATUS-TYPE                     (40) */
	UCHAR   ucNumActF;                  /* 3GPP2-Number-Active-Transitions   (26/30) */  
    UCHAR   ucAcctInOctF;               /* Acct-Input-Octets 					(42) */
	UCHAR   ucAcctOutOctF;              /* Acct-Ouput-Octets  					(43) */
	UCHAR   ucAlwaysOnF;                /* Always-On                         (26/78) */
	UCHAR   ucAcctInPktF;               /* Acct-Input-Packets 					(47) */
	UCHAR   ucAcctOutPktF;              /* Acct-Output-Packets 					(48) */

	UCHAR   ucBadPPPFrameCntF;          /* 3GPP2-Bad-PPP-Frame-Count 		 (26/25) */ 
	UCHAR   ucEventTimeF;               /* Event Time                           (55) */
	UCHAR   ucActTimeF;                 /* 3GPP2-Active-Time                 (26/49) */
	UCHAR   ucTermSDBOctCntF;           /* 3GPP2-Terminating-SDB-Octet-Count (26/31) */
	UCHAR   ucOrgSDBOctCntF;            /* 3GPP2-Originating-SDB-Octet-Count (26/32) */
	UCHAR   ucTermNumSDBF;              /* 3GPP2-Terminating-Number-SDBs     (26/33) */
	UCHAR   ucOrgNumSDBF;               /* 3GPP2-Originating-Number-SDBs     (26/34) */
	UCHAR   ucRcvHDLCOctF;              /* 3GPP2-Received-HDLC-Octets        (26/43) */

	UCHAR   ucIPQoSF;                   /* IP QoS                            (26/36) */
	UCHAR   ucAcctSessTimeF;            /* Acct-Session-Time                    (46) */    
	UCHAR   ucCompTunnelIndF;           /* 3GPP2-Compulsory-Tunnel-Indicator (26/23) */
	UCHAR   ucAcctAuthF;                /* Acct-Authentic                       (45) */
	UCHAR   ucAcctTermCauseF;           /* Acct-Terminate-Cause                 (49) */
	UCHAR   ucAcctDelayTimeF;           /* Acct-Delay-Time                      (41) */
	UCHAR   ucAirQoSF;                  /* 3GPP2-Airlink-Priority            (26/39) */
	UCHAR   ucRPConnectIDF;             /* 3GPP2-RP-Connect-ID               (26/41) */

	UCHAR   ucInMIPSigCntF;             /* 3GPP2_Mobile_IP_Signaling_Inbound_Count  (26/46) */
	UCHAR   ucOutMIPSigCntF;            /* 3GPP2_Mobile_IP_Signaling_Outbound_Count (26/47) */
	UCHAR   ucMDNF;                     /* MDN(MSISDN)                      (26/202) */
    UCHAR   ucAAAIPF;                   /* AAAIP                                     */
    UCHAR   ucRetryF;                   /* Retry-Flag                       (26/250) */
	UCHAR	ucAcctInterimF;				/* Acct-Interim-Interval                     */
    UCHAR   ucBeginningSessF;           /* Beginning-Session:True[1]Faulse[0](26/51) */ 
    UCHAR   szReservd[1];
    
    UCHAR   ucC23BITF;
	UCHAR	ucHBITF;					/* 080220, poopee, HBIT */
    UCHAR   ucSubnetF;					/* 071203, poopee, SUBNET */
    UCHAR   szReserved2[5];

	/**** VALUE ****/
    UINT    uiUDRSeq;                           /* UDR-Sequence                     */
    UINT    uiTimeStamp;                        /* UDR 생성 시간                    */ 
    UINT    uiAAAIP;                            /* AAAIP address                    */
    UINT    uiKey;                              /* UDRGEN <-> AAAIF DUPLICATION Key */

    UINT    uiFramedIP;                 /* Framed-IP-Address                     (8) */
	UINT    uiNASIP;                    /* PDSN-IP-ADDRESS                       (4) */ 
	UINT    uiPCFIP;                    /* PCF IP                             (26/9) */ 
	UINT    uiHAIP;                     /* HOME-AGENT                         (26/7) */ 

    UINT    uiRADIUSLen;                /* RADIUS PACKET LEN                        */
    UINT    uiSessContinue;             /* SESSION-CONTINUE                         */
    UINT    uiBeginnigSess;             /* BEGINNING-SESSION                (26/51) */
	INT     dSvcOpt;                    /* Service-Option                           */

	INT     dAcctStatType;              /* Acct-Status-Type                         */
	INT     dCompTunnelInd;             /* 3GPP2-Compulsory-Tunnel-Indicator        */
	INT     dNumAct;                    /* 3GPP2-Number-Active                      */
	INT     dSvcType;                   /* Service-Type                             */   

    INT     dFwdFCHMux;                 /* 3GPP2-Foward-FCH-Mux-Option      (26/12) */
    INT     dRevFCHMux;                 /* 3GPP2-Reverse-FCH-Mux-Option     (26/13) */
    INT     dFwdTrafType;               /* 3GPP2-Forward-Traffic-Type               */
    INT     dRevTrafType;               /* 3GPP2-Reverse-Traffic-Type               */

    INT     dFCHSize;                   /* 3GPP2-FCH-Frame-Size                     */
    INT     dFwdFCHRC;                  /* 3GPP2-Forward-FCH-RC                     */
    INT     dRevFCHRC;                  /* 3GPP2-Reverse-FCH-RC                     */
    INT     dIPTech;                    /* IP Technology                            */

	INT     dDCCHSize;                  /* 3GPP2-DCCH-Frame-Size                    */
	INT     dNASPort;                   /* NAS-Port(PDSN IP Addr)                   */
	INT     dNASPortType;               /* NAS-Port-Type (18=wireless other,22=1x, 24=EVDO) */
    INT     dReleaseInd;                /* Release Indicator                        */

    INT     dAcctInOct;                 /* Acct-Input-Octets                        */
    INT     dAcctOutOct;                /* Acct-Output-Octets                       */
    INT     dAcctInPkt;                 /* Acct-Input-Packets                       */
    INT     dAcctOutPkt;                /* Acct-Output-Packets                      */

    UINT    uiEventTime;                /* Event Time                               */
    UINT    uiActTime;                  /* 3GPP2-Active-Time                        */
    UINT    uiAcctSessTime;             /* Accounting Sesssion Time                 */
    UINT    uiAcctDelayTime;            /* Acct-Delay-Time                          */

    INT     dTermSDBOctCnt;             /* 3GPP2-Terminating-SDB-Octet-Count        */
    INT     dOrgSDBOctCnt;              /* 3GPP2-Originating-SDB-Octet-Count        */
    INT     dTermNumSDB;                /* 3GPP2-Terminating-Number-SDBs            */
    INT     dOrgNumSDB;                 /* 3GPP2-Originating-Number-SDBs            */

    INT     dRcvHDLCOct;                /* 3GPP2-Received-HDLC-Octets               */
    INT     dIPQoS;                     /* IP QoS                                   */
    INT     dAirQoS;                    /* Air QoS                                  */
    INT     dRPConnectID;               /* 3GPP2-RP-Connect-ID (26/41,integer)      */

    INT     dBadPPPFrameCnt;            /* 3GPP2-Bad-PPP-Frame-Count                */
    INT     dAcctAuth;                  /* Acct-Authentic (0=RADIUS, 1=LOCAL)       */
    INT     dAcctTermCause;             /* Acct-Terminate-Cause                     */
    INT     dAlwaysOn;                  /* Always-On                                */

    INT     dUserID;                    /* 3GPP2-User-ID                            */
    INT     dInMIPSigCnt;               /* 3GPP2_Mobile_IP_Signaling_Inbound_Count  */
    INT     dOutMIPSigCnt;              /* 3GPP2_Mobile_IP_Signaling_Inbound_Count  */ 
	INT     dAcctInterim;               /* Acct-Interim-Interval */

    INT64   llAcctSessID;                       /* Accounting Session ID            */
	INT64   llCorrelID;                         /* Correlation ID                   */
    UINT    uiRetryFlg;                         /* 재전송 시 포함 = 1               */
    INT     dReserved;

    INT     uiC23BIT;                   /*1:Network Mode 0:relay mode */
    INT     uiHBIT;						/* 080220, poopee, HBIT */

	UCHAR   szSubnet     [MAX_SUBNET_SIZE+1];       /* 071203, poopee, SUBNET */
	UCHAR	szReserved3	 [2];	
	UCHAR   szAuthen     [MAX_AUTH_SIZE];       /* RADIUS Authenticator : 16        */
	UCHAR   szMDN        [MAX_MIN_SIZE];        /* MDN : 15+1                       */
	UCHAR   szESN        [MAX_ESN_SIZE];        /* ESN : 15+1                       */ 
	UCHAR   szUserName   [MAX_USERNAME_SIZE];   /* User Name : 72                   */
	UCHAR   szBSID       [MAX_BSID_SIZE];       /* 3GPP2-BSID : 12                  */
	UCHAR   szNASPortID  [MAX_PORT_SIZE];       /* Variable(임의대로 24)            */
	UCHAR   szMIN        [MAX_MIN_SIZE];        /* Calling-Station-ID : 15+1        */

	st_AccessInfo	stAccessInfo;

} st_ACCInfo, *pst_ACCInfo;

#define DEF_ACCINFO_SIZE		sizeof(st_ACCInfo)

#pragma pack(4)
typedef struct _st_UDRInfo_ 
{
    UCHAR	    ucAcctSessIDF;            /* Accounting-Session-ID                   */

	UCHAR       ucDataSvcTypeF;            /* Service Type                            */
	UCHAR       ucTranIDF;                /* Transaction-ID                          */
    UCHAR       ucReqTimeF;                /* Request Time                            */
    UCHAR       ucResTimeF;			    /* Response Time                           */
	UCHAR       ucSessionTimeF;		    /* Service Duration                        */

    UCHAR       ucDestIPF;                /* Server IP                               */
    UCHAR    	ucDestPortF;               /* Server Port                             */
    UCHAR       ucSrcPortF;                /* Terminal Port                           */
	UCHAR		ucCTypeF;                  /* C-Type                         (26/212) */

	UCHAR       ucAppIDF;                  /* Application ID                          */
	UCHAR       ucContentCodeF;            /* Content Code                            */
	UCHAR       ucMethodTypeF;
	UCHAR       ucResultCodeF;

	UCHAR       ucIPUpSizeF;
	UCHAR       ucIPDownSizeF;
	UCHAR       ucRetransInSizeF;          /*  TCP Upload Retrans Size                */
	UCHAR       ucRetransOutSizeF;         /*  TCP Down Retrans Size                  */

    UCHAR       ucCPCodeF;                  /* CP-Identifier                     */
    UCHAR       ucUseCountF;               /* Usage Count                              */
    UCHAR       ucUseTimeF;                /* Usage Time                                */
    UCHAR       ucTotalSizeF;              /* Total Size                              */
    UCHAR       ucTotalTimeF;              /* Total Time                                */
    UCHAR       ucBillcomCountF;           /* BILLCOM Header Count                      */
    UCHAR       ucGWCountF;                /* Gateway Header Count                      */


	UCHAR       ucContentLenF;
	UCHAR       ucTranCompleteF;
	UCHAR       ucTranTermReasonF;

	UCHAR       ucURLF;                     /* URL(151)                                 */
	UCHAR       ucUserAgentF;	            /* SIZE:61                          */
	UCHAR	    ucHostF;			        /* SIZE:61                          */
	UCHAR	    ucMDNF;			            /* SIZE:16                          */
    UCHAR       ucPhoneNumF;                /* SIZE:16                          */
    UCHAR       ucModelF;                   /* SIZE:11                          */
    UCHAR       ucPacketCntF;               /* Packet Count                     */
    UCHAR       ucAudioInputIPSizeF;
	UCHAR       ucAudioOutputIPSizeF;
	UCHAR       ucVideoInputIPSizeF;
	UCHAR       ucVideoOutputIPSizeF;
    UCHAR       ucRSFLagF;                
    
    UCHAR       ucCallerMinF;
    UCHAR       ucCalledMinF;              
    char        szReserved[6];

    INT64      llAcctSessID;            /* Accounting-Session-ID                   */

	INT        dDataSvcType;            /* Service Type                            */
	int			dReserved;

	UINT       uiTranID;                /* Transaction-ID                          */
    time_t     tReqTime;                /* Request Time                            */
    time_t     tResTime;			    /* Response Time                           */
	time_t     tSessionTime;		    /* Service Duration                        */

    UINT       uiDestIP;                /* Server IP                               */
    INT    	   dDestPort;               /* Server Port                             */
    INT        dSrcPort;                /* Terminal Port                           */
	INT		   dCType;                  /* C-Type                         (26/212) */

	char       szAppID[MAX_APPID_SIZE];              /* Application ID:11          */
	char	   szContentCode[MAX_CNTS_SIZE];         /* Content Code:13            */

	INT        dMethodType;
	INT        dResultCode;

	INT        dIPUpSize;
	INT        dIPDownSize;
	INT        dRetransInSize;          /*  TCP Upload Retrans Size                */
	INT        dRetransOutSize;         /*  TCP Down Retrans Size                  */

    INT        dCPCode;                 /* CP-Identifier                            */
    INT        dUseCount;               /* Usage Count                              */
    INT        dUseTime;                /* Usage Time                                */
    INT        dTotalSize;              /* Total Size                              */
    INT        dTotalTime;              /* Total Time                                */
    INT        dBillcomCount;           /* BILLCOM Header Count                      */

    INT        dGWCount;                /* Gateway Header Count                      */
	INT        dContentLen;

	INT        dTranComplete;
	INT        dTranTermReason;
	
	INT         dAudioInputIPSize;
	INT         dAudioOutputIPSize;
		 
    INT         dVideoInputIPSize;      
	INT         dVideoOutputIPSize;  


	char       szURL[MAX_URL_SIZE];     /* URL(151)                                 */
	char       szUserAgent[MAX_USERAGENT_SIZE];	/* SIZE:61                          */
	char	   szHost[MAX_HOST_LEN];			/* SIZE:61                          */
	char	   szMDN[MAX_MIN_SIZE-1];			/* SIZE:16                          */
    char       szPhoneNum[MAX_MIN_SIZE-1];      /* SIZE:16                          */
    char       szModel[MAX_MODEL_SIZE];      /* SIZE:11                          */
	char       szPacketCnt[DEF_PKTCNT_SIZE];		/* PACKET COUNT:93				*/
	UCHAR	   ucURLCha;
    UCHAR       ucRSFlag;
    char        szReserved1[6];
 
    char        szCallerMin[MAX_MIN_SIZE-1];
    char        szCalledMin[MAX_MIN_SIZE-1];
} st_UDRInfo, *pst_UDRInfo;
#pragma pack()

typedef struct _st_AAAREQ
{
    st_ACCInfo  stInfo;

    INT     dUDRCount;
    INT     dReserved;

    st_UDRInfo stUDRInfo[MAX_UDR_COUNT];
}st_AAAREQ, *pst_AAAREQ;


/* 200607010(challa) add ---> */
/* SDMA <-> AAAIF: Duplicate */
#define  MAX_BODY_SIZE     4096
#define  MAX_QUEUE_NUM     10000

typedef struct _st_Queue {
    UINT    uiUDRSeq;
    UINT    uiKey;
    INT64   llAcctSessID;
    UCHAR   szMIN[MAX_MIN_SIZE];
    UCHAR   szReserved[3];
    UCHAR   szBody[MAX_BODY_SIZE];  /* st_AAAREQ */
} st_Queue, *pst_Queue;

typedef struct _st_QueueList {
    short       tail;
    short       head;
    st_Queue    stQueue[MAX_QUEUE_NUM];
} st_QueueList, *pst_QueueList;


/** USE STANDBY COPY **/ 
typedef struct _st_StanbyInfo{
    UINT     uiAAAIP;
    UCHAR    szMIN[MAX_MIN_SIZE];
    CHAR     szBody[MAX_BODY_SIZE];
} st_StandbyInfo, *pst_StandbyInfo;

/* <---- */




#define	DEF_ACCINFO_SIZE		sizeof(st_ACCInfo)

/*************** DEFINITION SESSSVC & DSCPIF INTERFACE ***************/
typedef struct _st_SndDSCP
{
	/* MIN -> Queue Header szMIN */
	char			szIMSI[MAX_IMSI_SIZE]; 				/* IWF/PDSN No Data */
	char			szWinSvcName[MAX_WINSVCNAME_SIZE]; 	/* IWF/PDSN No Data */
    UINT           	uiWSType;							/* IWF/PDSN WSType */
	UINT			uiSVCOption;						/* IWF/PDSN SVC Option */
	USHORT			usTerminal;							/* IWF/PDSN Termial Capability */
	char			szCellInfo[MAX_CELLINFO_SIZE];		/* IWF/PDSN No Data */
	/* NAS -> Queue Header ucNaType */
    /* PDSN_ADDRESS -> Queue Header	ulNASIP : ui -> char convert */
	INT64			llCallID;			/* DSCP Key Index */
	UCHAR			ucMode;				/* Interim Mode, 1 = Interim */
	UCHAR			ucStatus;			/* Call Status */
	UINT			uiSeqID;			/* Interim Sequence ID */
	time_t			uiStartTime;
	time_t			uiUsedTime;
	UINT			uiRetCode;			/* ADR or CauseCode Field */ 
	USHORT			usCDRCount;	
	st_CDRInfo		stCDRInfo[MAX_CATESVC_COUNT];
} st_SndDSCP, *pst_SndDSCP;

typedef struct _st_RcvDSCP
{
	/* MIN -> Queue Header szMIN */
	char			szIMSI[MAX_IMSI_SIZE];	/* IWF/PDSN No Data */
	INT64			llCallID;			/* DSCP Key Index */
	UCHAR			ucMode;				/* Interim Mode, 1 = Interim */
	UINT			uiSeqID;			/* Interim Sequence ID */
	UINT			uiTPeriod;			/* Time Period : Accounting Info */
	UINT			uiPPeriod;			/* Packet Period : Accounting Info */
	UINT			uiRetCode;			/* ADR or CauseCode Field */ 
} st_RcvDSCP, *pst_RcvDSCP;

/****************** IPAM UDP INTERFACE *******************/
typedef struct _st_IPAFUHeader
{
    INT64       llTID;          		/* Transzction ID */
	INT64		llIndex;				/* Request for DB Index Used IPAFUIF */
	char        szMin[MAX_MIN_SIZE]; 	/* USER MIN */
	UCHAR		szReserved[7];			/* Reserved */
	INT64		llAcctSessID;			/* Session ID */
	UINT        uiSIP;              	/* Framed-IP-Address */
	UCHAR       ucUserType;         	/* 1 : POSTPAID, 2 : PREPAID, 4:NOREMAIN*/
	UCHAR       ucIPAFID;           	/* IPAF ID */
    USHORT      usResult;  				/* Result */
	USHORT		usTotlLen;				/* Packet Header + Packet Length */
	USHORT		usBodyLen;				/* Packet Body Length + Extended Len */
    USHORT      usExtLen;       		/* Packet Extension Length or Error */
	UCHAR		ucReserved[2];			/* Reserved */
} st_IPAMUHeader, *pst_IPAMUHeader;

#define IPAMU_HEADER_LEN		sizeof(st_IPAMUHeader)
#define DEF_UHEADER_LEN			sizeof(st_IPAMUHeader)

#define MAGIC_NUMBER			0x3812121281282828L
typedef struct _st_IPAFTHeader
{
	INT64       llMagicNumber;      /* Magic Number 0x3812121281282828L*/
	INT64		llIndex;
	USHORT      usResult;           /* Result */
	USHORT      usSerial;
	UCHAR       ucIPAFID;
	UCHAR       szReserved[3];      /* Reserved */
	USHORT      usTotlLen;          /* Packet Header + Packet Length */
	USHORT      usBodyLen;          /* Packet Body Length + Extended Len */
	USHORT      usExtLen;           /* Packet Extension Length or Error */
	UCHAR       ucSvcID;
	UCHAR       ucMsgID;
}st_IPAFTHeader, *pst_IPAFTHeader;

#define IPAFT_HEADER_LEN        sizeof(st_IPAFTHeader)
typedef struct _st_TID
{
    UCHAR       ucMsgID;		/* Packet Service Msg Code */
    UCHAR       ucSvcID;        /* Packet Service Code */
    USHORT      usSerial;		/* Serial Number */
    time_t      stBuild; 		/* TID Build Time */
} st_TID, *pst_TID;

typedef union _un_TID
{
	INT64		llTID;
	st_TID		stTID;
} un_TID, *pun_TID;

typedef struct _st_Ack
{
	USHORT		usRetCode;
} st_Ack, *pst_Ack;

typedef struct _st_CDRAck
{
	USHORT		usRetCode;
	UCHAR		ucPayType;		
} st_CDRAck, *pst_CDRAck;

typedef struct _st_IPPool
{
	int			iSysType;
	int			iSysID;
	char		szSysIP[16];
	int			iPrefix;
} st_IPPool, *pst_IPPool;

typedef struct _st_CategoryInfo
{
	USHORT		usCategory;
	UCHAR		ucGroup;
	UCHAR		ucLayer;
	UCHAR		szServiceID[3];
    UCHAR   	ucMode;
    UCHAR   	ucSvcBlk;
	UCHAR		ucFilterOut;
	UCHAR		ucConCount;
	UCHAR		ucConSize;
	UCHAR		szCon[MAX_CON_COUNT][MAX_CON_SIZE];
	char		szReserved[6];
} st_CategoryInfo, *pst_CategoryInfo;

#define DEF_IPADDR_SIZE	16

typedef struct _st_IPPOOL
{
    UCHAR       ucSystemType;
    UCHAR       ucSystemID;
    char        szIP[DEF_IPADDR_SIZE];
    UCHAR       ucNetmask;
    char        szReserved[5];
}st_IPPOOL, *pst_IPPOOL;

typedef struct _st_SerCat
{
    UCHAR   ucCategory;
    UCHAR   ucGroup;
    UCHAR   ucLayer;
    UCHAR   ucFilterOut;
    int     dServiceID;
	UCHAR	ucMode;
	UCHAR	ucSvcBlk;
    UCHAR   ucConCount;
    UCHAR   ucConSize;
    UCHAR   szContent[DEF_CONTENT_SIZE];
    UCHAR   ucStatus;
	char    szAlias[DEF_SER_ALIAS_SIZE];
}st_SerCat, *pst_SerCat;

#define MAX_SERVICE_CATEGORY_COUNT  200

typedef struct _st_SerCatTot {
	int   dCount;
	int   dResev;
	st_SerCat  stSerCat[MAX_SERVICE_CATEGORY_COUNT];
} st_SerCatTot, *pst_SerCatTot;

typedef struct _st_AliveReq
{
	UINT		uiIdleTime;
} st_AliveReq, *pst_AliveReq;

typedef struct _st_PeriodReq
{
	UCHAR		ucAmountType;
	UINT		uiPacketPeriod;
	UINT		uiTimePeriod;
} st_PeriodReq, *pst_PeriodReq;



typedef struct _st_PatchName
{
	char		szPatchName[32];
} st_PatchName, *pst_PatchName;

#define DEF_PATCHNAME_SIZE	sizeof(st_PatchName)


typedef struct _st_PatchType
{
	UINT		uiPatchType;    /* 1: reboot, 2:reset processes and shared-mem, 3:reset processes*/
}st_PatchType, *pst_PatchType;

#define DEF_PATCHTYPE_SIZE	sizeof(st_PatchType)

#define PATCH_TYPE_UNKNOWN		0	/* unknown type */
#define PATCH_TYPE_REBOOT		1	/* after patch, reboot IPAF */
#define PATCH_TYPE_RESET_PS_SHM 2	/* after patch, restart all process(except ADMIN) and SHM */
#define PATCH_TYPE_RESET_PS		3	/* after patch, restart all process(except ADMIN) */

#define PATCH_TYPE_EACH_PS		4	/* Added by SJS, for patching each process */
#define MAX_PROCESS_NUM			100	/* Added by SJS, for patching each process */

/* ADD BY HWH 2004/05/11 */
/* STOP MSG ACK 시에 해당 Structure를 보내 줌 */
#define MAX_BSMSC_SIZE   13
typedef struct _st_TCP_DATA
{
    UINT        uiTotUpByte;        /* IP + TCP + APP DATA */
    UINT        uiTotDwByte;

    UINT        uiTotTCPUpByte;     /* TCP + APP DATA */
    UINT        uiTotTCPDwByte;

    UINT        uiTotTCPReUpByte;   /* TCP Retransmission */
    UINT        uiTotTCPReDwByte;

    UINT        uiTRANUpByte;       /* APP DATA (HEADER+BODY) */
    UINT        uiTRANDwByte;

    UINT        uiDROPUpByte;       /* 데이타 빠짐 */
    UINT        uiDROPDwByte;

    UINT        uiREALUpByte;       /* 실제 크기와 차이 오류 */
    UINT        uiREALDwByte;

    UINT        uiFAILUpByte;       /* 서비스군 오류 */
    UINT        uiFAILDwByte;

    UINT        uiTRANCnt;          /* TRANSACTION COUNT */
    UINT        uiReserved;
}st_TCP_DATA, *pst_TCP_DATA;

#define DEF_TCPDATA_SIZE		sizeof(st_TCP_DATA)

/* LGT BMT NEW STRUCTURE 2006.04.11 */
typedef struct _st_Radius_ 
{
    UCHAR Code;
    UCHAR Identifier;
    UCHAR Length[2];
    UCHAR Authenticator[16];
    UCHAR Attributes[1];
} st_Radius, *pst_Radius;

#define MAX_BSMSC_SIZE	13	/* 최대인 BS/MSC ID 기준 */

typedef struct _st_AccReq_
{
	UCHAR	ucCode;
	UCHAR	ucID;
	UCHAR	szAuth[17];
	UCHAR	szReqserved1[5];
	
	UINT	uiEventTime;
	UINT	uiAccStatus;
	UINT	uiFramedIP;
	UINT    uiSvcOpt;
	INT64   llAccSessID;
	INT64	llCorrelID;
	UCHAR	szCallingID[MAX_MIN_SIZE];
	UCHAR	szBSMSCID[MAX_BSMSC_SIZE];
	UCHAR	szReserved2[2];
} st_AccReq, *pst_AccReq;

#define DEF_ACCREQ_SIZE	sizeof(st_AccReq)

#define DEF_PAYINFO_CNT     500

typedef struct _st_PayInfo_
{
    int     dPayType;
    int     dAmountType;
    int     dPeriod;
    char    szMin[MAX_MIN_SIZE];
} st_PayInfo, *pst_PayInfo;

typedef struct _st_PayInfoList
{
    int         dCount;
    st_PayInfo  stPayInfo[DEF_PAYINFO_CNT];
} st_PayInfoList, *pst_PayInfoList;

typedef struct _st_UAFetchTerm
{
    unsigned int    uiFetchTerm;
} st_UAFetchTerm, *pst_UAFetchTerm;


typedef struct _st_SESSClean
{
	UCHAR		    szMIN[MAX_MIN_SIZE];		/* MIN Number */
    INT64           llAcctSessID;
    UINT            uiSrcIP;
} st_SESSClean, *pst_SESSClean;


/*

typedef struct _st_UDR_LIST_INFO
{
	time_t			startTime;
    unsigned int    cdrCnt;
    unsigned int    cdrId;
    char            fileName[32];
    int             fd;



} st_UDR_LIST_INFO, *pst_UDR_LIST_INFO;


*/

/* In Dual-Active Case, ACCInfo Update Req */
typedef struct _st_SessUpdACCInfo_t{
    time_t              tLastUpdTime;
    unsigned long long  llMIN;
    unsigned long long  llAcctSessID;
    st_ACCInfo          stACCInfo;
} st_SessUpdACCInfo_t, *pst_SessUpdACCInfo_t;


/* In Dual-Active Case, UDRInfo Update Req */
typedef struct _st_SessUpdUDRInfo_t{
    time_t              tLastUpdTime;
    unsigned long long  llMIN;
    unsigned long long  llAcctSessID;
    st_UDRInfo          stUDRInfo;
} st_SessUpdUDRInfo_t, *pst_SessUpdUDRInfo_t;

typedef struct _st_TCP_DUP_KEY
{
    UINT        uiSIP;              /* Source IP */
    UINT        uiDIP;              /* Destination IP */
    UCHAR       ucProType;          /* Protocal Type : 6->TCP 17->UDP */
    UCHAR       szReserved[3];      /* Reserved */
    USHORT      usSPort;            /* Source Port */
    USHORT      usDPort;            /* Destination Port */
} TCP_DUP_KEY;

#endif
