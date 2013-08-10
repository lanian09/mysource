#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stropts.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/old_procfs.h>

#define OS_PROC_DIR	"/proc"

int check_my_run_status (char *procname)
{
	DIR				*dirp;
	struct dirent	*dp;
	char            startTime[25];
	char			pname[256], *prealname;
	int				fd;
	prpsinfo_t		psInfo;
	pid_t			mypid;

	if ((dirp = opendir(OS_PROC_DIR)) == NULL) {
		fprintf(stderr,"\n opendir fail[%s]; err=%d(%s)\n\n", OS_PROC_DIR, errno, strerror(errno));
		return -1;
	}

	mypid = getpid();

	while ((dp = readdir(dirp)) != NULL) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;

		sprintf(pname,"%s/%s", OS_PROC_DIR, dp->d_name);
		
		if ((fd = open(pname, O_RDONLY)) < 0)
			continue;
		if (ioctl (fd, PIOCPSINFO, &psInfo) < 0) {
			close(fd);
			continue;
		}
		close(fd);

		// exception handling - process name "/BSDM/BIN/samd"
		if((prealname = strrchr(psInfo.pr_psargs, '/'))){
			prealname++;
		}else{
			prealname = psInfo.pr_psargs;
		}

		if(!strcasecmp(prealname, procname)){
			strftime (startTime, 32, "%m-%d %H:%M", localtime((time_t*)&(psInfo.pr_start.tv_sec)));
			if(psInfo.pr_pid != mypid){
				fprintf(stderr, "%s is already running as pid %d from %s\n", procname, 
					(int)psInfo.pr_pid, startTime);
					closedir(dirp);
					return -1;
			}
		}

	}
	closedir(dirp);

	return 0;
}
