#include "samd.h"

#ifdef TRU64
#include <sys/table.h>
#include <mach/vm_statistics.h>

extern	SFM_SysCommMsgType *loc_sadb;
extern	STM_LoadStatisticMsgType system_statistic;
extern	int		statistic_cnt;
extern	char	trcBuf[4096], trcTmp[1024];
extern	int		trcFlag, trcLogFlag;

void    Error(char *);
int getMemUsage_sh (void);


GetHwInfo()
{
	struct tbl_sysinfo		sibuf;
	vm_statistics_data_t	vmstats;
	int		r_tot;
	long	old_ticks[4],new_ticks[4],diff_ticks[4];
	long	delta_ticks=0;
	int		i;

	/* CPU의 사용량을 계산한다. */
	loc_sadb->cpuCount = 1; /* TRU64는 모든 CPU의 값을 합해서 사용한다 */

	if (table(TBL_SYSINFO,0,&sibuf,1,sizeof(struct tbl_sysinfo))<0) {
		Error("TBL_SYSINFO");
	}
	old_ticks[0] = sibuf.si_user ; old_ticks[1] = sibuf.si_nice;
	old_ticks[2] = sibuf.si_sys  ; old_ticks[3] = sibuf.si_idle;

	commlib_microSleep(100000);
	if (table(TBL_SYSINFO,0,&sibuf,1,sizeof(struct tbl_sysinfo))<0) {
		Error("TBL_SYSINFO");
	}
	new_ticks[0] = sibuf.si_user ; new_ticks[1] = sibuf.si_nice;
	new_ticks[2] = sibuf.si_sys  ; new_ticks[3] = sibuf.si_idle;

	for(i=0;i<4;i++) {
		diff_ticks[i] = new_ticks[i] - old_ticks[i];
		delta_ticks += diff_ticks[i];
	}

	if(delta_ticks) {
		loc_sadb->cpu_usage[0] = 1000 - (long)(diff_ticks[3]*1000/delta_ticks);
	}

	/* MEM의 사용량을 계산한다. */
#if 1
	(void) vm_statistics(task_self(),&vmstats);
	r_tot = pagetok((vmstats.free_count +
		vmstats.active_count + vmstats.inactive_count +
		vmstats.wire_count ))/1024;
	loc_sadb->mem_usage = (long) (1000*pagetok(vmstats.active_count + 
		vmstats.wire_count)/1024) /r_tot;
#endif

#if 0 /* shell command : "vmstat -r 1" */
	loc_sadb->mem_usage = getMemUsage_sh ();
#endif

	/* 통계 데이타를 위해 */
	statistic_cnt++;
#ifdef DEBUG
fprintf(stderr,"jean2222 statistic_cnt %d\n", statistic_cnt);
#endif
	for ( i=0 ; i < loc_sadb->cpuCount ; i++) {
		system_statistic.average_cpu[i] += loc_sadb->cpu_usage[i];
		if (system_statistic.max_cpu[i] < loc_sadb->cpu_usage[i] ) {
			system_statistic.max_cpu[i] = loc_sadb->cpu_usage[i];
		}
	}
	system_statistic.average_mem += loc_sadb->mem_usage;
	if (system_statistic.max_mem < loc_sadb->mem_usage)
		system_statistic.max_mem = loc_sadb->mem_usage;
}

void Error(char *error_name)
{
	sprintf(trcBuf,"[samd_tru64_sys] %s = %s\n", error_name, strerror(errno));
	trclib_writeLogErr(FL, trcBuf);
	exit(1);
}

