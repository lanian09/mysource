#include <stdio.h>

struct st_t{
	int a;
	char b;
};

int main()
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
