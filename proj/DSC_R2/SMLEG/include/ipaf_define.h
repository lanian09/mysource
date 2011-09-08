/**********************************************************
                 ABLEX SYSTEM DEFINITION

   Author   : LEE SANG HO
   Section  : IPAS Project
   SCCS ID  : %W%
   Date     : %G%
   Revision History :
        '01.  9. 19     Initial

   Description:
        Recovery

   Copyright (c) ABLEX 2001
***********************************************************/

#ifndef __IPAM_DEFINE_HEADER_FILE__
#define __IPAM_DEFINE_HEADER_FILE__

#define INTEL				1

#define PROC_PATH			"/proc"
#define PARENT_PATH			".."
#define HOME_PATH			"."

#define START_PATH			"/DSC"
#define APP_PATH			"/DSC/NEW"
#define BACKUP_PATH			"/DSC/OLD"

#define BIN_PATH			START_PATH"/NEW/BIN/"

#define APP_HOME_BIN		BIN_PATH
#define APP_HOME  			BIN_PATH	

/* configure data path */
#define DATA_PATH			START_PATH"/NEW/DATA/"
#define FIDB_FILE   		DATA_PATH"fidb.mem"
#define DEF_IP_POOL_FILE    DATA_PATH"IP_POOL.dat"
#define DEF_SER_CAT_FILE    DATA_PATH"SERVICE_CATEGORY.dat"
#define DEF_SER_CAT_NEW_FILE    DATA_PATH"SERVICE_CATEGORY_ADD.dat" /* CATEGORY 임시로 사용 */
#define DEF_NIFO_ZONE_CONF_FILE DATA_PATH"nifo_zone.conf"
#define DEF_NOTI_INDEX_FILE DATA_PATH"NOTI_INIT.conf"
#define DEF_TIMEOUT_FILE	DATA_PATH"TIMEOUT.conf"

#define MC_INIT				DATA_PATH"McInit"
#define TOTO_IPADDR_FILE	DATA_PATH"ipaddr.dat"
#define ALARMCLASS_FILE		DATA_PATH"AlarmClass"

#define VOD_PAYTYPE_TIME	DATA_PATH"VODS_TIMEOUT.dat"

#define DEF_SYSCONF_FILE    DATA_PATH"sysconfig"
#define DEF_FBDOMAIN_FILE	DATA_PATH"FBDomain.dat"

#define DEV_PATH 			DATA_PATH"dev_info.conf"
#define RDR_DEV_PATH 		DATA_PATH"rdr_dev_info.conf"

#define RDR_DEV_PATH 		DATA_PATH"rdr_dev_info.conf"

#define	PBTABLE_PATH 		DATA_PATH"RULESET_LIST.conf"
//#define DEF_DOMAIN_SVCTYPE_FILE    DATA_PATH"DOMAIN_SVCTYPE.dat"
//#define DEF_FREE_URL_FILE    DATA_PATH"FREE_URL.dat"

/* log path */
#define LOG_PATH			START_PATH"/APPLOG/"
#define CHSMD_LOG			LOG_PATH"CHSMD"
#define ALMD_LOG			LOG_PATH"ALMD"
#define COND_LOG			LOG_PATH"COND"
#define MMCD_LOG			LOG_PATH"MMCD"
#define QMONITOR_LOG		LOG_PATH"QMONITOR"
#define RMI_LOG				LOG_PATH"RMI"
#define MMC_LOG         	LOG_PATH"MMC"
#define EDFALMD_LOG			LOG_PATH"EDFALMD"
#define IPAFALMD_LOG		LOG_PATH"IPAFALMD"

#define IPAMTIF_LOG        	LOG_PATH"IPAMTIF"
#define IPAMUIF_LOG        	LOG_PATH"IPAMUIF"
#define MDBMGR_LOG        	LOG_PATH"MDBMGR"

/***** MODIFIED BY LEE : 20040303 *****/
#define SVCANA_LOG          LOG_PATH"SVCANA"
#define ETH_CAPD_LOG        LOG_PATH"ETHCAPD"
#define SESSANA_LOG         LOG_PATH"SESSANA"

#define RDRIF_LOG           LOG_PATH"RDRIF"
#define MESVC_LOG           LOG_PATH"MESVC"
#define KUNSVC_LOG          LOG_PATH"KUNSVC"
#define ADSSVC_LOG          LOG_PATH"ADSSVC"
#define MARSSVC_LOG         LOG_PATH"MARSSVC"

#define MACSSVC_LOG			LOG_PATH"MACSSVC"
#define WICGSSVC_LOG			LOG_PATH"WICGSSVC"

#define VODANA_LOG			LOG_PATH"VODANA"

#define MESSAGE_FILE        "/var/log/messages"
#define FAN_FILE            "/proc/cpqfan"
#define ENCLO_S_FILE        DATA_PATH"ENCLO_SERIAL.dat"

