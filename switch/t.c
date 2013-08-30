#include <stdio.h>

void test1()
{
	int a = 1;

	switch(a) {
		case 1:
			if (a > 0) {
				printf("not test1\n");
				break;
			}
			printf("test1\n");
			break;
		case 2:
			printf("test2\n");
			break;
		default:
			printf("test-default\n");
			break;
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
	return 0;
}
