#ifndef _JNC_INIT_H_
#define _JNC_INIT_H_

#include "typedef.h"
#include "mems.h"
#include "hasho.h"

extern void	SetUpSignal(void);
extern void	UserControlledSignal(S32 isign);
extern void IgnoreSignal(S32 isign);
extern S32 dInitJNC(stMEMSINFO **pMEMSINFO, stHASHOINFO **pJNCHASH);
extern void FinishProgram(void);

#endif /* _JNC_INIT_H_ */
