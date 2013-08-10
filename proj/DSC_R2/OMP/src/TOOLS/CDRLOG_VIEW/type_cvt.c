/**********************************************************
                 KTF IPAS Project

   Author   : Park Si Woo
   Section  : Infravalley Develpment
   SCCS ID  : @(#)type_cvt.c	1.1
   Date     : 8/11/03
   Revision History :
   Description:

   Copyright (c) Infravalley 2003
***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#include <ipaf_svc.h>

int CVT_INT( int value );
short CVT_SHORT( short value );
long long CVT_INT64_CP( long long *retVal, long long input_val );

char *cvt_ipaddr(unsigned int uiIP)
{
    struct in_addr  inaddr;

    inaddr.s_addr = uiIP ;

    return inet_ntoa(inaddr);
}

short CVT_SHORT( short value )
{
	union {
		short xValue;
		char ml[2];
	} u1, u2;

	u1.xValue = value;

#ifndef __SUN__
	u2.ml[0] = u1.ml[1];		
	u2.ml[1] = u1.ml[0];		

    return u2.xValue;
#endif
	return u1.xValue;
}

int CVT_INT( int value )
{
	union {
		int xValue;
		char ml[4];
	} u1, u2;

	u1.xValue = value;

#ifndef __SUN__
	u2.ml[0] = u1.ml[3];		
	u2.ml[1] = u1.ml[2];		
	u2.ml[2] = u1.ml[1];		
	u2.ml[3] = u1.ml[0];		
		
    return u2.xValue;
#endif
	return u1.xValue;
}

long long CVT_INT64_CP( long long *ret_val, long long input_value )
{
	union {
		long long xValue;
		char ml[8];
	} u1, u2;

	u1.xValue = input_value;

#ifdef __SUN__
	u2.ml[0] = u1.ml[7];		
	u2.ml[1] = u1.ml[6];		
	u2.ml[2] = u1.ml[5];		
	u2.ml[3] = u1.ml[4];		
	u2.ml[4] = u1.ml[3];		
	u2.ml[5] = u1.ml[2];		
	u2.ml[6] = u1.ml[1];		
	u2.ml[7] = u1.ml[0];		
    
    *ret_val = u2.xValue;
    return u2.xValue;
#endif
    *ret_val = u1.xValue;
	return u1.xValue;
}

short CVT_SHORT_CP( short value )
{
	union {
		short xValue;
		char ml[2];
	} u1, u2;

	u1.xValue = value;

#ifdef __SUN__
	u2.ml[0] = u1.ml[1];		
	u2.ml[1] = u1.ml[0];		

    return u2.xValue;
#endif
	return u1.xValue;
}

int CVT_INT_CP( int value )
{
	union {
		int xValue;
		char ml[4];
	} u1, u2;

	u1.xValue = value;

#ifdef __SUN__
	u2.ml[0] = u1.ml[3];		
	u2.ml[1] = u1.ml[2];		
	u2.ml[2] = u1.ml[1];		
	u2.ml[3] = u1.ml[0];		
		
    return u2.xValue;
#endif
	return u1.xValue;
}


int CVT_INT_CP_CDR( int value , int *res)
{
	union {
		int xValue;
		char ml[4];
	} u1, u2;

	u1.xValue = value;

#ifdef __SUN__
	u2.ml[0] = u1.ml[3];		
	u2.ml[1] = u1.ml[2];		
	u2.ml[2] = u1.ml[1];		
	u2.ml[3] = u1.ml[0];		
		
	*res = u2.xValue;
    return u2.xValue;
#endif
	*res = u2.xValue;
	return u1.xValue;
}


/*
long long CVT_INT64_CP( long long value )
{
	union {
		long long xValue;
		char ml[8];
	} u1, u2;

	u1.xValue = value;

#ifdef __SUN__
	u2.ml[0] = u1.ml[7];		
	u2.ml[1] = u1.ml[6];		
	u2.ml[2] = u1.ml[5];		
	u2.ml[3] = u1.ml[4];		
	u2.ml[4] = u1.ml[3];		
	u2.ml[5] = u1.ml[2];		
	u2.ml[6] = u1.ml[1];		
	u2.ml[7] = u1.ml[0];		

    return u2.xValue;
#endif
	return u1.xValue;
}
*/


    
INT CVT_INT_2( INT value, INT *Ret )
{
    
    int i; 
    int dValSize = 4;

    union {
        INT xValue; 
        char ml[4];
    } u1, u2;
    
    u1.xValue = value;
    
    u2.ml[0] = u1.ml[3];
    u2.ml[1] = u1.ml[2];
    u2.ml[2] = u1.ml[1];
    u2.ml[3] = u1.ml[0];
        
    *Ret =  u2.xValue;
    
}


void CvtBinToHexa(int dLen, unsigned char *szSrc, unsigned char *szOut)
{
	int      i;
	int      dOffset = 0;

   	for(i=0; i<dLen; i++)
  	{
		sprintf((char*)&szOut[dOffset], "%02X", szSrc[i]);
		dOffset += 2;
	}

	szOut[dOffset] = 0x00;
	return;
}


void conv_id(INT64 lltmp, char *str)
{

	INT64   tmp1, tmp2, bitmask;
	int i;

	for (i=0; i<8; i++)
	{
		tmp1 = lltmp >> i*8;
		bitmask = 0x000000ff;
		tmp2 = (tmp1 & bitmask);
		str[8-i-1] = (char) tmp2;
	}
	str[8] = 0;
}
