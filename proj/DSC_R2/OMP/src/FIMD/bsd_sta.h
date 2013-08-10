#ifndef __BSD_STATISTIC_H__
#define __BSD_STATISTIC_H__

#include <udrgen_stat.h>
#include <udrgen_stat_svctype.h>

#define MAX_STAT_SIZE   12
#define MAX_INFSTAT_COUNT       15  // hskim, 10 -> 15
#define MAX_FAILREASON_COUNT    19  // sunny, 15 -> 19
#define MAX_STATTIME_COUNT      12
#define MAX_TRAN_TYPE_LEN   16
#define SVC_TR_ROW_CNT      77
#define SVC_WAP1_ROW_CNT    35
#define SVC_WAP2_ROW_CNT    46
#define SVC_HTTP_ROW_CNT    46
#define SVC_JAVA_ROW_CNT    12
#define SVC_VODS_ROW_CNT    51
#define SVC_WIPI_ROW_CNT    16

typedef unsigned char   UCHAR;
typedef short           SHORT;
typedef unsigned short  USHORT;
typedef int             INT;
typedef unsigned int    UINT;
typedef long long       INT64;
typedef long            LONG;
typedef unsigned long   ULONG;

typedef struct {
    time_t          KeyTime;
    int             Reserved;
 
    unsigned int    uiUpFrames;
    unsigned int    uiDownFrames;
    long long    llUpBytes;
    long long    llDownBytes;
} st_UpDownStat;
 
typedef struct {
    char    upinf;
    char    downinf;
    char    DebugLevel;
    char    CDRLevel;
    char    IPAFID;
    char    reserved[3];
 
    int     IdleTime;
    int     RetryTime;
 
 /* 이후 정보만 IPAF관련 통계 정보 */
 /* IPAF 통계 */
    st_UpDownStat   ThruStat[MAX_STAT_SIZE];
 
    st_UpDownStat   TotStat[MAX_STAT_SIZE];
    st_UpDownStat   IPStat[MAX_STAT_SIZE];
    st_UpDownStat   UDPStat[MAX_STAT_SIZE];
    st_UpDownStat   TCPStat[MAX_STAT_SIZE];
 
    st_UpDownStat   IPError[MAX_STAT_SIZE];
    st_UpDownStat   UTCPError[MAX_STAT_SIZE];
    st_UpDownStat   TCPReTrans[MAX_STAT_SIZE];
 
    st_UpDownStat   OutOfIP[MAX_STAT_SIZE];
    st_UpDownStat   DropData[MAX_STAT_SIZE];
    st_UpDownStat   FailData[MAX_STAT_SIZE];
} T_GENINFO;

typedef struct 
{
    /* 이후 정보만 IPAF관련 통계 정보 */
 /* IPAF 통계 */
 //   long long IPAFID;
    st_UpDownStat   ThruStat;
 
    st_UpDownStat   TotStat;
    st_UpDownStat   IPStat;
    st_UpDownStat   UDPStat;
    st_UpDownStat   TCPStat;
 
    st_UpDownStat   IPError;
    st_UpDownStat   UTCPError;
    st_UpDownStat   TCPReTrans;
 
    st_UpDownStat   OutOfIP;
    st_UpDownStat   DropData;
    st_UpDownStat   FailData;
    st_UpDownStat   FilterOut;
}STM_IPAFStatisticMsgType; 

//////////////////////////////////////////////////////////
// 서비스 전송계층 통계 svc_tr
typedef struct _st_TotReqResStat
{
    UINT    uiReqCnt;
    UINT    uiReqRealCnt;
    UINT    uiReqByte;
   
    UINT    uiResCnt;
    UINT    uiResRealCnt;
    UINT    uiResByte;
}st_TotReqResStat, *pst_TotReqResStat;
 
typedef struct _st_ReqResStat
{
    UINT    uiReqCnt;
    UINT    uiReqByte;
   
    UINT    uiResCnt;
    UINT    uiResByte;
}st_ReqResStat, *pst_ReqResStat;
 
