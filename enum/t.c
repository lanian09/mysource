#include <stdio.h>

enum {
	AA = 0x01,
	BB, CC, DD=0x0a, EE,
	FF	
} _ETYPE_CODE;

int main()
{
	int i;
	for( i=0; i< FF; i++ ){
		printf("count=%d\n", i);
	}	
}
