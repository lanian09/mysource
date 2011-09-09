#include <stdarg.h>
#include <sys/stat.h>

#include <tcp_gen.h>

/* remove by tundra 
long long convert_llong( long long value);
*/

time_t convert_time_t( time_t value )
{
    int i;
    time_t xValue;

    char tszValue[4];
    char tszValue2[4];

    memcpy( tszValue, &value, sizeof(time_t) );

    for( i=0; i< 4; i++)
    {
        tszValue2[i] = tszValue[3-i];
    }

    memcpy( &xValue, tszValue2, sizeof(time_t));

    return xValue;
}

unsigned long convert_ulong( unsigned long value )
{
    int i;
    unsigned long xValue;

    char tszValue[4];
    char tszValue2[4];

    memcpy( tszValue, &value, sizeof(unsigned long) );

    for( i=0; i< 4; i++)
    {
        tszValue2[i] = tszValue[3-i];
    }

    memcpy( &xValue, tszValue2, sizeof(unsigned long));

    return xValue;
}

int convert_int( int value )
{
    int i;
    int xValue;

    char tszValue[4];
    char tszValue2[4];

    memcpy( tszValue, &value, sizeof(int) );

    for( i=0; i< 4; i++)
    {
        tszValue2[i] = tszValue[3-i];
    }

    memcpy( &xValue, tszValue2, sizeof(int));

    return xValue;
}

long convert_long( long value )
{
    int i;
    long xValue;

    char tszValue[4];
    char tszValue2[4];

    memcpy( tszValue, &value, sizeof(long) );

    for( i=0; i< 4; i++)
    {
        tszValue2[i] = tszValue[3-i];
    }

    memcpy( &xValue, tszValue2, sizeof(long));

    return xValue;
}

unsigned short convert_ushort( unsigned short value )
{
    int i;
    unsigned short xValue;

    char tszValue[2];
    char tszValue2[2];

    memcpy( tszValue, &value, sizeof(unsigned short) );

    for( i=0; i< 2; i++)
    {
        tszValue2[i] = tszValue[1-i];
    }

    memcpy( &xValue, tszValue2, sizeof(unsigned short));

    return xValue;
}

short convert_short( short value )
{
    int i;
    short xValue;

    char tszValue[2];
    char tszValue2[2];

    memcpy( tszValue, &value, sizeof(short) );

    for( i=0; i< 2; i++)
    {
        tszValue2[i] = tszValue[1-i];
    }

    memcpy( &xValue, tszValue2, sizeof(short));

    return xValue;
}

/*
long long convert_llong( long long value , long long *retval)
*/
long long convert_llong( long long value)
{
    int i;
    long long xValue;

    char tszValue[8];
    char tszValue2[8];

    memcpy( tszValue, &value, sizeof(long long) );

    for( i=0; i< 8; i++)
    {
        tszValue2[i] = tszValue[7-i];
    }

    memcpy( &xValue, &tszValue2[0], sizeof(long long));

/*
	*retval = xValue;
*/

    return xValue;
}

