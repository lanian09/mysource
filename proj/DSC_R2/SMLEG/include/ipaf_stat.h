#ifndef _IPAF_STAT
#define _IPAF_STAT

#include <ipaf_svc.h>
#include <ipaf_names.h>
#pragma pack(1)

typedef struct _st_ACCESS_Stat_
{
	UINT        usTotAuthReq;
	UINT        usTotAuthRes;
	UINT        usAuthTimeout;
	UINT        usTotAuthAcpt;
	UINT        usTotAuthRjt;
	UINT        usNoIterimAuthAcpt;
	UINT        usTotCmplAuthTxn;
	UINT        usOnlyAuthAcpt;
	UINT        usOnlyAuthRjt;
	UINT        usDecErrAuthReq;
	UINT        usDecErrAuthRes;
	UINT        usDupAuthReq;
	UINT        usEtcErrAuthReq;
	UINT        usEtcErrAuthRes;
} st_ACCESS_Stat, *pst_ACCESS_Stat;

typedef struct _st_ACCOUNTING_Stat_
{
	UINT		usTotAcctReq;
	UINT        usTotAcctRes;
	UINT        usTotAcctTimeout;
	UINT        usTotStart;
	UINT        usTotInterim;
	UINT        usTotStop;
	UINT        usSessContinue;
	UINT        usMDNAcctRes;
	UINT        usTotCmplAcctTxn;
	UINT        usOnlyAcctReq;
	UINT        usOnlyAcctRes;
	UINT        usDecErrAcctReq;
	UINT        usDecErrAcctRes;
	UINT        usDupAcctReq;
	UINT        usEtcErrAuthReq;
	UINT        usEtcErrAuthRes;
} st_ACCOUNTING_Stat, *pst_ACCOUNTING_Stat;
 
typedef struct
{
    unsigned int    uiUpFrames;
    unsigned int    uiDownFrames;
    unsigned int    uiUpBytes;
    unsigned int    uiDownBytes;
} st_IpafStat, *pst_IpafStat;

typedef struct _st_IpafStatList
{
    time_t          KeyTime;
    UCHAR           ucIPAFID;
    UCHAR           Reserved[3];

    st_IpafStat     TotStat;
    st_IpafStat     IPStat;
    st_IpafStat     UDPStat;
    st_IpafStat     TCPStat;

	st_IpafStat		IPError;
    st_IpafStat		UTCPError;
    st_IpafStat		TCPReTrans;

	st_IpafStat		OutOfIP;
    st_IpafStat		DropData;
    st_IpafStat		FailData;

}st_IpafStatList, *pst_IpafStatList;

#define DEF_IPAFSTAT_LEN	sizeof(st_IpafStatList)

#define DEF_ME_TYPE     0x01
#define DEF_KUN_TYPE    0x02
#define DEF_ADS_TYPE    0x03
#define DEF_MARS_TYPE   0x04

#define DEF_MACS_TYPE	0x05
#define DEF_WICGS_TYPE  0x06
#define DEF_VODM_TYPE	0x07
#define DEF_VODD_TYPE	0x08
#define DEF_VODS_TYPE	0x09

#define DEF_WAP_TYPE	0x10
#define DEF_WAP2_TYPE	0x11
#define DEF_HTTP_TYPE	0x12
#define DEF_VT_TYPE		0x13
#define DEF_FB_TYPE		0x14

/***** COMPLETE, TERMINATION FLAG *****/
#define DEF_COMPLETE        0x01
#define DEF_NONCOMPLETE     0x00

/* Term Flag */
#define DEF_TERM_LIST       		0x01
#define DEF_TERM_TIMER      		0x02
#define DEF_TERM_STOP       		0x03
#define DEF_TERM_CALL       		0x04
#define DEF_TERM_PREPAID       		0x05

/* Term Reason */
#define DEF_TERM_PREPAID_FIRST		1
#define DEF_TERM_PREPAID_FIRST_WAP1	199
#define DEF_TERM_PREPAID_FIRST_WAP2	299
#define DEF_TERM_PREPAID_FIRST_HTTP	399

