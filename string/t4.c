#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void make_str(char *ptr)
{
	sprintf(ptr, "test1");
}

int main()
{
	char str[128];
	int  i;
	char *ptr;
	
	sprintf(str, "SELECT 1234 DEST='00FFFF00', CMD='TEST'", i);
	if ((ptr = strstr(str, "DEST='00FFFF"))) {
		ptr += strlen("DEST='00FFFF") +1;
	}

	printf("oritinal str=%s\n", str);
	for (i = 0; i < 4; i++) {
		printf("[%d]%s\n", i, str);
		*ptr = '0' +i+1;
	}

	make_str(str);
	printf("str=%s\n", str);
	return;
}
