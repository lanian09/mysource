#include <stdio.h>
#include <string.h>

int main()
{
	char arr[10];

	unsigned int *a, *b;
	unsigned short *c;
	int i;

	memset(arr, 0, 10);

	a = (unsigned int *)&arr[0];
	b = (unsigned int *)&arr[4];
	c = (unsigned short *)&arr[8];

	*a = 12399809;
	*b = 99388491;
	*c = 28392;

	for (i = 0; i < sizeof(arr); i++) {
		printf("a[%d]=0x%02x\n", i, *(unsigned char *)&arr[i]);
	}


}