/* WAP1 */
#define DEF_TERM_FINDPAIR_NEWRES_WAP1	100
#define DEF_TERM_FINDPAIR_TIMEOUT_WAP1	101
#define DEF_TERM_FINDPAIR_TCPSTOP_WAP1	102
#define DEF_TERM_FINDPAIR_CALLSTOP_WAP1	103

#define DEF_TERM_NOTPAIR_TIMEOUT_WAP1  	110 
#define DEF_TERM_NOTPAIR_CALLSTOP_WAP1 	111 

#define DEF_TERM_NOTPAIR_NEWRES_WAP1	112	
#define DEF_TERM_NOTPAIR_TCPSTOP_WAP1	113	

#define DEF_TERM_ONLYREQ_TIMEOUT_WAP1  	120 
#define DEF_TERM_ONLYREQ_CALLSTOP_WAP1 	121 

#define DEF_TERM_ONLYREQ_NEWRES_WAP1	122	
#define DEF_TERM_ONLYREQ_TCPSTOP_WAP1	123	

/* WAP2 */
#define DEF_TERM_FINDPAIR_NEWRES_WAP2	200	
#define DEF_TERM_FINDPAIR_TIMEOUT_WAP2	201	
#define DEF_TERM_FINDPAIR_TCPSTOP_WAP2	202	
#define DEF_TERM_FINDPAIR_CALLSTOP_WAP2	203	

#define DEF_TERM_NOTPAIR_NEWRES_WAP2	210
#define DEF_TERM_NOTPAIR_TIMEOUT_WAP2	211
#define DEF_TERM_NOTPAIR_TCPSTOP_WAP2	212
#define DEF_TERM_NOTPAIR_CALLSTOP_WAP2	213

#define DEF_TERM_ONLYREQ_NEWRES_WAP2	220	
#define DEF_TERM_ONLYREQ_TIMEOUT_WAP2	221	
#define DEF_TERM_ONLYREQ_TCPSTOP_WAP2	222	
#define DEF_TERM_ONLYREQ_CALLSTOP_WAP2	223	

/* HTTP */
#define DEF_TERM_FINDPAIR_NEWRES_HTTP	300
#define DEF_TERM_FINDPAIR_TIMEOUT_HTTP	301
#define DEF_TERM_FINDPAIR_TCPSTOP_HTTP	302
#define DEF_TERM_FINDPAIR_CALLSTOP_HTTP	303

#define DEF_TERM_NOTPAIR_NEWRES_HTTP	310
#define DEF_TERM_NOTPAIR_TIMEOUT_HTTP	311
#define DEF_TERM_NOTPAIR_TCPSTOP_HTTP	312
#define DEF_TERM_NOTPAIR_CALLSTOP_HTTP	313

#define DEF_TERM_ONLYREQ_NEWRES_HTTP	320
#define DEF_TERM_ONLYREQ_TIMEOUT_HTTP	321
#define DEF_TERM_ONLYREQ_TCPSTOP_HTTP	322
#define DEF_TERM_ONLYREQ_CALLSTOP_HTTP	323


/* Added by syhan 20060718 */
#define DEF_TERM_ONLYACK_TIMEOUT_WAP1  	130 
#define DEF_TERM_ONLYACK_CALLSTOP_WAP1 	131 
#define DEF_TERM_ONLYACK_NEWRES_WAP1	132	
#define DEF_TERM_ONLYACK_TCPSTOP_WAP1	133	

#define DEF_TERM_ONLYREQRES_TIMEOUT_WAP1	140	
#define DEF_TERM_ONLYREQRES_CALLSTOP_WAP1	141	
#define DEF_TERM_ONLYREQRES_NEWRES_WAP1		142	
#define DEF_TERM_ONLYREQRES_TCPSTOP_WAP1	143	

#define DEF_TERM_ONLYRESACK_TIMEOUT_WAP1	150
#define DEF_TERM_ONLYRESACK_CALLSTOP_WAP1	151
#define DEF_TERM_ONLYRESACK_NEWRES_WAP1		152
#define DEF_TERM_ONLYRESACK_TCPSTOP_WAP1	153

