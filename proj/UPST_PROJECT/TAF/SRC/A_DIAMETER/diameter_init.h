#ifndef _DIAMETER_INIT_H_
#define _DIAMETER_INIT_H_

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"

// LIB
#include "mems.h"

/**
 * Declare functions
 */
extern int Init_TraceShm();
extern S32	dInitDIAMETERProc(stMEMSINFO **pstMEMS, stHASHOINFO **pstHASHO, stTIMERNINFO **pstTIMER);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);
extern void vDIATRANSTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);

#endif	/* _DIAMETER_INIT_H_ */
