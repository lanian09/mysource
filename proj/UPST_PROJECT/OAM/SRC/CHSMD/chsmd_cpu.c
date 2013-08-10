/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>
#include <string.h>
#include <errno.h>
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "chsmd_cpu.h"

extern pst_NTAM fidb;

/*******************************************************************************
 * CONVERT STATUS VALUE
*******************************************************************************/
long percentages(int cnt, unsigned long long *out, register unsigned long long *new, register unsigned long long *old, unsigned long long *diffs)
{
	register int					i;
	register unsigned long long		change, total_change, *dp;
	unsigned long long				half_total;

	/* initialization */
	total_change	= 0;
	dp				= diffs;

	/***************************************************************************
	* calculate changes for each state and the overall change
	***************************************************************************/
	for(i = 0; i < cnt; i++)
	{
		if( (change = *new - *old) < 0)
		{
			/* this only happens when the counter wraps */
			change = ((unsigned long long)(*new)-(unsigned long long)(*old));
		}
		total_change	+= (*dp++ = change);
		*old++			= *new++;
	}

	/***************************************************************************
	* avoid divide by zero potential
	***************************************************************************/
	if (total_change == 0)
		total_change = 1;

	/***************************************************************************
	* calculate percentages based on overall change, rounding up
	***************************************************************************/
	half_total = total_change / 2ll;
	for (i = 0; i < cnt; i++)
		*out++ = ((*diffs++ * 1000 + half_total) / total_change);

	/* return the total in case the caller wants to use it */
	return(total_change);
}

/*******************************************************************************
 * GET CPU ROAD VLAUE FROM /pro/stat FILE
*******************************************************************************/
int Read_CpuLoad(unsigned long long *system, unsigned long long *user, unsigned long long *nice, unsigned long long *idle, unsigned long long *total)
{
	char						buff[1024], path[20] = "/proc/stat" ;
	unsigned long long			cpustate[4];
	static unsigned long long	oldstate[4], newstate[4], diffstate[4];
	FILE						*fp;

	if( (fp = fopen(path, "r")) == NULL)
	{
		log_print(LOGN_CRI,LH"FILE OPEN ERROR. FILE=%s"EH, LT,path,ET);
		return -1;
	}
	memset(buff, 0x00, 1024);
	fgets(buff, 1024, fp);
	fclose(fp);
	sscanf(buff, "%s %llu %llu %llu %llu", path, &newstate[0], &newstate[1], &newstate[2], &newstate[3]);

	/***************************************************************************
	* CONVERT CPU STATUS VALUE
	***************************************************************************/
	percentages(4, cpustate, newstate, oldstate, diffstate);

	*user	= cpustate[0];
	*nice	= cpustate[1];
	*system	= cpustate[2];
	*idle	= cpustate[3];

	*total	= *user + *nice + *system + *idle ;

	return 1;
}

int cpu_compute(st_cpu_state *cpu)
{
	double				fVal;
	char				szLoad[6];

	unsigned long long	sys, user, nice, idle, total;

	Read_CpuLoad(&sys, &user, &nice, &idle, &total);
	log_print(LOGN_INFO," USER [%6.2f] SYSTEM[%6.2f] NICE[%6.2f] IDLE[%6.2f]",
		(double)user/(double)total*100,
		(double)sys/(double)total*100,
		(double)nice/(double)total*100,
		(double)idle/(double)total*100);

	sprintf(szLoad, "%6.2f", 100.0 - (double)idle/(double)total*100);
	if( (fVal = 100.0 - (double)idle/(double)total*100) < 0) {
		fidb->cpusts.lMax	= 1000;
		fidb->cpusts.llCur	= 0;

	} else {
		cpu->user	= (double)user/10.;
		cpu->nice	= (double)nice/10.;
		cpu->sys	= (double)sys/10.;
		cpu->idle	= (double)idle/10.;
		cpu->use	= total - cpu->idle;

		log_print(LOGN_INFO, "USER[%6.2f] NICE[%6.2f] SYST[%6.2f] IDLE[%6.2f] USE[%6.2f]",
			cpu->user, cpu->nice, cpu->sys, cpu->idle, cpu->use );

		fidb->cpusts.lMax	= total;
		fidb->cpusts.llCur	= total - idle;
	}
	log_print(LOGN_INFO,"[CPU] CUR[%lld] TOT[%lld]", fidb->cpusts.llCur, fidb->cpusts.lMax);

	return 1;
}