#define DEF_TERM_ONLYREQACK_TIMEOUT_WAP1	160
#define DEF_TERM_ONLYREQACK_CALLSTOP_WAP1	161
#define DEF_TERM_ONLYREQACK_NEWRES_WAP1		162
#define DEF_TERM_ONLYREQACK_TCPSTOP_WAP1	163

#define DEF_TRANSTAT_COMPLETE		50
#define DEF_TRANSTAT_NOTEQUAL		51


typedef struct _st_TotReqResStat
{
	UINT	uiReqCnt;
	UINT	uiReqRealCnt;
	INT64	llReqByte;
	
	UINT	uiResCnt;
	UINT	uiResRealCnt;
	INT64	llResByte;
}st_TotReqResStat, *pst_TotReqResStat;

typedef struct _st_ReqResStat
{
	UINT	uiReqCnt;
	UINT	uiResCnt;

	INT64	llReqByte;
	INT64	llResByte;
}st_ReqResStat, *pst_ReqResStat;

typedef struct _st_TranInfo_
{
	UINT	uiNewRES;
	UINT	uiTimeout;
	UINT	uiTcpFIN;
	UINT	uiAccStop;
}st_TranInfo, *pst_TranInfo;

typedef struct _st_SvcBlkStat
{
	time_t				tStatTime;			/* 통계 수집 시간 (time_t/300*300) : 5분단위 time_t 값임 */
	UCHAR				ucIPAFID;	
	UCHAR				ucSvcType;	/* 1:ME, 2:KUN, 3:ADS, 4:MARS */
	UCHAR				szReserved[2];
	
	st_TotReqResStat	stTotStat;
	st_ReqResStat		stDropStat;			/* TOTAL REQUEST PACKET DROP DATA */
	st_ReqResStat		stDropSessStat;		/* TCP SESSION NOT-FOUND REQUEST PACKET DROP DATA */
	st_ReqResStat		stDropTranStat;		/* HTTP TRANSACTION NOT-FOUND REQUEST PACKET DROP DATA */
	st_ReqResStat		stDropFailStat;		/* MMDB, LINKED-LIST FAIL DATA */

	/* NEW RETRAN DATA */
	st_ReqResStat		stRetranStat;
	
	UINT				uiHTTPTranCnt;		/* 전체 HTTP TRANSACTION 개수 */
	UINT				uiHTTPCompleteCnt;	/* 정상적으로 처리된 TRANSACTION 개수 */
											/* RESPONSE도 있고, LENGTH정보도 일치하는 경우 */
	UINT				uiNotEqualLenCnt;	/* RESPONSE의 CONTENT LENGTH가 헤더 정보와 일치하지 않는 경우의 개수  */
	UINT				uiNotEqualLenByte;	/* RESPONSE의 CONTENT LENGTH가 헤더 정보와 일치하지 않는 경우의 Bytes */

	st_TranInfo			stMatchedTran;		/* HTTP TRANSACTION이 이루어진 경우에 대한 통계 */
	st_TranInfo			stOnlyRESTran;		/* RESPONSE만으로 이루어진 TRANSACTION에 대한 통계 */
	st_TranInfo			stOnlyREQTran;		/* REQUEST만으로 이루어진 TRANSACTION에 대한 통계 */
	
	UINT				uiGETMethodCnt;
	UINT				uiPOSTMethodCnt;
	UINT				uiCONNECTMethodCnt;
	UINT				uiETCMethodCnt;
}st_SvcBlkStat, *pst_SvcBlkStat;


