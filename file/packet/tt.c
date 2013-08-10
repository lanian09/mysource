#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void write_ctx(char *fn, char *str, int len)
{
	int fd;
	ssize_t ss;

	fd = open(fn, O_CREAT|O_RDWR);
	if( fd < 0 ) { perror("failed open"); return; }

	ss = write(fd, str, len);
	if( ss < 0 ) { perror("failed write"); return; }

	close(fd);


}

int main()
{
	char *fn = "test_file.txt";
	char *s1 = "test=1234";
	char *s2 = "etst2=3234";
	write_ctx(fn, s1, sizeof(*s1));
	write_ctx(fn, s2, sizeof(*s2));
}
