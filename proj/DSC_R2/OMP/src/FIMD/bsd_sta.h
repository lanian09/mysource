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
 
 /* ���� ������ IPAF���� ��� ���� */
 /* IPAF ��� */
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
    /* ���� ������ IPAF���� ��� ���� */
 /* IPAF ��� */
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
// ���� ���۰��� ��� svc_tr
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

    st_TranInfo         stOnlyACKTran;       // ACK �� �ִ� ��� 
    st_TranInfo         stOnlyREQACKTran;    // REQ-ACK �� �ִ� ��� : RES �� ����
    st_TranInfo         stOnlyREQRESTran;    // REQ-RES �� �ִ� ��� : ACK�� ����
    st_TranInfo         stOnlyRESACKTran;    // RES-ACK �� �ִ� ��� : REQ �� ����.
 

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

    st_TranInfo         stOnlyACKTran;       // ACK �� �ִ� ���
    st_TranInfo         stOnlyREQACKTran;    // REQ-ACK �� �ִ� ��� : RES �� ����
    st_TranInfo         stOnlyREQRESTran;    // REQ-RES �� �ִ� ��� : ACK�� ����
    st_TranInfo         stOnlyRESACKTran;    // RES-ACK �� �ִ� ��� : REQ �� ����.


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
// ���񽺺� ���� ���.
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
//AAAIF ��� 72�� PTAS_R2.4.0/INC/ ���丮�� ipam_stat.h�� ���ǵ� ������ �����Ͻø� �˴ϴ�.
#if 0 //jean delete

typedef struct _st_RowStat      // sunny, USHORT -> UINT
{
    UINT            usReq;      // 1�� �޽��� Request ����
    UINT            usRep;      // 1�� �޽��� Reply ����
    UINT            usSucc;     // 1�� �޽��� Success ����
    UINT            usFail;     // 1�� �޽��� Fail ����
 
    UINT            usFailCount;
    UINT            usFailReason[MAX_FAILREASON_COUNT]; // Fail Reason [0~19] ���� �����ؼ� ���
}st_RowStat, *pst_RowStat;
 
typedef struct _st_MsgStat
{
    UCHAR           ucSysNo;        // �ش� ��� �� �ý��� ���̵�
    UCHAR           szReserved[5];
    USHORT          usRowCount;
 
    UINT            uiTotCnt;       // �޽��� �� ����
    UINT            uiReqCnt;       // Request Msg ����
    UINT            uiRepCnt;       // Reply Msg ����
    UINT            uiSysIP;
 
    UCHAR           ucInxStart;     //add hskim 20041206
    UCHAR           ucInxBreak;     //add hskim 20041206
    UCHAR           ucIsSum;        //add hskim 20041206
    UCHAR           reserv[5];
 
    st_RowStat      stRowStat[MAX_INFSTAT_COUNT];   // �޼��� �ִ� 15�� ����
}st_MsgStat, *pst_MsgStat;
 