typedef struct _st_WapSvcBlkStat
{
	time_t				tStatTime;			/* 통계 수집 시간 (time_t/300*300) : 5분단위 time_t 값임 */
	UCHAR				ucIPAFID;	
	UCHAR				ucSvcType;	/* 1:ME, 2:KUN, 3:ADS, 4:MARS */
	UCHAR				szReserved[2];
	
	st_TotReqResStat	stTotStat;
	st_ReqResStat		stDropStat;			/* TOTAL REQUEST PACKET DROP DATA */
	st_ReqResStat		stDropSessStat;		/* TCP SESSION NOT-FOUND REQUEST PACKET DROP DATA */
	st_ReqResStat		stDropTranStat;		/* HTTP TRANSACTION NOT-FOUND REQUEST PACKET DROP DATA */
	st_ReqResStat		stDropFailStat;		/* MMDB, LINKED-LIST FAIL DATA */

	/* NEW RETRAN DATA */
    st_ReqResStat       stRetranStat;
	
	UINT				uiWAPTranCnt;		/* 전체 HTTP TRANSACTION 개수 */
	UINT				uiWAPCompleteCnt;	/* 정상적으로 처리된 TRANSACTION 개수 */
											/* RESPONSE도 있고, LENGTH정보도 일치하는 경우 */
	UINT				uiNotEqualLenCnt;	/* RESPONSE의 CONTENT LENGTH가 헤더 정보와 일치하지 않는 경우의 개수  */
	UINT				uiNotEqualLenByte;	/* RESPONSE의 CONTENT LENGTH가 헤더 정보와 일치하지 않는 경우의 Bytes */

	st_TranInfo			stMatchedTran;		/* HTTP TRANSACTION이 이루어진 경우에 대한 통계 */
	st_TranInfo			stOnlyRESTran;		/* RESPONSE만으로 이루어진 TRANSACTION에 대한 통계 */
	st_TranInfo			stOnlyREQTran;		/* REQUEST만으로 이루어진 TRANSACTION에 대한 통계 */
	st_TranInfo			stOnlyACKTran;		/* ACK 만으로 이루어진 TRANSACTION에 대한 통계 */
	st_TranInfo			stOnlyREQACKTran;	/* REQUEST, ACK 만으로 이루어진 TRANSACTION에 대한 통계 */
	st_TranInfo			stOnlyREQRESTran;	/* REQUEST, RESPONSE 만으로 이루어진 TRANSACTION에 대한 통계 */
	st_TranInfo			stOnlyRESACKTran;	/* RESUEST, ACK만 있는 경우에 대한 통계 */
	
	UINT				uiGETMethodCnt;
	UINT				uiPOSTMethodCnt;
	UINT				uiCONNECTMethodCnt;
	UINT				uiETCMethodCnt;
}st_WapSvcBlkStat, *pst_WapSvcBlkStat;


typedef struct _stRTSPStat
{
	time_t              tStatTime;          /* 통계 수집 시간 (time_t/300*300) : 5분단위 time_t 값임 */
    UCHAR               ucIPAFID;
    UCHAR               ucSvcType;  /* 1:ME, 2:KUN, 3:ADS, 4:MARS */
    UCHAR               szReserved[2];

	/* RTSP STAT */
    st_TotReqResStat    stTotStat;
    st_ReqResStat       stDropStat;         /* TOTAL REQUEST PACKET DROP DATA */
    st_ReqResStat       stDropSessStat;     /* TCP SESSION NOT-FOUND REQUEST PACKET DROP DATA */
    st_ReqResStat       stDropTranStat;     /* HTTP TRANSACTION NOT-FOUND REQUEST PACKET DROP DATA */
    st_ReqResStat       stDropFailStat;     /* MMDB, LINKED-LIST FAIL DATA */

	st_ReqResStat		stRetranStat;		/* RETRANSMISSION DATA */

    UINT                uiHTTPTranCnt;      /* 전체 HTTP TRANSACTION 개수 */
	UINT				uiReserved1;

    st_TranInfo         stMatchedTran;      /* HTTP TRANSACTION이 이루어진 경우에 대한 통계 */
    st_TranInfo         stOnlyRESTran;      /* RESPONSE만으로 이루어진 TRANSACTION에 대한 통계 */
    st_TranInfo         stOnlyREQTran;      /* REQUEST만으로 이루어진 TRANSACTION에 대한 통계 */

	UINT				uiDESCRIBEMethodCnt;
	UINT				uiSETUPMethodCnt;
	UINT				uiPLAYMethodCnt;
	UINT				uiTEARDOWNMethodCnt;
	UINT				uiPAUSEMethodCnt;
	UINT				uiOPTIONSMethodCnt;
    UINT				uiANNOUNCEMethodCnt;
	UINT				uiETCMethodCnt;

	/* RTP/RTCP STAT */
	st_ReqResStat		stUDPStat;
} st_RTSPStat, *pst_RTSPStat;	


