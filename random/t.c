#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
int main(int ac, char **av)
{
	int a = 0;
	int rand;
	int val;
	struct timeval stime;
	int i,j,k,key,key12,key13;
	char get[2][30];

	if( ac < 2 ){
		printf("[ERROR]Paramcount is too small : %d\n",ac );
		return -1;
	}
	memset(get,0x00,sizeof(char)*2*30);

	a = atoi(av[1]);
	printf("Count : %d\n",a);
	if( a > 30 ){
		printf("[ERROR]Parameter value is too big : %d/30\n",a);
		return -1;
	}
	j=k=0;
	while( 1 ){
		
		for( i=0,j=0,k=0,rand=0; i< a; i++ ){
			gettimeofday(&stime,NULL);
			srandom((unsigned int)stime.tv_usec);
			rand = random();
			val = rand%2;
			if( val != 1 ){
				get[0][j++]=i+1;
				if( i+1 == 4 ){
					key = 0;
				}
				if( i+1 == 12 ){
					key12 = 0;
				}
				if( i+1 == 13 ){
					key13 = 0;
				}
			}
			else{
				get[1][k++]=i+1;
				if( i+1 == 4 ){
					key = 1;
				}
				if( i+1 == 12 ){
					key12 = 1;
				}
				if( i+1 == 13 ){
					key13 = 1;
				}
			}

			
		}

		if( (( j-k == 1) && (key == 0))
		|| ((k-j == 1) && (key == 1))
		&& (key12 != key13) ){
				break;	
		}
	}
	

	printf("print j integer set\n");
	for( i=0;i<j;i++ ){
		printf("%d ",get[0][i]);
	}
	printf("\n");

	printf("print k integer set\n");
	for( i=0;i<k;i++ ){
		printf("%d ",get[1][i]);
	}
	printf("\n");
	
	return 0;
}
