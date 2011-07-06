#include <stdio.h>

typedef struct _ac{
	int a;
	int b[3];
} st_ac;

int main()
{
	st_ac sta;
	sta.a =1;
	sta.b[0] = 2;
	sta.b[1] = 3;
	sta.b[2] = 4;
	printf("sta.a=%d\n", sta.a);
	printf("sta.b[0]=%d\n", sta.b[0]);
	printf("sta.b[1]=%d\n", sta.b[1]);
	printf("sta.b[2]=%d\n", sta.b[2]);
	printf("sta.b[-1]=%d\n", sta.b[-1]);
	printf("sta.b[-2]=%d\n", sta.b[-2]);
	printf("sta.b[-3]=%d\n", sta.b[-3]);
	printf("sta.b[-4]=%d\n", sta.b[-4]);
	return 0;
}