typedef struct _stSIPStat
{   
	time_t              tStatTime; 
	UCHAR               ucIPAFID;
	UCHAR               ucSvcType;  /* 1:ME, 2:KUN, 3:ADS, 4:MARS */
	UCHAR               szReserved[2];
	
	UINT                uiBYEMethodCnt;     
	UINT                uiINVITEMethodCnt;  

	UINT                uiNOTIFYMethodCnt;  
	UINT                uiMESSAGEMethodCnt; 

	UINT                uiPUBLISHMethodCnt; 
	UINT                uiREGISTERMethodCnt;

	UINT                uiSUBSCRIBEMethodCnt;
	UINT                uiUPDATEMethodCnt;  

	UINT                uiETCMethodCnt;     
	UINT                uiCallStartCnt;

	UINT				uiTermByeMethodCnt;
	UINT				uiTermCallStopCnt;
	UINT				uiTermTimeOutCnt;
	UINT				uiTermCallIDCnt;

	UINT				uiOtherNetCnt;
	UINT				uiReserved;

	UINT                uiTotAudioCnt;
	UINT                uiTotAudioBytes;
	UINT                uiTotVideoCnt;
	UINT                uiTotVideoBytes;

} st_SIPStat, *pst_SIPStat;


typedef struct _stMultiPackStat
{
    time_t              tStatTime;          /* 통계 수집 시간 (time_t/300*300) : 5분단위 time_t 값임 */
    UCHAR               ucIPAFID;
    UCHAR               ucSvcType;  /* 1:ME, 2:KUN, 3:ADS, 4:MARS */
    UCHAR               szReserved[2];

    st_TotReqResStat    stTotStat;
    st_ReqResStat       stFailStat;     /* MMDB, LINKED-LIST FAIL DATA */
    st_ReqResStat       stRetransStat;  /* MMDB, LINKED-LIST FAIL DATA */

	UINT				uiUpHeadCnt;
	UINT				uiDownHeadCnt;
	UINT				uiNormalRDRCnt;	
	UINT				uiAbnormalRDRCnt;
	
} st_MultiPackStat, *pst_MultiPackStat;

typedef struct _st_KVM_Stat                                                                            
{                                                                                                          
    time_t              tStatTime;
    UCHAR               ucIPAFID;                                                                          
    UCHAR               ucSvcType;
    UCHAR               szReserved[2];                                                                     

    st_TotReqResStat    stTotStat;                                                                         
    st_ReqResStat       stFailStat;
    st_ReqResStat       stRetranStat;

} st_KVMStat, *pst_KVMStat;  

/*
* DEF_SERVER : 1 , DEF_CLIENT : 2
*/
#define DEF_STAT_REAL		3
#define DEF_STAT_HTTPTRAN	4	
#define DEF_STAT_METHOD		5	
#define DEF_STAT_TERM		6
#define DEF_STAT_START		7
#define DEF_STAT_OTHER		8


#define DEF_STAT_TOTAL		0x01
#define DEF_STAT_DROPSESS	0x02
#define DEF_STAT_DROPTRAN	0x03
#define DEF_STAT_DROPFAIL	0x04

#define DEF_STAT_HEAD		0x06
#define DEF_STAT_FAIL		0x07

#define DEF_STAT_RETRAN		0x08

#define DEF_STAT_RDR		6

#define DEF_SET_CNT	2
#define DEF_STAT_SET_CNT	2
#define DEF_STAT_UNIT		300
#define CPS_UNIT			5
#define CALL_UNIT			5

#define DEF_NORMAL_RDR		2
#define DEF_ABNORMAL_RDR	3

