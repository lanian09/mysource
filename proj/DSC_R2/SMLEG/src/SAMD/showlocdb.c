#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/shm.h>

#include <commlib.h>
#include <sysconf.h>
#include <sfm_msgtypes.h>


SFM_SysCommMsgType	*loc_sadb;

SFM_sfdb	*sfdb;

#define SFM_L3PD_SIZE	sizeof(SFM_L3PD)
int attchSfdb (void);
void printGroupInfo (void);
void printSysInfo (void);
void printL3pdInfo (void);
void printSpecInfo (void);

extern int errno;

#define STAT_OFFSET_UNIT 300
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char *argv[])
{
	int	interval=2;
	//char opt[10];

	if (attchSfdb () < 0)
		return -1;

	while(1){
		printf ("=== SHOW LOC_SADB ===\n");
		printSysInfo ();
		printf ("==================\n\n");
		sleep(interval);
	}

	return 0;

} //----- End of main -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int attchSfdb ()
{
	char	*env, tmp[64], fname[256];
	int	shmId, key;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"not found %s environment name\n", IV_HOME);
		return -1;
	}

    sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	if(conflib_getNthTokenInFileSection(fname, "SHARED_MEMORY_KEY", "SHM_LOC_SADB", 1, tmp) < 0)
		return -1;

	key = strtol(tmp, 0, 0);

	if( (shmId = (int)shmget(key, sizeof(SFM_SysCommMsgType), 0666 | IPC_CREAT)) < 0)
	{
		if(errno != ENOENT)
		{
			fprintf(stderr, "[%s:%s:%d] shmget fail; key=0x%x, err=%d(%s)\n", __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
			return -1;
		}
	}

	if( (loc_sadb = (SFM_SysCommMsgType*)shmat(shmId, 0, 0)) == (SFM_SysCommMsgType*)-1)
	{
		fprintf(stderr, "[%s:%s:%d] shmat fail; key=0x%x, err=%d(%s)\n", __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
		return -1;
	}


	return 1;

} //----- End of attchSfdb -----//

//------------------------------------------------------------------------------

void printSpecInfo() {
	int 	i, j;
	//char sysName[2][5];

	//fprintf(stdout, "\n DSC H/W STATUS\n\n", loc_sadb->loc_system_dup.myLocalDupStatus);

	//fprintf(stdout, "\n BSD H/W STATUS\n\n");
	fprintf(stdout, "\n DSC H/W STATUS\n\n");
	
	for(i=1; i<3; i++){
		fprintf(stdout, "\n %s \n", sfdb->sys[i].commInfo.name);
		for(j=0; j < SFM_MAX_HPUX_HW_COM; j++){
			fprintf(stdout, "NAME: %s (mask:status:prevstatus:level)"
				" %d : %d : %d : %d"
				, sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].name
				, sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].mask
				, sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].status
				, sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].prevStatus
				, sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].level);
			if (j%5) 
				fprintf(stdout, "\n");
		}
	}

}


void printSysInfo (void)
{

	int i;

	fprintf (stderr, "loc_sadb->processCount: %d.\n", loc_sadb->processCount);

    // Process Infomation check : by sjjeon
    //for(i=0;i<SFM_MAX_PROC_CNT;i++)
    for(i=0;i<loc_sadb->processCount;i++)
    {
        if(strlen(loc_sadb->loc_process_sts[i].processName)<2)
            continue;

        fprintf(stdout,"procName:status:level:pid:time [%s:%d:%d:%d:%ld]\n",
                loc_sadb->loc_process_sts[i].processName,
                loc_sadb->loc_process_sts[i].status,
                loc_sadb->loc_process_sts[i].level,
                (int)loc_sadb->loc_process_sts[i].pid,
				loc_sadb->loc_process_sts[i].uptime
                );
    }

    fprintf(stdout,"RLEG SESS[%d:%d:%d:%d:%d]\n"
				, loc_sadb->smConn[0].dConn
				, loc_sadb->smConn[1].dConn
				, loc_sadb->smConn[2].dConn
				, loc_sadb->smConn[3].dConn
				, loc_sadb->smConn[4].dConn);
/*
	for (i=0; i< SFM_MAX_HPUX_HW_COM ; i++) {

		fprintf(stdout, "NAME: %s, %d\n", loc_sadb->sysSts.linkSts[i].StsName, loc_sadb->sysSts.linkSts[i].status);
	}
*/
}

