#ifndef _MSRPM_INIT_H_
#define _MSRPM_INIT_H_

#include "typedef.h"
#include "mems.h"
#include "hasho.h"


extern S32 dInitMSRPM(stMEMSINFO **pstMEMSINFO, stHASHOINFO **pstMSRPMHASH);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

#endif /* _MSRPM_INIT_H_ */
