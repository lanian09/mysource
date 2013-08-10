/**********************************************************
                 ABLEX Main-Memory DBMS

   Author   : SangWoo Lee and Jiyoon Chung
   Section  :
   SCCS ID  : %W%
   Date     : %G%
   Revision History : 
        '99.  9. 21     Initial
		'01.  7. 23		Revised for TPK

   Description:
        

   Copyright (c) ABLEX 1999, 2000, and 2001
***********************************************************/

/**A.1*  File Inclusion ***********************************/
#include "mmdb_greentry.h"

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/

void Init_GREENTRY()
{
    int     i;

    /* Free List »ý¼º */
    for (i = 0;i < JMM_GREENTRY_RECORD - 1;i++) {
		greentry_tbl->tuple[i].right = i+1;
    }
    greentry_tbl->tuple[i].right = -1;

	greentry_tbl->free = 0;
    greentry_tbl->root = -1;
	greentry_tbl->used = 0;

}

