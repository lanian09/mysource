#include <stdio.h>

#define SUNIT 100
#define MAX_TYPE 2

int get_id(int id)
{
	int ret;
	ret = id/SUNIT;
	if( !ret ){
		return -1;
	}
	ret = get_id(ret);
	return !(id/SUNIT)?-1:get_id(id/SUNIT);
}

int main()
{
	unsigned char gtam=0,tam=0,taf=0;
	int  id = 120492;

	gtam = get_id(id);
	tam  = get_id(gtam);
	taf  = UNIT;
	printf("id=%d\n", id);
	printf("gtam=%d\n", gtam);
	printf("gtam=%d\n", tam);
	printf("gtam=%d\n", taf);
}
