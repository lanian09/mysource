#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/shm.h>

#include <commlib.h>
#include <sysconf.h>
#include "sfm_msgtypes.h"

int main ()
{
	char    backpath[256], newpath[256], tarname[256], trcBuf[4096], tarname_bin[256];
    	char    cmd[1024], curdir[256], ivhome[256], tmp[128];
    	int     i, procIdx;
    	char    argSysName[COMM_MAX_NAME_LEN], option[COMM_MAX_NAME_LEN];
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
	    	//trclib_writeLogErr(FL, trcBuf);
        	return;
    	}

	strcpy(ivhome, getenv(IV_HOME));
    	strftime(tmp, sizeof(tmp), "%m%d", &tmnow);
    	sprintf(backpath, "%s/BACKUP/%s", ivhome, tmp);

    	if(mkdir(backpath, 0777) < 0){
        	if(errno != EEXIST){
            		sprintf(trcBuf, "BACKUP DIRECTORY CREATE FAIL\n");
            		//trclib_writeLogErr(FL, trcBuf);
            		return;
        	}
    	}

    	if(!getcwd(curdir, sizeof(curdir))){
        	sprintf(trcBuf, "SYSTEM LIBRARY(getcwd) CALL FAIL\n");
        	//trclib_writeLogErr(FL, trcBuf);
        	return;
    	}

    	//BSDM_DATA_0706181230.tar
    	sprintf(tarname, "%s/%s_%s_%.02d%.02d%.02d%.02d%.02d.tar",
    		backpath, mySysName, "DATA", (tmnow.tm_year-100), (tmnow.tm_mon+1),
        	tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min);

    	//sprintf(cmd, "/bin/tar cvf %s /BSDM/DATA ", tarname);
    	sprintf(cmd, "/bin/tar cvf %s /DSCM/DATA ", tarname);
    	
	if(system(cmd) < 0){
        	sprintf(trcBuf, "SYSTEM LIBRARY(system) CALL FAIL\n");
        	//trclib_writeLogErr(FL, trcBuf);
        	return;
    	}

	//BSDM_BIN_0706181230.tar
	sprintf(tarname, "%s/%s_%s_%.02d%.02d%.02d%.02d%.02d.tar",
		backpath, mySysName, "BIN", (tmnow.tm_year-100), (tmnow.tm_mon+1),
	        tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min);

   	//sprintf(cmd, "/bin/tar cvf %s /BSDM/BIN ", tarname);
   	sprintf(cmd, "/bin/tar cvf %s /DSCM/BIN ", tarname);
	
	if(system(cmd) < 0){
	    sprintf(trcBuf, "SYSTEM LIBRARY(system) CALL FAIL\n");
		//trclib_writeLogErr(FL, trcBuf);
	    return;
	}
    
	if(chdir(curdir) < 0){
        	sprintf(trcBuf, "SYSTEM LIBRARY(chdir) CALL FAIL\n");
        	//trclib_writeLogErr(FL, trcBuf);
        	return;
	}
	return;
}


