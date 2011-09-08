/**
 *	Include headers
 */
#include <unistd.h>			/*	getpid(2), sleep(3)	*/
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/types.h>		/*	getpid(2)	*/
#include <sys/ipc.h>
#include <sys/socket.h>		/*	inet_addr(3)	*/
#include <netinet/in.h>		/*	inet_addr(3)	*/
#include <arpa/inet.h>		/*	inet_addr(3)	*/

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

// LIB
#include "loglib.h"
#include "verlib.h"

// DQMS
#include "typedef.h"
#include "procid.h"
#include "path.h"
#include "sshmid.h"

// OAM
#include "filedb.h"

// .
#include "snmpif.h"
#include "prTable.h"
#include "memTable.h"
#include "dskTable.h"
#include "cpuTable.h"


/**
 *	Define constants
 */
#define TARGET					"DQMS"

#define MAX_BLOCK_COUNT			48
#define MAX_BLOCK_LENGTH		16

#define THREAD_CHECK_INTERVAL	5

#define cpuTAM				fidb->cpusts
#define memTAM				fidb->memsts
#define queTAM				fidb->queuests
#define nifoTAM				fidb->nifosts
#define dskTAM(d)			fidb->disksts[d]
#define ethTAM(d)			(fidb->link[d])
#define chnTAM(d)			(fidb->NTAFChnl[d])
#define	InterLockChnTAM(d)	(fidb->cInterlock[d])
#define ntpTAM(d)			(fidb->hwNTP[d])
#define fanTAM(d)			(fidb->hwFan[d])
#define	pwrTAM(d)			(fidb->hwPWR[d])
#define prTAM(d)			(fidb->mpsw[d])
#define piTAM(d)			(fidb->mpswinfo[d])

#define cpuTAF(d)			(sfdb->stNTAF[d]).cpusts
#define cemTAF(d)			(sfdb->stNTAF[d]).memsts
#define memTAF(d)			(sfdb->stNTAF[d]).memsts
#define queTAF(d)			(sfdb->stNTAF[d]).quests
#define nifoTAF(d)			(sfdb->stNTAF[d]).nifosts
#define dskTAF(d,x)			(sfdb->stNTAF[d]).disksts[x]
#define ethTAF(d,x)			(sfdb->stNTAF[d].link[x])
#define edTAF(d,x)			(sfdb->stNTAF[d].hwport[x])
#define ntpTAF(d,x)			(sfdb->stNTAF[d].hwntp[x])
#define fanTAF(d,x)			(sfdb->stNTAF[d].hwfan[x])
#define pwrTAF(d,x)			(sfdb->stNTAF[d].hwpwr[x])
#define prTAF(d,x)			(sfdb->stNTAF[d].mpsw[x])
#define piTAF(d,x)			(sfdb->stNTAF[d].mpswinfo[x])


/**
 *	Declare variables
 */
char				szVersion[7] = "R4.0.0";

pthread_t			t_agntx;
static int			gJiSTOPFlag;
int					dFinishSig;
int					dThreadStop;
st_NTAF_List_SHM	*sfdb;
st_SubSysList		stSubSysList;

char				SWLST_TAM_APP[MAX_BLOCK_COUNT][MAX_BLOCK_LENGTH];
char				SWLST_TAF_RP[MAX_BLOCK_COUNT][MAX_BLOCK_LENGTH];
char				SWLST_TAF_PI[MAX_BLOCK_COUNT][MAX_BLOCK_LENGTH];

pst_WNTAM			stWNTAM;
pst_NTAM			fidb;


/**
 *	Declare functions
 */
extern void put_cpuTable(int idx, char* name, int usage);
extern void put_memTable(int idx, char* name, int total, int avail, int used, int usage);
extern void put_dskTable(int idx, char* name, char *path, int total, int avail, int used, int usage);
extern void put_nifoTable(int idx, char* name, int total, int avail, int used, int usage);
extern void put_prTable(int idx, char* name, char* pname, int pid, int uptime, int status);
extern void put_ethTable(int idx, char* name, int port, int status);
extern void put_chnTable(int idx, char* name, int port, int status);
extern void put_edTable(int idx, char* name, int port, int status);
extern void put_queTable(int idx, char* name, int total, int avail, int used, int usage);
extern void put_pwrTable(int idx, char* name, int id, int status);
extern void put_fanTable(int idx, char* name, int id, int status);
extern void put_ntpTable(int idx, char* name, int id, int status);

