#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

int read_profiles(char *buffer, int buflen)
{
	int fcnt, len, temp;
	struct dirent *dp;
	DIR *dfd = opendir("./profiles");

	fcnt = 0;
	if(dfd != NULL) {
		len = 0;
		while((dp = readdir(dfd)) != NULL){
			printf("%s\n", dp->d_name);
			if( !strncmp(dp->d_name,".", 1) || !strncmp(dp->d_name, "..", 2) ) continue;

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

int main()
{
	char buffer[300];
	char *p;
	int cnt;
	cnt = read_profiles(buffer, 300);
	if( !cnt ){
		printf("no files\n");
		return(0);
	}

	p = strtok(buffer,"\n");
	printf("p1=[%s]\n", p);

	while( p=strtok(NULL,"\n") ){
		printf("p=[%s]\n", p);
	}
}
