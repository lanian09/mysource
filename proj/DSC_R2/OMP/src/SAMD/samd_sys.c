#include "samd.h"

#ifndef TRU64

#include <kvm.h>
#include <sys/proc.h>
#include <sys/procfs.h>
#include <sys/var.h>
#include <sys/cpuvar.h>
#include <sys/file.h>
#include <sys/swap.h>
#include <kstat.h>

/* system information related */
#define CPUSTATES       16 
#define NUM_STRINGS     8

#define CPUSTATE_IOWAIT 	3
#define CPUSTATE_SWAP   	4

kstat_ctl_t	*kopen = NULL;
kstat_t		**cpu_ks = NULL; //초기화가 필요하다
cpu_stat_t	*cpu_stat = NULL;

int	cpu_states[CPUSTATES];

int	memory_state[3];

int	ncpus;

int	memory_flag = 0;
int	cpu_flag = 0;

void 	Error(char *);

extern int  trcFlag, trcLogFlag;
extern char trcBuf[4096];
extern SFM_SysCommMsgType 	*loc_sadb;
extern SFM_L3PD			  	*l3pd;
extern SFM_SCE      		*g_pstSCEInfo;

extern int		statistic_cnt;
extern STM_LoadOMPStatMsgType    system_statistic;


void
get_cpu_relate_information(count, cpu_info, new, old, diffs)
int	count;
int	*cpu_info;
long	*new;
long	*old;
long	*diffs;
{
	int	i;
	long	change;
	long	total_change;
	long	*dp;
	long	half_total;

	total_change = 0;
	dp = diffs;

	for (i = 0; i < count; i++) {
		if ((change = *new - *old) < 0)
			change = (int) ((unsigned long)*new-(unsigned long)*old);

		total_change += (*dp++ = change);
		*old++ = *new++;
   	}

	if (total_change == 0)
		total_change = 1;

	half_total = total_change / 2L;
	for (i = 0; i < count; i++)
		*cpu_info++ = (int)((*diffs++ * 1000 + half_total) / total_change);
}

#define UPDKCID(nk,ok) \
if (nk == -1) { \
	Error("kstat read1"); \
	return -1; \
} \
if (nk != ok)\
  goto kcid_changed;

int
get_cpu_and_load_average()
{
	kstat_t	*lookup;
	kid_t	chaind_update;
	int	i;
	int	changed = 0;
	static	int ncpu = 0;
	static	kid_t chaing_ID = 0;
	kstat_named_t *data_lookup;

	/*	kstat_open : 커널 통계 설비를 초기화 한다. 시스템에 대한 통계 라이브러리에
		접근하기 위해 초기화 작업을 한다. */
	kopen = kstat_open();
	if (!kopen) {
		Error("kstat open");
		return -1;
	}

	changed = 1;
	chaing_ID = kopen->kc_chain_id; /* 200이 찍히는데 의미는 ? */

kcid_changed:

	/*	kstat_chain_update : chain은 현 시스템에 있는 모든 kstat리스트를 이 고리로
		이어준다. 커널에 있는 KCID(kstat chain ID)중 인자로 지정한 ID를 찾아 반환한다. */
    chaind_update = kstat_chain_update(kopen);

	if (chaind_update) { /* 현재 들어가지 않는다 */
		changed = 1;
		chaing_ID = chaind_update;
	}

	UPDKCID(chaind_update,0);

	/*	kstat_loopup : kstat 연결 고리들을 찾아 지정한 인자에 해당하는 것을 찾는다.
		고리는 kc이고 사용자가 찾는 것은 ks_module, ks_instance, ks_name으로 지정한다
		이 세가지 값으로 kstat에서 유일한 필드를 찾는다 */
	lookup = kstat_lookup(kopen, "unix", 0, "system_misc");
	/*	kstat_open으로 구하는 값은 kstat 고리이고, 이고리를 이용해 원하는 위치로 찾아
		들어가는 것은 ksatat_lookup이다. 그리고 그곳에서 정보를 읽는 작업은 kstat_read가
		담당한다. */
	if (kstat_read(kopen, lookup, 0) == -1) {
		Error("kstat read2");
		return -1;
	}

	/*	kstat_data_lookup : kstat_read로 읽은 정보 중 사용자가 원하는 것을 자세히 구하는
		작업은 kstat_data_lookup이 한다. kstat_data_loopup에서 지정한 이름에 해당하는 
		정보가 kstat_named_t구조체에 저방되므로 사용자가 이를 이용한다 */

	if (changed) {

		ncpu = 0;

		data_lookup = kstat_data_lookup(lookup, "ncpus");

		if (data_lookup && data_lookup->value.ui32 > ncpus) {
			ncpus = data_lookup->value.ui32; /* CPU 갯수 */
			cpu_ks = (kstat_t **) realloc (cpu_ks, ncpus * sizeof (kstat_t *));
			cpu_stat = (cpu_stat_t *) realloc (cpu_stat, ncpus * sizeof (cpu_stat_t));
			if (trcFlag || trcLogFlag) {
				//sprintf(trcBuf, " cpu_ks = %x, cpu_stat=%x\n", cpu_ks, cpu_stat);
				sprintf(trcBuf, " realloc fail \n");
				trclib_writeLog(FL, trcBuf);
			}
		}

		for (lookup = kopen->kc_chain; lookup; lookup = lookup->ks_next) {
			if (strncmp(lookup->ks_name, "cpu_stat", 8) == 0) {
				chaind_update = kstat_read(kopen, lookup, NULL);
				UPDKCID(chaind_update, chaing_ID);

				cpu_ks[ncpu] = lookup;
				ncpu++;
				if (ncpu > ncpus) {
					Error("kstat finds too many cpus");
					return -1;
				}
			}
		}
		changed = 0;
	}

	for (i = 0; i < ncpu; i++) {
		chaind_update = kstat_read(kopen, cpu_ks[i], &cpu_stat[i]);
		UPDKCID(chaind_update, chaing_ID);
	}

    return(ncpu);
}



