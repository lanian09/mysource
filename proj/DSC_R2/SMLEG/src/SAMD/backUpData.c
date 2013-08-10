#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/shm.h>

#include <commlib.h>
#include <sysconf.h>
#include "sfm_msgtypes.h"
#include "samd.h"

int main(void)
{
    char    oldpath[256], newpath[256], tarname[256], trcBuf[4096];
    char    cmd[1024], curdir[256], ivhome[256], tmp[128];
    char	mySysName[COMM_MAX_NAME_LEN];
	char	*env;
	struct tm       tmnow;
    time_t          now = time(NULL);

	if ((env = getenv(MY_SYS_NAME)) == NULL) {
		    fprintf(stderr,"[samd_init] not found %s environment name\n", MY_SYS_NAME);
			    return -1;
	}

	strcpy(mySysName, env);
    if(!localtime_r(&now, &tmnow)){
    	sprintf(trcBuf, "SYSTEM LIBRARY(local_time_r) CALL FAIL\n");
	    trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

	strcpy(ivhome, getenv(IV_HOME));
    strftime(tmp, sizeof(tmp), "%m%d", &tmnow);
    sprintf(oldpath, "%s/OLD/%s", ivhome, tmp);

#ifdef __LINUX__ /*sjjeon*/
    if(mkdir(oldpath, DEFFILEMODE) < 0){
#else
    //if(mkdir(oldpath, 0755) < 0){
    if(mkdir(oldpath, 0774) < 0){
#endif
        if(errno != EEXIST){
            sprintf(trcBuf, "BACKUP DIRECTORY CREATE FAIL\n");
            trclib_writeLogErr(FL, trcBuf);
            return -1;
        }
    }
    if(!getcwd(curdir, sizeof(curdir))){
        sprintf(trcBuf, "SYSTEM LIBRARY(getcwd) CALL FAIL\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    sprintf(newpath, "%s/NEW", ivhome);
	    if(chdir(newpath) < 0){
	        sprintf(trcBuf, "SYSTEM LIBRARY(chdir) CALL FAIL\n");
	        trclib_writeLogErr(FL, trcBuf);
	        return -1;
	    }

    //BSDA_DATA_0706181230.tar
    sprintf(tarname, "%s/%s_%s_%.02d%.02d%.02d%.02d%.02d.tar",
    		oldpath, mySysName, "DATA", (tmnow.tm_year-100), (tmnow.tm_mon+1),
            tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min);

    sprintf(cmd, "/bin/tar cvf %s DATA > /dev/null", tarname);

    if(system(cmd) < 0){
        sprintf(trcBuf, "SYSTEM LIBRARY(system) CALL FAIL\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

	//BSDA_BIN_0706181230.tar
	sprintf(tarname, "%s/%s_%s_%.02d%.02d%.02d%.02d%.02d.tar",
			oldpath, mySysName, "BIN", (tmnow.tm_year-100), (tmnow.tm_mon+1),
	        tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min);

   	sprintf(cmd, "/bin/tar cvf %s BIN > /dev/null", tarname);

	if(system(cmd) < 0){
	    sprintf(trcBuf, "SYSTEM LIBRARY(system) CALL FAIL\n");
		trclib_writeLogErr(FL, trcBuf);
	    return -1;
	}

	if(chdir(curdir) < 0){
        sprintf(trcBuf, "SYSTEM LIBRARY(chdir) CALL FAIL\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
	}
	return 0;
}
