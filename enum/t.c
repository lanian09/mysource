#include <stdio.h>

#if 0
enum {
	AA = 0x01,
	BB,
	CC,
	DD=0x0a,
	EE = DD,
	FF	
} _ETYPE_CODE;
#endif
#define AA 0x01
#define BB 2
#define CC 3
#define DD 0x0a
#define EE DD
#define FF EE+1

int main()
{
	int i;
	for( i=0; i< FF; i++ ){
		printf("count=%d\n", i);
	}	
	printf("AA=%d\n",AA);
	printf("BB=%d\n",BB);
	printf("CC=%d\n",CC);
	printf("DD=%d\n",DD);
	printf("EE=%d\n",EE);
	printf("FF=%d\n",FF);
}

