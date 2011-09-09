#ifndef _WIPI_INIT_H_
#define _WIPI_INIT_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"
#include "hasho.h"
#include "hashg.h"

/**
 *	Declare functions
 */
extern S32 dInitWIPI(stMEMSINFO **pMEMSINFO, stHASHGINFO **pLWIPIHASH);
extern void ReadData_WIPI(stHASHGINFO *pLWIPIHASH);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

#endif /* _WIPI_INIT_H_ */
