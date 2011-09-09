#ifndef __MOND_INIT_H__
#define __MOND_INIT_H__

extern int Init_Fidb(void);
extern int Init_SubFidb(void);
extern void UserControlledSignal(int sign);
extern void FinishProgram();
extern void IgnoreSignal(int sign);
extern int Init_LoadStat(void);
extern int Init_Keepalive(void);
extern int Load_KeepAlive(void);
extern int InitMonTotalShm(void);
extern int InitMonTotalShm(void);
extern int dGetSYSCFG(void);
extern void SetUpSignal(void);
extern int init_proc(void);

#endif /* __MOND_INIT_H__ */
