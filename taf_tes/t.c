#include <stdio.h>

#define TESTL "TESTLEN"

int main()
{
	char ccc[10];

	memset(ccc,0x00,10);
	sprintf(ccc,"%s","TESSSS");
	printf("ccc=[%s]\n",ccc);
}
