#ifndef __COND_FUNC_H__
#define __COND_FUNC_H__

#include <time.h>

#define IDX_CPU			0
#define IDX_MEM			1
#define IDX_QUE			2
#define IDX_NIFO		3
#define IDX_DBALIVE		4
#define IDX_DISK		5
#define IDX_LINK		9	/*	MAX_NTAM_LINK[8]	*/
#define IDX_CHNL		57	/*	MAX_NTAF_CHNL[8]	*/
#define IDX_PWR			73	/*	MAX_HW_PWR[4]		*/
#define IDX_FAN			79	/*	MAX_HW_FAN[8]		*/
#define IDX_DISKAR		87	/*	MAX_DISK_ARRAY[128]	*/

extern void  set_time(char loctype, char invtype, char invno, time_t tWhen);
extern char* szCvtIPAddr(unsigned int uiIP);
extern char* cvt_ipaddr(unsigned int uiIP);
extern char* cvtTime(time_t when);

#endif /* __COND_FUNC_H__ */
