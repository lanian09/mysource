#ifndef _UDRGEN_STAT_HEADER_
#define _UDRGEN_STAT_HEADER_

#define MAX_SVC_NUM 64
#define MAX_UDRGEN_STAT_NUM 12

typedef struct _st_StartStat_t{
    unsigned int    iCnt;
} st_StartStat_t, *pst_StartStat_t;

typedef struct _st_InterimStat_t{
    unsigned int    iCnt;
    unsigned int    iUrlCnt;
} st_InterimStat_t, *pst_InterimStat_t;

typedef struct _st_StopStat_t{
    unsigned int    iCnt;
    unsigned int    iUrlCnt;
} st_StopStat_t, *pst_StopStat_t;

typedef struct _st_PrePaidStat_t{
    unsigned int    iCnt;
    unsigned int    iUrlCnt;
} st_PrePaidStat_t, *pst_PrePaidStat_t;

typedef struct _st_MethodStat_t{
    unsigned int    iGetCnt;
    unsigned int    iPostCnt;
    unsigned int    iEtcCnt;
} st_MethodStat_t, *pst_MethodStat_t;

typedef struct _st_ResultStat_t{
    unsigned int    iInfoCnt;
    unsigned int    iSuccCnt;
    unsigned int    iRedirectCnt;
    unsigned int    iClientErrCnt;
    unsigned int    iServerErrCnt;
    unsigned int    iTermAbortCnt;
    unsigned int    iServerAbortCnt;
} st_ResultStat_t, *pst_ResultStat_t;

typedef struct _st_UsageStat_t{
    unsigned int    iIpUpBytes;
    unsigned int    iIpDownBytes;
    unsigned int    iRetransIpUpBytes;
    unsigned int    iRetransIpDownBytes;
} st_UsageStat_t, *pst_UsageStat_t;

typedef struct _st_TxnStat_t{
    unsigned int    iCompleteSuccCnt;
    unsigned int    iTermResonNormalCnt;
    unsigned int    iTermResonAbnormalCnt;
} st_TxnStat_t, *pst_TxnStat_t;

typedef struct _st_UDRTimeStat_t{
    unsigned int        iTotCnt;
    st_StartStat_t      st_StartStat;
    st_InterimStat_t    st_InterimStat;
    st_StopStat_t       st_StopStat;
    st_PrePaidStat_t    st_PrePaidStat;
    unsigned int        iDownloadCnt;
    st_MethodStat_t     st_MethodStat;
    st_ResultStat_t     st_ResultStat;
    st_UsageStat_t      st_UsageStat;
    st_TxnStat_t        st_TxnStat;
} st_UDRTimeStat_t, *pst_UDRTimeStat_t;


typedef struct _st_UDRSvcStat_t{
    unsigned int        iSvcType;
    st_UDRTimeStat_t     st_TimeStat[MAX_UDRGEN_STAT_NUM];
} st_UDRSvcStat_t, *pst_UDRSvcStat_t;

typedef struct _st_UDRStat_t{
    st_UDRSvcStat_t    st_SvcStat[MAX_SVC_NUM];
} st_UDRStat_t, *pst_UDRStat_t;


#endif