/*
* WAP2ANA, HTTPANA, FBANA
*/
typedef struct _st_SvcBlkStat_List_
{
	st_SvcBlkStat		stSvcBlkStat[12];
} st_SvcBlkStat_List, *pst_SvcBlkStat_List;

typedef struct _st_SvcBlkStat_MP {
	st_SvcBlkStat_List stStatList[MAX_MP_NUM2];
} st_SvcBlkStat_MP;

/*
* WAP1ANA
*/
typedef struct _st_WapSvcBlkStat_List_
{
	st_WapSvcBlkStat		stWapSvcBlkStat[12];
} st_WapSvcBlkStat_List, *pst_WapSvcBlkStat_List;

typedef struct _st_WapSvcBlkStat_MP
{
	st_WapSvcBlkStat_List		stStatList[MAX_MP_NUM2];
} st_WapSvcBlkStat_MP, *pst_WapSvcBlkStat_MP;

/*
* VODSANA
*/
typedef struct _st_RTSPStat_List_
{
	st_RTSPStat			stRTSPStat[12];
} st_RTSPStat_List, *pst_RTSPStat_List;

typedef struct _st_RTSPStat_MP
{
	st_RTSPStat_List	stStatList[MAX_MP_NUM2];
} st_RTSPStat_MP, *pst_RTSPStat_MP;

/*
* WIPINWANA
*/
typedef struct _st_MultiPackStat_List_
{
    st_MultiPackStat stMultiPackStat[12];
} st_MultiPackStat_List, *pst_MultiPack_List;

typedef struct _st_MultiPackStat_MP
{
    st_MultiPackStat_List stStatList[MAX_MP_NUM2];
} st_MultiPackStat_MP, *pst_MultiPack_MP;

/*
* JAVANWANA
*/
typedef struct _st_KVMStat_List_
{
    st_KVMStat stKVMStat[12];
} st_KVMStat_List;

typedef struct _st_KVMStat_MP
{
    st_KVMStat_List stStatList[MAX_MP_NUM2];
} st_KVMStat_MP;


/*
* RADIUS
*/
typedef struct _st_RadiusStat_List_
{
	st_ACCESS_Stat		stAuthStat[12];
	st_ACCOUNTING_Stat	stAcctStat[12];
} st_RadiusStat_List, *pst_RadiusStat_List;


/*
* VTANA
*/
typedef struct _st_SIPStat_List_
{
	st_SIPStat      stSIPStat[12];
} st_SIPStat_List, *pst_SIPStat_List;

typedef struct _st_SIPStat_MP
{
	st_SIPStat_List stStatList[MAX_MP_NUM2];
} st_SIPStat_MP, *pst_SIPStat_MP;


#define DEF_PDSN_CNT	32
typedef struct _st_PDSN_RadiusStat_List_
{
	UINT	uiCurCnt;
	UINT	uiReserved;
	UINT	uiPDSNAddr[DEF_PDSN_CNT];
		
	st_RadiusStat_List	rad_stat[MAX_MP_NUM1][DEF_PDSN_CNT];
} st_PDSN_RadiusStat_List, *pst_PDSN_RadiusStat_List;

#define DEF_SVCSTAT_SIZE    sizeof(st_SvcBlkStat)   /* DEF_SVCSTAT_SIZE = 112 */
                                                    /* DEF_SVCSTAT_SIZE*12 = 1344 */
#define DEF_VODSTAT_SIZE    sizeof(st_RTSPStat)
#define DEF_MULTISTAT_SIZE  sizeof(st_MultiPackStat)


typedef struct _st_SessAnaStat
{
	time_t			tStatTime;	/* 통계 수집 시간 (time_t/300*300) : 5분단위 time_t 값임 */
	UCHAR			ucIPAFID;	
	UCHAR			ucSvcType;	/* 0 : SESSANA */
	UCHAR			szReserved[2];

	
	st_ReqResStat		stTotStat;		/* TOTAL PACKET */
	st_ReqResStat		stDropSerCat;		/* 서비스군에 포함되지 않은 PACKET */
	st_ReqResStat		stDropCallSess;		/* CALL-SESSION이 시작되지 않아 버린 PACKET */
	st_ReqResStat		stDropReTran;		/* 재전송으로 버린 PACKET */
	st_ReqResStat		stDropETC;		/* 기타 에러로 인해 버린 PACKET */
}st_SessAnaStat, *pst_SessAnaStat;

