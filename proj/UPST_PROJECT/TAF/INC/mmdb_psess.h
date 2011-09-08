/**********************************************************
          ABLEX Main Memory Database TINYMMDB(TM)

   Author   : Jiyoon Chung
   Section  : 
   SCCS ID  : 
   Date     : 9/19/01
   Revision History : 
   		'01.  8.  4 Initial
		'20004. 6.	miheeh
			- Added Used() to count used tuple.
		
   Description:
		

   Copyright (c) ABLEX 2001 
***********************************************************/

#ifndef __PSESS_DB_HEADER___
#define __PSESS_DB_HEADER___

/**A.1*  File Inclusion ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "typedef.h"

#include "common_stg.h"
#include "VJ.h"

typedef struct _st_BSMSC_
{
    UCHAR   FA_ID;
    UCHAR   SEC_ID;
    UCHAR   SYS_ID;
    UCHAR   BSC_ID;
    USHORT  BTS_ID;
    USHORT  Resserved;
} st_BSMSC, *pst_BSMSC;

/* 최대 record의 개수 */
#define JMM_PSESS_RECORD    50002    

#define PSESS_BUF_MAX   (JMM_PSESS_RECORD - 2)

/* externel buffer for PSESS */
typedef struct _T_PPP_BUF {
    char            cFlag;          /* 0: free 1: occupied */
    char            cReserved[7];
    time_t          LastUpdateTime;

    unsigned int    uiServingPCF;
    unsigned int    uiKey;
    
    struct slcompress   m_comp1;    
    struct slcompress   m_comp2;

    unsigned char   szBuf1[2000];
    unsigned char   szBuf2[2000];
    
} T_PPP_BUF;

typedef struct _T_PSESS_BUF {
    T_PPP_BUF   stPPPBuf;
} T_PSESS_BUF;

/* PSESS ( A10 session ) */
typedef struct _T_PPP_INFO {
	unsigned int	uiSeq;	
	short			siLength;
	char			dPPPState;
	char			bMrgState;		/* 0: normal 1: abnormal */
} T_PPP_INFO;

typedef struct _T_SESS {
	time_t	StartTime;
	time_t  EndTime;

	int		StartMTime;
	int		EndMTime;

	unsigned int	uiSrcIP;
	unsigned int	uiDestIP;

	unsigned int    uiID;

	unsigned char	ucFlag;
	unsigned char	ucReqMsgCode;
	unsigned char	ucRepMsgCode;
	unsigned char	ucReqCount;

	unsigned char	ucNakCount;
	unsigned char	ucRejCount;
	unsigned char	ucReserved[6];
} T_SESS;

typedef struct _st_A11INFO_ {
	time_t  StartTime;
    time_t  EndTime;

    int     StartMTime;
    int     EndMTime;

    unsigned int    uiSrcIP;
    unsigned int    uiDestIP;

    unsigned char   ucFlag;
    unsigned char   ucReqMsgCode;
    unsigned char   ucRepMsgCode;
    unsigned char   ucReqCount;
    unsigned char   ucNakCount;
    unsigned char   ucRejCount;
	unsigned char   ucAirlink;
    unsigned char   ucRegiReply;

    unsigned int    uiID;
	unsigned short  usLifetime;
    unsigned short  usA11Flag;

	unsigned int    uiUpdateReason;
    unsigned int    uiAStopReason;
} st_A11INFO, *pst_A11INFO;

#define DEF_TSESS_SIZE		sizeof(T_SESS)
#define DEF_A11INFO_SIZE	sizeof(st_A11INFO)
	
typedef struct _st_psess_key {
	unsigned int	uiServingPCF;
	unsigned int	uiKey;

} st_psess_key, *pst_psess_key;


