#include <stdio.h>
#include <string.h>

int main()
{
	char *szCmd[] = { "DIS-TREND-INFO", "DIS-DEFECT-INFO" };

	int i;
	int j = sizeof( szCmd) / sizeof(char *);

	if( !strncasecmp("DIS-TREND-INI", szCmd[0], strlen(szCmd[0]) ) ||
		!strncasecmp("DIS-TREND-INI", szCmd[0], strlen(szCmd[0]) ) ){
			printf("CAUGHT\n");
	}
	printf("string=%s(%d)\n", szCmd[1], strlen(szCmd[1]));
//	for(i=0;i<j;i++){
	//i=0;
	//while( szCmd[i] ){
/*
		printf("string=%s(%d)\n", szCmd[i], strlen(szCmd[i]));
		if( !strncasecmp( "DIS-TREND-INFO", szCmd[i], strlen(szCmd[i])) )
			printf("CAUGHT\n");
*/
	//	i++;
//	}

	return;
}
