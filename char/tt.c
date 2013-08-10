#include <stdio.h>
#include <stdlib.h>

char * _2i_step01(int v)
{
	char *buffer;
	buffer = malloc(128);
	sprintf(buffer,"%d.%d", v/10, v%10);
	return buffer;
}

char * _2i_step05(int v)
{
	char *buffer;

	buffer = malloc(128);
	sprintf(buffer, "%d.%d", v/2, (v%2)*5);
	return buffer;
}

int main()
{
	char a = 31;
	printf("a(%d)*0.1=%f\n", a,a*0.1);
	printf("a*0.1=%d.%d\n", a/10, a%10);
	printf("a*0.5=%d.%d\n", a/2, (a%2)*5);
	printf("a2*0.1=%s\n", _2i_step01((int)a));
	printf("a2*0.5=%s\n", _2i_step05((int)a));
}
