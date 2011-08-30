#include <stdio.h>

#define MASK  0x80
#define UMASK 0x00

int main()
{
	unsigned char a =  0x01;

	printf("val = 0x%x:%d\n", a,a);
	a |= MASK;
	printf("masked val = 0x%x:%d\n", a,a);
	a ^= MASK;
	printf("umasked val = 0x%x:%d\n", a,a);
}
