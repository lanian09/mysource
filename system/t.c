#include <stdio.h>
#include <string.h>
#include <errno.h>

void lo_proc()
{
	int  ret;
	char cmd[100];

	strcpy(cmd,"ls -al > TEST.OUT");
	ret = system(cmd);

	printf( "ret=%d, errno=%d:%s\n", ret, errno, strerror(errno));
	
	return;
}

int main()
{
	lo_proc();
	return 0;
}
