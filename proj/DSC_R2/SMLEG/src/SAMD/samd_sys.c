#include "samd.h"

#include <kvm.h>
#include <sys/proc.h>
#include <sys/procfs.h>
#include <sys/var.h>
#include <sys/cpuvar.h>
#include <sys/file.h>
#include <sys/swap.h>
#include <kstat.h>

/* system information related */
#define CPUSTATES			16
#define NUM_STRINGS			8

#define CPUSTATE_IOWAIT		3
#define CPUSTATE_SWAP		4

// MOD: BY JUNE, 2011-02-11
#define UPDKCID(nk,ok) \
if (nk == -1) { \
	Error("kstat read1"); \
	return -1; \
} \
if (nk != ok)\
  goto kcid_changed;

kstat_ctl_t	*kopen		= NULL;
kstat_t		**cpu_ks = NULL;
cpu_stat_t	*cpu_status = NULL;

int	cpu_states[CPUSTATES];
int	memory_state[3];
int	ncpus;
int	memory_flag	= 0;
int	cpu_flag	= 0;

extern SFM_SysCommMsgType		*loc_sadb;
extern SFM_L3PD					*l3pd;
extern STM_LoadMPStatMsgType    system_statistic;

extern char		trcBuf[4096];

extern int		trcFlag;
extern int		trcLogFlag;
extern int		statistic_cnt;

void get_cpu_relate_information(int	count, int *cpu_info, long *new, long *old, long *diffs);
void get_system_information(int flag);
void Error(char *error_name);
int get_cpu_and_load_average();

void get_cpu_relate_information(int	count, int *cpu_info, long *new, long *old, long *diffs)
{
	int		i;
	long	change, total_change, half_total, *dp;

	total_change	= 0;
	dp				= diffs;

	for(i = 0; i < count; i++)
	{
		if( (change = *new - *old) < 0)
			change = (int)((unsigned long)*new-(unsigned long)*old);

		total_change += (*dp++ = change);
		*old++ = *new++;
	}

	if(total_change == 0)
		total_change = 1;

	half_total = total_change / 2L;
	for(i = 0; i < count; i++)
		*cpu_info++ = (int)((*diffs++ * 1000 + half_total) / total_change);
}

