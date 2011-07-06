#include <stdio.h>
#include <string.h>

int main()
{
	char buf[10];
	
	memset(buf, 0x00, 10);

	if( buf == NULL )
		printf("buf[0] isNull1\n");
	
	if( buf )
		printf("buf[0] isNull2\n");
}
