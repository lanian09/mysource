#include <stdio.h>

int main()
{
	char a = 0x37;
	printf("a.ver=%d.%d\n", a>>4, a&0x0f);
}
