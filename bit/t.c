#include <stdio.h>

#define AIS_NORMAL(a,b) ((a >> (8-(b*2))) & 0x01)
#define AIS_ALARM(a,b)  ((a >> (8-(b*2))) & 0x02)

#define IS_ACTIVE(a) (a & 0x03)

int main()
{
	char a = 0x40;
	char b = 0x30;
	char c = 0x0c;
	char d = 0x03;

	if( (a >> 6) & 0x01 ) printf("a normal\n");
	if( (a >> 6) & 0x02 ) printf("a alarm\n");
	if( IS_ACTIVE(0x02) ) printf("S normal1\n");
	if( IS_ACTIVE(0x01) ) printf("S normal2\n");
	if( (a >> 6) & 0x02 ) printf("a alarm\n");
	if( (b >> 4) & 0x01 ) printf("b normal\n");
	if( (b >> 4) & 0x02 ) printf("b alarm\n");
	if( (c >> 2) & 0x01 ) printf("c normal\n");
	if( (c >> 2) & 0x02 ) printf("c alarm\n");
	if( (d >> 0) & 0x01 ) printf("d normal\n");
	if( (d >> 0) & 0x02 ) printf("d alarm\n");
	return 0;
}