int getMemUsage_sh (void)
{
	char	tmp[256], lineBuf[256], *ptr, *next, token[8][32];
	long	act, free, wire;
	int		i, usage=0, pageSize, val;
	FILE	*fp;

	strcpy (tmp, "/tmp/samd_mem.tmp");
	sprintf (lineBuf, "vmstat -r 1 > %s", tmp);
	system (lineBuf);

	if ((fp = fopen (tmp, "r")) == NULL) {
		sprintf(trcBuf,"[getMemUsage_sh] fopen fail[%s]; err=%d(%s)\n", tmp, errno, strerror(errno));
		trclib_writeLogErr(FL, trcBuf);
		return usage;
	}

	// analyze first line; get page_size
	//
	if (fgets (lineBuf, sizeof(lineBuf), fp) == NULL) {
		sprintf(trcBuf,"[getMemUsage_sh] fgets fail first line; fname=%s\n", tmp);
		trclib_writeLogErr(FL, trcBuf);
		fclose(fp);
		return usage;
	}
	if ((ptr = (char*)strtok_r(lineBuf,"=",&next)) == NULL) {
		sprintf(trcBuf,"[getMemUsage_sh] strtok_r fail pageSize; fname=%s, first_line\n", tmp);
		trclib_writeLogErr(FL, trcBuf);
		fclose(fp);
		return usage;
	}
	pageSize = strtol (next,0,0);

	// skip next 2 lines
	//
	if (fgets (lineBuf, sizeof(lineBuf), fp) == NULL) {
		sprintf(trcBuf,"[getMemUsage_sh] fgets fail second line; fname=%s\n", tmp);
		trclib_writeLogErr(FL, trcBuf);
		fclose(fp);
		return usage;
	}
	if (fgets (lineBuf, sizeof(lineBuf), fp) == NULL) {
		sprintf(trcBuf,"[getMemUsage_sh] fgets fail third line; fname=%s\n", tmp);
		trclib_writeLogErr(FL, trcBuf);
		fclose(fp);
		return usage;
	}

	// analyze fourth line; get page_counts
	//
	if (fgets (lineBuf, sizeof(lineBuf), fp) == NULL) {
		sprintf(trcBuf,"[getMemUsage_sh] fgets fail fourth line; fname=%s\n", tmp);
		trclib_writeLogErr(FL, trcBuf);
		fclose(fp);
		return usage;
	}
	if (sscanf (lineBuf, "%s%s%s %s%s%s",
				token[0], token[1], token[2], token[3], token[4], token[5]) < 6) {
		sprintf(trcBuf,"[getMemUsage_sh] sscanf fail fourth line; fname=%s, lineBuf=%s\n", tmp, lineBuf);
		trclib_writeLogErr(FL, trcBuf);
		fclose(fp);
		return usage;
	}

	// extract active page count
	//
	act = strtol(token[3],0,0); // get page_count
	for (ptr=token[3]; isdigit(*ptr); ptr++); // get unit character if exist
	if (*ptr == 'K') act = act * 1024;
	if (*ptr == 'M') act = act * 1024 * 1024;
	if (*ptr == 'G') act = act * 1024 * 1024 * 1024; 

	// extract free page count
	//
	free = strtol(token[4],0,0); // get page_count
	for (ptr=token[4]; isdigit(*ptr); ptr++); // get unit character if exist
	if (*ptr == 'K') free = free * 1024;
	if (*ptr == 'M') free = free * 1024 * 1024;
	if (*ptr == 'G') free = free * 1024 * 1024 * 1024; 

	// extract free page count
	//
	wire = strtol(token[5],0,0); // get page_count
	for (ptr=token[5]; isdigit(*ptr); ptr++); // get unit character if exist
	if (*ptr == 'K') wire = wire * 1024;
	if (*ptr == 'M') wire = wire * 1024 * 1024;
	if (*ptr == 'G') wire = wire * 1024 * 1024 * 1024; 

	// calcurate usage
	//
	usage = (1000 * (act + wire)) / (act + free + wire);
#ifdef DEBUG
	fprintf(stderr," act=%d, free=%d, wire=%d, usage=%d \n", act, free, wire, usage);
#endif

	fclose(fp);
	return usage;
}

#endif /* TRU64 */
