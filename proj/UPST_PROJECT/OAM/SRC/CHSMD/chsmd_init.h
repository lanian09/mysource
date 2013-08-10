#ifndef __CHSMD_INIT_H__
#define __CHSMD_INIT_H__

#include "chsmd_disk.h"

#define NO_MSG      0
#define KillMC		2
#define StartMC		3

#define PROC_CHECK_LIMIT		1
#define	HW_CHECK_LIMIT			3
#define	DIRECT_CHECK_LIMIT		30
#define THREAD_CHECK_INTERVAL	60
#define	DB_CHECK_LIMIT			60

#define FORK_CHECK_LIMIT		180 /*** watch_dog process check **********/
#define CURR_PWR_CNT	4
#define CURR_FAN_CNT	6
#define CURR_NTP_CNT	2

#define DEF_SERVER_IP_LEN 16

#define FILE_SWITCH_CONF	DATA_PATH"/SWITCH.dat"
#define FILE_DIRECTOR_CONF  DATA_PATH"/DIRECTOR.dat"

extern int IsRcvedMessage(pst_MsgQ *pstMsg);
extern void *CheckDirector_init(void *arg);
extern void sigHandler_DirPthrd(int signum);
extern void sigWaitHandler_DirPthrd(int signum);
extern void *CheckSwitch_init(void *arg);
extern void sigHandler_SWPthrd(int signum);
extern void sigWaitHandler_SWPthrd(int signum);

extern int Init_Keep_Load(void);
extern int dValidIP(char *IPaddr);
extern int dGetDirCFG(void);
extern int dGetSwitchCFG(void);
extern void write_FIDB(void);
extern void Init_Chnl(void);
extern void Init_STATUS_SHM_VALUE(void);
extern void FinishProgram(void);
extern int  dInitProcess(void);
extern int  dGetSYSCFG(st_SoldfList *pstSolDfList);
extern void signal_handling(void);
extern int  dGetBlocks(char *fn, char (*p)[30]);

#endif /* __CHSMD_INIT_H__ */