extern void init_chnTable(void);
extern void init_edTable(void);
extern void init_ethTable(void);
extern void init_fanTable(void);
extern void init_ntpTable(void);
extern void init_pwrTable(void);
extern void init_queTable(void);

extern void init_vacm_vars(void);
extern void init_usmUser(void);

extern int send_start_trap(void);
extern int send_stop_trap(void);

//extern int Init_Fidb(void);
extern int	dInitLogShm(void);

int	init_sfdb(void);
int	dGetBlocks(char *fn, char (*p)[MAX_BLOCK_LENGTH]);
int dReadSubSysListFromFile(st_SubSysList *pstList);
void *p_agntx(void *args);
void SetUpSignal(void);
void UserControlledSignal(int sign);
void IgnoreSignal(int sign);
int dInitFidb(void);


/**
 *	Implement functions
 */


int main(int argc, char *argv[])
{
	int		agentx_subagent;	/* change this if you want to be a SNMP master agent */
	int		background;			/* change this if you want to run in the background */
	int		syslog;				/* change this if you want to use syslog */
	int		dRet, dOldThreadCount;
	time_t	tOldThreadCheck, tThreadCheck;

	agentx_subagent	= 1;
	background		= 0;
	syslog			= 1;

	// Initialize Log
	dRet = log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_SNMPIF, LOG_PATH"/SNMPIF", "SNMPIF");
	if(dRet < 0)
	{
		log_print(LOGN_WARN, LH"MAIN : Failed in Initialize LOGLIB Info [%d]", LT,  dRet);
		return -1;
	}

