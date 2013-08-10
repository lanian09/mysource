#ifndef _FV_INIT_H_
#define _FV_INIT_H_

/**
 *	Include headers
 */
// LIB
#include "mems.h"

/**
 *	Declare func.
 */
extern S32 dInitFV(stMEMSINFO **pMEMSINFO);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

#endif /* _FV_INIT_H_ */
