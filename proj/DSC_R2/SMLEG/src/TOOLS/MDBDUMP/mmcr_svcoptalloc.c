/**********************************************************
                 ABLEX 

   Author   : SangHo Lee
   Section  : SCE
   SCCS ID  : %W%
   Date     : %G%
   Revision History : 
        '99.  9. 21     Initial

   Description:
		VPNSERV

   Copyright (c) ABLEX
***********************************************************/

/**A.1*  File Inclusion ***********************************/

#include "mmdb_svcopt.h"

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

int svcopt_alloc()
{
    SVCOPT_TYPE *node;
	int index;

	node = &svcopt_tbl->tuple[svcopt_tbl->free];
	if( node->right == -1 )
		return -1;

	index = svcopt_tbl->free;
	svcopt_tbl->free = node->right;

	return(index);
}

void svcopt_dealloc(int index)
{
	SVCOPT_TYPE *node;
	/* 시스템 Recovery용 */
	SVCOPT_TYPE *dealloc = &svcopt_tbl->tuple[index];

	memset( &dealloc->key, 0x00, sizeof( SVCOPT_KEY ) );

	/* 시스템 노드 free시킴 */
    node = &svcopt_tbl->tuple[svcopt_tbl->free];
	dealloc->right = node->right;
	node->right = index;
}
