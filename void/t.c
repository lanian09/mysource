#include <stdio.h>

int main()
{
	void *a;
	int da = 1;

	a = (void *)&da;

	printf("da=%d\n", da);
	printf("a=%d\n", *(int *)a);


}
