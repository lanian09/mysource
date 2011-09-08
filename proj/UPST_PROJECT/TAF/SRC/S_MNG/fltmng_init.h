#ifndef __FLTMNG_INIT_H__
#define __FLTMNG_INIT_H__

extern int dInit_Info();
extern void Init_Signal();
extern void IgnoreSignal(int dSigNo);
extern void FinishProgram(int dSigNo);
extern int dInitProc();
#endif /* __FLTMNG_INIT_H__ */
