#ifndef _SIP_INIT_H_
#define _SIP_INIT_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"

/**
 * Declare functions
 */
S32 dInitSIP(stMEMSINFO **pMEMSINFO);
void SetUpSignal(void);
void UserControlledSignal(S32 isign);
void FinishProgram(void);
void IgnoreSignal(S32 isign);

#endif	/* _SIP_INIT_H_	*/
