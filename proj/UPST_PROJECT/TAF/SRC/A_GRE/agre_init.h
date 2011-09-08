#ifndef _AGRE_INIT_H_
#define _AGRE_INIT_H_

/**
 * Declare functions
 */
extern int dInit_Proc();
extern void UserControlledSignal(int sign);
extern void IgnoreSignal(int sign);
extern void SetUpSignal();

#endif	/* _AGRE_INIT_H_ */
