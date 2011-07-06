#include <stdio.h>
enum enLIST  { IDX=0,IDX2,IDX3,IDX4,IDX5,ID,ID2,ID4,IDX_COUNT };
#define COUNT_SIZE IDX_COUNT

int main()
{
	int i;
	int a=1;
	int j=COUNT_SIZE;
	for ( i=0;i< a;i++ ){
//		printf("test[%d]\n",i+1);
//		pritnf("enumsize = %d\n",COUNT_SIZE);
		printf("j=%d\n",j);
	}
	return 0;
}