#define MAX_MSGQ_COUNT  	100

/* message queue, shared memory */
#define MSGQ_DEFINE			8000
#define PORT_DEFINE			18000	    
#define PROC_DEFINE			9000
#define SSHM_DEFINE			10000
#define SEMA_DEFINE			11000

/* Management Block Definition */
#define S_MSGQ_CHSMD	   	( MSGQ_DEFINE + 0 )		/*** 1F40 ***/
#define S_MSGQ_COND	    	( MSGQ_DEFINE + 1 )		/*** 1F41 NOT USED ***/
#define S_MSGQ_MMCD	   		( MSGQ_DEFINE + 2 )		/*** 1F42 ***/
#define S_MSGQ_EDFALMD		( MSGQ_DEFINE + 3 )		/*** 1F43 NOT USED ***/
#define S_MSGQ_IPAFALMD		( MSGQ_DEFINE + 4 )		/*** 1F44 NOT USED ***/
#define S_MSGQ_ALMD			( MSGQ_DEFINE + 5 )		/*** 1F45 ***/

/* Service Block Definition */
#define S_MSGQ_IPAMTIF	 	( MSGQ_DEFINE + 11 )	/*** 1F4B ***/
#define S_MSGQ_IPAMUIF	 	( MSGQ_DEFINE + 12 )	/*** 1F4C ***/
#define S_MSGQ_CDRSVR		( MSGQ_DEFINE + 13 )	/*** 1F4D ***/
#define S_MSGQ_MDBMGR		( MSGQ_DEFINE + 14 )	/*** 1F4E ***/
#define S_MSGQ_ADMIN		( MSGQ_DEFINE + 15 )	/*** 1F4F ***/

/* NEW 20040305 */
#define S_MSGQ_MGNSVC       ( MSGQ_DEFINE + 21 )    /*** 1F55 NOT USED ***/
#define S_MSGQ_RDRIF        ( MSGQ_DEFINE + 22 )    /*** 1F56 ***/
#define S_MSGQ_MESVC        ( MSGQ_DEFINE + 23 )    /*** 1F57 ***/
#define S_MSGQ_SESSANA      ( MSGQ_DEFINE + 24 )    /*** 1F58 ***/
#define S_MSGQ_KUNSVC       ( MSGQ_DEFINE + 25 )    /*** 1F59 ***/
#define S_MSGQ_ADSSVC       ( MSGQ_DEFINE + 26 )    /*** 1F5A ***/
#define S_MSGQ_MARSSVC      ( MSGQ_DEFINE + 27 )    /*** 1F5B ***/
#define S_MSGQ_VODANA		( MSGQ_DEFINE + 28 )	/*** 1F5C ***/
#define S_MSGQ_VODMANA		( MSGQ_DEFINE + 29 )	/*** 1F5D ***/
#define S_MSGQ_VODDANA		( MSGQ_DEFINE + 30 )	/*** 1F5E ***/
#define S_MSGQ_SIM		    ( MSGQ_DEFINE + 40 )    /*** 1F68 ***/

#define S_MSGQ_MACSSVC		( MSGQ_DEFINE + 41 )	/*** 1F69 ***/
#define S_MSGQ_WICGSSVC		( MSGQ_DEFINE + 42 )	/*** 1F6A ***/
#define S_MSGQ_AAAIFSVC		( MSGQ_DEFINE + 43 )	/*** 1F6B ***/

/* Management Port Definition */
#define S_PORT_ALMD			( PORT_DEFINE + 301 )
#define S_PORT_COND			( PORT_DEFINE + 302 )
#define S_PORT_MMCD			( PORT_DEFINE + 500 )
#define S_PORT_IPAMTIF      ( PORT_DEFINE + 600 )
#define S_PORT_IPAMUIF     	( PORT_DEFINE + 700 )

/* NEW 2004.03.12 */
#define S_PORT_RDRIF        ( PORT_DEFINE + 800 )

#define S_PORT_ACCOUNT		1813
#define S_PORT_QUD			49149

