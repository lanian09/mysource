/**A.1*  File Inclusion ***********************************/

#include <mmdblist_ftp.h>

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
	/* 시스템 Recovery용 */
	SESS_TYPE *dealloc = &pstSessTbl->tuple[dIndex];

	memset( &dealloc->key, 0x00, SESS_KEY_SIZE );

	/* 시스템 노드 free시킴 */
    pNode = &pstSessTbl->tuple[pstSessTbl->free];
	dealloc->right = pNode->right;
    pNode->right = dIndex;
}
