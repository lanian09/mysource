#ifndef __O_SVCMON_INIT_H__
#define __O_SVCMON_INIT_H__

#include "typedef.h"
#include "hasho.h"

extern S32 dInitOSVCMON(stHASHOINFO **pDefHash, stHASHOINFO **pAMonHash, stHASHOINFO **pSMonHash, stHASHOINFO **pThresHash, stHASHOINFO **pNasIPHash);
extern void SetUpSignal(void);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);
extern S32 dReadDefData(stHASHOINFO *pDefHash);
extern void UserControlledSignal(S32 isign);


#endif /* __O_SVCMON_INIT_H__ */
