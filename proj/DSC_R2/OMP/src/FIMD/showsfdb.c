#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/shm.h>

#include <commlib.h>
#include <sysconf.h>
#include <sfm_msgtypes.h>
#include "fimd_proto.h"
#include "../../include/nmsif.h"

SFM_sfdb	*sfdb;
SFM_L3PD	*l3pd;
SFM_SCE     *g_pstSCEInfo;
SFM_L2Dev	*g_pstL2Dev;
/* hjjung */
//SFM_LEG		*g_pstLEGInfo;
SFM_CALL	*g_pstCALLInfo;

#define SFM_L3PD_SIZE	sizeof(SFM_L3PD)
int attchSfdb (void);
int attchL3pd (void);
int attchsce (void);
/* hjjung */
int attchleg (void);
int attchtps (void);
void printGroupInfo (void);
void printSysInfo (void);
void printL3pdInfo (void);
void printSpecInfo (void);
void printSCEinfo (void);
/* hjjung */
void printLEGinfo (void);
void printTPSinfo (void); // added by dcham 20110525 for TPS
void printL2Info(void);
void print_NMSIF_Info(void);
extern int attchL2Dev();

extern int errno;
extern char *int2dot (int ipaddr);
#define STAT_OFFSET_UNIT 300
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	int	interval=2;/*,  count=0;*/
	char opt[10];

	if(ac>=2){
		strcpy(opt, av[1]);
		
//		return -1;
	}
	
	if (attchSfdb () < 0)
		return -1;

	if (attchL3pd () < 0)
		return -1;
	
	if (attchsce () < 0)
		return -1;

	/* hjjung */
	if (attchleg () < 0)
		return -1;

	if (attchtps () < 0)
		return -1;

	if(attchL2Dev() < 0)
		return -1;

	sfdb->sys[0].commInfo.procInfo[3].status = 0;

//	printGroupInfo ();
	if(av[1]){
		if(!strcasecmp(av[1], "SYS")){
			while(1){
				printSysInfo ();
				sleep(interval);
			}
		}

		if(!strcasecmp(av[1], "TAP")){
			while(1){
				printL3pdInfo();
				sleep(interval);
			}
		}

		if(!strcasecmp(av[1], "SPEC")){
			while(1){
				printSpecInfo();
				sleep(interval);
			}
		}

		if(!strcasecmp(av[1], "SCE")){
			while(1){
				printSCEinfo();
				sleep(interval);
			}
		}

		/* hjjung */
		if(!strcasecmp(av[1], "RLEG")){
			while(1){
				printLEGinfo();
				sleep(interval);
			}
		}
		/* added by dcham 20110525 for TPS */
		if(!strcasecmp(av[1], "TPS")){
			while(1){
				printTPSinfo();
				sleep(interval);
			}
		}
		
		if(!strcasecmp(av[1], "L2")){
			while(1){
				printL2Info();
				sleep(interval);
			}
		}

		if(!strcasecmp(av[1], "NMS")){
			while(1){
				print_NMSIF_Info();
				sleep(interval);
			}
		}

	}else
		fprintf(stderr, "No input type~! (LIST = [SYS:TAP:SPEC:SCE:L2:NMS:RLEG:TPS])\n");
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

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_SFDB", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	if ((shmId = (int)shmget (key, SFM_SFDB_SIZE, 0666)) < 0) {
		fprintf(stderr,"SFDB shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	if ((sfdb = (SFM_sfdb*) shmat (shmId,0,0)) == (SFM_sfdb*)-1) {
		fprintf(stderr,"SFDB shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	return 1;

} //----- End of attchSfdb -----//

//------------------------------------------------------------------------------
int attchL3pd ()
{
	char	*env, tmp[64], fname[256];
	int	shmId, key;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"not found %s environment name\n", IV_HOME);
		return -1;
	}
    	sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_L3PD", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	if ((shmId = (int)shmget (key, SFM_L3PD_SIZE, 0644)) < 0) {
		fprintf(stderr,"TAP shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		fprintf(stderr, "#1\n");
		return -1;
	}

	if ((l3pd = (SFM_L3PD*) shmat (shmId,0,0)) == (SFM_L3PD*)-1) {
		fprintf(stderr,"TAP shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		fprintf(stderr, "#2\n");
		return -1;
	}

	return 1;

} //----- End of attchL3pd -----//

