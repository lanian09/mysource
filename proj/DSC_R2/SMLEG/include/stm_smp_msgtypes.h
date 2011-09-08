
#ifndef __STM_SMP_MSGTYPES_H__
#define __STM_SMP_MSGTYPES_H__

#define MAX_WIN_SVC_NUM		14

typedef struct {
#define		DOWN_CTS_STAT_TYPE	0
#define		DOWN_VPN_STAT_TYPE	1
#define		DOWN_ALS_STAT_TYPE	2
#define		DOWN_CKS_STAT_TYPE	3
#define		DOWN_ACS_STAT_TYPE	4
#define		DOWN_WVPN_STAT_TYPE	5
#define		DOWN_ARS_STAT_TYPE	6
#define		DOWN_FCS_STAT_TYPE	7
#define		DOWN_CIS_STAT_TYPE	8
#define		DOWN_ICS_STAT_TYPE	9
#define		DOWN_RMS_STAT_TYPE	10
#define		DOWN_TCS_STAT_TYPE	11
#define		DOWN_HPS_STAT_TYPE	12
#define   	DOWN_NPS_STAT_TYPE   13  // 2006년 11월 7일 추가
#define		DOWN_ULS_STAT_TYPE	14

// 2006/10/20 타입변경
//#define		DBH_WVPN_STAT_TYPE	19  => DBH_ACS_STAT_TYPE
//#define		DBH_ARS_STAT_TYPE	20 => DBH_WVPN_STAT_TYPE	

#define     DBH_CTS_STAT_TYPE   20
#define     DBH_VPN_STAT_TYPE   21
#define     DBH_ALS_STAT_TYPE   22
#define     DBH_CKS_STAT_TYPE   23

#define     DBH_ACS_STAT_TYPE   24
#define     DBH_WVPN_STAT_TYPE  25
#define     DBH_FCS_STAT_TYPE   26
#define     DBH_CIS_STAT_TYPE   27
#define     DBH_ICS_STAT_TYPE   28
#define     DBH_RMS_STAT_TYPE   29
#define     DBH_TCS_STAT_TYPE   30
#define     DBH_HPS_STAT_TYPE   31
#define		DBH_NPS_STAT_TYPE    32  // 2006년 11월 7일 추가
#define     DBH_ULS_STAT_TYPE   33

#define		LOAD_STAT_TYPE			40
#define		FAULT_STAT_TYPE			41
#define		CDR_STAT_TYPE				42	
#define		REFILL_STAT_TYPE		43	

/////////////////////////////////////////////////////////////////////
// SMP로 서비스별 가입자 수 데이터 관련 구조체
////////////////////////////////////////////////////////////////////
#define SVCSUBS_STAT_TYPE				 44
#define ALS_CLSSUBS_STAT_TYPE      45
#define CIS_CLSSUBS_STAT_TYPE      46
#define CTS_CLSSUBS_STAT_TYPE      47
#define HPS_CLSSUBS_STAT_TYPE      48
#define WVPN_GRPSUBS_STAT_TYPE    49
#define ULS_CLSSUBS_STAT_TYPE    	50
#define LCS_STAT_TYPE    	51
#define CSREFILL_STAT_TYPE    	52

	unsigned int  statType;
	unsigned int  msgLen; /*  Header를 제외한 Body의 길이 */
} SMP_STM_HEADER;

typedef struct {
	STM_HEADER header;
	char body[4096];
}  SMP_STM_statUpdate;



typedef struct {
	unsigned int scpid;
	unsigned int bdis_svc;
	unsigned int rcv_attempt;
	unsigned int rcv_success;
	unsigned int rcv_fail;
	unsigned int snd_attempt;
	unsigned int snd_success;
	unsigned int snd_fail;
	unsigned int record_total;
} STM_Cdr_StatData;

typedef struct {
	int count;
	#define MAX_BDIS_SVC 10
	STM_Cdr_StatData cdr[MAX_BDIS_SVC * 8];
} SMP_STM_CdrStatMsgtype;



typedef struct {
	char	scpInfo[10];
	char	bdisSvc[10];
    long long rcv_attempt;
    long long rcv_success;
    long long rcv_fail;
    long long snd_attempt;
    long long snd_success;
    long long snd_fail;
    long long record_total;
} long_SMP_STM_Cdr_StatData;


