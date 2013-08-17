#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *make_str(char **str)
{
	*str = malloc(128);
	sprintf(*str, "SELECT TEST.1023 WHERE DEST='00FFFF00', AND UP TO HERE");
}

int main()
{
	char *dummy;
	char *str;
	char *ptr;
	int   i;

	dummy = "DEST='00FFFF";

	make_str(&str);
	printf("str=%s\n", str);
	ptr = strstr(str, dummy);
	if (!ptr) {
		perror("ptr is null\n");
		exit(-1);
	}

	printf("ptr=%x, dummy=%s, len=%d\n", ptr, dummy, strlen(dummy));
	ptr += strlen(dummy)+1;
	printf("ptr=%x, dummy=%s, len=%d\n", ptr, dummy, strlen(dummy));

	for (i = 0; i < 3; i++) {
		printf("str=%s]\n", str);
		*ptr = '0' +(i+1);
	}

	
	free(str);
	return;
}