typedef struct _st_psess {
    st_psess_key 	key;

	time_t	CreateTime;
	time_t  LastUpdateTime;

	int		CreateMTime;	
	int		LastUpdateMTime;

	st_A11INFO	RegA11;
	st_A11INFO	UpA11;
	st_A11INFO	SessA11;
	T_SESS	UpLCP;
	T_SESS	DownLCP;
	T_SESS	UpLCPEcho;
	T_SESS	DownLCPEcho;
	T_SESS	CHAPAP;
	T_SESS	UpIPCP;
	T_SESS	DownIPCP;
	T_SESS 	LCPTerm;

	time_t	UpLCPStartTime;
	time_t  DownLCPStartTime;
	int		UpLCPStartMTime;
	int		DownLCPStartMTime;
	
	time_t	AuthReqTime;
	time_t  CHAPResTime;
	int		AuthReqMTime;
	int		CHAPResMTime;
	
	time_t	AuthEndTime;
	time_t  IPCPStartTime;
	int		AuthEndMTime;
	int		IPCPStartMTime;

	time_t	PPPSetupTime;
	time_t  PPPTermTime;
	int		PPPSetupMTime;
	int		PPPTermMTime;

	time_t	DNSReqTime;
	time_t  TCPSynTime;
	int		DNSReqMTime;
	int		TCPSynMTime;

	time_t  RPTTime;
	time_t  LastStopTime;
    int     RPTMTime;
	int		LastStopMTime;

	char	szMIN[MAX_MIN_SIZE];
	char	szTraceMIN[MAX_MIN_SIZE];
		
	unsigned int	OpTime;
	unsigned int	uiStopFlag;

	unsigned char	szNetOption[MAX_SVCOPTION_SIZE];    
	unsigned int    uiSvcOption;
    unsigned int    uiFMux;
    unsigned int    uiRMux;
	unsigned int	uiNASIP;	
	unsigned int	uiIPAddr;	
	unsigned int	uiHomeAgent;
	unsigned int	uiDNS;
	unsigned int	uiSvcDestIP;

	unsigned short  usSessLifetime;
    unsigned short  usSessA11Flag;

	unsigned short	usPPPFlag;
	unsigned short	usSvcFlag;

	unsigned char	ucDNSRet;
	unsigned char	ucAuthResultCode;
	unsigned char	ucAppType;
	unsigned char	ucAuthMethod;
	unsigned char	ucReserved[4];

	unsigned short	usCliPort;			 
	unsigned short	usSvcPort;
	unsigned char	ucSvcGroup;
	unsigned char	ucSvcCode;
	unsigned char	ucSvcProtocol;
	unsigned char	ucBSMSCChgCount;
	
	unsigned char	ucRegiReqCount;
	unsigned char	ucRegiSuccCount;
	unsigned char	ucUpdateReqCount;
	unsigned char	ucUpdateAckCount;
	unsigned char	ucUpLCPReqCount;
	unsigned char	ucDownLCPReqCount;
	unsigned char	ucUpIPCPReqCount;
	unsigned char	ucDownIPCPReqCount;
	
    unsigned char   ucAStartCount; 	/* Active Start count */
    unsigned char   ucAStopCount;  	/* Active Stop count */
	unsigned char	ucSvcOptChange;	/* Service Option change count */
	unsigned char	ucCHAPFailCount;
	unsigned char	ucPPPSetupCount;
	unsigned char	ucIPChgCount;
	unsigned char	ucDNSReqCount;
	unsigned char	ucDNSSuccCount;

	char			szAuthUserName[32];

	char			szCHAPFailMsg[64];

	unsigned int	uiRPUpTime;
	unsigned int	uiLCPDuration;
	unsigned int	uiIPCPDuration;
	unsigned int	uiAuthDuration;
	unsigned int	uiDNSRepDuration;
	unsigned int	uiTCPSetupDuration;
	unsigned int	uiIdleDuration;
	unsigned int	uiActiveDuration;

	unsigned int	uiUpGREFrames;
	unsigned int	uiDownGREFrames;
	unsigned int	uiUpGREBytes;
	unsigned int	uiDownGREBytes;
	
	unsigned int	uiUpPPPFrames;
	unsigned int	uiDownPPPFrames;
	unsigned int	uiUpPPPBytes;
	unsigned int	uiDownPPPBytes;

	unsigned int	uiUpFCSErrFrames;
	unsigned int	uiDownFCSErrFrames;
	unsigned int	uiUpFCSErrBytes;
	unsigned int	uiDownFCSErrBytes;

	unsigned int	uiUpAnaErrFrames;
	unsigned int	uiDownAnaErrFrames;

	unsigned int	uiSeq;			/* overall packet sequence 1-0xffff */
	short			siBufIndex;
	short			siReserved2;

	unsigned char   ucBSMSC[DEF_BSMSD_LENGTH];
	unsigned char	ucAlwaysOn;
	unsigned char	szReserved[2];

	unsigned int	uiCallType;		/* 발신 호와 착신 호를 구분한다. 0: 발신(기본) 1: 착신 */

	st_BSMSC		stBSMSC;
	
	T_PPP_INFO		stETH1;
	T_PPP_INFO		stETH2;

	T_PSESS_BUF		SESS_BUF;

} st_psess, *pst_psess;

