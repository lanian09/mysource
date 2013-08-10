#ifndef _ONLINE_INIT_H_
#define _ONLINE_INIT_H_

/**
 * Include headers
 */

// LIB
#include "typedef.h"
#include "mems.h"

/**
 * Declare functions
 */
extern S32 dInitOnline(stMEMSINFO **pMEMSINFO, stHASHOINFO **pHASH);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);


#endif	/* _ONLINE_INIT_H_ */
