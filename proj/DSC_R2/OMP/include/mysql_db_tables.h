#ifndef __MYSQL_DB_TABLES_H__
#define __MYSQL_DB_TABLES_H__


//--------------------------------------------------------------------------------
// mysql DB 이름
//--------------------------------------------------------------------------------
#define BSDM_DB_NAME		"DSCM"
#define CM_DB_NAME			"mysql"
#define STM_DB_NAME			"DSCM"

#define SFM_ALM_DB_NAME		BSDM_DB_NAME // 장애 발생,해지 정보를 저장할 DB
#define	MML_COMMAND_DB_NAME	BSDM_DB_NAME // MMC 명령어 정보를 저장할 DB
#define	STM_STATISTIC_DB_NAME	BSDM_DB_NAME // 통계 정보를 저장할 DB


//--------------------------------------------------------------------------------
// mysql DB table 이름
//--------------------------------------------------------------------------------

#define SFM_ALM_HIS_DB_TABLE_NAME	    "alarm_history" // 장애 발생,해지 이력 정보 table
#define SFM_CURR_ALM_DB_TABLE_NAME	    "current_alarm" // 현재 발생되어 있는 장애 리스트 table
#define MML_COMMAND_DB_TABLE_NAME	    "mmc_command"   // 명령어 등급, 파라미터 갯수, 처리 프로세스 등이 저장되는 table
#define MML_PARAMETER_DB_TABLE_NAME	    "mmc_parameter" // 명령어별 파라미터에 대한 syntax가 저장되는 table

// RLEG LogIn/Out LOG 테이블 
#define STM_STATISTIC_LOG_LEG_TBL_NAME 		"leg_history" 

// delay 통계 테이블 
#define STM_STATISTIC_5MINUTE_DELAY_TBL_NAME "pkt_delay_5minute_statistics" 
#define STM_STATISTIC_HOUR_DELAY_TBL_NAME 	"pkt_delay_hour_statistics" 
#define STM_STATISTIC_DAY_DELAY_TBL_NAME 	"pkt_delay_day_statistics" 
#define STM_STATISTIC_WEEK_DELAY_TBL_NAME 	"pkt_delay_week_statistics" 
#define STM_STATISTIC_MONTH_DELAY_TBL_NAME 	"pkt_delay_month_statistics" 

// RLEG 통계 테이블 
#define STM_STATISTIC_5MINUTE_LEG_TBL_NAME 	"leg_5minute_statistics" 
#define STM_STATISTIC_HOUR_LEG_TBL_NAME 	"leg_hour_statistics" 
#define STM_STATISTIC_DAY_LEG_TBL_NAME 		"leg_day_statistics" 
#define STM_STATISTIC_WEEK_LEG_TBL_NAME 	"leg_week_statistics" 
#define STM_STATISTIC_MONTH_LEG_TBL_NAME 	"leg_month_statistics" 

// LOGON 통계 테이블 
#define STM_STATISTIC_5MINUTE_LOGON_TBL_NAME	"logon_5minute_statistics" 
#define STM_STATISTIC_HOUR_LOGON_TBL_NAME 	"logon_hour_statistics" 
#define STM_STATISTIC_DAY_LOGON_TBL_NAME 	"logon_day_statistics" 
#define STM_STATISTIC_WEEK_LOGON_TBL_NAME 	"logon_week_statistics" 
#define STM_STATISTIC_MONTH_LOGON_TBL_NAME 	"logon_month_statistics" 

// FLOW 통계 테이블 2010.08.23
#define STM_REPORT_IMSI_FLOW_TBL_NAME		"flow_report_imsi" 
#define STM_STATISTIC_5MINUTE_FLOW_TBL_NAME	"flow_5minute_statistics" 
#define STM_STATISTIC_HOUR_FLOW_TBL_NAME 	"flow_hour_statistics" 
#define STM_STATISTIC_DAY_FLOW_TBL_NAME 	"flow_day_statistics" 
#define STM_STATISTIC_WEEK_FLOW_TBL_NAME 	"flow_week_statistics" 
#define STM_STATISTIC_MONTH_FLOW_TBL_NAME 	"flow_month_statistics" 

// 통계 정보를 저장할 mysql table 이름
#define STM_STATISTIC_5MINUTE_LOAD_TBL_NAME "load_5minute_statistics" 
#define STM_STATISTIC_HOUR_LOAD_TBL_NAME 	"load_hour_statistics" 
#define STM_STATISTIC_DAY_LOAD_TBL_NAME 	"load_day_statistics" 
#define STM_STATISTIC_WEEK_LOAD_TBL_NAME 	"load_week_statistics" 
#define STM_STATISTIC_MONTH_LOAD_TBL_NAME 	"load_month_statistics" 

