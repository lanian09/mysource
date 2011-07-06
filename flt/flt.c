#include <stdio.h>

int main()
{
	unsigned int t,i,ipcnt,ip[500], cnt[500];
	char buf[20];

	for(i=0;i<500;i++){
		ip[i]=cnt[i]=0;
	}

	ipcnt=0;
	while(scanf("%u", &t) != EOF){
		if( ipcnt == 0 ){
			ip[0] = t;
			cnt[0]++;
			ipcnt++;
		}else{
			for( i = 0; i<ipcnt; i++){
				if( ip[i] == t ){
					/* exist */
					cnt[i]++;
					break;
				}
			}
			if( i == ipcnt ){
				/* new */
				ip[i] = t;
				cnt[i]++;
				ipcnt++;
			}
		}
	}

	for(i=0;i<ipcnt;i++){
		if( cnt[i] != 3 )
		printf("%03d\t%u\tcnt=%d \n",i,ip[i], cnt[i]); 
	}
	printf("-----------------------------\n");
	printf("*** PRINT END ** TOTAL CNT=%d\n", ipcnt);
}
