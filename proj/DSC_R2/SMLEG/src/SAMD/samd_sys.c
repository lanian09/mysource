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

	/*	kstat_open : Ŀ�� ��� ���� �ʱ�ȭ �Ѵ�. �ý��ۿ� ���� ��� ���̺귯���� �����ϱ� ���� �ʱ�ȭ �۾��� �Ѵ�. */
	if (!kopen)  {
		kopen = kstat_open();
		if (!kopen) {
			Error("kstat open");
			return -1;
		}

		changed		= 1;
		chaing_ID	= kopen->kc_chain_id;	/* 200�� �����µ� �ǹ̴� ? */
	}

kcid_changed:
	/*	kstat_chain_update : chain�� �� �ý��ۿ� �ִ� ��� kstat����Ʈ�� �� ����
	�̾��ش�. Ŀ�ο� �ִ� KCID(kstat chain ID)�� ���ڷ� ������ ID�� ã�� ��ȯ�Ѵ�. */
	chaind_update	= kstat_chain_update(kopen);
	if(chaind_update)
	{
		/* ���� ���� �ʴ´� */
		changed = 1;
		chaing_ID = chaind_update;
	}
	UPDKCID(chaind_update,0);

	/*	kstat_loopup : kstat ���� ������ ã�� ������ ���ڿ� �ش��ϴ� ���� ã�´�.
	���� kc�̰� ����ڰ� ã�� ���� ks_module, ks_instance, ks_name���� �����Ѵ�
	�� ������ ������ kstat���� ������ �ʵ带 ã�´� */
	lookup = kstat_lookup(kopen, "unix", 0, "system_misc");
	/*	kstat_open���� ���ϴ� ���� kstat ���̰�, �̰��� �̿��� ���ϴ� ��ġ�� ã��
	���� ���� ksatat_lookup�̴�. �׸��� �װ����� ������ �д� �۾��� kstat_read��
	����Ѵ�. */
	if(kstat_read(kopen, lookup, 0) == -1) {
		Error("kstat read2");
		return -1;
	}

	/*	kstat_data_lookup : kstat_read�� ���� ���� �� ����ڰ� ���ϴ� ���� �ڼ��� ���ϴ�
	�۾��� kstat_data_lookup�� �Ѵ�. kstat_data_loopup���� ������ �̸��� �ش��ϴ�
	������ kstat_named_t����ü�� ����ǹǷ� ����ڰ� �̸� �̿��Ѵ� */


	if(changed)
	{
		ncpu		= 0;
		data_lookup	= kstat_data_lookup(lookup, "ncpus");

		if(data_lookup && data_lookup->value.ui32 > ncpus)
		{
			ncpus		= data_lookup->value.ui32; /* CPU ���� */

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
	//      Kernel ���� KSTAT �� �� �������� ��� ���μ��� �����ϴ� ���� ������
	//      �������� �ʰ� ���� �ݿ� ���ϴ� ������� ����(�䱸����)
	//      �ش� ��쿡 ���� ���� ��� ���� ���� �ȵ�.
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

//	loc_sadb->cpuCount = cpu_number;			/*	CPU������ �Է��Ѵ�	*/
	loc_sadb->cpuCount = 1;			/*	CPU������ �Է��Ѵ�	*/


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

#if 0 // realloc�� ������ �ѹ��� �ޱ� ������ free�� �ʿ䰡 ����
	free(cpu_ks);
	free(cpu_status);
#endif

	if(kstat_close(kopen) < 0)
	{
		/*	kstat_open�� �ʱ�ȭ �Ѵ�	*/
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
		// cpu ��踦 ����
		statistic_cnt++;
		for(i = 0; i < loc_sadb->cpuCount; i++)
		{
			system_statistic.comminfo.average_cpu[i] += loc_sadb->cpu_usage[i];
			if(system_statistic.comminfo.max_cpu[i] < loc_sadb->cpu_usage[i])
				system_statistic.comminfo.max_cpu[i] = loc_sadb->cpu_usage[i];
		}

		// mem ��Ը� ����
		system_statistic.comminfo.avg_mem += loc_sadb->mem_usage;
		if(system_statistic.comminfo.max_mem < loc_sadb->mem_usage)
			system_statistic.comminfo.max_mem = loc_sadb->mem_usage;


		// by helca 07.31 jean
		// disk ��踦 ����
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
		// msgQ ��踦 ����
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
