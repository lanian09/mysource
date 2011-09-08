#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/shm.h>

//#include <commlib.h>
#include <sysconf.h>
#include <sfm_msgtypes.h>
//#include "fimd_proto.h"

SFM_SysCommMsgType  *loc_sadb;

typedef struct _mem_count_ {
	double rad_sess;
} _mem_check;
_mem_check      	*gpShmem;

int attchSadb (void);
void printSadbInfo (void);
void printSessInfo (void);

extern int errno;

#define STAT_OFFSET_UNIT 300
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (char ac, char *av[])
{
	int	i, interval=2, countFlag=0, count;
	char opt[10];

	if(ac>=2){
		strcpy(opt, av[1]);
		
//		return -1;
	}
	
	if (attchSadb () < 0)
		return -1;

	if (attchSess () < 0)
		return -1;

	if(av[1]){

		if(!strcasecmp(av[1], "ACT")){ 		// system mode view(1 : active, 2 : standby)
			while(1){
				printSadbInfo();
				sleep(interval);
			}
		}
		else if(!strcasecmp(av[1], "SESS")){ 		// session count
			while(1){
				printSessInfo();
				sleep(interval);
			}
		}
	}else
		fprintf(stderr, "No input type~! (LIST = [ACT:SESS])\n");
	return 0;

} //----- End of main -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/* INIT_SHM: SHM_LOC_SADB */
int attchSadb ()
{
	char	*env, tmp[64], fname[256];
	int	shmId, key;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"not found %s environment name\n", IV_HOME);
		return -1;
	}
    sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	if (conflib_getNthTokenInFileSection(fname, "SHARED_MEMORY_KEY", "SHM_LOC_SADB", 1, tmp) < 0)
		return -1;
	key = strtol(tmp, 0, 0);

	if( (shmId = (int)shmget(key, sizeof(SFM_SysCommMsgType), 0644 | IPC_CREAT)) < 0)
	{
		if(errno != ENOENT)
		{
			fprintf(stderr, "[%s:%s:%d] shmget fail; key=0x%x, err=%d(%s)\n"
					, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
			return -1;
		}
	}

	if( (loc_sadb = (SFM_SysCommMsgType*)shmat(shmId, 0, 0)) == (SFM_SysCommMsgType*)-1)
	{
		fprintf(stderr, "[%s:%s:%d] shmat fail; key=0x%x, err=%d(%s)\n"
				, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
		return -1;
	}
}
//------------------------------------------------------------------------------
int attchSess ()
{
	char	*env, tmp[64], fname[256];
	int	shmId, key;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"not found %s environment name\n", IV_HOME);
		return -1;
	}
	sprintf (fname, "%s/%s", env, SYSCONF_FILE);
	if (conflib_getNthTokenInFileSection(fname, "SHARED_MEMORY_KEY", "SHM_LEG_SESS_CNT", 1, tmp) < 0)
		return -1;

	key = strtol(tmp, 0, 0);
	if( (shmId = (int)shmget(key, sizeof(_mem_check), 0666 | IPC_CREAT)) < 0)
	{
		if(errno != ENOENT)
		{
			fprintf(stderr, "[%s:%s:%d] shmget fail; key=0x%x, err=%d(%s)\n"
					, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
			return -1;
		}
	}
	if( (gpShmem = (_mem_check *)shmat(shmId, 0, 0)) == (_mem_check *)-1)
	{
		fprintf(stderr, "[%s:%s:%d] shmat fail; key=0x%x, err=%d(%s)\n"
				, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
		return -1;                                                                                                  
	}                                                                                                               
	memset(gpShmem, 0x00, sizeof(_mem_check)); 
}
//------------------------------------------------------------------------------
#if 1
void printSadbInfo (void)
{
	int i;
	
	fprintf(stdout,"\n############################################################\n");
	fprintf(stdout,"\n########           loc_sadb info       #####################\n");
	fprintf(stdout,"\n############################################################\n");
	fprintf(stdout,"\n[SYSTEM OPERATION MODE = %d]\n"
			, loc_sadb->loc_system_dup.myLocalDupStatus);
//sjjeon
    for(i=0; i<SFM_HW_MAX_FAN_NUM; i++){
        fprintf(stdout,"(hw:name:sts)(fan(%d):%s:%d)\t",i, loc_sadb->sysSts.fanSts[i].StsName, loc_sadb->sysSts.fanSts[i].status);
		if(!(i%3)) fprintf(stdout,"\n");
    }    
	fprintf(stdout,"\n");
    for(i=0; i<SFM_HW_MAX_PWR_NUM; i++){
        fprintf(stdout,"(hw:sts)(pwr(%d):%d)\t",i, loc_sadb->sysSts.pwrSts[i].status);
    }    
	fprintf(stdout,"\n");
/*
    for(i=0; i<SFM_MAX_DEV_CNT; i++){
        fprintf(stdout,"(hw:sts)(loc_link(%d):%d)\t",i, loc_sadb->loc_link_sts[i].status);
    }    
	fprintf(stdout,"\n");
*/
    for(i=0; i<SFM_HW_MAX_LINK_NUM; i++){ //link : 8 , mysql : 1, timetens : 1, sm: 1, sm :1
        fprintf(stdout,"(hw:sts)(link(%d):%d)\t",i, loc_sadb->sysSts.linkSts[i].status);
		if(!(i%3)) fprintf(stdout,"\n");
    }    
	fprintf(stdout,"\n");
/*
    for(i=0; i<SFM_HW_MAX_CPU_NUM; i++){
        fprintf(stdout,"(hw:sts)(cpu(%d):%d)\t",i, loc_sadb->sysSts.cpuSts[i].status);
		if(!(i%3)) fprintf(stdout,"\n");
    }    
	fprintf(stdout,"\n");
*/
    for(i=0; i<SFM_HW_MAX_DISK_NUM; i++){
        fprintf(stdout,"(hw:sts)(disk(%d):%d)\t",i, loc_sadb->sysSts.diskSts[i].status);
    }    
	fprintf(stdout,"\n");
	
	fprintf(stdout,"\n############################################################\n");

}

#endif
//------------------------------------------------------------------------------
void printSessInfo (void)
{
	fprintf(stdout,"\n\n[SESSION COUNT = %f]"
			, gpShmem->rad_sess);
}

