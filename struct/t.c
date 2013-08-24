#include <stdio.h>

struct st_t{
	int a;
	char b;
};

int test1()
{
	struct st_t ab[3];
	struct st_t *pab;

	pab = &ab[0];

	pab->a = 1;
	pab->b = 'c';

	pab++;
	pab->a = 2;
	pab->b = 'd';

	pab++;

	pab->a = 3;
	pab->b = 'e';

	int i;
	printf("size st=%d\n", sizeof(struct st_t));
	printf("size ab=%d\n", sizeof(ab));
	for (i = 0; i < 3; i++) {
		printf("[%d]a=%d, b=%c\n", i, ab[i].a, ab[i].b);
	}

	return 0;
}

int test_copy()
{
	struct st_t a;
	struct st_t b;

	a.a = 1;
	a.b = 2;

	b = a;

	printf("a.a = %d\n", a.a);
	printf("a.b = %d\n", a.b);
	printf("b.a = %d\n", b.a);
	printf("b.b = %d\n", b.b);
	return 0;
}

int main()
{
	test1();
	test_copy();
}