int attchsce ()
{
	char	*env, tmp[64], fname[256];
	int	shmId, key;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"not found %s environment name\n", IV_HOME);
		return -1;
	}
    	sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_SCE", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	if ((shmId = (int)shmget (key, SFM_SCE_SIZE, 0644)) < 0) {
		fprintf(stderr,"SCE shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		fprintf(stderr, "#1\n");
		return -1;
	}

	if ((g_pstSCEInfo = (SFM_SCE*) shmat (shmId,0,0)) == (SFM_SCE*)-1) {
		fprintf(stderr,"SCE shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		fprintf(stderr, "#2\n");
		return -1;
	}

	return 1;

} //----- End of attchsce -----//

int attchleg ()
{
	char	*env, tmp[64], fname[256];
	int	shmId, key;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"not found %s environment name\n", IV_HOME);
		return -1;
	}
    	sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_CALL", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	if ((shmId = (int)shmget (key, SFM_CALL_SIZE, 0644)) < 0) {
		fprintf(stderr,"LEG shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		fprintf(stderr, "#1\n");
		return -1;
	}

	if ((g_pstCALLInfo = (SFM_CALL*) shmat (shmId,0,0)) == (SFM_CALL*)-1) {
		fprintf(stderr,"CALL shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		fprintf(stderr, "#2\n");
		return -1;
	}

	return 1;

} //----- End of attchleg -----//

int attchL2Dev()
{
	char	*env, tmp[64], fname[256];
	int	shmId, key;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"not found %s environment name\n", IV_HOME);
		return -1;
	}
    	sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_L2SW", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	if ((shmId = (int)shmget (key, SFM_L2DEV_SIZE, 0644)) < 0) {
		fprintf(stderr,"SCE shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		fprintf(stderr, "#1\n");
		return -1;
	}

	if ((g_pstL2Dev= (SFM_L2Dev*) shmat (shmId,0,0)) == (SFM_L2Dev*)-1) {
		fprintf(stderr,"L2Device shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		fprintf(stderr, "#2\n");
		return -1;
	}

	return 1;

} //----- End of attchL2Dev-----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void printGroupInfo ()
{
	int	i, j;

	for (i=0; i<SYSCONF_MAX_GROUP_NUM; i++)
	{
		if (!strcasecmp (sfdb->group[i].name, ""))
			continue;

		fprintf(stdout,"\n[group=%s,type=%s] memCnt=%d : ",
				sfdb->group[i].name,sfdb->group[i].type, sfdb->group[i].memberCnt);
		for (j=0; j<sfdb->group[i].memberCnt; j++) {
			fprintf(stdout,"(%s)", sfdb->group[i].memberName[j]);
		}
		fprintf(stdout,", lev=%d,min=%d,maj=%d,cri=%d\n",
				sfdb->group[i].level,
				sfdb->group[i].minCnt,
				sfdb->group[i].majCnt,
				sfdb->group[i].criCnt);
	}

	return;

} //----- End of printGroupInfo -----//

