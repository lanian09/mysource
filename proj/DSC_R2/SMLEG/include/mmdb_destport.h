/**********************************************************
          ABLEX Main Memory Database TINYMMDB(TM)

   Author   : Jiyoon Chung
   Section  : 
   SCCS ID  : 
   Date     : 9/19/01
   Revision History : 
   		'01.  8.  4 Initial
		'04.  4. 20 Insert By LSH for MMDB Record Count

   Description:
		

   Copyright (c) ABLEX 2001, and 2004
***********************************************************/

#ifndef __MM_DESTPORT_DB_HEADER___
#define __MM_DESTPORT_DB_HEADER___

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
#include <sys/shm.h>

#include <ipaf_define.h>
#include <ipaf_sem.h>

#pragma pack(1)
/**B.1*  Definition of New Constants *********************/

typedef struct _st_destport_key {
    unsigned char	ucProtocol;		/* TCP : 6, UDP : 17 */
	unsigned char	Reserved;
	unsigned short	usDestPort;

    unsigned int	uiDestIP;
} st_destport_key, *pst_destport_key;

typedef struct _st_destport {
    st_destport_key key;

	unsigned int	uiNetmask;
    unsigned short  usCatID;
   	unsigned char   ucGroupID;
	unsigned char   ucSerial;

	unsigned char   ucFilterOut;
	unsigned char   ucLayer;
	unsigned char   ucMode;
	unsigned char   ucSvcBlk;
	unsigned char	ucIPType;
	unsigned char	ucURLChar;
	unsigned char   ucCDRIPFlag;
	unsigned char   ucUDRFlag; // add by helca	
} st_destport, *pst_destport;

// 최대 record의 개수 Last Record Not Used && Min Value && Max Value == 3
#define JMM_DESTPORT_RECORD		1003

// hwang woo hyoung add 2003/05/28
typedef struct _st_DestIPList
{
    int         dCount;
    int         dReserved;
    st_destport stDestIP[JMM_DESTPORT_RECORD];
}stDestIPList, *pstDestIPList;

// DBMS의 기본 구조체를 DBMS의 구조체로 연결 

#define	DESTPORT_KEY			st_destport_key
#define	PDESTPORT_KEY			pst_destport_key
#define	DESTPORT_DATA			st_destport
#define	PDESTPORT_DATA			pst_destport

// 최대 record의 길이, 8byte 기준 
#define JMM_DESTPORT_KEY_LEN	( sizeof( DESTPORT_KEY ) / 8 )
#define JMM_DESTPORT_BODY_LEN	( ( sizeof( DESTPORT_DATA ) / 8 ) - JMM_DESTPORT_KEY_LEN )

// 이 DB의 Shared Memory Key
#define DESTPORT_SHM_KEY        S_SSHM_MMDBDESTPORT

/**B.2*  Definition of New Type  **************************/
// Typical format of Record 
typedef struct JMM_typical_tbl_destport {
	DESTPORT_KEY	key;    					/* Primary Key */
	long long   body[JMM_DESTPORT_BODY_LEN];
	int			left;
	int			right;
	short		bf;         					/* balance factor*/
	short   	reserved[3];
} DESTPORT_TYPE;

// Typical format of Database 
typedef struct {
	DESTPORT_TYPE	tuple[JMM_DESTPORT_RECORD];
	int				free;
	int				root;
	unsigned int	uiCount;		/* MMDB Used Record Count */
	int				reserved[127];
} DESTPORT_TABLE;

/**C.1*  Declaration of Variables  ************************/

extern DESTPORT_TABLE	*destport_tbl;
extern int 				semid_destport;		/* semaphore id */
/**D.1*  Definition of Functions  *************************/

int avl_insert_destport( PDESTPORT_KEY key, long long *body, int *root );
int left_rotation_destport( int index, int *pindex );
int right_rotation_destport( int index, int *pindex );
DESTPORT_TYPE *avl_search_destport( int root, PDESTPORT_KEY key );
int avl_delete_destport( int *root, PDESTPORT_KEY key );
DESTPORT_TYPE *avl_select_destport( int root, PDESTPORT_KEY first_key, PDESTPORT_KEY last_key );
int avl_update_destport( DESTPORT_TYPE *tree, long long *body );
DESTPORT_TYPE *get_destport( int index );

int destport_alloc();
void destport_dealloc( int index );

int Insert_DESTPORT( PDESTPORT_DATA disp );
PDESTPORT_DATA Search_DESTPORT( PDESTPORT_KEY key );
int Delete_DESTPORT( PDESTPORT_KEY key );
int Update_DESTPORT( PDESTPORT_DATA disp, PDESTPORT_DATA input );
PDESTPORT_DATA Select_DESTPORT( PDESTPORT_KEY first_key, PDESTPORT_KEY last_key );
PDESTPORT_DATA Filter_DESTPORT( PDESTPORT_KEY key );

#pragma pack()
#endif