typedef struct _st_TranInfo_
{
    UINT    uiNewRES;
    UINT    uiTimeout;
    UINT    uiTcpFIN;
    UINT    uiAccStop;
}st_TranInfo, *pst_TranInfo;

typedef struct _st_SvcBlkStat
{
    time_t              tStatTime;
    UCHAR               ucIPAFID;
    UCHAR               ucSvcType;  
    UCHAR               szReserved[2];
   
    st_TotReqResStat    stTotStat;
    st_ReqResStat       stDropStat; 
    st_ReqResStat       stDropSessStat;
    st_ReqResStat       stDropTranStat;
    st_ReqResStat       stDropFailStat;
   
    UINT                uiHTTPTranCnt;
    UINT                uiHTTPCompleteCnt;                                            
    UINT                uiNotEqualLenCnt;
    UINT                uiNotEqualLenByte;
 
    st_TranInfo         stMatchedTran;
    st_TranInfo         stOnlyRESTran;
    st_TranInfo         stOnlyREQTran;

    st_TranInfo         stOnlyACKTran;       // ACK 만 있는 경우 
    st_TranInfo         stOnlyREQACKTran;    // REQ-ACK 만 있는 경우 : RES 가 없음
    st_TranInfo         stOnlyREQRESTran;    // REQ-RES 만 있는 경우 : ACK가 없음
    st_TranInfo         stOnlyRESACKTran;    // RES-ACK 만 있는 경우 : REQ 가 없음.
 

    UINT                uiGETMethodCnt;
    UINT                uiPOSTMethodCnt;
    UINT                uiCONNECTMethodCnt;
    UINT                uiETCMethodCnt;

    
}st_SvcBlkStat, *pst_SvcBlkStat;

typedef struct _st_SvcBlkStat_List_
{
    st_SvcBlkStat       stSvcBlkStat;
} st_SvcBlkStat_List, *pst_SvcBlkStat_List;

typedef struct
{
    unsigned char       tranType[MAX_TRAN_TYPE_LEN];

    st_TotReqResStat    stTotStat;
    st_ReqResStat       stDropStat;
    st_ReqResStat       stDropSessStat;
    st_ReqResStat       stDropTranStat;
    st_ReqResStat       stDropFailStat;
    st_ReqResStat       stRetranStat;

    UINT                uiHTTPTranCnt;
    UINT                uiHTTPCompleteCnt;
    UINT                uiNotEqualLenCnt;
    UINT                uiNotEqualLenByte;

    st_TranInfo         stMatchedTran;
    st_TranInfo         stOnlyRESTran;
    st_TranInfo         stOnlyREQTran;

    st_TranInfo         stOnlyACKTran;       // ACK 만 있는 경우
    st_TranInfo         stOnlyREQACKTran;    // REQ-ACK 만 있는 경우 : RES 가 없음
    st_TranInfo         stOnlyREQRESTran;    // REQ-RES 만 있는 경우 : ACK가 없음
    st_TranInfo         stOnlyRESACKTran;    // RES-ACK 만 있는 경우 : REQ 가 없음.


    UINT                uiGETMethodCnt;
    UINT                uiPOSTMethodCnt;
    UINT                uiCONNECTMethodCnt;
    UINT                uiDESCMethodCnt;
    
    UINT                uiSETUPMethodCnt;
    UINT                uiPLAYMethodCnt;
    UINT                uiTEARDOWNMethodCnt;
    UINT                uiPAUSEMethodCnt;
    
    UINT                uiOPTIONMethodCnt;
    UINT                uiANNOUNCEMethodCnt;
    UINT                uiUPHeadMethodCnt;
    UINT                uiDownHeadMethodCnt;
    
    UINT                uiNormUDRCnt;
    UINT                uiAbnormUDRCnt;
    UINT                uiETCMethodCnt;
    UINT                uiUDPUPCnt;
    
    UINT                uiUDPUPBytes;
    UINT                uiUDPDownCnt;
    UINT                uiUDPDownBytes;
    
}STM_SVC_TRStatisticMsgType;

