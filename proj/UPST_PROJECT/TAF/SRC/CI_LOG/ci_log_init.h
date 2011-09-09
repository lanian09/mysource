#ifndef _CI_LOG_INIT_H_
#define _CI_LOG_INIT_H_

/**
 *	Include headers
 */
// LOG
#include "mems.h"

/**
 *	Declare func.
 */
extern S32 dInitCI_LOG(stMEMSINFO **pMEMSINFO);
extern void UserControlledSignal(int sign);
extern void FinishProgram();
extern void IgnoreSignal(int sign);
extern void SetUpSignal();

#endif	/* _CI_LOG_INIT_H_ */