#define S_SSHM_FIDB         ( SSHM_DEFINE + 300 )   /*** 283C ***/
#define S_SSHM_KEEPALIVE    ( SSHM_DEFINE + 350 )   /*** 286E ***/
#define S_SSHM_UPGRADE_DB   ( SSHM_DEFINE + 360 )   /*** 2878 ***/
#define S_SSHM_GENINFO      ( SSHM_DEFINE + 370 )   /*** 2882 ***/
#define S_SSHM_MEMHDR       ( SSHM_DEFINE + 380 )   /*** 288C CAPD BUFFER HEADER SHM ***/
#define S_SSHM_MEM          ( SSHM_DEFINE + 381 )   /*** 288D CAPD BUFFER SHM ***/
#define S_SSHM_MMDBSESS     ( SSHM_DEFINE + 210 )   /*** 27E2 ***/
#define S_SSHM_MMDBOBJ      ( SSHM_DEFINE + 211 )   /*** 27E3 ***/
#define S_SSHM_MMDBCDR      ( SSHM_DEFINE + 212 )   /*** 27E4 ***/
#define S_SSHM_MMDBDESTIP   ( SSHM_DEFINE + 213 )   /*** 27E5 ***/
#define S_SSHM_MMDBDESTPORT ( SSHM_DEFINE + 214 )   /*** 27E6 ***/
#define S_SSHM_VERSION      ( SSHM_DEFINE + 215 )   /*** 27E7, 040114,poopee */
#define S_SSHM_MMDBLIST     ( SSHM_DEFINE + 216 )   /*** 27E8 MMDBLIST SHMKEY */
#define S_SSHM_SESSANA      ( SSHM_DEFINE + 217 )   /*** 27E9 SESS ANA, CALL-TCP SESSION ANA */
#define S_SSHM_MESVC        ( SSHM_DEFINE + 218 )   /*** 27EA MESVC ***/
#define S_SSHM_KUNSVC       ( SSHM_DEFINE + 219 )   /*** 27EB ***/
#define S_SSHM_ADSSVC       ( SSHM_DEFINE + 220 )   /*** 27EC ***/
#define S_SSHM_MARSSVC      ( SSHM_DEFINE + 221 )   /*** 27ED ***/

#define S_SSHM_MESTAT		( SSHM_DEFINE + 222 )	/*** 27EE ***/
#define S_SSHM_KUNSTAT		( SSHM_DEFINE + 223 )	/*** 27EF ***/
#define S_SSHM_ADSSTAT		( SSHM_DEFINE + 224 )	/*** 27F0 ***/
#define S_SSHM_MARSSTAT		( SSHM_DEFINE + 225 )	/*** 27F1 ***/
#define S_SSHM_SESSSTAT		( SSHM_DEFINE + 226 )	/*** 27F2 ***/

#define S_SSHM_RDRSEQ		( SSHM_DEFINE + 227 )	/*** 27F3 ***/

#define S_SSHM_VODANA		( SSHM_DEFINE + 228 )	/*** 27F4 ***/
#define S_SSHM_VODUDP		( SSHM_DEFINE + 229 )	/*** 27F5 ***/

#define S_SSHM_MACSSVC		( SSHM_DEFINE + 230 )	/*** 27F6 ***/
#define S_SSHM_MACSSTAT		( SSHM_DEFINE + 231 )   /*** 27F7 ***/

#define S_SSHM_VODMANA		( SSHM_DEFINE + 232 )	/*** 27F8 ***/
#define S_SSHM_VODDANA		( SSHM_DEFINE + 233 )	/*** 27F9 ***/ 

#define S_SSHM_WICGSSVC		( SSHM_DEFINE + 234 )	/*** 27FA ***/
#define S_SSHM_WICGSSTAT	( SSHM_DEFINE + 235 )   /*** 27FB ***/

#define S_SSHM_VODSSTAT		( SSHM_DEFINE + 236 )	/*** 27FC ***/
#define S_SSHM_VODMSTAT		( SSHM_DEFINE + 237 )	/*** 27FD ***/
#define S_SSHM_VODDSTAT		( SSHM_DEFINE + 238 )	/*** 27FE ***/

#define S_SSHM_WAPSVC		( SSHM_DEFINE + 239 )	/*** 27FF ***/
#define S_SSHM_WAPSTAT		( SSHM_DEFINE + 240 ) 	/*** 2800 ***/


/* Semapore Definition */
#define S_SEMA_SESS			( SEMA_DEFINE + 0 )
#define S_SEMA_CDR			( SEMA_DEFINE + 1 )
#define S_SEMA_OBJ			( SEMA_DEFINE + 2 )
#define S_SEMA_DESTIP		( SEMA_DEFINE + 3 )
#define S_SEMA_DESTPORT		( SEMA_DEFINE + 4 )
#define S_SEMA_CAPINDEX		( SEMA_DEFINE + 5 )
#define S_SEMA_CDR2			( SEMA_DEFINE + 6 )
#define S_SEMA_OBJ2			( SEMA_DEFINE + 7 )
#define S_SEMA_SESS2		( SEMA_DEFINE + 8 )
#define S_SEMA_MIF			( SEMA_DEFINE + 9 )
#define S_SEMA_ACC			( SEMA_DEFINE + 10 )
#define S_SEMA_MIF2			( SEMA_DEFINE + 11 )

/* Map file path Definition */
#define MAP_PATH			DATA_PATH"/MMAP/"

#endif
