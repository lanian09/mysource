#ifndef __IF_INIT_H__
#define __IF_INIT_H__

#define DEF_LOGOUT  0
#define DEF_CONNECT 3

extern void SetUpSignal();
extern void IgnoreSignal(int sign);
extern void FinishProgram(int dSocket);
extern void UserControlledSignal(int sign);
extern int  dInitProc();
#endif /* __IF_INIT_H__ */