/**B.1*  Definition of New Constants *********************/

/* DBMS의 기본 구조체를 DBMS의 구조체로 연결 */

#define	PSESS_KEY		st_psess_key
#define	PPSESS_KEY		pst_psess_key
#define	PSESS_DATA		st_psess
#define	PPSESS_DATA		pst_psess

/* 최대 record의 길이, 8byte 기준  */
#define JMM_PSESS_KEY_LEN	( sizeof(PSESS_KEY ) / 8)
#define JMM_PSESS_BODY_LEN	( (sizeof(PSESS_DATA ) / 8) - JMM_PSESS_KEY_LEN )

/* 이 DB의 Shared Memory Key    */
#define PSESS_SHM_KEY		S_SSHM_PSESS

/**B.2*  Definition of New Type  **************************/
/* Typical format of Record */
typedef struct JMM_typical_tbl_psess {
	PSESS_KEY	key;    /* Primary Key */
	long long  	body[JMM_PSESS_BODY_LEN];
	int    	left;
	int    	right;
	short  	bf;         /* balance factor*/
	short  	reserved[3];
} PSESS_TYPE;

/* Typical format of Database */
typedef struct {
	PSESS_TYPE	tuple[JMM_PSESS_RECORD];
	int		free;
	int     root;

	int		used;
	int		reserved[127];
} SESS_TABLE, *PSESS_TABLE;


/**C.1*  Declaration of Variables  ************************/

PSESS_TABLE		psess_tbl;
int	semid_psess;

/**D.1*  Definition of Functions  *************************/

int avl_insert_psess( PPSESS_KEY key, long long *body, int *root );
int left_rotation_psess( int index, int *pindex );
int right_rotation_psess( int index, int *pindex );
PSESS_TYPE *avl_search_psess( int root, PPSESS_KEY key );
int avl_delete_psess( int *root, PPSESS_KEY key );
PSESS_TYPE *avl_select_psess( int root, PPSESS_KEY first_key, PPSESS_KEY last_key );
int avl_update_psess( PSESS_TYPE *tree, long long *body );
PSESS_TYPE *get_psess( int index );
int psess_alloc();
void psess_dealloc( int index );

void Init_PSESS();
int Insert_PSESS( PPSESS_DATA disp );
PPSESS_DATA Search_PSESS( PPSESS_KEY key );
int Delete_PSESS( PPSESS_KEY key );
int Update_PSESS( PPSESS_DATA input );
PPSESS_DATA Select_PSESS( PPSESS_KEY first_key, PPSESS_KEY last_key );
PPSESS_DATA Filter_PSESS( PPSESS_KEY key );
int Count_PSESS();
int GetCount_PSESS(void);

#endif
