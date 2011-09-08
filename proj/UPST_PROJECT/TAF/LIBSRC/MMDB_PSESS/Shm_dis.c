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

#include "mmdb_psess.h"

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

int psess_alloc()
{
    PSESS_TYPE *node;
	int index;

	index = psess_tbl->free;
	node = &psess_tbl->tuple[psess_tbl->free];
	if( index == node->right || node->right == -1 )
	{	/* not initialize or full */
		return -1;
	}

	psess_tbl->free = node->right;

	return(index);
}

void psess_dealloc(int index)
{
	PSESS_TYPE *node;
	/* �ý��� Recovery�� */
	PSESS_TYPE *dealloc = &psess_tbl->tuple[index];

	//memset( &dealloc->key, 0x00, sizeof( PSESS_KEY ) );

	/* �ý��� ��� free��Ŵ */
    node = &psess_tbl->tuple[psess_tbl->free];
	dealloc->right = node->right;
	node->right = index;
}