typedef struct {
    unsigned int almCode;      /* 1 ~ 9999 */

#define	FLT_TYPE_SMP_SW			1
#define	FLT_TYPE_STANDBY_STS	2
#define	FLT_TYPE_CPU_LOAD_STS	3
#define	FLT_TYPE_MEM_USG_STS	4
#define	FLT_TYPE_HDD_USG_STS	5
#define	FLT_TYPE_INFORMIX_STS	6
#define FLT_TYPE_LAN_STS		7
#define FLT_TYPE_HW_STS			8
#define FLT_TYPE_REP_STS		9
    unsigned int almType;

#define FLT_LEVEL_MINOR		1
#define FLT_LEVEL_MAJOR		2
#define FLT_LEVEL_CRITICAL 	3
	unsigned int almLevel;
	unsigned int almCnt;
} SMP_STM_Fault_StatData;

typedef struct {
    long long almCode;      /* 1 ~ 9999 */

#define	FLT_TYPE_SMP_SW		1
#define	FLT_TYPE_STANDBY_STS	2
#define	FLT_TYPE_CPU_LOAD_STS	3
#define	FLT_TYPE_MEM_USG_STS	4
#define	FLT_TYPE_HDD_USG_STS	5
#define	FLT_TYPE_INFORMIX_STS	6
#define FLT_TYPE_LAN_STS		7
#define FLT_TYPE_HW_STS         8
#define FLT_TYPE_REP_STS        9
    long long almType;

#define FLT_LEVEL_MINOR		1
#define FLT_LEVEL_MAJOR		2
#define FLT_LEVEL_CRITICAL 	3
	long long almLevel;
	long long almCnt;
} long_SMP_STM_Fault_StatData;



typedef struct {
	unsigned int cnt;   
#define MAX_FLT_CNT  100
	SMP_STM_Fault_StatData fault[MAX_FLT_CNT];
} SMP_STM_Fault_StatMsgtype;


typedef struct {
/* max_cpu, max_mem, average_cpu, average_mem : 9999 = 99.99 */
/* max_cps, average_cps : 99 = 99 */
	unsigned int     max_cpu;
    unsigned int     max_mem;
    unsigned int     average_cpu;
    unsigned int     average_mem;
} SMP_STM_Load_StatData;

typedef struct {
/* max_cpu, max_mem, average_cpu, average_mem : 9999 = 99.99 */
/* max_cps, average_cps : 99 = 99 */
	long long     max_cpu;
   long long     max_mem;
   long long     average_cpu;
   long long     average_mem;
} long_SMP_STM_Load_StatData;

typedef struct {
	SMP_STM_Load_StatData load;
} SMP_STM_LoadStatMsgtype;


typedef struct {
#define REFILL_STAT_ALS 0
#define REFILL_STAT_HPS 1
#define REFILL_STAT_ULS 2
#define REFILL_STAT_SVC 3
    unsigned int   attempt[REFILL_STAT_SVC];
    unsigned int   success[REFILL_STAT_SVC];
    unsigned int   fail[REFILL_STAT_SVC];
    unsigned int   not_reg[REFILL_STAT_SVC];
    unsigned int   syntax_err[REFILL_STAT_SVC];
    unsigned int   invalid_para[REFILL_STAT_SVC];
    unsigned int   scp_fail[REFILL_STAT_SVC];
    unsigned int   scp_tmout[REFILL_STAT_SVC];
    unsigned int   etc[REFILL_STAT_SVC];
} SMP_STM_Refill_StatData;

typedef struct {
#define REFILL_STAT_ALS 0
#define REFILL_STAT_HPS 1
#define REFILL_STAT_ULS 2
#define REFILL_STAT_SVC 3
    long long   attempt[REFILL_STAT_SVC];
    long long   success[REFILL_STAT_SVC];
    long long   fail[REFILL_STAT_SVC];
    long long   not_reg[REFILL_STAT_SVC];
    long long   syntax_err[REFILL_STAT_SVC];
    long long   invalid_para[REFILL_STAT_SVC];
    long long   scp_fail[REFILL_STAT_SVC];
    long long   scp_tmout[REFILL_STAT_SVC];
    long long   etc[REFILL_STAT_SVC];
} long_SMP_STM_Refill_StatData;

typedef struct {
	SMP_STM_Refill_StatData refill;
} SMP_STM_RefillStatMsgtype;


typedef struct {
	unsigned int	attempt;
	unsigned int	success;
	unsigned int	fail;
	unsigned int	miss_para;
	unsigned int	unrec_para;
	unsigned int	timeout;
	unsigned int	down_others;
} SMP_STM_Down_StatData;