#define STM_STATISTIC_5MINUTE_FAULT_TBL_NAME	"fault_5minute_statistics" 
#define STM_STATISTIC_HOUR_FAULT_TBL_NAME	"fault_hour_statistics" 
#define STM_STATISTIC_DAY_FAULT_TBL_NAME	"fault_day_statistics" 
#define STM_STATISTIC_WEEK_FAULT_TBL_NAME	"fault_week_statistics" 
#define STM_STATISTIC_MONTH_FAULT_TBL_NAME	"fault_month_statistics" 

#define STM_STATISTIC_5MINUTE_BSD_FLT_TBL_NAME	"bsd_flt_5minute_statistics" 
#define STM_STATISTIC_HOUR_BSD_FLT_TBL_NAME	"bsd_flt_hour_statistics" 
#define STM_STATISTIC_DAY_BSD_FLT_TBL_NAME	"bsd_flt_day_statistics" 
#define STM_STATISTIC_WEEK_BSD_FLT_TBL_NAME	"bsd_flt_week_statistics" 
#define STM_STATISTIC_MONTH_BSD_FLT_TBL_NAME	"bsd_flt_month_statistics" 

// -- CM RDR Report 관련 설정  by jjinri 2009.04.18
#define STM_STATISTIC_5MINUTE_LUR_TBL_NAME  "rdr_lur_5minute_statistics" 
#define STM_STATISTIC_HOUR_LUR_TBL_NAME     "rdr_lur_hour_statistics" 
#define STM_STATISTIC_DAY_LUR_TBL_NAME      "rdr_lur_day_statistics" 
#define STM_STATISTIC_WEEK_LUR_TBL_NAME     "rdr_lur_week_statistics" 
#define STM_STATISTIC_MONTH_LUR_TBL_NAME    "rdr_lur_month_statistics" 

#define STM_STATISTIC_5MINUTE_TRRULE_TBL_NAME   "rdr_trrule_5minute_statistics" 
#define STM_STATISTIC_5MINUTE_TRENTRY_TBL_NAME   "rdr_trentry_5minute_statistics" 

#define STM_STATISTIC_5MINUTE_TR_TBL_NAME   "rdr_tr_5minute_statistics" 
#define STM_STATISTIC_HOUR_TR_TBL_NAME      "rdr_tr_hour_statistics" 
#define STM_STATISTIC_DAY_TR_TBL_NAME       "rdr_tr_day_statistics" 
#define STM_STATISTIC_WEEK_TR_TBL_NAME      "rdr_tr_week_statistics" 
#define STM_STATISTIC_MONTH_TR_TBL_NAME     "rdr_tr_month_statistics"

// -- CM RDR Report 추가 by jjinri 2009.04.22
#define STM_STATISTIC_5MINUTE_PUR_TBL_NAME   "rdr_pur_5minute_statistics" 
#define STM_STATISTIC_HOUR_PUR_TBL_NAME      "rdr_pur_hour_statistics" 
#define STM_STATISTIC_DAY_PUR_TBL_NAME       "rdr_pur_day_statistics" 
#define STM_STATISTIC_WEEK_PUR_TBL_NAME      "rdr_pur_week_statistics" 
#define STM_STATISTIC_MONTH_PUR_TBL_NAME     "rdr_pur_month_statistics"

#define STM_STATISTIC_5MINUTE_MALUR_TBL_NAME   "rdr_malur_5minute_statistics" 
#define STM_STATISTIC_HOUR_MALUR_TBL_NAME      "rdr_malur_hour_statistics" 
#define STM_STATISTIC_DAY_MALUR_TBL_NAME       "rdr_malur_day_statistics" 
#define STM_STATISTIC_WEEK_MALUR_TBL_NAME      "rdr_malur_week_statistics" 
#define STM_STATISTIC_MONTH_MALUR_TBL_NAME     "rdr_malur_month_statistics"

#define STM_STATISTIC_5MINUTE_BLOCKRULE_TBL_NAME   "rdr_blockrule_5minute_statistics" 
#define STM_STATISTIC_5MINUTE_BLOCKENTRY_TBL_NAME   "rdr_blockentry_5minute_statistics" 

#define STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME   "rdr_block_5minute_statistics" 
#define STM_STATISTIC_HOUR_BLOCK_TBL_NAME      "rdr_block_hour_statistics" 
#define STM_STATISTIC_DAY_BLOCK_TBL_NAME       "rdr_block_day_statistics" 
#define STM_STATISTIC_WEEK_BLOCK_TBL_NAME      "rdr_block_week_statistics" 
#define STM_STATISTIC_MONTH_BLOCK_TBL_NAME     "rdr_block_month_statistics"

