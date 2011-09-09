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

#include "mmdb_destip.h"

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

//DESTIP_TYPE *destip_alloc()
int destip_alloc()
{
    DESTIP_TYPE *node;
	int index;
	/*
	if (destip_tbl->free->right == NULL)
		return NULL;
    
    node = destip_tbl->free;
    destip_tbl->free = destip_tbl->free->right;

    return(node);
	*/

	node = &destip_tbl->tuple[destip_tbl->free];
	if( node->right == -1 )
		return -1;

	index = destip_tbl->free;
	destip_tbl->free = node->right;

	return(index);
}

//void destip_dealloc(DESTIP_TYPE *dealloc)
void destip_dealloc(int index)
{
	DESTIP_TYPE *node;
	/* �ý��� Recovery�� */
	DESTIP_TYPE *dealloc = &destip_tbl->tuple[index];

	memset( &dealloc->key, 0x00, sizeof( DESTIP_KEY ) );

	/* �ý��� ��� free��Ŵ */
    node = &destip_tbl->tuple[destip_tbl->free];
	dealloc->right = node->right;
    //destip_tbl->free->right = dealloc;
	node->right = index;
}
