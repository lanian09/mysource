#ifndef _INET_INIT_H_
#define _INET_INIT_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "hasho.h"
#include "mems.h"
#include "timerN.h"

/**
 * Declare functions
 */
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);
extern void vINETTimerReConstruct(stHASHOINFO *pINETHASH, stTIMERNINFO *pTIMER);
extern S32 dInitINET(stMEMSINFO **pMEMSINFO, stHASHOINFO **pCALLHASH, stHASHOINFO **pINETHASH, stTIMERNINFO **pTIMER, S32 dProcNum);

#endif	/* _INET_INIT_H_ */
