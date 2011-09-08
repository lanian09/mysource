#ifndef __CHSMD_CPU_H__
#define __CHSMD_CPU_H__


typedef struct _st_cpu_stat{
	double user;
	double nice;
	double sys;
	double idle;
	double use;
} st_cpu_state;

extern int cpu_compute( st_cpu_state* );

#endif /* __CHSMD_CPU_H__ */
