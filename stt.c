#include <stdio.h>
int main()
{
	char    szLoad[6];
	unsigned long currload;
	sprintf(szLoad,"%6.2f",(((double)761636)*100.0)/((double)4147752));

	printf("LoadValue=%s\n",szLoad);
	currload = atol(szLoad);
	printf("after AtoL LoadValue=%u\n",currload);
	
	return 0;
}