#define STM_STATISTIC_5MINUTE_HTTP_TBL_NAME   "rdr_http_5minute_statistics" 
#define STM_STATISTIC_HOUR_HTTP_TBL_NAME      "rdr_http_hour_statistics" 
#define STM_STATISTIC_DAY_HTTP_TBL_NAME       "rdr_http_day_statistics" 
#define STM_STATISTIC_WEEK_HTTP_TBL_NAME      "rdr_http_week_statistics" 
#define STM_STATISTIC_MONTH_HTTP_TBL_NAME     "rdr_http_month_statistics"

#define STM_STATISTIC_5MINUTE_RULE_TBL_NAME   "rdr_rulecnt_5minute_statistics" 
#define STM_STATISTIC_HOUR_RULE_TBL_NAME      "rdr_rulecnt_hour_statistics" 
#define STM_STATISTIC_DAY_RULE_TBL_NAME       "rdr_rulecnt_day_statistics" 
#define STM_STATISTIC_WEEK_RULE_TBL_NAME      "rdr_rulecnt_week_statistics" 
#define STM_STATISTIC_MONTH_RULE_TBL_NAME     "rdr_rulecnt_month_statistics"

#define STM_STATISTIC_5MINUTE_RULESET_TBL_NAME   "rdr_ruleset_5minute_statistics" 
#define STM_STATISTIC_HOUR_RULESET_TBL_NAME      "rdr_ruleset_hour_statistics" 
#define STM_STATISTIC_DAY_RULESET_TBL_NAME       "rdr_ruleset_day_statistics" 
#define STM_STATISTIC_WEEK_RULESET_TBL_NAME      "rdr_ruleset_week_statistics" 
#define STM_STATISTIC_MONTH_RULESET_TBL_NAME     "rdr_ruleset_month_statistics"

#define STM_STATISTIC_5MINUTE_RULEENT_TBL_NAME   "rdr_ruleent_5minute_statistics" 
#define STM_STATISTIC_HOUR_RULEENT_TBL_NAME      "rdr_ruleent_hour_statistics" 
#define STM_STATISTIC_DAY_RULEENT_TBL_NAME       "rdr_ruleent_day_statistics" 
#define STM_STATISTIC_WEEK_RULEENT_TBL_NAME      "rdr_ruleent_week_statistics" 
#define STM_STATISTIC_MONTH_RULEENT_TBL_NAME     "rdr_ruleent_month_statistics"

#define STM_STATISTIC_5MINUTE_SMS_TBL_NAME   "sms_5minute_statistics" 
#define STM_STATISTIC_HOUR_SMS_TBL_NAME      "sms_hour_statistics" 
#define STM_STATISTIC_DAY_SMS_TBL_NAME       "sms_day_statistics" 
#define STM_STATISTIC_WEEK_SMS_TBL_NAME      "sms_week_statistics" 
#define STM_STATISTIC_MONTH_SMS_TBL_NAME     "sms_month_statistics"
#define SMS_HISTORY     "sms_history"


/* by helca 2007.05.10 */
#if 0
#define STM_STATISTIC_5MINUTE_VT_TBL_NAME     "vt_5minute_statistics" 
#define STM_STATISTIC_HOUR_VT_TBL_NAME        "vt_hour_statistics" 
#define STM_STATISTIC_DAY_VT_TBL_NAME         "vt_day_statistics" 
#define STM_STATISTIC_WEEK_VT_TBL_NAME        "vt_week_statistics" 
#define STM_STATISTIC_MONTH_VT_TBL_NAME       "vt_month_statistics"

#define STM_STATISTIC_5MINUTE_CDR2_TBL_NAME     "cdr2_5minute_statistics" 
#define STM_STATISTIC_HOUR_CDR2_TBL_NAME        "cdr2_hour_statistics" 
#define STM_STATISTIC_DAY_CDR2_TBL_NAME         "cdr2_day_statistics" 
#define STM_STATISTIC_WEEK_CDR2_TBL_NAME        "cdr2_week_statistics" 
#define STM_STATISTIC_MONTH_CDR2_TBL_NAME       "cdr2_month_statistics"

#define STM_STATISTIC_5MINUTE_SVC_TYPE_UDR_TBL_NAME 	"udr_svcType_5minute_statistics" 
#define STM_STATISTIC_HOUR_SVC_TYPE_UDR_TBL_NAME  	"udr_svcType_hour_statistics" 
#define STM_STATISTIC_DAY_SVC_TYPE_UDR_TBL_NAME  	"udr_svcType_day_statistics" 
#define STM_STATISTIC_WEEK_SVC_TYPE_UDR_TBL_NAME  	"udr_svcType_week_statistics" 
#define STM_STATISTIC_MONTH_SVC_TYPE_UDR_TBL_NAME  	"udr_svcType_month_statistics" 

