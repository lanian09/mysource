#include <stdio.h>

void test_1()
{
	unsigned int a;
	unsigned int *b;
	a=10;
	b=&a;
	printf("1.value(a)=%ld, ptr-value(b)=%ld\n",a,*b);
	a=5;
	printf("2.value(a)=%ld, ptr-value(b)=%ld\n",a,*b);
	
}

struct st1 {
	int a;
	char *nid;
};

void test_st2(char **pnid2)
{
	printf("pnid2=%x\n", *pnid2);
}

void test_st1(char **pnid)
{
	printf("pnid=%x\n", *pnid);
	test_st2(pnid);
}
void test_st()
{
	struct st1 a;
	struct st1 *pa;

	pa = &a;

	printf("a = %x\n", &a);
	printf("a = %x\n", a);
	printf("a.nid = %x\n", a.nid);
	printf("&a.nid = %x\n", &a.nid);
	test_st1(&a.nid);
	test_st1(&pa->nid);
}

int main()
{
	//test_1();
	test_st();
}
