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
kstat_t		**cpu_ks = NULL; //�ʱ�ȭ�� �ʿ��ϴ�
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

	/*	kstat_open : Ŀ�� ��� ���� �ʱ�ȭ �Ѵ�. �ý��ۿ� ���� ��� ���̺귯����
		�����ϱ� ���� �ʱ�ȭ �۾��� �Ѵ�. */
	kopen = kstat_open();
	if (!kopen) {
		Error("kstat open");
		return -1;
	}

	changed = 1;
	chaing_ID = kopen->kc_chain_id; /* 200�� �����µ� �ǹ̴� ? */

kcid_changed:

	/*	kstat_chain_update : chain�� �� �ý��ۿ� �ִ� ��� kstat����Ʈ�� �� ����
		�̾��ش�. Ŀ�ο� �ִ� KCID(kstat chain ID)�� ���ڷ� ������ ID�� ã�� ��ȯ�Ѵ�. */
    chaind_update = kstat_chain_update(kopen);

	if (chaind_update) { /* ���� ���� �ʴ´� */
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
	if (kstat_read(kopen, lookup, 0) == -1) {
		Error("kstat read2");
		return -1;
	}

	/*	kstat_data_lookup : kstat_read�� ���� ���� �� ����ڰ� ���ϴ� ���� �ڼ��� ���ϴ�
		�۾��� kstat_data_lookup�� �Ѵ�. kstat_data_loopup���� ������ �̸��� �ش��ϴ� 
		������ kstat_named_t����ü�� ����ǹǷ� ����ڰ� �̸� �̿��Ѵ� */

	if (changed) {

		ncpu = 0;

		data_lookup = kstat_data_lookup(lookup, "ncpus");

		if (data_lookup && data_lookup->value.ui32 > ncpus) {
			ncpus = data_lookup->value.ui32; /* CPU ���� */
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

	loc_sadb->cpuCount = 1; /* ������ CPU������ �Է��Ѵ� */

	for (i = 0; i < cpu_number; i++) {
		/* ****************************************************	*/
		// idle%, user%, kernel%�� ���� ���� cpu_info[]�� �����Ѵ�.
		/* ****************************************************	*/
		for (j = 0; j < 3; j++) {
			cpu_info[j] += (long) cpu_stat[i].cpu_sysinfo.cpu[j];
		}
		
		/* ****************************************************	*/
		// iowait% ���ϱ�
		/* ****************************************************	*/
		cpu_info[CPUSTATE_IOWAIT] += (long) cpu_stat[i].cpu_sysinfo.wait[W_IO] 
			+ (long) cpu_stat[i].cpu_sysinfo.wait[W_PIO];
		/* ****************************************************	*/
		// swap% ���ϱ�
		/* ****************************************************	*/
		cpu_info[CPUSTATE_SWAP] += (long) cpu_stat[i].cpu_sysinfo.wait[W_SWAP];
   	}

	lookup = kstat_lookup(kopen, "unix", 0, "system_pages");
	if (kstat_read(kopen, lookup, 0) == -1) {
		Error("kstat read3");
		return;
	}

	/* ****************************************************	*/
	// free memory ���ϱ�
	/* ****************************************************	*/
	data_lookup = kstat_data_lookup(lookup, "freemem");
	if (data_lookup)
		free_memory = data_lookup->value.ul;

	/* ****************************************************	*/
	// cpu_states[] : idle, user, kernel, iowait, swap ����
	// ����ȴ�.
	/* ****************************************************	*/
	get_cpu_relate_information (CPUSTATES,
		cpu_states, cpu_info, cpu_old, system_diff);


	/* ****************************************************	*/
	// sysconf()�� ��ü �޸� ���ϱ�
	/* ****************************************************	*/
	if ( !memory_flag ) {
		total_memory = sysconf(_SC_PHYS_PAGES);
		memory_flag = 1;
	}

#if 0 // realloc�� ������ �ѹ��� �ޱ� ������ free�� �ʿ䰡 ����
	free(cpu_ks);
	free(cpu_stat);
#endif

	if ( kstat_close(kopen) < 0) { /* kstat_open�� �ʱ�ȭ �Ѵ� */
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
		// cpu ��踦 ���� 
		statistic_cnt++;
		for ( i=0 ; i < loc_sadb->cpuCount ; i++) {
			system_statistic.ompInfo.comminfo.average_cpu[i] += loc_sadb->cpu_usage[i];
			if ( system_statistic.ompInfo.comminfo.max_cpu[i] < loc_sadb->cpu_usage[i] ) {
				system_statistic.ompInfo.comminfo.max_cpu[i] = loc_sadb->cpu_usage[i];
			}
		}
		// mem ��Ը� ����
		system_statistic.ompInfo.comminfo.avg_mem += loc_sadb->mem_usage;
		if ( system_statistic.ompInfo.comminfo.max_mem < loc_sadb->mem_usage)
			system_statistic.ompInfo.comminfo.max_mem = loc_sadb->mem_usage;
		
		// by helca 07.29 
		// PD ��踦 ����...
		for ( i=0 ; i < MAX_PROBE_DEV_NUM ; i++) {
			system_statistic.pdInfo[i].average_cpu[0] += (l3pd->l3ProbeDev[i].cpuInfo.usage * 10);
			if ( system_statistic.pdInfo[i].max_cpu[0] < (l3pd->l3ProbeDev[i].cpuInfo.usage * 10)) {
				system_statistic.pdInfo[i].max_cpu[0] = (l3pd->l3ProbeDev[i].cpuInfo.usage * 10);
			}
		}
		// PD mem ��踦 ����
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
		// disk ��踦 ����
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
		// msgQ ��踦 ����
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
