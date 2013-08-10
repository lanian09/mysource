#include <stdio.h>
#include <string.h>
#include <unistd.h>

int file_exists(const char * filename)
{
	FILE *file = NULL;
	file = fopen(filename, "r");
	if (file) {
		fclose(file);
		return 1;
	}
	return 0;
}

void write_ctx(char *fn, char *str, int len)
{
	FILE *fd = NULL;
	size_t ss;

	int exi = file_exists(fn);

	printf("file_exist=%s\n", exi?"exist":"NOT exist" );
	fd = fopen(fn, "a");
	if( !fd ) { perror("failed open"); return; }

	ss = fwrite(str, 1, len, fd);
	if( ss < 0 ) { perror("failed write"); return; }

	fclose(fd);

}

void read_ctx(char *fn, char *buffer, int len)
{
	FILE *fd = NULL;
	size_t ss;
	char *p;

	fd = fopen(fn, "r");
	if( !fd ) { perror("failed open2"); return; }

	fread(buffer, len, 1, fd);
	//printf("buffer=[%s]\n", buffer);

	/*
	p = strtok(buffer, "\n");
	printf("%s\n", p);
	while(p=strtok(NULL,"\n")){
		printf("%s\n", p);
	}
	*/


	fclose(fd);

}

void parse(char *src, char **path, char **value)
{
	char *ptr;

	if (!strlen(src)) return;

	ptr = strstr(src, "=");
	if (!ptr) return;

	*path = src;
	*value = ptr+1;
	*ptr = 0;

}


int main()
{
	char buffer[4096];
	char *fn = "sample.profile";
	char *path, *value;
	read_ctx(fn, buffer, sizeof(buffer));

	char *p = strtok(buffer, "\n");
	printf("[%s]", p);
	parse(p, &path, &value);
	printf("==>path=[%s], value=[%s]\n", path, value);
	while(p=strtok(NULL,"\n")){

		printf("[%s]", p);
		parse(p, &path, &value);
		printf("==>path=[%s], value=[%s]\n", path, value);
	}


	printf("ssss\n");

	//char *getcwd(char *buf, size_t size);
	path = getcwd(buffer, 4096);
	printf("path=%p\n", path);
	printf("buffer=%p,%s\n", buffer, buffer);
}
