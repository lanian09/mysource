#include <stdio.h>

void util_cvtupperz(char *pdest, char *psrc)
{
	while( *psrc ){
		if( *psrc && *psrc >= 'a' && *psrc <= 'z' ){
			*pdest = toupper(*psrc);
		}else{
			*pdest = *psrc;
		}
		pdest++;
		psrc++;
	}
	*pdest = 0x00;
}

int main()
{
	char buf[] = "C a function to uppercase a String";
	char buf2[256];

	printf("buf=%s\n", buf);
	util_cvtupperz(buf2, buf);
	printf("buf2=%s\n", buf2);
	return 0;
}