///////////////////////////////////////////////
// 서비스별 계층 통계.
typedef struct _st_SessAnaStat
{
    time_t          tStatTime;
    UCHAR           ucIPAFID;
    UCHAR           ucSvcType;
    UCHAR           szReserved[2];
  
    st_ReqResStat       stTotStat;
    st_ReqResStat       stDropSerCat;
    st_ReqResStat       stDropCallSess;
    st_ReqResStat       stDropReTran;
    st_ReqResStat       stDropETC;
}st_SessAnaStat, *pst_SessAnaStat;

typedef struct
{
    st_ReqResStat       stTotStat;
    st_ReqResStat       stDropSerCat;
    st_ReqResStat       stDropCallSess;
    st_ReqResStat       stDropReTran;
    st_ReqResStat       stDropETC;
}SessAnaStatMsgType;
 
typedef struct _st_SessAnaStatList
{
    st_SessAnaStat      stSessAnaStat[12];
} st_SessAnaStatList, *pst_SessAnaStatList;

////////////////////////////////////////////////
//AAAIF 통계 72번 PTAS_R2.4.0/INC/ 디렉토리의 ipam_stat.h에 정의된 내용을 참조하시면 됩니다.
#if 0 //jean delete

typedef struct _st_RowStat      // sunny, USHORT -> UINT
{
    UINT            usReq;      // 1번 메시지 Request 개수
    UINT            usRep;      // 1번 메시지 Reply 개수
    UINT            usSucc;     // 1번 메시지 Success 개수
    UINT            usFail;     // 1번 메시지 Fail 개수
 
    UINT            usFailCount;
    UINT            usFailReason[MAX_FAILREASON_COUNT]; // Fail Reason [0~19] 까지 정의해서 사용
}st_RowStat, *pst_RowStat;
 
typedef struct _st_MsgStat
{
    UCHAR           ucSysNo;        // 해당 통계 블럭 시스템 아이디
    UCHAR           szReserved[5];
    USHORT          usRowCount;
 
    UINT            uiTotCnt;       // 메시지 총 개수
    UINT            uiReqCnt;       // Request Msg 개수
    UINT            uiRepCnt;       // Reply Msg 개수
    UINT            uiSysIP;
 
    UCHAR           ucInxStart;     //add hskim 20041206
    UCHAR           ucInxBreak;     //add hskim 20041206
    UCHAR           ucIsSum;        //add hskim 20041206
    UCHAR           reserv[5];
 
    st_RowStat      stRowStat[MAX_INFSTAT_COUNT];   // 메세지 최대 15개 까지
}st_MsgStat, *pst_MsgStat;
 
typedef struct _st_InfStat
{
    time_t          tRegTime;       // 통계 등록 시간
    int             dCount;
    st_MsgStat      stMsgStat[32];  // system num
}st_InfStat, *pst_InfStat;
 

typedef struct _st_InfStatList
{
    st_InfStat  stInfStat[MAX_STATTIME_COUNT];
}st_InfStatList, *pst_InfStatList;

#endif //jean delete end

typedef struct _st_FailReason
{
    UINT           usTimeOut;
    UINT           usRetrans;
    UINT           usNetErr;
    UINT           usEtcErr;
}st_FailReason, *pst_FailReason;


//전송 메세지에 대한 통계 구조 
typedef struct _st_RowStat      
{
    UINT            uiReq;      
    UINT            uiRep;         
    UINT            uiFail;
    UINT            uiTotStart;
    UINT            uiTotInterim;
    UINT            uiTotEnd;
    UINT            uiTotTimeOut;       // TotTimeOut = start+interim+end Timeout

    UINT            uiReserved;

/*    st_FailReason   stFailReason; *//* revised. 2006.08.12 */

    st_FailReason   stStartFailReason;
    st_FailReason   stInterimFailReason;
    st_FailReason   stEndFailReason;
} st_RowStat, *pst_RowStat;

