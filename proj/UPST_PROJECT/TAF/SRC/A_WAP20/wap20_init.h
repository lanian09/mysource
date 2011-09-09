#ifndef _WAP20_INIT_H_
#define _WAP20_INIT_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"

/**
 *	Declare func.
 */
extern S32 dInitWAP20(stMEMSINFO **pMEMSINFO);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

#endif /* _WAP20_INIT_H_ */
