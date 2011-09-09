/*********************************************************
                 ABLEX IPAS Project (IPAM BLOCK)

   Author   : LEE SANG HO
   Section  : IPAS(IPAM) Project
   SCCS ID  : %W%
   Date     : %G%
   Revision History :
        '03.    01. 15. initial

   Description:

   Copyright (c) ABLEX 2003
*********************************************************/

#ifndef _SESSIF_H
#define _SESSIF_H

#include <time.h>
#include <ipaf_svc.h>
#include <define.h>

typedef struct _st_RADIUS_HEAD
{
	UCHAR	ucCode;
	UCHAR	ucID;
	short	usLength;
	UCHAR	szAuth[16];
} st_RADIUS_HEAD, *pst_RADIUS_HEAD;

#endif