//해당 AAA에 대한 전송 정보
typedef struct _st_MsgStat
{   
    UCHAR           ucSysNo;
    UINT            uiSysIP;      //해당 AAA IP 정보     
    st_RowStat      stRowStat;
  
}st_MsgStat, *pst_MsgStat;

typedef struct _st_InfStat
{   
    time_t           tRegTime;       // 통계등록시간
    int              dCount;         // AAA시스템 개수에 해당:32
    st_MsgStat      stMsgStat[32];  // 각 AAA에 대한 연동 통계 정보를 가짐

}st_InfStat, *pst_InfStat;


typedef struct _st_InfStatList
{   
    st_InfStat  stInfStat[MAX_STATTIME_COUNT]; //MAXMAX_STATTIME_COUNT:12
} st_InfStatList, *pst_InfStatList; 


typedef struct 
{
    UINT    uiSysIP;
    UCHAR   ucSysNo; // 1:AAA 2:AN_AAA by helca 0104

    st_RowStat stRowSet;   
        
}st_InfStatType;

typedef struct
{
    st_InfStatType  stInfType[32];
   
}STM_AAAStatisticMsgType;


/* FAIL UDR Statistics by helca 2007.05.31 */
typedef struct
{
    UINT        uiSysIP;
    UCHAR       ucSysNo; // 1: AAA 2: AN_AAA
    st_RowStat  stRowSet;
}STM_FailUdrsubtype;

typedef struct{
    STM_FailUdrsubtype  stInfType[32];
} STM_FailUdrStatisticMsgType;



////////////////////////////////////////////////


//jean add
#ifndef MAX_WAPGW_NUM
#define MAX_WAPGW_NUM 3
#endif

#ifndef MAX_IPADDR_LEN
#define MAX_IPADDR_LEN 16
#endif


typedef struct{
    unsigned int    iCnt;
    unsigned int    iCompleteCnt;
    unsigned int    iTimeoutCnt;
    unsigned int    iCancelledCnt;
    unsigned int    iUploadSize;
    unsigned int    iDownloadSize;
    unsigned int    iGetMethodCnt;
    unsigned int    iPostMethodCnt;
    unsigned int    iEtcMethodCnt;
} st_WapSuccStat_t, *pst_WapSuccStat_t;


typedef struct{
    unsigned int        iSessErrCnt;
    unsigned int        iUDRErrCnt;
    unsigned int        iEtcErrCnt;
    unsigned int        iSessErrUpSize;
    unsigned int        iSessErrDownSize;
    unsigned int        iEtcErrUpSize;
    unsigned int        iEtcErrDownSize;
} st_WapFailStat_t, *pst_WapFailStat_t;


typedef struct{
    unsigned int        iCnt;
    unsigned int        iDiscardCnt;
    st_WapSuccStat_t    st_Succ;
    st_WapFailStat_t    st_Fail;
} st_WapCnt_t, *pst_WapCnt_t;


typedef struct _st_WapGwStat_t{
    unsigned char   sIpAddress[MAX_IPADDR_LEN];
    st_WapCnt_t     st_Wap;
    unsigned int    iDecErrCnt;
} st_WapGwStat_t, *pst_WapGwStat_t;

typedef struct _st_UaWapStat_t{
    st_WapGwStat_t GwStat[MAX_WAPGW_NUM];
} st_UAWAP_STAT_t, *pst_UAWAP_STAT_t;

typedef struct {
    st_WapGwStat_t GwStat[MAX_WAPGW_NUM];
} STM_UAWAPStatMsgType;

///////////////////////////////////////////////
typedef struct _st_ACCESS_Stat_
{
    UINT      usTotAuthReq;
    UINT      usTotAuthRes;
    UINT      usAuthTimeout;
    UINT      usTotAuthAcpt;
    UINT      usTotAuthRjt;
    UINT      usNoIterimAuthAcpt;
    UINT      usTotCmplAuthTxn;
    UINT      usOnlyAuthAcpt;
    UINT      usOnlyAuthRjt;
    UINT      usDecErrAuthReq;
    UINT      usDecErrAuthRes;
    UINT      usDupAuthReq;
    UINT      usEtcErrAuthReq;
    UINT      usEtcErrAuthRes;
} st_ACCESS_Stat, *pst_ACCESS_Stat;
 
