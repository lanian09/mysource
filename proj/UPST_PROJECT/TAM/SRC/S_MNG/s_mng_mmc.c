/**A.1*  File Inclusion *******************************************************/
#include <stdio.h>
#include <mysql/mysql.h>
#include <arpa/inet.h>			/* inet_ntop */
#include <errno.h>
#include <ctype.h>				/* TOUPPER(3) */
#include <unistd.h>				/* USLEEP(3) */

#include "s_mng_mmc.h"		/* stTafSvcinfo */
#include "s_mng_func.h"		/* dWriteTimerFile() */
#include "s_mng_msg.h"
#include "s_mng_flt.h"
#include "s_mng_util.h"		/* tGetTimeFromStr() */
#include "s_mng_init.h"		/* dWriteSvcStat() */

#include "filter.h"
#include "filedb.h"
#include "tools.h"			/* dWriteTraceTblList() */

#include "db_api.h"
#include "db_struct.h"
#include "db_define.h"
#include "common_stg.h"
#include "commdef.h"		/* FILE_XX */
#include "sockio.h"
#include "path.h"

#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

#include "mmcdef.h"		/* MI_ */
#include "msgdef.h"
#include "procid.h"

#include "loglib.h"
#include "utillib.h"

int						gdSysNo;
st_AlmLevel_List		stNtafAlarm[MAX_NTAF_NUM];
st_Taf_Svcinfo			stTafSvclist;

extern stMEMSINFO		*pMEMSINFO;
extern stCIFO			*gpCIFO;
extern MYSQL			stMySQL;
extern st_keepalive		*keepalive;
extern st_subsys_mng	*pstSubSys;
extern st_Flt_Info		*flt_info;
extern st_TraceList		*trace_tbl;
extern st_Mmlsg			g_stMmlsg;

/* dGet***()Function  ->  DBLIB/dblib.pc */

