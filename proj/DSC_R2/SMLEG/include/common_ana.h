// BY JUNE, 2011-03016
typedef struct _sm_buf_clr {
	int	dUsed;
	int	dPeriod;
	int	dHour;
	unsigned int uiClrPeriod;
} SM_BUF_CLR;

typedef struct _st_login_info_{
	int 			dArgIsAdditive;
	int				dargAutoLogoutTime;
} SM_LOGIN_INFO, *PSM_LOGIN_INFO;

/* PH Bit Table Information */
typedef struct _st_Pkginfo_ {
	UCHAR			ucUsedFlag;
	UCHAR			ucSMSFlag;
	UCHAR 			ucReserved[2];
	SHORT			sPkgNo;
	SHORT 			sRePkgNo;
} ST_PKG_INFO, *PST_PKG_INFO;

#define	DEF_ST_PKG_INFO_SIZE 	sizeof(ST_PKG_INFO)

#define MAX_PBIT_CNT						100
#define MAX_HBIT_CNT						100		// RULESET 관리에 사용. ex) RULESET[MAX_PBIT_CNT][MAX_HBIT_CNT]
typedef struct _st_PBTable_List_ {
	UINT 			dCount;
	ST_PKG_INFO		stPBTable[MAX_PBIT_CNT][MAX_HBIT_CNT];		/* PBit, HBit Table */
} ST_PBTABLE_LIST, *PST_PBTABLE_LIST;

#define	DEF_ST_PBTABLE_SIZE 	sizeof(ST_PBTABLE_LIST)

typedef struct _st_pdsn_attr_ {
#define DEF_PDSN_NAME_SIZE			16
	UINT			uiAddr;
	UCHAR			ucName[DEF_PDSN_NAME_SIZE];
} PDSN_ATTR, *PPDSN_ATTR;

typedef struct _st_pdsn_list_ {
	UINT			uiCount;
	UINT			uiAddr[DEF_PDSN_CNT];
} PDSN_LIST, *PPDSN_LIST;
#define DEF_PDSN_LIST_SIZE		sizeof(PDSN_LIST)

typedef struct _st_ruleset_used_info_ {
	UINT			uiPBit;
	UINT			uiHBit;
	UINT 			uiOperF;		/* mmc 로 uiUsedF 설정 여부 */
	UINT			uiUsedF;		/* rule set 사용 여부 */
} RULESET_USED_INFO, *pRULESET_USED_INFO;

typedef struct _st_ruleset_used_flag_{
	UINT				uiCount;
	RULESET_USED_INFO	stRule[MAX_PBIT_CNT][MAX_HBIT_CNT];
} RULESET_USED_FLAG, *PRULESET_USED_FLAG;

#define DEF_ST_SM_INFO_SIZE		sizeof(SM_INFO)
typedef struct _st_sm_resp_ {
	long long		llSuccCnt;
	long long		llFailCnt;
} SM_RSP;

typedef struct _st_statistic_ {
	long long       llRadStartCnt;
	long long       llRadInterimCnt;
	long long       llRadStopCnt;
	long long       llLoginCnt;
	long long       llLoginSuccCnt;
	long long       llLogoutCnt;
	long long       llLogoutSuccCnt;

} LEG_STATISTIC;

typedef struct _st_rleg_stat_ {
	unsigned int	uiRecvCnt;
	unsigned int	uiFailCnt;
	LEG_STAT1		stat;
} RLEG_STAT;

typedef struct _st_leg_cps_ {
	long long	llLogOnCps;
	long long	llLogOutCps;
} LEG_CPS;

typedef struct _st_leg_cps_info_ {
	unsigned int uiPrevLogOnCps;
	unsigned int uiLogOnCps;
	unsigned int uiLogOutCps;
	unsigned int uiSuccCps;
	unsigned int uiFailCps;
} LEG_CPS_INFO;

typedef struct _st_leg_cps_conut_ {
	unsigned int llLogOnCps;
	unsigned int llLogOutCps;
	unsigned int llSuccCps;
	unsigned int llFailCps;
} LEG_CPS_COUNT;

typedef struct _st_cps_ovld_ctrl_ {
	int				over_cps;
	int				over_rate;
	unsigned char 	over_flag;
} CPS_OVLD_CTRL;


