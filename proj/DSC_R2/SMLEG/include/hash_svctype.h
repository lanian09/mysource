/*******************************************************************************
                 uPresto Main-Memory DBMS

   Author   : LEE DONG HWAN 
   Section  : BSD Project
   SCCS ID  : @(#)hash_svctype.h 1.0
   Date     : 11/07/08
   Revision History :
        '08.  11. 07     Initial

   Description:
        Recovery

   Copyright (c) uPresto 2005
*******************************************************************************/

#ifndef _HASH_PDSN_H_
#define _HASH_PDSN_H_

/**A.1*  FILE INCLUSION *******************************************************/
#include <hasho.h>

#pragma pack(1)
/**B.1*  DEFINITION OF NEW CONSTANTS ******************************************/

#define MAX_HASH_PDSN_CNT			37 // PDSN CNT 32 보다 큰 가장 작은 소수 

typedef struct _st_Pdsn_Hash_
{
	unsigned int		uiIP;
} st_Pdsn_Hash, *pst_Pdsn_Hash;
#define DEF_PDSN_HASH_SIZE	sizeof(st_Pdsn_Hash)

#endif
