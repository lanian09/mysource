#include <stdio.h>

int main()
{
	int i=1, y=0;

	if( i ) printf("ONE is true\n");
	else 	printf("ONE is false\n");

	if( !i ) printf("!ONE is true\n");
	else 	printf("!ONE is false\n");

	if( y ) printf("ZERO is true\n");
	else 	printf("ZERO is false\n");

	if( !y ) printf("!ZERO is true\n");
	else 	printf("!ZERO is false\n");
}