#if 0	
	if( (dRet = Init_shm_common()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN Init_shm_common() [errno:%d-%s]", LT, -dRet, strerror(-dRet));
		return -2;
	}
#endif

	// Set version
	if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_SNMPIF, szVersion)) < 0)
	{
        log_print(LOGN_WARN, LH"ERROR IN set_version() dRet[%d]", LT, dRet);
		return -3;
    }

    if( (dRet = dGetBlocks(FILE_MC_INIT_M_PRI, SWLST_TAM_APP)) < 0)
    {
        log_print(LOGN_CRI, LH"ERROR IN dGetBlocks(%s) dRet[%d]", LT, FILE_MC_INIT_M_PRI, dRet);
		return -4;
    }

    if( (dRet = dGetBlocks(FILE_MC_INIT_F_PRI, SWLST_TAF_RP)) < 0)
    {
        log_print(LOGN_CRI, LH"ERROR IN dGetBlocks(%s) dRet[%d]", LT, FILE_MC_INIT_F_PRI, dRet);
		return -5;
    }

    if( (dRet = dGetBlocks(FILE_MC_INIT_F_SEC, SWLST_TAF_PI)) < 0)
    {
        log_print(LOGN_CRI, LH"ERROR IN dGetBlocks(%s) dRet[%d]", LT, FILE_MC_INIT_F_SEC, dRet);
		return -6;
    }

	if( (dRet = dReadSubSysListFromFile(&stSubSysList)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dReadSubSysListFromFile() dRet[%d]", LT, dRet);
		return -7;
	}

	putenv("MIBS=ALL");

	/* print log errors to syslog or stderr */
	if(syslog)
		snmp_enable_calllog();
	else
		snmp_enable_stderrlog();

	/* we're an agentx subagent? */
	if(agentx_subagent)
	{
		/* make us a agentx client. */
		netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);
	}

	/* run in background, if requested */
	if(background && netsnmp_daemonize(1, !syslog))
		exit(-1);

	/* initialize tcpip, if necessary */
	SOCK_STARTUP;

	/* initialize the agent library */
	init_agent(TARGET);

	/* initialize mib code here */

	/* mib code: init_hardware from hardware.C */
	init_chnTable();
	init_dskTable();
	init_edTable();
	init_ethTable();
	init_fanTable();
	init_memTable();
	init_ntpTable();
	init_prTable();
	init_pwrTable();
	init_queTable();
	init_cpuTable();
	init_nifoTable();


	/* initialize vacm/usm access control  */
	if(!agentx_subagent)
	{
		init_vacm_vars();
		init_usmUser();
	}

	/* Initialize fidb */
	if((dRet = dInitFidb()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dInitFidb() dRet=%d", LT, dRet);
		exit(-4);
	}

	/* Initialize sfdb */
	if( (dRet = init_sfdb()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN init_sfdb() dRet[%d]", LT, dRet);
		exit(-5);
	}

	if( (dRet = pthread_create(&t_agntx, NULL, p_agntx, NULL)) != 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN pthread_create(t_agntx[%lu]) dRet[%d] errno[%d-%s]", LT,
			t_agntx, dRet, errno, strerror(errno));
		exit(-6);
	}
	else if( (dRet = pthread_detach(t_agntx)) != 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN pthread_detach(t_agntx[%lu]) dRet[%d] errno[%d-%s]", LT,
			t_agntx, dRet, errno, strerror(errno));
		exit(-7);
	}

	tOldThreadCheck = time(NULL);
	dOldThreadCount	= dThreadStop;

	/* In case we recevie a request to stop (kill -TERM or kill -INT) */
	gJiSTOPFlag = 1;
	SetUpSignal();
	send_start_trap();

	log_print(LOGN_CRI, "SNMPIF[%s] START", szVersion);
	while(gJiSTOPFlag)
	{
		if( ((tThreadCheck = time(NULL)) - tOldThreadCheck) > THREAD_CHECK_INTERVAL)
		{
			tOldThreadCheck = tThreadCheck;
			if(dThreadStop == dOldThreadCount)
			{
				log_print(LOGN_CRI, LH"STOPED thread(p_agntx) dThreadStop[%d] dOldThreadCount[%d]", LT,
					dThreadStop, dOldThreadCount);
				if( (dRet = pthread_create(&t_agntx, NULL, p_agntx, NULL)) != 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN pthread_create(t_agntx[%lu]) dRet[%d] errno[%d-%s]", LT,
						t_agntx, dRet, errno, strerror(errno));
					exit(-7);
				}
				else
				{
					log_print(LOGN_CRI, LH"SUCCESS IN pthread_create(t_agntx[%lu])", LT, t_agntx);
					dThreadStop		= 0;
				}
			}
			log_print(LOGN_DEBUG, LH"dThreadStop[%d] dOldThreadCount[%d]", LT, dThreadStop, dOldThreadCount);
			dOldThreadCount = dThreadStop;
		}

		/* if you use select(), see snmp_select_info() in snmp_api(3) */
		/*     --- OR ---  */
		alarm(1);
		agent_check_and_process(1); /* 0 == don't block */
		alarm(0);
	}

	/* at shutdown time */
	send_stop_trap();
	snmp_shutdown("SNMPIF");
	SOCK_CLEANUP;
	log_print(LOGN_CRI, "SNMPIF[%s] END", szVersion);

	return 0;
}

