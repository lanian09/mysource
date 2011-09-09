/**********************************************************
                 ABLEX Main-Memory DBMS

   Author   : LEE SANG HO
   Section  : IPAS Project
   SCCS ID  : @(#)mmdblist_tbl.h	1.6
   Date     : 03/05/04
   Revision History :
        '04.  03. 05     Initial

   Description:
        Recovery

   Copyright (c) ABLEX 2004
***********************************************************/

#ifndef __MMDBLIST_HTTP_TABLE_DEFINE___
#define __MMDBLIST_HTTP_TABLE_DEFINE___

/**A.1*  File Inclusion ***********************************/
#include <define.h>
#include <typedef.h>
#include <common_stg.h>

/**B.1*  Definition of New Constants *********************/

/********* MMDB SESSION DEFINITION *********/
#define MAX_SESS_RECORD		10003						/* MAX MMDB SESSION RECORD COUNT */
#define MAX_REALSESS_RECORD	MAX_SESS_RECORD-1			/* MAX REAL MMDB SESSION RECORD COUNT */

#define DEF_TIMEOUT_TIME	60	
#define DEF_TIMEOUT_TIME1	300

/**B.2*  Definition of New Type  **************************/
typedef struct _st_LOG_FTP_INFO_
{
	USHORT	usPlatformType;
	USHORT	usSvcL4Type;
	USHORT	usSvcL7Type;
	USHORT	usReserved;

	UINT	uiServerIP;
    UINT	uiFTPLogonDuration;

    UINT	uiEndTime;
    UINT	uiEndMTime;
    UINT	uiEndDuration;
    UINT	uiTcpSynTime;
    UINT	uiTcpSynMTime;
    UINT	uiTcpSynAckTime;

    UINT	uiUpFrames;
    UINT	uiDownFrames;
    UINT	uiUpBytes;
    UINT	uiDownBytes;

    UINT	uiUpRetranBytes;
    UINT	uiDownRetranBytes;

    UINT	uiFTPSynTime;
    UINT	uiFTPSynMTime;
    UINT	uiFTPFinTime;
    UINT	uiFTPFinMTime;

    UINT	uiFTPUpFirstSeq;
    UINT	uiFTPUpLastSeq;
    UINT	uiFTPDownFirstSeq;
    UINT	uiFTPDownLastSeq;
    UINT	uiLastUpdateTime;
    UINT	uiFTPTimeDuration;
    USHORT	usCliFTPStatus;
    USHORT	usSvrFTPStatus;
	UINT	uiClientIP;

    UINT	OpStartTime;                            /**<  분석 시작 Time */
    UINT	OpStartMTime;                           /**<  분석 시작 Micro Time */
    UINT	OpEndTime;                              /**<  마지막 처리 Time */
    UINT	OpEndMTime;                             /**<  마지막 처리 Micro Time */
} LOG_FTP_INFO;

#define DEF_LOG_FTP_INFO_SIZE		sizeof(LOG_FTP_INFO)

/********* MMDB SESSION DEFINITION *********/

/* MMDB SESSION KEY */
typedef struct _st_SessKey {
	UINT			uiSrcIP;
	UINT			uiDestIP;
	USHORT			usSrcPort;
	USHORT			usDestPort;
	UINT			uiReserved;
} st_SessKey, *pst_SessKey;

#define SESS_KEY             st_SessKey
#define PSESS_KEY            pst_SessKey

/* TABLE Dispatcher's Record Details ***/
typedef struct _st_SessData 
{
	/* Management Parameters */
	SESS_KEY		stKey;			/* My Key */

	char			cStorRetrFlag;	/* 1: FTP CMD STOR or RETR Received */
	char			reserved[3];
	int				dTimeOutTime;

	LOG_FTP_INFO	stFTPSTAT;

	/**** TCP INFO ****/
	SESS_KEY		stSignalKey;
	SESS_KEY		stDataKey;
} st_SessData, *pst_SessData;

#define SESS_DATA            st_SessData
#define PSESS_DATA           pst_SessData

