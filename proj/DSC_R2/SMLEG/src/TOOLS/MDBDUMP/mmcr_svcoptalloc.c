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
/		Alloc(): ����ִ� Ʃ���� �Ҵ�
/			freelist: linked list�� �� Ʃ���� ���� ù��° ����
/					  ����Ŵ.
/
/ freelist+------+    +------+       +-----+
/    +--->|      | -->|      |....-->|     |-->NULL
/	   	  +------+    +------+       +-----+
/
/
/		Dealloc(): ������ Ʃ���� ���ο� �ڿ����� ����.
/		 	 dealloc : dealloc�� freelist�ٷεڿ� �پ 
/					   linked list�� �Ͽ��� ��.
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
	/* �ý��� Recovery�� */
	SVCOPT_TYPE *dealloc = &svcopt_tbl->tuple[index];

	memset( &dealloc->key, 0x00, sizeof( SVCOPT_KEY ) );

	/* �ý��� ��� free��Ŵ */
    node = &svcopt_tbl->tuple[svcopt_tbl->free];
	dealloc->right = node->right;
	node->right = index;
}
