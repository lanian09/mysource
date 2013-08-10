#ifndef _SIPM_INIT_H_
#define _SIPM_INIT_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"

/**
 * Declare functions
 */
extern S32 dInitSIPM(stMEMSINFO **pstMEMSINFO, stHASHOINFO **pstSIPMHASH);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

#endif	/* _SIPM_INIT_H_ */
