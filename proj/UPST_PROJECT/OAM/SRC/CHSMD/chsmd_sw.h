#ifndef __CHSMD_SW_H__
#define __CHSMD_SW_H__

extern int dCheckMySQLD(void);
extern void Send_CondMess(int sysno, int loctype, char invtype, short invno, char almstatus, char oldalm);
extern void check_software(void);


extern int get_proc_id(char *name);
extern int auto_restart_the_process(int idx);


#endif /* __CHSMD_SW_H__ */