int attchtps ()
{
    char    *env, tmp[64], fname[256];
    int shmId, key;

    if ((env = getenv(IV_HOME)) == NULL) {
        fprintf(stderr,"not found %s environment name\n", IV_HOME);
        return -1;
    }
        sprintf (fname, "%s/%s", env, SYSCONF_FILE);

    if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_CALL", 1, tmp) < 0)
        return -1;
    key = strtol(tmp,0,0);

    if ((shmId = (int)shmget (key, SFM_CALL_SIZE, 0644)) < 0) {
        fprintf(stderr,"TPS shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        fprintf(stderr, "#1\n");
        return -1;
    }

    if ((g_pstCALLInfo = (SFM_CALL*) shmat (shmId,0,0)) == (SFM_CALL*)-1) {
        fprintf(stderr,"CALL shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        fprintf(stderr, "#2\n");
        return -1;
    }

    return 1;

} //----- End of attchleg -----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void printSysInfo ()
{
	int	i, j;
	int	l;

	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++)
	{
		if (!strcasecmp (sfdb->sys[i].commInfo.name, ""))
		{
			continue;
		}

		if(!strcasecmp(sfdb->sys[i].commInfo.type, "MP"))
		{
			fprintf(stdout, "\n\nDuplication myStatus:%d,yourStatus:%d\n", 
					sfdb->sys[i].commInfo.systemDup.myStatus,sfdb->sys[i].commInfo.systemDup.yourStatus);
		}

		fprintf(stdout,"\nsysindex=%d, [%s-%s-%s],lev=%d,min=%d,maj=%d,cri=%d\n",
				i,
				sfdb->sys[i].commInfo.type,
				sfdb->sys[i].commInfo.group,
				sfdb->sys[i].commInfo.name,
				sfdb->sys[i].almInfo.level,
				sfdb->sys[i].almInfo.minCnt,
				sfdb->sys[i].almInfo.majCnt,
				sfdb->sys[i].almInfo.criCnt);

		fprintf(stdout,"  cpu:");
		for (j=0; j<sfdb->sys[i].commInfo.cpuCnt; j++) {
			fprintf(stdout,"(%d,lev=%d)",
					sfdb->sys[i].commInfo.cpuInfo.usage[j],
					sfdb->sys[i].commInfo.cpuInfo.level[j]);
		}

		fprintf(stdout,", mem:(%d,lev=%d,mask=%d)\n",
				sfdb->sys[i].commInfo.memInfo.usage,
				sfdb->sys[i].commInfo.memInfo.level,
				sfdb->sys[i].commInfo.memInfo.mask);

		fprintf(stdout,"  disk:");
		for (j=0; j<sfdb->sys[i].commInfo.diskCnt; j++) {
			fprintf(stdout,"(%s:%d)",
					sfdb->sys[i].commInfo.diskInfo[j].name,
					sfdb->sys[i].commInfo.diskInfo[j].usage);
		}
        	fprintf(stdout, "(Total_disk_usage: %d)", sfdb->sys[i].commInfo.total_disk_usage);

		fprintf(stdout,"\n  lan:"); l=0;
		for (j=0; j<sfdb->sys[i].commInfo.lanCnt; j++) {
			if(!(++l%4)){ fprintf(stdout,"\n       ");}
			fprintf(stdout,"(%s:%s:%d:lev=%d)", sfdb->sys[i].commInfo.lanInfo[j].name,
					sfdb->sys[i].commInfo.lanInfo[j].targetIp,
					sfdb->sys[i].commInfo.lanInfo[j].status,
					sfdb->sys[i].commInfo.lanInfo[j].level);
		}
		/* by helca */
		if(!strcasecmp(sfdb->sys[i].commInfo.type, "MP")){
			fprintf(stdout,"\n  Remote_lan:"); l=0;
			for (j=0; j<sfdb->sys[i].commInfo.lanCnt; j++) {
				if(!(++l%4)){ fprintf(stdout,"\n       ");}
				fprintf(stdout,"(%s:%s:%d:lev=%d)", sfdb->sys[i].commInfo.rmtLanInfo[j].name,
					sfdb->sys[i].commInfo.rmtLanInfo[j].targetIp,
					sfdb->sys[i].commInfo.rmtLanInfo[j].status,
					sfdb->sys[i].commInfo.rmtLanInfo[j].level);
			}
		}
		/* by helca */
		if(!strcasecmp(sfdb->sys[i].commInfo.type, "MP")){
			fprintf(stdout,"\n  Optical_lan:"); l=0;
			if(!(++l%4)){ fprintf(stdout,"\n       ");}
			fprintf(stdout,"(%s:%d:lev=%d)", sfdb->sys[i].commInfo.optLanInfo[j].name,
					sfdb->sys[i].commInfo.optLanInfo[j].status,
					sfdb->sys[i].commInfo.optLanInfo[j].level);
			
		}

		
		fprintf(stdout,"\n  proc:(name:sta:lev)"); l=0;
		for (j=0; j<sfdb->sys[i].commInfo.procCnt; j++) {
			if(!(++l%5)){ fprintf(stdout,"\n       ");}
			fprintf(stdout,"(%s:%d:%d) %d,%d",
					sfdb->sys[i].commInfo.procInfo[j].name,
					sfdb->sys[i].commInfo.procInfo[j].status,
					sfdb->sys[i].commInfo.procInfo[j].level,
					(int)sfdb->sys[i].commInfo.procInfo[j].pid,
					(int)sfdb->sys[i].commInfo.procInfo[j].uptime
					);
		}

		fprintf(stdout,"\n  queue:(Mask:ID:KEY:Name:Num:Cb:Qb:level:load:Minlimit:Majlimit:Crilimit) cnt=%d", 
				sfdb->sys[i].commInfo.queCnt); l=0;
		//fprintf(stdout,"       ");
		for (j=0; j<sfdb->sys[i].commInfo.queCnt; j++) {
			if(!(l%2)){ fprintf(stdout,"\n       ");}
			fprintf(stdout,"(%d:%d:%d:%s:%d:%d:%d:%d:%d:%d:%d:%d)",
					sfdb->sys[i].commInfo.queInfo[j].mask,
					sfdb->sys[i].commInfo.queInfo[j].qID,
					sfdb->sys[i].commInfo.queInfo[j].qKEY,
					sfdb->sys[i].commInfo.queInfo[j].qNAME,
					sfdb->sys[i].commInfo.queInfo[j].qNUM,
					sfdb->sys[i].commInfo.queInfo[j].cBYTES,
					sfdb->sys[i].commInfo.queInfo[j].qBYTES,
					sfdb->sys[i].commInfo.queInfo[j].level,
					sfdb->sys[i].commInfo.queInfo[j].load,
					sfdb->sys[i].commInfo.queInfo[j].minLimit,
					sfdb->sys[i].commInfo.queInfo[j].majLimit,
					sfdb->sys[i].commInfo.queInfo[j].criLimit);
			l++;	
		}
		/* cps */
		if (strcasecmp (sfdb->sys[i].commInfo.name, "DSCM"))
		{
		//	sfdb->sys[i].commInfo.cpsOverSts.level= 1;
			fprintf( stdout, "\n      cpsStat:(mask:preStatus:status:level)(%d:%d:%d:%d)", 
				sfdb->sys[i].commInfo.cpsOverSts.mask,
				sfdb->sys[i].commInfo.cpsOverSts.preStatus,
				sfdb->sys[i].commInfo.cpsOverSts.status,
				sfdb->sys[i].commInfo.cpsOverSts.level
				);
		}

		/* callDATA-cps */
		fprintf( stdout, "\n      cpsValue:(logon:logout)(%d:%d)", 
				sfdb->callData.cps.uiLogOnSumCps, sfdb->callData.cps.uiLogOutSumCps);
		/* callDATA-tps */
		for( j=0;j<2;j++ ){
		fprintf( stdout, "\n      tpsValue[%d]:(mask:num:level)(%d:%u:%d)", 
				j,
				sfdb->callData.tpsInfo[j].mask,
				sfdb->callData.tpsInfo[j].num,
				sfdb->callData.tpsInfo[j].level);
		}
		/* callDATA-sess */
		for( j=0;j<2;j++ ){
		fprintf( stdout, "\n      legInfo(sess)[%d]:(mask:num:level)(%d:%u:%d)", 
				j,
				sfdb->callData.legInfo[j].mask,
				sfdb->callData.legInfo[j].num,
				sfdb->callData.legInfo[j].level);
		}
#if 0
		// hw info
		fprintf(stdout,"\n   hw(%d) Stat:(name:mask:preStat:stat:level)\n",i);
		for(j=0; j<27;j++)
		{
			if(!(j%3)&&j>2) printf("\n");
			if(strlen(sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].name))
			{
				fprintf(stdout,"\t(%8s:%d:%d:%d:%d)",
					sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].name,
					sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].mask,
					sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].prevStatus,
					sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].status,
					sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].level
				);
			}
		}

		/** SMLINK */
		fprintf(stdout,"\n	SMChannel Info (mask:status:preStatus:level)\n");
		for(j=0;j < 1;j++){
			fprintf(stdout,"\tCh%d(%d:%d:%d:%d)\n",
			j,
			sfdb->sys[i].commInfo.smChSts.each[j].mask,
			sfdb->sys[i].commInfo.smChSts.each[j].status,
			sfdb->sys[i].commInfo.smChSts.each[j].preStatus,
			sfdb->sys[i].commInfo.smChSts.each[j].level);
		}
		fprintf(stdout,"\t SM (Level)(LMT, min:maj:cri)(Flag, min:maj:cri)\n");
		fprintf(stdout,"\t    (%d)(%d:%d:%d)(%d:%d:%d)\n",
			sfdb->sys[i].commInfo.smChSts.level,
			sfdb->sys[i].commInfo.smChSts.minLimit,
			sfdb->sys[i].commInfo.smChSts.majLimit,
			sfdb->sys[i].commInfo.smChSts.criLimit,
			sfdb->sys[i].commInfo.smChSts.minFlag,
			sfdb->sys[i].commInfo.smChSts.majFlag,
			sfdb->sys[i].commInfo.smChSts.criFlag);