int dHandleMMC(st_MsgQ *pstMsgQ)
{
	int			dRet;
	mml_msg		*ml;

	ml = (mml_msg*)pstMsgQ->szBody;

	log_print(LOGN_INFO, LH"MML MSG(msg_id[%hu] GET", LT, ml->msg_id);
	switch(ml->msg_id)
	{
		case MI_DIS_NTAM:	/*	DIS-NTAM-CONF	*/
			if( (dRet = dis_ntam_conf(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_ntam_conf() dRet[%d]", LT, dRet);
			break;

		case MI_CHG_NTAM:	/*	SET-NTAM-CONF	*/
			if( (dRet = set_ntam_conf(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN set_ntam_conf() dRet[%d]", LT, dRet);
			break;

		case MI_DIS_ALM_NTAM:	/*	DIS-NTAM-ALM	*/
			if( (dRet = dis_ntam_alm(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_ntam_alm() dRet[%d]", LT, dRet);
			break;

		case MI_CHG_ALM_NTAM:	/*	SET-NTAM-ALM	*/
			if( (dRet = set_ntam_alm(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN set_ntam_alm() dRet[%d]", LT, dRet);
			break;

		case MI_DIS_NTAF:	/*	DIS-NTAF-CONF	*/
			if( (dRet = dis_ntaf_conf(ml, pstMsgQ->llNID, pstMsgQ->llIndex)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_ntaf_conf() dRet[%d]", LT, dRet);
			break;

		case MI_DIS_ALM_NTAF:	/*	DIS-NTAF-ALM	*/
			if( (dRet = dis_ntaf_alm(ml, pstMsgQ->llNID,pstMsgQ->llIndex)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_ntaf_alm() dRet[%d]", LT, dRet);
			break;

		case MI_CHG_NTAF:	/*	SET-NTAF-CONF	*/
			if( (dRet = set_ntaf_conf(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN set_ntaf_conf() dRet[%d]", LT, dRet);
			break;

		case MI_CHG_ALM_NTAF:	/*	SET-NTAF-ALM	*/
			if( (dRet = set_ntaf_alm(ml, pstMsgQ->llNID, pstMsgQ->llIndex)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN set_ntaf_alm() dRet[%d]", LT, dRet);
			break;

		/** TIMER ****************************************************************/
		case MI_DIS_TIMER:	/*	DIS-TIMER-INFO	*/
			log_print(LOGN_INFO, LH"MML MSG(MI_DIS_TIMER[%d]): dis_timer_info()", LT, MI_DIS_TIMER);
			if( (dRet = dis_timer_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_timer_info() dRet[%d]", LT, dRet);
			break;

		case MI_SET_TIMER:	/*	SET-TIMER-INFO	*/
			log_print(LOGN_INFO, LH"MML MSG(MI_SET_TIMER[%d]): dis_timer_info()", LT, MI_SET_TIMER);
			if( (dRet = set_timer_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN set_timer_info() dRet[%d]", LT, dRet);
			break;

		/** SERVICE ON/OFF  *****************************************************/
		case MI_DIS_SVC_SW:	/*	DIS-SVC-CONF	*/
			log_print(LOGN_INFO, LH"MML MSG(MI_DIS_SVC_SW[%d])", LT, MI_DIS_SVC_SW);
			if( (dRet = dis_svc_conf(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_svc_conf() dRet[%d]", LT, dRet);

			break;

		case MI_SET_SVC_SW:	/*	SET-SVC-CONF */
			log_print(LOGN_INFO, LH"MML MSG(MI_SET_SVC_SW[%d])", LT, MI_SET_SVC_SW);
			if( (dRet = set_svc_conf(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN set_svc_conf() dRet[%d]", LT, dRet);

			break;


		/** TRACE ****************************************************************/
		case MI_DIS_TRC_INFO:	/*	DIS-TRC-INFO	*/
			if( (dRet = dis_trc_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_trc_info() dRet[%d]", LT, dRet);
			break;

		case MI_ADD_TRC_INFO:	/*	ADD-TRC-INFO	*/
			if( (dRet = add_trc_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN add_trc_info() dRet[%d]", LT, dRet);
			break;

		case MI_DEL_TRC_INFO:	/*	DEL-TRC-INFO	*/
			if( (dRet = del_trc_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN del_trc_info() dRet[%d]", LT, dRet);
			break;

		/** SERVER FILTER     **********************************************************/
		case MI_DIS_SVC:	/*	DIS-FLT-SVR	*/
			if( (dRet = dis_flt_svr(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_flt_svr() dRet[%d]", LT, dRet);
			break;

		case MI_ADD_SVC:	/*	ADD-FLT-SVR	*/
			if( (dRet = add_flt_svr(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN add_flt_svr() dRet[%d]", LT, dRet);
			break;

		case MI_DEL_SVC:	/*	DEL-FLT-SVR	*/
			if( (dRet = del_flt_svr(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN del_flt_svr() dRet[%d]", LT, dRet);
			break;

		/** MN FILTER *********************************************************/
		case MI_DIS_MNIP:	/*	DIS-FLT-CLT	*/
            if( (dRet = dis_flt_clt(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_flt_clt() dRet[%d]", LT, dRet);
            break;

        case MI_ADD_MNIP:	/*	ADD-FLT-CLT	*/
            if( (dRet = add_flt_clt(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN add_flt_clt() dRet[%d]", LT, dRet);
            break;

        case MI_DEL_MNIP:	/*	DEL-FLT-CLT	*/
            if( (dRet = del_flt_clt(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN del_flt_clt() dRet[%d]", LT, dRet);
            break;

		/** SCTP FILTER *********************************************************/
		case MI_DIS_SCTP:	/*	DIS-FLT-SCTP	*/
            if( (dRet = dis_flt_sctp(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_flt_sctp() dRet[%d]", LT, dRet);
            break;

        case MI_ADD_SCTP:	/*	ADD-FLT-SCTP	*/
            if( (dRet = add_flt_sctp(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN add_flt_sctp() dRet[%d]", LT, dRet);
            break;

        case MI_DEL_SCTP:	/*	DEL-FLT-SCTP	*/
            if( (dRet = del_flt_sctp(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN del_flt_sctp() dRet[%d]", LT, dRet);
            break;

		/** EQUIP  - NASIP ********************************************************/
		case MI_DIS_EQUIP_INFO:	/*	DIS-EQUIP-INFO	*/
			if( (dRet = dis_equip_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_equip_info() dRet[%d]", LT, dRet);
			break;

		case MI_ADD_EQUIP_INFO:	/*	ADD-EQUIP-INFO	*/
			if( (dRet = add_equip_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN add_equip_info() dRet[%d]", LT, dRet);
			break;

        case MI_DEL_EQUIP_INFO:	/*	DEL-EQUIP-INFO	*/
            if( (dRet = del_equip_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN del_equip_info() dRet[%d]", LT, dRet);
            break;

		/** MON THRESHOLD      ********************************************************/
		case MI_DIS_MONTHRES:	/*	DIS-MONTHRS-INFO	*/
			if( (dRet = dis_monthrs_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_monthrs_info() dRet[%d]", LT, dRet);
			break;

		case MI_CHG_MONTHRES:	/*	SET-MONTHRS-INFO	*/
			if( (dRet = set_monthrs_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN set_monthrs_info() dRet[%d]", LT, dRet);
			break;
        /* added by dcham 2011.06.26 */
		case MI_ADD_MONTHRES:	/*	ADD-MONTHRS-INFO	*/
			if( (dRet = add_monthrs_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN add_monthrs_info() dRet[%d]", LT, dRet);
			break;

		/** THRESHOLD      ********************************************************/
		case MI_DIS_THRES_INFO:	/*	DIS-THRS-INFO	*/
			if( (dRet = dis_thrs_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_thrs_info() dRet[%d]", LT, dRet);
			break;

		case MI_CHG_THRES_INFO:	/*	SET_THRS-INFO	*/
			if( (dRet = set_thrs_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN set_thrs_info() dRet[%d]", LT, dRet);
			break;

		/** etc            ********************************************************/
		case MI_DIS_HIS_CMD:	/*	DIS-CMD-HIS	*/
			if( (dRet = dis_his_cmd(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_his_cmd() dRet[%d]", LT, dRet);
			break;

		/** USER INFO      ********************************************************/
		case MI_DIS_USER_INFO:	/*	DIS-USER-INFO	*/
			if( (dRet = dis_all_user_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_all_user_info() dRet[%d]", LT, dRet);
			break;

		case MI_ADD_USER:	/*	ADD-USER-INFO	*/
			if( (dRet = add_admin_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN add_admin_info() dRet[%d]", LT, dRet);
			break;

		case MI_DEL_USER:	/*	DEL-USER-INFO	*/
			if( (dRet = del_admin_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN del_admin_info() dRet[%d]", LT, dRet);
			break;

		case MI_CHG_USER_INFO:	/*	CHG-USER-INFO	*/
			if( (dRet = set_admin_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN set_admin_info() dRet[%d]", LT, dRet);
			break;

		/** DIS FAULT STAT ********************************************************/
		case MI_DIS_FAULT_STAT:	/*	DIS-FAULT-STAT	*/
			if( (dRet = dis_system_stat(ml, pstMsgQ->llNID, DEF_STAT_FAULT)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_system_stat(DEF_STAT_FAULT) dRet[%d]", LT, dRet);
			break;

		/** DIS LOAD STAT ********************************************************/
		case MI_DIS_LOAD_STAT:	/*	DIS-LOAD-STAT	*/
			if( (dRet = dis_system_stat(ml, pstMsgQ->llNID, DEF_STAT_LOAD)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_system_stat(DEF_STAT_LOAD) dRet[%d]", LT, dRet);
			break;

		/** DIS TRAFFIC STAT ********************************************************/
		case MI_DIS_TRAFFIC_STAT:	/*	DIS-LOAD-STAT	*/
			if( (dRet = dis_system_stat(ml, pstMsgQ->llNID, DEF_STAT_TRAFFIC)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_system_stat(DEF_STAT_TRAFFIC) dRet[%d]", LT, dRet);
			break;

		case MI_DIS_FLT_HIS:	/*	DIS-FLT-HIS		*/
			if( (dRet = dis_flt_his(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_flt_his(DEF_STAT_LOAD) dRet[%d]", LT, dRet);
			break;

		case MI_DIS_TREND_INFO:
			if( (dRet = dis_trend_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_trend_info() dRet[%d]", LT, dRet);
			break;

		case MI_DIS_DEFECT_INFO:
			if( (dRet = dis_defect_info(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dis_defect_info() dRet[%d]", LT, dRet);
			break;

		/*	RELOAD-INFO	*/
		case MI_RELOAD_INFO:
			if( (dRet = dReloadInfo(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dReloadInfo() dRet[%d]", LT, dRet);
			break;

		/*	SND-DNMS-CONF	*/
		case MI_SND_DNMS_CONF:
			if( (dRet = dSndDNMSInfo(ml, pstMsgQ->llNID)) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dSndDNMSInfo() dRet[%d]", LT, dRet);
			break;

		default:
			log_print(LOGN_CRI, "dHandleMMC : WRONG COMMOND msg_id[%d]", ml->msg_id);
			break;
	}

	return 0;
}

/* added by uamyd 20080228 for GT INFO */

/*
	setting dbm_msg_t by uamyd 20080215
	SET TotPage, CurPage
	if totalcnt = -1, NO SET totalcnt else SET totalcnt
*/
void set_dbm_cnt(dbm_msg_t *mml, short total_cnt, short TotPage, short CurPage )
{
	if( total_cnt != INVALID_FLAG )
		mml->common.total_cnt = total_cnt;
	mml->common.TotPage = TotPage;
	mml->common.CurPage = CurPage;
	return;
}

/*
	setting dbm_msg_t by uamyd 20080215
	SET mml_err, cont_flag
	if msg_len = -1, NO SET msg_len else SET msg_len
*/
void set_dbm_ret(dbm_msg_t *mml, short mml_err, short cont_flag, short msg_len )
{
	mml->common.mml_err = mml_err;
	mml->common.cont_flag = cont_flag;

	if( msg_len != INVALID_FLAG )
		mml->head.msg_len = msg_len;

	return;
}

int dGetQuery(int dType, char *sSqlStmt, int dSysType, int dSysID, time_t tStartTime, time_t tEndTime)
{
	char	sTableName[DEF_TABLENAME_SIZE];
	size_t	szPos;

	if(dType == DEF_STAT_FAULT)
	{
		/* FAULT */
		sprintf(sTableName, "STAT_FAULT_5MIN");
		sprintf(sSqlStmt,
			"SELECT "
				"FAULTTYPE,IFNULL(SUM(CRI),0),IFNULL(SUM(MAJ),0),IFNULL(SUM(MIN),0),IFNULL(SUM(STOP),0),IFNULL(SUM(NORMAL),0) "
			"FROM ");
	}
	else if(dType == DEF_STAT_LOAD)
	{
		/* LOAD */
		sprintf(sTableName,"STAT_LOAD_5MIN");
		sprintf(sSqlStmt,
			"SELECT "
				"IFNULL(TRUNCATE(AVG(CPUAVG),2),0),IFNULL(TRUNCATE(MAX(CPUMAX),2),0),IFNULL(TRUNCATE(MIN(CPUMIN),2),0),"
				"IFNULL(TRUNCATE(AVG(MEMAVG),2),0),IFNULL(TRUNCATE(MAX(MEMMAX),2),0),IFNULL(TRUNCATE(MIN(MEMMIN),2),0),"
				"IFNULL(TRUNCATE(AVG(QUEAVG),2),0),IFNULL(TRUNCATE(MAX(QUEMAX),2),0),IFNULL(TRUNCATE(MIN(QUEMIN),2),0),"
				"IFNULL(TRUNCATE(AVG(NIFOAVG),2),0),IFNULL(TRUNCATE(MAX(NIFOMAX),2),0),IFNULL(TRUNCATE(MIN(NIFOMIN),2),0),"
				"IFNULL(TRUNCATE(AVG(TRAFFICAVG),2),0),IFNULL(TRUNCATE(MAX(TRAFFICMAX),2),0),IFNULL(TRUNCATE(MIN(TRAFFICMIN),2),0),"
				"IFNULL(TRUNCATE(AVG(DISK1AVG),2),0),IFNULL(TRUNCATE(MAX(DISK1MAX),2),0),IFNULL(TRUNCATE(MIN(DISK1MIN),2),0),"
				"IFNULL(TRUNCATE(AVG(DISK2AVG),2),0),IFNULL(TRUNCATE(MAX(DISK2MAX),2),0),IFNULL(TRUNCATE(MIN(DISK2MIN),2),0),"
				"IFNULL(TRUNCATE(AVG(DISK3AVG),2),0),IFNULL(TRUNCATE(MAX(DISK3MAX),2),0),IFNULL(TRUNCATE(MIN(DISK3MIN),2),0),"
				"IFNULL(TRUNCATE(AVG(DISK4AVG),2),0),IFNULL(TRUNCATE(MAX(DISK4MAX),2),0),IFNULL(TRUNCATE(MIN(DISK4MIN),2),0) "
			"FROM ");
	}
	else
	{
		sprintf(sTableName, "STAT_TRAFFIC_5MIN");
		sprintf(sSqlStmt,
			"SELECT "
				"IFNULL(SUM(THRU_FRAMES),0),IFNULL(SUM(THRU_BYTES),0),"
				"IFNULL(SUM(TOT_FRAMES),0),IFNULL(SUM(TOT_BYTES),0),"
				"IFNULL(SUM(IP_FRAMES),0),IFNULL(SUM(IP_BYTES),0),"
				"IFNULL(SUM(UDP_FRAMES),0),IFNULL(SUM(UDP_BYTES),0),"
				"IFNULL(SUM(TCP_FRAMES),0),IFNULL(SUM(TCP_BYTES),0),"
				"IFNULL(SUM(SCTP_FRAMES),0),IFNULL(SUM(SCTP_BYTES),0),"
				"IFNULL(SUM(ETC_FRAMES),0),IFNULL(SUM(ETC_BYTES),0),"
				"IFNULL(SUM(IPERROR_FRAMES),0),IFNULL(SUM(IPERROR_BYTES),0),"
				"IFNULL(SUM(UTCP_FRAMES),0),IFNULL(SUM(UTCP_BYTES),0),"
				"IFNULL(SUM(FAILDATA_FRAMES),0),IFNULL(SUM(FAILDATA_BYTES),0),"
				"IFNULL(SUM(FILTEROUT_FRAMES),0),IFNULL(SUM(FILTEROUT_BYTES),0) "
			"FROM ");
	}

	log_print(LOGN_INFO, LH"Query=%s", LT, sSqlStmt);
	log_print(LOGN_INFO, "TABLE[%s]", sTableName);

	/*	MAKE QUERY	*/
	szPos = strlen(sSqlStmt);

	if(dType == DEF_STAT_TRAFFIC)
	{
		sprintf(&sSqlStmt[szPos],
			"%s "
			"WHERE "
				"(STATTIME>='%lu') AND (STATTIME<'%lu') AND (TAFID='%d')",
			sTableName, tStartTime, tEndTime, dSysID);
	}
	else
	{
		sprintf(&sSqlStmt[szPos],
			"%s "
			"WHERE "
				"(STATTIME>='%lu') AND (STATTIME<'%lu') AND (SYSTEMTYPE='%d') AND (SYSTEMID='%d')",
			sTableName, tStartTime, tEndTime, dSysType, dSysID);

		szPos = strlen(sSqlStmt);

		if(dType == DEF_STAT_FAULT)
			sprintf(&sSqlStmt[szPos], " GROUP BY FAULTTYPE");
	}

	return 0;
}

int dis_system_stat(mml_msg *ml, long long llNID, int dType)
{
	int				i, dRet, dSysType, dSysID;
	time_t			tStartTime, tEndTime;
	size_t			szStrLen;
	dbm_msg_t		smsg;
	st_SYS_STAT		stSysStat[9];
	st_traffic_stat	stTraffic;
	char			sQuery[DEF_SQLSTMT_SIZE], sStartTime[LEN_YYYYMMDDHHMM], sEndTime[LEN_YYYYMMDDHHMM];

	log_print(LOGN_INFO, LH"dType(%s)", LT,
		(dType==DEF_STAT_FAULT)?"DEF_STAT_FAULT":((dType==DEF_STAT_LOAD)?"DEF_STAT_LOAD":((dType==DEF_STAT_TRAFFIC)?"DEF_STAT_TRAFFIC":"UNKNOWN TYPE")));

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 63:	/*	1st parameter: TAMAPP, TAF				*/
				dSysType = atoi(ml->msg_body[i].para_cont);
				break;
			case 40:
			case 441:	/*	2nd parameter: System Number			*/
				dSysID	= atoi(ml->msg_body[i].para_cont);
				break;
			case 577:	/*	3rd parameter(optional): Start Time		*/
				if( (szStrLen = strlen(ml->msg_body[i].para_cont)) > LEN_YYYYMMDDHHMM)
				{
					log_print(LOGN_WARN, LH"INVALID PARAMETER=%s", LT, ml->msg_body[i].para_cont);

					set_dbm_ret(&smsg, eBadParameter, DBM_END, 0);
					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
				else
					strncpy(sStartTime, ml->msg_body[i].para_cont, sizeof(sStartTime));
				break;
			case 578:	/*	4th parameter(optional): End Time		*/
				if( (szStrLen = strlen(ml->msg_body[i].para_cont)) > LEN_YYYYMMDDHHMM)
				{
					log_print(LOGN_WARN, LH"INVALID PARAMETER=%s", LT, ml->msg_body[i].para_cont);

					set_dbm_ret(&smsg, eBadParameter, DBM_END, 0);
					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -3;
					}
					return -4;
				}
				else
					strncpy(sEndTime, ml->msg_body[i].para_cont, sizeof(sStartTime));
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT, ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	if(dType == DEF_STAT_TRAFFIC)
		dSysType = DEF_SYS_TAF;

	/*	Validation check: 1st, 2nd Parameters 	*/
	if( ((dSysType == DEF_SYS_TAM) && (dSysID!=0)) || 
		((dSysType == DEF_SYS_TAF) && (!dSysID || 
		(dSysID    != pstSubSys->sys[dSysID-1].usSysNo))))
	{
		log_print(LOGN_DEBUG, LH"dSysType[%s] dSysID[%d]", LT, (dSysType==DEF_SYS_TAM)?"TAMAPP":"TAF", dSysID);
		set_dbm_ret(&smsg, eINVALID_SUBSYSTEM, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

	/*	NO OPTIONAL PARAMETERS	*/
	if( ((dType==DEF_STAT_TRAFFIC) && (ml->num_of_para==3)) || (((dType==DEF_STAT_FAULT)||(dType==DEF_STAT_LOAD)) && (ml->num_of_para==4)))
	{
		tStartTime	= tGetTimeFromStr(sStartTime);
		tEndTime	= tGetTimeFromStr(sEndTime);
		/* Delete a MAXIMUM ONE MONTH LIMIT
		if( (tStartTime<0) || (tEndTime<0) || (tStartTime>tEndTime) || ((tEndTime-tStartTime)>SEC_OF_MON))		*/
		if( (tStartTime < 0) || (tEndTime < 0) || (tStartTime > tEndTime))
		{
			log_print(LOGN_WARN, LH"tStartTime[%s] tEndTime[%s]", LT, sStartTime, sEndTime);
			set_dbm_ret(&smsg, eINVALIDE_SEARCH_TIME, DBM_END,0);
			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -7;
			}
			return -8;
		}
	}
	else
	{
		/*	DEFAULT: 1 HOUR	*/
		tEndTime	= time(NULL);
		tStartTime	= tEndTime - SEC_OF_HOUR;
	}

	memset(&smsg, 0x00, sizeof(dbm_msg_t));
	memset(&stSysStat, 0x00, sizeof(st_SYS_STAT)*8);
	memset(sQuery, 0x00, DEF_SQLSTMT_SIZE);

	if( (dRet = dGetQuery(dType, sQuery, dSysType, dSysID, tStartTime, tEndTime)) < 0)
	{
		log_print(LOGN_WARN, LH"ERROR IN dGetQuery() dRet[%d]", LT, dRet);
		set_dbm_ret(&smsg, eTABLE_NOT_EXIST, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -9;
		}
		return -10;
	}

	if(dType == DEF_STAT_TRAFFIC)
	{
		if( (dRet = dGetTrafficStat(&stMySQL, &stTraffic, sQuery)) < 0)
		{
			set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);
			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -11;
			}
			return -12;
		}

		stTraffic.starttime		= tStartTime;
		stTraffic.finishtime	= tEndTime;

		log_print(LOGN_INFO, "TRAFFIC STAT LOG: START[%lu] END[%lu] TAFID[%d]", stTraffic.starttime, stTraffic.finishtime, dSysID);
		log_print(LOGN_INFO, "TRAFFIC STAT LOG: ThruStat[%u][%lu] TotStat[%u][%lu] IPStat[%u][%lu]",
			stTraffic.uThruStatFrames, stTraffic.ulThruStatBytes,
			stTraffic.uTotStatFrames, stTraffic.ulTotStatBytes,
			stTraffic.uIPStatFrames, stTraffic.ulIPStatBytes);
		log_print(LOGN_INFO, "TRAFFIC STAT LOG: UDPStat[%u][%lu] TCPStat[%u][%lu] SCTPStat[%u][%lu]",
			stTraffic.uUDPStatFrames, stTraffic.ulUDPStatBytes,
			stTraffic.uTCPStatFrames, stTraffic.ulTCPStatBytes,
			stTraffic.uSCTPStatFrames, stTraffic.ulSCTPStatBytes);
		log_print(LOGN_INFO, "TRAFFIC STAT LOG: ETCStat[%u][%lu] IPError[%u][%lu] UTCPError[%u][%lu]",
			stTraffic.uETCStatFrames, stTraffic.ulETCStatBytes,
			stTraffic.uIPErrorFrames, stTraffic.ulIPErrorBytes,
			stTraffic.uUTCPErrorFrames, stTraffic.ulUTCPErrorBytes);
		log_print(LOGN_INFO, "TRAFFIC STAT LOG: FailData[%u][%lu] FilterOut[%u][%lu]",
			stTraffic.uFailDataFrames, stTraffic.ulFailDataBytes,
			stTraffic.uFilterOutFrames, stTraffic.ulFilterOutBytes);

		set_dbm_ret(&smsg, DBM_SUCCESS, DBM_END, sizeof(st_traffic_stat));
		memcpy(smsg.data, &stTraffic, sizeof(st_traffic_stat));

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -13;
		}
	}
	else
	{
		if( (dRet = dGetSysStat(&stMySQL, stSysStat, sQuery, dType)) < 0)
		{
			set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);
			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -14;
			}
			return -15;
		}

		if(dRet > 0)
			set_dbm_cnt(&smsg, 1, dRet, 1);

		if(dRet == 0)
		{
			for(i = 0; i < 9; i++)
				stSysStat[i].usType = INVALID_VALUE;
		}

		for(i = 0; i < dRet; i++)
		{
			stSysStat[i].starttime	= tStartTime;
			stSysStat[i].finishtime	= tEndTime;
			stSysStat[i].usSysType	= dSysType;
			stSysStat[i].usSysID	= dSysID;

			if(dType == DEF_STAT_FAULT)
				stSysStat[i].usStatType = DEF_STAT_FAULT;
			else
			{
				stSysStat[i].usStatType = DEF_STAT_LOAD;
				stSysStat[i].usType		= i;
			}

			if( (stSysStat[i].uiMax == 0) && (stSysStat[i].uiMin == 10000))
			{
				stSysStat[i].usType = INVALID_VALUE;
				stSysStat[i].uiAvg = 0;
				stSysStat[i].uiMax = 0;
				stSysStat[i].uiMin = 0;
			}

			log_print(LOGN_INFO,"STAT LOG: START=%ld, END=%ld, STAT_TYPE=%d, CHKTYPE=%d, SYSTYPE=%d, SYSID=%d",
				stSysStat[i].starttime, stSysStat[i].finishtime, stSysStat[i].usStatType, stSysStat[i].usType,
				stSysStat[i].usSysType, stSysStat[i].usSysID );

			if(dType == DEF_STAT_LOAD)
				log_print(LOGN_INFO,"STAT LOG: AVG=%3.2f, MAX=%3.2f, MIN=%3.2f",
				(float)(stSysStat[i].uiAvg/100.0), (float)(stSysStat[i].uiMax/100.0), (float)(stSysStat[i].uiMin/100.0) );
			else
				log_print(LOGN_INFO,"STAT LOG: CRI=%d, MAJ=%d, MIN=%d, STOP=%d, NORMAL=%d",
				stSysStat[i].uiCri, stSysStat[i].uiMaj, stSysStat[i].uiMin, stSysStat[i].uiStop, stSysStat[i].uiCnt );
		}

		log_print(LOGN_INFO,"STAT LOG: QUERY=%s",sQuery);
		set_dbm_ret(&smsg, DBM_SUCCESS, DBM_END, sizeof(stSysStat));
		memcpy(smsg.data, &stSysStat[0], sizeof(stSysStat));

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -16;
		}
	}

	return 0;
}

int dis_ntam_conf(mml_msg *ml, long long llNID)
{
	int			dRet;
	dbm_msg_t	smsg;
	st_Conf		stConf;

	smsg.common.total_cnt	= 1;
	smsg.common.TotPage		= 1;
	smsg.common.CurPage		= 1;

	smsg.data[0]		= 0x00;

	stConf.sType		= 0;
	stConf.usLogLevel	= g_stLogLevel->usLogLevel[0];
	stConf.usSysNo		= gdSysNo;
	memcpy(smsg.data, &stConf, sizeof(st_Conf));
	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= sizeof(st_Conf);

	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -1;
	}

	return 0;
}


int set_ntam_conf(mml_msg *ml, long long llNID)
{
	int			i, dRet;
	char		szName[DEF_MAX_NAMELEN];
	size_t		szLen;
	dbm_msg_t	smsg;

	st_LogLevel_List	stList;

	smsg.common.TotPage	= 0;
	smsg.common.CurPage	= 0;

	smsg.data[0]		= 0x00;

	sprintf(szName, "%s", ml->msg_body[0].para_cont);
	szLen	= strlen(szName);
	for(i = 0; i < szLen; i++)
		szName[i] = toupper(szName[i]);

	if(strcmp(szName, "LOGLVL") == 0)
	{
		stList.dCount = MAX_SW_COUNT;
		for(i = 0; i < stList.dCount; i++)
		{
			stList.stLogLevel.usLogLevel[i] = atoi(ml->msg_body[1].para_cont);
			log_print(LOGN_INFO, LH"gLogLevel->usLogLevel[%d]=[%d]",
				LT, i, stList.stLogLevel.usLogLevel[i]);
		}

		if( (dRet = dLogWrite(&stList)) < 0)
		{
			szLen = strlen(smsg.data) + 1;
			smsg.common.mml_err		= eDBQUERYERROR;
			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= szLen;
			log_print(LOGN_INFO, LH"ERROR IN dLogWrite() dRet[%d]", LT, dRet);

			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -2;
			}
			return -3;
		}
		else
		{
			szLen = strlen(smsg.data) + 1;
			smsg.common.mml_err		= DBM_SUCCESS;
			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= szLen;

			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -4;
			}

			if( (dRet = dInitLogLvl()) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dInitLogLvl() dRet[%d]", LT, dRet);
				return -5;
			}

			for(i = 0; i < dRet; i++)
				log_print(LOGN_INFO, LH"gLogLevel->usLogLevel[%d]=[%d]", LT, i, g_stLogLevel->usLogLevel[i]);
		}
	}
	else
	{
		smsg.common.mml_err		= eBadParameter;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
	}

	return 0;
}

int dis_ntam_alm(mml_msg *ml, long long llNID)
{
	int					dRet;
	dbm_msg_t			smsg;
	st_AlmLevel_List	stList;

	smsg.common.total_cnt	= 1;
	smsg.common.TotPage		= 1;
	smsg.common.CurPage		= 1;

	memset(smsg.data, 0x00, sizeof(smsg.data));

	/** CPU, MEM, DISK, QUE, TABSPC **/
	stList.dCount	= ALM_CNT;
	sprintf(stList.stAlmLevel[CPU_LOC].szTypeName, "CPU");
	stList.stAlmLevel[CPU_LOC].sCriticalLevel	= keepalive->stTAMLoad.cpu.usCritical;
	stList.stAlmLevel[CPU_LOC].sMajorLevel		= keepalive->stTAMLoad.cpu.usMajor;
	stList.stAlmLevel[CPU_LOC].sMinorLevel		= keepalive->stTAMLoad.cpu.usMinor;

	sprintf(stList.stAlmLevel[MEM_LOC].szTypeName, "MEM");
	stList.stAlmLevel[MEM_LOC].sCriticalLevel	= keepalive->stTAMLoad.mem.usCritical;
	stList.stAlmLevel[MEM_LOC].sMajorLevel		= keepalive->stTAMLoad.mem.usMajor;
	stList.stAlmLevel[MEM_LOC].sMinorLevel		= keepalive->stTAMLoad.mem.usMinor;

	sprintf(stList.stAlmLevel[DISK_LOC].szTypeName, "DISK");
	stList.stAlmLevel[DISK_LOC].sCriticalLevel	= keepalive->stTAMLoad.disk.usCritical;
	stList.stAlmLevel[DISK_LOC].sMajorLevel		= keepalive->stTAMLoad.disk.usMajor;
	stList.stAlmLevel[DISK_LOC].sMinorLevel		= keepalive->stTAMLoad.disk.usMinor;

	sprintf(stList.stAlmLevel[QUE_LOC].szTypeName, "QUE");
	stList.stAlmLevel[QUE_LOC].sCriticalLevel	= keepalive->stTAMLoad.que.usCritical;
	stList.stAlmLevel[QUE_LOC].sMajorLevel		= keepalive->stTAMLoad.que.usMajor;
	stList.stAlmLevel[QUE_LOC].sMinorLevel		= keepalive->stTAMLoad.que.usMinor;

	sprintf(stList.stAlmLevel[NIFO_LOC].szTypeName, "NIFO");
	stList.stAlmLevel[NIFO_LOC].sCriticalLevel	= keepalive->stTAMLoad.nifo.usCritical;
	stList.stAlmLevel[NIFO_LOC].sMajorLevel		= keepalive->stTAMLoad.nifo.usMajor;
	stList.stAlmLevel[NIFO_LOC].sMinorLevel		= keepalive->stTAMLoad.nifo.usMinor;

	sprintf(stList.stAlmLevel[SWCPU_LOC].szTypeName, "SW_CPU");
	stList.stAlmLevel[SWCPU_LOC].sCriticalLevel	= keepalive->stSWCHLoad.cpu.usCritical;
	stList.stAlmLevel[SWCPU_LOC].sMajorLevel	= keepalive->stSWCHLoad.cpu.usMajor;
	stList.stAlmLevel[SWCPU_LOC].sMinorLevel	= keepalive->stSWCHLoad.cpu.usMinor;

	sprintf(stList.stAlmLevel[SWMEM_LOC].szTypeName, "SW_MEM");
	stList.stAlmLevel[SWMEM_LOC].sCriticalLevel	= keepalive->stSWCHLoad.mem.usCritical;
	stList.stAlmLevel[SWMEM_LOC].sMajorLevel	= keepalive->stSWCHLoad.mem.usMajor;
	stList.stAlmLevel[SWMEM_LOC].sMinorLevel	= keepalive->stSWCHLoad.mem.usMinor;

	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= sizeof(st_AlmLevel_List);

	memcpy(smsg.data, &stList, sizeof(st_AlmLevel_List));
	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -1;
	}

	return 0;
}

int set_ntam_alm(mml_msg *ml, long long llNID)
{
	int				i, dRet;
	size_t			szSlen;
	unsigned int	uiCriLvl, uiMinLvl, uiMajLvl;
	char			szName[DEF_MAX_NAMELEN];
	dbm_msg_t		smsg;
	size_t			szLen;

	smsg.common.TotPage	= 0;
	smsg.common.CurPage	= 0;
	smsg.data[0]		= 0x00;

	sprintf(szName, "%s", ml->msg_body[0].para_cont);

	uiCriLvl	= atoi(ml->msg_body[1].para_cont);
	uiMajLvl	= atoi(ml->msg_body[2].para_cont);
	uiMinLvl	= atoi(ml->msg_body[3].para_cont);
	log_print(LOGN_DEBUG, LH"CRI[%d] MAJOR[%d] MINOR[%d]", LT, uiCriLvl, uiMajLvl,uiMinLvl);

	if( (uiCriLvl <= uiMajLvl) || (uiMajLvl <= uiMinLvl))
	{
		log_print(LOGN_CRI, LH"ERROR IN RANGE NAME[%s] CRI[%u] MAJOR[%u] MINOR[%u]", LT, szName, uiCriLvl, uiMajLvl, uiMinLvl);

		szSlen					= strlen(smsg.data);
		smsg.common.mml_err		= eINVALID_PARAM_RANGE;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= szSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	szLen = strlen(szName);
	for(i = 0; i < szLen; i++)
		szName[i] = toupper(szName[i]);

	if( (strcmp("CPU", szName) != 0) && (strcmp("MEM", szName) != 0) &&
		(strcmp("DISK", szName) != 0) && (strcmp("NIFO", szName) != 0) &&
		(strcmp("QUE", szName) != 0) && (strcmp("SWCPU", szName) != 0) &&
		(strcmp("SWMEM", szName) != 0))
	{
		log_print(LOGN_CRI, LH"ERROR IN NAME[%s]", LT, szName);

		szLen					= strlen(smsg.data);
		smsg.common.mml_err		= eBadParameter;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= szLen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if(strcmp("CPU", szName) == 0)
	{
		keepalive->stTAMLoad.cpu.usCritical  = uiCriLvl;
		keepalive->stTAMLoad.cpu.usMajor     = uiMajLvl;
		keepalive->stTAMLoad.cpu.usMinor     = uiMinLvl;
	}
	else if(strcmp("MEM", szName) == 0)
	{
		keepalive->stTAMLoad.mem.usCritical  = uiCriLvl;
		keepalive->stTAMLoad.mem.usMajor     = uiMajLvl;
		keepalive->stTAMLoad.mem.usMinor     = uiMinLvl;
	}
	else if(strcmp("DISK", szName) == 0)
	{
		keepalive->stTAMLoad.disk.usCritical = uiCriLvl;
		keepalive->stTAMLoad.disk.usMajor    = uiMajLvl;
		keepalive->stTAMLoad.disk.usMinor    = uiMinLvl;
	}
	else if( 0 == strcmp("QUE", szName))
	{
		keepalive->stTAMLoad.que.usCritical  = uiCriLvl;
		keepalive->stTAMLoad.que.usMajor     = uiMajLvl;
		keepalive->stTAMLoad.que.usMinor     = uiMinLvl;
	}
	else if( 0 == strcmp("NIFO", szName))
	{
		keepalive->stTAMLoad.nifo.usCritical  = uiCriLvl;
		keepalive->stTAMLoad.nifo.usMajor     = uiMajLvl;
		keepalive->stTAMLoad.nifo.usMinor     = uiMinLvl;
	}
	if(strcmp("SWCPU", szName) == 0)
	{
		keepalive->stSWCHLoad.cpu.usCritical  = uiCriLvl;
		keepalive->stSWCHLoad.cpu.usMajor     = uiMajLvl;
		keepalive->stSWCHLoad.cpu.usMinor     = uiMinLvl;
	}
	else if(strcmp("SWMEM", szName) == 0)
	{
		keepalive->stSWCHLoad.mem.usCritical  = uiCriLvl;
		keepalive->stSWCHLoad.mem.usMajor     = uiMajLvl;
		keepalive->stSWCHLoad.mem.usMinor     = uiMinLvl;
	}

	if( (dRet = dAlmWrite_NTAM(keepalive)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dAlmWrite_NTAM() dRet[%d]", LT, dRet);

		szLen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= szLen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

	szLen					= 0;
	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= szLen;
	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -7;
	}

	return 0;
}

int dis_ntaf_conf(mml_msg *ml, long long llNID, int uIndex)
{
	int			dRet;
	size_t		dSlen;
	st_MsgQ		stMsgQ;
	pst_MsgQSub	pstMsgQSub;
	dbm_msg_t	smsg;

	smsg.common.total_cnt	= 1;
	smsg.common.TotPage		= 1;
	smsg.common.CurPage		= 1;

	stMsgQ.ucNTAFID	= atoi(ml->msg_body[0].para_cont);

	/* VALID CHECK WNTAF ID */
	if(pstSubSys->sys[stMsgQ.ucNTAFID-1].usSysNo != stMsgQ.ucNTAFID)
	{
		log_print(LOGN_CRI, LH"ERROR IN PARAMETER stMsgQ.ucNTAFID[%d], usSysNo[%d]", 
			LT, stMsgQ.ucNTAFID, pstSubSys->sys[stMsgQ.ucNTAFID-1].usSysNo);

		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eINVALID_SUBSYSTEM;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	pstMsgQSub	= (pst_MsgQSub)&stMsgQ.llMType;
	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usSvcID	= SID_CHKREQ;
	pstMsgQSub->usMsgID	= MID_DIS_NTAF_CONF;

	stMsgQ.llIndex		= uIndex;
	stMsgQ.usBodyLen	= NTAFT_HEADER_LEN;
	stMsgQ.ucProID		= SEQ_PROC_S_MNG;
	stMsgQ.dMsgQID		= 0;

	if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_SI_SVC)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg() dRet[%d]", LT, dRet);
		return -3;
	}
	g_stMmlsg.src_func[uIndex]	= ml->src_func;
	g_stMmlsg.src_proc[uIndex]	= ml->src_proc;
	g_stMmlsg.cmd_id[uIndex]	= ml->cmd_id;
	g_stMmlsg.msg_id[uIndex]	= ml->msg_id;

	return 0;
}

int dis_ntaf_alm(mml_msg *ml, long long llNID,int uIndex)
{
	int					i, j, dRet, dSlen;
	dbm_msg_t			smsg;
	st_AlmLevel_List	stList, *pstList;

	smsg.common.total_cnt	= 1;
	smsg.common.TotPage		= 1;
	smsg.common.CurPage		= 1;

	memcpy(&stList, &stNtafAlarm[atoi(ml->msg_body[0].para_cont)-1], sizeof(st_AlmLevel_List));
	for(i = 0; i < MAX_NTAF_NUM; i++)
	{
		pstList = &stNtafAlarm[i];
		if(pstList->dCount > 0)
		{
			log_print(LOGN_INFO, LH"POS=%d COUNT=%d SYSNO=%d", LT, i, pstList->dCount, pstList->dSysNo);
			for(j = 0; j < pstList->dCount; j++)
			{
				log_print(LOGN_INFO, LH"TYPE=%s] CRI=%u MAJ=%u MIN=%u", LT, pstList->stAlmLevel[j].szTypeName,
					pstList->stAlmLevel[j].sCriticalLevel, pstList->stAlmLevel[j].sMajorLevel,
					pstList->stAlmLevel[j].sMinorLevel);
			}
		}
	}
	log_print(LOGN_INFO, LH"dCounc[%d],dSysno[%d],param[%d],size[%lu]", LT, stList.dCount, stList.dSysNo,
		atoi(ml->msg_body[0].para_cont),sizeof(st_AlmLevel_List));

	if(stList.dSysNo < 1)
	{
		smsg.common.mml_err		= eINVALID_SUBSYSTEM;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	memcpy(smsg.data, &stList, sizeof(st_AlmLevel_List));
	dSlen = sizeof(st_AlmLevel_List);
	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= dSlen;
	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -3;
	}

	return 0;
}

int set_ntaf_conf(mml_msg *ml, long long llNID)
{
	int			i, dRet;
	size_t		szLen;
	char		szName[DEF_MAX_NAMELEN];
	dbm_msg_t	smsg;
	st_Conf		stConf;
	st_MsgQ		stMsgQ;
	pst_MsgQSub	pstMsgQSub;

	smsg.common.total_cnt	= 1;
	smsg.common.TotPage		= 1;
	smsg.common.CurPage		= 1;

	smsg.data[0] = 0x00;

	stMsgQ.ucNTAFID	= atoi(ml->msg_body[0].para_cont);

	/* VALID CHECK WNTAF ID */
	if(pstSubSys->sys[stMsgQ.ucNTAFID-1].usSysNo != stMsgQ.ucNTAFID)
	{
		log_print(LOGN_CRI, LH"ERROR IN PARAMETER stMsgQ.ucNTAFID[%d], usSysNo[%d]", 
			LT, stMsgQ.ucNTAFID, pstSubSys->sys[stMsgQ.ucNTAFID-1].usSysNo);

		smsg.common.mml_err		= eINVALID_SUBSYSTEM;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	sprintf(szName, "%s", ml->msg_body[1].para_cont);

	szLen	= strlen(szName);
	for(i = 0; i < szLen; i++)
		szName[i] = toupper(szName[i]);

	pstMsgQSub = (pst_MsgQSub)&stMsgQ.llMType;
	if(strcmp(szName, "LOGLVL") == 0)
	{
		pstMsgQSub->usMsgID	= MID_FLT_LOG;
		stConf.sType		= 1;	/*	msg Type	*/
		stConf.usLogLevel	= atoi(ml->msg_body[2].para_cont);
		stConf.usSysNo		= 0;
		stConf.usDBNo		= 0;
		stConf.usTcpHdr		= 0;
		log_print(LOGN_INFO, LH"SND usLogLevel[%d]", LT, stConf.usLogLevel);
	}
	else
	{
		log_print(LOGN_INFO, LH"ERROR IN parameter[%s]", LT, szName);
		smsg.common.mml_err		= eBadParameter;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if(dSendMMC(ml, &smsg, llNID) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
	}
	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usSvcID	= SID_FLT;

	stMsgQ.usBodyLen	= sizeof(st_Conf)+ NTAFT_HEADER_LEN;
	stMsgQ.ucProID		= SEQ_PROC_S_MNG;
	stMsgQ.dMsgQID		= 0;
	memcpy(&stMsgQ.szBody[NTAFT_HEADER_LEN], &stConf, sizeof(st_Conf));

	if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_SI_SVC)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg() dRet[%d]", LT, dRet);
		return -4;
	}

	szLen					= strlen(smsg.data) + 1;
	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= szLen;

	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -5;
	}

	return 0;
}

int set_ntaf_alm(mml_msg *ml, long long llNID, int uIndex)
{
	int			i, dRet, dSysNo, dIDX, dCri, dMaj, dMin;
	size_t		dSlen;
	char		szName[DEF_MAX_NAMELEN];
	dbm_msg_t	smsg;

	smsg.data[0]			= 0x00;
	smsg.common.total_cnt	= 1;
	smsg.common.TotPage		= 1;
	smsg.common.CurPage		= 1;

	dSysNo	= atoi(ml->msg_body[0].para_cont);
	strcpy(szName, ml->msg_body[1].para_cont);
	dCri	= atoi(ml->msg_body[2].para_cont);
	dMaj	= atoi(ml->msg_body[3].para_cont);
	dMin	= atoi(ml->msg_body[4].para_cont);

	dRet = 0;
	if( (dSysNo < 1) || (dSysNo > MAX_NTAF_NUM) || (dCri <= dMaj) || (dMaj <= dMin))
		dRet = eINVALID_PARAM_RANGE;
	else
	{
		for(i = 0; i < strlen(szName); i++)
			szName[i] = toupper(szName[i]);

		if(strcmp("CPU", szName) == 0)
			dIDX = CPU_LOC;
		else if(strcmp("MEM", szName) == 0)
			dIDX = MEM_LOC;
		else if(strcmp("DISK", szName) == 0)
			dIDX = DISK_LOC;
		else if(strcmp("QUE", szName) == 0)
			dIDX = QUE_LOC;
		else if(strcmp("NIFO", szName) == 0)
			dIDX = NIFO_LOC;
		else if(strcmp("TRAFFIC", szName) == 0)
			dIDX = TAF_TRAFFIC_LOC;
		else
			dRet = eBadParameter;
	}

	if(dRet != 0)
	{
		log_print(LOGN_CRI, "## set_ntaf_alm : Failed , NAME =[%s]",szName);
		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= dRet;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	if(stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].szTypeName[0] == 0x00)
		stNtafAlarm[dSysNo - 1].dCount++;

	stNtafAlarm[dSysNo-1].dSysNo	= dSysNo;
	strcpy(stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].szTypeName, szName);
	stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].sCriticalLevel	= dCri;
	stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].sMajorLevel		= dMaj;
	stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].sMinorLevel		= dMin;

	if( (dRet = dAlmWrite_NTAF()) < 0)
	{
		log_print(LOGN_CRI,LH"ERROR IN dAlmWrite_NTAF() dRet[%d]", LT, dRet);
		return -3;
	}

	/*** ALARM LEVEL을 NTAF로 전송 *****/
	if( (dRet = dSend_NTAF_AlarmLevel(dSysNo)) < 0)
	{
		log_print(LOGN_CRI,LH"ERROR IN dSend_NTAF_AlarmLevel(dSysNo[%d]) dRet[%d]", LT, dSysNo, dRet);
		return -4;
	}

	dSlen					= strlen(smsg.data) + 1;
	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= dSlen;
	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -5;
	}

	return 0;
}

/* */
int set_svc_conf(mml_msg *ml, long long llNID)
{
	int				i, dRet;
	dbm_msg_t		smsg;
	st_MsgQ			stMsgQ;
	pst_MsgQSub		pstMsgQSub;
	SVC_CONF		stSvcConf;

	memset(&stSvcConf, 0x00, sizeof(SVC_CONF));

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 29: /* Service ID */
				stSvcConf.dSvcID = atoi(ml->msg_body[i].para_cont);
				log_print(LOGN_DEBUG, "[COMMANDLIST] SVCID[%d]", stSvcConf.dSvcID);
				break;

			case 119: /* ONOFF */
				stSvcConf.dOnOff = atoi(ml->msg_body[i].para_cont);
				if(stSvcConf.dOnOff == 308) /* DQMS_ENUM ON */
					stSvcConf.dOnOff = 1;
				else if(stSvcConf.dOnOff == 309) /* DQMS_ENUM OFF */
					stSvcConf.dOnOff = 0;

				log_print(LOGN_DEBUG, "[COMMANDLIST] ONOFF[%d]", stSvcConf.dOnOff);
				break;
		}
	}

	for(i = 0; i < MAX_SVC_STAT_CNT; i++)
	{
		if(stTafSvclist.stSvcStat[i].dSvcID == stSvcConf.dSvcID)
		{
			stTafSvclist.stSvcStat[i].dOnOff = stSvcConf.dOnOff; 
			break;
		}
	}

	smsg.data[0]	= 0x00;

	if(i == MAX_SVC_STAT_CNT)
		smsg.common.mml_err		= DBM_FAILURE;
	else
		smsg.common.mml_err		= DBM_SUCCESS;

	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= 0;

	smsg.head.src_proc		= SEQ_PROC_S_MNG;
	smsg.head.dst_func		= ml->src_func;
	smsg.head.dst_proc		= ml->src_proc;
	smsg.head.cmd_id		= ml->cmd_id;
	smsg.head.msg_id		= ml->msg_id;

	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -6;
	}

	if(i != MAX_SVC_STAT_CNT)
	{
		/* Write File */
		dWriteSvcStat();

		pstMsgQSub = (pst_MsgQSub)&stMsgQ.llMType;

		pstMsgQSub->usType 	= DEF_SYS;
		pstMsgQSub->usSvcID = SID_FLT;
		pstMsgQSub->usMsgID = MID_FLT_ONOFF;

		stMsgQ.ucNTAFID 	= 101;		/*	NTAF_BROAD_CAST[101]: defined in SRC/SI_SVC/si_svc_def.h	*/
		stMsgQ.usBodyLen 	= sizeof(SVC_CONF) + NTAFT_HEADER_LEN;
		stMsgQ.ucProID 		= SEQ_PROC_S_MNG;
		stMsgQ.dMsgQID 		= 0;

		memcpy(&stMsgQ.szBody[NTAFT_HEADER_LEN], &stSvcConf, sizeof(SVC_CONF));

		if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_SI_SVC)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMsg(SI_SVC[%d]) dRet[%d]", 
				LT , SEQ_PROC_SI_SVC, dRet);
			return -3;
		}

		log_print(LOGN_DEBUG, "[COMMANDLIST] NTAF SERVICE CONFIG SEND SUCCESS");
	}

    return 0;
}

int set_timer_info(mml_msg *ml, long long llNID)
{
	int				i, dSlen, dRet;
	unsigned int	uTimerValue;
	unsigned short	huTimerIdx;
	dbm_msg_t		smsg;
	st_MsgQ			stMsgQ;
	pst_MsgQSub		pstMsgQSub;

	smsg.data[0]		= 0x00;

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 708:
				huTimerIdx = (unsigned short)atoi(ml->msg_body[i].para_cont);
				switch(huTimerIdx)
				{
					case RPPI_CALL_TIMEOUT:
					case RPPI_WAIT_TIMEOUT:
					case PI_VT_TIMEOUT:
					case PI_IM_TIMEOUT:
					case PI_TCP_RSTWAIT:
					case PI_TCP_TIMEOUT:
					case PI_DNS_TIMEOUT:
					case PI_SIP_TIMEOUT:
					case PI_MSRP_TIMEOUT:
					case PI_RAD_TIMEOUT:
					case PI_DIA_TIMEOUT:
					case PI_CALL_TIMEOUT:
					case PI_WAIT_TIMEOUT:
					case PI_DORM_TIMEOUT:
					case RP_CALL_TIMEOUT:
					case RP_DORM_TIMEOUT:
					case PI_INET_TIMEOUT: /* added by uamyd 20110419 */
					case PI_RCALL_TIMEOUT: /* added by uamyd 20110419 */
					case RP_RCALL_TIMEOUT: /* added by uamyd 20110419 */
					case PI_RCALL_SIGWAIT:
					case RP_RCALL_SIGWAIT:
						log_print(LOGN_DEBUG, LH"huTimerIdx para_id[%hu] para_cont[%s]", LT,
							ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
						break;
					default:
						log_print(LOGN_CRI, LH"ERROR IN Parameter para_id[%hu] para_cont[%s]", LT,
							ml->msg_body[i].para_id, ml->msg_body[i].para_cont);

						dSlen					= strlen(smsg.data) + 1;
						smsg.common.mml_err		= eBadParameter;
						smsg.common.cont_flag	= DBM_END;
						smsg.head.msg_len		= dSlen;

						if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
						{
							log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
							return -1;
						}
						return -2;
				}
				break;
			case 709:
				uTimerValue = (unsigned int)atoi(ml->msg_body[i].para_cont);
				log_print(LOGN_DEBUG, LH"uTimerValue para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
				break;
			default:
				log_print(LOGN_CRI, LH"ERROR IN Parameter para_id[%hu]", LT, ml->msg_body[i].para_id);
		}
	}
	smsg.data[0]	= 0x00;

	flt_info->stTimerInfo.usTimerInfo[huTimerIdx] = uTimerValue;

	if( (dRet = dWriteTimerFile(&flt_info->stTimerInfo)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dWriteTimerInfo() dRet[%d]", LT, dRet);
		return -5;
	}

	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= 0;

	smsg.head.src_proc		= SEQ_PROC_S_MNG;
	smsg.head.dst_func		= ml->src_func;
	smsg.head.dst_proc		= ml->src_proc;
	smsg.head.cmd_id		= ml->cmd_id;
	smsg.head.msg_id		= ml->msg_id;

	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -6;
	}

	pstMsgQSub = (pst_MsgQSub)&stMsgQ.llMType;

	pstMsgQSub->usType 	= DEF_SYS;
	pstMsgQSub->usSvcID = SID_FLT;
	pstMsgQSub->usMsgID = MID_FLT_TIMER;

	stMsgQ.ucNTAFID 	= 101;		/*	NTAF_BROAD_CAST[101]: defined in SRC/SI_SVC/si_svc_def.h	*/
	stMsgQ.usBodyLen 	= sizeof(TIMER_INFO) + NTAFT_HEADER_LEN;
	stMsgQ.ucProID 		= SEQ_PROC_S_MNG;
	stMsgQ.dMsgQID 		= 0;

	memcpy(&stMsgQ.szBody[NTAFT_HEADER_LEN], &flt_info->stTimerInfo, sizeof(TIMER_INFO));

	if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_SI_SVC)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg(SI_SVC[%d]) dRet[%d]", 
			LT, SEQ_PROC_SI_SVC, dRet);
		return -3;
	}
	log_print(LOGN_DEBUG, LH"NTAF TIMER SEND SUCCESS", LT);

    return 0;
}

int	dis_svc_conf(mml_msg *ml, long long llNID)
{
	dbm_msg_t	smsg;
	char		szBuf[MSG_DATA_LEN];
	int			dRet, dSlen;

	memset(szBuf, 0x00, MSG_DATA_LEN);
	smsg.data[0] = 0x00;

	sprintf(szBuf, "  --------------------------------------------------------------------------");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  SERVICE_ID     STAT");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  --------------------------------------------------------------------------");
	strcat(smsg.data, szBuf);

	sprintf(szBuf, "\n  %-15s%s", "SVC_MENU", stTafSvclist.stSvcStat[0].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_DN", stTafSvclist.stSvcStat[1].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_STREAM", stTafSvclist.stSvcStat[2].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_MMS", stTafSvclist.stSvcStat[3].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_WIDGET", stTafSvclist.stSvcStat[4].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_PHONE", stTafSvclist.stSvcStat[5].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_EMS", stTafSvclist.stSvcStat[6].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_BANK", stTafSvclist.stSvcStat[7].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_FV", stTafSvclist.stSvcStat[8].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_IM", stTafSvclist.stSvcStat[9].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_VT", stTafSvclist.stSvcStat[10].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_ETC", stTafSvclist.stSvcStat[11].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_CORP", stTafSvclist.stSvcStat[12].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_INET", stTafSvclist.stSvcStat[13].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_FB", stTafSvclist.stSvcStat[14].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n  %-15s%s", "SVC_IV", stTafSvclist.stSvcStat[15].dOnOff == 1 ? "ON" : "OFF");
	strcat(smsg.data, szBuf);

	sprintf(szBuf, "\n  --------------------------------------------------------------------------");
	strcat(smsg.data, szBuf);

	dSlen = strlen(smsg.data) + 1;
	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= dSlen;

	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -1;
	}

	return 1;
}

int	dis_timer_info(mml_msg *ml, long long llNID)
{
	dbm_msg_t	smsg;
	char		szBuf[MSG_DATA_LEN];
	int			dRet, dSlen;

	memset(szBuf, 0x00, MSG_DATA_LEN);
	smsg.data[0] = 0x00;

	sprintf(szBuf, " ---------------------------------------------------------------------------------------");
	strcat(smsg.data, szBuf);

	sprintf(szBuf, "\n %17s\t = %u", "RPPI_CALL_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[RPPI_CALL_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "RPPI_WAIT_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[RPPI_WAIT_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_VT_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_VT_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_IM_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_IM_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_TCP_RSTWAIT", flt_info->stTimerInfo.usTimerInfo[PI_TCP_RSTWAIT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_TCP_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_TCP_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_DNS_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_DNS_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_SIP_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_SIP_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_MSRP_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_MSRP_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_RAD_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_RAD_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_DIA_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_DIA_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_CALL_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_WAIT_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_WAIT_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_DORM_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_DORM_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "RP_CALL_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[RP_CALL_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "RP_DORM_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[RP_DORM_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_INET_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_INET_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_RCALL_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[PI_RCALL_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "RP_RCALL_TIMEOUT", flt_info->stTimerInfo.usTimerInfo[RP_RCALL_TIMEOUT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "PI_RCALL_SIGWAIT", flt_info->stTimerInfo.usTimerInfo[PI_RCALL_SIGWAIT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n %17s\t = %u", "RP_RCALL_SIGWAIT", flt_info->stTimerInfo.usTimerInfo[RP_RCALL_SIGWAIT]);
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n ---------------------------------------------------------------------------------------");
	strcat(smsg.data, szBuf);

	dSlen = strlen(smsg.data) + 1;
	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= dSlen;

	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -1;
	}

	return 0;
}
int ntaf_tm_sess(mml_msg *ml, long long llNID)
{
	int			dRet, dSysID;
	dbm_msg_t	smsg;
	pst_MsgQ	pstMsgQ;
	pst_MsgQSub	pstMsgQSub;
	U8*			pNODE;

	dSysID = atoi(ml->msg_body[0].para_cont);

	/* VALID CHECK TAF ID */
	if( pstSubSys->sys[dSysID-1].usSysNo != dSysID )
	{
		smsg.common.mml_err		= eINVALID_SUBSYSTEM;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		log_print(LOGN_DEBUG, LH"INVALID SYSID=%d", LT, dSysID);
		return -2;
	}

	pNODE = nifo_node_alloc(pMEMSINFO);
	if( pNODE == NULL ){
		log_print(LOGN_WARN, LH"FAILED IN nifo_node_alloc"EH, LT, ET);
		return -1;
	}
	
	pstMsgQ = (pst_MsgQ)nifo_tlv_alloc(pMEMSINFO, pNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF);
	if( pstMsgQ == NULL ){
		log_print(LOGN_CRI, LH"FAILED IN nifo_tlv_alloc, return  NULL", LT);
		nifo_node_delete(pMEMSINFO, pNODE);
		return -2;
	}

	pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;

	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usSvcID	= SID_MML;
	pstMsgQSub->usMsgID	= MID_MML_REQ;

	pstMsgQ->ucNTAFID	= dSysID;
	log_print(LOGN_INFO, "NTAF SYSNO [%d]", pstMsgQ->ucNTAFID);

	pstMsgQ->usBodyLen	= sizeof(mml_msg)+ NTAFT_HEADER_LEN;
	pstMsgQ->ucProID	= SEQ_PROC_S_MNG;
	memcpy(&pstMsgQ->szBody[NTAFT_HEADER_LEN], ml, sizeof(mml_msg));

	if( gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_S_MNG, SEQ_PROC_SI_SVC, nifo_offset(pMEMSINFO, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN gifo_write(S_MNG:%d > SI_SVC:%d), offset=%ld"EH,
			LT, SEQ_PROC_S_MNG, SEQ_PROC_SI_SVC, nifo_offset(pMEMSINFO, pNODE), ET);
		nifo_node_delete(pMEMSINFO, pNODE);
		usleep(0);
		return -3;
	}

	log_print(LOGN_DEBUG,LH"SUCCESS SEND TO SI_SVC", LT);

	return 0;
}

/*
	일정 시간동안 trace한 내용들에 대하여 trace를 정지하도록 각각의 TAF에 내려주고, 해당 st_TraceList에서 삭제
	 - Writer: Han-jin Park
	 - DAte: 2008.09.19
*/
int stop_trc_info(st_TraceList *pTraceList, int count)
{
	int				i, dRet;

	if(!pTraceList->count)
	{
		log_print(LOGN_CRI, LH"Can't process when pTraceList->count[%02d] is 0[Zero].", LT, pTraceList->count);
		return -1;
	}
	else
		log_print(LOGN_INFO, LH"DELETE Time Expired TRACE INDEX[count:%02d, pTraceList->count:%02d]", LT, count, pTraceList->count);

	for(i = count; i < pTraceList->count; i++)
	{
		if( (i+1) > (MAX_TRACE_CNT-1))
			break;
		memcpy(&pTraceList->stTraceInfo[i], &pTraceList->stTraceInfo[i+1], sizeof(pTraceList->stTraceInfo[i]));
	}
	memset(&pTraceList->stTraceInfo[pTraceList->count], 0x00, sizeof(pTraceList->stTraceInfo[pTraceList->count]));

	pTraceList->count--;

	if( (dRet = dWriteTraceTblList(FILE_TRACE_TBL, trace_tbl)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dWriteTraceTblList(%s) dRet[%d]", LT, FILE_TRACE_TBL, dRet);
		return -2;
	}
	log_print(LOGN_DEBUG, LH"SUCCEED IN dWriteTraceTblList(%s)", LT, FILE_TRACE_TBL);

	for(i = 0; i < MAX_NTAF_NUM; i++)
	{
		if(pstSubSys->sys[i].uiIP > 0)
		{
			if( (dRet = dSendTrcToNtaf(pstSubSys->sys[i].usSysNo)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendTrcToNtaf() dRet[%d]", LT, dRet);
				return -3;
			}
		}
	}

	return 1;
}

int dis_trc_info(mml_msg *ml, long long llNID)
{
	int			i, j, k, dLoop, dRet;
	dbm_msg_t	smsg;

	smsg.data[0]			= 0x00;
	smsg.common.total_cnt	= 1;
	smsg.common.TotPage		= 1;
	smsg.common.CurPage		= 1;

	trace_tbl->dSysNo = flt_info->stTmfInfo.usTmfID;
	if(!trace_tbl->count)
	{
		memcpy(smsg.data, trace_tbl, sizeof(*trace_tbl));
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.src_proc		= SEQ_PROC_S_MNG;

		smsg.head.dst_func	= ml->src_func;
		smsg.head.dst_proc	= ml->src_proc;
		smsg.head.cmd_id	= ml->cmd_id;
		smsg.head.msg_id	= ml->msg_id;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	if(trace_tbl->count % 25)
		dLoop = trace_tbl->count / 25 + 1;
	else
		dLoop = trace_tbl->count / 25;

	for(i = 0; i < dLoop; i++)
	{
		memcpy(&smsg.data[0], &trace_tbl[0], 2*sizeof(int));

		for(j = i*25, k = 0; (j < trace_tbl->count) && (k < 25); j++, k++)
		{
			memcpy(&smsg.data[8+sizeof(st_TraceInfo)*k], &trace_tbl->stTraceInfo[j], sizeof(st_TraceInfo));

			if(trace_tbl->stTraceInfo[j].dType == TRC_TYPE_IMSI)
				log_print(LOGN_INFO, LH"[%d] [IMSI] : [%s]", LT, j, trace_tbl->stTraceInfo[j].stTraceID.szMIN);
			else if(trace_tbl->stTraceInfo[j].dType == TRC_TYPE_MDN)
				log_print(LOGN_INFO, LH"[%d] [MSISDN] : [%s]", LT, j, trace_tbl->stTraceInfo[j].stTraceID.szMIN);
		}
		smsg.common.mml_err		= DBM_SUCCESS;

		if(i+1 == dLoop)
			smsg.common.cont_flag	= DBM_END;
		else
			smsg.common.cont_flag	= DBM_CONTINUE;

		smsg.head.msg_len	= (2*sizeof(int))+(sizeof(st_TraceInfo)*k);
		smsg.head.src_proc	= SEQ_PROC_S_MNG;
		smsg.head.dst_func	= ml->src_func;
		smsg.head.dst_proc	= ml->src_proc;
		smsg.head.cmd_id	= ml->cmd_id;
		smsg.head.msg_id	= ml->msg_id;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
	}

	return 0;
}

int add_trc_info(mml_msg *ml, long long llNID)
{
	int			i, dSlen, dRet, dErrorFlag, dTrcType;
	dbm_msg_t	smsg;
	char		szIP[32];

	dErrorFlag		= 0;
	dTrcType		= 0;
	smsg.data[0]	= 0x00;

	if(trace_tbl->count == MAX_TRACE_CNT)
	{
		sprintf(smsg.data, "\n OVER MAX TRACE COUNT(%d)", MAX_TRACE_CNT);

		dSlen = strlen(smsg.data) + 1;
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	dTrcType = atoi(ml->msg_body[0].para_cont);
	switch ( dTrcType ){
		case TRC_TYPE_IMSI:
		case TRC_TYPE_MDN:
		case TRC_TYPE_ROAM_IMSI:
		case TRC_TYPE_ROAM_MDN:
			for(i = 0; i < trace_tbl->count; i++)
			{
				log_print(LOGN_DEBUG, LH"trace_tbl->stTraceInfo[%d].stTraceID.szMIN = [%s]", LT, i, trace_tbl->stTraceInfo[i].stTraceID.szMIN);

				if( (trace_tbl->stTraceInfo[i].dType == dTrcType) && (strcmp((char*)trace_tbl->stTraceInfo[i].stTraceID.szMIN, ml->msg_body[1].para_cont) == 0))
				{
					log_print(LOGN_DEBUG, LH"trace_tbl->stTraceInfo[%d].stTraceID.szMIN[%s] is existed. BREAK", LT, i, trace_tbl->stTraceInfo[i].stTraceID.szMIN);
					dErrorFlag = 2;
					break;
				}
			}
			break;
		default :
			log_print(LOGN_DEBUG, LH"INVALID PARAMETER[%s]", LT, ml->msg_body[1].para_cont);
			dErrorFlag = 1;
			break;
			
	}

	log_print(LOGN_DEBUG, LH"ml->msg_body[0].para_cont = [ENUM:%s]:[%s]", LT, ml->msg_body[0].para_cont, 
						TRC_TYPE_IMSI == dTrcType ? "IMSI" : TRC_TYPE_MDN == dTrcType ? "CTN" : TRC_TYPE_ROAM_IMSI == dTrcType ? "ROAM_IMSI" : "ROAM_CTN" );
	log_print(LOGN_DEBUG, LH"ml->msg_body[1].para_cont = [VALUE:%s]", LT, ml->msg_body[1].para_cont);

	switch(dErrorFlag)
	{
		case 1:
			smsg.common.mml_err		= eBadParameter;
			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= 0;

			smsg.head.src_proc	= SEQ_PROC_S_MNG;
			smsg.head.dst_func	= ml->src_func;
			smsg.head.dst_proc	= ml->src_proc;
			smsg.head.cmd_id	= ml->cmd_id;
			smsg.head.msg_id	= ml->msg_id;

			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -3;
			}
			return -4;
		case 2:
			sprintf(smsg.data, "\n ALREADY REGISTERED TRACE VALUE(%s)", ml->msg_body[1].para_cont);
			dSlen = strlen(smsg.data) + 1;

			smsg.common.mml_err		= DBM_SUCCESS;
			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= dSlen;

			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -5;
			}
			return -6;
		default:
			log_print(LOGN_CRI, LH"SUCCEED in parameter check[1:%s][2:%s]", LT, ml->msg_body[0].para_cont, ml->msg_body[1].para_cont);
			break;
	}
	trace_tbl->stTraceInfo[trace_tbl->count].dType = dTrcType;
	sprintf((char*)trace_tbl->stTraceInfo[trace_tbl->count].stTraceID.szMIN, "%s", ml->msg_body[1].para_cont);

	switch(dTrcType)
	{
		case TRC_TYPE_IMSI:
			log_print(LOGN_INFO, LH"%s[%s]", LT,
				"IMSI", trace_tbl->stTraceInfo[trace_tbl->count].stTraceID.szMIN);
			break;
		case TRC_TYPE_MDN:
			log_print(LOGN_INFO, LH"%s[%s]", LT,
				"CTN", trace_tbl->stTraceInfo[trace_tbl->count].stTraceID.szMIN);
			break;
		case TRC_TYPE_ROAM_IMSI:
			log_print(LOGN_INFO, LH"%s[%s]", LT,
				"ROAM_IMSI", trace_tbl->stTraceInfo[trace_tbl->count].stTraceID.szMIN);
			break;
		case TRC_TYPE_ROAM_MDN:
			log_print(LOGN_INFO, LH"%s[%s]", LT,
				"ROAM_CTN", trace_tbl->stTraceInfo[trace_tbl->count].stTraceID.szMIN);
			break;
		default:
			log_print(LOGN_INFO, LH"%s[%s]", LT,
				"UNKNOWN", trace_tbl->stTraceInfo[trace_tbl->count].stTraceID.szMIN);
			break;
	}

	/*
		추가된 trace_tbl 구조체의 내용에 대한 작업한다.
		 - Writer: HAN-JIN PARK
		 - DAte: 2008.09.19
	*/
	/*	add-trc-info의 명령의 2번째 인자값이 없을 경우, default 값으로 2시간을 설정	*/
	if(atoi(ml->msg_body[2].para_cont) == 0)
		trace_tbl->stTraceInfo[trace_tbl->count].usEstimatedTime = 2;
	else
		trace_tbl->stTraceInfo[trace_tbl->count].usEstimatedTime = (unsigned short)atoi(ml->msg_body[2].para_cont);

	/*	trace_tbl의 tExpiredTime에 trace 하는 종료 시간값을 time_t 분 단위의 값으로 저장	*/
	trace_tbl->stTraceInfo[trace_tbl->count].tExpiredTime = ((time(NULL)/SEC_OF_MIN)*SEC_OF_MIN) + (trace_tbl->stTraceInfo[trace_tbl->count].usEstimatedTime * SEC_OF_HOUR);

	/*	trace 명령을 실행한 adminid를 저장	*/
	trace_tbl->stTraceInfo[trace_tbl->count].adminID[0] = 0x00;
	strncpy((char*)trace_tbl->stTraceInfo[trace_tbl->count].adminID, ml->adminid, MAX_USER_NAME_LEN - 1);
	if(strlen(ml->adminid) >= MAX_USER_NAME_LEN)
		trace_tbl->stTraceInfo[trace_tbl->count].adminID[MAX_USER_NAME_LEN - 1] = 0x00;

	log_print(LOGN_INFO, LH"[count = %d] [IP = %s] [Estimated Time = %hu] [Expired Time = %d] [Admin: %s]", 
		LT,
		trace_tbl->count, util_cvtipaddr(szIP, trace_tbl->stTraceInfo[trace_tbl->count].stTraceID.uIP),
		trace_tbl->stTraceInfo[trace_tbl->count].usEstimatedTime, trace_tbl->stTraceInfo[trace_tbl->count].tExpiredTime,
		trace_tbl->stTraceInfo[trace_tbl->count].adminID);
	trace_tbl->count++;

	/*
		실제로 변경되어진 trace_tbl 구조체의 내용을 파일로 기록하여, 전체 프로세스가 새로
		시작되는 경우에 파일로부터 정보를 가져오기 위해 기록한다.
		 - Writer: HAN-JIN PARK
		 - Date: 2008.09.19
	*/
	if( (dRet = dWriteTraceTblList(FILE_TRACE_TBL, trace_tbl)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dWriteTraceTblList(%s ) dRet:%d)", 
			LT, FILE_TRACE_TBL, dRet);
		return -7;
	}

	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= 0;

	smsg.head.src_proc		= SEQ_PROC_S_MNG;
	smsg.head.dst_func		= ml->src_func;
	smsg.head.dst_proc		= ml->src_proc;
	smsg.head.cmd_id		= ml->cmd_id;
	smsg.head.msg_id		= ml->msg_id;

	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", 
			LT, dRet);
		return -8;
	}

	for(i = 0; i < MAX_NTAF_NUM; i++)
    {
        if(pstSubSys->sys[i].uiIP > 0)
        {
            if( (dRet = dSendTrcToNtaf(pstSubSys->sys[i].usSysNo)) < 0)
            {
                log_print(LOGN_CRI, LH"ERROR IN dSendTrcToNtaf(usSysNo[%d][%hu]) dRet[%d]", 
					LT, i, pstSubSys->sys[i].usSysNo, dRet);
                return -9;
            }
        }
    }

    return 0;
}

int del_trc_info(mml_msg *ml, long long llNID)
{
	int			i, dRet, dSlen, dDelIdx, dErrorFlag, dTrcType;
	dbm_msg_t	smsg;

	dDelIdx			= 0;
	dErrorFlag		= 0;
	smsg.data[0]	= 0x00;

	if(trace_tbl->count == 0)
	{
		sprintf(smsg.data, "\n NOT EXIST TRACE INFORMATION");
		dSlen = strlen(smsg.data) + 1;

		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	dTrcType = atoi(ml->msg_body[0].para_cont);
	switch( dTrcType ){
		case TRC_TYPE_IMSI:
		case TRC_TYPE_MDN:
		case TRC_TYPE_ROAM_IMSI:
		case TRC_TYPE_ROAM_MDN:
			for( i = 0; i< trace_tbl->count; i++ ){
				if( trace_tbl->stTraceInfo[i].dType == dTrcType ){
					if(strcmp((char*)trace_tbl->stTraceInfo[i].stTraceID.szMIN, ml->msg_body[1].para_cont) == 0){
						dDelIdx = i;
						break;
					}
				}
			}
			break;
		default:
			log_print(LOGN_DEBUG, LH"BAD PARAMETER [%s]", LT, ml->msg_body[0].para_cont);
			dErrorFlag = 1;
	}

	if(i == trace_tbl->count)
	{
		smsg.common.mml_err = eNotFoundData;
		dErrorFlag = 1;
	}

	if(dErrorFlag != 1)
	{
		log_print(LOGN_DEBUG, LH"INDEX[%d]", LT, dDelIdx);

		for(i = dDelIdx; i < trace_tbl->count; i ++)
		{
			if( (i+1) > (MAX_TRACE_CNT-1))
				break;
			memcpy(&trace_tbl->stTraceInfo[i], &trace_tbl->stTraceInfo[i+1], sizeof(trace_tbl->stTraceInfo[i]));
		}

		trace_tbl->count--;
	}

	/*
		실제로 변경되어진 trace_tbl 구조체의 내용을 파일로 기록하여, 전체 프로세스가 새로 시작되는 경우에
		파일로부터 정보를 가져오기 위해 기록한다.
		- Writer: HAN-JIN PARK
		- Date: 2008.09.19
	*/
	if( (dRet = dWriteTraceTblList(FILE_TRACE_TBL, trace_tbl)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dWriteTraceTblList(%s) dRet[%d]", 
			LT, FILE_TRACE_TBL, dRet);
		return -3;
	}

	if(dErrorFlag == 0)
		smsg.common.mml_err	= DBM_SUCCESS;

	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= 0;

	smsg.head.src_proc	= SEQ_PROC_S_MNG;
	smsg.head.dst_func	= ml->src_func;
	smsg.head.dst_proc	= ml->src_proc;
	smsg.head.cmd_id	= ml->cmd_id;
	smsg.head.msg_id	= ml->msg_id;

	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI,LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -4;
	}

	for(i = 0; i < MAX_NTAF_NUM; i++)
    {
        if(pstSubSys->sys[i].uiIP > 0)
        {
            if( (dRet = dSendTrcToNtaf(pstSubSys->sys[i].usSysNo)) < 0)
            {
                log_print(LOGN_CRI, LH"ERROR IN dSendTrcToNtaf(usSysNo[%d][%hu]) dRet[%d]", 
					LT, i, pstSubSys->sys[i].usSysNo, dRet);
                return -5;
            }
        }
    }

	return 0;
}
