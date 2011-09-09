/**********************************************************
                 ABLEX 

   Author   : SangHo Lee
   Section  : SCE
   SCCS ID  : %W%
   Date     : %G%
   Revision History : 
        '99.  9. 21     Initial
		'02.  12.		miheeh: 데이터구조를 pointer에서 index로 변환
   Description:
		MMDB V2.0

   Copyright (c) ABLEX
***********************************************************/

/**A.1*  File Inclusion ***********************************/

#include "mmdb_greentry.h"

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/

/****************************************************************
/		Alloc(): 비어있는 튜플을 할당
/			freelist: linked list로 된 튜플중 제일 첫번째 값을
/					  가리킴.
/
/ freelist+------+    +------+       +-----+
/    +--->|      | -->|      |....-->|     |-->NULL
/	   	  +------+    +------+       +-----+
/
/
/		Dealloc(): 지워진 튜플을 새로운 자원으로 만듬.
/		 	 dealloc : dealloc은 freelist바로뒤에 붙어서 
/					   linked list의 일원이 됨.
/
/  freelist+------+    +------+       +-----+
/     +--->|      | +->|      |....-->|     |-->NULL
/		   +------+ |  +------+       +-----+
/		       |	+--------------------+ 
/		       |	  	dealloc +------+ |
/		       +--------------->|      |-+
/			     				+------+
/
****************************************************************/

int greentry_alloc()
{
    GREENTRY_TYPE *node;
	int index;

	index = greentry_tbl->free;
	node = &greentry_tbl->tuple[greentry_tbl->free];
	if( index == node->right || node->right == -1 )
	{	/* not initialize or full */
		return -1;
	}

	greentry_tbl->free = node->right;

	return(index);
}

void greentry_dealloc(int index)
{
	GREENTRY_TYPE *node;
	/* 시스템 Recovery용 */
	GREENTRY_TYPE *dealloc = &greentry_tbl->tuple[index];

	//memset( &dealloc->key, 0x00, sizeof( GREENTRY_KEY ) );

	/* 시스템 노드 free시킴 */
    node = &greentry_tbl->tuple[greentry_tbl->free];
	dealloc->right = node->right;
	node->right = index;
}
