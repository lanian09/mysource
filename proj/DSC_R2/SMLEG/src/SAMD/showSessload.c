/* make by helca 2007.02.01*/
/* ±ÞÇÏ°Ô ¸¸µé¾î ¾û¸ÁÀÌ´Ù...¤Ñ¤Ñ;; */
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

SFM_SysCommMsgType	        *loc_sadb;
void showSessLoad();
int main()
{
	char	*env, tmp[32];
	char    mySysName[COMM_MAX_NAME_LEN];
	char	iv_home[64], l_sysconf[256];
	int		key, shmId;
	int		interval=2;

	GeneralQMsgType rxGenQMsg;
	memset(&rxGenQMsg, 0, sizeof(GeneralQMsgType));

	if ((env = getenv(MY_SYS_NAME)) == NULL) {
		fprintf(stderr,"[samd_init] not found %s environment name\n", MY_SYS_NAME);
		return -1;
	}
	strcpy (mySysName, env);

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[samd_init] not found %s environment name\n", IV_HOME);
		return -1;
	}
	strcpy (iv_home, env);

	sprintf(l_sysconf, "%s/%s", iv_home, SYSCONF_FILE);

	if (conflib_getNthTokenInFileSection (l_sysconf, "SHARED_MEMORY_KEY", "SHM_LOC_SADB", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	if ((shmId = (int)shmget (key, sizeof(SFM_SysCommMsgType), 0666|IPC_CREAT)) < 0) {
		if (errno != ENOENT) {
			fprintf (stderr,"[samd_init1] shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
	}

	if ((loc_sadb = (SFM_SysCommMsgType*) shmat (shmId,0,0)) == (SFM_SysCommMsgType*)-1) {
		fprintf (stderr,"[samd_init2] shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	/*
	for(i=0;i<loc_sadb->processCount; i++){
		fprintf(stderr, "TEST prc_name:%s, sts:%d\n",
				loc_sadb->sdmd_Prc_sts[i].processName, loc_sadb->sdmd_Prc_sts[i].status);
	}
	fprintf(stderr, "--------------------------------------------\n");
	*/

	/*
	fprintf(stderr, "SFM_SysCommMsgType: %d\n", sizeof(SFM_SysCommMsgType));
	fprintf(stderr, "SFM_SysCommProcSts: %d\n", sizeof(SFM_SysCommProcSts));
	fprintf(stderr, "SFM_SysCommDiskSts: %d\n", sizeof(SFM_SysCommDiskSts));
	fprintf(stderr, "SFM_SysCommLanSts: %d\n", sizeof(SFM_SysCommLanSts));
	fprintf(stderr, "SFM_SysCommQueSts: %d\n", sizeof(SFM_SysCommQueSts));
	fprintf(stderr, "SFM_SysCommLinkSts: %d\n", sizeof(SFM_SysCommLinkSts));
	fprintf(stderr, "SFM_SysDuplicationSts: %d\n", sizeof(SFM_SysDuplicationSts));
	fprintf(stderr, "SFM_SysSuccessRate: %d\n", sizeof(SFM_SysSuccessRate));
	fprintf(stderr, "SFM_SysIFBond: %d\n", sizeof(SFM_SysIFBond));
	fprintf(stderr, "SFM_SysRsrcLoad: %d\n", sizeof(SFM_SysRsrcLoad));
	fprintf(stderr, "SFM_SysSts: %d\n", sizeof(SFM_SysSts));
	fprintf(stderr, "SFM_Duia: %d\n", sizeof(SFM_Duia));
	fprintf(stderr, "SFM_SysCommSdmdPrcSts: %d\n", sizeof(SFM_SysCommSdmdPrcSts));

	fprintf(stderr, "char:%d\n", sizeof(char));
	fprintf(stderr, "unsigned char:%d\n", sizeof(unsigned char));
	fprintf(stderr, "unsigned short:%d\n", sizeof(unsigned short));
	fprintf(stderr, "pid_t:%d\n", sizeof(pid_t));
	fprintf(stderr, "time_t:%d\n", sizeof(time_t));

	fprintf(stderr, "cpuCount:%d,diskCount:%d,lanCount:%d\n",
			loc_sadb->cpuCount, loc_sadb->diskCount,loc_sadb->lanCount);

	*/


	while(1){
		showSessLoad (mySysName);
		sleep(interval);
	}
}
void showSessLoad(char* mySysName)
{
	int     rsrcload[SFM_MAX_RSRC_LOAD_CNT];
	int     i,j;

	memset(rsrcload, 0x00, (sizeof(int)*SFM_MAX_RSRC_LOAD_CNT));

	for(i=0; i<SFM_MAX_RSRC_LOAD_CNT; i++){
		for(j=0; j<MAX_MP_NUM1; j++){
			rsrcload[i] += loc_sadb->rsrc_load.rsrcload[i][j];
		}
	}

    fprintf (stderr,"    SYSTEM = %s\n",mySysName);
    fprintf (stderr,"    ======================================================\n");
    fprintf (stderr,"      SESSION_TYPE    CURRENT_SESSION_COUNT(MAX_SESSION)       \n");
	fprintf (stderr,"    ======================================================\n");
    fprintf (stderr,"      CDR_TCPIP                  %10d(     500000)\n", rsrcload[DEF_MMDB_SESS]);
    fprintf (stderr,"      CDR_SESSION                %10d(     150000)\n", rsrcload[DEF_MMDB_OBJ]);
    fprintf (stderr,"      PCDR_TCPIP                 %10d(     500000)\n", rsrcload[DEF_MMDB_SESS2]);
    fprintf (stderr,"      PCDR_SESSION               %10d(     150000)\n", rsrcload[DEF_MMDB_OBJ2]);
	fprintf (stderr,"      TRCDR_SESSION              %10d(     150000)\n", rsrcload[DEF_MMDB_CALL]);
    fprintf (stderr,"      WAP1_SESSION               %10d(       5000)\n", rsrcload[DEF_MMDB_WAP1]);
    fprintf (stderr,"      WAP2_SESSION               %10d(       5000)\n", rsrcload[DEF_MMDB_WAP2]);
    fprintf (stderr,"      HTTP_SESSION               %10d(       5000)\n", rsrcload[DEF_MMDB_HTTP]);
    fprintf (stderr,"      VOD_SESSION                %10d(      20000)\n", rsrcload[DEF_MMDB_VODS]);
	fprintf (stderr,"      VT_CALL                    %10d(      40000)\n", rsrcload[DEF_MMDB_VT]);
	fprintf (stderr,"      UDRGEN_CALL                %10d(     150000)\n", rsrcload[DEF_MMDB_UDR]);
    fprintf (stderr,"    ------------------------------------------------------\n");
    fprintf (stderr,"      TOTAL COUNT = 10                    \n");
    fprintf (stderr,"    ======================================================\n");

    return;
}

