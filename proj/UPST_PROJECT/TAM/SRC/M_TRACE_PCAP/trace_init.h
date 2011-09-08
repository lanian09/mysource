#ifndef _TRACE_INIT_H_
#define _TRACE_INIT_H_

/**
 *	Declare functions
 */
extern int dInit_Proc(void);
extern void SetUpSignal();
extern void UserControlledSignal(int sign);
extern void FinishProgram();
extern void IgnoreSignal(int sign);
extern void InitTraceFile();
extern void InitServTraceFile();

#endif	/* _TRACE_INIT_H_ */
