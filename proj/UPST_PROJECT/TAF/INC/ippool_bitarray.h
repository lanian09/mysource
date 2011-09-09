/*******************************************************************************
                LGT BSD PROJECT 

   Author   : LEE DONG HWAN 
   Section  : BSD Project
   SCCS ID  : @(#)ippool_bitarray.h	1.0
   Date     : 11/07/08
   Revision History :
        '08.  11. 07     Initial

   Description:
        Recovery

   Copyright (c) uPresto 2005
*******************************************************************************/

#ifndef __IPPOOL_BITARRAY_DEFINE___
#define __IPPOOL_BITARRAY_DEFINE___

/**A.1*  FILE INCLUSION *******************************************************/

/**B.1*  DEFINITION OF NEW CONSTANTS ******************************************/
/*
* DEFINITION
*/
#define IPPOOL_SIZE			4294967296					/* 256*256*256*256 */
#define IPPOOL_BASE			sizeof(int)					/* 4 */
#define BPBYTE				8							/* BIT PER BYTE */
#define FD_NFDBITS			(IPPOOL_BASE*BPBYTE)		/* */


/*
* MACRO
*/
#define _IPPOOL_ZERO(p)				bzero( (p), sizeof(*(p)) )									/* 모두 0으로 셋팅 */
#define _IPPOOL_SET(n, p)			p->IPPOOL[n/FD_NFDBITS] |= (1ul << (n%FD_NFDBITS))			/* 특정 IP를 1로 셋팅 */
#define _IPPOOL_CLR(n, p)			p->IPPOOL[n/FD_NFDBITS] &= ~(1ul << (n%FD_NFDBITS))			/* 특정 IP를 0으로 셋팅 */
#define _IPPOOL_ISSET(n, p)			(p->IPPOOL[n/FD_NFDBITS] & (1ul<<( n%FD_NFDBITS))) != 0l	/* 특정 IP가 1인지 확인 */


/*
* STRUCTURE : 512Mbytes
*/
typedef struct _st_IPPOOLLIST_
{
	unsigned int	IPPOOL[IPPOOL_SIZE/(IPPOOL_BASE*BPBYTE)];
} st_IPPOOLLIST, *pst_IPPOOLLIST;

#define DEF_IPPOOLBIT_SIZE		sizeof(st_IPPOOLLIST)


#endif
