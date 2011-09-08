#ifndef _L2TP_INIT_H_
#define _L2TP_INIT_H_

#include "typedef.h"
#include "mems.h"
#include "hasho.h"
#include "timerN.h"

/**
 * Declare functions
 */
int Init_TraceShm();
S32	dInitL2TPProc(stMEMSINFO **pstMEMSINFO);
void SetUpSignal(void);
void UserControlledSignal(S32 isign);
void FinishProgram(void);
void IgnoreSignal(S32 isign);
void vTunnelIDTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);
void vSessionIDTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);
void vControlTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);
void vCallsessTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);

#endif	/* _L2TP_INIT_H_ */
