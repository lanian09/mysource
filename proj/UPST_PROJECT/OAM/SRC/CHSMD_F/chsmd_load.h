#ifndef __CHSMD_LOAD_H__
#define __CHSMD_LOAD_H__

typedef struct _st_linkdev
{
    char          szDevName[7];
    unsigned char ucFlag;
}st_linkdev, *pst_linkdev;

#define DEF_LINKDEV_SIZE sizeof(st_linkdev)

extern char cDecideCurrentStatus(unsigned long luCurLoad, unsigned long luCriticalValue, unsigned long luMajorValue, unsigned long luMinorValue);
extern int Read_CpuLoad(unsigned long long *system, unsigned long long *user, unsigned long long *nice, unsigned long long *idle, unsigned long long *total);
extern int dCheckLoad(int dType, int dLoadNo, char *sCurLoad);
extern long long percentages(int cnt, long long *out, register long long *new, register long long *old, long long *diffs);
extern int cpu_compute(void);
extern int mem_compute(void);
extern int queue_compute(void);
extern int nifo_compute(void);
extern int dLinkCheck(void);
#endif /* __CHSMD_LOAD_H__ */
