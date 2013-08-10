#ifndef __IF_INIT_H__
#define __IF_INiT_H__

extern int  dReadSubSysInfo(char *szFileName);
extern void UserControlledSignal(int sign);
extern void FinishProgram();
extern void SetUpSignal();
extern void IgnoreSignal(int sign);
extern int  Init_Fidb();
extern int  dInitProc();

#endif /* __IF_INIT_H__ */
