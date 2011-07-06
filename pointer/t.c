#include <stdio.h>
typedef struct _st_new{
	int idx;
	int buffer[3];
}st_new,*pst_new;

int main()
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

	return 0;
}
