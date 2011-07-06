#include <stdio.h>

int main()
{
	printf("\n  PATTERN                          TTL\n");
	printf("\n  %-32s %-3u\n", "Android", 32);
	printf("\n  %-32s %-3u\n", "12345678901234567890123456789012", 32);
	printf("\n  %-32s %3u\n", "Android", 32);
	printf("\n  %-32s %3u\n", "12345678901234567890123456789012", 32);
	
	return 0;
}