// 최대 record의 길이, 8byte 기준
#define MAX_SESS_KEY_LEN     ( sizeof( SESS_KEY ) / 8 )
#define MAX_SESS_BODY_LEN    ( ( sizeof( SESS_DATA ) / 8 ) - MAX_SESS_KEY_LEN )

typedef struct _st_MMDBSESSION {
    SESS_KEY	key;    		/* Primary Key */
    INT64   	body[MAX_SESS_BODY_LEN];
    INT         left;   		/* Index left */
    INT         right;  		/* Index right */
    SHORT       bf;     		/* balance factor*/
    SHORT       reserved[3];
} SESS_TYPE, *PSESS_TYPE;

// Typical format of Database
typedef struct _st_MMDB_SESSION_TABLE {
    SESS_TYPE	tuple[MAX_SESS_RECORD];
    INT			free;   		/* index free */
    INT			root;   		/* index root */
	UINT		uiSessCount;	/* Current Used Session Count */
	CHAR		szReserved[128];
} SESS_TABLE, *pSESS_TABLE;

#define SESS_TABLE_SIZE		sizeof(SESS_TABLE)
#define SESS_TYPE_SIZE		sizeof(SESS_TYPE)
#define SESS_KEY_SIZE		sizeof(SESS_KEY)

typedef struct _st_FTPLIST_TABLE {
    SESS_TABLE      stSessTbl;
} st_FTPLIST_TABLE, *pFTPLIST_TABLE;

#define FTPLIST_TABLE      st_FTPLIST_TABLE

#define FTPLIST_TABLE_SIZE     sizeof(FTPLIST_TABLE)  

/**C.1*  Declaration of Variables  ************************/
extern int             gdMMDBSHMID;        /* SHM ID */
extern pFTPLIST_TABLE pstMMDBLISTTbl;       /* MMDB&LIST TABLE POINTER */
extern pSESS_TABLE     pstSessTbl;         /* SESSION MMDB POINTER */

/**D.1*  Definition of Functions  *************************/

/********* MMDB SESSION INTERNAL FUNCATION DEFINITION *********/
int avl_insert_tcp( PSESS_KEY key, long long *body, int *root);
int left_rotation_tcp( int index, int *pindex );
SESS_TYPE *avl_search_tcp( int root, PSESS_KEY key );
int avl_delete_tcp( int *root, PSESS_KEY key );
SESS_TYPE *avl_select_tcp( int root, PSESS_KEY first_key, PSESS_KEY last_key );
int avl_update_tcp( SESS_TYPE *tree, long long *body );
SESS_TYPE *get_tcp( int index);
void Init_SessDB_FTP();
int sess_alloc();
void sess_dealloc( int index );
int Insert_SESS(PSESS_DATA pstSrc);			
PSESS_DATA Search_SESS(PSESS_KEY stKey);
int Delete_SESS(PSESS_KEY pstKey);
int Update_SESS(PSESS_DATA disp, PSESS_DATA input);
PSESS_DATA pstSelectMMDB(PSESS_KEY pstFKey, PSESS_KEY pstLKey);

/********* Initialize INTERNAL MMDB & LINKED LIST *********/
int dMmapGet_FTP(char *path);
int dShmGet_FTP(int dSHMKey);

/********* USER FUNCTION DEFINITION *********/
/* Initialize Function */
int dInitMMAPMMDB_FTP(char *szMMAPFile); 		/* Initializing Function Used For MMAP */
int dInitSHMMMAB_FTP(int dMMDBSHMKey);			/* Initializing Function Used For SHM */

/* MMDB User Function */
int dInsertMMDB(PSESS_DATA pstSrc, PSESS_DATA *ppstDst);
int dSetMMDB(PSESS_KEY pstKey, PSESS_DATA *pstMMDB);
int dFreeMMDB(PSESS_DATA pstMMDB);

/* Timer User Function */
int dTimerListLastProc(time_t stCurrTime, UINT uiTimeOut, UINT uiCheck);
int dTimerListFullProc(time_t stCurrTime, UINT uiTimeOut, UINT uiCheck);

#endif
