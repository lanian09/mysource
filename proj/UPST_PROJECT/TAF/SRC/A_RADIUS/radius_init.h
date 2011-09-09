#ifndef _RADIUS_INIT_H_
#define _RADIUS_INIT_H_

/**
 * Include headers
 */
// LIB headers
#include "mems.h"
#include "hasho.h"
#include "timerN.h"


/**
 * Declare functions
 */
extern S32	dInitRADIUSProc(stMEMSINFO **pstMEMSINFO, stHASHOINFO **pstHASHOINFO, stTIMERNINFO **pstTIMERNINFO);
extern int dReadIPPool(void);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);
extern void vRADIUSTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);

#endif	/* _RADIUS_INIT_H_ */
