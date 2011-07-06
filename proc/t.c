#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/procfs.h>

int getproc()
{
	DIR *dirp;
	struct dirent *direntp;
	prpsinfo_t psInfo;

	if( (dirp = opendir("/proc")) == (DIR *)NULL ){
		printf("error open dir\n");
		return -1;
	}
	
	while( (direntp = readdir(dirp)) != NULL ){
		printf("direntp->d_name=%s\n", direntp->d_name);
	}
	return 1;

}

int main()
{
	getproc();
	return 0;
}
