#include<stdio.h>

int main()
{
	int i=0;
	unsigned char a;

	for( i; i<8; i++ ){
		a |= ( 0x00000001 <<i ); 
		printf(" a |= 0x00000001 << %d :: 0x%x\n", i,a);
	}
}
