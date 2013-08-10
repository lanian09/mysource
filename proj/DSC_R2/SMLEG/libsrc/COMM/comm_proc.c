#include "loglib.h"
#include "comm_proc.h"

extern int trclib_writeLogErr (char *fName, int lNum, char *msg);

int get_system_uptime(time_t *sysuptime)
{
    FILE	*fp;
    char    temp[128], logBuf[1024];
    time_t  system_boot_time;
    int     btime_found;

    fp = fopen(SYS_STATUS_FILE, "r");

    if(fp == NULL){
        sprintf(logBuf, "%s open fail - %s\n", SYS_STATUS_FILE, strerror(errno));
        trclib_writeLogErr(FL, logBuf);
        return -1;
    }

    btime_found = 0;
    while(fscanf(fp, "%s %ld", temp, &system_boot_time) != EOF){
        if(!strncmp(temp, "btime", sizeof("btime"))){
            btime_found = 1;
            break;
        }
    }

    fclose(fp);

    if(!btime_found){
        sprintf(logBuf, "ERROR : Not found \"btime\" in %s\n", SYS_STATUS_FILE);
        trclib_writeLogErr(FL, logBuf);
		*sysuptime = 0;
        return -1;
    }

	*sysuptime = system_boot_time;

    return 0;
}

int get_proc_starttime(pid_t prcpid, time_t sysuptime, time_t *prcuptime, char *starttime)
{
    FILE	*fp;
    char    readbuf[1024], logBuf[1024];
    char    prcfilename[128], *pc, *pt;
    int     space_on, column_on, column;
    time_t  proc_start_time;
    long long tmp_long_long;
    struct tm ptime;
    

    snprintf(prcfilename, sizeof(prcfilename), "/proc/%lu/stat", prcpid);

    fp = fopen(prcfilename, "r");
    if(!fp){
        sprintf(logBuf, "ERROR : %s fopen fail - %s\n", prcfilename, strerror(errno));
        trclib_writeLogErr(FL, logBuf);
        return -1;
    }

    memset(readbuf, 0x00, sizeof(readbuf));

    if(!fgets(readbuf, sizeof(readbuf), fp)){
        sprintf(logBuf, "ERROR : %s fgets fail - %s\n", prcfilename, strerror(errno));
        trclib_writeLogErr(FL, logBuf);
        return -1;
    }

    fclose(fp);

	column = 0;
    column_on = 0;
    space_on = 0;
    pc = readbuf;
    while(*pc){
		if(*pc == ' ' || *pc == '\t'){
            space_on = 1;
            column_on = 0;
		}else{
            if(!column_on){
                space_on = 0;
                column++;
            }
            column_on = 1;
		}

        if(column == PROC_STAT_STATTIME_POS)
            break;
        pc++;
    }

	pt = pc;
    while(*pc != ' ' && *pc != '\t')
		*pc++;

	*pc = 0x00;
	
	/*
	proc_start_time = atol(pt);
	proc_start_time /= HZ;
	proc_start_time += sysuptime;
	*/
	tmp_long_long = strtoll(pt, NULL, 10);
#ifdef __LINUX__
	proc_start_time = tmp_long_long / HZ;
#else
	/*add by sjjeon : 2009/04/07*/
	proc_start_time = tmp_long_long / 100;
#endif
	proc_start_time += sysuptime;
    
	if(prcuptime != NULL){
        *prcuptime = proc_start_time;
    }

    if(starttime != NULL){
        localtime_r(&proc_start_time, &ptime);
        sprintf(starttime, "%02d-%02d %02d:%02d",
                ptime.tm_mon+1, ptime.tm_mday, ptime.tm_hour, ptime.tm_min);
    }

	return 0;
}