typedef struct {
	long long	attempt;
	long long	success;
	long long	fail;
	long long	miss_para;
	long long	unrec_para;
	long long	timeout;
	long long	down_others;
} long_SMP_STM_Down_StatData;


typedef	struct {
#define AREA_ADD		0
#define AREA_MODIFY		1
#define AREA_DELETE		2
#define	AREA_SEARCH_1	3
#define	AREA_COPY		4
#define	AREA_RESET		5
#define	AREA_SEARCH_2	6
#define	AREA_UPDATE		7
//#define	AREA_OTEHR		8
#define MSG_MAX_DOWN_CMD	8

	SMP_STM_Down_StatData down[MSG_MAX_DOWN_CMD];
} SMP_STM_DownStatMsgtype;

typedef struct {
	unsigned int	attempt;
	unsigned int	success;
	unsigned int	fail;
	/** fail Reason **/
	unsigned int	al_reg_subs;	/* Already reg subs */
	unsigned int	no_reg_subs;	/* Not reg subs */
	unsigned int	syntax_err;		/* Syntax Error */
	unsigned int	inv_para_val;	/* Invalid para */
	unsigned int	dbh_others;		/* Other Error */
} SMP_STM_Dbh_StatData;

typedef struct {
	long long	attempt;
	long long	success;
	long long	fail;
	/** fail Reason **/
	long long	al_reg_subs;	/* Already reg subs */
	long long	no_reg_subs;	/* Not reg subs */
	long long	syntax_err;		/* Syntax Error */
	long long	inv_para_val;	/* Invalid para */
	long long	dbh_others;		/* Other Error */
} long_SMP_STM_Dbh_StatData;


typedef	struct {
#define	AREA_INS	0
#define AREA_DEL	1
#define	AREA_UPD	2
#define	AREA_SEL	3
#define	MAX_DBH_CMD	4
	SMP_STM_Dbh_StatData 	dbh[MAX_DBH_CMD]; 
} SMP_STM_DbhStatMsgtype;


/////////////////////////////////////////////////////////////////////
// SMP로 서비스별 가입자 수 데이터 관련 구조체
////////////////////////////////////////////////////////////////////
typedef struct {

#define ID_SVC_CTS		0
#define ID_SVC_VPN_G		1
#define ID_SVC_VPN_S		2
#define ID_SVC_ALS		3
#define ID_SVC_ALS_1		4
#define ID_SVC_ALS_2		5
#define ID_SVC_CKS          	6
#define ID_SVC_ACS          	7
#define ID_SVC_WVPN_G		8
#define ID_SVC_WVPN_S		9
#define ID_SVC_FCS          	10
#define ID_SVC_CIS          	11
#define ID_SVC_ICS          	12       
#define ID_SVC_RMS         	13      
#define ID_SVC_TCS          	14     
#define ID_SVC_HPS          	15
#define ID_SVC_NPS          	16		//2006/11/09  추가
#define ID_SVC_ULS		17
#define ID_SVC_LCS		18	// 2008/03/13 add
#define ID_SVC_TOTAL        	19   //2006/11/09  index 수정

//#define ID_SVC_ARS          10
//#define ID_SVC_FCS          11
//#define ID_SVC_CIS          12
//#define ID_SVC_ICS          13       
//#define ID_SVC_RMS         14      
//#define ID_SVC_TCS          15     
//#define ID_SVC_HPS          16
//#define ID_SVC_TOTAL       17
//#define ID_SVC_UNKN        18

	unsigned int    svc_type;
	unsigned int	svc_subs_cnt;
} STM_SvcSubs_StatData;


typedef struct {
   long long    svc_type;
	long long	svc_subs_cnt;
} long_STM_SvcSubs_StatData;


typedef struct {
	unsigned int	count;
	#define MAX_SVC_TYPE         50
	STM_SvcSubs_StatData 	svc_subs[MAX_SVC_TYPE];
} STM_SvcSubs_StatMsgtype;
//////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////
//	 ALS_CLSSUBS_STAT_TYPE:
// CIS_CLSSUBS_STAT_TYPE:
// CTS_CLSSUBS_STAT_TYPE:
// HPS_CLSSUBS_STAT_TYPE:
// ULS_CLSSUBS_STAT_TYPE:

typedef struct {
	unsigned int	  cls_type;
 	unsigned int    cls_subs_cnt;
	unsigned int	  expire_count;
} STM_ClsSubs_StatData;


