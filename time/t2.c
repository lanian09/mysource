#include <stdio.h>
#include <time.h>

void convert_time(time_t twhen)
{
	struct tm *tm;
	tm = localtime(&twhen);
	printf("ctime=%02d/%02d/%02d %02d:%02d:%02d\n",
			tm->tm_year+1900,tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    
}

int main(int ac, char **av)
{
	convert_time(atoi(av[1]));
	return 0;
}