void* p_agntx(void* args)
{
	char	sSysName[BUFSIZ], sBlockName[MAX_BLOCK_LENGTH], sNTPSysName[BUFSIZ];
	int		i, j, dProcIdx, dCPUIdx, dMemoryIdx, dQueueIdx, dNifoIdx, dDiskIdx, dEthernetIdx, dNTAFChnIdx, dPowerIdx, dFanIdx, dNTPIdx, dEndaceIdx;

	while(gJiSTOPFlag)
	{
		dProcIdx			= 1;
		dCPUIdx				= 1;
		dMemoryIdx			= 1;
		dQueueIdx			= 1;
		dNifoIdx			= 1;
		dDiskIdx			= 1;
		dEthernetIdx		= 1;
		dNTAFChnIdx			= 1;
		dPowerIdx			= 1;
		dFanIdx				= 1;
		dNTPIdx				= 1;
		dEndaceIdx			= 1;

		/*	TAM_APP	*/
		/*	PROCESS	*/
		for(i = 0; i < MAX_BLOCK_COUNT; i++)
		{
			if(strlen(SWLST_TAM_APP[i]) == 0)
				break;
			else
				strncpy(sBlockName, SWLST_TAM_APP[i], MAX_BLOCK_LENGTH);
			put_prTable(dProcIdx++, "TAM_APP", sBlockName, piTAM(i).pid, piTAM(i).when, prTAM(i));
		}

		/*	CPU		*/
		put_cpuTable(dCPUIdx++, "TAM_APP", cpuTAM.llCur);

		/*	MEMORY	*/
		put_memTable(dMemoryIdx++, "TAM_APP", memTAM.lMax, memTAM.lMax-memTAM.llCur, memTAM.llCur, ((memTAM.lMax)!=0)?(int)((((float)memTAM.llCur)/((float)memTAM.lMax))*100):0);

		/*	QUEUE		*/
		put_queTable(dQueueIdx++, "TAM_APP", queTAM.lMax, (queTAM.lMax-queTAM.llCur), queTAM.llCur, ((queTAM.lMax)!=0)?(int)((((float)queTAM.llCur)/((float)queTAM.lMax))*100):0);

		/*	NIFO		*/
		put_nifoTable(dNifoIdx++, "TAM_APP", nifoTAM.lMax, (nifoTAM.lMax-nifoTAM.llCur), nifoTAM.llCur, ((nifoTAM.lMax)!=0)?(int)((((float)nifoTAM.llCur)/((float)nifoTAM.lMax))*100):0);

		/*	DISK	*/
		put_dskTable(dDiskIdx++, "TAM_APP", "/", dskTAM(0).lMax, (dskTAM(0).lMax-dskTAM(0).llCur), dskTAM(0).llCur, (dskTAM(0).lMax!=0)?(int)(((float)dskTAM(0).llCur/(float)dskTAM(0).lMax)*100):0);
		put_dskTable(dDiskIdx++, "TAM_APP", "/DATA ", dskTAM(1).lMax, (dskTAM(1).lMax-dskTAM(1).llCur), dskTAM(1).llCur, (dskTAM(1).lMax!=0)?(int)(((float)dskTAM(1).llCur/(float)dskTAM(1).lMax)*100):0);
		put_dskTable(dDiskIdx++, "TAM_APP", "/LOG", dskTAM(2).lMax, (dskTAM(2).lMax-dskTAM(2).llCur), dskTAM(2).llCur, (dskTAM(2).lMax!=0)?(int)(((float)dskTAM(2).llCur/(float)dskTAM(2).lMax)*100):0);
		put_dskTable(dDiskIdx++, "TAM_APP", "/BACKUP", dskTAM(3).lMax, (dskTAM(3).lMax-dskTAM(3).llCur), dskTAM(3).llCur, (dskTAM(3).lMax!=0)?(int)(((float)dskTAM(3).llCur/(float)dskTAM(3).lMax)*100):0);

		/*	ETHERNET	*/
		put_ethTable(dEthernetIdx++, "TAM_APP", 1, ethTAM(0));
		put_ethTable(dEthernetIdx++, "TAM_APP", 2, ethTAM(1));
		put_ethTable(dEthernetIdx++, "TAM_APP", 3, ethTAM(2));
		put_ethTable(dEthernetIdx++, "TAM_APP", 4, ethTAM(3));

		/*	NTAF CHANNEL		*/
		put_chnTable(dNTAFChnIdx++, "TAM_APP", 1, chnTAM(0));
		put_chnTable(dNTAFChnIdx++, "TAM_APP", 2, chnTAM(1));
		put_chnTable(dNTAFChnIdx++, "TAM_APP", 3, chnTAM(2));
		put_chnTable(dNTAFChnIdx++, "TAM_APP", 4, chnTAM(3));
		put_chnTable(dNTAFChnIdx++, "TAM_APP", 5, chnTAM(4));
		put_chnTable(dNTAFChnIdx++, "TAM_APP", 6, chnTAM(5));
		put_chnTable(dNTAFChnIdx++, "TAM_APP", 7, chnTAM(6));
		put_chnTable(dNTAFChnIdx++, "TAM_APP", 8, chnTAM(7));

		/*	INTERLOCK CHANNEL	*/
		put_chnTable(dNTAFChnIdx++, "TAM_APP", 1, InterLockChnTAM(0));
		put_chnTable(dNTAFChnIdx++, "TAM_APP", 2, InterLockChnTAM(1));
		put_chnTable(dNTAFChnIdx++, "TAM_APP", 3, InterLockChnTAM(2));
		put_chnTable(dNTAFChnIdx++, "TAM_APP", 4, InterLockChnTAM(3));

		/*	POWER		*/
		put_pwrTable(dPowerIdx++, "TAM_APP", 1, pwrTAM(0));
		put_pwrTable(dPowerIdx++, "TAM_APP", 2, pwrTAM(1));
		put_pwrTable(dPowerIdx++, "TAM_APP", 3, pwrTAM(2));
		put_pwrTable(dPowerIdx++, "TAM_APP", 4, pwrTAM(3));

		/*	FAN			*/
		put_fanTable(dFanIdx++, "TAM_APP", 1, fanTAM(0));
		put_fanTable(dFanIdx++, "TAM_APP", 2, fanTAM(1));
		put_fanTable(dFanIdx++, "TAM_APP", 3, fanTAM(2));
		put_fanTable(dFanIdx++, "TAM_APP", 4, fanTAM(3));
		put_fanTable(dFanIdx++, "TAM_APP", 5, fanTAM(4));
		put_fanTable(dFanIdx++, "TAM_APP", 6, fanTAM(5));

		/*	NTP			*/
		put_ntpTable(dNTPIdx++, "TAM_APP_DAEMON", 1, ntpTAM(0));
		put_ntpTable(dNTPIdx++, "TAM_APP_CHANNEL", 2, ntpTAM(1));

		/*	TAF	*/
		for(i = 0; i < stSubSysList.dCount; i++)
		{
			if(stSubSysList.stSubSys[i].dType == 1)		/*	RP_TAF	*/
				sprintf(sSysName, "%s_TAF%d", "RP", stSubSysList.stSubSys[i].dNo);
			else if(stSubSysList.stSubSys[i].dType == 2)	/*	PI_TAF	*/
				sprintf(sSysName, "%s_TAF%d", "PI", stSubSysList.stSubSys[i].dNo);

			/*	PROCESS	*/
			for(j = 0; j < MAX_BLOCK_COUNT; j++)
			{
				if(stSubSysList.stSubSys[i].dType == 1)		/*	RP_TAF	*/
				{
					if(strlen(SWLST_TAF_RP[j]) == 0)
						break;
					else
						strncpy(sBlockName, SWLST_TAF_RP[j], MAX_BLOCK_LENGTH);
				}
				else if(stSubSysList.stSubSys[i].dType == 2)	/*	PI_TAF	*/
				{
					if(strlen(SWLST_TAF_PI[j]) == 0)
						break;
					else
						strncpy(sBlockName, SWLST_TAF_PI[j], MAX_BLOCK_LENGTH);
				}

				put_prTable(dProcIdx++, sSysName, sBlockName, piTAF(stSubSysList.stSubSys[i].dNo-1,j).pid, piTAF(stSubSysList.stSubSys[i].dNo-1,j).when, prTAF(stSubSysList.stSubSys[i].dNo-1,j));
			}

			/*	CPU		*/
			put_cpuTable(dCPUIdx++, sSysName, cpuTAF(stSubSysList.stSubSys[i].dNo-1).llCur);

			/*	Memory	*/
			put_memTable(dMemoryIdx++, sSysName, memTAF(stSubSysList.stSubSys[i].dNo-1).lMax,
				(memTAF(stSubSysList.stSubSys[i].dNo-1).lMax-memTAF(stSubSysList.stSubSys[i].dNo-1).llCur),
				memTAF(stSubSysList.stSubSys[i].dNo-1).llCur,
				(memTAF(stSubSysList.stSubSys[i].dNo-1).lMax!=0)?(int)(((float)(memTAF(stSubSysList.stSubSys[i].dNo-1).llCur)/(float)(memTAF(stSubSysList.stSubSys[i].dNo-1).lMax))*100):0);

			/*	Disk	*/
			put_dskTable(dDiskIdx++, sSysName, "/", dskTAF(stSubSysList.stSubSys[i].dNo-1,0).lMax,
				((dskTAF(stSubSysList.stSubSys[i].dNo-1,0).lMax)-(dskTAF(stSubSysList.stSubSys[i].dNo-1,0).llCur)),
				dskTAF(stSubSysList.stSubSys[i].dNo-1,0).llCur,
				((dskTAF(stSubSysList.stSubSys[i].dNo-1,0).lMax)!=0)?(int)(((float)(dskTAF(stSubSysList.stSubSys[i].dNo-1,0).llCur)/(float)(dskTAF(stSubSysList.stSubSys[i].dNo-1,0).lMax))*100):0);
			put_dskTable(dDiskIdx++, sSysName, "/DATA", dskTAF(stSubSysList.stSubSys[i].dNo-1,1).lMax,
				((dskTAF(stSubSysList.stSubSys[i].dNo-1,1).lMax)-(dskTAF(stSubSysList.stSubSys[i].dNo-1,1).llCur)),
				dskTAF(stSubSysList.stSubSys[i].dNo-1,1).llCur,
				((dskTAF(stSubSysList.stSubSys[i].dNo-1,1).lMax)!=0)?(int)(((float)(dskTAF(stSubSysList.stSubSys[i].dNo-1,1).llCur)/(float)(dskTAF(stSubSysList.stSubSys[i].dNo-1,1).lMax))*100):0);

			/*	Ethernet	*/
			put_ethTable(dEthernetIdx++, sSysName, 1, ethTAF(stSubSysList.stSubSys[i].dNo-1,0));
			put_ethTable(dEthernetIdx++, sSysName, 2, ethTAF(stSubSysList.stSubSys[i].dNo-1,1));

			/*	Endace		*/
			put_edTable(dEndaceIdx++, sSysName, 1, edTAF(stSubSysList.stSubSys[i].dNo-1,0));
			put_edTable(dEndaceIdx++, sSysName, 2, edTAF(stSubSysList.stSubSys[i].dNo-1,1));

			/*	Queue		*/
			put_queTable(dQueueIdx++, sSysName, queTAF(stSubSysList.stSubSys[i].dNo-1).lMax,
				(queTAF(stSubSysList.stSubSys[i].dNo-1).lMax-queTAF(stSubSysList.stSubSys[i].dNo-1).llCur),
				queTAF(stSubSysList.stSubSys[i].dNo-1).llCur,
				((queTAF(stSubSysList.stSubSys[i].dNo-1).lMax)!=0)?(int)(((float)(queTAF(stSubSysList.stSubSys[i].dNo-1).llCur)/(float)(queTAF(stSubSysList.stSubSys[i].dNo-1).lMax))*100):0);

			/*	NIFO		*/
			put_nifoTable(dNifoIdx++, sSysName, nifoTAF(stSubSysList.stSubSys[i].dNo-1).lMax,
				(nifoTAF(stSubSysList.stSubSys[i].dNo-1).lMax-nifoTAF(stSubSysList.stSubSys[i].dNo-1).llCur),
				nifoTAF(stSubSysList.stSubSys[i].dNo-1).llCur,
				((nifoTAF(stSubSysList.stSubSys[i].dNo-1).lMax)!=0)?(int)(((float)(nifoTAF(stSubSysList.stSubSys[i].dNo-1).llCur)/(float)(nifoTAF(stSubSysList.stSubSys[i].dNo-1).lMax))*100):0);

			/*	Power		*/
			put_pwrTable(dPowerIdx++, sSysName, 1, pwrTAF(stSubSysList.stSubSys[i].dNo-1,0));
			put_pwrTable(dPowerIdx++, sSysName, 2, pwrTAF(stSubSysList.stSubSys[i].dNo-1,1));

			/*	FAN			*/
			put_fanTable(dFanIdx++, sSysName, 1, fanTAF(stSubSysList.stSubSys[i].dNo-1,0));
			put_fanTable(dFanIdx++, sSysName, 2, fanTAF(stSubSysList.stSubSys[i].dNo-1,1));
			put_fanTable(dFanIdx++, sSysName, 3, fanTAF(stSubSysList.stSubSys[i].dNo-1,2));
			put_fanTable(dFanIdx++, sSysName, 4, fanTAF(stSubSysList.stSubSys[i].dNo-1,3));
			put_fanTable(dFanIdx++, sSysName, 5, fanTAF(stSubSysList.stSubSys[i].dNo-1,4));
			put_fanTable(dFanIdx++, sSysName, 6, fanTAF(stSubSysList.stSubSys[i].dNo-1,5));

			/*	NTP			*/
			sprintf(sNTPSysName, "%s_%s", sSysName, "DAEMON");
			sNTPSysName[strlen(sNTPSysName)] = 0x00;
			put_ntpTable(dNTPIdx++, sNTPSysName, 1, ntpTAF(stSubSysList.stSubSys[i].dNo-1,0));
			sprintf(sNTPSysName, "%s_%s", sSysName, "CHANNEL");
			sNTPSysName[strlen(sNTPSysName)] = 0x00;
			put_ntpTable(dNTPIdx++, sNTPSysName, 2, ntpTAF(stSubSysList.stSubSys[i].dNo-1,1));
		}
		dThreadStop++;
		sleep(5);
	}
	pthread_exit(NULL);
}

