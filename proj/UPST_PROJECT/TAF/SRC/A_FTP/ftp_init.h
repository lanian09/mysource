#ifndef _FTP_INIT_H_
#define _FTP_INIT_H_

/**
 *	Declare func.
 */
extern int init_ipcs();
extern void UserControlledSignal(int sign);
extern void IgnoreSignal(int sign);
extern void SetUpSignal();
extern int dInitProc();
extern void FinishProgram();

#endif	/* _FTP_INIT_H_ */
