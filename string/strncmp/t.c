#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


int main()
{
	char *tt="IFSWAP";
	char ntt[100];
	int i=0, len=0;

	len=strlen(tt);

	for( i=0 ; i < len ; i++ )
		ntt[i]= tolower(tt[i]);

	ntt[len]='\0';

	if( !strncmp(ntt,"ifswap",6 )) {
		printf("catch!\n");
	}else{
		printf("non-catch!\n");
	}
	return 0;
}