typedef struct _st_ACCOUNTING_Stat_
{
    UINT      usTotAcctReq;
    UINT      usTotAcctRes;
    UINT      usTotAcctTimeout;
    UINT      usTotStart;
    UINT      usTotInterim;
    UINT      usTotStop;
    UINT      usSessContinue;
    UINT      usMDNAcctRes;
    UINT      usTotCmplAcctTxn;
    UINT      usOnlyAcctReq;
    UINT      usOnlyAcctRes;
    UINT      usDecErrAcctReq;
    UINT      usDecErrAcctRes;
    UINT      usDupAcctReq;
    UINT      usEtcErrAuthReq;
    UINT      usEtcErrAuthRes;
} st_ACCOUNTING_Stat, *pst_ACCOUNTING_Stat;

typedef struct _st_RadiusStat_List_
{
    st_ACCESS_Stat      stAuthStat[12];
    st_ACCOUNTING_Stat  stAcctStat[12];
} st_RadiusStat_List, *pst_RadiusStat_List;

typedef struct
{
    st_ACCESS_Stat      stAuthStat;
    st_ACCOUNTING_Stat  stAcctStat;
} STM_RadiusStatMsgType;

#define DEF_PDSN_CNT  32 
typedef struct 
{
    UINT    uiCurCnt;
    UINT    uiReserved;
    UINT    uiPDSNAddr[DEF_PDSN_CNT];
    STM_RadiusStatMsgType  rad_stat[DEF_PDSN_CNT];
} PDSN_RadiusStatMsg;

//////////////////////////////////////////////

typedef struct
{
    char type[MAX_TRAN_TYPE_LEN];
    unsigned int res[58];
} TmpResBuf;


/* 2006.08.22 : UDR 통계 추가 by sdlee
*/
typedef struct {
    int     svcType;
    st_UDRTimeStat_t    udrInfo;
} STM_UDRStatisticMsgType;

/* 2006.11.23 by helca */

/////////////////////////
// WIPI ONLINE 통계
 
typedef struct _stMultiPackStat
{
    time_t      tStatTime;
    UCHAR       ucIPAFID;
    UCHAR       ucSvcType;
    UCHAR       szReserved[2];
    st_TotReqResStat        stTotStat;
    st_ReqResStat           stFailStat;     /* MMDB, LINKED-LIST FAIL DATA */
    st_ReqResStat           stRetransStat;  /* RETRANSMISSION DATA */
    UINT        uiUpHeadCnt;
    UINT        uiDownHeadCnt;
    UINT        uiNormalRDRCnt;
    UINT        uiAbnormalRDRCnt;

} st_MultiPackStat, *pst_MultiPackStat;

typedef struct
{
    UINT        reqPacketCnt; /* 단말이 서버로 전송한 패킷 갯수 */
    UINT        reqPacketBytes; /* 단말이 서버로 전송한 패킷 데이타의 byte 량 */
    UINT        resPacketCnt;  /* 서버가 단말로 전송한 패킷 갯수 */
    UINT        resPacketBytes;/* 서버가 단말로 전송한 패킷 데이타의 byte 량 */

    /* 서비스계층 관련 패킷 중 재전송으로 판단한 TCP/IP 패킷 */ 
    UINT        reReqPacketCnt;
    UINT        reReqPacketBytes;
    UINT        reResPacketCnt;
    UINT        reResPacketBytes;
   
    /* BSD 내부 오류로 인해 DROP한 메세지 갯수와 Byte 량 */
    UINT        failReqPacketCnt;
    UINT        failReqPacketBytes;
    UINT        failResPacketCnt;
    UINT        failResPacketBytes;

    UINT        upHeadCnt; /* 단말에서 서버로 전송한 BILLCOM HEADER의 갯수 */
    UINT        downHeadCnt; /* 서버에서 단말로 전송한 WICGS HEADER의 갯수 */
    UINT        normalUdrCnt; /* 인터림 주기에 의해 생성된 정상적인 UDR의 갯수 */
    UINT        abnormalUdrCnt; /* 어플리케이션의 변경등에 의해 생성된 UDR의 갯수 */                

} STM_WIPIStatisticMsgType;
/////////////////////////
// KVM 통계

