
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <radius_define.h>

#include <errno.h>


ULONG CVT_ULONG( ULONG value );
LONG CVT_LONG( LONG value );
INT CVT_INT( INT value );
UINT CVT_UINT( UINT value );
SHORT CVT_SHORT( SHORT value );
USHORT CVT_USHORT( USHORT value );
INT64 CVT_INT64( INT64 value );
//char *cvt_ipaddr(UINT uiIP);


SHORT CVT_SHORT( SHORT value )
{

    int i;
    int dValSize=2;

	union {
		INT xValue;
		char ml[2];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}

    return u2.xValue;
}

USHORT CVT_USHORT( USHORT value )
{
    int i;
    int dValSize=2;

	union {
		INT xValue;
		char ml[2];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}


    return u2.xValue;
}


INT CVT_INT( INT value )
{
	int i;
	int dValSize = 4;

	union {
		INT xValue;
		char ml[4];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}
		
    return u2.xValue;
}


UINT CVT_UINT( UINT value )
{
	int i;
	int dValSize = 4;

	union {
		UINT xValue;
		char ml[4];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}

    return u2.xValue;
}

LONG CVT_LONG( LONG value )
{
	int i;
	int dValSize = 8;

	union {
		UINT xValue;
		char ml[8];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}

    return u2.xValue;
}

ULONG CVT_ULONG( ULONG value )
{
	int i;
	int dValSize = 8;

	union {
		ULONG xValue;
		char ml[8];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}

    return u2.xValue;
}

INT64 CVT_INT64( INT64 value )
{
	int i;
	int dValSize = 8;

	union {
		INT64 xValue;
		char ml[8];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}

    return u2.xValue;
}


/*
char *cvt_ipaddr(UINT uiIP)
{
    struct in_addr inaddr;
         
    inaddr.s_addr = uiIP;

    return inet_ntoa(inaddr);

}
*/

