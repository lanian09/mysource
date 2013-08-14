#include <stdio.h>
#include <string.h>

int cat(char *str, char *src, char *dest, int srclen, int destlen)
{
	sprintf(&src[srclen], str);
	sprintf(&dest[destlen], str);
}

int main()
{
	char src[64];
	char dest[64];
	char *test = "test";

	sprintf(src, "testSRC:");
	sprintf(dest, "testDEST:");

	printf("src=%s\n", src);
	printf("dest=%s\n", dest);

	cat(test, src, dest, strlen(src), strlen(dest));

	printf("src=%s\n", src);
	printf("dest=%s\n", dest);
}
