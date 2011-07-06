#include <stdio.h>
#include <time.h>

int main()
{
	time_t now;
	unsigned int tt;
	time(&now);
	printf("now=%d\n", now);
	tt = now;
	printf("tt =%u\n", tt);
	printf("now=%ld\n", now - 100);
}
