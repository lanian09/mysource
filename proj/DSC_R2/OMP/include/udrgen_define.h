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


#define PROC_PATH			"/proc"
#define PARENT_PATH			".."
#define HOME_PATH			"."

#define START_PATH			"/DSCM/NEW"

#define BIN_PATH			START_PATH"/BIN/"
#define CDR_PATH			START_PATH"/CDR/"
#define HEADLOG_PATH		"/DSC/LOG/HEADLOG/"
//#define HEADLOG_PATH		"/DSCM/HEADLOG/"

#define APP_HOME_BIN		BIN_PATH
#define APP_HOME  			BIN_PATH	

/* configure data path */
#define DATA_PATH			START_PATH"/DATA/"
#define FIDB_FILE   		DATA_PATH"fidb.mem"

#define DEF_TRACE_INFO_FILE         DATA_PATH"TRACE_INFO.conf"
#define DEF_AAAPARA_LIST_FILE       DATA_PATH"AAAPARA.conf"
#define DEF_AAAIP_LIST_FILE         DATA_PATH"AAAIF.conf"
#define DEF_BSD_IP_FILE             DATA_PATH"BSD_IP.conf"
/* MsgQKey Read : Add 20060703(challa) ---> */
#define DEF_SYSCONF_FILE            DATA_PATH"sysconfig"
#define DEF_CHEK_MD5_FILE           DATA_PATH"CHEK_MD5_INFO.conf"
/* <--- */

/* log path */
#define LOG_PATH			"/DSC/LOG/"
#define UDRGEN_LOG			LOG_PATH"UDRGEN"
#define AAAIF_LOG			LOG_PATH"AAAIF"


/* Accounting Status Type */
#define DEF_ACCT_REQ_START   1  /* Accounting Start        */ 
#define DEF_ACCT_REQ_STOP    2  /* Accounting Stop         */
#define DEF_ACCT_INTERIM     3  /* Accounting Interim : UDR*/

/* TRACE FLAG */
#define TRACE_CONTINUE       1  /* trace message continue  */
#define TRACE_COMPLETE       0  /* trace complete          */

/* RETRANS FLAG : BSD -> AAAIF */
#define RETRY_ON             1
#define RETRY_OFF            2

/* MD5 Check Flag */
#define MD5_NOTCHEK          0
#define MD5_CHEK             1


#define	MAX_PNUM			4
#define MAX_MSGQ_COUNT  	100

/* return value */
#define RET_MMC_REQ     1 
#define RET_SVC_REQ     2

/* Send,Receive Flag for Trace */
#define DEF_RCV_FLAG    1
#define DEF_SND_FLAG    2 

/* message queue, shared memory */
#define MSGQ_DEFINE			8000
#define PORT_DEFINE			18000	    
#define PROC_DEFINE			9000
#define SSHM_DEFINE			10000

#define S_MSGQ_UDRGEN		( MSGQ_DEFINE + 43 )
#define S_MSGQ_AAAIF		( MSGQ_DEFINE + 44 )
#define S_MSGQ_SDMD         ( MSGQ_DEFINE + 45 )

/* R2.0.0 Add 2004.0406 (lander) */
#define	S_MSGQ_MRDRIF		( MSGQ_DEFINE + 26 )
#define	S_MSGQ_IDRIF		( MSGQ_DEFINE + 27 )

/* Management Port Definition */
#define DEF_TIMEOUT_SEC     3

#define S_PORT_AAAIF		40019   /* AAAIF Port Number */
#define S_PORT_SENDTO		1813    /* Send Port to AAA  */ 

#define S_SSHM_FIDB			( SSHM_DEFINE + 300 )
#define S_SSHM_ADMIN		( SSHM_DEFINE + 301 )		/*** USELESS ***/	
#define S_SSHM_IPAFTIF		( SSHM_DEFINE + 302 )
#define S_SSHM_IPAFINFSTAT	( SSHM_DEFINE + 303 )		/***/
#define S_SSHM_PDSNINFSTAT	( SSHM_DEFINE + 304 )		/***/
#define S_SSHM_IWFINFSTAT	( SSHM_DEFINE + 305 )		/***/
#define S_SSHM_AAAINFSTAT	( SSHM_DEFINE + 306 )		/***/
#define S_SSHM_DSCPINFSTAT	( SSHM_DEFINE + 307 )		/***/
#define S_SSHM_PDSNQUDINFSTAT	( SSHM_DEFINE + 308 )	/***/
#define S_SSHM_IWFQUDINFSTAT	( SSHM_DEFINE + 309 )	/***/
#define S_SSHM_GENMEM			( SSHM_DEFINE + 310 )
#define S_SSHM_GGSNINFSTAT		( SSHM_DEFINE + 311 )	/***/
#define S_SSHM_GGSNQUDINFSTAT	( SSHM_DEFINE + 312 )	/***/
#define	S_SSHM_VERSION		( SSHM_DEFINE + 313 )		/* 040112,poopee */
#define S_SSHM_IDRINFSTAT		( SSHM_DEFINE + 314 )	/* 20040424, sunny : IDR*/
/* DPP_R1.0.0 add 2004.1101 (lander) ---> */
#define S_SSHM_WAGINFSTAT  ( SSHM_DEFINE + 315 )     // WLAN-KT
#define S_SSHM_WAGQUDINFSTAT	( SSHM_DEFINE + 316 )	// WLAN-KT QUD
/* <--- */

#define S_SSHM_KEEPALIVE	( SSHM_DEFINE + 350 )	
#define S_SSHM_UPGRADE_DB	( SSHM_DEFINE + 360 )	
#define S_SSHM_MMDB			( SSHM_DEFINE + 210 )

#define S_SSHM_FAULTSTAT	( SSHM_DEFINE + 200 )
#define S_SSHM_LOADSTAT 	( SSHM_DEFINE + 220 )

#define MAX_USHORT			65535

typedef struct _st_DUMP_INFO
{
	int		udr_cnt;
	int		udr_id;
	int		udr_sequence;
	char	fileName[MAX_FILENAME_SIZE];
    int     fd;
    time_t  udr_crt_time;
}st_DumpInfo, *pst_DumpInfo;

#endif
