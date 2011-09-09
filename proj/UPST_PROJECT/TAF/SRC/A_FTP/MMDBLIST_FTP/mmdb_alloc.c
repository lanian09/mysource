/**A.1*  File Inclusion ***********************************/

#include <mmdblist_ftp.h>

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

int sess_alloc()
{
    SESS_TYPE 	*pNode;
	int 		dIndex;

	pNode = &pstSessTbl->tuple[pstSessTbl->free];
	if( pNode->right == -1 )
		return -1;

	dIndex = pstSessTbl->free;
	pstSessTbl->free = pNode->right;

	return(dIndex);
}

void sess_dealloc(int dIndex)
{
	SESS_TYPE *pNode;
	/* �ý��� Recovery�� */
	SESS_TYPE *dealloc = &pstSessTbl->tuple[dIndex];

	memset( &dealloc->key, 0x00, SESS_KEY_SIZE );

	/* �ý��� ��� free��Ŵ */
    pNode = &pstSessTbl->tuple[pstSessTbl->free];
	dealloc->right = pNode->right;
    pNode->right = dIndex;
}
