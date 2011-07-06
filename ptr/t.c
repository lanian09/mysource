#include <stdio.h>

int main()
{
	unsigned int a;
	unsigned int *b;
	a=10;
	b=&a;
	printf("1.value(a)=%ld, ptr-value(b)=%ld\n",a,*b);
	a=5;
	printf("2.value(a)=%ld, ptr-value(b)=%ld\n",a,*b);
	
}
