#ifndef __CHSMD_INIT_H__
#define __CHSMD_INIT_H__

#include "msgdef.h"		    /* st_Msg */
#include "chsmd_disk.h"		/* st_SoldfList */

#define CURR_PWR_CNT 2
#define CURR_NTP_CNT 2
#define CURR_FAN_CNT 6
#define CURR_PORT_CNT 2

#define		NO_MSG					0  /* MSG Queue NO MSG */
#define 	KillMC					2
#define 	StartMC					3

#define 	PROC_CHECK_LIMIT		2 /* 프로세스 체크 타임 */
#define     HW_CHECK_LIMIT          1 /* HW CHECK TIME */
#define     DISK_CHECK_LIMIT        1 /* DISK CHECK TIME */
#define     MIRROR_CHECK_LIMIT      10 /* MIRROR CHECK TIME */

extern int IsRcvedMessage(pst_MsgQ *pstMsg);

extern int Init_STATUS_SHM_VALUE(void);
extern int read_FIDB(void);
extern int write_FIDB(void);
extern int dGetSYSCFG(st_SoldfList *pstSolDfList);
extern int init_CHSMD(void);
extern int init_prcmd(void);
extern int dInit_Mirror_Timer(int *dTerm);
extern int dGetBlocks(char *fn, char (*p)[30]);
extern void FinishProgram(void);
extern void AlarmSignal(int sign);
extern void IgnoreSignal(int sign);
extern void UserControlledSignal(int sign);
extern int dGetBlockBIN(char *sBlockName, char *sBinName, int dBinLength);

#endif /* __CHSMD_INIT_H__ */
