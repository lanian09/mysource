/**A.1*  File Inclusion ***********************************/
#include <common_stg.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <mmdblist_ftp.h>


/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/
void Init_SessDB_FTP()
{
	int		i;
	
	for(i=0; i < MAX_SESS_RECORD-1; i++)
		pstSessTbl->tuple[i].right = i+1;		

	pstSessTbl->tuple[i].right = -1;

    pstSessTbl->free = 0;
    pstSessTbl->root = -1;
    pstSessTbl->uiSessCount = 0;

}

int Insert_SESS(PSESS_DATA pstSrc)
{
	int			ret = 0;
	SESS_KEY	*P;
	long long 	*P2;

	P = (SESS_KEY *)pstSrc;
	P2 = (long long *)pstSrc;
	ret = avl_insert_tcp(P, &P2[MAX_SESS_KEY_LEN], &pstSessTbl->root);

	return ret;
}

PSESS_DATA Search_SESS(PSESS_KEY pstKey)
{
	PSESS_DATA pstData;

	pstData = (PSESS_DATA)avl_search_tcp(pstSessTbl->root, pstKey);

	return pstData;
}

int Delete_SESS(PSESS_KEY pstKey)
{
	int ret;
	
	ret = avl_delete_tcp(&pstSessTbl->root, pstKey);

	return ret;
}

PSESS_DATA Select_SESS(PSESS_KEY first_key, PSESS_KEY last_key)
{
	PSESS_DATA pstData;

    pstData = (PSESS_DATA)avl_select_tcp(pstSessTbl->root, first_key, last_key);

	return pstData;
}

int Update_SESS(PSESS_DATA disp, PSESS_DATA input)
{
	int ret;
    long long *P2;
    P2 = (long long *)input;

    ret = avl_update_tcp((SESS_TYPE *)disp, &P2[MAX_SESS_KEY_LEN]);

	return ret;
}

PSESS_DATA pstSelectMMDB(PSESS_KEY pstFKey, PSESS_KEY pstLKey)
{
    PSESS_DATA      pstData;

    pstData = (PSESS_DATA)avl_select_tcp(pstSessTbl->root, pstFKey, pstLKey);

    return pstData;
}
