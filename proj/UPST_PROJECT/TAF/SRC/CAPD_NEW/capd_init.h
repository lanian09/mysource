#ifndef _CAPD_INIT_H_
#define _CAPD_INIT_H_

/**
 * Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"

/**
 * Declare functions
 */
S32 dInitCapd(stMEMSINFO **pMEMSINFO, stCIFO **pCIFO);
void UserControlledSignal(int sign);
void IgnoreSignal(int sign);
void SetUpSignal();
void FinishProgram();

#endif	/* _CAPD_INIT_H_ */
