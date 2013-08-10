#ifndef _ROAM_INIT_H_
#define _ROAM_INIT_H_

/**
 *	INCLUDE HEADER FILES
 */
#include "hasho.h"
#include "timerN.h"

/**
 *	DECLARE FUNCTIONS
 */
extern int dInitRPPI();
extern void SetUpSignal();
extern void UserControlledSignal(int sign);
extern void FinishProgram(void);
extern void IgnoreSignal(int sign); 
extern void vRPPITimerReConstruct(stHASHOINFO *pRPPIHASH, stTIMERNINFO *pTIMER);

#endif	/* _ROAM_INIT_H_ */
