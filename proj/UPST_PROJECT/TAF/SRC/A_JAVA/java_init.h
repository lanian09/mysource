#ifndef _JAVA_INIT_H_
#define _JAVA_INIT_H_

#include "typedef.h"
#include "mems.h"

extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void IgnoreSignal(S32 isign);
extern S32 dInitJAVA(stMEMSINFO **pMEMSINFO);
extern void FinishProgram(void);
#endif /* _JAVA_INIT_H_ */
