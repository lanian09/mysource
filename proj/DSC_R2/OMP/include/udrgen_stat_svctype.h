#define MAX_SVC_TYPE_NUM 200 	// 080423, poopee, 100 -> 500
#define MAX_SVC_OPTTYPE_NUM 32
#define MAX_UDRGEN_STAT_NUM 12
 
 
typedef struct _st_SvcType_MethodStat_t{
    unsigned int    iGetCnt;
    unsigned int    iPostCnt;
    unsigned int    iEtcCnt;
} st_SvcType_MethodStat_t, *pst_SvcType_MethodStat_t;
 
typedef struct _st_SvcType_ResultStat_t{
    unsigned int    iInfoCnt;
    unsigned int    iSuccCnt;
    unsigned int    iRedirectCnt;
    unsigned int    iClientErrCnt;
    unsigned int    iServerErrCnt;
    unsigned int    iTermAbortCnt;
    unsigned int    iServerAbortCnt;
} st_SvcType_ResultStat_t, *pst_SvcType_ResultStat_t;
 
typedef struct _st_SvcType_UsageStat_t{
    unsigned int    iIpUpBytes;
    unsigned int    iIpDownBytes;
    unsigned int    iRetransIpUpBytes;
    unsigned int    iRetransIpDownBytes;
} st_SvcType_UsageStat_t, *pst_SvcType_UsageStat_t;
 
typedef struct _st_SvcType_TxnStat_t{
    unsigned int    iCompleteSuccCnt;
    unsigned int    iTermReasonNormalCnt;
    unsigned int    iTermReasonAbnormalCnt;
} st_SvcType_TxnStat_t, *pst_SvcType_TxnStat_t;
 
typedef struct _st_SvcType_UDRTimeStat_t{
    unsigned int        iTotCnt;
    unsigned int        iInterimUrlCnt;
    unsigned int        iStopUrlCnt;
    unsigned int        iPPSUrlCnt;
    unsigned int        iDownloadCnt;
    st_SvcType_MethodStat_t     st_MethodStat;
    st_SvcType_ResultStat_t     st_ResultStat;
    st_SvcType_UsageStat_t      st_UsageStat;
    st_SvcType_TxnStat_t        st_TxnStat;
} st_SvcType_UDRTimeStat_t, *pst_SvcType_UDRTimeStat_t;
 
 
typedef struct _st_SvcType_UDRSvcOpt_t{
    unsigned int                iSvcOpt;
    st_SvcType_UDRTimeStat_t    st_TimeStat[MAX_UDRGEN_STAT_NUM];
} st_SvcType_UDRSvcOpt_t, *pst_SvcType_UDRSvcOpt_t;
 

typedef struct _st_SvcType_UDRSvcStat_t{
    unsigned int                iSvcType;
    st_SvcType_UDRSvcOpt_t    st_OptStat[MAX_SVC_OPTTYPE_NUM];
} st_SvcType_UDRSvcStat_t, *pst_SvcType_UDRSvcStat_t;
 
typedef struct _st_SvcType_UDRStat_t{
st_SvcType_UDRSvcStat_t    st_SvcStat[MAX_SVC_TYPE_NUM];
} st_SvcType_UDRStat_t, *pst_SvcType_UDRStat_t;