#endif
	}

	fprintf(stdout,"\n--------------------------------------------------------------------------------------------------\n");

	return;
}//----- End of printSysInfo -----//

void printL3pdInfo() {
	int	i, j;
	char devName[2][6];
	strcpy(devName[0], "TAP_A");
	strcpy(devName[1], "TAP_B");
	fprintf(stdout, "\n PROBING DEVICE STATUS\n\n");
	
	for(i=0; i<2; i++){
		fprintf(stdout, "%s\n", devName[i]);
		fprintf(stdout, "TAP_CPU=> (mask:usage:level:minLimit:majLimit:criLimit:minDurat:majDurat:criDurat) \n"
				"%d : %d : %d : %d : %d : %d : %d : %d : %d\n\n"
				,l3pd->l3ProbeDev[i].cpuInfo.mask, l3pd->l3ProbeDev[i].cpuInfo.usage, l3pd->l3ProbeDev[i].cpuInfo.level
				,l3pd->l3ProbeDev[i].cpuInfo.minLimit, l3pd->l3ProbeDev[i].cpuInfo.majLimit, l3pd->l3ProbeDev[i].cpuInfo.criLimit
				,l3pd->l3ProbeDev[i].cpuInfo.minDurat, l3pd->l3ProbeDev[i].cpuInfo.majDurat, l3pd->l3ProbeDev[i].cpuInfo.criDurat);
	
		fprintf(stdout, "TAP_MEM=> (mask:usage:level:minLimit:majLimit:criLimit:minDurat:majDurat:criDurat) \n"
				"%d : %d : %d : %d : %d : %d : %d : %d : %d\n\n"
				,l3pd->l3ProbeDev[i].memInfo.mask, l3pd->l3ProbeDev[i].memInfo.usage, l3pd->l3ProbeDev[i].memInfo.level
				,l3pd->l3ProbeDev[i].memInfo.minLimit, l3pd->l3ProbeDev[i].memInfo.majLimit, l3pd->l3ProbeDev[i].memInfo.criLimit
				,l3pd->l3ProbeDev[i].memInfo.minDurat, l3pd->l3ProbeDev[i].memInfo.majDurat, l3pd->l3ProbeDev[i].memInfo.criDurat);
		
		for(j=0; j< MAX_GIGA_LAN_NUM; j++){
			fprintf(stdout, "TAP_PORT(%d)=> (mask:status:prevStatus:level)"
					"%d : %d : %d : %d\n"
					,j, l3pd->l3ProbeDev[i].gigaLanInfo[j].mask, l3pd->l3ProbeDev[i].gigaLanInfo[j].status
					,l3pd->l3ProbeDev[i].gigaLanInfo[j].prevStatus, l3pd->l3ProbeDev[i].gigaLanInfo[j].level);

		}
		for(j=0; j< MAX_POWER_NUM; j++){ // 20110422 by dcham
			fprintf(stdout, "TAP_POWER(%d)=> (mask:status:prevStatus:level)"
					"%d : %d : %d : %d\n"
					,j, l3pd->l3ProbeDev[i].powerInfo[j].mask, l3pd->l3ProbeDev[i].powerInfo[j].status
					,l3pd->l3ProbeDev[i].powerInfo[j].prevStatus, l3pd->l3ProbeDev[i].powerInfo[j].level);
		}
	}
}
//  sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j]
void printSpecInfo() {
	int 	i, j;

	fprintf(stdout, "\n DSC H/W STATUS\n\n");
	
	for(i=1; i<3; i++){
		fprintf(stdout, "\n %s \n", sfdb->sys[i].commInfo.name);
		//for(j=0; j < SFM_MAX_HPUX_HW_COM; j++){
		for(j=0; j < 20; j++){
			fprintf(stdout, "NAME: %s (mask:status:prevstatus:level)"
				" %d : %d : %d : %d\n"
				, sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].name
				, sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].mask
				, sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].status
				, sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].prevStatus
				, sfdb->sys[i].specInfo.u.sms.hpuxHWInfo.hwcom[j].level);
		}
	}

}