typedef struct _st_SessAnaStatList
{
	st_SessAnaStat		stSessAnaStat[12];
} st_SessAnaStatList, *pst_SessAnaStatList;

typedef struct _st_SessAnaStat_MP {
	st_SessAnaStatList stStatList[MAX_MP_NUM1];
} st_SessAnaStat_MP;

#define DEF_SESSSTAT_SIZE	sizeof(st_SessAnaStat)	/* DEF_SESSSTAT_SIZE */

/* SESSION STAT ERROR DEFINE */
#define DEF_ALL					0x01
#define DEF_DROP_SVC_CAT		0x02
#define DEF_DROP_CALL_SESS		0x03
#define DEF_DROP_RE_TRAN		0x04
#define DEF_DROP_ETC			0x05


#define MAX_RDRSEQ_LIST_CNT		150002

typedef struct _st_RDRSeqNode_
{
	int     dListCount;
	int     dReserved;

	int		next;
	int		prev;
} st_RDRSeqNode, *pst_RDRSeqNode; 

typedef struct _st_RDRSeqList_
{
	int		_dFreeLinkedList;
	int		_dUsedNode_First;
	int		_dCurNodeCnt;
	int		_dReserved;
	
	st_RDRSeqNode	stRDRSeqList[MAX_RDRSEQ_LIST_CNT];
} st_RDRSeqList, *pst_RDRSeqList;
#define DEF_RDRSEQLIST_SIZE		sizeof(st_RDRSeqList)

/** 5분 단위 통계 **************************************************************************/
#define MAX_RLEG_CNT		1
/*** START of ACCOUNT STATISTIC ***/
/* 2009.06.10 by dhkim */
typedef struct _st_pdsn_statistic_ {
	unsigned int	uiPDSN_IP;
	unsigned int	uiPDSN_RecvCnt;     // PDSN 별 ACCOUNT-REQUEST를 받은 개수 
	unsigned int	uiPDSN_StartCnt;    // PDSN 별 ACCOUNT-REQUEST START를 받은 개수 
	unsigned int	uiPDSN_InterimCnt;  // PDSN 별 ACCOUNT-REQUEST INTERIM를 받은 개수 
	unsigned int	uiPDSN_StopCnt;     // PDSN 별 ACCOUNT-REQUEST STOP를 받은 개수 
	unsigned int	uiPDSN_DiscReqCnt;  // PDSN 별 DISCONNECT-REQUEST를 받은 개수 
	unsigned int	uiLogOn_StartCnt;   // PDSN 별 ACCOUNT-REQUEST (START)로 인한 LOGON 시도수 
	unsigned int	uiLogOn_InterimCnt; // PDSN 별 ACCOUNT-REQUEST (INTERIM)로 인한 LOGON 시도수
	unsigned int	uiLogOn_StopCnt;    // PDSN 별 ACCOUNT-REQUEST (STOP)로 인한 LOGOUT 시도수 
	unsigned int	uiLogOn_DiscReqCnt; // PDSN 별 DISCONNECT-REQUEST로 인한 LOGON 시도수 
} PDSN_STAT_t;
#define DEF_PDSN_STAT_SIZE sizeof(PDSN_STAT_t)

typedef struct _st_leg_stat_ {
	unsigned int	uiPDSN_Cnt;
	PDSN_STAT_t		staPDSN_Stat[DEF_PDSN_CNT];
} LEG_STAT_t;
#define DEF_LEG_STAT_SIZE sizeof(LEG_STAT_t)
/*** END of ACCOUNT STATISTIC ***/