typedef struct {
	long long	  cls_type;
 	long long    cls_subs_cnt;
	long long	  expire_count;
} long_STM_ClsSubs_StatData;


typedef struct {
	#define MAX_CLS_TYPE         20	
    unsigned int 	    count;
    STM_ClsSubs_StatData    cls_subs[MAX_CLS_TYPE];
} STM_ClsSubs_StatMsgtype;
/////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// WVPN_GRPSUBS_STAT_TYPE
typedef struct {
	char wvpn_grp_mdn[20];
 	unsigned int    wvpn_subs_cnt;
} STM_WvpnSubs_StatData;

typedef struct {
	char wvpn_grp_mdn[20];
 	long long    wvpn_subs_cnt;
} long_STM_WvpnSubs_StatData;

typedef struct {
    unsigned int 	    count;
	#define MAX_WVPN_TYPE         50	
    STM_WvpnSubs_StatData    wvpn_subs[MAX_WVPN_TYPE];
} STM_WvpnSubs_StatMsgtype;
////////////////////////////////////////////////////////////

typedef struct {
#define LCS_SYS_QRY_TYPE    0
#define LCS_ADD_SVC_TYPE    1
#define LCS_DEL_SVC_TYPE    2
#define LCS_SEL_SVC_TYPE    3
#define LCS_NOTI_SVC_TYPE   4
#define LCS_MAX_TYPE_NUM    5
    unsigned int   attempt[LCS_MAX_TYPE_NUM];
    unsigned int   success[LCS_MAX_TYPE_NUM];
    unsigned int   fail[LCS_MAX_TYPE_NUM];
    unsigned int   timeout[LCS_MAX_TYPE_NUM];
    unsigned int   not_regsubs[LCS_MAX_TYPE_NUM];
    unsigned int   too_many[LCS_MAX_TYPE_NUM];
    unsigned int   other[LCS_MAX_TYPE_NUM];
} SMP_STM_Lcs_StatData;

typedef struct {
    long long   attempt[LCS_MAX_TYPE_NUM];
    long long   success[LCS_MAX_TYPE_NUM];
    long long   fail[LCS_MAX_TYPE_NUM];
    long long   timeout[LCS_MAX_TYPE_NUM];
    long long   not_regsubs[LCS_MAX_TYPE_NUM];
    long long   too_many[LCS_MAX_TYPE_NUM];
    long long   other[LCS_MAX_TYPE_NUM];
} long_SMP_STM_Lcs_StatData;

typedef struct {
	SMP_STM_Lcs_StatData lcs;
} SMP_STM_LcsStatMsgtype;

typedef struct {
#define CSREFILL_SYS_QRY_TYPE    0
#define CSREFILL_SEL_SVC_TYPE    1
#define CSREFILL_UPD_SVC_TYPE    2
#define CSREFILL_PLUS_SVC_TYPE    3
#define CSREFILL_MIN_SVC_TYPE   4
#define CSREFILL_MAX_TYPE_NUM    5
    unsigned int   attempt[CSREFILL_MAX_TYPE_NUM];
    unsigned int   success[CSREFILL_MAX_TYPE_NUM];
    unsigned int   fail[CSREFILL_MAX_TYPE_NUM];
    unsigned int   timeout[CSREFILL_MAX_TYPE_NUM];
    unsigned int   not_reg[CSREFILL_MAX_TYPE_NUM];
    unsigned int   format[CSREFILL_MAX_TYPE_NUM];
    unsigned int   permission[CSREFILL_MAX_TYPE_NUM];
    unsigned int   other[CSREFILL_MAX_TYPE_NUM];
} SMP_STM_Csrefill_StatData;

typedef struct {
    long long   attempt[CSREFILL_MAX_TYPE_NUM];
    long long   success[CSREFILL_MAX_TYPE_NUM];
    long long   fail[CSREFILL_MAX_TYPE_NUM];
    long long   timeout[CSREFILL_MAX_TYPE_NUM];
    long long   not_reg[CSREFILL_MAX_TYPE_NUM];
    long long   format[CSREFILL_MAX_TYPE_NUM];
    long long   permission[CSREFILL_MAX_TYPE_NUM];
    long long   other[CSREFILL_MAX_TYPE_NUM];
} long_SMP_STM_Csrefill_StatData;

typedef struct {
	SMP_STM_Csrefill_StatData cs;
} SMP_STM_CsRefillStatMsgtype;


#endif
