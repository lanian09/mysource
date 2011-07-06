#include <stdio.h>
#include <time.h>

int main()
{
	time_t ctime;
	printf("[1]ctime=%d\n",ctime);
	ctime = time(NULL);
	printf("[2]ctime=%d\n",ctime);
	return 0;
}