typedef struct _st_InfStat
{
    time_t          tRegTime;       // ��� ��� �ð�
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


//���� �޼����� ���� ��� ���� 
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

//�ش� AAA�� ���� ���� ����
typedef struct _st_MsgStat
{   
    UCHAR           ucSysNo;
    UINT            uiSysIP;      //�ش� AAA IP ����     
    st_RowStat      stRowStat;
  
}st_MsgStat, *pst_MsgStat;

typedef struct _st_InfStat
{   
    time_t           tRegTime;       // ����Ͻð�
    int              dCount;         // AAA�ý��� ������ �ش�:32
    st_MsgStat      stMsgStat[32];  // �� AAA�� ���� ���� ��� ������ ����

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


/* 2006.08.22 : UDR ��� �߰� by sdlee
*/
typedef struct {
    int     svcType;
    st_UDRTimeStat_t    udrInfo;
} STM_UDRStatisticMsgType;

/* 2006.11.23 by helca */

/////////////////////////
// WIPI ONLINE ���
 
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
    UINT        reqPacketCnt; /* �ܸ��� ������ ������ ��Ŷ ���� */
    UINT        reqPacketBytes; /* �ܸ��� ������ ������ ��Ŷ ����Ÿ�� byte �� */
    UINT        resPacketCnt;  /* ������ �ܸ��� ������ ��Ŷ ���� */
    UINT        resPacketBytes;/* ������ �ܸ��� ������ ��Ŷ ����Ÿ�� byte �� */

    /* ���񽺰��� ���� ��Ŷ �� ���������� �Ǵ��� TCP/IP ��Ŷ */ 
    UINT        reReqPacketCnt;
    UINT        reReqPacketBytes;
    UINT        reResPacketCnt;
    UINT        reResPacketBytes;
   
    /* BSD ���� ������ ���� DROP�� �޼��� ������ Byte �� */
    UINT        failReqPacketCnt;
    UINT        failReqPacketBytes;
    UINT        failResPacketCnt;
    UINT        failResPacketBytes;

    UINT        upHeadCnt; /* �ܸ����� ������ ������ BILLCOM HEADER�� ���� */
    UINT        downHeadCnt; /* �������� �ܸ��� ������ WICGS HEADER�� ���� */
    UINT        normalUdrCnt; /* ���͸� �ֱ⿡ ���� ������ �������� UDR�� ���� */
    UINT        abnormalUdrCnt; /* ���ø����̼��� ���� ���� ������ UDR�� ���� */                

} STM_WIPIStatisticMsgType;
/////////////////////////
// KVM ���

typedef struct _st_KBM_Stat
{
    time_t      tStatTime;
    UCHAR       ucIPAFID;
    UCHAR       ucSvcType;
    UCHAR       szReserved[2];
    st_TotReqResStat        stTotStat;      /* ������ ����Ÿ ��뷮 */
    st_ReqResStat           stFailStat;     /* TCP ������ ���� �� �� ��� ������ ��뷮 */ 
    st_ReqResStat           stRetransStat;  /* ������ ��뷮 */

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
// VOD ��Ʈ���� ���

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
    UINT        uiHTTPTranCnt;  /* ��ü HTTP TRANSACTION ���� */
    UINT        uiReserved1;
    st_TranInfo             stMatchedTran;  /* HTTP TRANSACTION�� �̷���� ��쿡 ���� ��� */
    st_TranInfo             stOnlyRESTran;  /* RESPONSE������ �̷���� TRANSACTION�� ���� ��� */
    st_TranInfo             stOnlyREQTran;  /* REQUEST������ �̷���� TRANSACTION�� ���� ��� */
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

    /* ���񽺰��� ���� ��Ŷ �� ���������� �Ǵ��� TCP/IP ��Ŷ */ 
    UINT        reReqPacketCnt;
    UINT        reReqPacketBytes;
    UINT        reResPacketCnt;
    UINT        reResPacketBytes;

    /* ���� Ʈ����� ���� �� ������ �޼��� �� DROP�� Packet ������ Byte �� */
    UINT        dropReqPacketCnt;
    UINT        dropReqPacketBytes;
    UINT        dropResPacketCnt;
    UINT        dropResPacketBytes;

    /* TCP/IP������ �������� �ʾ� ���������� �������� ���ϰ� DROP�� �޼��� ������ Byte�� */
    UINT        tcpDropReqPacketCnt;
    UINT        tcpDropReqPacketBytes;
    UINT        tcpDropResPacketCnt;
    UINT        tcpDropResPacketBytes;

    /* ���� Ʈ����� ������ �������� �ʾ� ���������� �������� ���ϰ� DROP�� �޼��� ������ Byte�� */
    UINT        txnDropReqPacketCnt;
    UINT        txnDropReqPacketBytes;
    UINT        txnDropResPacketCnt;
    UINT        txnDropResPacketBytes;

    /* BSD ���� ������ ���� DROP�� �޼��� ������ Byte �� */
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