void printSCEinfo() {
	int 	i, j;

	fprintf(stdout, "\n SCE STATUS\n\n");
	
	for(i=0; i<MAX_SCE_DEV_NUM; i++){
		if(i==0)
			fprintf(stdout, "\n %s \n", "SCEA");
		else
			fprintf(stdout, "\n %s \n", "SCEB");

		for(j=0; j < MAX_SCE_IFN_CNT; j++){
			fprintf(stdout, "NAME: LINK(%d:%d) (mask:status:prevstatus:level)"
				" %d : %d : %d : %d\n"
				, i, j+1
				, g_pstSCEInfo->SCEDev[i].portStatus[j].mask
				, g_pstSCEInfo->SCEDev[i].portStatus[j].status
				, g_pstSCEInfo->SCEDev[i].portStatus[j].preStatus
				, g_pstSCEInfo->SCEDev[i].portStatus[j].level);
		}

		for(j=0; j < MAX_SCE_CPU_CNT; j++){
			fprintf(stdout, "NAME: CPU(%d:%d) (mask:usage:level)"
				" %d : %d : %d \n"
				, i, j+1
				, g_pstSCEInfo->SCEDev[i].cpuInfo[j].mask
				, g_pstSCEInfo->SCEDev[i].cpuInfo[j].usage
				, g_pstSCEInfo->SCEDev[i].cpuInfo[j].level);
		}

		fprintf(stdout, "NAME: DISK(%d:%d) (mask:usage:level)"
				" %d : %d : %d  \n"
				, i, j+1
				, g_pstSCEInfo->SCEDev[i].diskInfo.mask
				, g_pstSCEInfo->SCEDev[i].diskInfo.usage
				, g_pstSCEInfo->SCEDev[i].diskInfo.level);

		/* hjjung_20100823 */
		fprintf(stdout, "NAME: USER(%d:%d) (mask:num:level)"
				" %d : %d : %d  \n"
				, i, j+1
				, g_pstSCEInfo->SCEDev[i].userInfo.mask
				, g_pstSCEInfo->SCEDev[i].userInfo.num
				, g_pstSCEInfo->SCEDev[i].userInfo.level);

		for(j=0; j < MAX_SCE_MEM_CNT; j++){
			fprintf(stdout, "NAME: MEM(%d:%d) (mask:usage:level)"
				" %d : %d : %d \n"
				, i,j+1
				, g_pstSCEInfo->SCEDev[i].memInfo[j].mask
				, g_pstSCEInfo->SCEDev[i].memInfo[j].usage
				, g_pstSCEInfo->SCEDev[i].memInfo[j].level);
		}

		fprintf(stdout, "NAME: STATUS(%d) (mask:status:prevstatus:level)"
				" %d : %d : %d : %d\n"
				, i 
				, g_pstSCEInfo->SCEDev[i].sysStatus.mask
				, g_pstSCEInfo->SCEDev[i].sysStatus.status
				, g_pstSCEInfo->SCEDev[i].sysStatus.preStatus
				, g_pstSCEInfo->SCEDev[i].sysStatus.level);

		fprintf(stdout, "NAME: POWER(%d) (mask:status:prevstatus:level) "
				" %d : %d : %d : %d\n"
				, i 
				, g_pstSCEInfo->SCEDev[i].pwrStatus.mask
				, g_pstSCEInfo->SCEDev[i].pwrStatus.status
				, g_pstSCEInfo->SCEDev[i].pwrStatus.preStatus
				, g_pstSCEInfo->SCEDev[i].pwrStatus.level);

		fprintf(stdout, "NAME: FAN(%d) (mask:status:prevstatus:level)   "
				" %d : %d : %d : %d\n"
				, i 
				, g_pstSCEInfo->SCEDev[i].fanStatus.mask
				, g_pstSCEInfo->SCEDev[i].fanStatus.status
				, g_pstSCEInfo->SCEDev[i].fanStatus.preStatus
				, g_pstSCEInfo->SCEDev[i].fanStatus.level);

		fprintf(stdout, "NAME: TEMP(%d) (mask:status:prevstatus:level)  "
				" %d : %d : %d : %d\n"
				, i 
				, g_pstSCEInfo->SCEDev[i].tempStatus.mask
				, g_pstSCEInfo->SCEDev[i].tempStatus.status
				, g_pstSCEInfo->SCEDev[i].tempStatus.preStatus
				, g_pstSCEInfo->SCEDev[i].tempStatus.level);

		for(j=0; j<MAX_SCE_RDR_INFO_CNT;j++){
			fprintf(stdout, "NAME: RDR_CONN(%d) (mask:status:prevstatus:level)"
					" %d : %d : %d : %d\n"
					, i 
					, g_pstSCEInfo->SCEDev[i].rdrConnStatus[j].mask
					, g_pstSCEInfo->SCEDev[i].rdrConnStatus[j].status
					, g_pstSCEInfo->SCEDev[i].rdrConnStatus[j].preStatus
					, g_pstSCEInfo->SCEDev[i].rdrConnStatus[j].level);
		}

		fprintf(stdout, "NAME: VERSION (%s)\n" , g_pstSCEInfo->SCEDev[i].sysInfo.version);
	}

}

