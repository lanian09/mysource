/**		@file	define.h
 * 		- SHM, SEMA, MSGQ KEY 정의
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: define.h,v 1.1.1.1 2011/04/19 14:13:43 june Exp $
 *
 * 		@Author		$Author: june $
 * 		@version	$Revision: 1.1.1.1 $
 * 		@date		$Date: 2011/04/19 14:13:43 $
 * 		@warning	nothing
 * 		@ref		define.h
 * 		@todo		nothing
 *
 * 		@section	Intro(소개)
 * 		- SHM, SEMA, MSGQ KEY 정의
 *
 * 		@section	Requirement
 * 		 @li 
 *
 **/

#ifndef _DEFINE_H__
#define _DEFINE_H__

#define INTEL				1

#define PROC_PATH			"/proc"
#define PARENT_PATH			".."
#define HOME_PATH			"."

#ifndef RELATIVE_PATH			/* case relative path*/
	#define START_PATH			"./"
#else
	#define START_PATH			"./"
#endif

#define APP_PATH			"/WNTAFAPP/NEW"
#define BACKUP_PATH			"/WNTAFAPP/OLD"

#define BIN_PATH			START_PATH"/bin/"
#define PACKET_PATH			START_PATH"/PACKET/"

#define APP_HOME_BIN		BIN_PATH
#define APP_HOME  			BIN_PATH	

/* configure data path */
#define DATA_PATH			START_PATH"/data/"

#define DEF_TAF_INFO		DATA_PATH"taf_info.dat"
#define DEF_TAM_INFO		DATA_PATH"tam_info.dat"

#define FIDB_FILE   		DATA_PATH"fidb.mem"
#define DEF_IP_POOL_FILE    DATA_PATH"IP_POOL.dat"
#define DEF_SER_CAT_FILE    DATA_PATH"SERVICE_CATEGORY.dat"
#define DEF_SER_CAT_NEW_FILE    DATA_PATH"SERVICE_CATEGORY_ADD.dat" /* CATEGORY 임시로 사용 */

#define MC_INIT				DATA_PATH"McInit"
#define ALARMCLASS_FILE		DATA_PATH"AlarmClass"
#define SYS_CONFIG_FILE     DATA_PATH"SYS_CFG"

#define PATH_FILTER			DATA_PATH"conf-new.DAT"
#define PATH_ETH			DATA_PATH"eth.conf"
#define TAS_IPADDR_FILE     DATA_PATH"tas_ipaddr.dat"
#define DEF_SCTP_SYSIP_FILE	DATA_PATH"sctp_sysip.dat"

/* log path */
#define LOG_PATH			START_PATH"/log/"
#define CHSMD_LOG			LOG_PATH"CHSMD"
#define ALMD_LOG			LOG_PATH"ALMD"
#define COND_LOG			LOG_PATH"COND"
#define MMCD_LOG			LOG_PATH"MMCD"
#define QMONITOR_LOG		LOG_PATH"QMONITOR"
#define RMI_LOG				LOG_PATH"RMI"
#define MMC_LOG         	LOG_PATH"MMC"

#define IPAMTIF_LOG        	LOG_PATH"IPAMTIF"
#define IPAMUIF_LOG        	LOG_PATH"IPAMUIF"
#define MDBMGR_LOG        	LOG_PATH"MDBMGR"

/* message queue, shared memory */
#define MSGQ_DEFINE			8000
#define PORT_DEFINE			18000	    
#define PROC_DEFINE			9000
#define SSHM_DEFINE			10000
#define SEMA_DEFINE			11000

/* Management Port Definition */
#define S_PORT_ALMD			( PORT_DEFINE + 301 )
#define S_PORT_COND			( PORT_DEFINE + 302 )
#define S_PORT_MMCD			( PORT_DEFINE + 500 )
#define S_PORT_TAMTIF		( PORT_DEFINE + 600 )
#define S_PORT_TAMUIF   	( PORT_DEFINE + 700 )

#define S_PORT_SI_LOG		( PORT_DEFINE + 1000 )
#define S_PORT_SI_SIG		( PORT_DEFINE + 1002 )
#define S_PORT_SI_SVC		( PORT_DEFINE + 1003 )

#endif

/*
  	$Log: define.h,v $
  	Revision 1.1.1.1  2011/04/19 14:13:43  june
  	성능 패키지
  	
  	Revision 1.1.1.1  2011/01/20 12:18:51  june
  	DSC CVS RECOVERY
  	
  	Revision 1.1  2009/05/09 09:41:01  dsc
  	init
  	
  	Revision 1.1  2008/12/12 00:07:21  yhshin
  	*** empty log message ***
  	
  	Revision 1.5  2008/01/16 10:39:51  yhshin
  	path
  	
  	Revision 1.4  2008/01/13 12:37:42  yhshin
  	path
  	
  	Revision 1.3  2008/01/12 05:56:46  june
  	relative_path define add
  	
  	Revision 1.2  2007/12/28 04:42:58  yhshin
  	warning 제거
  	
  	Revision 1.1.1.1  2007/11/12 09:52:29  yhshin
  	lgt accelator
  	
 */
