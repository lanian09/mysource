#ifndef __MSRPT_INIT_H__
#define __MSRPT_INIT_H__

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"
#include "hasho.h"
#include "timerN.h"

/**
 *	Declare functions
 */
extern S32 dInitMSRPT(stMEMSINFO **pstMEMSINFO, stHASHOINFO **pstMSRPTHASH, stTIMERNINFO **pstTIMERNINFO);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);
extern void vMSRPTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);

#endif	/* __MSRPT_INIT_H__ */
