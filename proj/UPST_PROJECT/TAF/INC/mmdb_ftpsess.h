/**********************************************************
          ABLEX Main Memory Database TINYMMDB(TM)

   Author   : Jiyoon Chung
   Section  : 
   SCCS ID  : 
   Date     : 9/19/01
   Revision History : 
   		'01.  8.  4 Initial

   Description:
		

   Copyright (c) ABLEX 2001 
***********************************************************/

#ifndef __JMM_FTPSESS_DB_HEADER___
#define __JMM_FTPSESS_DB_HEADER___

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
#include <sys/time.h>
#include <unistd.h>

#include "typedef.h"
#include "define.h"


typedef struct _st_ftpsess_key {
	unsigned int	uSrvIP;
	unsigned int	uCliIP;

	unsigned short	usSrvPort;
	unsigned short	usCliPort;

	unsigned int	uReserved;
} st_ftpsess_key, *pst_ftpsess_key;


typedef struct _st_ftpsess {
    st_ftpsess_key key;

	struct timeval	tvTcpSynTime;
	struct timeval	tvTcpSetupTime;
	struct timeval	tvTcpRelTime;
	struct timeval	tvFtpLogonTime;
	struct timeval	tvFtpDataStartTime;
	struct timeval	tvFtpDataEndTime;

    USHORT  usFTPSessCount;
    USHORT  usFTPEndCount;
    USHORT  dRequestBufUsed;
    USHORT  dResponseBufUsed;

    char    cLongLine;
    char    cReqWaitState;
    char    cReqHdrFlag;
    char    cReqHdrValFlag;
    char    cResWaitState;
    UCHAR   ucFinAck1;
    UCHAR   ucFinAck2;
    UCHAR   ucFin1stFlag;

    char    cResHdrFlag;
    char    cResHdrValFlag;
    USHORT  usRedirectSessID;
	int		dUptimeIdx;

	char	cUrlEnd;
	char	cSSLTunnel;
	USHORT	usTCPFailCode;
	USHORT	usSynRtxType;
	short	reserved;

	UINT	uUpRetransCnt;
	UINT	uUpPacketCnt;

	UINT	uDownRetransCnt;
	UINT	uDownPacketCnt;

	UINT	uUpSize;
	UINT	uDownSize;

	UINT	uTotUpSize;
	UINT	uTotDownSize;

	UINT	uDataTransDuration;
	int		reserved2;

	UINT	dataip;
	UINT	dataport;

} st_ftpsess, *pst_ftpsess;


/* DBMS의 기본 구조체를 DBMS의 구조체로 연결 */

#define	FTPSESS_KEY		st_ftpsess_key
#define	PFTPSESS_KEY		pst_ftpsess_key
#define	FTPSESS_DATA		st_ftpsess
#define	PFTPSESS_DATA		pst_ftpsess

/* 최대 record의 개수 */
#define JMM_FTPSESS_RECORD	102

/* 최대 record의 길이, 8byte 기준  */
#define JMM_FTPSESS_KEY_LEN	( sizeof( FTPSESS_KEY ) / 8 )
#define JMM_FTPSESS_BODY_LEN	( ( sizeof( FTPSESS_DATA ) / 8 ) - JMM_FTPSESS_KEY_LEN )

/* 이 DB의 Shared Memory Key    */
#define FTPSESS_SHM_KEY       S_SSHM_FTPSESS

/**B.2*  Definition of New Type  **************************/
/* Typical format of Record */
typedef struct JMM_typical_tbl_ftpsess {
	FTPSESS_KEY	key;    /* Primary Key */
	long long   body[JMM_FTPSESS_BODY_LEN];
	/* struct  JMM_typical_tbl_ftpsess *left;  */ /* Index    left */
	/* struct  JMM_typical_tbl_ftpsess *right; */ /* Index    right */
	int     left;
	int     right;

	short   bf;         /* balance factor*/
	short   reserved[3];
} FTPSESS_TYPE;

/* Typical format of Database */
typedef struct {
	FTPSESS_TYPE	tuple[JMM_FTPSESS_RECORD];
	/* FTPSESS_TYPE	*free; */
	/* FTPSESS_TYPE	*root; */
	int         free;
	int         root;

	int			used_count;
	int			reserved[127];
} FTPSESS_TABLE;

/**C.1*  Declaration of Variables  ************************/

FTPSESS_TABLE	*ftpsess_tbl;

/**D.1*  Definition of Functions  *************************/
int avl_insert_ftpsess( PFTPSESS_KEY key, long long *body, int *root );
int left_rotation_ftpsess( int index, int *pindex );
int right_rotation_ftpsess( int index, int *pindex );
FTPSESS_TYPE *avl_search_ftpsess( int root, PFTPSESS_KEY key );
int avl_delete_ftpsess( int *root, PFTPSESS_KEY key );
FTPSESS_TYPE *avl_select_ftpsess( int root, PFTPSESS_KEY first_key, PFTPSESS_KEY last_key );
int avl_update_ftpsess( FTPSESS_TYPE *tree, long long *body );
FTPSESS_TYPE *get_ftpsess( int index );
int ftpsess_alloc();
void ftpsess_dealloc( int index );

void Init_FTPSESS();
int Insert_FTPSESS( PFTPSESS_DATA disp );
PFTPSESS_DATA Search_FTPSESS( PFTPSESS_KEY key );
int Delete_FTPSESS( PFTPSESS_KEY key );
int Update_FTPSESS( PFTPSESS_DATA disp, PFTPSESS_DATA input );
PFTPSESS_DATA Select_FTPSESS( PFTPSESS_KEY first_key, PFTPSESS_KEY last_key );
void KeyTo_FTPSESS( PFTPSESS_KEY key );
int GetCount_FTPSESS(void);

#endif
