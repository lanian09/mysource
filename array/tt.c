#include <stdio.h>
#include <string.h>

typedef struct _st_msgq {
	int key;
	char name[12]; 
} st_msgq, *pst_msgq;

int getQueList(st_msgq *arr)
{
	int cnt = 0, dummy = 1003;
	pst_msgq pmsgq;
	
	while( cnt < 4 ){
		pmsgq = arr++;
		pmsgq->key = dummy;
		sprintf(pmsgq->name,"Q%d", dummy++); 
		cnt++;
	}

	return cnt;

}

int main()
{
	st_msgq msgq[128];
	int     cnt;

	memset(msgq, 0x00, 128);

	cnt = getQueList(msgq);
	
	while(cnt--){
		printf("%d. %s=%d\n", cnt, msgq[cnt].name, msgq[cnt].key);
	}
	return 0;
}
