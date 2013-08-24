#include <stdio.h>

void sub1()
{
	static int a = 0;
	printf("a=%d\n", a++);
	printf("testline\n");
}

int test1()
{
	sub1();
	sub1();
	sub1();
	sub1();
	return(1);
}

int main()
{
	test1();
}
