#include <stdio.h>

int main()
{
	char *c,buf[] = {"test"};
	int  len;
	printf("buf=%s\n", buf);
	c = &buf[0];
	len = 4;
	while(len--){
		printf("%x:",*c++);
	}
	printf("\n");
	
	return 0;
}
