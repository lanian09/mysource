#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

int read_profiles(char *buffer, int buflen)
{
	int fcnt, len, temp;
	struct dirent *dp;
	DIR *dfd = opendir(".");

	fcnt = 0;
	if(dfd != NULL) {
		len = 0;
		while((dp = readdir(dfd)) != NULL){
			printf("%s\n", dp->d_name);
			if( !strncmp(dp->d_name,".", 1) || !strncmp(dp->d_name, "..", 2) ) continue;
			if( !strstr(dp->d_name,".profile") ) continue;

			/* file */
			temp = strlen(dp->d_name);
			if( len + temp > buflen ) break;
			sprintf(&buffer[len], "%s\n", dp->d_name);
			len += strlen(buffer);
			fcnt++;
		}
		closedir(dfd);
	}
	return fcnt;
}

char *parse(char *buffer, char **handle)
{
	char *ptr;
	char *start_ptr;

	if (!buffer) {
		start_ptr = *handle;
	} else {
		start_ptr = buffer;
	}
	ptr = strstr(start_ptr, "\n");
	if (!ptr) return NULL;

	*handle = ptr+1;
	*ptr = 0;
	return start_ptr;
}

int main()
{
	char buffer[300];
	char *p, *handle;
	int cnt;
	cnt = read_profiles(buffer, 300);
	if( !cnt ){
		printf("no files\n");
		return(0);
	}

	printf("buffer[%x]=%s\n", buffer, buffer);
	p = strtok(buffer,"\n");
	//p = parse(buffer, &handle);
	printf("buffer[%x]=%s\n", buffer, buffer);
	printf("p1[%x]=[%s]\n", p, p);

	while( p=strtok(NULL,"\n") ){
	//while( p=parse(NULL,&handle) ){
		printf("p=[%s]\n", p);
	}

	printf("sizeof(buffer)=%d\n", sizeof(buffer));
}
