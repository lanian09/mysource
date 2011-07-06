#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main()
{
	char szTmp[30];
	strcpy(szTmp,"TEST");
	printf("szTmp=%s\n",szTmp);
	strcpy(&szTmp[strlen(szTmp)],"_TEST2");
	printf("szTmp2=%s\n",szTmp);
	return 1;
}
