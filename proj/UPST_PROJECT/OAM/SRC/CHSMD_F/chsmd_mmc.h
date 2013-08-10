#ifndef __CHSMD_MMC_H__
#define __CHSMD_MMC_H__

#include "mmcdef.h"		/* mml_msg, dbm_msg_t */

/* mmc_func.c */
extern int dAct_prc(mml_msg *ml);
extern int dDact_prc(mml_msg *ml);
extern int dDis_prc(mml_msg *ml);
extern int dSysInfo(mml_msg *ml);
extern int dGetVerBlock(char *fn, char (*p)[30]);
extern int dSendToNtam(mml_msg *ml, dbm_msg_t *smsg);
extern int MMC_Handle_Proc(mml_msg *mmsg, long mtype, long long  llIndex);
extern int auto_restart_the_process(int idx);

#endif /* __CHSMD_MMC_H__ */