#define STM_STATISTIC_5MINUTE_SVC_WAP1_TBL_NAME  	"wap1_5minute_statistics" 
#define STM_STATISTIC_HOUR_SVC_WAP1_TBL_NAME  		"wap1_hour_statistics" 
#define STM_STATISTIC_DAY_SVC_WAP1_TBL_NAME  		"wap1_day_statistics" 
#define STM_STATISTIC_WEEK_SVC_WAP1_TBL_NAME  		"wap1_week_statistics" 
#define STM_STATISTIC_MONTH_SVC_WAP1_TBL_NAME  		"wap1_month_statistics"

#define STM_STATISTIC_5MINUTE_SVC_WAP2_TBL_NAME  	"wap2_5minute_statistics" 
#define STM_STATISTIC_HOUR_SVC_WAP2_TBL_NAME  		"wap2_hour_statistics" 
#define STM_STATISTIC_DAY_SVC_WAP2_TBL_NAME  		"wap2_day_statistics" 
#define STM_STATISTIC_WEEK_SVC_WAP2_TBL_NAME  		"wap2_week_statistics" 
#define STM_STATISTIC_MONTH_SVC_WAP2_TBL_NAME  		"wap2_month_statistics"

#define STM_STATISTIC_5MINUTE_SVC_HTTP_TBL_NAME  	"http_5minute_statistics" 
#define STM_STATISTIC_HOUR_SVC_HTTP_TBL_NAME  		"http_hour_statistics" 
#define STM_STATISTIC_DAY_SVC_HTTP_TBL_NAME  		"http_day_statistics" 
#define STM_STATISTIC_WEEK_SVC_HTTP_TBL_NAME  		"http_week_statistics" 
#define STM_STATISTIC_MONTH_SVC_HTTP_TBL_NAME  		"http_month_statistics"

#define STM_STATISTIC_5MINUTE_SVC_WIPI_TBL_NAME  	"wipi_5minute_statistics" 
#define STM_STATISTIC_HOUR_SVC_WIPI_TBL_NAME  		"wipi_hour_statistics" 
#define STM_STATISTIC_DAY_SVC_WIPI_TBL_NAME  		"wipi_day_statistics" 
#define STM_STATISTIC_WEEK_SVC_WIPI_TBL_NAME  		"wipi_week_statistics" 
#define STM_STATISTIC_MONTH_SVC_WIPI_TBL_NAME  		"wipi_month_statistics"

#define STM_STATISTIC_5MINUTE_SVC_VODS_TBL_NAME  	"vods_5minute_statistics" 
#define STM_STATISTIC_HOUR_SVC_VODS_TBL_NAME  		"vods_hour_statistics" 
#define STM_STATISTIC_DAY_SVC_VODS_TBL_NAME  		"vods_day_statistics" 
#define STM_STATISTIC_WEEK_SVC_VODS_TBL_NAME  		"vods_week_statistics" 
#define STM_STATISTIC_MONTH_SVC_VODS_TBL_NAME  		"vods_month_statistics"

#define STM_STATISTIC_5MINUTE_SVC_JAVA_TBL_NAME  	"java_5minute_statistics" 
#define STM_STATISTIC_HOUR_SVC_JAVA_TBL_NAME  		"java_hour_statistics" 
#define STM_STATISTIC_DAY_SVC_JAVA_TBL_NAME  		"java_day_statistics" 
#define STM_STATISTIC_WEEK_SVC_JAVA_TBL_NAME  		"java_week_statistics" 
#define STM_STATISTIC_MONTH_SVC_JAVA_TBL_NAME  		"java_month_statistics"

#define STM_STATISTIC_5MINUTE_FAIL_UDR_TBL_NAME         "fail_udr_5minute_statistics"
#define STM_STATISTIC_HOUR_FAIL_UDR_TBL_NAME            "fail_udr_hour_statistics"
#define STM_STATISTIC_DAY_FAIL_UDR_TBL_NAME             "fail_udr_day_statistics"
#define STM_STATISTIC_WEEK_FAIL_UDR_TBL_NAME            "fail_udr_week_statistics"
#define STM_STATISTIC_MONTH_FAIL_UDR_TBL_NAME           "fail_udr_month_statistics"
#endif

#endif /*__MYSQL_DB_TABLES_H__*/