/* hjjung */
void printLEGinfo() {
	int 	i, j;

	fprintf(stdout, "\n RLEG STATUS\n\n");
	
	for(i=0; i<MAX_CALL_DEV_NUM; i++){
		if(i==0)
			fprintf(stdout, "\n %s \n", "RLEGA");
		else
			fprintf(stdout, "\n %s \n", "RLEGB");

		fprintf(stdout, "NAME: SESSION(%d:%d) (mask:num:level)"
				" %d : %d : %d \n"
				, i, j+1
				, g_pstCALLInfo->legInfo[i].mask
				, g_pstCALLInfo->legInfo[i].num
				, g_pstCALLInfo->legInfo[i].level);
	}
}

/* added by dcham 20110525 for TPS */
void printTPSinfo() {
	int 	i, j;

	fprintf(stdout, "\n TPS STATUS\n\n");
	
	for(i=0; i<MAX_CALL_DEV_NUM; i++){
		if(i==0)
			fprintf(stdout, "\n %s \n", "TPSA");
		else
			fprintf(stdout, "\n %s \n", "TPSB");

		fprintf(stdout, "NAME: TPS(%d:%d) (mask:num:level)"
				" %d : %d : %d \n"
				, i, j+1
				, g_pstCALLInfo->tpsInfo[i].mask
				, g_pstCALLInfo->tpsInfo[i].num
				, g_pstCALLInfo->tpsInfo[i].level);
	}
}