void get_system_information (system_info *system_information, int flag)
{
	static	int free_memory;
	static	int total_memory;
	static	long cpu_info[CPUSTATES];
	static	long cpu_old[CPUSTATES];
	static	long system_diff[CPUSTATES];
	int	j, i, disk_usage=0, disk_sub_max[loc_sadb->diskCount], disk_tot_max=0, msgq_usage=0, tempUsage=0;

	kstat_t		*lookup;
	kstat_named_t	*data_lookup;
	int		cpu_number;

	for (j = 0; j < CPUSTATES; j++)
		cpu_info[j] = 0L;

	cpu_number = get_cpu_and_load_average();
	// ADD: BY JUNE, 2011-02-10
	//      Kernel 에서 KSTAT 을 못 가져오는 경우 프로세스 종료하던 기존 로직을
	//      종료하지 않고 상태 반영 안하는 방법으로 변경(요구사항)
	//      해당 경우에 상태 감시 대상에 대해 감시 안됨.
	if (cpu_number == -1) {
		if (kopen) {
			if(kstat_close(kopen) < 0) {
				sprintf(trcBuf, "case Error(), kstat_close error=%d(%s)\n", errno, strerror(errno));
				trclib_writeLogErr(FL, trcBuf);
			} 
			kopen = NULL;
		} 
		return;
	}

	loc_sadb->cpuCount = 1; /* 강제로 CPU갯수를 입력한다 */

	for (i = 0; i < cpu_number; i++) {
		/* ****************************************************	*/
		// idle%, user%, kernel%에 대한 값을 cpu_info[]에 저장한다.
		/* ****************************************************	*/
		for (j = 0; j < 3; j++) {
			cpu_info[j] += (long) cpu_stat[i].cpu_sysinfo.cpu[j];
		}
		
		/* ****************************************************	*/
		// iowait% 구하기
		/* ****************************************************	*/
		cpu_info[CPUSTATE_IOWAIT] += (long) cpu_stat[i].cpu_sysinfo.wait[W_IO] 
			+ (long) cpu_stat[i].cpu_sysinfo.wait[W_PIO];
		/* ****************************************************	*/
		// swap% 구하기
		/* ****************************************************	*/
		cpu_info[CPUSTATE_SWAP] += (long) cpu_stat[i].cpu_sysinfo.wait[W_SWAP];
   	}

	lookup = kstat_lookup(kopen, "unix", 0, "system_pages");
	if (kstat_read(kopen, lookup, 0) == -1) {
		Error("kstat read3");
		return;
	}

	/* ****************************************************	*/
	// free memory 구하기
	/* ****************************************************	*/
	data_lookup = kstat_data_lookup(lookup, "freemem");
	if (data_lookup)
		free_memory = data_lookup->value.ul;

	/* ****************************************************	*/
	// cpu_states[] : idle, user, kernel, iowait, swap 등이
	// 저장된다.
	/* ****************************************************	*/
	get_cpu_relate_information (CPUSTATES,
		cpu_states, cpu_info, cpu_old, system_diff);


	/* ****************************************************	*/
	// sysconf()로 전체 메모리 구하기
	/* ****************************************************	*/
	if ( !memory_flag ) {
		total_memory = sysconf(_SC_PHYS_PAGES);
		memory_flag = 1;
	}

#if 0 // realloc로 정보를 한번만 받기 때문에 free할 필요가 없음
	free(cpu_ks);
	free(cpu_stat);
#endif

	if ( kstat_close(kopen) < 0) { /* kstat_open을 초기화 한다 */
		sprintf(trcBuf, "kstat_close error = %s\n", strerror(errno));
		trclib_writeLogErr(FL, trcBuf);
	}
	kopen = NULL;

	memory_state[0] = (total_memory << 3) / 1024;
	memory_state[1] = 0;
	memory_state[2] = (free_memory << 3) / 1024;

// yhshin	
//	system_information->cpu_state = cpu_states;
//	system_information->memory_state = memory_state;

	// first
	loc_sadb->cpu_usage[0] = 1000 - *(cpu_states);
	for ( i=0 ; i < cpu_number ; i++) {
		// average
		loc_sadb->cpu_usage[0] = ((loc_sadb->cpu_usage[0]) + (1000 - *(cpu_states)))/2;
		if (trcFlag || trcLogFlag) { 
			sprintf(trcBuf, "cpuCount = %d, usage = %d\n", cpu_number, loc_sadb->cpu_usage[0]);
			trclib_writeLog(FL, trcBuf);
		}
	} 
	
	loc_sadb->mem_usage = 1000 - ((memory_state[2] * 1000) / memory_state[0]) ;
	if (trcFlag || trcLogFlag) {
		sprintf(trcBuf, "mem_usage = %d\n", loc_sadb->mem_usage);
		trclib_writeLog(FL, trcBuf);
	}
	if (flag != INIT_FLOW) {
		// cpu 통계를 위해 
		statistic_cnt++;
		for ( i=0 ; i < loc_sadb->cpuCount ; i++) {
			system_statistic.ompInfo.comminfo.average_cpu[i] += loc_sadb->cpu_usage[i];
			if ( system_statistic.ompInfo.comminfo.max_cpu[i] < loc_sadb->cpu_usage[i] ) {
				system_statistic.ompInfo.comminfo.max_cpu[i] = loc_sadb->cpu_usage[i];
			}
		}
		// mem 통게를 위해
		system_statistic.ompInfo.comminfo.avg_mem += loc_sadb->mem_usage;
		if ( system_statistic.ompInfo.comminfo.max_mem < loc_sadb->mem_usage)
			system_statistic.ompInfo.comminfo.max_mem = loc_sadb->mem_usage;
		
		// by helca 07.29 
		// PD 통계를 위해...
		for ( i=0 ; i < MAX_PROBE_DEV_NUM ; i++) {
			system_statistic.pdInfo[i].average_cpu[0] += (l3pd->l3ProbeDev[i].cpuInfo.usage * 10);
			if ( system_statistic.pdInfo[i].max_cpu[0] < (l3pd->l3ProbeDev[i].cpuInfo.usage * 10)) {
				system_statistic.pdInfo[i].max_cpu[0] = (l3pd->l3ProbeDev[i].cpuInfo.usage * 10);
			}
		}
		// PD mem 통계를 위해
		for ( i=0 ; i < MAX_PROBE_DEV_NUM ; i++) {
			system_statistic.pdInfo[i].avg_mem += (l3pd->l3ProbeDev[i].memInfo.usage * 10);
			if ( system_statistic.pdInfo[i].max_mem < (l3pd->l3ProbeDev[i].memInfo.usage * 10))
				system_statistic.pdInfo[i].max_mem = (l3pd->l3ProbeDev[i].memInfo.usage * 10);
		}

		/* CISCO SCE statistic, by june */
		/* CPU */
		for ( i=0 ; i < MAX_SCE_DEV_NUM; i++) {
			for ( j=0 ; j < MAX_SCE_CPU_CNT ; j++) {
				system_statistic.sceInfo[i].avg_cpu[j] += (g_pstSCEInfo->SCEDev[i].cpuInfo[j].usage * 10);
				if ( system_statistic.sceInfo[i].max_cpu[j] < (g_pstSCEInfo->SCEDev[i].cpuInfo[j].usage * 10)) {
					system_statistic.sceInfo[i].max_cpu[j] = (g_pstSCEInfo->SCEDev[i].cpuInfo[j].usage * 10);
				}
			}
		}
		/* MEMORY */
		for ( i=0 ; i < MAX_SCE_DEV_NUM; i++) {
			for ( j=0 ; j < MAX_SCE_MEM_CNT ; j++) {
				system_statistic.sceInfo[i].avg_mem[j] += (g_pstSCEInfo->SCEDev[i].memInfo[j].usage * 10);
				if ( system_statistic.sceInfo[i].max_mem[j] < (g_pstSCEInfo->SCEDev[i].memInfo[j].usage * 10)) {
					system_statistic.sceInfo[i].max_mem[j] = (g_pstSCEInfo->SCEDev[i].memInfo[j].usage * 10);
				}
			}
		}
		/* DISK */
		for ( i=0 ; i < MAX_SCE_DEV_NUM; i++) {
			system_statistic.sceInfo[i].avg_disk += (g_pstSCEInfo->SCEDev[i].diskInfo.usage * 10);
			if ( system_statistic.sceInfo[i].max_disk < (g_pstSCEInfo->SCEDev[i].diskInfo.usage * 10)) {
				system_statistic.sceInfo[i].max_disk = (g_pstSCEInfo->SCEDev[i].diskInfo.usage * 10);
			}
		}

		/* hjjung_20100823 */
		/* USER */
		for ( i=0 ; i < MAX_SCE_DEV_NUM; i++) {
			system_statistic.sceInfo[i].avg_user += (g_pstSCEInfo->SCEDev[i].userInfo.num * 10);
			if ( system_statistic.sceInfo[i].max_user < (g_pstSCEInfo->SCEDev[i].userInfo.num * 10)) {
				system_statistic.sceInfo[i].max_user = (g_pstSCEInfo->SCEDev[i].userInfo.num * 10);
			}
		}
		
	    // by helca 07.31 jean
		// disk 통계를 위해
		for(i=0; i< loc_sadb->diskCount; i++){
			disk_sub_max[i] = 0;
			disk_usage += loc_sadb->loc_disk_sts[i].disk_usage;
			if(disk_sub_max[i] < loc_sadb->loc_disk_sts[i].disk_usage){
				disk_sub_max[i] = loc_sadb->loc_disk_sts[i].disk_usage;
			}      
		}
		for(i=0; i< loc_sadb->diskCount; i++){
			disk_tot_max += disk_sub_max[i];
		}
		disk_tot_max = disk_tot_max/loc_sadb->diskCount;
		system_statistic.ompInfo.max_disk += disk_tot_max;

		if (loc_sadb->diskCount == 0)
			disk_usage = 0;
		else
			disk_usage = disk_usage/loc_sadb->diskCount;

		system_statistic.ompInfo.avg_disk += disk_usage;

		// by helca 07.31 //
		// msgQ 통계를 위해
		for(i=0; i< loc_sadb->queCount; i++){
			if ( loc_sadb->loc_que_sts[i].qBYTES == 0 )
				tempUsage = 0;
			else
				tempUsage = (loc_sadb->loc_que_sts[i].cBYTES*100)/loc_sadb->loc_que_sts[i].qBYTES;
				msgq_usage += tempUsage;
			if(system_statistic.ompInfo.max_msgQ < tempUsage){
				system_statistic.ompInfo.max_msgQ = tempUsage;
			}
		}
		if(loc_sadb->queCount == 0)
			msgq_usage =0;
		else
			msgq_usage =msgq_usage/loc_sadb->queCount;

		system_statistic.ompInfo.avg_msgQ += msgq_usage;
	}
}

void
Error(char *error_name)
{
	sprintf(trcBuf,"[samd_sys] %s = %s\n", error_name, strerror(errno));
	trclib_writeLogErr (FL,trcBuf);
	//exit(0);
}

#endif /* not_TRU64 */