typedef struct _st_KBM_Stat
{
    time_t      tStatTime;
    UCHAR       ucIPAFID;
    UCHAR       ucSvcType;
    UCHAR       szReserved[2];
    st_TotReqResStat        stTotStat;      /* 수집된 데이타 사용량 */
    st_ReqResStat           stFailStat;     /* TCP 세션을 생성 할 수 없어서 낙오된 사용량 */ 
    st_ReqResStat           stRetransStat;  /* 재전송 사용량 */

} st_KVMStat, *pst_KVMStat;

typedef struct
{
    UINT        reqPacketCnt;
    UINT        reqPacketBytes;
    UINT        resPacketCnt;
    UINT        resPacketBytes;

    UINT        reReqPacketCnt;
    UINT        reReqPacketBytes;
    UINT        reResPacketCnt;
    UINT        reResPacketBytes;

    UINT        failReqPacketCnt;
    UINT        failReqPacketBytes;
    UINT        failResPacketCnt;
    UINT        failResPacketBytes;

} STM_KVMStatisticMsgType;
//////////////////////////
// VOD 스트리밍 통계

typedef struct _stRTSPStat
{
    time_t      tStatTime;
    UCHAR       ucIPAFID;
    UCHAR       ucSvcType;
    UCHAR       szReserved[2];

    /* RTSP STAT */
    st_TotReqResStat        stTotStat;
    st_ReqResStat           stDropStat;     /* TOTAL REQUEST PACKET DROP DATA */
    st_ReqResStat           stDropSessStat; /* TCP SESSION NOT-FOUND REQUEST PACKET DROP DATA */
    st_ReqResStat           stDropTranStat; /* HTTP TRANSACTION NOT-FOUND REQUEST PACKET DROP DATA */
    st_ReqResStat           stDropFailStat; /* MMDB, LINKED-LIST FAIL DATA */
    st_ReqResStat           stRetranStat;   /* RETRANSMISSION DATA */
    UINT        uiHTTPTranCnt;  /* 전체 HTTP TRANSACTION 개수 */
    UINT        uiReserved1;
    st_TranInfo             stMatchedTran;  /* HTTP TRANSACTION이 이루어진 경우에 대한 통계 */
    st_TranInfo             stOnlyRESTran;  /* RESPONSE만으로 이루어진 TRANSACTION에 대한 통계 */
    st_TranInfo             stOnlyREQTran;  /* REQUEST만으로 이루어진 TRANSACTION에 대한 통계 */
    UINT        uiDESCRIBEMethodCnt;
    UINT        uiSETUPMethodCnt;
    UINT        uiPLAYMethodCnt;
    UINT        uiTEARDOWNMethodCnt;
    UINT        uiPAUSEMethodCnt;
    UINT        uiOPTIONSMethodCnt;
    UINT        uiANNOUNCEMethodCnt;
    UINT        uiETCMethodCnt;
    
    /* RTP/RTCP STAT */
    st_ReqResStat           stUDPStat;                                      

} st_RTSPStat, *pst_RTSPStat;

