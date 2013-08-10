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

#ifndef __GREENTRY_DB_HEADER___
#define __GREENTRY_DB_HEADER___

/**A.1*  File Inclusion ***********************************/
#include <common_stg.h>
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

#include <typedef.h>
#include "VJ.h"

/* 최대 record의 개수 */
#define JMM_GREENTRY_RECORD    50002    

#define GREENTRY_BUF_MAX   (JMM_GREENTRY_RECORD - 2)

typedef struct _st_greentry_key {
	unsigned int	uiPCFIP;
	unsigned int	uiGREKey;
} st_greentry_key, *pst_greentry_key;

typedef struct _st_greentry {
	st_greentry_key key;
	unsigned int	uiMainGREKey;
	char			reserved[4];
} st_greentry, *pst_greentry;

/**B.1*  Definition of New Constants *********************/

/* DBMS의 기본 구조체를 DBMS의 구조체로 연결 */

#define	GREENTRY_KEY		st_greentry_key
#define	PGREENTRY_KEY		pst_greentry_key
#define	GREENTRY_DATA		st_greentry
#define	PGREENTRY_DATA		pst_greentry

/* 최대 record의 길이, 8byte 기준  */
#define JMM_GREENTRY_KEY_LEN	( sizeof(GREENTRY_KEY ) / 8)
#define JMM_GREENTRY_BODY_LEN	( (sizeof(GREENTRY_DATA ) / 8) - JMM_GREENTRY_KEY_LEN )

/* 이 DB의 Shared Memory Key    */
#define GREENTRY_SHM_KEY		S_SSHM_GREENTRY

/**B.2*  Definition of New Type  **************************/
/* Typical format of Record */
typedef struct JMM_typical_tbl_greentry {
	GREENTRY_KEY	key;    /* Primary Key */
	long long  	body[JMM_GREENTRY_BODY_LEN];
	int    	left;
	int    	right;
	short  	bf;         /* balance factor*/
	short  	reserved[3];
} GREENTRY_TYPE;

/* Typical format of Database */
typedef struct {
	GREENTRY_TYPE	tuple[JMM_GREENTRY_RECORD];
	int		free;
	int     root;

	int		used;
	int		reserved[127];
} GREENTRY_TABLE, *PGREENTRY_TABLE;


/**C.1*  Declaration of Variables  ************************/

PGREENTRY_TABLE		greentry_tbl;
int	semid_greentry;

/**D.1*  Definition of Functions  *************************/

int avl_insert_greentry( PGREENTRY_KEY key, long long *body, int *root );
int left_rotation_greentry( int index, int *pindex );
int right_rotation_greentry( int index, int *pindex );
GREENTRY_TYPE *avl_search_greentry( int root, PGREENTRY_KEY key );
int avl_delete_greentry( int *root, PGREENTRY_KEY key );
GREENTRY_TYPE *avl_select_greentry( int root, PGREENTRY_KEY first_key, PGREENTRY_KEY last_key );
int avl_update_greentry( GREENTRY_TYPE *tree, long long *body );
GREENTRY_TYPE *get_greentry( int index );
int greentry_alloc();
void greentry_dealloc( int index );

void Init_GREENTRY();
int Insert_GREENTRY( PGREENTRY_DATA disp );
PGREENTRY_DATA Search_GREENTRY( PGREENTRY_KEY key );
int Delete_GREENTRY( PGREENTRY_KEY key );
int Update_GREENTRY( PGREENTRY_DATA input );
PGREENTRY_DATA Select_GREENTRY( PGREENTRY_KEY first_key, PGREENTRY_KEY last_key );
PGREENTRY_DATA Filter_GREENTRY( PGREENTRY_KEY key );
int Count_GREENTRY();
int GetCount_GREENTRY(void);

#endif
