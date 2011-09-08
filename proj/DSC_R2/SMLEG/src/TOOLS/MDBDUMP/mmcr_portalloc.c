/**********************************************************
                 ABLEX SCE 

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

#include "mmdb_destport.h"

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

int destport_alloc()
{
    DESTPORT_TYPE *node;
	int index;
	/*
	if (destport_tbl->free->right == NULL)
		return NULL;
    
    node = destport_tbl->free;
    destport_tbl->free = destport_tbl->free->right;

    return(node);
	*/

	node = &destport_tbl->tuple[destport_tbl->free];
	if( node->right == -1 )
		return -1;

	index = destport_tbl->free;
	destport_tbl->free = node->right;

	return(index);
}

void destport_dealloc(int index)
{
	DESTPORT_TYPE *node;
	/* �ý��� Recovery�� */
	DESTPORT_TYPE *dealloc = &destport_tbl->tuple[index];
	
	memset( &dealloc->key, 0x00, sizeof( DESTPORT_KEY ) );

	/* �ý��� ��� free��Ŵ */
    node = &destport_tbl->tuple[destport_tbl->free];
	dealloc->right = node->right;
    node->right = index;
}
