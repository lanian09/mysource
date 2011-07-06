#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
	pid_t pp;
	pp = getpid();
	printf("pp = %u, size=%d\n", pp, sizeof(pp));
	return 0;
}
