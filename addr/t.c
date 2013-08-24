#include <stdio.h>

struct st_temp_key {
	int a;
	int b;
};
struct st_temp1 {
	int a;
	int b;

	int c;
};
struct st_temp2 {
	int a;
	int b;

	char c;
};

int main()
{
	struct st_temp1 a;
	struct st_temp_key *pa;
	a.a = 1;
	a.b = 2;
	a.c = 3;

	printf(" a    =0x%x\n", a);
	printf(" &a   =0x%x\n", &a);
	printf(" &a.a =0x%x\n", &a.a);
	printf(" a.a  =%d\n", a.a);
	printf(" a.b  =%d\n", a.b);
	printf(" a.c  =%d\n", a.c);

	pa = (struct st_temp_key *)&a;
	printf(" pa->a=%d\n", pa->a);
	printf(" pa->b=%d\n", pa->b);
	return 0;
}
