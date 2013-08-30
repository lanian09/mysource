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

struct st_a {
 int a;
 int b;
};

struct st_b {
	struct st_a sa[3];
	int a;
	int b;
};

int test2()
{
	struct st_a *psa;
	struct st_b sb;

	sb.a = 1;
	sb.b = 2;
	psa = sb.sa;

	psa[0].a = 3;
	psa[0].b = 4;
	psa[1].a = 5;
	psa[1].b = 6;
	psa[2].a = 7;
	psa[2].b = 8;

	printf("sb.a=%d\n", sb.a);
	printf("sb.b=%d\n", sb.b);
	printf("sb.sa[0].a=%d\n", sb.sa[0].a);
	printf("sb.sa[0].b=%d\n", sb.sa[0].b);
	printf("sb.sa[1].a=%d\n", sb.sa[1].a);
	printf("sb.sa[1].b=%d\n", sb.sa[1].b);
	printf("sb.sa[2].a=%d\n", sb.sa[2].a);
	printf("sb.sa[2].b=%d\n", sb.sa[2].b);
}

int print(int (*cb)())
{
	static test_cnt = 1;
	printf("=== test start(%d) ===\n", test_cnt++);
	cb();
	printf("=== test finished ===\n");
	return 0;
}

int main()
{
	print(test1);
	print(test_copy);
	print(test2);
}
