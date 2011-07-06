#include <stdio.h>
int main()
{
	int dLoop = 9,i;
	for( i=0; i<dLoop; i++ ){

		if( i == (dLoop -1) )
			printf("1ST-END\n");
		else
			printf("1ST-COUNT=%d\n",i+1);

		if( i == dLoop -1 )
			printf("2ND-END\n");
		else
			printf("2ND-COUNT=%d\n",i+1);
		
	}
}
