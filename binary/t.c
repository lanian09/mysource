#include <stdio.h>

#define POW(n) (1<<n)

int getSysServices(int v)
{
	int r = v;
	int n;
	int t = 1;

	printf("insert : %d\n", v);
	while (r) {

		/* search */
		for (n = 0; r >= POW(n); n++);

		r -= POW(n-1);

		printf("r%d = %d(%d), remained=%d\n", t++, n, POW(n), r);
	}
}

void test1()
{
	getSysServices(72);
	getSysServices(78);
}

void test2()
{
	int i;

	for (i = 0; i < 3; i++) {
		printf("No%02d = %d\n", i, POW(i));
	}
}

int print(void (*cb)())
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
	//print(test2);
	return 0;
}