void printL2Info()
{
    int     i, sysType;

    fprintf(stdout, "\n L2 Switch H/W STATUS\n\n");
   
	for(sysType =0; sysType<2; sysType++){ 
		for(i=0; i<24; i++){
			fprintf(stdout, "NAME: PORT-%02d(%d:%d:%d:%d) (mask:prests:status:level)\n",
					i+1,
					g_pstL2Dev->l2Info[sysType].portInfo[i].mask,
					g_pstL2Dev->l2Info[sysType].portInfo[i].prevStatus,
					g_pstL2Dev->l2Info[sysType].portInfo[i].status, 
					g_pstL2Dev->l2Info[sysType].portInfo[i].level);

		}

		fprintf(stdout, "NAME: CPU(%d:%d:%d) (mask:usage:level)\n",
				g_pstL2Dev->l2Info[sysType].cpuInfo.mask,
				g_pstL2Dev->l2Info[sysType].cpuInfo.usage, 
				g_pstL2Dev->l2Info[sysType].cpuInfo.level);

		fprintf(stdout, "NAME: MEM(%d:%d:%d) (mask:usage:level)\n",
				g_pstL2Dev->l2Info[sysType].memInfo.mask,
				g_pstL2Dev->l2Info[sysType].memInfo.usage, 
				g_pstL2Dev->l2Info[sysType].memInfo.level);
		fprintf(stdout,"================================================================\n");
	}	

}/*End of printL2Info*/

char *int2dot (int ipaddr)
{
    static char dot_buf[20];
    struct  in_addr n_info;
    
    memset (dot_buf, 0, 20);
    
    n_info.s_addr = ipaddr;
    sprintf (dot_buf, "%s", inet_ntoa (n_info));
            
    return dot_buf;
}           


// NMSIF sfdb info
void print_NMSIF_Info()
{
	int i=0;

	for(i = 0; i < MAX_NMS_CON; i++)
	{
	//	if(sfdb->nmsInfo.fd[i]<0)
	//		continue;
		fprintf(stdout,"NMS Client(i:%d)%d:%d:%d:%d:(%s) (mask:fd:level:port:ipddr)\n",
				i,
				sfdb->nmsInfo.mask[i],
				sfdb->nmsInfo.fd[i],
				sfdb->nmsInfo.level[i],
				sfdb->nmsInfo.port[i],
				int2dot(sfdb->nmsInfo.ipaddr[i])
				);

	}
	fprintf(stdout,"===============================\n");

}/* print_NMSIF_Info() */
