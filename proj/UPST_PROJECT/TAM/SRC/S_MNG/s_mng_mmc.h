#ifndef __S_MNG_MMC_H__
#define __S_MNG_MMC_H__

#include "typedef.h"
#include "mmcdef.h"			/* mml_msg */
#include "msgdef.h"			/* st_MsgQ */
#include "common_stg.h"		/* st_TraceList */

/*	START: SCTP의 DIAMETER가 추가됨에 따라 PCRF, OCS, IPLS에 대한 감시 항목 추가	*/
#define DEF_SYSTYPE_SGSN	DEF_TYPE_SGSN
#define DEF_SYSTYPE_GGSN	DEF_TYPE_GGSN
#define DEF_SYSTYPE_AAA		DEF_TYPE_AAA
#define DEF_SYSTYPE_SMSC	3
#define DEF_SYSTYPE_BSC		4
#define DEF_SYSTYPE_IPAS	DEF_TYPE_IPAS
#define DEF_SYSTYPE_DLR		7
#define DEF_SYSTYPE_DNS		DEF_TYPE_DNS
#define DEF_SYSTYPE_WIN		DEF_TYPE_WIN

#define DEF_SYSTYPE_IPLS	DEF_TYPE_IPLS
#define	DEF_SYSTYPE_PCRF	DEF_TYPE_PCRF
#define	DEF_SYSTYPE_OCS		DEF_TYPE_OCS

#define DEF_STAT_FAULT      0
#define DEF_STAT_LOAD       1
#define DEF_STAT_TRAFFIC    2

#define INVALID_FLAG		-1

#define LEN_YYYYMMDDHHMM    13
#define LEN_YYYYMM          6

#define INVALID_VALUE		100

#define DEF_SYS_TAF			1
#define DEF_SYS_TAM			0

#define DEF_SQLSTMT_SIZE	1024
#define DEF_TABLENAME_SIZE	256

typedef struct _st_Svc_Stat{
    S32     dSvcID;
    S32     dOnOff;
} st_Svc_Stat;

#define MAX_SVC_STAT_CNT    16
typedef struct _st_Taf_SvcStat{
    st_Svc_Stat stSvcStat[MAX_SVC_STAT_CNT];
} st_Taf_Svcinfo;

/* mmc.c */
extern int dHandleMMC(st_MsgQ *pstMsgQ);
extern int stop_trc_info(st_TraceList *pTraceList, int count);
extern int set_ntam_alm(mml_msg *ml, long long llNID);
extern int dis_ntaf_conf(mml_msg *ml, long long llNID, int uIndex);
extern int dis_ntaf_alm(mml_msg *ml, long long llNID,int uIndex);
extern int set_ntaf_conf(mml_msg *ml, long long llNID);
extern int set_ntaf_alm(mml_msg *ml, long long llNID, int uIndex);
extern int	dis_timer_info(mml_msg *ml, long long llNID);
extern int set_timer_info(mml_msg *ml, long long llNID);
extern int	dis_timer_info(mml_msg *ml, long long llNID);
extern int set_svc_conf(mml_msg *ml, long long llNID);
extern int dis_trc_info(mml_msg *ml, long long llNID);
extern int add_trc_info(mml_msg *ml, long long llNID);
extern int del_trc_info(mml_msg *ml, long long llNID);
extern int dis_system_stat(mml_msg *ml, long long llNID, int dType);
extern int dis_ntam_conf(mml_msg *ml, long long llNID);
extern int set_ntam_conf(mml_msg *ml, long long llNID);
extern int dis_ntam_alm(mml_msg *ml, long long llNID);
extern int	dis_svc_conf(mml_msg *ml, long long llNID);
extern void set_dbm_ret(dbm_msg_t *mml, short mml_err, short cont_flag, short msg_len );

/* mmc2.c */
extern int dis_flt_his(mml_msg *ml, INT64 llNID);
extern int dis_trend_info(mml_msg *ml, INT64 llNID);
extern int dis_defect_info(mml_msg *ml, long long llNID);
extern int dReloadInfo(mml_msg *ml, INT64 llNID);
extern int dSndDNMSInfo(mml_msg *ml, INT64 llNID);
extern int dis_all_user_info(mml_msg *ml, long long llNID);
extern int dis_flt_svr(mml_msg *ml, long long llNID);
extern int add_flt_svr(mml_msg *ml, long long llNID);
extern int del_flt_svr(mml_msg *ml, long long llNID);
extern int dis_flt_clt(mml_msg *ml, long long llNID);
extern int add_flt_clt(mml_msg *ml, long long llNID);
extern int del_flt_clt(mml_msg *ml, long long llNID);
extern int del_flt_sctp(mml_msg *ml, long long llNID);
extern int add_flt_sctp(mml_msg *ml, long long llNID);
extern int dis_flt_sctp(mml_msg *ml, long long llNID);
extern int dis_equip_info(mml_msg *ml, long long llNID);
extern int add_equip_info(mml_msg *ml, long long llNID);
extern int del_equip_info(mml_msg *ml, long long llNID);
extern int dis_monthrs_info(mml_msg *ml, long long llNID);
extern int set_monthrs_info(mml_msg *ml, long long llNID);
extern int add_monthrs_info(mml_msg *ml, long long llNID);
extern int dis_thrs_info(mml_msg *ml, long long llNID);
extern int set_thrs_info(mml_msg *ml, long long llNID);
extern int dis_his_cmd(mml_msg *ml, INT64 llNID);
extern int add_admin_info(mml_msg *ml, long long llNID);
extern int del_admin_info(mml_msg *ml, long long llNID);
extern int set_admin_info(mml_msg *ml, long long llNID);

#endif /* __S_MNG_MMC_H__ */
