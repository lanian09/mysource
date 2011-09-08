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
		

   Copyright (c) ABLEX 2001 and 2004
***********************************************************/

#ifndef __MM_DESTIP_DB_HEADER___
#define __MM_DESTIP_DB_HEADER___

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

#define DEF_ALL_ACCT			4
#define DEF_CONTENT				3
#define DEF_SERVICE_CONTENT		2
#define DEF_SERVICE				1

typedef struct _st_destip_key {
	unsigned char	ucFlag;		/* S: SrcIP block D: DestIP block */
	unsigned char	Reserved[3];
    unsigned int	uiIP;
} st_destip_key, *pst_destip_key;

typedef struct _st_destip {
    st_destip_key key;

	unsigned int	uiNetmask;
	unsigned short	usCatID;
	unsigned char	ucGroupID;
	unsigned char	ucSerial;

	unsigned char	ucFilterOut;
	unsigned char   ucLayer;
	unsigned char   ucMode;
	unsigned char   ucSvcBlk;
	unsigned char	ucIPType;
	unsigned char	ucURLChar;
	unsigned char	ucCDRIPFlag;
	unsigned char   ucUDRFlag; // add by helca
	unsigned char   ucPdsnType;	// 0:1x, 1:1xEV-DO
	unsigned char	ucReserved[7];
} st_destip, *pst_destip;

// DBMS의 기본 구조체를 DBMS의 구조체로 연결 

#define	DESTIP_KEY				st_destip_key
#define	PDESTIP_KEY				pst_destip_key
#define	DESTIP_DATA				st_destip
#define	PDESTIP_DATA			pst_destip

// 최대 record의 개수 Last Record Not Used && Min Value && Max Value == 3
#define JMM_DESTIP_RECORD		1003

// 최대 record의 길이, 8byte 기준 
#define JMM_DESTIP_KEY_LEN		( sizeof( DESTIP_KEY ) / 8 )
#define JMM_DESTIP_BODY_LEN		( ( sizeof( DESTIP_DATA ) / 8 ) - JMM_DESTIP_KEY_LEN )

// 이 DB의 Shared Memory Key
#define DESTIP_SHM_KEY          S_SSHM_MMDBDESTIP

/**B.2*  Definition of New Type  **************************/
// Typical format of Record 
typedef struct JMM_typical_tbl_destip {
	DESTIP_KEY	key;    					/* Primary Key */
	long long   body[JMM_DESTIP_BODY_LEN];
	int			left;	/* Index left */
	int			right;	/* Index right */
	short		bf;		/* balance factor*/
	short   reserved[3];
} DESTIP_TYPE;

// Typical format of Database 
typedef struct {
	DESTIP_TYPE	tuple[JMM_DESTIP_RECORD];
	int				free;			/* Index free */
	int				root;			/* Index root */
	unsigned int	uiCount;		/* MMDB Used tuple Count */
	int				reserved[127];
} DESTIP_TABLE;

/**C.1*  Declaration of Variables  ************************/

extern DESTIP_TABLE	*destip_tbl;
extern int			semid_destip;	/* semaphore id */

/**D.1*  Definition of Functions  *************************/

int avl_insert_destip( PDESTIP_KEY key, long long *body, int *root );
int left_rotation_destip( int index, int *pindex );
int right_rotation_destip( int index, int *pindex );
DESTIP_TYPE *avl_search_destip( int root, PDESTIP_KEY key );
int avl_delete_destip( int *root, PDESTIP_KEY key );
DESTIP_TYPE *avl_select_destip( int root, PDESTIP_KEY first_key, PDESTIP_KEY last_key );
int avl_update_destip( DESTIP_TYPE *tree, long long *body );
DESTIP_TYPE *get_destip( int index );

int destip_alloc();
void destip_dealloc( int index );

int Insert_DESTIP( PDESTIP_DATA disp );
PDESTIP_DATA Search_DESTIP( PDESTIP_KEY key );
int Delete_DESTIP( PDESTIP_KEY key );
int Update_DESTIP( PDESTIP_DATA disp, PDESTIP_DATA input );
PDESTIP_DATA Select_DESTIP( PDESTIP_KEY first_key, PDESTIP_KEY last_key );
void KeyTo_DESTIP( PDESTIP_KEY key );
PDESTIP_DATA Filter_DESTIP( PDESTIP_KEY key );

#pragma pack()
#endif
