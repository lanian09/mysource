#include <stdio.h>

//int sum(int from, int to, int arr[])
int sum(int from, int to, int *arr)
{
	int i, dRet;
	for( dRet = 0, i = from; i < to; i++ ){
		//dRet += arr[i];
		dRet += *(arr+i);
	}
	return dRet;
}

int main(int ac, int **av)
{
	int from, to;
	int arr[10] = { 1,2,3,4,5,6,7,8,9,10 };

	if( ac != 3 ){
		printf("param needs\n");
		return -1;
	}
	
	from = atoi(av[1]);
	to   = atoi(av[2]);

	if( from < 0 || from > 10 ){
		printf("from value is must larger than 0, smller than 10, from=%d\n", from);
		return -2;
	}

	if( to < 0 || to > 10 ){
		printf("to value is must larger than 0, smller than 10, to=%d\n", to);
		return -3;
	}

	printf("from=%d, to=%d, sum=%d\n", from, to, sum(from,to,&arr[0]));
	return 0;

}
