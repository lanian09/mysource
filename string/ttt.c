#include <stdio.h>
#include <string.h>

int main()
{
	char szBuf[10];
	sprintf(szBuf, "hello");
	printf("init=%s(%d)\n", szBuf,strlen(szBuf));
	szBuf[0] = 0x00;
	printf("2nd =%s(%d)\n", szBuf,strlen(szBuf));
	return 0;
}