#define MAX_HBIT_STAT_CNT				32		// LOGON STATISTIC 에 사용.
#define DEF_LOG_ON			0
#define DEF_LOG_OUT			1
#define DEF_LOG_OUT_MMC		2
/*** START of LOGON STATISTIC ***/
typedef struct _st_logon_stat_ {
	unsigned int	uiSMIndex;			// RLEG 구분자 . [0, 1, 2, 3, 4]
	unsigned int	uiLogMode;			// LogOn 인지 LogOut 인지 구분하는 0 : On, 1 : Out
	unsigned int	uiLogOn_Request;	// SM에 대해 LOGON+LOGOUT 요청수의 합.
	unsigned int	uiLogOn_Success;	// SM에 대해 LOGON+LOGOUT 요청 성공수의 합.
	unsigned int	uiLogOn_Fail;		// SM에 대해 LOGON+LOGOUT 요청 실패수의 합.
	unsigned int	uiLogOn_Reason1;	// SM internal error 발생수.
	unsigned int	uiLogOn_Reason2;	// SM internal error 외에 error 발생수.
	unsigned int	uiLogOn_Reason3;	// SM 으로 요청한 LOGON+LOGOUT에 대한 timeout 발생수.
	unsigned int	uiLogOn_Reason4;	// 기타 LOGON+LOGOUT 처리 오류 발생수.
	unsigned int	uiLogOn_APIReqErr;
	unsigned int	uiLogOn_APITimeout;
	unsigned int	uiLogOn_HBIT[MAX_HBIT_STAT_CNT];
										// P BIT=1 & H BIT=0인 가입자에 대한 LOGON 성공수.
										// ADD, BY JUNE, 2010-08-22, HBIT 변경(1,2,3,etc -> 32)
} LOGON_STAT_t;
#define DEF_LOGON_STAT_SIZE sizeof(LOGON_STAT_t)
/*** END of LOGON STATISTIC ***/

#define LOG_MODE_CNT 2
typedef struct _st_leg_tot_stat_ {
	char			szSysName[8];		
	LEG_STAT_t		stAcct;
	LOGON_STAT_t	stLogon[MAX_RLEG_CNT][LOG_MODE_CNT];
} LEG_TOT_STAT_t;
#define DEF_LEG_TOT_STAT_SIZE sizeof(LEG_TOT_STAT_t)

/** 5초 단위 CPS report *****************************************/
typedef struct _st_leg_sum_cps_ {
	unsigned int uiLogOnSumCps;
	unsigned int uiLogOutSumCps;
} LEG_SUM_CPS;
#define DEF_LEG_SUM_CPS_SIZE sizeof(LEG_SUM_CPS)

typedef struct _st_leg_sess_data_ {
	unsigned int 	uiAmount;
} LEG_SESS_DATA;
#define DEF_LEG_SESS_DATA_SIZE sizeof(LEG_SESS_DATA)

#define CONNECTED		1
#define DISCONNECTED	0
typedef struct _st_leg_sm_conn {
	unsigned int	dConn; // SM 연결 상태 
} LEG_SM_CONN;
#define DEF_LEG_SESS_DATA_SIZE sizeof(LEG_SESS_DATA)

typedef struct _st_call_data_{
	LEG_SUM_CPS     cps;
	LEG_SESS_DATA   sess;
	unsigned int	tps;
} LEG_CALL_DATA;
#define DEF_LEG_CALL_DATA_SIZE sizeof(LEG_CALL_DATA)

typedef struct _st_leg_data_sum_{
	unsigned int	uiTPS;
	LEG_SUM_CPS     cps[MAX_RLEG_CNT+1];
} LEG_DATA_SUM;
#define DEF_LEG_DATA_SUM_SIZE sizeof(LEG_DATA_SUM)
/** 5초 단위 CPS report *****************************************/

// functions
// RDRLIST_LIB
int Init_RDRSEQ_LIST( int key );
pst_RDRSeqNode dGetFirstNode_RDRSEQ();
void free_node_RDRSEQ(pst_RDRSeqNode node);
pst_RDRSeqNode get_node_RDRSEQ(int *cid);
void init_linked_list_RDRSEQ();
pst_RDRSeqNode set_node_RDRSEQ(int cid);

#pragma pack()
#endif
