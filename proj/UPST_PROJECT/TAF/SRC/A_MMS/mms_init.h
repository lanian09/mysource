#ifndef _MMS_INIT_H_
#define _MMS_INIT_H_

#include "typedef.h"
#include "mems.h"

extern S32 dInitMMS(stMEMSINFO **pMEMSINFO);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

#endif /* _MMS_INIT_H_ */
