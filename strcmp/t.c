#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int dCmpExCmd(char *a)
{
	char *cmd[] = { "ADD-TEST-FLT", "ADD-FLT-SCTP" };
	int  i,len = sizeof(cmd)/sizeof(char *);
	
	printf("len=%d\n",len);
	for(i=0; i<len; i++)
		if( !strcmp( a, cmd[i] ) )
			return 1;
	return 0;
}

int main()
{
	char buf[100];
	char *a;
	
	memset(buf,0x00, 100);
	sprintf(buf,"ADD-FLT-SCTP");
	printf("buf=%s]\n", buf);
	a = &buf[0];
	if( dCmpExCmd(a) )
		printf("CAUGHT~~~\n");
}
