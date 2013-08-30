#include <stdio.h>

struct stt{
	int a;
	char sub[2];
};

int pout(int aa, int ab)
{
	struct stt a;

	a.a = aa;
	a.sub[0] = ((char *)ab)[0];
	a.sub[1] = ((char *)ab)[1];

	printf("a=%d\n", a.a);
	printf("a.sub[0]=%d\n", a.sub[0]);
	printf("a.sub[1]=%d\n", a.sub[1]);
}

int test_assign(int a, int b, int c)
{
	char sub[2];

	sub[0] = (char)b;
	sub[1] = (char)c;

	pout(a,(int)sub);
	return(0);
}

int main()
{
	test_assign(1,2,3);
	return;
}

