/**********************************************************
                 ABLEX 

   Author   : SangHo Lee
   Section  : SCE
   SCCS ID  : %W%
   Date     : %G%
   Revision History : 
        '99.  9. 21     Initial
		'02.  12.		miheeh: �����ͱ����� pointer���� index�� ��ȯ
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
	/* �ý��� Recovery�� */
	GREENTRY_TYPE *dealloc = &greentry_tbl->tuple[index];

	//memset( &dealloc->key, 0x00, sizeof( GREENTRY_KEY ) );

	/* �ý��� ��� free��Ŵ */
    node = &greentry_tbl->tuple[greentry_tbl->free];
	dealloc->right = node->right;
	node->right = index;
}
