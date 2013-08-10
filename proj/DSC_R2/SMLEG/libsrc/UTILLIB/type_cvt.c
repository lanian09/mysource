
/**********************************************************
                 KTF IPAS Project

   Author   : Park Si Woo
   Section  : Infravalley Develpment
   SCCS ID  : @(#)type_cvt.c	1.1
   Date     : 8/11/03
   Revision History :
    '03.  1. 6  Revised by Siwoo Park


   Description:

   Copyright (c) Infravalley 2003
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ipaf_svc.h>

UINT 	CVT_UINT(UINT );
USHORT 	CVT_USHORT(USHORT );
INT64 	CVT_INT64(INT64 );
INT CVT_INT( INT value );
char *CVT_ipaddr(UINT uiIP);

USHORT CVT_USHORT( USHORT value )
{
	union {
		USHORT xValue;
		char ml[2];
	} u1, u2;

	u1.xValue = value;

	u2.ml[0] = u1.ml[1];		
	u2.ml[1] = u1.ml[0];		

    return u2.xValue;
}

INT CVT_INT( INT value )
{
    union {
        INT xValue;
        char ml[4];
    } u1, u2;

    u1.xValue = value;

    u2.ml[0] = u1.ml[3];
    u2.ml[1] = u1.ml[2];
    u2.ml[2] = u1.ml[1];
    u2.ml[3] = u1.ml[0];

    return u2.xValue;
}

UINT CVT_UINT( UINT value )
{
	union {
		UINT xValue;
		char ml[4];
	} u1, u2;

	u1.xValue = value;

	u2.ml[0] = u1.ml[3];		
	u2.ml[1] = u1.ml[2];		
	u2.ml[2] = u1.ml[1];		
	u2.ml[3] = u1.ml[0];		
		
    return u2.xValue;
}

INT64 CVT_INT64( INT64 value )
{
	union {
		INT64 xValue;
		char ml[8];
	} u1, u2;

	u1.xValue = value;

	u2.ml[0] = u1.ml[7];		
	u2.ml[1] = u1.ml[6];		
	u2.ml[2] = u1.ml[5];		
	u2.ml[3] = u1.ml[4];		
	u2.ml[4] = u1.ml[3];		
	u2.ml[5] = u1.ml[2];		
	u2.ml[6] = u1.ml[1];		
	u2.ml[7] = u1.ml[0];		

    return u2.xValue;
}

CHAR *CVT_INT2STR_IP(UINT uiIP)
{
	struct in_addr  inaddr;

	inaddr.s_addr = (uiIP);

	return (char *)inet_ntoa(inaddr);
}

CHAR *CVT_INT2STR_NIP(UINT uiIP)
{
	struct in_addr  inaddr;

	inaddr.s_addr = htonl(uiIP);

	return (char *)inet_ntoa(inaddr);
}