int get_cpu_and_load_average()
{
	kstat_t	*lookup;
	kid_t	chaind_update;
	int		i, changed = 0;
	static int		ncpu = 0;
	static kid_t	chaing_ID = 0;
	kstat_named_t	*data_lookup;

	/*	kstat_open : 커널 통계 설비를 초기화 한다. 시스템에 대한 통계 라이브러리에 접근하기 위해 초기화 작업을 한다. */
	if (!kopen)  {
		kopen = kstat_open();
		if (!kopen) {
			Error("kstat open");
			return -1;
		}

		changed		= 1;
		chaing_ID	= kopen->kc_chain_id;	/* 200이 찍히는데 의미는 ? */
	}

kcid_changed:
	/*	kstat_chain_update : chain은 현 시스템에 있는 모든 kstat리스트를 이 고리로
	이어준다. 커널에 있는 KCID(kstat chain ID)중 인자로 지정한 ID를 찾아 반환한다. */
	chaind_update	= kstat_chain_update(kopen);
	if(chaind_update)
	{
		/* 현재 들어가지 않는다 */
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
	if(kstat_read(kopen, lookup, 0) == -1) {
		Error("kstat read2");
		return -1;
	}

	/*	kstat_data_lookup : kstat_read로 읽은 정보 중 사용자가 원하는 것을 자세히 구하는
	작업은 kstat_data_lookup이 한다. kstat_data_loopup에서 지정한 이름에 해당하는
	정보가 kstat_named_t구조체에 저방되므로 사용자가 이를 이용한다 */


	if(changed)
	{
		ncpu		= 0;
		data_lookup	= kstat_data_lookup(lookup, "ncpus");

		if(data_lookup && data_lookup->value.ui32 > ncpus)
		{
			ncpus		= data_lookup->value.ui32; /* CPU 갯수 */

			cpu_ks		= (kstat_t**)realloc(cpu_ks, ncpus * sizeof (kstat_t*));
			cpu_status	= (cpu_stat_t*)realloc(cpu_status, ncpus * sizeof (cpu_stat_t));
			if(trcFlag || trcLogFlag)
			{
				sprintf(trcBuf, " cpu_ks = %p, cpu_status=%p\n", cpu_ks, cpu_status);
				trclib_writeLog(FL, trcBuf);
			}
		}

		for(lookup = kopen->kc_chain; lookup; lookup = lookup->ks_next)
		{
			if(strncmp(lookup->ks_name, "cpu_stat", 8) == 0)
			{
				chaind_update = kstat_read(kopen, lookup, NULL);
				UPDKCID(chaind_update, chaing_ID);

				cpu_ks[ncpu] = lookup;
				ncpu++;
				if(ncpu > ncpus) {
					Error("kstat finds too many cpus");
					return -1;
				}
			}
		}
		changed = 0;
	}

	for(i = 0; i < ncpu; i++)
	{
		chaind_update = kstat_read(kopen, cpu_ks[i], &cpu_status[i]);
		UPDKCID(chaind_update, chaing_ID);
	}

	return ncpu;
}

void get_system_information(int flag)
{
	static int	free_memory, total_memory;
	static long	cpu_info[CPUSTATES], cpu_old[CPUSTATES], system_diff[CPUSTATES];
	int		i, j, cpu_number, disk_sub_max[loc_sadb->diskCount], msgq_usage;
	int		disk_usage, disk_tot_max, tempUsage, rsrcLoad, rsrctmp;

	kstat_t			*lookup;
	kstat_named_t	*data_lookup;

	disk_usage		= 0;
	disk_tot_max	= 0;
	msgq_usage		= 0;
	tempUsage		= 0;
	rsrcLoad		= 0;
	rsrctmp			= 0;

	for(j = 0; j < CPUSTATES; j++)
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

//	loc_sadb->cpuCount = cpu_number;			/*	CPU갯수를 입력한다	*/
	loc_sadb->cpuCount = 1;			/*	CPU갯수를 입력한다	*/


	for(i = 0; i < cpu_number; i++)
	{
		/******************************************************
			idle%, user%, kernel%
		******************************************************/
		for(j = 0; j < 3; j++)
			cpu_info[j] += (long)cpu_status[i].cpu_sysinfo.cpu[j];

		/******************************************************
			iowait%
		******************************************************/
		cpu_info[CPUSTATE_IOWAIT] += (long) cpu_status[i].cpu_sysinfo.wait[W_IO]
			+ (long) cpu_status[i].cpu_sysinfo.wait[W_PIO];
		/******************************************************
			swapwait%
		******************************************************/
		cpu_info[CPUSTATE_SWAP] += (long) cpu_status[i].cpu_sysinfo.wait[W_SWAP];
   	}

	lookup = kstat_lookup(kopen, "unix", 0, "system_pages");
	if (kstat_read(kopen, lookup, 0) == -1) {
		Error("kstat read3");
		return;
	}
	/******************************************************
		Free memory
	******************************************************/
	data_lookup = kstat_data_lookup(lookup, "freemem");
	if (data_lookup)
		free_memory = data_lookup->value.ul;

	/***************************************************************
		cpu_status[]: idle%, user%, kernel%, iowait%, swapwait%
	***************************************************************/
	get_cpu_relate_information(CPUSTATES, cpu_states, cpu_info, cpu_old, system_diff);

	/***************************************************************
		total physical memory using sysconf()
	***************************************************************/
	if(!memory_flag)
	{
		total_memory = sysconf(_SC_PHYS_PAGES);
		memory_flag = 1;
	}

#if 0 // realloc로 정보를 한번만 받기 때문에 free할 필요가 없음
	free(cpu_ks);
	free(cpu_status);
#endif

	if(kstat_close(kopen) < 0)
	{
		/*	kstat_open을 초기화 한다	*/
		sprintf(trcBuf, "kstat_close error = %s\n", strerror(errno));
		trclib_writeLogErr(FL, trcBuf);
		kopen = NULL;
	}
	kopen = NULL;

	memory_state[0] = (total_memory << 3) / 1024;
	memory_state[1] = 0;
	memory_state[2] = (free_memory << 3) / 1024;

	// first
	loc_sadb->cpu_usage[0] = 1000 - *(cpu_states);
	for(i = 0; i < cpu_number; i++)
	{
		loc_sadb->cpu_usage[0] = ((loc_sadb->cpu_usage[0]) + (1000 - *(cpu_states)))/2;
		if(trcFlag || trcLogFlag)
		{
			sprintf(trcBuf, "cpuCount = %d, usage = %d\n", cpu_number, loc_sadb->cpu_usage[0]);
			trclib_writeLog(FL, trcBuf);
		}
	}

	loc_sadb->mem_usage = 1000 - ((memory_state[2] * 1000) / memory_state[0]) ;
	if(trcFlag || trcLogFlag)
	{
		sprintf(trcBuf, "mem_usage = %d\n", loc_sadb->mem_usage);
		trclib_writeLog(FL, trcBuf);
	}

	if(flag != INIT_FLOW)
	{
		// cpu 통계를 위해
		statistic_cnt++;
		for(i = 0; i < loc_sadb->cpuCount; i++)
		{
			system_statistic.comminfo.average_cpu[i] += loc_sadb->cpu_usage[i];
			if(system_statistic.comminfo.max_cpu[i] < loc_sadb->cpu_usage[i])
				system_statistic.comminfo.max_cpu[i] = loc_sadb->cpu_usage[i];
		}

		// mem 통게를 위해
		system_statistic.comminfo.avg_mem += loc_sadb->mem_usage;
		if(system_statistic.comminfo.max_mem < loc_sadb->mem_usage)
			system_statistic.comminfo.max_mem = loc_sadb->mem_usage;


		// by helca 07.31 jean
		// disk 통계를 위해
		for(i = 0; i < loc_sadb->diskCount; i++)
		{
			disk_sub_max[i]	= 0;
			disk_usage		+= loc_sadb->loc_disk_sts[i].disk_usage;
			if(disk_sub_max[i] < loc_sadb->loc_disk_sts[i].disk_usage)
				disk_sub_max[i] = loc_sadb->loc_disk_sts[i].disk_usage;
		}

		for(i = 0; i < loc_sadb->diskCount; i++)
			disk_tot_max += disk_sub_max[i];
		disk_tot_max = disk_tot_max/loc_sadb->diskCount;
		system_statistic.max_disk += disk_tot_max;


		if(loc_sadb->diskCount == 0)
			disk_usage = 0;
		else
			disk_usage = disk_usage/loc_sadb->diskCount;

		system_statistic.avg_disk += disk_usage;

		// by helca 07.31 //
		// msgQ 통계를 위해
		for(i = 0; i < loc_sadb->queCount; i++)
		{
			if(loc_sadb->loc_que_sts[i].qBYTES == 0)
				tempUsage = 0;
			else
				tempUsage = (loc_sadb->loc_que_sts[i].cBYTES*100)/loc_sadb->loc_que_sts[i].qBYTES;
			msgq_usage += tempUsage;
			if(system_statistic.max_msgQ < tempUsage)
				system_statistic.max_msgQ = tempUsage;
		}

		if(loc_sadb->queCount == 0)
			msgq_usage =0;
		else
			msgq_usage =msgq_usage/loc_sadb->queCount;

		system_statistic.avg_msgQ += msgq_usage;
	}
}

void Error(char *error_name)
{
	sprintf(trcBuf,"[samd_sys] %s = %s\n", error_name, strerror(errno));
	trclib_writeLogErr (FL,trcBuf);
	//exit(0);
}
