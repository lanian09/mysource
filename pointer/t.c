#include <stdio.h>
#include <string.h>

typedef struct _st_new{
	int idx;
	int buffer[3];
}st_new,*pst_new;

void test1()
{
	int i,dSize;
	st_new stnew[3];
	pst_new pstnew;
	dSize = sizeof(st_new);
	stnew[0].idx=0;
	stnew[1].idx=2;
	stnew[2].idx=4;

	printf("real ----->\n");
	for(i=0;i<3;i++){
		printf("idx[%d]=%d\n",i,stnew[i].idx);
	}
	pstnew = &stnew[0];
	printf("ptr ----->\n");
	for(i=0;i<3;i++){
		printf("idx[%d]=%d[%u]\n",i,pstnew->idx,pstnew);
		pstnew++;
	}
}

void test2()
{
	char arr[10];

	unsigned int *a, *b;
	unsigned short *c;
	int i;

	memset(arr, 0, 10);

	a = (unsigned int *)&arr[0];
	b = (unsigned int *)&arr[4];
	c = (unsigned short *)&arr[8];

	*a = 12399809;
	*b = 99388491;
	*c = 28392;

	for (i = 0; i < sizeof(arr); i++) {
		printf("a[%d]=0x%02x\n", i, *(unsigned char *)&arr[i]);
	}


}

void test3()
{
	char *c,buf[] = {"test"};
	int  len;
	printf("buf=%s\n", buf);
	c = &buf[0];
	len = 4;
	while(len--){
		printf("%x:",*c++);
	}
	printf("\n");
}

int print(void (*cb)())
{
	static test_cnt = 1;
	printf("=== test start(%d) ===\n", test_cnt++);
	cb();
	printf("=== test finished ===\n");
	return 0;
}

int main()
{
	print(test1);
	print(test2);
	print(test3);
	return 0;
}
