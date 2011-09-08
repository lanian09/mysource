#ifndef __SI_LOG_INIT_H__
#define __SI_LOG_INIT_H__

extern int dInitProc();
extern void FinishProgram();
extern void SetUpSignal();
extern void UserControlledSignal(int sign);
extern void IgnoreSignal(int sign);

#endif /* __SI_LOG_INIT_H__ */