typedef struct
{
    UINT        reqRealPacketCnt;
    UINT        reqPackCnt;
    UINT        reqPacketBytes;
    UINT        resRealPackerCnt;
    UINT        resPacketCnt;
    UINT        resPacketBytes;

    /* 서비스계층 관련 패킷 중 재전송으로 판단한 TCP/IP 패킷 */ 
    UINT        reReqPacketCnt;
    UINT        reReqPacketBytes;
    UINT        reResPacketCnt;
    UINT        reResPacketBytes;

    /* 서비스 트랜잭션 구성 시 전송한 메세지 중 DROP한 Packet 갯수와 Byte 량 */
    UINT        dropReqPacketCnt;
    UINT        dropReqPacketBytes;
    UINT        dropResPacketCnt;
    UINT        dropResPacketBytes;

    /* TCP/IP세션이 존재하지 않아 과금정보에 포함하지 못하고 DROP한 메세지 갯수와 Byte량 */
    UINT        tcpDropReqPacketCnt;
    UINT        tcpDropReqPacketBytes;
    UINT        tcpDropResPacketCnt;
    UINT        tcpDropResPacketBytes;

    /* 관련 트랜잭션 세션이 존재하지 않아 과금정보에 포함하지 못하고 DROP한 메세지 갯수와 Byte량 */
    UINT        txnDropReqPacketCnt;
    UINT        txnDropReqPacketBytes;
    UINT        txnDropResPacketCnt;
    UINT        txnDropResPacketBytes;

    /* BSD 내부 오류로 인해 DROP한 메세지 갯수와 Byte 량 */
    UINT        failDropReqPacketCnt;
    UINT        failDropReqPacketBytes;
    UINT        failDropResPacketCnt;
    UINT        failDropResPacketBytes;

    UINT        normTxnCnt;

    UINT        mathedResCnt;
    UINT        mathedResTimeCnt;
    UINT        mathedResTcpCnt;
    UINT        mathedResCallStopCnt;

    UINT        onlyReqCnt;
    UINT        onlyReqTimeCnt;
    UINT        onlyReqTcpCnt;
    UINT        onlyReqCallStopCnt;

    UINT        onlyResCnt;
    UINT        onlyResTimeCnt;
    UINT        onlyResTcpCnt;
    UINT        onlyResCallStopCnt;

    UINT        describeMethodCnt;
    UINT        setupMethodCnt;
    UINT        playMethodCnt;
    UINT        tearDownMethodCnt;
    UINT        pauseMethodCnt;
    UINT        optionMethodCnt;
    UINT        announceMethodCnt;
    UINT        etcMethodCnt;

    
    /* RTP/RTCP STAT */
    UINT        udpReqCnt;
    UINT        udpReqBytes;
    UINT        udpResCnt;
    UINT        udpResBytes;


} STM_VODStatisticMsgType;
#endif

/* 2007.05.31 by helca*/
typedef struct {
    int     svcType;
    st_SvcType_UDRTimeStat_t    st_TimeStat;
} SvcType_UDRSvcOptMsgType;

typedef struct {
    int     svcOpt;
    SvcType_UDRSvcOptMsgType    svc_udrInfo;
} NEW_UDRStatisticMsgType;

// by helca 200705.14
typedef struct
{
    time_t      tStatTime;
    UCHAR       ucIPAFID;
    UCHAR       ucSvcType; /* 1:ME, 2:KUN, 3:ADS, 4:MARS */
    UCHAR       szReserved[2];

    UINT        uiBYEMethodCnt;
    UINT        uiINVITEMethodCnt;

    UINT        uiNOTIFYMethodCnt;
    UINT        uiMESSAGEMethodCnt;

    UINT        uiPUBLISHMethodCnt;
    UINT        uiREGISTERMethodCnt;

    UINT        uiSUBSCRIBEMethodCnt;
    UINT        uiUPDATEMethodCnt;

    UINT        uiETCMethodCnt;
    UINT        uiCallStartCnt;

    UINT        uiTermByeMethodCnt;
    UINT        uiTermCallStopCnt;
    UINT        uiTermTimeOutCnt;
    UINT        uiTermCallIDCnt;

    UINT        uiOtherNetCnt;
    UINT        uiReserved;

    UINT        uiTotAudioCnt;
    UINT        uiTotAudioBytes;
    UINT        uiTotVideoCnt;
    UINT        uiTotVideoBytes;

} STM_SIPStatisticMsgType;




