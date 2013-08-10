#include <stdio.h>

int main()
{
	short int sia = 0x927e;
	printf("0x927e\n");
	printf("hs=%d\n", sia);
	printf("hs/256=%f\n", (float)sia/256);
	
	printf("0x9283\n");
	sia = 0x9283;
	printf("hs=%d\n", sia);
	printf("hs/256=%f\n", (float)sia/256);
}
