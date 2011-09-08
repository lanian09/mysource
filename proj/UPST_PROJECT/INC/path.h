#ifndef _PATH_H_
#define _PATH_H_

/*** O&M 에서는 다음의 파일 경로를.... 다음과 같이 사용하고자 함
START_PATH		".."
DATA_PATH		START_PATH"/DATA"
LOG_PATH		START_PATH"/LOG"
MC_INIT			DATA_PATH"/McInit"
... general 하게 설계를 하자니 이렇게 하는 것이 나을 것 같음
	다른 의견이 있으면 comment 부탁요망
****************************************************************/

/**
 * Default path defines
 */
#define START_PATH			".."
#define DATA_PATH			START_PATH"/DATA"
#define LOG_PATH			START_PATH"/LOG"
#define DEF_HIS_PATH		START_PATH"/HIS"
#define BACKUP_PATH         START_PATH"/BACKUP_SVCMON"

#define HOME_PATH			"."
#define PARENT_PATH			".."
#define PROC_PATH			"/proc"
#define APP_HOME_BIN		START_PATH"/BIN"

/* FILE PATH(DATA, LOG, ...) */

/**
 * Config, data file path defines
 */
#define FILE_MC_INIT		DATA_PATH"/McInit"
#define FILE_MC_INIT_M_PRI	DATA_PATH"/McInit.TAM_APP"	/* COND ONLY , it same 'McInit'*/
#define FILE_MC_INIT_F_PRI	DATA_PATH"/McInit.TAF_RP"	/* COND ONLY */
#define FILE_MC_INIT_F_SEC	DATA_PATH"/McInit.TAF_PI"	/* COND ONLY */
#define FILE_SYS_CONFIG		DATA_PATH"/SYS_CFG"
#define FILE_SUB_SYS		DATA_PATH"/SUBSYS_INFO.dat"
#define FILE_TAM_LOAD_DATA	DATA_PATH"/AlmClsLoad_TAM"
#define FILE_TAF_LOAD_DATA	DATA_PATH"/AlmClsLoad_TAF"
#define FILE_SHM_INFO		DATA_PATH"/SHM_INFO.dat"
#define FILE_SEM_INFO		DATA_PATH"/SEM_INFO.dat" 
#define FILE_MYSQL_CONF		DATA_PATH"/MySQL.conf"
#define FILE_SUP_IP_CONF	DATA_PATH"/tas_ipaddr.dat"
#define FILE_FIDB			DATA_PATH"/fidb.mem"
#define FILE_MASK			DATA_PATH"/mask.mem"
#define FILE_ENCLO_SERIAL	DATA_PATH"/ENCLO_SERIAL.dat"
#define FILE_MIRROR_TIME	DATA_PATH"/MIRROR_TIME.dat"
#define FILE_SVC_LIST		DATA_PATH"/TamSvclist.dat"
#define FILE_WATCHFILTER	DATA_PATH"/WatchFilter.dat"

#define FILE_PATH_FILTER	DATA_PATH"/conf-new.DAT"
#define FILE_FLT_MNIP		DATA_PATH"/FLT_MNIP.dat"
#define FILE_FLT_INFO		DATA_PATH"/FLT_INFO"
#define FILE_GN_TIMEOUT		DATA_PATH"/GN_TIMEOUT.dat"
#define FILE_FLT_SERVICE	DATA_PATH"/SERVICE_CONF.dat"
#define FILE_FLT_SVR		DATA_PATH"/FLT_SVR.dat"
#define FILE_FLT_SCTP		DATA_PATH"/FLT_SCTP.dat"
#define FILE_LOG_LEVEL		DATA_PATH"/LOGLVL.dat"
#define FILE_TRACE_TBL		DATA_PATH"/TRACE_TBL.dat"
#define FILE_SVRTRACE_TBL	DATA_PATH"/SVRTRACE_TBL.dat"
#define FILE_TIMER			DATA_PATH"/Timer.dat"
#define FILE_CIAPP_INFO		DATA_PATH"/CIAPP_INFO.dat"
#define FILE_CIAPP_REMAIN	DATA_PATH"/CIAPP_REMAIN.dat"
#define FILE_SVCMON_INFO	DATA_PATH"/SI_SVCMON_INFO.dat"
#define FILE_SVCMON_REMAIN	DATA_PATH"/SI_SVCMON_REMAIN.dat"
#define FILE_DEFECT_CODE	DATA_PATH"/DEFECT_CODE.dat"
#define FILE_IRM			DATA_PATH"/IRM.DAT"
#define FILE_NMSPORT		DATA_PATH"/NMSPort.dat"
#define FILE_NMSIP			DATA_PATH"/NMSIP.dat"
#define FILE_NMSOID			DATA_PATH"/NMSOid.dat"
#define FILE_PIRP_MAP		DATA_PATH"/PIRP_MAPPING.DAT"
#define FILE_IPPOOLINFO		DATA_PATH"/IPPOOL_INFO.dat"

#define FILE_NIFO_ZONE		DATA_PATH"/nifo_zone.conf"
#define FILE_CIFO_CONF		DATA_PATH"/cifo.conf"
#define FILE_GIFO_CONF		DATA_PATH"/gifo.conf"



#endif	/* _PATH_H_	*/
