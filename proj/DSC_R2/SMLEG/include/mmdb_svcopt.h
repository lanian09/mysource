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

#ifndef __MM_SVCOPT_DB_HEADER___
#define __MM_SVCOPT_DB_HEADER___

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

typedef struct _st_svcopt_key {
	unsigned short	svcOpt;
	unsigned char	Reserved[6];
} st_svcopt_key, *pst_svcopt_key;

typedef struct _st_svcopt {
    st_svcopt_key key;
	unsigned short	svcType;
	unsigned char	layer;
	unsigned char	fout;
	unsigned char	block;
	unsigned char   urlCha;
	unsigned char	Reserved[2];
} st_svcopt, *pst_svcopt;

// DBMS의 기본 구조체를 DBMS의 구조체로 연결 

#define	SVCOPT_KEY				st_svcopt_key
#define	PSVCOPT_KEY				pst_svcopt_key
#define	SVCOPT_DATA				st_svcopt
#define	PSVCOPT_DATA			pst_svcopt

// Semaphore Key
#define S_SEMA_SVCOPT           0x20001

// 최대 record의 개수 Last Record Not Used && Min Value && Max Value == 3
#define JMM_SVCOPT_RECORD		100

// 최대 record의 길이, 8byte 기준 
#define JMM_SVCOPT_KEY_LEN		( sizeof( SVCOPT_KEY ) / 8 )
#define JMM_SVCOPT_BODY_LEN		( ( sizeof( SVCOPT_DATA ) / 8 ) - JMM_SVCOPT_KEY_LEN )


/**B.2*  Definition of New Type  **************************/
// Typical format of Record 
typedef struct JMM_typical_tbl_svcopt {
	SVCOPT_KEY	key;    					/* Primary Key */
	long long   body[JMM_SVCOPT_BODY_LEN];
	int			left;	/* Index left */
	int			right;	/* Index right */
	short		bf;		/* balance factor*/
	short   reserved[3];
} SVCOPT_TYPE;

// Typical format of Database 
typedef struct {
	SVCOPT_TYPE	tuple[JMM_SVCOPT_RECORD];
	int				free;			/* Index free */
	int				root;			/* Index root */
	unsigned int	uiCount;		/* MMDB Used tuple Count */
	int				reserved[127];
} SVCOPT_TABLE;

/**C.1*  Declaration of Variables  ************************/
extern SVCOPT_TABLE     *svcopt_tbl;
extern int             semid_svcopt;

/**D.1*  Definition of Functions  *************************/

int avl_insert_svcopt( PSVCOPT_KEY key, long long *body, int *root );
int left_rotation_svcopt( int index, int *pindex );
int right_rotation_svcopt( int index, int *pindex );
SVCOPT_TYPE *avl_search_svcopt( int root, PSVCOPT_KEY key );
int avl_delete_svcopt( int *root, PSVCOPT_KEY key );
SVCOPT_TYPE *avl_select_svcopt( int root, PSVCOPT_KEY first_key, PSVCOPT_KEY last_key );
int avl_update_svcopt( SVCOPT_TYPE *tree, long long *body );
SVCOPT_TYPE *get_svcopt( int index );

int svcopt_alloc();
void svcopt_dealloc( int index );

int Insert_SVCOPT( PSVCOPT_DATA disp );
PSVCOPT_DATA Search_SVCOPT( PSVCOPT_KEY key );
int Delete_SVCOPT( PSVCOPT_KEY key );
int Update_SVCOPT( PSVCOPT_DATA disp, PSVCOPT_DATA input );
PSVCOPT_DATA Select_SVCOPT( PSVCOPT_KEY first_key, PSVCOPT_KEY last_key );
void KeyTo_SVCOPT( PSVCOPT_KEY key );

#pragma pack()
#endif
