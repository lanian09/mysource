#include <stdio.h>
#include <string.h>

char testName[2][30];
int  testLen[2];

int get_test(char *devName)
{
	if( !strncasecmp(devName, testName[0], testLen[0]) || !strncasecmp(devName, testName[1], testLen[1]) ){
		if( !strncasecmp(devName, testName[0], testLen[0]) ){
			printf("test case1\n");
		}
		if( !strncasecmp(devName, testName[1], testLen[1]) ){
			printf("test case2\n");
		}
		return 3;
	}
	return 19;
	
}

int main()
{
	int  rst = 0;
	char tmp[30];
	
	sprintf(tmp, "SMCON");

	sprintf(testName[0],"e1000");
	testLen[0] = strlen(testName[0]);

	printf("size empty string? %d\n", strlen(testName[1]));

	rst = get_test(tmp);
	printf("%s=%d\n", tmp, rst);
	return;
	
}
