#include <stdio.h>

typedef struct _st_test{
	char ca;
	char cReserved;
	short int sia;
	int		ia;
	char szStr[20];
	char cb;
}st_test,*pst_test;

void _print_(st_test *p)
{
	printf("a.ca    = %d\n",p->ca);
	printf("a.sia   = %d\n",p->sia);
	printf("a.ia    = %d\n",p->ia);
	printf("a.szStr = %s\n",p->szStr);
	printf("a.cb    = %d\n",p->cb);
	printf("_print_finished_______\n");
}

int main()
{
	st_test a,b;
	memset(&a,0x00,sizeof(st_test));
	memset(&b,0x00,sizeof(st_test));

	a.ca = 1;
	a.sia = 23456;
	a.ia = 70605040;
	strcpy(a.szStr,"_TEST_LINE_");
	a.cb = 22;
	_print_(&a);
	_print_(&b);
	b=a;
	b.ca++;
	b.ia++;
	b.cb++;
	_print_(&a);
	_print_(&b);
	
	return 0;
}

