#ifndef __CHSMD_MMC_H__
#define __CHSMD_MMC_H__

enum {
	TYPE_MONITOR = 1,
	TYPE_MIRROR,
	TYPE_ONE,
	TYPE_ALL,
	TYPE_POWER
};

/* mmc.c */
extern int dAct_prc(mml_msg *ml);
extern int dDact_prc(mml_msg *ml);
extern int dDis_prc(mml_msg *ml);
extern int dDis_SysInfo(mml_msg *ml);
extern int ntaf_proc_ctl(mml_msg *ml);
extern int MMC_Handle_Proc(mml_msg *mmsg, long mtype);

/* mmc_func.c */
extern int dDis_prc(mml_msg *ml);
extern int dDis_SysInfo(mml_msg *ml);
extern int dAct_prc(mml_msg *ml);
extern int dDact_prc(mml_msg *ml);
extern int ntaf_proc_ctl(mml_msg *ml);
extern int taf_sys_info(mml_msg *ml);
extern int dMaskDirectPort(mml_msg *ml);
extern int dUmaskDirectPort(mml_msg *ml);
extern int dMaskSwitchPort(mml_msg *ml);
extern int dUmaskSwitchPort(mml_msg *ml);
extern int dSendMess(mml_msg *ml, dbm_msg_t *smsg, int dTotPage, int dCurPage);

extern void Send_CondMess(int sysno, int loctype, char invtype, short invno, char almstatus, char oldalm);



#endif /* __CHSMD_MMC_H__ */