int init_sfdb(void)
{
    int     i, j, dRet;
    time_t  now;

	if( (dRet = shm_init(S_SSHM_TAF_FIDB, sizeof(st_NTAF_List_SHM), (void**)&sfdb)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN shm_init(TAF_FIDB=0x%x)",LT, S_SSHM_TAF_FIDB);
		return -1;
	}

    time(&now);

    for(i = 0; i < MAX_NTAF_COUNT; i++) {
        sfdb->stNTAF[i].tUpTime  = now;
        for(j = 0; j < MAX_NTAF_SW_BLOCK; j++)
            sfdb->stNTAF[i].mpswinfo[j].when = now;
    }
    return 0;
}

int dGetBlocks(char *fn, char (*p)[MAX_BLOCK_LENGTH])
{
	int		dIineNo, dReadCount, dScanCount;
	char	sBuf[BUFSIZ], sBlockName[MAX_BLOCK_LENGTH];
	FILE	*fp;

	if( (fp = fopen(fn, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN fopen(fn[%s]) errno[%d-%s]", LT, fn, errno, strerror(errno));
		return -1;
	}

	dIineNo		= 0;
	dReadCount	= 0;
	while(fgets(sBuf, BUFSIZ, fp) != NULL)
	{
		dIineNo++;
		if(sBuf[0] != '#')
		{
			log_print(LOGN_CRI, LH"%s File [%d] ROW FORMAT ERR", LT, fn, dIineNo);
			return -2;
		}
		else if(sBuf[1] == '#')
			continue;
		else if(sBuf[1] == 'E')
			break;
		else if(sBuf[1] == '@')
		{
			if( (dScanCount= sscanf(&sBuf[2], "%s %*s", sBlockName)) != 1)
				sprintf(sBlockName, " - ");
			strcpy(*(p+dReadCount), sBlockName);
			dReadCount++;
		}
		else
		{
			log_print(LOGN_CRI, LH"SYNTAX ERROR FILE:%s, LINK:%d", LT, fn, dIineNo);
			return -3;
		}
	}
	fclose(fp);

	return 0;
}

int dReadSubSysListFromFile(st_SubSysList *pstList)
{
    int 		dReadCount, dLineCount;
	in_addr_t	htohIP;
    FILE 		*fp;
    char 		sBuf[BUFSIZ], sScanInfo_1[MAX_SCAN_SIZE], sScanInfo_2[MAX_SCAN_SIZE], sScanInfo_3[MAX_SCAN_SIZE], sScanInfo_4[MAX_SCAN_SIZE], sScanInfo_5[MAX_SCAN_SIZE];

    if( (fp = fopen(FILE_SUB_SYS, "r")) == NULL)
	{
        log_print(LOGN_CRI, LH"FAILED IN fopen(%s)"EH, LT, FILE_SUB_SYS, ET);
        return -1;
    }

	dReadCount	= 0;
	dLineCount	= 0;
	while(fgets(sBuf, BUFSIZ, fp) != NULL)
	{
		if(sBuf[0] != '#')
		{
			log_print(LOGN_CRI, LH"File=%s:%d ROW FORMAT ERR", LT, FILE_SUB_SYS, dReadCount+1);
			fclose(fp);
			return -2;
		}

		if(sBuf[1] == '#')
			continue;
		else if(sBuf[1] == 'E')
			break;
		else if(sBuf[1] == '@')
		{
			if(sscanf(&sBuf[2], "%s %s %s %s %s %*s", sScanInfo_1, sScanInfo_2, sScanInfo_3, sScanInfo_4, sScanInfo_5) == 5)
			{
				if( (strcmp(sScanInfo_1,"TMF") == 0) || (strcmp(sScanInfo_1, "TMP") == 0))
				{
					if(dReadCount >= MAX_SUBSYS_COUNT)
					{
						log_print(LOGN_CRI, "SUB MAX DISCARD dLineCount[%d] sScanInfo_1[%s] sScanInfo_2[%s] sScanInfo_3[%s] sScanInfo_4[%s] sScanInfo_5[%s]",
							dLineCount, sScanInfo_1, sScanInfo_2, sScanInfo_3, sScanInfo_4, sScanInfo_5);
						break;
					}
					pstList->stSubSys[dReadCount].dType	= atoi(sScanInfo_2);
					pstList->stSubSys[dReadCount].dNo	= atoi(sScanInfo_3);
					if(pstList->stSubSys[dReadCount].dNo < 1)
					{
						log_print(LOGN_CRI, "SUB NUM DISCARD dLineCount[%d] sScanInfo_1[%s] sScanInfo_2[%s] sScanInfo_3[%s] sScanInfo_4[%s] sScanInfo_5[%s]",
							dLineCount, sScanInfo_1, sScanInfo_2, sScanInfo_3, sScanInfo_4, sScanInfo_5);
						continue;
					}

					htohIP	= inet_addr(sScanInfo_4);
					pstList->stSubSys[dReadCount].uiBIP	= ntohl(htohIP);
					pstList->stSubSys[dReadCount].dFlag	= atoi(sScanInfo_5);
					log_print(LOGN_CRI, "SUB TAF INFO POS[%d] dType[%d] dNo[%d] uiBIP[%u:%s] dFlag[%d]",
						dReadCount, pstList->stSubSys[dReadCount].dType, pstList->stSubSys[dReadCount].dNo, pstList->stSubSys[dReadCount].uiBIP, sScanInfo_4, pstList->stSubSys[dReadCount].dFlag);
					dReadCount++;
				}
				else
				{
					log_print(LOGN_CRI, "SUB DISCARD dLineCount[%d] sScanInfo_1[%s] sScanInfo_2[%s] sScanInfo_3[%s] sScanInfo_4[%s] sScanInfo_5[%s]",
						dLineCount, sScanInfo_1, sScanInfo_2, sScanInfo_3, sScanInfo_4, sScanInfo_5);
				}
			}
		}
		dLineCount++;
	}
	fclose(fp);

	pstList->dCount = dReadCount;
	if(pstList->dCount < 1)
	{
		log_print(LOGN_CRI, "READ SUBSYS INFO NO DATA: pstList->dCount[%d]", pstList->dCount);
		return -3;
	}
	log_print(LOGN_DEBUG, "READ SUBSYS INFO DATA: pstList->dCount[%d]", pstList->dCount);

	return 0;
}

void SetUpSignal(void)
{
	gJiSTOPFlag = 1;

    /* WANTED SIGNALS   */
    signal(SIGTERM, UserControlledSignal);
    signal(SIGINT,  UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    /* UNWANTED SIGNALS */
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);

	log_print(LOGN_DEBUG, LH"SIGNAL HANDLER WAS INSTALLED! gJiSTOPFlag[%d]", LT, gJiSTOPFlag);
}

void UserControlledSignal(int dSigNo)
{
	gJiSTOPFlag	= 0;
	dFinishSig	= dSigNo;
}

void IgnoreSignal(int dSigNo)
{
	if(dSigNo != SIGALRM)
		log_print(LOGN_CRI, "IgnoreSignal: UNWANTED SIGNAL IS RECEIVED, dSigNo[%d]", dSigNo);

	signal(dSigNo, IgnoreSignal);
}

int dInitFidb(void)
{
    int     dRet;

    if( (dRet = shm_init(S_SSHM_FIDB, DEF_WNTAM_SIZE, (void**)&stWNTAM)) < 0 ){
        log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_FIDB[0x%04X], dRet=%d)"EH,
        LT,S_SSHM_FIDB,dRet,ET);
        return -6;
    }

    fidb     = &stWNTAM->stNTAM;
    //director = &stWNTAM->stDirectTOT;
    //swch     = &stWNTAM->stSwitchTOT;

	/**
	 * 공유메모리 초기화 부분이 빠져있음
	 */

    return 0;
}
